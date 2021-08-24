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
 * @defgroup zwave_command_class_basic Basic Command Class
 * @ingroup command_classes
 * @brief Basic Command Class control handler
 *
 * Handling of the Basic Command Class is a little special since basic is not
 * present in the NIF. The only way to know if a node supports the Basic Command Class
 * is to try to send a Basic Get Command to the node (sad but true).
 *
 * Some nodes also wrongfully send Basic Report to their Lifeline instead of
 * using the "main" Command Class (e.g. Binary Switch). In this case, we must
 * not start controlling the Basic Command Class
 *
@startuml{attribute_store_basic_command_class.png} "Basic data model" width=10cm
title Basic Command Class data model
allow_mixing
skinparam objectBorderColor black

legend top
<font color=#FEFEFE>ATTRIBUTE(type)</font> : ATTRIBUTE_COMMAND_CLASS_BASIC_type
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

  object "Value" as value #FFFFFF
  value : Attribute Type = ATTRIBUTE(VALUE)
  value : value = Desired: [0x00], Reported: [0xFF]

  object "Duration" as duration #FFFFFF
  duration : Attribute Type = ATTRIBUTE(DURATION)
  duration : value = Desired: [10], Reported: [10]

  object "Probe State" as probe_status #FFFFFF
  probe_status : Attribute Type = ATTRIBUTE(PROBE_STATUS)
  probe_status : value = Desired: [] - Reported: [2]
}

node *-- endpoint
endpoint *-- version
endpoint *-- value
endpoint *-- duration
endpoint *-- probe_status

@enduml
 *
 * @{
*/

#ifndef ZWAVE_COMMAND_CLASS_BASIC_H
#define ZWAVE_COMMAND_CLASS_BASIC_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the basic command class handler
 *
 * @return sl_status_t
 */
sl_status_t zwave_command_class_basic_init();
#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_BASIC_H
/** @} end zwave_command_class_basic */
