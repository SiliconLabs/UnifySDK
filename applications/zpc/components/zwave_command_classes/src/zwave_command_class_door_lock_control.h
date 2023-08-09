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
 * @defgroup door_lock_command_class DoorLock Command Class
 * @ingroup command_classes
 * @brief The module handles the control part of DoorLock Command Class
 *

@startuml
title Door Lock data model
allow_mixing
skinparam objectBorderColor black

legend top
<font color=#FEFEFE>ATTRIBUTE(type)</font> : ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_type
endlegend

package "Attribute Store" <<Database>> {
  object "NodeID" as node #f2ffe6
  node : Attribute Type = ATTRIBUTE_NODE_ID
  node : value = Desired: [], Reported: [03]

  object "Endpoint Attribute" as endpoint #e6fff7
  endpoint : Attribute Type = ATTRIBUTE_ENDPOINT_ID
  endpoint : value = Desired: [] - Reported: [04]

  object "Version" as version #FEFEFE
  version : Attribute Type = ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_VERSION
  version : value = Desired: [] - Reported: [2]

  object "Operation Lock Mode" as oper_lock_mode #FFFFFF
  oper_lock_mode : Attribute Type = OPERATION_DOOR_LOCK_MODE
  oper_lock_mode : value = Desired: [0x00], Reported: [0xFF]

  object "Operation Inside Handles Mode" as oper_inside_handles #FFFFFF
  oper_inside_handles : Attribute Type = OPERATION_INSIDE_DOOR_HANDLES_MODE
  oper_inside_handles : value = Desired: [10], Reported: [10]

  object "Operation Outside Handles Mode" as oper_outside_handles #FFFFFF
  oper_outside_handles : Attribute Type = OPERATION_OUTSIDE_DOOR_HANDLES_MODE
  oper_outside_handles : value = Desired: [10], Reported: [10]

  object "Operation Condition" as oper_condition #FFFFFF
  oper_condition : Attribute Type = OPERATION_DOOR_CONDITION
  oper_condition : value = Desired: [0x00], Reported: [0xFF]

  object "Operation Lock Timeout Min" as oper_lock_min #FFFFFF
  oper_lock_min : Attribute Type = OPERATION_LOCK_TIMEOUT_MINUTES
  oper_lock_min : value = Desired: [10], Reported: [10]

  object "Operation Lock Timeout Sec" as oper_lock_sec #FFFFFF
  oper_lock_sec : Attribute Type = OPERATION_LOCK_TIMEOUT_SECONDS
  oper_lock_sec : value = Desired: [10], Reported: [10]

  object "Configuration Type" as conf_type #FFFFFF
  conf_type : Attribute Type = CONFIGURATION_OPERATION_TYPE
  conf_type : value = Desired: [1] - Reported: [0]

  object "Configuration Inside Handles State" as conf_inside_handles #FFFFFF
  conf_inside_handles : Attribute Type = CONFIGURATION_INSIDE_DOOR_HANDLES_STATE
  conf_inside_handles : value = Desired: [0x00], Reported: [0xFF]

  object "Configuration Outside Handles State" as conf_outside_handles #FFFFFF
  conf_outside_handles : Attribute Type = CONFIGURATION_OUTSIDE_DOOR_HANDLES_STATE
  conf_outside_handles : value = Desired: [10], Reported: [10]

  object "Configuration Lock Timeout Minutes" as conf_lock_min #FFFFFF
  conf_lock_min : Attribute Type = CONFIGURATION_LOCK_TIMEOUT_MINUTES
  conf_lock_min : value = Desired: [0x00], Reported: [0xFF]

  object "Configuration Lock Timeout Seconds" as conf_lock_sec #FFFFFF
  conf_lock_sec : Attribute Type = CONFIGURATION_LOCK_TIMEOUT_SECONDS
  conf_lock_sec : value = Desired: [10], Reported: [10]

  object "Operation" as operation #FFFFFF
  operation : Attribute Type = LOCK_OPERATION_REPORT
  operation : value = Desired: [10], Reported: [10]

  object "Configuration" as configuration #FFFFFF
  configuration : Attribute Type = CONFIGURATION
  configuration : value = Desired: [0x00], Reported: [0xFF]

  object "Capabilities" as capabilities #FFFFFF
  capabilities : Attribute Type = CAPABILITIES
  capabilities : value = Desired: [10], Reported: [10]


  object "Configuration Auto Relock" as conf_relock #FFFFFF
  conf_relock : Attribute Type = CONFIGURATION_AUTO_RELOCK
  conf_relock : value = Desired: [10], Reported: [10]

  object "Configuration Hold Release Time" as conf_release_time #FFFFFF
  conf_release_time : Attribute Type = CONFIGURATION_HOLD_RELEASE_TIME
  conf_release_time : value = Desired: [10], Reported: [10]

  object "Configuration BTB" as conf_btb #FFFFFF
  conf_btb : Attribute Type = CONFIGURATION_BTB
  conf_btb : value = Desired: [10], Reported: [10]

  object "Configuration TA" as conf_ta #FFFFFF
  conf_ta : Attribute Type = CONFIGURATION_TA
  conf_ta : value = Desired: [10], Reported: [10]


  object "Operation Report Target Mode" as oper_mode #FFFFFF
  oper_mode : Attribute Type = OPERATION_REPORT_TARGET_MODE
  oper_mode : value = Desired: [10], Reported: [10]

  object "Operation Report Duration" as oper_duration #FFFFFF
  oper_duration : Attribute Type = OPERATION_REPORT_DURATION
  oper_duration : value = Desired: [10], Reported: [10]


  object "Capabilities Operation Type Bitmask" as capabilities_oper_type #FFFFFF
  capabilities_oper_type : Attribute Type = CAPABILITIES_OPERATION_TYPE_BITMASK
  capabilities_oper_type : value = Desired: [10], Reported: [10]

  object "Capabilities Lock Mode" as cap_lock_mode #FFFFFF
  cap_lock_mode : Attribute Type = CAPABILITIES_LOCK_MODE
  cap_lock_mode : value = Desired: [10], Reported: [10]

  object "Capabilities Outside Handle Modes" as cap_outside_handles_modes #FFFFFF
  cap_outside_handles_modes : Attribute Type = CAPABILITIES_OUTSIDE_HANDLE_MODES_BITMASK
  cap_outside_handles_modes : value = Desired: [10], Reported: [10]

  object "Capabilities Inside Handle Modes" as cap_inside_handles_modes #FFFFFF
  cap_inside_handles_modes : Attribute Type = CAPABILITIES_INSIDE_HANDLE_MODES_BITMASK
  cap_inside_handles_modes : value = Desired: [10], Reported: [10]
  
  object "Capabilities Door Component" as cap_door_component #FFFFFF
  cap_door_component : Attribute Type = CAPABILITIES_DOOR_COMPONENT_BITMASK
  cap_door_component : value = Desired: [10], Reported: [10]

  object "Capabilities ASR" as cap_asr #FFFFFF
  cap_asr : Attribute Type = CAPABILITIES_ARS
  cap_asr : value = Desired: [10], Reported: [10]

  object "Capabilities HRS" as cap_hrs #FFFFFF
  cap_hrs : Attribute Type = CAPABILITIES_HRS
  cap_hrs  : value = Desired: [10], Reported: [10]

  object "Capabilities TAS" as cap_tas #FFFFFF
  cap_tas : Attribute Type = CAPABILITIES_TAS
  cap_tas : value = Desired: [10], Reported: [10]
 
  object "Capabilities BTBS" as cap_btbs #FFFFFF
  cap_btbs : Attribute Type = CAPABILITIES_BTBS
  cap_btbs : value = Desired: [10], Reported: [10]
}

node *-- endpoint

endpoint *-- version

endpoint *-- operation 
endpoint *-- oper_lock_mode
endpoint *-- oper_inside_handles 
endpoint *-- oper_outside_handles 
endpoint *-- oper_condition 
endpoint *-- oper_lock_min 
endpoint *-- oper_lock_sec 
operation *-- oper_mode 
operation *-- oper_duration 

oper_lock_mode *-[hidden]- oper_inside_handles
oper_inside_handles *-[hidden]- oper_outside_handles 
oper_outside_handles *-[hidden]- oper_condition 
oper_condition *-[hidden]- oper_lock_min
oper_lock_min *-[hidden]- oper_lock_sec
oper_lock_sec *-[hidden]- operation 
operation *-[hidden]- oper_mode
oper_mode *-[hidden]- oper_duration 

endpoint *-- configuration 
endpoint *-- conf_type
endpoint *-- conf_outside_handles 
endpoint *-- conf_inside_handles
endpoint *-- conf_lock_min 
endpoint *-- conf_lock_sec 
configuration *-- conf_relock
configuration *-- conf_release_time 
configuration *-- conf_btb 
configuration *-- conf_ta

conf_type *-[hidden]- conf_outside_handles 
conf_outside_handles *-[hidden]- conf_inside_handles
conf_inside_handles *-[hidden]- conf_lock_min 
conf_lock_min *-[hidden]- conf_lock_sec 
conf_lock_sec *-[hidden]- configuration 
configuration *-[hidden]- conf_relock
conf_relock *-[hidden]- conf_release_time 
conf_release_time *-[hidden]- conf_btb 
conf_btb *-[hidden]- conf_ta

endpoint *-- capabilities 
capabilities *-- capabilities_oper_type 
capabilities *-- cap_lock_mode 
capabilities *-- cap_outside_handles_modes 
capabilities *-- cap_inside_handles_modes 
capabilities *-- cap_door_component 
capabilities *-- cap_asr 
capabilities *-- cap_hrs  
capabilities *-- cap_tas 
capabilities *-- cap_btbs 

capabilities *-[hidden]- capabilities_oper_type 
capabilities_oper_type *-[hidden]- cap_lock_mode  
cap_lock_mode *-[hidden]- cap_outside_handles_modes 
cap_outside_handles_modes *-[hidden]- cap_inside_handles_modes 
cap_inside_handles_modes *-[hidden]- cap_door_component 
cap_door_component *-[hidden]- cap_asr 
cap_asr *-[hidden]- cap_hrs  
cap_hrs *-[hidden]- cap_tas 
cap_tas *-[hidden]- cap_btbs 

@enduml

 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_DOOR_LOCK_H
#define ZWAVE_COMMAND_CLASS_DOOR_LOCK_H

#include "sl_status.h"
#include "attribute_store.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief This function initialize the DoorLock Command Class handler
 *
 * @return SL_STATUS_OK on success, any other error code for an error.
 */
sl_status_t zwave_command_class_door_lock_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_DOOR_LOOK_H
/** @} end door_lock_command_class */
