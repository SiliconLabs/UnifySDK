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
#include <algorithm>
#include <chrono>

// UIC components
#include "sl_log.h"

// This component
#include "mqtt_client_fsm.hpp"
#include "mqtt_client.hpp"
#include "mqtt_client.h"

#define LOG_TAG "mqtt_client_fsm_connecting"

void mqtt_client_fsm_connecting::enter(mqtt_client *client_instance)
{
  sl_log_debug(LOG_TAG, "Entering state: FSM-Connecting.\n");
  if (client_instance->connect() != SL_STATUS_OK) {
    // Connection wasn't successful so we'll need to try again.
    // We start by re-entering the DISCONNECTED state
    client_instance->send_event(MQTT_TRANSITION_DISCONNECTED, nullptr);
    // Then we send a delayed MQTT_EVENT_CONNECT event, where the delay
    // is the next increment of our back-off time.
    client_instance->send_delayed_event(
      MQTT_EVENT_CONNECT,
      client_instance->next_reconnect_backoff);

    std::chrono::milliseconds next_potential_backoff
      = client_instance->next_reconnect_backoff * 2;

    // Take the lower (min) of our two potential back-off times, in case the
    // incremental one has gotten higher than the maximum back-off, we'll use
    // the maximum back-off.
    client_instance->next_reconnect_backoff
      = std::min(next_potential_backoff,
                 client_instance->max_reconnect_backoff);
  } else {
    // Reset our back-off time.
    client_instance->next_reconnect_backoff = std::chrono::milliseconds(
      MQTT_CLIENT_MINIMUM_RECONNECT_BACKOFF_MILLISECONDS);
  }
}

void mqtt_client_fsm_connecting::exit(mqtt_client *client_instance)
{
  sl_log_debug(LOG_TAG, "Leaving state: FSM-Connecting.\n");
}

void mqtt_client_fsm_connecting::event(int incoming_event,
                                       mqtt_client *client_instance)
{
  switch (incoming_event) {
    case MQTT_EVENT_PUBLISH:
      // Ignore event.
      break;
    case MQTT_EVENT_SUBSCRIBE:
      // Ignore event.
      break;
    case MQTT_EVENT_UNSUBSCRIBE:
      // Ignore event.
      break;
    case MQTT_TRANSITION_DISCONNECTED:
      client_instance->transition(mqtt_client_fsm_disconnected::get_instance());
      break;
    case MQTT_TRANSITION_CONNECTED:
      client_instance->transition(mqtt_client_fsm_connected::get_instance());
      sl_log_info(LOG_TAG, "We are connected to the MQTT broker.");
      break;
    default:
      sl_log_warning(LOG_TAG,
                     "FSM-Connecting: Unhandled event: 0x%x\n",
                     incoming_event);
      break;
  }
}

mqtt_client_fsm &mqtt_client_fsm_connecting::get_instance()
{
  static mqtt_client_fsm_connecting singleton;
  return singleton;
}
