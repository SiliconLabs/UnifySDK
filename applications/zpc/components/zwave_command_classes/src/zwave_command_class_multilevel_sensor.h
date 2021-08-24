
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

#ifndef ZWAVE_COMMAND_CLASS_MULTILEVEL_SENSOR_H
#define ZWAVE_COMMAND_CLASS_MULTILEVEL_SENSOR_H

#include "sl_status.h"

// Command Cass specific defines
#define SUPPORTED_REPORT_BITMASK_INDEX 2

#define SUPPORTED_SCALE_REPORT_SENSOR_TYPE_INDEX 2
#define SUPPORTED_SCALE_REPORT_BITMASK_INDEX     3

#define REPORT_SENSOR_TYPE_INDEX  2
#define REPORT_PROPERTIES_INDEX   3
#define REPORT_SENSOR_VALUE_INDEX 4
#define REPORT_SIZE_BITMASK       0x07
#define REPORT_SCALE_BITMASK      0x18
#define REPORT_PRECISION_BITMASK  0xE0

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup multilevel_sensor_command_class Multilevel Sensor Command Class
 * @ingroup command_classes
 * @brief Multilevel Sensor Command Class handlers and control functions
 *
 * This module takes care of Multilevel Sensor Command Class support (none)
 * and control (currently controlling version 11)
 *
 * This command class relies on the following Z-Wave Attribute structure:
 * \image html attribute_store_multilevel_sensor_command_class.png "Attribute store Multilevel Sensor Command Class Attributes"
 *
 * @{
 */

/**
 * @brief Initialize the Multilevel Sensor Command Class control
 *
 * @return Always true
 */
sl_status_t zwave_command_class_multilevel_sensor_init();

/** @} end multilevel_sensor_command_class */

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_MULTILEVEL_SENSOR_H