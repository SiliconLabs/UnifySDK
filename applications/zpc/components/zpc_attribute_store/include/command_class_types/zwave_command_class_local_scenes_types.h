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
 * @defgroup zwave_command_class_local_scenes_types Type definitions for attribute storage of the Local Scenes Command Classes
 * @ingroup zpc_attribute_store_command_classes_types
 * @brief Type definitions for the Local Scenes Command Classes
 * (Scene Activation, Scene Actuator Configuration, Scene Controller Configuration)
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_LOCAL_SCENES_TYPES_H
#define ZWAVE_COMMAND_CLASS_LOCAL_SCENES_TYPES_H

#include <stdbool.h>
#include <stdint.h>

// Scene Identifiers.
typedef uint8_t local_scene_id_t;

/// Actuator Scene settings
// Dimming duration
typedef uint8_t local_scene_dimming_duration_t;
// Level
typedef uint8_t local_scene_level_t;
// Override
typedef uint8_t local_scene_override_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_LOCAL_SCENES_TYPES_H
/** @} end zwave_command_class_local_scenes_types */
