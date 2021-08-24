
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

#ifndef ZWAVE_COMMAND_CLASS_THERMOSTAT_SETPOINT_ATTRIBUTES_H
#define ZWAVE_COMMAND_CLASS_THERMOSTAT_SETPOINT_ATTRIBUTES_H

// Includes from other components
#include "sl_status.h"
#include "zwave_controller_types.h"
#include "attribute_store.h"

// Generic includes
#include <stdbool.h>

// Attribute definitions
// Thermostat Setpoint Command Class
#define COMMAND_CLASS_THERMOSTAT_SETPOINT 0x43

#define ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VERSION \
  ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_THERMOSTAT_SETPOINT)

#define ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SUPPORTED_SETPOINT_TYPES \
  ((attribute_store_type_t)((COMMAND_CLASS_THERMOSTAT_SETPOINT << 8) | 0x02))

#define ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_TYPE \
  ((attribute_store_type_t)((COMMAND_CLASS_THERMOSTAT_SETPOINT << 8) | 0x03))

#define ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VALUE \
  ((attribute_store_type_t)((COMMAND_CLASS_THERMOSTAT_SETPOINT << 8) | 0x04))

#define ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VALUE_SCALE \
  ((attribute_store_type_t)((COMMAND_CLASS_THERMOSTAT_SETPOINT << 8) | 0x05))

#define ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_MIN_VALUE \
  ((attribute_store_type_t)((COMMAND_CLASS_THERMOSTAT_SETPOINT << 8) | 0x06))

#define ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_MIN_VALUE_SCALE \
  ((attribute_store_type_t)((COMMAND_CLASS_THERMOSTAT_SETPOINT << 8) | 0x07))

#define ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_MAX_VALUE \
  ((attribute_store_type_t)((COMMAND_CLASS_THERMOSTAT_SETPOINT << 8) | 0x08))

#define ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_MAX_VALUE_SCALE \
  ((attribute_store_type_t)((COMMAND_CLASS_THERMOSTAT_SETPOINT << 8) | 0x09))

#endif  //ZWAVE_COMMAND_CLASS_THERMOSTAT_SETPOINT_ATTRIBUTES_H
