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
 * @defgroup zwave_command_class_thermostat_fan_mode_types Type definitions for attribute storage of the Sound Switch Command Class
 * @ingroup zpc_attribute_store_command_classes_types
 * @brief Type definitions for the Sound Switch Command Class.
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_TYPES_H
#define ZWAVE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_TYPES_H

#include <stdint.h>


// Fan Mode CC
///> Off flag (version >= 2). uint8_t
typedef uint8_t thermostat_fan_mode_off_flag_t;
///> Fan mode. uint8_t
typedef uint8_t thermostat_fan_mode_t;
///> Supported mode bitmask. uint32_t
typedef uint32_t thermostat_fan_supported_modes_t;

// Fan State CC
///> Fan state. uint8_t
typedef uint8_t thermostat_fan_state_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_TYPES_H
/** @} end zwave_command_class_thermostat_fan_mode_types */
