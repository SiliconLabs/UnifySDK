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

/**
 * @file ucl_node_state.h
 * @ingroup ucl_mqtt
 * @brief Handles publishing node state to `ucl/by-unid/<node>/State` topic
 *
 * This component handles publishing the `ucl/by-unid/<node>/State` topic in MQTT
 * It register callbacks in the \ref attribute_store for NETWORK_STATUS attribute
 * and acts on these callbacks, when the state of the node changes, and publish
 * the changes to MQTT.
 *
 * @startuml UCL Node State design
    box "MQTT Broker" #LightYellow
    participant MQTT
    end box

    box "ZPC" #LightBlue
    participant StateTopic
    participant AttributeStore
    end box
    note over StateTopic,AttributeStore: Init
    StateTopic          -> AttributeStore       : attribute_store_register_callback_by_type(att_store_node_cb, ATTRIBUTE_NODE_ID)
    StateTopic          -> AttributeStore       : attribute_store_register_callback_by_type_and_state(att_store_network_status_cb, ATTRIBUTE_NETWORK_STATUS, REPORTED_ATTRIBUTE)
    ... ...
    note over StateTopic,AttributeStore: NETWORK_STATUS updated (NODE_STATE_TOPIC_STATE_INCLUDED)
    AttributeStore      -> StateTopic           : att_store_network_status_cb(NETWORK_STATUS, ATTRIBUTE_UPDATED)
    StateTopic          -> MQTT                 : <i>ucl/by-unid/<new-node>/State</i>\n{"NetworkState": "Included", ...}
    ... ...
    note over StateTopic,AttributeStore: Remove Node
    AttributeStore      -> StateTopic           : att_store_node_cb(node_id, ATTRIBUTE_DELETED)
    StateTopic          -> MQTT                 : delete <i>ucl/by-unid/<new-node></i>
  @enduml
 *
 * @{
 */

#ifndef UCL_NODE_STATE_H
#define UCL_NODE_STATE_H

#include "process.h"
#include "sl_status.h"

/// This represents with an unkonwn delay before it can be reached
#define MAX_COMMAND_DELAY_UNKNOWN 0xFFFFFFFF
/// This represents with a possibly infinite delay before it can be reached
/// (wakes up only on manual activations)
#define MAX_COMMAND_DELAY_INFINITE 0xFFFFFFFE

/**
 * @brief Possible Security values for a node in the State topic.
 */
typedef enum {
  /// Non-secure
  ///
  NODES_STATE_TOPIC_SECURITY_NONE,
  /// Z-Wave S2 Unauthenticated
  ///
  NODES_STATE_TOPIC_SECURITY_S2_UNAUTHENTICATED,
  /// Z-Wave S2 Authenticated
  ///
  NODES_STATE_TOPIC_SECURITY_S2_AUTHENTICATED,
  /// Z-Wave S2 Access
  ///
  NODES_STATE_TOPIC_SECURITY_S2_ACCESS,
  /// Z-Wave S0
  ///
  NODES_STATE_TOPIC_SECURITY_S0,
  /// This element must always be last
  ///
  NODES_STATE_TOPIC_SECURITY_LAST
} node_state_topic_security_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Name the of @ref contiki process for the Ucl Node State.
 *
 * This is used to register the name of the Ucl Node State Process.
 */
PROCESS_NAME(ucl_node_state_process);

#ifdef __cplusplus
}
#endif

#endif  //UCL_NODE_STATE_H
/** @} end ucl_node_state */
