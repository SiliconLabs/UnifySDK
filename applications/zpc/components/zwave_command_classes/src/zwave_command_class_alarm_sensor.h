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
 * @defgroup alarm_sensor_command_class Alarm Sensor Command Class
 * @ingroup command_classes
 * @brief Alarm Sensor Command Class handlers and control functions
 *
 * This module implement functions for generating and parsing the Z-Wave frames
 * for controlling the Alarm Sensor Command Class.
 *
 * The data model used for this command class is tailored to be mapped to the
 * ZCL Level cluster.
 *
 * The State and Bitmask attribute are hook for Get rules registrations. The
 * actual values that are resolved are the NodeID, State and the Seconds.
@startuml{attribute_store_alarm_sensor_command_class.png} "Alarm Sensor data model" width=10cm
title Alarm Sensor data model
allow_mixing
skinparam objectBorderColor black

legend top
<font color=#FEFEFE>ATTRIBUTE(type)</font> : ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_type
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
  version : value = Desired: [] - Reported: [2]

  object "Type" as type #FFFFFF
  type : Attribute Type = ATTRIBUTE(TYPE)
  type : value = Reported: [00]

  object "Bitmask" as bitmask #FFFFFF
  bitmask : Attribute Type = ATTRIBUTE(BITMASK)
  bitmask : value = Reported: [01]

  object "State" as state #FFFFFF
  state : Attribute Type = ATTRIBUTE(STATE)
  state : value = Reported: [255]

  object "NodeID" as nodeid #FFFFFF
  nodeid : Attribute Type = ATTRIBUTE(NODE_ID)
  nodeid : value = Reported: [20]

  object "Seconds" as seconds #FFFFFF
  seconds : Attribute Type = ATTRIBUTE(SECONDS)
  seconds : value = Reported: [10]
}

node *-- endpoint
endpoint *-- version
endpoint *-- type
type *-- state
type *-- nodeid
type *-- seconds
endpoint *-- bitmask


@enduml
 *
 * @{
*/

#ifndef ZWAVE_COMMAND_CLASS_ALARM_SENSOR_H
#define ZWAVE_COMMAND_CLASS_ALARM_SENSOR_H

#include "sl_status.h"
#include "attribute_store.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief This function initialize the Alarm Sensor Command Class handler
 *
 * @return SL_STATUS_OK on success, any other error code for an error.
 */
sl_status_t zwave_command_class_alarm_sensor_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_ALARM_SENSOR_H
/** @} end alarm_sensor_command_class */
