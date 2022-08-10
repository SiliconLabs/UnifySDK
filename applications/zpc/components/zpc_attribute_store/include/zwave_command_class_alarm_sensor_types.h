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
 * @defgroup zwave_command_class_alarm_sensor_types Type definitions for attribute storage of the Alarm Sensor Command Class
 * @ingroup zpc_attribute_store_command_classes_types
 * @brief Type definitions for the Alarm Sensor Command Class.
 *
 * @{
 */

#ifndef ZWAVE_COMMAND_CLASS_ALARM_SENSOR_TYPES_H
#define ZWAVE_COMMAND_CLASS_ALARM_SENSOR_TYPES_H

typedef uint8_t alarm_sensor_type_t;
typedef uint8_t alarm_sensor_state_t;
typedef zwave_node_id_t alarm_sensor_id_t;
typedef uint16_t alarm_sensor_seconds_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_COMMAND_CLASS_ALARM_SENSOR_TYPES_H
/** @} end zwave_command_class_alarm_sensor_types */
