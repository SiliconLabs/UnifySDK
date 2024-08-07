/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup zwave_command_class_thermostat_setpoint_types Type definitions for attribute storage of the Thermostat SetPoint Command Class
 * @ingroup zpc_attribute_store_command_classes_types
 * @brief Type definitions for the Thermostat SetPoint Command Class.
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_THERMOSTAT_SETPOINT_TYPES_H
#define ZWAVE_COMMAND_CLASS_THERMOSTAT_SETPOINT_TYPES_H

#define SETPOINT_TYPE_HEATING 1
#define SETPOINT_TYPE_COOLING 2

typedef uint8_t thermostat_setpoint_type_t;
typedef int32_t thermostat_setpoint_value_t;
typedef uint8_t thermostat_setpoint_scale_t;
typedef uint8_t thermostat_setpoint_precision_t;


#endif  //ZWAVE_COMMAND_CLASS_THERMOSTAT_SETPOINT_TYPES_H
/** @} end zwave_command_class_thermostat_setpoint_types */
