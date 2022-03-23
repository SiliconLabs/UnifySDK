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
 * @defgroup zwave_command_class_configuration_types Type definitions for attribute storage of the Configuration Command Class
 * @ingroup zpc_attribute_store_command_classes_types
 * @brief Type definitions for the Configuration Command Class.
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_CONFIGURATION_TYPES_H
#define ZWAVE_COMMAND_CLASS_CONFIGURATION_TYPES_H

#include <stdbool.h>
#include <stdint.h>

// Configuration parameter number
typedef uint16_t configuration_parameter_id_t;
// Configuration parameter value
typedef int64_t configuration_parameter_value_t;
// Configuration bulk support.
typedef bool configuration_bulk_support_t;
// Configuration parameter flags (read-only, advanced, etc.)
typedef bool configuration_parameter_flag_t;
// Configuration parameter size (1, 2 or 4 bytes)
typedef uint8_t configuration_parameter_size_t;
// Configuration parameter format (enum)
typedef uint8_t configuration_parameter_format_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_CONFIGURATION_TYPES_H
/** @} end zwave_command_class_configuration_types */
