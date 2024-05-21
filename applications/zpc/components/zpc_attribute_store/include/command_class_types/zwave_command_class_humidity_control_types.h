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
 * @defgroup zwave_command_class_humidity_control_mode_types Type definitions for attribute storage of the Humidity Control Command Class
 * @ingroup zpc_attribute_store_command_classes_types
 * @brief Type definitions for the Humidity Control Command Class (Mode, State and Setpoint).
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_TYPES_H
#define ZWAVE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_TYPES_H

#include <stdint.h>

//>> Humidity Control Mode CC
///> Humidity Control Mode. uint8_t
typedef uint8_t humidity_control_mode_t;
///> Humidity Control Suppoted Mode Bitmask. uint8_t
typedef uint8_t humidity_control_supported_modes_t;

//>> Humidity Control Operating State CC
///> Humidity Control Operating State. uint8_t
typedef uint8_t humidity_control_operating_state_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_TYPES_H
/** @} end zwave_command_class_humidity_control_mode_types */
