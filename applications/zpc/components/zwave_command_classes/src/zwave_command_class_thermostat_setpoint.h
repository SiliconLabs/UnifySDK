
/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef ZWAVE_COMMAND_CLASS_THERMOSTAT_SETPOINT_H
#define ZWAVE_COMMAND_CLASS_THERMOSTAT_SETPOINT_H

#include "sl_status.h"

// Command Class specific defines
// What we assume when we can't discover min/max capabilities.
#define CELSIUS_SCALE     0
#define FAHRENHEIT_SCALE  1
#define DEFAULT_MIN_VALUE 0
#define DEFAULT_MAX_VALUE 100
#define DEFAULT_SCALE     CELSIUS_SCALE

#define SETPOINT_TYPE_MASK 0x0F

#define SIZE_MASK      0x07
#define SCALE_MASK     0x18
#define PRECISION_MASK 0xE0

#define SET_DEFAULT_PRECISION (3 << 5)
#define SET_DEFAULT_SIZE      4

#define REPORT_SETPOINT_TYPE_INDEX 2

#define REPORT_PRECISION_SCALE_SIZE_INDEX 3
#define REPORT_VALUE_INDEX                4

#define CAPABILITIES_REPORT_SETPOINT_TYPE_INDEX            2
#define CAPABILITIES_REPORT_MIN_PRECISION_SCALE_SIZE_INDEX 3
#define CAPABILITIES_REPORT_MIN_VALUE_INDEX                4

#define SUPPORTED_REPORT_BITMASK_INDEX 2

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup thermostat_setpoint_command_class Thermostat Setpoint Command Class
 * @ingroup command_classes
 * @brief Thermostat Setpoint Command Class handlers and control functions
 *
 * This module takes care of the Thermostat Setpoint Command Class support (none)
 * and control (currently controlling version 3)
 *
 * This Command Class relies on the following Z-Wave Attribute structure:
 * \image html attribute_store_thermostat_setpoint_command_class.png "Attribute store Thermostat Setpoint Command Class Attributes"
 *
 * @{
 */

/**
 * @brief Initialize the Thermostat Setpoint Command Class control.
 *
 * @return Always true
 */
sl_status_t zwave_command_class_thermostat_setpoint_init();

/** @} end thermostat_setpoint_command_class */

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_THERMOSTAT_SETPOINT_H