/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
// Standard library
#include <chrono>
#include <string>
#include <sstream>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <regex>

// Unify components
#include "sl_status.h"
#include "sl_status_strings.h"
#include "sl_log.h"

// This component
#include "mqtt_client.hpp"
#include "mqtt_client.h"
#include "mqtt_wrapper.h"
#define LOG_TAG "mqtt_client"

// Boilerplate

// Mosquitto specific callback declarations. Those are not in the header because
// their signature is very much Mosquitto-specific.
void mqtt_client_on_connect_callback(void *instance,
                                     void *obj,
                                     int return_code);

void mqtt_client_on_disconnect_callback(void *instance,
                                        void *obj,
                                        int result_code);

void mqtt_client_on_message_callback(void *instance,
                                     void *obj,
                                     const struct mqtt_message *message);

// C++ implementation

mqtt_client::mqtt_client(
  const std::string &hostname,
  const uint32_t port,
  const std::string &client_id,
  const std::string &cafile,
  const std::string &certfile,
  const std::string &keyfile,
  std::function<int(const int, void *)> event_sender,
  std::function<void(const int, const unsigned long)> delayed_event_sender,
  std::function<int(const int, void *)> event_counter) :
  event_sender(event_sender),
  delayed_event_sender(delayed_event_sender),
  event_counter(event_counter),
  hostname(hostname),
  port(port),
  socket_file_descriptor(-1),
  next_reconnect_backoff(std::chrono::milliseconds(
    MQTT_CLIENT_MINIMUM_RECONNECT_BACKOFF_MILLISECONDS)),
  max_reconnect_backoff(std::chrono::milliseconds(
    MQTT_CLIENT_MAXIMUM_RECONNECT_BACKOFF_MILLISECONDS)),
  mqtt_client_id(client_id),
  mqtt_cafile(cafile),
  mqtt_certfile(certfile),
  mqtt_keyfile(keyfile),
  current_state(&mqtt_client_fsm_disconnected::get_instance())
{}

sl_status_t mqtt_client::setup()
{
  if (mqtt_wrapper_lib_init() != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  sl_log_debug(LOG_TAG,
               "Creating a new MQTT instance: %s\n",
               mqtt_client_id.c_str());
  mqtt_client_library_instance
    = mqtt_wrapper_new(mqtt_client_id.c_str(), true, this);
  if (mqtt_client_library_instance == nullptr) {
    sl_log_error(LOG_TAG,
                 "Failed to create a new MQTT instance. "
                 "Aborting initialization. ");
    return SL_STATUS_FAIL;
  }

  if (!mqtt_cafile.empty() && !mqtt_certfile.empty() && !mqtt_keyfile.empty()) {
    sl_log_debug(LOG_TAG, "Setting Certificate based TLS.\n");
    if (mqtt_wrapper_tls_set(mqtt_client_library_instance,
                             mqtt_cafile.c_str(),
                             mqtt_certfile.c_str(),
                             mqtt_keyfile.c_str())
        != SL_STATUS_OK) {
      sl_log_error(LOG_TAG, "Certificate based TLS setting failed\n");
      return SL_STATUS_FAIL;
    }
  } else if (!mqtt_cafile.empty() || !mqtt_certfile.empty()
             || !mqtt_keyfile.empty()) {
    sl_log_error(LOG_TAG,
                 "One of the TLS certificate configuration file missing."
                 "Certificate based TLS setting failed\n");
    return SL_STATUS_FAIL;
  }

  sl_log_debug(
    LOG_TAG,
    "Registering MQTT wrapper callbacks for connect/disconnect/messages.\n");
  mqtt_wrapper_connect_callback_set(mqtt_client_library_instance,
                                    mqtt_client_on_connect_callback);

  mqtt_wrapper_disconnect_callback_set(mqtt_client_library_instance,
                                       mqtt_client_on_disconnect_callback);

  mqtt_wrapper_message_callback_set(mqtt_client_library_instance,
                                    mqtt_client_on_message_callback);

  return SL_STATUS_OK;
}

sl_status_t mqtt_client::teardown()
{
  sl_log_debug(LOG_TAG, "Cleaning up Mosquitto instance and library.\n");
  mqtt_wrapper_destroy(mqtt_client_library_instance);
  return mqtt_wrapper_lib_cleanup();
}

int mqtt_client::file_descriptor()
{
  return socket_file_descriptor;
}

void mqtt_client::poll()
{
  if (current_state == &mqtt_client_fsm_disconnected::get_instance()) {
    return;
  }
  // We have already select()ed on the socket, so we're going to run the individual
  // read-, write-, and misc-operations.
  switch (auto status_code
          = mqtt_wrapper_loop_read(mqtt_client_library_instance)) {
    case SL_STATUS_OK:
      break;
    case SL_STATUS_ERRNO:
      sl_log_error(LOG_TAG,
                   "Unable to perform an MQTT read: %s\n",
                   std::strerror(errno));
      break;
    default:
      sl_log_error(LOG_TAG,
                   "Unable to perform an MQTT read: %s (%s)\n",
                   sl_status_string(status_code),
                   sl_status_string_verbose(status_code));
      break;
  }

  switch (auto status_code
          = mqtt_wrapper_loop_write(mqtt_client_library_instance)) {
    case SL_STATUS_OK:
      break;
    case SL_STATUS_ERRNO:
      sl_log_error(LOG_TAG,
                   "Unable to perform an MQTT write: %s\n",
                   std::strerror(errno));
      break;
    default:
      sl_log_error(LOG_TAG,
                   "Unable to perform an MQTT write: %s (%s)\n",
                   sl_status_string(status_code),
                   sl_status_string_verbose(status_code));
      break;
  }

  if (auto status_code
      = mqtt_wrapper_loop_misc(mqtt_client_library_instance) != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Unable to perform 'misc.' MQTT activities: %s (%s)\n",
                 sl_status_string(status_code),
                 sl_status_string_verbose(status_code));
  }
}

void mqtt_client::disconnect()
{
  sl_log_debug(LOG_TAG, "Attempting graceful disconnect from broker.\n");
  // Flush the publishing-queue
  int messages_waiting = publishing_queue.size();
  int messages_flushed = 0;
  while (publish_messages_waiting()) {
    if (publish_to_broker(true) != SL_STATUS_OK) {
      // As soon as we have an error we'll give up in order not to go
      // mad spinning on something that will most likely never succeed.
      break;
    }
    ++messages_flushed;
  }
  sl_log_debug(LOG_TAG,
               "Pushed %d of %d queued messages to broker.\n",
               messages_flushed,
               messages_waiting);

  // Under Contiki, the event-loop has already exited by this point, so we need
  // to disconnect directly.
  int retval = mqtt_wrapper_disconnect(mqtt_client_library_instance);
  switch (retval) {
    case SL_STATUS_OK:
      sl_log_debug(LOG_TAG, "Disconnected.\n");
      break;
    case SL_STATUS_NET_MQTT_NO_CONN:
      sl_log_debug(LOG_TAG, "Connection not open.\n");
      break;
    case SL_STATUS_INVALID_PARAMETER:
      sl_log_critical(LOG_TAG,
                      "Invalid parameters. This should never happen.\n");
      break;
    default:
      sl_log_critical(
        LOG_TAG,
        "Unknown return value from mqtt_wrapper_disconnect: 0x%04x\n",
        retval);
      break;
  }
}

void mqtt_client::event(const int incoming_event)
{
  // All events get passed straight to the FSM.
  current_state->event(incoming_event, this);
}

// Publishing
void mqtt_client::publish(const std::string &topic,
                          const std::string &message,
                          size_t message_length,
                          bool retain)
{
  bool enqueue_message = true;

  if (retain) {
    if (message_length > 0) {
      //Build our set of retained topics
      retained_topics.insert(topic);
    } else {
      //Remove is this as retain delete message
      retained_topics.erase(topic);
    }
    // Try to find retain message with same topic
    // And replace message content and message_length
    // Try to find retained message with same topic,
    // change message content and message_length
    for (auto it = publishing_queue.begin(); it != publishing_queue.end();
         ++it) {
      if ((*it).retain && (*it).topic.compare(topic) == 0) {
        // variant1: modify content of existing message
        (*it).message.assign(message);
        (*it).message_length = message_length;
        enqueue_message      = false;
        break;
      }
    }
  } else {
    //Remove is this as non-retained
    retained_topics.erase(topic);
  }

  if (enqueue_message) {
    message_queue_element_t new_message;
    new_message.topic.assign(topic);
    new_message.message.assign(message);
    new_message.message_length = message_length;
    new_message.retain         = retain;
    publishing_queue.push_back(std::move(new_message));
  }

  if (event_counter(MQTT_EVENT_PUBLISH, nullptr) == 0) {
    // We will only send publish-events if there aren't any.
    send_event(MQTT_EVENT_PUBLISH, nullptr);
  }
}

int mqtt_client::count_topics(const std::string &prefix_pattern)
{
  int count       = 0;
  bool full_match = retained_topics.count(prefix_pattern) > 0;

  // We want to locate all topics starting with pattern
  // As std::set is sorted we insert the prefix string into the set to
  // get an iterator which would point to the first potential match.
  // Then we do a substring match of the consecutive candidates,
  // until there are no more matches.
  //
  auto start = retained_topics.insert(prefix_pattern).first;
  auto end   = start;

  while ((end != retained_topics.end())
         && (*end).rfind(prefix_pattern, 0) == 0) {
    end++;
    count++;
  }

  if (!full_match) {
    retained_topics.erase(start);
    count = count - 1;
  }

  return count;
}

void mqtt_client::unretain(const std::string &prefix_pattern)
{
  // We want to locate all topics starting with pattern
  // As std::set is sorted we insert the prefix string into the set to
  // get an iterator which would point to the first potential match.
  // Then we do a substring match of the consecutive candidates,
  // until there are no more matches.

  auto [start, inserted] = retained_topics.insert(prefix_pattern);
  auto end               = start;
  // Check if we already have topic in retained_topics set
  if (!inserted) {
    publishing_queue.push_back({prefix_pattern, "", 0, true});
  } else {
    while ((end != retained_topics.end())
           && (*end).rfind(prefix_pattern, 0) == 0) {
      end++;
    }
    // Add topics to the publishing_queue, skip the first since its our probe.
    for (auto it = (start); it != end; it++) {
      if (it == start)
        continue;
      publishing_queue.push_back({*it, "", 0, true});
    }
  }
  retained_topics.erase(start, end);
  if (event_counter(MQTT_EVENT_PUBLISH, nullptr) == 0) {
    send_event(MQTT_EVENT_PUBLISH, nullptr);
  }
}

void mqtt_client::unretain_by_regex(const std::string &pattern)
{
  std::regex re(pattern);
  for (auto it = retained_topics.begin(); it != retained_topics.end();) {
    if (std::regex_match(*it, re)) {
      publishing_queue.push_back({*it, "", 0, true});
      retained_topics.erase(it++);
    } else {
      it++;
    }
  }
  if (event_counter(MQTT_EVENT_PUBLISH, nullptr) == 0) {
    send_event(MQTT_EVENT_PUBLISH, nullptr);
  }
}

bool mqtt_client::publish_messages_waiting()
{
  return !publishing_queue.empty();
}

sl_status_t mqtt_client::publish_to_broker(bool flushing)
{
  if (!publish_messages_waiting()) {
    return SL_STATUS_FAIL;
  }

  message_queue_element_t next_message = publishing_queue.front();
  int publish_retval = mqtt_wrapper_publish(mqtt_client_library_instance,
                                            nullptr,
                                            next_message.topic.c_str(),
                                            next_message.message_length,
                                            next_message.message.c_str(),
                                            MQTT_CLIENT_QoS,
                                            next_message.retain);
  if (publish_retval == SL_STATUS_OK) {
    publishing_queue.pop_front();
  } else {
    if (publish_retval == SL_STATUS_ERRNO) {
      sl_log_error(LOG_TAG,
                   "System error publishing to broker: %s\n",
                   std::strerror(errno));
    } else {
      sl_log_error(LOG_TAG,
                   "Error publishing to broker: %s(0x%04x): %s\n",
                   sl_status_string(publish_retval),
                   publish_retval,
                   sl_status_string_verbose(publish_retval));
    }
    on_disconnect(publish_retval);
    return publish_retval;
  }

  if (publish_messages_waiting() && (!flushing)
      && (event_counter(MQTT_EVENT_PUBLISH, nullptr) == 0)) {
    // When we're flushing the queue, that means we're disconnecting and
    // probably shutting down and don't have an event-loop anymore.
    send_event(MQTT_EVENT_PUBLISH, nullptr);
  }
  return publish_retval;
}

// Subscribing
void mqtt_client::subscribe(const std::string &topic,
                            const callback_info &callback)
{
  // Check for duplicate subscriptions
  for (const auto &[cb_topic, callbacks]: subscription_callbacks) {
    bool matches = false;

    // First we check if the topic-string is equivalent (accounting for wild-cards).
    if (mqtt_wrapper_topic_matches_sub(cb_topic.c_str(),
                                       topic.c_str(),
                                       &matches)
        == SL_STATUS_OK) {
      if (matches) {
        // We have a matching topic-string. Let's see if we have a pointer to
        // the same callback-function.
        for (const auto &cb: callbacks) {
          if (cb == callback) {
            sl_log_debug(
              LOG_TAG,
              "Equivalent pre-existing subscription. Dropping: %s.\n",
              topic.c_str());
            return;
          }
        }
        // Here, we have a matching topic-string with different pointer to
        // the callback-function. So, we added the callback to the internal
        // routing without sending subscription-message to the broker.
        if (cb_topic == topic) {
          auto pr = subscription_callbacks.insert(
            make_pair(topic, std::vector<callback_info>()));
          pr.first->second.push_back(callback);
          return;
        }
      }
    }
  }

  // Now we accept the new subscription
  sl_log_debug(LOG_TAG, "Subscribing to: %s\n", topic.c_str());
  auto pr = subscription_callbacks.insert(
    make_pair(topic, std::vector<callback_info>()));
  pr.first->second.push_back(callback);
  subscription_queue.push(topic);
  if (event_counter(MQTT_EVENT_SUBSCRIBE, nullptr) == 0) {
    // We will only send subscription-events if there aren't any.
    send_event(MQTT_EVENT_SUBSCRIBE, nullptr);
  }
}

// Re-subscribe to topics, e.g. on a reconnect if connection was lost
void mqtt_client::resubscribe()
{
  for (const auto &pair: subscription_callbacks) {
    subscription_queue.push(pair.first);
  }
  if ((subscription_callbacks.size() > 0)
      && (event_counter(MQTT_EVENT_SUBSCRIBE, nullptr) == 0)) {
    send_event(MQTT_EVENT_SUBSCRIBE, nullptr);
  }
}

bool mqtt_client::subscription_messages_waiting()
{
  return !subscription_queue.empty();
}

sl_status_t mqtt_client::subscribe_to_topic()
{
  if (!subscription_messages_waiting()) {
    return SL_STATUS_FAIL;
  }
  std::string new_topic = subscription_queue.front();
  int subscribe_retval  = mqtt_wrapper_subscribe(mqtt_client_library_instance,
                                                nullptr,
                                                new_topic.c_str(),
                                                MQTT_CLIENT_QoS);

  if (subscribe_retval == SL_STATUS_OK) {
    subscription_queue.pop();
    sl_log_debug(LOG_TAG, "Subscription to %s successful\n", new_topic.c_str());
  } else {
    if (subscribe_retval == SL_STATUS_ERRNO) {
      sl_log_error(LOG_TAG,
                   "System error subscribing to topic [%s]: %s\n",
                   new_topic.c_str(),
                   std::strerror(errno));
    } else {
      sl_log_error(LOG_TAG,
                   "Error subscribing to topic [%s]: %s(0x%04x): %s\n",
                   new_topic.c_str(),
                   sl_status_string(subscribe_retval),
                   subscribe_retval,
                   sl_status_string_verbose(subscribe_retval));
    }
    on_disconnect(subscribe_retval);
    return subscribe_retval;
  }
  if (subscription_messages_waiting()
      && (event_counter(MQTT_EVENT_SUBSCRIBE, nullptr) == 0)) {
    send_event(MQTT_EVENT_SUBSCRIBE, nullptr);
  }
  return subscribe_retval;
}

// Unsubscribing
void mqtt_client::unsubscribe(const std::string &topic,
                              const callback_info &callback)
// Remove our internal callback instantly.

{
  sl_log_debug(LOG_TAG, "Unsubscribing from: %s\n", topic.c_str());
  subscription_callbacks[topic].erase(
    std::remove(subscription_callbacks[topic].begin(),
                subscription_callbacks[topic].end(),
                callback),
    subscription_callbacks[topic].end());
  if (subscription_callbacks[topic].empty()) {
    unsubscription_queue.push(topic);
    if (event_counter(MQTT_EVENT_UNSUBSCRIBE, nullptr) == 0) {
      // We will only send unsubscription-events if there aren't any.
      send_event(MQTT_EVENT_UNSUBSCRIBE, nullptr);
    }
    subscription_callbacks.erase(topic);
  }
}

bool mqtt_client::unsubscription_messages_waiting()
{
  return !unsubscription_queue.empty();
}

sl_status_t mqtt_client::unsubscribe_from_topic()
{
  if (!unsubscription_messages_waiting()) {
    return SL_STATUS_FAIL;
  }
  std::string topic = unsubscription_queue.front();
  int unsubscribe_retval
    = mqtt_wrapper_unsubscribe(mqtt_client_library_instance,
                               nullptr,
                               topic.c_str());
  if (unsubscribe_retval == SL_STATUS_OK) {
    unsubscription_queue.pop();
    sl_log_debug(LOG_TAG, "Unsubscription from %s successful.", topic.c_str());
  } else {
    if (unsubscribe_retval == SL_STATUS_ERRNO) {
      sl_log_error(LOG_TAG,
                   "System error unsubscribing from topic [%s]: %s\n",
                   topic.c_str(),
                   std::strerror(errno));
    } else {
      sl_log_error(LOG_TAG,
                   "Error unsubscribing from topic [%s]: %s(0x%04x): %s\n",
                   topic.c_str(),
                   sl_status_string(unsubscribe_retval),
                   unsubscribe_retval,
                   sl_status_string_verbose(unsubscribe_retval));
    }
    on_disconnect(unsubscribe_retval);
    return unsubscribe_retval;
  }
  if (unsubscription_messages_waiting()
      && (event_counter(MQTT_EVENT_UNSUBSCRIBE, nullptr) == 0)) {
    send_event(MQTT_EVENT_UNSUBSCRIBE, nullptr);
  }
  return unsubscribe_retval;
}

// Wrapper for the function-pointer to the external event-posting mechanism.
// This has the same signature as Contiki's process_post() function.
void mqtt_client::send_event(const int outgoing_event, void *procdata)
{
  event_sender(outgoing_event, procdata);
}

void mqtt_client::send_delayed_event(const int outgoing_event,
                                     const std::chrono::milliseconds delay)
{
  delayed_event_sender(outgoing_event, delay.count());
}

// Transition between states. This runs the exit() and enter() functions.
void mqtt_client::transition(mqtt_client_fsm &new_state)
{
  current_state->exit(this);
  current_state = &new_state;
  current_state->enter(this);
}

void mqtt_client::on_connect_callback_set(
  connection_callback_t callback_function)
{
  on_connect_callback = callback_function;
}

void mqtt_client::on_disconnect_callback_set(
  connection_callback_t callback_function)
{
  on_disconnect_callback = callback_function;
}

void mqtt_client::before_disconnect_callback_set(
  connection_status_callback_t callback_function)
{
  before_disconnect_callback = callback_function;
}

void mqtt_client::after_connect_callback_set(
  connection_status_callback_t callback_function)
{
  after_connect_callback = callback_function;
}

sl_status_t mqtt_client::connect()
{
  int retval
    = mqtt_wrapper_connect(mqtt_client_library_instance,
                           hostname.c_str(),
                           port,
                           MQTT_CLIENT_KEEP_ALIVE_INTERVAL_MILLISECONDS / 1000);
  switch (retval) {
    case SL_STATUS_OK:
      sl_log_info(LOG_TAG,
                  "Connection to MQTT broker %s:%d established.\n",
                  hostname.c_str(),
                  port);
      this->after_connect_callback_call();
      break;
    case SL_STATUS_INVALID_PARAMETER:
      sl_log_error(LOG_TAG, "Invalid input-parameters (hostname or port).\n");
      // TODO Send some relevant events or assert()
      break;
    case SL_STATUS_ERRNO:
      sl_log_warning(LOG_TAG, "System error: %s\n", strerror(errno));
      // TODO Send some relevant events?
      break;
    default:
      sl_log_error(LOG_TAG,
                   "MQTT-wrapper returned an invalid status-code: 0x%04x\n",
                   retval);
      break;
  }
  poll();
  return retval;
}

// Mosquitto-callback. This gets wrapped in a C-function.
void mqtt_client::on_connect(int return_code)
{
  switch (return_code) {
    case 0:
      socket_file_descriptor
        = mqtt_wrapper_socket(mqtt_client_library_instance);
      send_event(MQTT_TRANSITION_CONNECTED, nullptr);
      break;
    case 1:   // Connection refused (unacceptable protocol version)
    case 2:   // Connection refused (identifier rejected)
    case 3:   // Connection refused (broker unavailable)
    default:  // 4-255 reserved for (Mosquitto) future use.
      break;
  }
  if (return_code != 0) {
    socket_file_descriptor = 0;
  }

  if (on_connect_callback != nullptr) {
    on_connect_callback(this, socket_file_descriptor);
  }
}

// Mosquitto-callback. This gets wrapped in a C-function.
void mqtt_client::on_disconnect(int result_code)
{
  if (result_code != 0) {
    if (result_code == SL_STATUS_ERRNO) {
      sl_log_warning(LOG_TAG,
                     "Unexpected disconnect: %s\n",
                     std::strerror(errno));
    } else {
      sl_log_warning(LOG_TAG,
                     "Unexpected disconnect: %s(0x%04x): %s\n",
                     sl_status_string(result_code),
                     result_code,
                     sl_status_string_verbose(result_code));
    }
    send_event(MQTT_TRANSITION_DISCONNECTED, nullptr);
    send_delayed_event(MQTT_EVENT_CONNECT, next_reconnect_backoff);
  }

  if (on_disconnect_callback != nullptr) {
    on_disconnect_callback(this, socket_file_descriptor);
  }
}

// Mosquitto-callback. This gets wrapped in a C-function.
void mqtt_client::on_message(const std::string &topic,
                             const std::string &message,
                             size_t message_length,
                             int message_qos,
                             bool message_retain)
{
  sl_log_debug(LOG_TAG,
               "mqtt_client::on_message: %s, %s, %d\n",
               topic.c_str(),
               message.c_str(),
               message_qos);
  if (subscription_callbacks.empty()) {
    return;
  }

  bool match_topic_result;

  for (const auto& [cb_topic, callbacks]: subscription_callbacks) {
    match_topic_result = false;
    mqtt_wrapper_topic_matches_sub(cb_topic.c_str(),
                                   topic.c_str(),
                                   &match_topic_result);

    if (match_topic_result) {
      for (const callback_info &cb: callbacks) {
        cb.callback(topic.c_str(), message.c_str(), message_length, cb.user);
      }
    }
  }
}

void mqtt_client::after_connect_callback_call()
{
  if (this->after_connect_callback != nullptr) {
    this->after_connect_callback();
  }
}

void mqtt_client::before_disconnect_callback_call()
{
  if (this->before_disconnect_callback != nullptr) {
    this->before_disconnect_callback();
  }
}

// C interface implementation.
// All of those functions/functionalities have a counterpart in C++-land.
mqtt_client_t mqtt_client_new(
  const char *hostname,
  const uint32_t port,
  const char *client_id,
  const char *cafile,
  const char *certfile,
  const char *keyfile,
  int (*event_sender)(const int event, void *procdata),
  void (*delayed_event_sender)(const int event,
                               const unsigned long milliseconds),
  int (*event_counter)(const int event, void *procdata))
{
  return new mqtt_client(hostname,
                         port,
                         client_id,
                         cafile,
                         certfile,
                         keyfile,
                         event_sender,
                         delayed_event_sender,
                         event_counter);
}

sl_status_t mqtt_client_setup(mqtt_client_t instance)
{
  if (instance)
    return instance->setup();
  else
    return SL_STATUS_NULL_POINTER;
}

void mqtt_client_disconnect(mqtt_client_t instance)
{
  if (instance)
    instance->before_disconnect_callback_call();

  if (instance)
    instance->disconnect();
}

sl_status_t mqtt_client_teardown(mqtt_client_t instance)
{
  int retval = instance->teardown();
  sl_log_debug(LOG_TAG, "Cleaning up mqtt_client instance.\n");
  return retval;
}

void mqtt_client_delete(mqtt_client_t instance)
{
  delete instance;
}

int mqtt_client_file_descriptor(mqtt_client_t instance)
{
  if (instance) {
    sl_log_debug(LOG_TAG, "Returning socket file-descriptor.\n");
    return instance->file_descriptor();
  }
  return -1;
}

void mqtt_client_poll(mqtt_client_t instance)
{
  if (instance)
    instance->poll();
}

void mqtt_client_event(mqtt_client_t instance, const int incoming_event)
{
  if (instance)
    instance->event(incoming_event);
}

void mqtt_client_publish(mqtt_client_t instance,
                         const char *topic,
                         const char *message,
                         const size_t message_length,
                         bool retain)
{
  if (instance)
    instance->publish(std::string(topic),
                      std::string(message, message_length),
                      message_length,
                      retain);
}

int mqtt_client_count_topics(mqtt_client_t instance, const char *prefix_pattern)
{
  if (instance) {
    return instance->count_topics(std::string(prefix_pattern));
  } else {
    return 0;
  }
}

void mqtt_client_unretain(mqtt_client_t instance, const char *prefix_pattern)
{
  if (instance)
    instance->unretain(std::string(prefix_pattern));
}

void mqtt_client_unretain_by_regex(mqtt_client_t instance,
                                   const char *prefix_pattern)
{
  if (instance) {
    instance->unretain_by_regex(std::string(prefix_pattern));
  }
}

void mqtt_client_subscribe(mqtt_client_t instance,
                           const char *topic,
                           message_callback_t callback)
{
  mqtt_client_subscribe_ex(instance,
                           topic,
                           (message_callback_ex_t)callback,
                           nullptr);
}

void mqtt_client_subscribe_ex(mqtt_client_t instance,
                              const char *topic,
                              message_callback_ex_t callback,
                              void *user)
{
  struct mqtt_client::callback_info info = {callback, user};

  if (instance) {
    instance->subscribe(std::string(topic), info);
  }
}

void mqtt_client_unsubscribe(mqtt_client_t instance,
                             const char *topic,
                             message_callback_t callback)
{
  message_callback_ex_t cb = (message_callback_ex_t)callback;
  mqtt_client_unsubscribe_ex(instance, topic, cb, nullptr);
}

void mqtt_client_unsubscribe_ex(mqtt_client_t instance,
                                const char *topic,
                                message_callback_ex_t callback,
                                void *user)
{
  struct mqtt_client::callback_info info = {callback, user};

  if (instance) {
    instance->unsubscribe(std::string(topic), info);
  }
}

void mqtt_client_on_connect_callback_set(
  mqtt_client_t instance,
  void (*on_connect)(mqtt_client_t inst, const int file_descriptor))
{
  if (instance)
    instance->on_connect_callback_set(on_connect);
}

void mqtt_client_on_disconnect_callback_set(
  mqtt_client_t instance,
  void (*on_disconnect)(mqtt_client_t inst, const int file_descriptor))
{
  if (instance)
    instance->on_disconnect_callback_set(on_disconnect);
}

void mqtt_client_before_disconnect_callback_set(mqtt_client_t instance,
                                                void (*before_disconnect)())
{
  if (instance)
    instance->before_disconnect_callback_set(before_disconnect);
}

void mqtt_client_after_connect_callback_set(mqtt_client_t instance,
                                            void (*after_connect)())
{
  if (instance)
    instance->after_connect_callback_set(after_connect);
}

// C interface for Mosquitto-callbacks
void mqtt_client_on_connect_callback(void *instance, void *obj, int return_code)
{
  ((mqtt_client_t)obj)->on_connect(return_code);
}

void mqtt_client_on_disconnect_callback(void *instance,
                                        void *obj,
                                        int return_code)
{
  ((mqtt_client_t)obj)->on_disconnect(return_code);
}

void mqtt_client_on_message_callback(void *instance,
                                     void *obj,
                                     const struct mqtt_message *message)
{
  ((mqtt_client_t)obj)
    ->on_message(
      std::string(message->topic),
      std::string((char *)message->payload, (size_t)message->payloadlen),
      (size_t)message->payloadlen,
      message->qos,
      message->retain);
}
