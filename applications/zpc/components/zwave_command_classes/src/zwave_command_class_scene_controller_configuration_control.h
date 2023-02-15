/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup zwave_command_class_scene_controller_configuration_control Scene Controller Configuration Command Class
 * @ingroup command_classes
 * @brief Scene Controller Configuration Command Class control functions
 *
 * This module provides control of the Scene Controller Configuration Command Class.
 * The attribute store model is as follows:
 *
@startuml{attribute_store_scene_controller_configuration_command_class.png} "Scene Controller Configuration data model" width=10cm
allow_mixing
skinparam objectBorderColor black

legend top
<font color=#FEFEFE>ATTRIBUTE(type)</font> : ATTRIBUTE_COMMAND_CLASS_SCENE_type
endlegend

package "Attribute Store" <<Database>> {
  object "NodeID" as node #f2ffe6
  node : Attribute Type = ATTRIBUTE_NODE_ID
  node : value = Desired: [], Reported: [03]

  object "Endpoint Attribute" as endpoint #e6fff7
  endpoint : Attribute Type = ATTRIBUTE_ENDPOINT_ID
  endpoint : value = Desired: [] - Reported: [04]

  object "Version" as version #FEFEFE
  version : Attribute Type = ATTRIBUTE(ACTUATOR_CONF_VERSION)
  version : value = Desired: [] - Reported: [1]

  package "For all supported Association groups" <<Rectangle>> {
    object "Association Group ID" as association_group_id #FFFFFF
    association_group_id : Attribute Type = ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID
    association_group_id : value = Desired: [] - Reported: [1]

    object "Local Scene Configuration" as local_scene_configuration #FFFFFF
    local_scene_configuration : Attribute Type = ATTRIBUTE(CONTROLLER_CONFIGURATION)
    local_scene_configuration : value = Desired: [0] - Reported: [0]

    object "Scene ID" as scene_id #FFFFFF
    scene_id : Attribute Type = ATTRIBUTE(CONTROLLER_SETTING_ID)
    scene_id : value = Desired: [0] - Reported: [0]

    object "Dimming Duration Setting" as dimming_duration_setting #FFFFFF
    dimming_duration_setting : Attribute Type = ATTRIBUTE(CONTROLLER_SETTING_DIMMING_DURATION)
    dimming_duration_setting : value = Desired: [0] - Reported: [20]
  }
}

node *-- endpoint
endpoint *-- version
endpoint *-- association_group_id
association_group_id *-- local_scene_configuration
local_scene_configuration *-- scene_id
local_scene_configuration *-- dimming_duration_setting
@enduml
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_SCENE_CONTROLLER_CONFIGURATION_CONTROL_H
#define ZWAVE_COMMAND_CLASS_SCENE_CONTROLLER_CONFIGURATION_CONTROL_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the Scene Controller Configuration Command Class control handler.
 *
 * @returns SL_STATUS_OK
 */
sl_status_t zwave_command_class_scene_controller_configuration_control_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_SCENE_CONTROLLER_CONFIGURATION_CONTROL_H
/** @} end zwave_command_class_scene_controller_configuration_control */
