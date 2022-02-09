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
 * @defgroup zwave_command_class_central_scene Central Scene Command Class
 * @ingroup command_classes
 * @brief Central Scene Command Class handlers and control functions
 *
 * This module provide the control of the Central Scene Command Class.
 *
 * The Central Scene scene number and key attribute are mapped into a "unique"
 * scene combination, called active scene.
 *
 * This active scene has a special value of 0 (NO_SCENE_ACTIVE), which means
 * that no scene is active. When a Central Scene notification is received
 * a scene will stay active for 5 seconds by default, or 60 seconds if slow refresh
 * is configured and we are receiving key held down key attribute.
 *
 * The attribute store model is as follows:
 *
@startuml{attribute_store_central_scene_command_class.png} "Central Scene data model" width=10cm
title Central Scene data model
allow_mixing
skinparam objectBorderColor black

legend top
<font color=#FEFEFE>ATTRIBUTE(type)</font> : ATTRIBUTE_COMMAND_CLASS_CENTRAL_SCENE_type
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

  object "Number of scenes" as number_of_scenes #FFFFFF
  number_of_scenes : Attribute Type = ATTRIBUTE(NUMBER_OF_SCENES)
  number_of_scenes : value = Desired: [] - Reported: [10]

  object "Max Number of Key attributes" as key_attributes #FFFFFF
  key_attributes : Attribute Type = ATTRIBUTE(MAX_KEY_ATTRIBUTE)
  key_attributes : value = Desired: [] - Reported: [25]

  object "Slow Refresh" as slow_refresh #FFFFFF
  slow_refresh : Attribute Type = ATTRIBUTE(SLOW_REFRESH)
  slow_refresh : value = Desired: [1], Reported: [0]

  object "Active Scene" as active_scene #FFFFFF
  active_scene : Attribute Type = ATTRIBUTE(ACTIVE_SCENE)
  active_scene : value = Desired: [], Reported: [0]

  object "Active Scene Sequence Number" as active_scene_sequence_number #FFFFFF
  active_scene_sequence_number : Attribute Type = ATTRIBUTE(ACTIVE_SCENE_SEQUENCE_NUMBER)
  active_scene_sequence_number : value = Desired: [], Reported: [2]
}

node *-- endpoint
endpoint *-- version
endpoint *-- number_of_scenes
endpoint *-- key_attributes
endpoint *-- slow_refresh
endpoint *-- active_scene
active_scene *-- active_scene_sequence_number

@enduml


 *
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_CENTRAL_SCENE_H
#define ZWAVE_COMMAND_CLASS_CENTRAL_SCENE_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief This function initialize the Central Scene Command Class handler
 *
 * @return SL_STATUS_OK on success, any other error code for an error.
 */
sl_status_t zwave_command_class_central_scene_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_CENTRAL_SCENE_H
/** @} end zwave_command_class_central_scene */
