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
// UIC components
#include "sl_log.h"

// This component
#include "mqtt_client_fsm.hpp"
#include "mqtt_client.hpp"
#include "mqtt_client.h"

#define LOG_TAG "mqtt_client_fsm_disconnected"

// Disconnected
void mqtt_client_fsm_disconnected::enter(mqtt_client *client_instance)
{
  sl_log_debug(LOG_TAG, "FSM-Disconnected: Entering state.\n");
}

void mqtt_client_fsm_disconnected::exit(mqtt_client *client_instance)
{
  sl_log_debug(LOG_TAG, "FSM-Disconnected: Leaving state.\n");
}

void mqtt_client_fsm_disconnected::event(int incoming_event,
                                         mqtt_client *client_instance)
{
  switch (incoming_event) {
    case MQTT_EVENT_CONNECT:
      client_instance->send_event(MQTT_TRANSITION_CONNECTING, nullptr);
      break;
    case MQTT_TRANSITION_CONNECTING:
      client_instance->transition(mqtt_client_fsm_connecting::get_instance());
      break;
    case MQTT_EVENT_PUBLISH:
      // Ignore event.
      break;
    case MQTT_EVENT_SUBSCRIBE:
      // Ignore event.
      break;
    case MQTT_EVENT_UNSUBSCRIBE:
      // Ignore event.
      break;
    default:
      sl_log_critical(LOG_TAG, "FSM-Disconnected: Unhandled event: 0x%x\n",
                      incoming_event);
      break;
  }
}

mqtt_client_fsm &mqtt_client_fsm_disconnected::get_instance()
{
  static mqtt_client_fsm_disconnected singleton;
  return singleton;
}
