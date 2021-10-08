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
 * @defgroup zwave_command_class_firmware_update Firmware Update Command Class
 * @ingroup command_classes
 * @brief Firmware Update Command Class transfer API functions
 *
 * This module implement functions for the support and control of the
 * Firmware Update Command Class.
 *
 * The control part is sub-divided into 2 parts:
 * - Gathering Firmware Update information
 * - Transferring images.
 *
 * All control of the Firmware Update Command Class relies on the Attribute
 * Store state. 2 Subtrees are used to keep track of everything.
 *
 * The following subtree is used to store information about firmwares from
 * supporting nodes:
 *
@startuml{attribute_store_firmware_update.png} "Firmware Update information data model" width=10cm
title Firmware Update data model
allow_mixing
skinparam objectBorderColor black

legend top
<font color=#FEFEFE>ATTRIBUTE(type)</font> : ATTRIBUTE_COMMAND_CLASS_FWU_MD_type
endlegend

package "Attribute Store" <<Database>> {
  object "NodeID" as node #f2ffe6
  node : Attribute Type = ATTRIBUTE_NODE_ID
  node : value = Desired: [], Reported: [03]

  object "Endpoint Attribute" as endpoint #e6fff7
  endpoint : Attribute Type = ATTRIBUTE_ENDPOINT_ID
  endpoint : value = Desired: [] - Reported: [04]

  object "Version" as version #FFFFFF
  version : Attribute Type = ATTRIBUTE(VERSION)
  version : value = Desired: [] - Reported: [5]

  object "Firmware Update" as firmware_update #FFFFFF
  firmware_update : Attribute Type = ATTRIBUTE(FWU)
  firmware_update : value = Desired: [], Reported: [1]

  package "For each Firmware ID (0..N)" <<Rectangle>> {
    object "Firmware Entry" as firmare_entry #FFFFFF
    firmare_entry : Attribute Type = ATTRIBUTE(FWU_FW)
    firmare_entry : value = Desired: [], Reported: [0]

    object "Firmware ID" as firmare_id #FFFFFF
    firmare_id : Attribute Type = ATTRIBUTE(FWU_FW_ID)
    firmare_id : value = Desired: [], Reported: [1234]

    object "Firmware Upgradable" as firmare_upgradable #FFFFFF
    firmare_upgradable : Attribute Type = ATTRIBUTE(FWU_FW_ID)
    firmare_upgradable : value = Desired: [], Reported: [0xFF]
  }

  object "Manufacturer ID" as manufacturer_id #FFFFFF
  manufacturer_id : Attribute Type = ATTRIBUTE(FWU_MANUFACTURER_ID)
  manufacturer_id : value = Desired: [], Reported: [0000]

  object "Fragment size" as fragment_size #FFFFFF
  fragment_size : Attribute Type = ATTRIBUTE(FWU_FRAGMENT_SIZE)
  fragment_size : value = Desired: [], Reported: [50]

  object "Activation" as activation #FFFFFF
  activation : Attribute Type = ATTRIBUTE(FWU_ACTIVATION)
  activation : value = Desired: [], Reported: [1]

  object "CC" as cc #FFFFFF
  cc : Attribute Type = ATTRIBUTE(FWU_CC)
  cc : value = Desired: [], Reported: [1]
}

node *-- endpoint
endpoint *-- version
endpoint *-- firmware_update
firmware_update *-- firmare_entry
firmare_entry *-- firmare_id
firmare_entry *-- firmare_upgradable
firmware_update *-- manufacturer_id
firmware_update *-- fragment_size
firmware_update *-- activation
firmware_update *-- cc

manufacturer_id -[hidden]--> fragment_size
activation -[hidden]--> cc

@enduml
 *
 * The following subtree is used to keep track of firmware updates image
 * transfers:
 *
@startuml{attribute_store_firmware_update_transfer.png} "Firmware Update transfer data model" width=10cm
title Firmware Update transfer data model
allow_mixing
skinparam objectBorderColor black

legend top
<font color=#FEFEFE>ATTRIBUTE(type)</font> : ATTRIBUTE_COMMAND_CLASS_FWU_MD_type
endlegend

package "Attribute Store" <<Database>> {
  object "NodeID" as node #f2ffe6
  node : Attribute Type = ATTRIBUTE_NODE_ID
  node : value = Desired: [], Reported: [03]

  object "Endpoint Attribute" as endpoint #e6fff7
  endpoint : Attribute Type = ATTRIBUTE_ENDPOINT_ID
  endpoint : value = Desired: [] - Reported: [04]

  object "Firmware Update" as firmware_update #FFFFFF
  firmware_update : Attribute Type = ATTRIBUTE(FWU)
  firmware_update : value = Desired: [], Reported: [1]

  object "Transfer State" as transfer_state #FFFFFF
  transfer_state : Attribute Type = ATTRIBUTE(FW_TRANSFER)
  transfer_state : value = Desired: [1] - Reported: []

  object "Target" as firmware_target #FFFFFF
  firmware_target : Attribute Type = ATTRIBUTE(TRANSFER_TARGET)
  firmware_target : value = Desired: [], Reported: [0]

  object "Apply Timestamp" as apply_timestamp #FFFFFF
  apply_timestamp : Attribute Type = ATTRIBUTE(TRANSFER_APPLY_TIMESTAMP)
  apply_timestamp : value = Desired: [], Reported: [234030495820439]

  object "Filename" as filename #FFFFFF
  filename : Attribute Type = ATTRIBUTE(TRANSFER_FILENAME)
  filename : value = Desired: [], Reported: [ZW_PowerStrip.gbl]

  object "Offset" as transfer_offset #FFFFFF
  transfer_offset : Attribute Type = ATTRIBUTE(TRANSFER_OFFSET)
  transfer_offset : value = Desired: [], Reported: [4]

  object "Size" as total_size #FFFFFF
  total_size : Attribute Type = ATTRIBUTE(TRANSFER_SIZE)
  total_size : value = Desired: [], Reported: [85020]

  object "Last Status" as last_status #FFFFFF
  last_status : Attribute Type = ATTRIBUTE(FW_TRANSFER_LAST_STATUS)
  last_status : value = Desired: [], Reported: [0]

  object "Fragment Size" as fragment_size #FFFFFF
  fragment_size : Attribute Type = ATTRIBUTE(FWU_FRAGMENT_SIZE)
  fragment_size : value = Desired: [], Reported: [1C]

  object "Expiry time" as expiry_time #FFFFFF
  expiry_time : Attribute Type = ATTRIBUTE(TRANSFER_EXPIRY_TIME)
  expiry_time : value = Desired: [], Reported: [234030496320439]
}

node *-- endpoint
endpoint *-- firmware_update
firmware_update *-- transfer_state
transfer_state *-- firmware_target
transfer_state *-- apply_timestamp
transfer_state *-- filename

firmware_target -[hidden]--> transfer_offset
apply_timestamp -[hidden]--> total_size
filename -[hidden]--> last_status
transfer_offset -[hidden]--> fragment_size
total_size -[hidden]--> expiry_time

transfer_state *-- transfer_offset
transfer_state *-- total_size
transfer_state *-- last_status
transfer_state *-- fragment_size
transfer_state *-- expiry_time

@enduml
 *
 * The following state diagram denotes how state transitions while doing firmware update:
 *
@startuml
[*] --> IDLE
IDLE --> ONGOING : Initiate firmware update
IDLE: Reported: IDLE
IDLE: Desired: IDLE
ONGOING --> ONGOING : End node responds and frame valid
ONGOING: Reported: ONGOING
ONGOING: Desired: IDLE
ONGOING --> WAITING_FOR_ACTIVATION : Update done and end node supports activation
WAITING_FOR_ACTIVATION --> ACTIVATION_TIMEOUT: Set apply timer callback
ACTIVATION_TIMEOUT --> IDLE : Get rule triggerred to send Activtion command. Update either success or fail.
WAITING_FOR_ACTIVATION: Reported: WAITING_FOR_ACTIVATION
WAITING_FOR_ACTIVATION: Desired: IDLE
ACTIVATION_TIMEOUT: Reported: Undefined
ACTIVATION_TIMEOUT: Desired: WAITING_FOR_ACTIVATION
ONGOING --> WAITING_FOR_REBOOT : Update done and end node requires reboot
WAITING_FOR_REBOOT: Reported: WAITING_FOR_REBOOT
WAITING_FOR_REBOOT: Desired: IDLE
WAITING_FOR_REBOOT --> IDLE : Reboot done. Update either success or fail.
ONGOING --> IDLE : Update either success or fail. No reboot needed.
@enduml
 *
 * @{
*/

#ifndef ZWAVE_COMMAND_CLASS_FIRMWARE_UPDATE_H
#define ZWAVE_COMMAND_CLASS_FIRMWARE_UPDATE_H

#include "sl_status.h"
#include "zwave_utils.h"                       //zwave_node_id_t
#include "zwave_controller_types.h"            //zwave_endpoint_id_t
#include "zwave_controller_connection_info.h"  //zwave_controller_encapsulation_scheme_t
#include "contiki-conf.h"                      // clock_time_t

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Attempts to start a Firmware Update procedure for a node.
 *
 * This function will verify that that paramters are in accordance with the
 * nodes capabilities and configure the Attribute Store state to initiate
 * a Firmware Update request, and subsequent transfer.
 *
 * @param node_id           The NodeID for which the Firmware Update must be
 *                          initiated
 * @param endpoint_id       The Endpoint ID for which the Firmware Update must
 *                          take place.
 * @param firmware_target   The Firmware target to upgrade at the end node.
 * @param apply_after       Timestamp of when to apply the firmware.
 *                          If the node does not support subsequent activations,
 *                          the firmware update attempt will be rejected and
 *                          postponed.
 * @param filename          The filename to open to access the content of the
 *                          firmware
 *
 * @returns SL_STATUS_OK if the request was accepted and will be initiated. Other
 *          error codes otherwise.
 */
sl_status_t command_class_firmware_update_initiate_firmware_update(
  zwave_node_id_t node_id,
  zwave_endpoint_id_t endpoint_id,
  uint32_t firmware_target,
  unsigned long apply_after,
  const char *filename);

/**
 * @brief Abort any ongoing Firmware operation with a node
 *
 * This function will instruct the Firmware Update Command Class
 * to return the firmware transfer to idle.
 *
 * @param node_id           The NodeID for which the Firmware Update transfer
 *                          state must go to idle
 * @param endpoint_id       The Endpoint ID for which the Firmware Update must
 *                          go to idle.
 */
void command_class_firmware_update_abort_ongoing_firmware_operation(
  zwave_node_id_t node_id, zwave_endpoint_id_t endpoint_id);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_FIRMWARE_UPDATE_H
/** @} end zwave_command_class_firmware_update */
