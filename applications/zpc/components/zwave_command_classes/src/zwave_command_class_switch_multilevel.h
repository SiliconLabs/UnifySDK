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

#ifndef ZWAVE_COMMAND_CLASS_SWITCH_MULTILEVEL_H
#define ZWAVE_COMMAND_CLASS_SWITCH_MULTILEVEL_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup switch_multilevel_command_class Switch Multilevel Command Class
 * @ingroup command_classes
 * @brief Switch Multilevel Command Class handlers and control functions
 *
 * This module implement functions for generating and parsing the Z-Wave frames
 * for controlling the Multilevel Switch Command Class.
 *
 * The data model used for this command class is tailored to be mapped to the
 * ZCL Level cluster.
 *
 * The State attribute is a hook for the Set / Get rule registrations. The
 * actual values being set and resolved are the duration and the value.
 * If the duration and/or the value needs a resolution, the Command Class
 * handler adjusts the state in order to trigger the resolver:
 * state = reported [] for a get resolution
 * state = desired [1] reported [0] for a set resolution
 * state = desired [0] reported [0] for a no resolution
 *
 * on_off is a virtual setting directly mapped to the OnOff attribute of the
 * OnOff ZCL cluster. It helps deciding if the command class should take action
 *
@startuml{attribute_store_multilevel_switch_command_class.png} "Multilevel Switch data model" width=10cm
title Multilevel Switch data model
allow_mixing
skinparam objectBorderColor black

legend top
<font color=#FEFEFE>ATTRIBUTE(type)</font> : ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_type
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
  version : value = Desired: [] - Reported: [4]

  object "State" as state #FFFFFF
  state : Attribute Type = ATTRIBUTE(STATE)
  state : value = Desired: [1] - Reported: [1]

  object "Value" as value #FFFFFF
  value : Attribute Type = ATTRIBUTE(VALUE)
  value : value = Desired: [63], Reported: [63]

  object "Duration" as duration #FFFFFF
  duration : Attribute Type = ATTRIBUTE(DURATION)
  duration : value = Desired: [10], Reported: [10]

  object "Substate update" as substate_update #FFFFFF
  substate_update : Attribute Type = ATTRIBUTE(SUBSTATE_UPDATE)
  substate_update : value = Desired: [1], Reported: []

  object "On Off (virtual state)" as on_off #FFFFFF
  on_off : Attribute Type = ATTRIBUTE(ON_OFF)
  on_off : value = Desired: [0], Reported: [255]

  object "Capabilities requested" as capabilities_requested #FFFFFF
  capabilities_requested : Attribute Type = ATTRIBUTE(CAPABILITIES_REQUESTED)
  capabilities_requested : value = Desired: [], Reported: [1]
}

node *-- endpoint
endpoint *-- version
endpoint *-- state
state *-- value
state *-- duration
state *-- on_off
state *-- substate_update
endpoint *-- capabilities_requested

value -[hidden]--> on_off
duration -[hidden]--> substate_update

@enduml
 *
 * @{
*/

/**
 * @brief This function initialize the Multilevel Switch Command Class handler
 *
 * @return SL_STATUS_OK on success, any other error code for an error.
 */
sl_status_t zwave_command_class_switch_multilevel_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_SWITCH_MULTILEVEL_H
/** @} end switch_multilevel_command_class */
