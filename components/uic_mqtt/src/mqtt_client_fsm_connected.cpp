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
// Unify components
#include "sl_log.h"

// This component
#include "mqtt_client_fsm.hpp"
#include "mqtt_client.hpp"
#include "mqtt_client.h"

#define LOG_TAG "mqtt_client_fsm_connected"

void mqtt_client_fsm_connected::enter(mqtt_client *client_instance)
{
  sl_log_debug(LOG_TAG, "FSM-Connected: Entering state.\n");

  // First, let's resubscribe to any previously subscribed topics (i.e. if this is a
  // reconnect).
  client_instance->resubscribe();

  // See if we have any queued up messages (subscription, publish, unsubscription)
  // that need to be sent.
  if (client_instance->subscription_messages_waiting()
      && (client_instance->event_counter(MQTT_EVENT_SUBSCRIBE, nullptr) == 0)) {
    client_instance->send_event(MQTT_EVENT_SUBSCRIBE, nullptr);
  }

  if (client_instance->publish_messages_waiting()
      && (client_instance->event_counter(MQTT_EVENT_PUBLISH, nullptr) == 0)) {
    client_instance->send_event(MQTT_EVENT_PUBLISH, nullptr);
  }

  if (client_instance->unsubscription_messages_waiting()
      && (client_instance->event_counter(MQTT_EVENT_UNSUBSCRIBE, nullptr)
          == 0)) {
    client_instance->send_event(MQTT_EVENT_UNSUBSCRIBE, nullptr);
  }
}

void mqtt_client_fsm_connected::exit(mqtt_client *client_instance)
{
  sl_log_debug(LOG_TAG, "FSM-Connected: Leaving state.\n");
}

void mqtt_client_fsm_connected::event(int incoming_event,
                                      mqtt_client *client_instance)
{
  switch (incoming_event) {
    case MQTT_EVENT_PUBLISH:
      client_instance->publish_to_broker();
      break;
    case MQTT_EVENT_SUBSCRIBE:
      client_instance->subscribe_to_topic();
      break;
    case MQTT_EVENT_UNSUBSCRIBE:
      client_instance->unsubscribe_from_topic();
      break;
    case MQTT_TRANSITION_DISCONNECTED:
      client_instance->transition(mqtt_client_fsm_disconnected::get_instance());
      break;
    default:
      sl_log_warning(LOG_TAG, "FSM-Connected: Unhandled event: 0x%x\n", incoming_event);
      break;
  }
}

mqtt_client_fsm &mqtt_client_fsm_connected::get_instance()
{
  static mqtt_client_fsm_connected singleton;
  return singleton;
}
