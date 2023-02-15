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
 * @defgroup multi_channel_command_class Multi Channel Command Class
 * @ingroup command_classes
 * @brief Multi Channel Command Class implementation
 *
 * This module implements the Multi Channel Command Class handler (control).
 * The Multi Channel Transport is defined in \ref zwave_transports
 *
 * The control of Multi Channel relies on the following Attribute Store system:
@startuml{multi_channel_attribute_store_tree.png} "Multi Channel Attributes" width=10cm
title Attribute Store Multi Channel Command Class attributes
allow_mixing
skinparam objectBorderColor black

object HomeID #fffae6
HomeID : ID = ATTRIBUTE_HOME_ID
HomeID : value = Desired: [], Reported: [8C CE FB E6]

object "NodeID" as NodeID_1 #f2ffe6
NodeID_1 : ID = ATTRIBUTE_NODE_ID
NodeID_1 : value = Desired: [], Reported: [01]

object "Endpoint 0" as endpoint_0 #e6fff7
endpoint_0 : ID = ATTRIBUTE_ENDPOINT_ID
endpoint_0 : value = Desired: [], Reported: [00]

object "Attribute" as attribute_1 #FFFFFF
attribute_1 : ID = MULTI_CHANNEL_VERSION
attribute_1 : value = Desired: [], Reported: [04]

object "Attribute" as attribute_2 #FFFFFF
attribute_2 : ID = MULTI_CHANNEL_IDENTICAL_ENDPOINTS
attribute_2 : value = Desired: [], Reported: [00]

object "Attribute" as attribute_3 #FFFFFF
attribute_3 : ID = MULTI_CHANNEL_INDIVIDUAL_ENDPOINTS
attribute_3 : value = Desired: [], Reported: [03]

object "Attribute" as attribute_4 #FFFFFF
attribute_4 : ID = MULTI_CHANNEL_AGGREGATED_ENDPOINTS
attribute_4 : value = Desired: [], Reported: [02]

object "Attribute" as attribute_5 #FFFFFF
attribute_5 : ID = MULTI_CHANNEL_ALL_INDIVIDUAL_ENDPOINTS_FOUND
attribute_5 : value = Desired: [], Reported: [01]


package "For each Individual endpoint" <<Rectangle>> {
  object "Endpoint X" as endpoint_x #e6fff7
  endpoint_x : ID = ATTRIBUTE_ENDPOINT_ID
  endpoint_x : value = Desired: [], Reported: [X]
}

package "For each Aggregated endpoint" <<Rectangle>> {
  object "Endpoint Y" as endpoint_y #e6fff7
  endpoint_y : ID = ATTRIBUTE_ENDPOINT_ID
  endpoint_y : value = Desired: [], Reported: [Y]

  object "Attribute" as attribute_6 #FFFFFF
  attribute_6 : ID = MULTI_CHANNEL_AGGREGATED_MEMBERS
  attribute_6 : value = Desired: [], Reported: [X1 X2 X3]
}

HomeID *-- NodeID_1
NodeID_1 *-- endpoint_0
NodeID_1 *-- endpoint_x
NodeID_1 *-- endpoint_y
endpoint_0 *-- attribute_1
endpoint_0 *-- attribute_2
endpoint_0 *-- attribute_3
endpoint_0 *-- attribute_4
endpoint_0 *-- attribute_5
endpoint_y *-- attribute_6

attribute_1 -[hidden]--> attribute_3
attribute_2 -[hidden]--> attribute_4
attribute_3 -[hidden]--> attribute_5
attribute_4 -[hidden]--> attribute_6

@enduml
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_MULTI_CHANNEL_H
#define ZWAVE_COMMAND_CLASS_MULTI_CHANNEL_H

#include "sl_status.h"
#include "zwave_controller_connection_info.h"
#include "attribute_store.h"
#include "zwave_rx_definitions.h"

// Other components (like the transport) will use this define
// to keep the version aligned.
#define MULTI_CHANNEL_VERSION MULTI_CHANNEL_VERSION_V4

// Frame parsing define
#define ENDPOINT_MASK 0x7F

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Multi Channel Command Class (handler)
 * *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error occurred
 */
sl_status_t zwave_command_class_multi_channel_init(void);

/**
 * @brief Resolves a Non-secure NIF for an Endpoint ID > 0.
 *
 * Refer to @ref attribute_resolver_function_t for details.
 *
 * @param node                The attribute store node that is being resolved
 * @param [out] frame         Pointer where to write the frame data to resolve
 *                            the attribute
 * @param [out] frame_length  Pointer where to write length of the frame data
 *                            that has been writen to the frame pointer.
 *
 * @returns SL_STATUS_OK if the frame was written
 */
sl_status_t zwave_command_class_multi_channel_capability_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length);

/**
 * @brief Checks if an endpoints is aggregated
 *
 * It verifies if an attribute of type
 * ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_MEMBERS exists under the
 * endpoint.
 *
 * @param endpoint_node       The attribute store node that represents the
 *                            End Point ID
 * @returns false for individual endpoints and true for aggregated endpoints
 */
bool is_endpoint_aggregated(attribute_store_node_t endpoint_node);

/**
 * @brief Handles incoming Multi Channel encapsulated commands
 *
 * @param connection_info Info about the connection properties of this frame.
 * @param rx_options Info about the transport properties of this frame.
 * @param frame_data The data payload of this frame.
 * @param frame_length The length of this frame.
 * @returns sl_status_t representing the outcome of receiving the frame
 *          In case where the command is controlled, it should be set to SL_STATUS_OK,
 *          even when encountering some parsing errors
 *          In case where the command is supported, it should be set to the @ref sl_status_t
 *          corresponding to the correct time Status code. Refer to @ref zwave_command_handler_t
 */
sl_status_t zwave_command_class_multi_channel_handler(
  const zwave_controller_connection_info_t *connection_info,
  const zwave_rx_receive_options_t *rx_options,
  const uint8_t *frame_data,
  uint16_t frame_length);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_MULTI_CHANNEL_H
/** @} end zwave_command_class_multi_channel */
