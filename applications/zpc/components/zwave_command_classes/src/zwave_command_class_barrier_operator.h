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
 * @defgroup barrier_operator_command_class Barrier Operator Command Class
 * @ingroup command_classes
 * @brief Barrier Operator Command Class handlers and control functions
 *
 * This module implement functions for generating and parsing the Z-Wave frames
 * for controlling the Barrier Operator Command Class.
 *
 * The data model used for this command class is tailored to be mapped to the
 * ZCL Level cluster.
 *
 * The State and Subsystem State attributes are hook for Set/Get rules registrations, 
 * Bitmsk is hook for Get rule registration. The
 * actual values that are resolved is the Subsystem Type.
@startuml{attribute_store_barrier_operator_command_class.png} "Barrier Operator data model" width=10cm
title Barrier Operator data model
allow_mixing
skinparam objectBorderColor black

legend top
<font color=#FEFEFE>ATTRIBUTE(type)</font> : ATTRIBUTE_COMMAND_CLASS_BARRIER_OPERATOR_type
endlegend

package "Attribute Store" <<Database>> {
  object "NodeID" as node #f2ffe6
  node : Attribute Type = ATTRIBUTE_NODE_ID
  node : value = Desired: [], Reported: [03]

  object "Endpoint Attribute" as endpoint #e6fff7
  endpoint : Attribute Type = ATTRIBUTE_ENDPOINT_ID
  endpoint : value = Desired: [] - Reported: [04]

  object "Version" as version #FEFEFE
  version : Attribute Type = ATTRIBUTE(VERSION)
  version : value = Desired: [] - Reported: [1]

  object "State" as state #FFFFFF
  state : Attribute Type = ATTRIBUTE(STATE)
  state : value = Desired: [00] - Reported: [255]

  object "Bitmask" as bitmask #FFFFFF
  bitmask : Attribute Type = ATTRIBUTE(BITMASK)
  bitmask : value = Reported: [01]

  object "Subsystem Type" as stype #FFFFFF
  stype : Attribute Type = ATTRIBUTE(SUBSYSTEM_TYPE)
  stype : value = Reported: [0]

  object "Subsystem State" as sstate #FFFFFF
  sstate : Attribute Type = ATTRIBUTE(SUBSYSTEM_STATE)
  sstate : value =  Desired: [00] - Reported: [255]

  
}

node *-- endpoint
endpoint *-- version
endpoint *-- state
endpoint *-- bitmask
endpoint *-- stype
stype *-- sstate


@enduml
 *
 * @{
*/

#ifndef ZWAVE_COMMAND_CLASS_BARRIER_OPERATOR_H
#define ZWAVE_COMMAND_CLASS_BARRIER_OPERATOR_H

#include "sl_status.h"
#include "attribute_store.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief This function initialize the Barrier Operator Command Class handler
 *
 * @return SL_STATUS_OK on success, any other error code for an error.
 */
sl_status_t zwave_command_class_barrier_operator_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_BARRIER_OPERATOR_H
/** @} end barrier_operator_command_class */
