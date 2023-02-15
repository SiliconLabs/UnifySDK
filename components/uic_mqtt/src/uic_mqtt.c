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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// Contiki
#include "sys/etimer.h"
#include "sys/process.h"

// Unify components
#include "sl_log.h"
#include "config.h"
#include "uic_main_externals.h"

// This component
#include "uic_mqtt.h"
#include "uic_mqtt_int.h"
#include "mqtt_client.h"
#include "mqtt_client_id.h"

#define LOG_TAG "uic_mqtt"

PROCESS(uic_mqtt_process, "Unify MQTT Client");

// An instance of our C++ struct/class. We need to pass this to all of
// its wrapper-functions.
mqtt_client_t client_instance = NULL;

// Timer for making sure we poll the client regularly for keepalive, etc.
static struct etimer uic_mqtt_loop_timer;
static struct etimer uic_mqtt_post_delay_timer;
static bool uic_mqtt_post_delay_timer_flag = false;
static int delayed_event                   = 0;

// A wrapper for the Contiki's process_post. This is done so the mqtt_client
// doesn't need to know about the uic_mqtt_process or any other Contiki-specifics.
int uic_mqtt_process_post(const int event, void *procdata)
{
  return process_post(&uic_mqtt_process, (process_event_t)event, procdata);
}

void uic_mqtt_process_post_delayed(const int event, unsigned long milliseconds)
{
  if (!uic_mqtt_post_delay_timer_flag) {
    delayed_event = event;
    sl_log_debug(LOG_TAG,
                 "Setting delayed_event timer: %ld ms, event: 0x%02x\n",
                 milliseconds,
                 event);
    uic_mqtt_post_delay_timer_flag = true;
    etimer_set(&uic_mqtt_post_delay_timer, milliseconds);
  } else {
    sl_log_debug(LOG_TAG,
                 "delayed_event timer already set. Dropping event: 0x%02x\n",
                 event);
  }
}

int uic_mqtt_process_count_events(const int event, void *procdata)
{
  return process_count_events(&uic_mqtt_process,
                              (process_event_t)event,
                              procdata);
}

void uic_mqtt_process_on_connect(mqtt_client_t instance,
                                 const int file_descriptor)
{
  if (uic_main_ext_register_rfd(file_descriptor, NULL, &uic_mqtt_process)
      == SL_STATUS_OK) {
    sl_log_debug(LOG_TAG, "Registered file-descriptor in Contiki.\n");
  } else {
    sl_log_critical(LOG_TAG,
                    "Could not register file-descriptor in Contiki.\n");
  }
}

void uic_mqtt_process_on_disconnect(mqtt_client_t instance,
                                    const int file_descriptor)
{
  if (uic_main_ext_unregister_rfd(file_descriptor) == SL_STATUS_OK) {
    sl_log_debug(LOG_TAG, "Unregistered file-descriptor.\n");
  } else {
    sl_log_warning(LOG_TAG, "Unable to unregister file-descriptor.\n");
  }
}

// Implementation of the public API.
sl_status_t uic_mqtt_setup()
{
  const char *mqtt_host;
  const char *cafile;
  const char *certfile;
  const char *keyfile;

  int mqtt_port;
  if (CONFIG_STATUS_OK
      != config_get_as_string(CONFIG_KEY_MQTT_HOST, &mqtt_host)) {
    return SL_STATUS_FAIL;
  }
  if (CONFIG_STATUS_OK != config_get_as_int(CONFIG_KEY_MQTT_PORT, &mqtt_port)) {
    return SL_STATUS_FAIL;
  }
  config_get_as_string(CONFIG_KEY_MQTT_CAFILE, &cafile);
  config_get_as_string(CONFIG_KEY_MQTT_CERTFILE, &certfile);
  config_get_as_string(CONFIG_KEY_MQTT_KEYFILE, &keyfile);

  client_instance = mqtt_client_new(mqtt_host,
                                    mqtt_port,
                                    mqtt_client_id_get_from_config(),
                                    cafile,
                                    certfile,
                                    keyfile,
                                    uic_mqtt_process_post,
                                    uic_mqtt_process_post_delayed,
                                    uic_mqtt_process_count_events);

  // Register the callbacks we need from mqtt_client
  mqtt_client_on_connect_callback_set(client_instance,
                                      uic_mqtt_process_on_connect);
  mqtt_client_on_disconnect_callback_set(client_instance,
                                         uic_mqtt_process_on_disconnect);

  process_start(&uic_mqtt_process, NULL);
  return mqtt_client_setup(client_instance);
}

int uic_mqtt_teardown()
{
  int retval = mqtt_client_teardown(client_instance);
  mqtt_client_delete(client_instance);
  client_instance = NULL;
  return retval;
}

void uic_mqtt_subscribe(const char *topic, mqtt_message_callback_t callBack)
{
  mqtt_client_subscribe(client_instance, topic, callBack);
}

void uic_mqtt_subscribe_ex(const char *topic,
                           mqtt_message_callback_ex_t callBack,
                           void *user)
{
  mqtt_client_subscribe_ex(client_instance, topic, callBack, user);
}

void uic_mqtt_publish(const char *topic,
                      const char *message,
                      const size_t message_length,
                      bool retain)
{
  mqtt_client_publish(client_instance, topic, message, message_length, retain);
}

int uic_mqtt_count_topics(const char *prefix_pattern)
{
  return mqtt_client_count_topics(client_instance, prefix_pattern);
}

void uic_mqtt_unretain(const char *prefix)
{
  sl_log_debug(LOG_TAG,
               "Removing topic (retained empty MQTT publish message) : %s/#",
               prefix);
  mqtt_client_unretain(client_instance, prefix);
}

void uic_mqtt_unretain_by_regex(const char *regex)
{
  sl_log_debug(LOG_TAG,
               "Removing all topics (retained empty MQTT publish message) "
               "matching regex : %s",
               regex);
  mqtt_client_unretain_by_regex(client_instance, regex);
}

void uic_mqtt_unsubscribe(const char *topic, mqtt_message_callback_t callback)
{
  mqtt_client_unsubscribe(client_instance, topic, callback);
}

void uic_mqtt_unsubscribe_ex(const char *topic,
                             mqtt_message_callback_ex_t callback,
                             void *user)
{
  mqtt_client_unsubscribe_ex(client_instance, topic, callback, user);
}

void uic_mqtt_set_before_disconnect_callback(
  mqtt_connection_callbacks_t before_disconnect)
{
  mqtt_client_before_disconnect_callback_set(client_instance,
                                             before_disconnect);
}

void uic_mqtt_set_after_connect_callback(
  mqtt_connection_callbacks_t after_connect)
{
  mqtt_client_after_connect_callback_set(client_instance, after_connect);
}

const char *uic_mqtt_get_client_id()
{
  return mqtt_client_get_client_id(client_instance);
}

bool uic_mqtt_is_connected_to_broker()
{
  return mqtt_client_is_connected_to_broker(client_instance);
}

// Contiki process implementation
PROCESS_THREAD(uic_mqtt_process, ev, data)
{
  PROCESS_POLLHANDLER(mqtt_client_poll(client_instance));

  PROCESS_BEGIN()

  while (1) {
    if (ev == PROCESS_EVENT_INIT) {
      process_post(&uic_mqtt_process, MQTT_EVENT_CONNECT, NULL);
      etimer_set(&uic_mqtt_loop_timer, MQTT_CLIENT_POLL_TIMER_MILLISECONDS);

    } else if (ev == PROCESS_EVENT_EXIT) {
      // The event-loop has been shut down a this point, so we need to disconnect
      // and clean-up directly.
      mqtt_client_disconnect(client_instance);

    } else if (ev == PROCESS_EVENT_EXITED) {
      // Ignore EXITED-events from other processes. We dont depend on them.

    } else if (ev == PROCESS_EVENT_TIMER) {
      if ((struct etimer *)data == &uic_mqtt_loop_timer) {
        etimer_restart(&uic_mqtt_loop_timer);
        mqtt_client_poll(client_instance);
      } else if ((struct etimer *)data == &uic_mqtt_post_delay_timer) {
        sl_log_debug(LOG_TAG,
                     "process_posting delayed event: %d\n",
                     delayed_event);
        uic_mqtt_post_delay_timer_flag = false;
        process_post(&uic_mqtt_process, delayed_event, NULL);
      }

    } else if (ev == PROCESS_EVENT_POLL) {
      // Just capturing the event so the MQTT-client doesn't have to concern itself
      // with it.

    } else {
      // All other events are passed on to the mqtt_client.
      mqtt_client_event(client_instance, ev);
    }
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}
