

/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup unify_dotdot_attribute_store_node_state UCL Node State
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
    StateTopic          -> AttributeStore       : attribute_store_register_callback_by_type_and_state(att_store_network_status_cb, DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS, REPORTED_ATTRIBUTE)
    ... ...
    note over StateTopic,AttributeStore: NETWORK_STATUS updated (ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL)
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

#ifndef UNIFY_DOTDOT_ATTRIBUTE_STORE_NODE_STATE_H
#define UNIFY_DOTDOT_ATTRIBUTE_STORE_NODE_STATE_H

#include "uic_enum.h"
#include "attribute_store.h"
#include "zap-types.h"

#ifdef __cplusplus
extern "C" {
#endif

/// This represents with an unknown delay before it can be reached
#define MAX_COMMAND_DELAY_UNKNOWN 0xFFFFFFFF
/// This represents with a possibly infinite delay before it can be reached
/// (wakes up only on manual activations)
#define MAX_COMMAND_DELAY_INFINITE 0xFFFFFFFE

/**
 * @brief Reads the UCL network status of a node, starting from any attribute
 * under the NodeID in the attribute store.
 *
 * @param node     Attribute store node under the NodeID for which we want
 * to read the network status.
 */
NodeStateNetworkStatus unify_attribute_store_node_state_get_status(
  attribute_store_node_t node);


sl_status_t unify_dotdot_attribute_store_node_state_init();

#ifdef __cplusplus
}
#endif

#endif  //UNIFY_DOTDOT_ATTRIBUTE_STORE_NODE_STATE_H
/** @} end unify_dotdot_attribute_store_node_state */


