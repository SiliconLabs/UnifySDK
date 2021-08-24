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

#ifndef ZWAVE_COMMAND_CLASS_ASSOCIATION_H
#define ZWAVE_COMMAND_CLASS_ASSOCIATION_H

#include "sl_status.h"
#include "zwave_controller_connection_info.h"
#include "zwave_rx.h"
#include "attribute_store.h"

///> Association Specific Group Report frame.
typedef struct association_specific_report_frame {
  uint8_t command_class;
  uint8_t command;
  uint8_t group;
} association_specific_report_frame_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup association_command_class Association Command Class
 * @ingroup command_classes
 * @brief Association Command Class handlers and control functions
 *
 * This module takes care of Association Command Class support
 * and control
 *
 * It uses a subscription to ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_VERSION
 * in the @ref attribute_store to find out about nodes that support the
 * Association Command Class.
 *
 * When a new version attribute is added, the component create
 * ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_SUPPORTED_GROUPINGS and register
 * rules to the attribute resolver to initiate a node interview by sending
 * an Association Supported Get Command() to the corresponding nodeID/endpoint.
 * The attribute store gets populated as it will receive replies from the node
 * supporting Association.
 *
 * @ref multi_channel_association_command_class,
 * @ref agi_command_class
 * and @ref association_command_class
 * share the same attribute subtree.
 *
@startuml{attribute_store_association_command_class.png} "Association data model" width=10cm
title Association data model
allow_mixing
skinparam objectBorderColor black

legend top
<font color=#FEFEFE>ATTRIBUTE(type)</font> : ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_type
endlegend

package "Attribute Store" <<Database>> {
  object "NodeID" as node #f2ffe6
  node : Attribute Type = ATTRIBUTE_NODE_ID
  node : value = Desired: [], Reported: [03]

  object "Endpoint Attribute" as endpoint #e6fff7
  endpoint : Attribute Type = ATTRIBUTE_ENDPOINT_ID
  endpoint : value = Desired: [] - Reported: [04]

  object "Supported Groupings" as group_number #FFFFFF
  group_number : Attribute Type = ATTRIBUTE(SUPPORTED_GROUPINGS)
  group_number : value = Desired: [] - Reported: [1]

  package "For each Association Group" <<Rectangle>> {
    object "Group ID" as group_id #FFFFFF
    group_id : ID = ATTRIBUTE(GROUP_ID)
    group_id : value = Desired: [], Reported: [Group X]

    object "Group Name" as group_name #FFFFFF
    group_name : ID = ATTRIBUTE(GROUP_NAME)
    group_name : value = Desired: [], Reported: ["Group Name"]

    object "Group Profile" as group_profile #FFFFFF
    group_profile : ID = ATTRIBUTE(GROUP_PROFILE)
    group_profile : value = Desired: [], Reported: [0x0001]

    object "Group Command List" as group_command_list #FFFFFF
    group_command_list : ID = ATTRIBUTE(GROUP_COMMAND_LIST)
    group_command_list : value = Desired: [], Reported: [0x5A 0x01 0x25 0x03]

    object "Group Content" as group_content #FFFFFF
    group_content : ID = ATTRIBUTE(GROUP_CONTENT)
    group_content : value = Desired: [03 00 01 00], Reported: [02]

    object "Group Capacity" as group_capacity #FFFFFF
    group_capacity : ID = ATTRIBUTE(MAX_NODES_SUPPORTED)
    group_capacity : value = Desired: [], Reported: [05]
  }
}

node *-- endpoint
endpoint *-- group_number
endpoint *-- group_id
group_id *-- group_name
group_id *-- group_profile
group_id *-- group_command_list
group_id *-- group_content
group_id *-- group_capacity

group_command_list -[hidden]--> group_content
group_profile -[hidden]--> group_capacity

@enduml
 *
 *
 * @{
 */

/**
 * @brief Setup fixture for @ref association_command_class
 *
 * This setup function will register callbacks to the Attribute Store, so that
 * it can be notified when to issue commands to the Z-Wave nodes.
 *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error occurred
 */
sl_status_t zwave_command_class_association_init(void);

/**
 * @brief Resolves the ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_SUPPORTED_GROUPINGS
 * for a given node
 *
 * @param node               The Attribute Store node to resolve (get)
 * @param [out] frame        Pointer to copy the frame data that will allow resolution
 * @param [out] frame_length Pointer to write how many bytes were copied at the
 *                           frame pointer
 *
 * @returns status codes according to @ref attribute_resolver_function_t
 */
sl_status_t zwave_command_class_association_groupings_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length);

/**
 * @brief Sets the ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_CONTENT
 * for a given node
 *
 * @param node               The Attribute Store node to  resolve (set)
 * @param [out] frame        Pointer to copy the frame data that will allow resolution
 * @param [out] frame_length Pointer to write how many bytes were copied at the
 *                           frame pointer
 *
 * @returns status codes according to @ref attribute_resolver_function_t
 */
sl_status_t zwave_command_class_association_set(attribute_store_node_t node,
                                                uint8_t *frame,
                                                uint16_t *frame_length);

/**
 * @brief Gets the ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_CONTENT
 * for a given node
 *
 * @param node               The Attribute Store node to  resolve (get)
 * @param [out] frame        Pointer to copy the frame data that will allow resolution
 * @param [out] frame_length Pointer to write how many bytes were copied at the
 *                           frame pointer
 *
 * @returns status codes according to @ref attribute_resolver_function_t
 */
sl_status_t zwave_command_class_association_get(attribute_store_node_t node,
                                                uint8_t *frame,
                                                uint16_t *frame_length);

#ifdef __cplusplus
}
#endif

/** @} end association_command_class */

#endif  //ZWAVE_COMMAND_CLASS_ASSOCIATION_H
