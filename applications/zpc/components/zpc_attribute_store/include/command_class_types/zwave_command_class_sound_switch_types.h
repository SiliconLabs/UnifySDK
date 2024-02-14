/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup zpc_attribute_store_command_classes_types Type definitions for attribute storage of Command Classes
 * @ingroup zpc_attribute_store
 * @brief Type definitions for Command Classes, used for @ref attribute_store storage.
 *
 */

/**
 * @defgroup zwave_command_class_sound_switch_types Type definitions for attribute storage of the Sound Switch Command Class
 * @ingroup zpc_attribute_store_command_classes_types
 * @brief Type definitions for the Sound Switch Command Class.
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_SOUND_SWITCH_TYPES_H
#define ZWAVE_COMMAND_CLASS_SOUND_SWITCH_TYPES_H

#include <stdint.h>

///> Tone identifier. uint8_t
typedef uint8_t sound_switch_tone_id_t;
///> Tone duration. uint16_t
typedef uint16_t sound_switch_tone_duration_t;
///> Volume representation. uint8_t
typedef uint8_t sound_switch_volume_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_SOUND_SWITCH_TYPES_H
/** @} end zwave_command_class_sound_switch_types */
