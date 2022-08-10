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
 * @defgroup zwave_command_class_meter_types Type definitions for attribute storage of the Meter Command Class
 * @ingroup zpc_attribute_store_command_classes_types
 * @brief Type definitions for the Meter Command Class.
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_METER_TYPES_H
#define ZWAVE_COMMAND_CLASS_METER_TYPES_H

#include <stdbool.h>
#include <stdint.h>

// Capabilities
typedef bool meter_reset_supported_t;
typedef uint8_t meter_supported_rate_types_t;

// Indices
typedef uint8_t meter_type_t;
typedef uint8_t meter_rate_type_t;
typedef uint16_t meter_scale_t;

// 4 bytes unsigned meter value
typedef int32_t meter_value_t;
// Delta time: number of seconds between measurements
typedef uint16_t meter_delta_time_t;

// Size and precision
typedef uint8_t meter_precision_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_METER_TYPES_H
/** @} end zwave_command_class_meter_types */
