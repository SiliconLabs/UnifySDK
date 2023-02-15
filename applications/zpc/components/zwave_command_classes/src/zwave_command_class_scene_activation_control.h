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
 * @defgroup zwave_command_class_scene_activation_control Scene Activation Command Class Control
 * @ingroup command_classes
 * @brief Scene Activation Command Class control functions
 *
 * This module provides control of the Scene Activation Command Class.
 * The attribute store model is as follows:
 *
 @startuml{attribute_store_scene_activation_command_class.png} "Scene Activation data model" width=10cm
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
  version : Attribute Type = ATTRIBUTE(ACTIVATION_VERSION)
  version : value = Desired: [] - Reported: [1]

  object "Local Scenes" as local_scenes #FFFFFF
  local_scenes : Attribute Type = ATTRIBUTE(LOCAL_SCENES)
  local_scenes : value = Desired: [] - Reported: []

  object "Active Scene" as active_scene #FFFFFF
  active_scene : Attribute Type = ATTRIBUTE(ACTIVATION_ACTIVE_SCENE)
  active_scene : value = Desired: [2] - Reported: [1]

  object "Dimming Duration Setting" as dimming_duration_setting #FFFFFF
  dimming_duration_setting : Attribute Type = ATTRIBUTE(SETTING_DIMMING_DURATION)
  dimming_duration_setting : value = Desired: [0] - Reported: [24]
}

node *-- endpoint
endpoint *-- version
endpoint *-- local_scenes
local_scenes *-- active_scene
active_scene *-- dimming_duration_setting

@enduml
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_SCENE_ACTIVATION_CONTROL_H
#define ZWAVE_COMMAND_CLASS_SCENE_ACTIVATION_CONTROL_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the Scene Activation Command Class control handler.
 *
 * @return sl_status_t
 */
sl_status_t zwave_command_class_scene_activation_control_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_SCENE_ACTIVATION_CONTROL_H
/** @} end zwave_command_class_scene_activation_control */
