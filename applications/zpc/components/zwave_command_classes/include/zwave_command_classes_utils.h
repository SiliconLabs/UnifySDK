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

#ifndef ZWAVE_COMMAND_CLASSES_UTILS_H
#define ZWAVE_COMMAND_CLASSES_UTILS_H

// Generic includes
#include <stdbool.h>

// UIC includes
#include "sl_status.h"
#include "zwave_controller_connection_info.h"
#include "zwave_generic_types.h"
#include "attribute_store.h"
#include "zwave_utils.h"

// ZPC includes
#include "zwave_controller_connection_info.h"

// Interfaces
#include "zwave_generic_types.h"

// Contiki timing includes
#include "clock.h"

/**
 * @defgroup command_classes_utils Command Classes utils
 * @ingroup command_classes
 * @brief Utils for Command Classes handlers
 *
 * This module provides some utilities helping with
 * functionalities relating to command classes
 *
 * @{
 */

///> Minimal Z-Wave Frame that can be sent (Command Class / Command).
typedef struct zwave_minimum_frame {
  uint8_t command_class;
  uint8_t command;
} zwave_minimum_frame_t;

///< Values that can be used for "Umbrella" attributes
typedef enum {
  FINAL_STATE             = 0,
  NEEDS_ONE_COMMAND       = 1,
  NEEDS_MULTIPLE_COMMANDS = 2,
} command_status_values_t;

// Helper macros
#define FAHRENHEIT_TO_DEGREES(value) ((value - 32.0F) * 5 / 9);
#define DEGREES_TO_FAHRENHEIT(value) (value * 9 / 5.0F) + 32;

// Constants
/// Additional delay in ms to wait before issuing a Get Command
/// after a node has finished a transition.
#define PROBE_BACK_OFF 500

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Find the Z-Wave Endpoint ID Node attribute based on a const
 * zwave_controller_connection_info_t object
 *
 * @param connection_info
 * @return attribute_store_node_t
 */
attribute_store_node_t zwave_command_class_get_endpoint_node(
  const zwave_controller_connection_info_t *connection_info);

/**
 * @brief Find the Z-Wave NodeID Node attribute based on a const
 * zwave_controller_connection_info_t object
 *
 * @param connection_info
 * @return attribute_store_node_t
 */
attribute_store_node_t zwave_command_class_get_node_id_node(
  const zwave_controller_connection_info_t *connection_info);

/**
 * @brief Gets the Attribute Store Endpoint ID node for a given
 *        Z-Wave NodeID / Endpoint ID in our network
 *
 * @param node_id
 * @param endpoint_id
 * @return attribute_store_node_t
 */
attribute_store_node_t
  zwave_command_class_get_endpoint_id_node(zwave_node_id_t node_id,
                                           zwave_endpoint_id_t endpoint_id);

/**
 * @brief function to send report frames with default tx options.
 *
 * @param connection_info Info about the connection properties of this frame.
 * @param report_size     The length of this frame.
 * @param report_data     The data payload of the report frame.
 *
 * @returns sl_status_t representing the outcome of sending the report frame.
 */
sl_status_t zwave_command_class_send_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint16_t report_size,
  const uint8_t *const report_data);

/**
 * @brief function helper function to help Command Classes decide if they should
 * create the supporting node attributes on Command Class version attribute update.
 *
 * @param command_class The Command Class that is to be verified
 * @param updated_node  Attribute Store node that was updated.
 *                      (it can be anything under an endpoint.)
 *
 * @returns true if the handler must not create attributes and filter the command
 * class. false if the handler must go ahead and create its attributes.
 */
bool is_zwave_command_class_filtered_for_root_device(
  zwave_command_class_t command_class, attribute_store_node_t updated_node);

/**
 * @brief function helper function that determines if a transmission was done
 * using the ZPC's highest security class
 *
 * @param connection     Pointer to Controller connection info to check against.
 *
 * @returns true if the using our highest security class, false otherwise
 */
bool is_using_zpc_highest_security_class(
  const zwave_controller_connection_info_t *connection);

/**
 * @brief Parse a value and pack it into a uint32
 *
 * @param size
 * @param precision
 * @param value
 * @return value * 1000
 */

int32_t command_class_get_int32_value(uint8_t size,
                                      uint8_t precision,
                                      const uint8_t *value);

/**
 * @brief Converts a clock_time_t duration to a Z-Wave Command Class duration
 * byte
 *
 * Refer to "Duration encoding" in the Application Command Class specifications
 * note that value 0xFE is interpreted as unkonwn here.
 *
 * @param time            The system time duration
 * @returns uint8_t       The corresponding Z-Wave duration encoding.
 */
uint8_t time_to_zwave_duration(clock_time_t time);

/**
 * @brief Converts a duration byte encoded for a Z-Wave command class and returns
 * its corresponding value in clock_time_t
 *
 * Refer to "Duration encoding" in the Application Command Class specifications
 * note that value 0xFE is interpreted as unkonwn here.
 *
 * @param zwave_duration  The value used in the Set / Report.
 * @returns clock_time_t  The corresponding system duration.
 */
clock_time_t zwave_duration_to_time(uint8_t zwave_duration);

/**
 * @brief Verifies if a Command Class is an actuator Command Class
 *
 * @param command_class The Command Class to verify
 *
 * @returns true if the Command Class is an actuator Command Class, false otherwise
 */
bool is_actuator_command_class(zwave_command_class_t command_class);

/**
 * @brief Provides the Role Type of a node
 *
 * @param node_id The node ID which Role Type is requested for
 *
 * @return the Role Type of a node
 */
zwave_role_type_t get_zwave_node_role_type(zwave_node_id_t node_id);

/**
 * @brief Verifies if a Node is a Portable End Node. (PS / PEN Role type)
 *
 * @param node  Any attribute store node under the NodeID node
 * @returns true if the Role type of the node is ROLE_TYPE_END_NODE_PORTABLE,
 *          false otherwise
 */
bool is_portable_end_node(attribute_store_node_t node);

#ifdef __cplusplus
}
#endif

/** @} end command_classes_utils */

#endif  // ZWAVE_COMMAND_CLASSES_UTILS_H
