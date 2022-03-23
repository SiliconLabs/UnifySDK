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

// Generic includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Unify components
#include "uic_mqtt.h"
#include "ucl_mqtt_helper.h"
#include "dotdot_mqtt.h"

#define STATE_TOPIC_TEMPLATE "ucl/by-unid/%s/State"

// Template for a node state payload.
#define NODE_STATE_TEMPLATE        \
  "{\n"                            \
  "  \"NetworkStatus\": \"%s\",\n" \
  "  \"Security\": \"None\",\n"    \
  "  \"MaximumCommandDelay\": 0\n" \
  "}"

// Values used for an asset Tag State payload.
#define TAG_STATE                                 \
  "{\n"                                           \
  "  \"NetworkStatus\": \"Online functional\",\n" \
  "  \"Security\": \"None\",\n"                   \
  "  \"MaximumCommandDelay\": \"unknown\"\n"      \
  "}"

#define AOX_POSITION_ESTIMATION_SUPPORTED_COMMANDS_TOPIC_TEMPLATE \
  "ucl/by-unid/%s/ep0/AoXPositionEstimation/SupportedCommands"
#define STATE_SUPPORTED_COMMANDS_TOPIC_TEMPLATE \
  "ucl/by-unid/%s/State/SupportedCommands"
#define STATE_BASE_TOPIC_TEMPLATE "ucl/by-unid/%s"
#define SUPPORTED_COMMANDS        "{\"value\":[]}"

// List of endpoints (just endpoint 0) for AoXPC / AoX Asset tags.
static const uint8_t endpoint_list[] = {0};

void publish_node_state(char *unid, bool available)
{
  char topic[100];
  char message[200];
  char *status = available ? "Online functional" : "Unavailable";

  if (unid == NULL) {
    return;
  }

  // Publish network state.
  snprintf(topic, sizeof(topic), STATE_TOPIC_TEMPLATE, unid);
  snprintf(message, sizeof(message), NODE_STATE_TEMPLATE, status);
  uic_mqtt_publish(topic, message, strlen(message), true);

  // Publish the endpointIdList if the node is available
  if (true == available) {
    char base_topic[100];
    snprintf(base_topic, sizeof(base_topic), STATE_BASE_TOPIC_TEMPLATE, unid);
    uic_mqtt_dotdot_state_endpoint_id_list_publish(base_topic,
                                                   sizeof(endpoint_list),
                                                   endpoint_list,
                                                   UCL_MQTT_PUBLISH_TYPE_ALL);
  }
}

void publish_tag_state(char *unid)
{
  char topic[100];

  if (unid == NULL) {
    return;
  }

  // Publish network state.
  snprintf(topic, sizeof(topic), STATE_TOPIC_TEMPLATE, unid);
  uic_mqtt_publish(topic, TAG_STATE, strlen(TAG_STATE), true);

  // Publish State SupportedCommands
  snprintf(topic, sizeof(topic), STATE_SUPPORTED_COMMANDS_TOPIC_TEMPLATE, unid);
  uic_mqtt_publish(topic, SUPPORTED_COMMANDS, strlen(SUPPORTED_COMMANDS), true);

  // Publish the endpointIdList
  char base_topic[100];
  snprintf(base_topic, sizeof(base_topic), STATE_BASE_TOPIC_TEMPLATE, unid);
  uic_mqtt_dotdot_state_endpoint_id_list_publish(base_topic,
                                                 sizeof(endpoint_list),
                                                 endpoint_list,
                                                 UCL_MQTT_PUBLISH_TYPE_ALL);

  // Publish AoXPositionEstimation SupportedCommands.
  snprintf(topic,
           sizeof(topic),
           AOX_POSITION_ESTIMATION_SUPPORTED_COMMANDS_TOPIC_TEMPLATE,
           unid);
  uic_mqtt_publish(topic, SUPPORTED_COMMANDS, strlen(SUPPORTED_COMMANDS), true);
}
