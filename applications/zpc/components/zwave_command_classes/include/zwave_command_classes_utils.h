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
#include <string.h>

// Unify includes
#include "sl_status.h"
#include "zwave_controller_connection_info.h"
#include "zwave_generic_types.h"
#include "attribute_store.h"
#include "zwave_utils.h"

// ZPC includes
#include "zwave_controller_connection_info.h"
#include "zwave_command_class_generic_types.h"

// Interfaces
#include "zwave_generic_types.h"

// Contiki timing includes
#include "clock.h"

// Value to set in a 8-bits reserved field
#define RESERVED_BYTE 0x00

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

// Helper macros
#define FAHRENHEIT_TO_DEGREES(value) ((value - 32.0) * 5 / 9);
#define DEGREES_TO_FAHRENHEIT(value) (value * 9 / 5.0) + 32;

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
 * @brief Find the version of the command class by navigating up to the
 * endpoint from a given attribute store node.
 *
 * @param node            Attribute Store node under the endpoint
 * @param command_class   Command class for which we want the version
 * @return zwave_cc_version_t
 */
zwave_cc_version_t zwave_command_class_get_version_from_node(
  attribute_store_node_t node, zwave_command_class_t command_class);

/**
 * @brief Verifies if a node has some Reports to Follow.
 *
 * @param node  Attribute Store node under which we want to check if there
 *              are some reports to follow.
 * @returns true if there is at least 1 report to follow. False otherwise
 */
bool has_reports_to_follow(attribute_store_node_t node);

/**
 * @brief Gets the number of Reports to Follow for a node.
 *
 * @param node  Attribute Store node under which we want to check if there
 *              are some reports to follow.
 * @returns A value if there is at least 1 report to follow. 0 otherwise
 */
reports_to_follow_t get_reports_to_follow(attribute_store_node_t node);

/**
 * @brief Places a ATTRIBUTE_REPORTS_TO_FOLLOW attribute under an attribute if
 * it does not exist and set it to the requested reports_to_follow value
 *
 * @param node                  Attribute Store node under which to set a
 *                              reports to follow value.
 * @param reports_to_follow     The value to set in the
 *                              ATTRIBUTE_REPORTS_TO_FOLLOW attribute
 * @returns sl_status_t
 */
sl_status_t set_reports_to_follow(attribute_store_node_t node,
                                  reports_to_follow_t reports_to_follow);

/**
 * @brief Set the desired and reported value of a command_status_t type of node
 *
 * @param command_status_node     Attribute Store node to set
 * @param reported                Reported value to set.
 * @param desired                 Desired value to set.
 */
void set_command_status_value(attribute_store_node_t command_status_node,
                              command_status_values_t reported,
                              command_status_values_t desired);

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
 * @brief Extracts a 1,2 or 4 bytes signed value from a frame
 *
 * @param [in]  frame   Pointer to the frame data where the value is stored
 * @param [in]  size    Size of the value (1, 2 or 4 bytes)
 *
 * @returns Parsed value at the frame pointer.
 */
int32_t get_signed_value_from_frame_and_size(const uint8_t *frame,
                                             uint8_t size);

/**
 * @brief Extracts a 1,2 or 4 bytes unsigned value from a frame
 *
 * @param [in]  frame   Pointer to the frame data where the value is stored
 * @param [in]  size    Size of the value (1, 2 or 4 bytes)
 *
 * @returns Parsed value at the frame pointer.
 */
uint32_t get_unsigned_value_from_frame_and_size(const uint8_t *frame,
                                                uint8_t size);

/**
 * @brief Convert a value from the Z-Wave world (precision = [0..7] and C° + F) into a UCL (Zigbee) world (precision = 2 and C°)
 * 
 * @param zwave_value     Current Z-Wave value
 * @param zwave_precision Reported Z-Wave precision
 * @param zwave_scale     Reported Z-Wave scale (0 : C°, 1 : F)
 * 
 * @return int16_t UCL temperature. Rounded down if Z-Wave precision is too high.
 */
int16_t zwave_temperature_to_ucl_temperature(int32_t zwave_value,
                                             uint8_t zwave_precision,
                                             uint8_t zwave_scale);

/**
 * @brief Convert a value from the UCL world (Zigbee) (precision = 2 and C°) to the ZWave world (precision = [0..7] and C° + F)
 * 
 * @param ucl_value       Current UCL value
 * @param zwave_precision Expected Z-Wave precision
 * @param zwave_scale     Expected Z-Wave scale (0 : C°, 1 : F)
 * 
 * @return int32_t Z-Wave temperature with given precision and scale.
 */
int32_t ucl_temperature_to_zwave_temperature(int16_t ucl_value,
                                             uint8_t zwave_precision,
                                             uint8_t zwave_scale);
  /**
 * @brief Converts a clock_time_t duration to a Z-Wave Command Class duration
 * byte
 *
 * Refer to "Duration encoding" in the Application Command Class specifications
 * note that value 0xFE is interpreted as unknown here.
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
 * note that value 0xFE is interpreted as unknown here.
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
