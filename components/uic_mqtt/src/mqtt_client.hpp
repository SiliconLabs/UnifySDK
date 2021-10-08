/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#ifndef MQTT_CLIENT_HPP
#define MQTT_CLIENT_HPP

// Standard-library
#include <functional>
#include <string>
#include <queue>
#include <map>
#include <set>
#include <vector>
#include <chrono>

// UIC components
#include "sl_status.h"

// This component
#include "mqtt_client.h"
#include "mqtt_client_fsm.hpp"

// Some type-definitions to help with internal consistency
typedef struct {
  std::string topic;
  std::string message;
  size_t message_length;
  bool retain;
} message_queue_element_t;

typedef void (*message_callback_t)(const char *topic,
                                   const char *message,
                                   const size_t message_length);

typedef std::function<void(mqtt_client_t instance, const int file_descriptor)>
  connection_callback_t;

struct mqtt_client {
  public:
  mqtt_client(
    const std::string &hostname,
    const uint32_t port,
    const std::string &client_id,
    const std::string &cafile,
    const std::string &certfile,
    const std::string &keyfile,
    std::function<int(const int, void *)> event_sender,
    std::function<void(const int, const unsigned long)> delayed_event_sender,
    std::function<int(const int, void *)> event_counter);

  // Process (lifetime) management functions: Initialization, tear-down, and
  // event-functionality.
  sl_status_t setup();
  sl_status_t teardown();
  int file_descriptor();
  void poll();
  void disconnect();  // Needs to be public so graceful disconnect can take
                      // place, even after the event-queue has be torn down.
  void event(const int incoming_event);
  std::function<int(const int, void *)>
    event_sender;  // Contiki: process_post(...)
  std::function<void(const int, const unsigned long)> delayed_event_sender;
  std::function<int(const int, void *)>
    event_counter;  // Contiki: process_count_events(...)

  // Publish and subscribe functions. This is what the uic_mqtt_* functions call.
  void publish(const std::string &topic,
               const std::string &message,
               size_t message_length,
               bool retain);

  /**
   * @brief Un-retain previously retained messages
   *
   * Any topic which has the prefix will be un-retained
   * @param prefix_pattern
   */
  void unretain(const std::string &prefix_pattern);

  /**
   * @brief Un-retain previously retained messages
   *
   * Any topic matches the regex pattern
   * @param regex
   */
  void unretain_by_regex(const std::string &regex);

  /**
   * @brief Count the number of topics published
   * 
   * This function counts the number of topics which has been published
   * retained matching a pattern prefix.
   * @param prefix_pattern 
   * @return int 
   */
  int count_topics(const std::string &prefix_pattern);

  void subscribe(const std::string &topic, message_callback_t callback);
  void resubscribe();
  void unsubscribe(const std::string &topic, message_callback_t callback);

  // Callbacks to the outside world
  void on_connect_callback_set(connection_callback_t);
  void on_disconnect_callback_set(connection_callback_t);

  // The MQTT callbacks. Those are called (through wrappers) from
  // the Mosquitto library.
  void on_connect(int return_code);
  void on_disconnect(int return_code);
  void on_message(const std::string &topic,
                  const std::string &message,
                  size_t message_length,
                  int message_qos,
                  bool message_retain);

  private:
  const std::string hostname;
  const uint32_t port;
  int socket_file_descriptor;
  std::chrono::milliseconds next_reconnect_backoff;
  std::chrono::milliseconds max_reconnect_backoff;
  connection_callback_t on_connect_callback    = nullptr;
  connection_callback_t on_disconnect_callback = nullptr;

  // Message-queueing and routing: Containers and arbitrators. The arbitrators are
  // what allows for an asynchronous implementation of the client.
  std::deque<message_queue_element_t> publishing_queue;
  std::queue<std::string> subscription_queue;
  std::queue<std::string> unsubscription_queue;
  std::map<std::string, std::vector<message_callback_t>> subscription_callbacks;
  std::set<std::string> retained_topics;

  bool publish_messages_waiting();
  sl_status_t publish_to_broker(bool flushing = false);
  bool subscription_messages_waiting();
  sl_status_t subscribe_to_topic();
  bool unsubscription_messages_waiting();
  sl_status_t unsubscribe_from_topic();

  // MQTT-client-library fields. Those need to be client-library agnostic but
  // were, at the time of writing, based on the Mosquitto architecture.
  void *mqtt_client_library_instance;
  std::string mqtt_client_id;
  std::string mqtt_cafile;
  std::string mqtt_certfile;
  std::string mqtt_keyfile;

  // FSM functionality
  friend class mqtt_client_fsm_disconnected;
  friend class mqtt_client_fsm_connecting;
  friend class mqtt_client_fsm_connected;
  mqtt_client_fsm *current_state;
  void send_event(const int outgoing_event, void *procdata);
  void send_delayed_event(const int outgoing_event,
                          std::chrono::milliseconds delay_time);
  void transition(mqtt_client_fsm &new_state);
  sl_status_t connect();
};

#endif  // MQTT_CLIENT_HPP
