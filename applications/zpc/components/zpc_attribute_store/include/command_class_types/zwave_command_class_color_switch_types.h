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
 * @defgroup zwave_command_class_color_switch_types Type definitions for attribute storage of the Color Switch Command Class.
 * @ingroup zpc_attribute_store_command_classes_types
 * @brief Type definitions for the Color Switch Command Class.
 *
 *
@startuml{attribute_store_color_switch_command_class.png} "Color Switch data model" width=10cm
allow_mixing
skinparam objectBorderColor black

legend top
<font color=#FEFEFE>ATTRIBUTE(type)</font> : ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_type
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

  object "Supported Components IDs" as supported_components_ids #FFFFFF
  supported_components_ids : Attribute Type = ATTRIBUTE(SUPPORTED_COLOR_COMPONENT_MASK)
  supported_components_ids : value = Desired: [] - Reported: [1C]

  object "Color State" as color_state #FFFFFF
  color_state : Attribute Type = ATTRIBUTE(ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_STATE)
  color_state : value = Desired: [1] - Reported: [0]

  package "For each Component ID" <<Rectangle>> {

    object "Component ID" as component_id #FFFFFF
    component_id : Attribute Type = ATTRIBUTE(COLOR_COMPONENT_ID)
    component_id : value = Desired: [] - Reported: [3]

    object "Value" as component_id_value #FFFFFF
    component_id_value : Attribute Type = ATTRIBUTE(VALUE)
    component_id_value : value = Desired: [] - Reported: [255]

    object "Duration" as component_id_duration #FFFFFF
    component_id_duration : Attribute Type = ATTRIBUTE(DURATION)
    component_id_duration : value = Desired: [] - Reported: [2]
}

node *-- endpoint
endpoint *-- version
endpoint *-- supported_components_ids
endpoint *-- color_state
color_state *-- component_id
component_id *-- component_id_value
component_id *-- component_id_duration

@enduml

 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_COLOR_SWITCH_TYPES_H
#define ZWAVE_COMMAND_CLASS_COLOR_SWITCH_TYPES_H

#include <stdint.h>

///> Color component bitmask. uint16_t
typedef uint16_t color_component_bitmask_t;

///> Color Component ID, identifying a color component.
typedef uint8_t color_component_id_t;

///> Color Component ID's value.
typedef uint32_t color_component_id_value_t;
typedef uint32_t color_component_id_duration_t;

#endif  //ZWAVE_COMMAND_CLASS_COLOR_SWITCH_TYPES_H
/** @} end zwave_command_class_color_switch_types */
