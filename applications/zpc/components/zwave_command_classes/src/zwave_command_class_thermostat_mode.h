
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

#ifndef ZWAVE_COMMAND_CLASS_THERMOSTAT_MODE_H
#define ZWAVE_COMMAND_CLASS_THERMOSTAT_MODE_H

#include "sl_status.h"

#define THERMOSTAT_MODE_REPORT_MODE_INDEX 2
#define THERMOSTAT_MODE_SUPPORTED_REPORT_BITMASK_INDEX 2

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup thermostat_mode_command_class Thermostat Mode Command Class
 * @ingroup command_classes
 * @brief Thermostat Mode Command Class handlers and control functions
 *
 * This module takes care of the Thermostat Mode Command Class support (none)
 * and control (currently controlling version 3)
 * 
 * @{
 */

/**
 * @brief Initialize the Thermostat Mode Command Class control.
 * 
 * @return Always true
 */
sl_status_t zwave_command_class_thermostat_mode_init();

/** @} end thermostat_mode_command_class */

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_THERMOSTAT_MODE_H