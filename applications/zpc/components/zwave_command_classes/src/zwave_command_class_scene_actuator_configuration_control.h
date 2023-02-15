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
 * @defgroup zwave_command_class_scene_actuator_configuration_control Scene Actuator Configuration Command Class Control
 * @ingroup command_classes
 * @brief Scene Actuator Configuration Command Class control functions
 *
 * This module provides control of the Scene Actuator Configuration Command Class.
 * The attribute store model is as follows:
 *
 @startuml{attribute_store_scene_actuator_configuration_command_class.png} "Scene Actuator Configuration data model" width=10cm

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
  version : Attribute Type = ATTRIBUTE(CONTROLLER_CONF_VERSION)
  version : value = Desired: [] - Reported: [1]

  object "Local Scenes" as local_scenes #FFFFFF
  local_scenes : Attribute Type = ATTRIBUTE(LOCAL_SCENES)
  local_scenes : value = Desired: [] - Reported: []

  object "Scene Configuration" as scene_configuration #FFFFFF
  scene_configuration : Attribute Type = ATTRIBUTE(ACTUATOR_CONFIGURATION)
  scene_configuration : value = Desired: [] - Reported: []

  package "For Scene IDs 1..255" <<Rectangle>> {
    object "Scene ID" as scene_id #FFFFFF
    scene_id : Attribute Type = ATTRIBUTE(ACTUATOR_ID)
    scene_id : value = Desired: [] - Reported: [x]

    object "Dimming Duration Setting" as dimming_duration_setting #FFFFFF
    dimming_duration_setting : Attribute Type = ATTRIBUTE(ACTUATOR_SETTING_DIMMING_DURATION)
    dimming_duration_setting : value = Desired: [0] - Reported: [20]

    object "Level Setting" as level_setting #FFFFFF
    level_setting : Attribute Type = ATTRIBUTE(ACTUATOR_SETTING_LEVEL)
    level_setting : value = Desired: [3] - Reported: [100]

    object "Override Setting" as override_setting #FFFFFF
    override_setting : Attribute Type = ATTRIBUTE(ACTUATOR_SETTING_OVERRIDE)
    override_setting : value = Desired: [1] - Reported: [0]
  }
}

node *-- endpoint
endpoint *-- version
endpoint *-- local_scenes
local_scenes *-- scene_configuration
scene_configuration *-- scene_id
scene_id *-- dimming_duration_setting
scene_id *-- level_setting
scene_id *-- override_setting
@enduml
 *
 * Note that this command class will be a "no interview" command class.
 * The Actuator Configuration attribute will be created at start, but we will
 * not create the Scene IDs 1..255, nor read them.
 *
 * When an end user wants to configure scenes, only then, we will Set/Get the
 * actuator configuration.
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_SCENE_ACTUATOR_CONFIGURATION_CONTROL_H
#define ZWAVE_COMMAND_CLASS_SCENE_ACTUATOR_CONFIGURATION_CONTROL_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the Scene Actuator Configuration Command Class control handler.
 *
 * @returns SL_STATUS_OK
 */
sl_status_t zwave_command_class_scene_actuator_configuration_control_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_SCENE_ACTUATOR_CONFIGURATION_CONTROL_H
/** @} end zwave_command_class_scene_actuator_configuration_control */
