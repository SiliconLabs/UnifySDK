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

/**
 * @defgroup zwave_command_class_wake_up Wake Up Command Class
 * @ingroup command_classes
 * @brief Command Class handler for the Wake Up Command Class
 *
 * It is in charge of configuring the Wake Up interval, unpausing nodes when
 * they issue a Wake Up Notification and send a Wake Up No More when the
 * resolver is done.
 *
 * Wake Up capabilities are always created, also for v1 nodes. For v1 nodes,
 * they will be filled up automatically with default values.
 *
 * The Wake Up Command Class uses this model in the Attribute Store to represent
 * supporting nodes:
@startuml{attribute_store_wake_up_command_class.png} "Wake Up Command Class data model" width=10cm
title Wake Up data model
allow_mixing
skinparam objectBorderColor black
skinparam NoteBackgroundColor #fffae3
skinparam NoteBorderColor red

legend top
<font color=#FEFEFE>ATTRIBUTE(type)</font> : ATTRIBUTE_COMMAND_CLASS_WAKE_UP_type
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

  object "Setting" as setting #FFFFFF
  setting : Attribute Type = ATTRIBUTE(SETTING)
  setting : value = Desired: [0] - Reported: [1]

  object "Capabilities" as capabilities #FFFFFF
  capabilities : Attribute Type = ATTRIBUTE(CAPABILITIES)
  capabilities : value = Desired: [], Reported: [01]

  object "Wake Up NodeID" as wake_up_node_id #FFFFFF
  wake_up_node_id : Attribute Type = ATTRIBUTE(NODE_ID)
  wake_up_node_id : value = Desired: [01], Reported: [10]

  object "Wake Up Interval" as wake_up_interval #FFFFFF
  wake_up_interval : Attribute Type = ATTRIBUTE(INTERVAL)
  wake_up_interval : value = Desired: [80,51,01,00], Reported: [14,00,00,00]

  object "Maximum Wake Up Interval" as maximum_wake_up_interval #FFFFFF
  maximum_wake_up_interval : Attribute Type = ATTRIBUTE(MAXIMUM_INTERVAL)
  maximum_wake_up_interval : value = Desired: [], Reported: [80,51,01,00]

  object "Minimum Wake Up Interval" as minimum_wake_up_interval #FFFFFF
  minimum_wake_up_interval : Attribute Type = ATTRIBUTE(MINIMUM_INTERVAL)
  minimum_wake_up_interval : value = Desired: [], Reported: [14,00,00,00]

  object "Default Wake Up Interval" as default_wake_up_interval #FFFFFF
  default_wake_up_interval : Attribute Type = ATTRIBUTE(DEFAULT_INTERVAL)
  default_wake_up_interval : value = Desired: [], Reported: [2c,01,00,00]

  object "Wake Up Interval Step" as wake_up_interval_step #FFFFFF
  wake_up_interval_step : Attribute Type = ATTRIBUTE(INTERVAL_STEP)
  wake_up_interval_step : value = Desired: [], Reported: [14,00,00,00]

  object "Capabilities bitmask" as capabilties_bitmask #FFFFFF
  capabilties_bitmask : Attribute Type = ATTRIBUTE(CAPABILITIES_BITMASK)
  capabilties_bitmask : value = Desired: [], Reported: [01]

  note left of capabilities
    Resolution listener
    Configure Wake Up Interval
  end note

  note left of node
    Resolution listener
    Send Wake Up No More
  end note
}

node *-- endpoint
endpoint *-- version
endpoint *-- setting
endpoint *-- capabilities

setting *-- wake_up_node_id
setting *-- wake_up_interval

capabilities *-- maximum_wake_up_interval
capabilities *-- minimum_wake_up_interval
capabilities *-- default_wake_up_interval
capabilities *-- wake_up_interval_step
capabilities *-- capabilties_bitmask

maximum_wake_up_interval -[hidden]--> default_wake_up_interval
minimum_wake_up_interval -[hidden]--> wake_up_interval_step

@enduml
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_WAKE_UP_H
#define ZWAVE_COMMAND_CLASS_WAKE_UP_H

#include "attribute_store.h"
#include "sl_status.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the Wake Up Command Class handler
 */
sl_status_t zwave_command_class_wake_up_init();

/**
 * @brief Verifies if a Node supports the Wake Up on demand functionality
 *
 * @param node_id_node      Attribute Store node representing the NodeID
 *
 * @returns true if the NodeID supports Wake Up On demand
 * @returns false if the NodeID does not supports Wake Up On demand
 */
bool zwave_command_class_wake_up_supports_wake_up_on_demand(
  attribute_store_node_t node_id_node);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_WAKE_UP_H
/** @} end zwave_command_class_wake_up */
