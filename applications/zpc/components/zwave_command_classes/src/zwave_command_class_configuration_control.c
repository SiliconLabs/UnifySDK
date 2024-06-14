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
//Includes from this component
#include "zwave_command_class_configuration.h"
#include "zwave_command_classes_utils.h"

// Interfaces
#include "zwave_command_class_configuration_types.h"
#include "attribute_store_defined_attribute_types.h"

// Includes from other ZPC Components
#include "zwave_command_class_indices.h"
#include "zwave_command_handler.h"
#include "zwave_controller_connection_info.h"
#include "zwave_controller_utils.h"
#include "zpc_attribute_store_network_helper.h"
#include "zpc_attribute_resolver.h"

// Includes from Unify Components
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_resolver.h"

// Generic includes
#include <string.h>

#include "sl_log.h"
#define LOG_TAG "zwave_command_class_configuration"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_##type

// Configuration default Attributes. For all versions.
static const attribute_store_type_t default_attributes[]
  = {ATTRIBUTE(NEXT_SUPPORTED_PARAMETER_ID), ATTRIBUTE(BULK_SUPPORT)};

// List of attributes for each configuration parameter
static const attribute_store_type_t configuration_parameter_attributes[]
  = {ATTRIBUTE(PARAMETER_SIZE),
     ATTRIBUTE(PARAMETER_FORMAT),
     ATTRIBUTE(PARAMETER_NAME),
     ATTRIBUTE(PARAMETER_INFO),
     ATTRIBUTE(PARAMETER_VALUE),
     ATTRIBUTE(PARAMETER_MIN_VALUE),
     ATTRIBUTE(PARAMETER_MAX_VALUE),
     ATTRIBUTE(PARAMETER_DEFAULT_VALUE),
     ATTRIBUTE(PARAMETER_ADVANCED),
     ATTRIBUTE(PARAMETER_READ_ONLY),
     ATTRIBUTE(PARAMETER_ALTERING_CAPABILITIES)};

///////////////////////////////////////////////////////////////////////////////
// Frame parsing defines
///////////////////////////////////////////////////////////////////////////////
#define CONFIGURATION_SIZE_MASK                     0x7
#define CONFIGURATION_FORMAT_MASK                   0x38
#define CONFIGURATION_READ_ONLY_MASK                0x40
#define CONFIGURATION_ALTERING_CAPABILITIES_MASK    0x80
#define CONFIGURATION_ADVANCED_MASK                 0x01
#define CONFIGURATION_NO_BULK_SUPPORT_MASK          0x02
#define CONFIGURATION_SET_VALUE_INDEX               4
#define CONFIGURATION_REPORT_VALUE_INDEX            4
#define CONFIGURATION_BULK_SET_VALUE_INDEX          6
#define CONFIGURATION_BULK_REPORT_FIRST_VALUE_INDEX 7

#define CONFIGURATION_PROPERTIES_REPORT_FIRST_VALUE_INDEX 5

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Checks if the node we are talking to supports Bulk Commands
 *
 * @param configuration_attribute_node    Attribute store node attribute under
 *                                        the Endpoint ID
 * @return true if the node supports Bulk, false otherwise or if we don't know.
 */
static bool
  is_bulk_supported(attribute_store_node_t configuration_attribute_node)
{
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(configuration_attribute_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  attribute_store_node_t bulk_support_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(BULK_SUPPORT));

  configuration_parameter_flag_t bulk_support = false;
  attribute_store_get_reported(bulk_support_node,
                               &bulk_support,
                               sizeof(bulk_support));

  return bulk_support;
}

/**
 * @brief Verifies if a configuration parameter is read-only
 *
 * @param configuration_attribute_node  Configuration Parameter node,
 *                                      located under ATTRIBUTE(PARAMETER_ID)
 * @returns true if it is read-only, false if not or if we do not know.
 */
static bool
  is_parameter_read_only(attribute_store_node_t configuration_attribute_node)
{
  attribute_store_node_t parameter_id_node
    = attribute_store_get_first_parent_with_type(configuration_attribute_node,
                                                 ATTRIBUTE(PARAMETER_ID));

  bool read_only = false;
  attribute_store_node_t read_only_node
    = attribute_store_get_first_child_by_type(parameter_id_node,
                                              ATTRIBUTE(PARAMETER_READ_ONLY));
  attribute_store_get_reported(read_only_node, &read_only, sizeof(read_only));
  return read_only;
}

/**
 * @brief Fetch or create a configuration parameter with all its sub-attributes
 *        in the attribute store
 *
 * @param endpoint_node    Attribute store node for the Endpoint ID
 * @param parameter_id     Configuration Parameter ID to create in the Attribute
 *                         Store
 * @returns The created (or exisiting) attribute store node for the configuration
 * parameter ID
 */
static attribute_store_node_t
  create_configuration_parameter(attribute_store_node_t endpoint_node,
                                 configuration_parameter_id_t parameter_id)
{
  attribute_store_node_t parameter_id_node
    = attribute_store_emplace(endpoint_node,
                              ATTRIBUTE(PARAMETER_ID),
                              &parameter_id,
                              sizeof(parameter_id));

  return parameter_id_node;
}

/**
 * @brief Get the configuration parameter size for a given parameter id.
 *
 * @param parameter_id_node     Attribute Store Node for the Parameter ID.
 * @return configuration_parameter_size_t, 0 if the size is unknown.
 */
static configuration_parameter_size_t
  get_configuration_parameter_size(attribute_store_node_t parameter_id_node)
{
  attribute_store_node_t parameter_size_node
    = attribute_store_get_first_child_by_type(parameter_id_node,
                                              ATTRIBUTE(PARAMETER_SIZE));

  configuration_parameter_size_t size = 0;
  attribute_store_get_reported(parameter_size_node, &size, sizeof(size));
  return size;
}

/**
 * @brief Get the configuration parameter value for a given parameter id.
 *
 * @param parameter_id_node     Attribute Store Node for the Parameter ID.
 * @return configuration_parameter_value_t, 0 if the value is unknown.
 */
static configuration_parameter_value_t
  get_configuration_parameter_value(attribute_store_node_t parameter_id_node)
{
  attribute_store_node_t parameter_value_node
    = attribute_store_get_first_child_by_type(parameter_id_node,
                                              ATTRIBUTE(PARAMETER_VALUE));

  configuration_parameter_value_t value = 0;
  attribute_store_get_reported(parameter_value_node, &value, sizeof(value));
  return value;
}

/**
 * @brief Get the configuration parameter format for a given parameter id.
 *
 * @param parameter_id_node     Attribute Store Node for the Parameter ID.
 * @return configuration_parameter_format_t, 0 if the format is unknown (default).
 */
static configuration_parameter_format_t
  get_configuration_parameter_format(attribute_store_node_t parameter_id_node)
{
  attribute_store_node_t parameter_format_node
    = attribute_store_get_first_child_by_type(parameter_id_node,
                                              ATTRIBUTE(PARAMETER_FORMAT));

  configuration_parameter_format_t format = 0;
  attribute_store_get_reported(parameter_format_node, &format, sizeof(format));
  return format;
}

/**
 * @brief Set the parameter value value in the frame buffer based on the parameter
 * size, MSB first.
 *
 * @param frame     "Slice" of the received frame starting where the value is to be written
 * @param value     Configuration parameter value to write in the frame
 * @param size      Configuration parameter size
 * @param format    Configuration parameter format
 */
static void
  set_parameter_value_in_buffer(uint8_t *frame,
                                configuration_parameter_value_t value,
                                configuration_parameter_size_t size,
                                configuration_parameter_format_t format)
{
  if (format != PARAMETER_FORMAT_SIGNED_INTEGER) {
    // We stored the value as signed by default, if not signed format, convert to unsigned.
    uint32_t new_value = (uint32_t)value;
    for (uint8_t i = 0; i < size; i++) {
      frame[i] = (new_value >> (8 * (size - 1 - i))) & 0xFF;
    }
  } else {
    int32_t new_value = (int32_t)value;
    for (uint8_t i = 0; i < size; i++) {
      frame[i] = (new_value >> (8 * (size - 1 - i))) & 0xFF;
    }
  }
}

static void save_parameter_value(const uint8_t *frame,
                                 attribute_store_node_t parameter_id_node,
                                 attribute_store_type_t value_type)
{
  configuration_parameter_size_t size
    = get_configuration_parameter_size(parameter_id_node);

  configuration_parameter_format_t format
    = get_configuration_parameter_format(parameter_id_node);

  configuration_parameter_value_t value = 0;
  if (format != PARAMETER_FORMAT_SIGNED_INTEGER) {
    // We stored the value as signed by default, if not signed format, convert to unsigned.
    value = get_unsigned_value_from_frame_and_size(frame, size);
  } else {
    value = get_signed_value_from_frame_and_size(frame, size);
  }

  // Store in the attribute store.
  attribute_store_set_child_reported(parameter_id_node,
                                     value_type,
                                     &value,
                                     sizeof(value));
}

/**
 * @brief Sets the default parameters properties for a parameter in case
 *        a node does not support Properties Get Commands.
 *
 * @param parameter_id_node     Attribute Store Node for the Parameter ID.
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t
  set_default_parameter_properties(attribute_store_node_t parameter_id_node)
{
  sl_log_debug(LOG_TAG,
               "Setting default properties for parameter ID node %d",
               parameter_id_node);
  // default is not read-only:
  const configuration_parameter_flag_t flag_value = false;
  attribute_store_set_child_reported_only_if_undefined(
    parameter_id_node,
    ATTRIBUTE(PARAMETER_READ_ONLY),
    &flag_value,
    sizeof(flag_value));

  // Not advanced
  attribute_store_set_child_reported_only_if_undefined(
    parameter_id_node,
    ATTRIBUTE(PARAMETER_ADVANCED),
    &flag_value,
    sizeof(flag_value));

  // Altering capabilties
  attribute_store_set_child_reported_only_if_undefined(
    parameter_id_node,
    ATTRIBUTE(PARAMETER_ALTERING_CAPABILITIES),
    &flag_value,
    sizeof(flag_value));
  // Default format: Signed integer
  const configuration_parameter_format_t default_format
    = PARAMETER_FORMAT_SIGNED_INTEGER;
  attribute_store_set_child_reported_only_if_undefined(
    parameter_id_node,
    ATTRIBUTE(PARAMETER_FORMAT),
    &default_format,
    sizeof(default_format));

  // Next stop: min/default/max. we need to know the size for this.
  configuration_parameter_size_t size
    = get_configuration_parameter_size(parameter_id_node);
  if (size == 0) {
    // We need the parameter size to be resolved first.
    return SL_STATUS_IS_WAITING;
  }

  // Set the default to the current setting:
  configuration_parameter_value_t value
    = get_configuration_parameter_value(parameter_id_node);
  attribute_store_set_child_reported_only_if_undefined(
    parameter_id_node,
    ATTRIBUTE(PARAMETER_DEFAULT_VALUE),
    &value,
    sizeof(value));

  // set the Minimum depending on the size.
  if (size == 1) {
    value = INT8_MIN;
  } else if (size == 2) {
    value = INT16_MIN;
  } else {
    value = INT32_MIN;
  }
  attribute_store_set_child_reported_only_if_undefined(
    parameter_id_node,
    ATTRIBUTE(PARAMETER_MIN_VALUE),
    &value,
    sizeof(value));

  // set the Maximum depending on the size.
  if (size == 1) {
    value = INT8_MAX;
  } else if (size == 2) {
    value = INT16_MAX;
  } else {
    value = INT32_MAX;
  }
  attribute_store_set_child_reported_only_if_undefined(
    parameter_id_node,
    ATTRIBUTE(PARAMETER_MAX_VALUE),
    &value,
    sizeof(value));

  return SL_STATUS_ALREADY_EXISTS;
}

/**
 * @brief Set the all Configuration parameters to their default value.
 *
 * @param node Attribute store node under the Endpoint for which
 * all parameters have to be set to default.
 */
static void
  set_all_parameters_to_their_default_value(attribute_store_node_t node)
{
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);

  size_t child_index = 0;
  attribute_store_node_t parameter_id_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(PARAMETER_ID),
                                             child_index);

  while (parameter_id_node != ATTRIBUTE_STORE_INVALID_NODE) {
    attribute_store_node_t value_node
      = attribute_store_get_first_child_by_type(parameter_id_node,
                                                ATTRIBUTE(PARAMETER_VALUE));
    attribute_store_node_t default_value_node
      = attribute_store_get_first_child_by_type(
        parameter_id_node,
        ATTRIBUTE(PARAMETER_DEFAULT_VALUE));

    // Do not update if it is read-only
    if (false == is_parameter_read_only(value_node)) {
      attribute_store_undefine_desired(value_node);
      attribute_store_copy_value(default_value_node,
                                 value_node,
                                 REPORTED_ATTRIBUTE);
    }

    // Increment to the next parameter ID.
    child_index += 1;
    parameter_id_node
      = attribute_store_get_node_child_by_type(endpoint_node,
                                               ATTRIBUTE(PARAMETER_ID),
                                               child_index);
  }
}

/**
 * @brief Undefine the all Configuration parameters values, so they get resolved
 * again.
 *
 * @param node Attribute store node under the Endpoint for which
 * all parameters have to be undefined.
 */
static void undefine_all_parameter_values(attribute_store_node_t node)
{
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);

  size_t child_index = 0;
  attribute_store_node_t parameter_id_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(PARAMETER_ID),
                                             child_index);

  while (parameter_id_node != ATTRIBUTE_STORE_INVALID_NODE) {
    attribute_store_node_t value_node
      = attribute_store_get_first_child_by_type(parameter_id_node,
                                                ATTRIBUTE(PARAMETER_VALUE));
    attribute_store_undefine_desired(value_node);
    attribute_store_undefine_reported(value_node);
    // Increment to the next parameter ID.
    child_index += 1;
    parameter_id_node
      = attribute_store_get_node_child_by_type(endpoint_node,
                                               ATTRIBUTE(PARAMETER_ID),
                                               child_index);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Attribute store navigation private helper functions
///////////////////////////////////////////////////////////////////////////////
static attribute_store_node_t
  get_parameter_id_node(zwave_node_id_t node_id,
                        zwave_endpoint_id_t endpoint_id,
                        configuration_parameter_id_t parameter_id)
{
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_id_node(node_id, endpoint_id);
  return attribute_store_get_node_child_by_value(endpoint_node,
                                                 ATTRIBUTE(PARAMETER_ID),
                                                 REPORTED_ATTRIBUTE,
                                                 (uint8_t *)&parameter_id,
                                                 sizeof(parameter_id),
                                                 0);
}

///////////////////////////////////////////////////////////////////////////////
// Command Handler functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Handle incoming Configuration Report Commands
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t zwave_command_class_configuration_handle_report_command(
  const zwave_controller_connection_info_t *info,
  const uint8_t *frame,
  uint16_t frame_length)
{
  if (frame_length < 4) {
    return SL_STATUS_OK;
  }

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(info);

  // Fetch the parameter ID from the frame:
  configuration_parameter_id_t parameter_id = frame[2];
  attribute_store_node_t parameter_id_node
    = create_configuration_parameter(endpoint_node, parameter_id);

  configuration_parameter_size_t parameter_size
    = frame[3] & CONFIGURATION_SIZE_MASK;

  // If node is v1-v2, make sure to set back the next parameter id to 0.
  zwave_cc_version_t supporting_node_version
    = zwave_command_class_get_version_from_node(parameter_id_node,
                                                COMMAND_CLASS_CONFIGURATION_V4);
  if (supporting_node_version < 3) {
    configuration_parameter_id_t next_id = 0;
    attribute_store_set_child_reported(endpoint_node,
                                       ATTRIBUTE(NEXT_SUPPORTED_PARAMETER_ID),
                                       &next_id,
                                       sizeof(next_id));
  }

  // If we found a parameter size = 0, it means the parameter ID does not exist.
  if (parameter_size == 0) {
    attribute_store_delete_node(parameter_id_node);
    sl_log_debug(LOG_TAG,
                 "NodeID %d:%d reports that configuration "
                 "parameter %d is not supported. "
                 "Erasing from the Attribute Store",
                 info->remote.node_id,
                 info->remote.endpoint_id,
                 parameter_id);
    return SL_STATUS_OK;
  }

  // Save the size
  attribute_store_set_child_reported(parameter_id_node,
                                     ATTRIBUTE(PARAMETER_SIZE),
                                     &parameter_size,
                                     sizeof(parameter_size));

  // Save the value
  if (frame_length >= (parameter_size + 4)) {
    save_parameter_value(&frame[CONFIGURATION_REPORT_VALUE_INDEX],
                         parameter_id_node,
                         ATTRIBUTE(PARAMETER_VALUE));
  } else {
    sl_log_debug(LOG_TAG,
                 "NodeID %d:%d reports too small configuration "
                 "value in Configuration Report for parameter %d. "
                 "Ignoring",
                 info->remote.node_id,
                 info->remote.endpoint_id,
                 parameter_id);
  }

  return SL_STATUS_OK;
}

/**
 * @brief Handle incoming Configuration Bulk Report Commands
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t zwave_command_class_configuration_handle_bulk_report_command(
  const zwave_controller_connection_info_t *info,
  const uint8_t *frame,
  uint16_t frame_length)
{
  if (frame_length < 7) {
    return SL_STATUS_OK;
  }
  uint8_t number_of_parameters = frame[4];
  if (number_of_parameters == 0) {
    return SL_STATUS_OK;
  }

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_id_node(info->remote.node_id,
                                               info->remote.endpoint_id);

  reports_to_follow_t reports = frame[5];
  if (reports > 0) {
    zwave_tx_set_expected_frames(info->remote.node_id, reports);
  }

  // Get the size
  configuration_parameter_size_t parameter_size
    = frame[6] & CONFIGURATION_SIZE_MASK;

  // Fetch the first parameter ID from the frame:
  configuration_parameter_id_t parameter_id
    = (configuration_parameter_id_t)((frame[2] << 8) | frame[3]);

  // Size 0, parameters are invalid.
  if (parameter_size == 0) {
    for (uint8_t i = 0; i < number_of_parameters; i++) {
      attribute_store_node_t parameter_id_node
        = create_configuration_parameter(endpoint_node, parameter_id + i);
      attribute_store_delete_node(parameter_id_node);
      sl_log_debug(LOG_TAG,
                   "NodeID %d:%d reports that configuration "
                   "parameter %d is not supported. "
                   "Erasing from the Attribute Store",
                   info->remote.node_id,
                   info->remote.endpoint_id,
                   parameter_id + i);
    }
    return SL_STATUS_OK;
  }

  // Else, let's just save those parameters
  for (uint8_t i = 0; i < number_of_parameters; i++) {
    configuration_parameter_id_t current_parameter_id = parameter_id + i;
    attribute_store_node_t parameter_id_node
      = create_configuration_parameter(endpoint_node, current_parameter_id);
    // Save the size
    attribute_store_set_child_reported(parameter_id_node,
                                       ATTRIBUTE(PARAMETER_SIZE),
                                       &parameter_size,
                                       sizeof(parameter_size));

    if (frame_length >= ((i + 1) * parameter_size
                         + CONFIGURATION_BULK_REPORT_FIRST_VALUE_INDEX)) {
      save_parameter_value(
        &frame[(i * parameter_size)
               + CONFIGURATION_BULK_REPORT_FIRST_VALUE_INDEX],
        parameter_id_node,
        ATTRIBUTE(PARAMETER_VALUE));
    } else {
      sl_log_debug(LOG_TAG,
                   "NodeID %d:%d reports too small configuration "
                   "value in Configuration Bulk Report for parameter %d. "
                   "Ignoring value.",
                   info->remote.node_id,
                   info->remote.endpoint_id,
                   current_parameter_id);
    }
  }

  return SL_STATUS_OK;
}

/**
 * @brief Handle incoming Configuration Properties Report Commands
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t
  zwave_command_class_configuration_handle_properties_report_command(
    const zwave_controller_connection_info_t *info,
    const uint8_t *frame,
    uint16_t frame_length)
{
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_id_node(info->remote.node_id,
                                               info->remote.endpoint_id);

  configuration_parameter_id_t next_id = 0;
  attribute_store_node_t next_id_node = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE(NEXT_SUPPORTED_PARAMETER_ID));
  if (frame_length < 5) {
    // Make sure to stop trying to do Properties Get:
    attribute_store_set_reported(next_id_node, &next_id, sizeof(next_id));
    return SL_STATUS_OK;
  }

  configuration_parameter_id_t parameter_id
    = (configuration_parameter_id_t)((frame[2] << 8) | frame[3]);
  configuration_parameter_size_t parameter_size
    = frame[4] & CONFIGURATION_SIZE_MASK;
  configuration_parameter_format_t parameter_format
    = (frame[4] & CONFIGURATION_FORMAT_MASK) >> 3;

  attribute_store_node_t parameter_id_node
    = create_configuration_parameter(endpoint_node, parameter_id);

  // Case of parameter size = 0, make sure we erase the curent parameter and save
  // what the next parameter is.
  if (parameter_size == 0) {
    sl_log_debug(LOG_TAG,
                 "NodeID %d:%d reports that configuration "
                 "parameter %d is not supported. "
                 "Erasing from the Attribute Store",
                 info->remote.node_id,
                 info->remote.endpoint_id,
                 parameter_id);
    attribute_store_delete_node(parameter_id_node);
  }

  // If there is a size, save all the information of the configuration parameter.
  // Save the format:
  attribute_store_set_child_reported(parameter_id_node,
                                     ATTRIBUTE(PARAMETER_FORMAT),
                                     &parameter_format,
                                     sizeof(parameter_format));
  // Save the size:
  attribute_store_set_child_reported(parameter_id_node,
                                     ATTRIBUTE(PARAMETER_SIZE),
                                     &parameter_size,
                                     sizeof(parameter_size));

  // Save the read-only flag:
  configuration_parameter_flag_t read_only
    = (frame[4] & CONFIGURATION_READ_ONLY_MASK) >> 6;
  attribute_store_set_child_reported(parameter_id_node,
                                     ATTRIBUTE(PARAMETER_READ_ONLY),
                                     &read_only,
                                     sizeof(read_only));

  // Save the altering capabilities
  configuration_parameter_flag_t altering_capabilities
    = (frame[4] & CONFIGURATION_ALTERING_CAPABILITIES_MASK) >> 7;
  attribute_store_set_child_reported(parameter_id_node,
                                     ATTRIBUTE(PARAMETER_ALTERING_CAPABILITIES),
                                     &altering_capabilities,
                                     sizeof(altering_capabilities));

  // Min value:
  uint8_t current_index = CONFIGURATION_PROPERTIES_REPORT_FIRST_VALUE_INDEX;
  if (frame_length >= (current_index + parameter_size)) {
    save_parameter_value(&frame[current_index],
                         parameter_id_node,
                         ATTRIBUTE(PARAMETER_MIN_VALUE));
  }

  // Max value:
  current_index
    = CONFIGURATION_PROPERTIES_REPORT_FIRST_VALUE_INDEX + parameter_size;
  if (frame_length >= (current_index + parameter_size)) {
    save_parameter_value(&frame[current_index],
                         parameter_id_node,
                         ATTRIBUTE(PARAMETER_MAX_VALUE));
  }

  // Default value
  if (frame_length >= (current_index + parameter_size)) {
    current_index = CONFIGURATION_PROPERTIES_REPORT_FIRST_VALUE_INDEX
                    + (2 * parameter_size);
    save_parameter_value(&frame[current_index],
                         parameter_id_node,
                         ATTRIBUTE(PARAMETER_DEFAULT_VALUE));
  }

  // Look for the Next parameter ID.
  current_index
    = CONFIGURATION_PROPERTIES_REPORT_FIRST_VALUE_INDEX + (3 * parameter_size);
  if (frame_length >= (current_index + 2)) {
    next_id = (configuration_parameter_id_t)((frame[current_index] << 8)
                                             | frame[current_index + 1]);
    if (next_id == parameter_id) {
      sl_log_debug(LOG_TAG,
                   "NodeID %d:%d reports that next parameter number %d"
                   "equals the current one %d, ignoring.",
                   info->remote.node_id,
                   info->remote.endpoint_id,
                   next_id,
                   parameter_id);
      next_id = 0;
    }
    // Indicate the next parameter to search for:
    attribute_store_set_desired(next_id_node, &next_id, sizeof(next_id));
    // Set the reported, then undefine it so the resolver tries a new get immediately.
    attribute_store_set_reported(next_id_node, &next_id, sizeof(next_id));
    if (next_id != 0) {
      // If next ID is 0, we are done finding parameters.
      attribute_store_undefine_reported(next_id_node);
    }
  } else {
    attribute_store_set_reported(next_id_node, &next_id, sizeof(next_id));
  }

  // Look for the v4 flags:
  current_index = CONFIGURATION_PROPERTIES_REPORT_FIRST_VALUE_INDEX
                  + (3 * parameter_size) + 2;
  configuration_parameter_flag_t advanced     = false;
  configuration_parameter_flag_t bulk_support = true;
  if (frame_length > current_index) {
    // Advanced flag:
    advanced = (frame[current_index] & CONFIGURATION_ADVANCED_MASK);
    // No bulk support flag: (under the endpoint node this one)
    bulk_support
      = (frame[current_index] & CONFIGURATION_NO_BULK_SUPPORT_MASK) >> 1;
    // Node advertises "No bulk support", while we save "bulk support".
    // Invert the bool value
    bulk_support = !bulk_support;
  }
  // Save the flag values.
  attribute_store_set_child_reported(parameter_id_node,
                                     ATTRIBUTE(PARAMETER_ADVANCED),
                                     &advanced,
                                     sizeof(advanced));
  attribute_store_set_child_reported(endpoint_node,
                                     ATTRIBUTE(BULK_SUPPORT),
                                     &bulk_support,
                                     sizeof(bulk_support));

  return SL_STATUS_OK;
}

/**
 * @brief Handle incoming Configuration Info Report Commands
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t zwave_command_class_configuration_handle_info_report_command(
  const zwave_controller_connection_info_t *info,
  const uint8_t *frame,
  uint16_t frame_length)
{
  if (frame_length < 4) {
    return SL_STATUS_OK;
  }

  // Fetch the parameter ID from the frame:
  configuration_parameter_id_t parameter_id
    = (configuration_parameter_id_t)((frame[2] << 8) | frame[3]);
  attribute_store_node_t parameter_id_node
    = get_parameter_id_node(info->remote.node_id,
                            info->remote.endpoint_id,
                            parameter_id);
  attribute_store_node_t parameter_info_node
    = attribute_store_get_first_child_by_type(parameter_id_node,
                                              ATTRIBUTE(PARAMETER_INFO));

  // Check if we just received a "Report To Follow"
  bool concatenate_with_previous_value = false;
  reports_to_follow_t previous_reports_to_follow
    = get_reports_to_follow(parameter_info_node);

  reports_to_follow_t reports = 0;
  if (frame_length >= 4) {
    reports = frame[4];
  }
  set_reports_to_follow(parameter_info_node, reports);

  if ((previous_reports_to_follow != 0)
      && (reports == (previous_reports_to_follow - 1))) {
    // We received a report with a decrement of a previous Report To Follow
    // state. Concatenate the value.
    concatenate_with_previous_value = true;
  }

  char parameter_info[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  int16_t info_length                                       = frame_length - 5;
  if ((info_length > 0)
      && (info_length < ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH)) {
    strncpy(parameter_info, (const char *)&frame[5], info_length);
  }
  // Ensure null termination
  parameter_info[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH - 1] = '\0';
  if (concatenate_with_previous_value == true) {
    attribute_store_concatenate_to_reported_string(parameter_info_node,
                                                   parameter_info);
  } else {
    attribute_store_set_reported_string(parameter_info_node, parameter_info);
  }
  return SL_STATUS_OK;
}

/**
 * @brief Handle incoming Configuration Name Report Commands
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t zwave_command_class_configuration_handle_name_report_command(
  const zwave_controller_connection_info_t *info,
  const uint8_t *frame,
  uint16_t frame_length)
{
  if (frame_length < 4) {
    return SL_STATUS_OK;
  }

  // Fetch the parameter ID from the frame:
  configuration_parameter_id_t parameter_id
    = (configuration_parameter_id_t)((frame[2] << 8) | frame[3]);
  attribute_store_node_t parameter_id_node
    = get_parameter_id_node(info->remote.node_id,
                            info->remote.endpoint_id,
                            parameter_id);
  attribute_store_node_t parameter_name_node
    = attribute_store_get_first_child_by_type(parameter_id_node,
                                              ATTRIBUTE(PARAMETER_NAME));

  // Check if we just received a "Report To Follow"
  bool concatenate_with_previous_value = false;
  reports_to_follow_t previous_reports_to_follow
    = get_reports_to_follow(parameter_name_node);

  reports_to_follow_t reports = 0;
  if (frame_length >= 4) {
    reports = frame[4];
  }
  set_reports_to_follow(parameter_name_node, reports);

  if ((previous_reports_to_follow != 0)
      && (reports == (previous_reports_to_follow - 1))) {
    // We received a report with a decrement of a previous Report To Follow
    // state. Concatenate the value.
    concatenate_with_previous_value = true;
  }

  char name[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  int16_t name_length                             = frame_length - 5;
  if ((name_length > 0)
      && (name_length < ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH)) {
    strncpy(name, (const char *)&frame[5], name_length);
  }
  // Ensure null termination
  name[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH - 1] = '\0';
  if (concatenate_with_previous_value == true) {
    attribute_store_concatenate_to_reported_string(parameter_name_node, name);
  } else {
    attribute_store_set_reported_string(parameter_name_node, name);
  }

  return SL_STATUS_OK;
}

/** @brief Dispatches incoming Configuration Commands to individual
 *        command handlers.
 *
 * @param info           zwave_controller_connection_info_t object.
 * @param frame          Pointer to frame data
 * @param frame_length   Number of bytes contained in the frame
 *
 * @returns sl_status_t following the frame handling return codes.
 */
static sl_status_t zwave_command_class_configuration_control_handler(
  const zwave_controller_connection_info_t *info,
  const uint8_t *frame,
  uint16_t frame_length)
{
  if (frame_length < COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (frame[COMMAND_CLASS_INDEX] != COMMAND_CLASS_CONFIGURATION_V4) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame[COMMAND_INDEX]) {
    case CONFIGURATION_REPORT_V4:
      return zwave_command_class_configuration_handle_report_command(
        info,
        frame,
        frame_length);

    case CONFIGURATION_BULK_REPORT_V4:
      return zwave_command_class_configuration_handle_bulk_report_command(
        info,
        frame,
        frame_length);

    case CONFIGURATION_PROPERTIES_REPORT_V4:
      return zwave_command_class_configuration_handle_properties_report_command(
        info,
        frame,
        frame_length);

    case CONFIGURATION_NAME_REPORT_V4:
      return zwave_command_class_configuration_handle_name_report_command(
        info,
        frame,
        frame_length);

    case CONFIGURATION_INFO_REPORT_V4:
      return zwave_command_class_configuration_handle_info_report_command(
        info,
        frame,
        frame_length);

    default:
      sl_log_debug(LOG_TAG, "Just received a command of the future. Ignoring");
      return SL_STATUS_NOT_SUPPORTED;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Attribute resolution functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Generates a Configuration Get or Bulk Get Command to find out the
 * value of a parameter.
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 *                       Expects a PARAMETER_VALUE under PARAMETER_ID
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_configuration_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Find the parameter ID that we are trying to find the value of:
  configuration_parameter_id_t parameter_id = 0;
  attribute_store_node_t parameter_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE(PARAMETER_ID));
  attribute_store_get_reported(parameter_id_node,
                               &parameter_id,
                               sizeof(parameter_id));

  // Invalid parameter ID, remove it.
  if (parameter_id == 0) {
    attribute_store_delete_node(parameter_id_node);
    *frame_length = 0;
    return SL_STATUS_ALREADY_EXISTS;
  }

  if (parameter_id <= 255) {
    // Create a frame for the attribute resolver. All versions support this frame.
    ZW_CONFIGURATION_GET_V4_FRAME *get_frame
      = (ZW_CONFIGURATION_GET_V4_FRAME *)frame;
    get_frame->cmdClass        = COMMAND_CLASS_CONFIGURATION_V4;
    get_frame->cmd             = CONFIGURATION_GET_V4;
    get_frame->parameterNumber = (uint8_t)parameter_id;
    *frame_length              = sizeof(ZW_CONFIGURATION_GET_V4_FRAME);
    return SL_STATUS_OK;
  }

  // For parameters > 255, we need bulk support
  if (false == is_bulk_supported(node)) {
    sl_log_debug(LOG_TAG,
                 "No Bulk Support, removing Parameter ID "
                 "%d from the attribute store.",
                 parameter_id);
    attribute_store_delete_node(parameter_id_node);
    *frame_length = 0;
    return SL_STATUS_ALREADY_EXISTS;
  }

  // We are good to send a Bulk Get:
  ZW_CONFIGURATION_BULK_GET_V4_FRAME *get_frame
    = (ZW_CONFIGURATION_BULK_GET_V4_FRAME *)frame;
  get_frame->cmdClass           = COMMAND_CLASS_CONFIGURATION_V4;
  get_frame->cmd                = CONFIGURATION_BULK_GET_V4;
  get_frame->numberOfParameters = 1;
  get_frame->parameterOffset1   = (uint8_t)(parameter_id >> 8);
  get_frame->parameterOffset2   = (uint8_t)(parameter_id & 0xFF);
  *frame_length                 = sizeof(ZW_CONFIGURATION_BULK_GET_V4_FRAME);
  return SL_STATUS_OK;
}

/**
 * @brief Generates a Configuration Set or Bulk Set Command to set the value
 *        of a parameter.
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 *                       Expects a PARAMETER_VALUE under PARAMETER_ID
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_configuration_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Find the parameter ID that we are trying set the value:
  configuration_parameter_id_t parameter_id = 0;
  attribute_store_node_t parameter_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE(PARAMETER_ID));
  attribute_store_get_reported(parameter_id_node,
                               &parameter_id,
                               sizeof(parameter_id));

  // Invalid parameter ID, remove it.
  if (parameter_id == 0) {
    attribute_store_delete_node(parameter_id_node);
    *frame_length = 0;
    return SL_STATUS_ALREADY_EXISTS;
  }

  // Fetch the size and the value
  attribute_store_node_t parameter_value_node
    = attribute_store_get_first_child_by_type(parameter_id_node,
                                              ATTRIBUTE(PARAMETER_VALUE));
  configuration_parameter_value_t value = 0;
  attribute_store_get_desired(parameter_value_node, &value, sizeof(value));
  configuration_parameter_size_t size
    = get_configuration_parameter_size(parameter_id_node);
  configuration_parameter_size_t format
    = get_configuration_parameter_format(parameter_id_node);

  // Is it a read-only parameter ? then bail out.
  if (true == is_parameter_read_only(parameter_value_node)) {
    attribute_store_undefine_desired(parameter_value_node);
    *frame_length = 0;
    return SL_STATUS_ALREADY_EXISTS;
  }

  if (parameter_id <= 255) {
    // Create a frame for the attribute resolver. All versions support this frame.
    ZW_CONFIGURATION_SET_1BYTE_V4_FRAME *set_frame
      = (ZW_CONFIGURATION_SET_1BYTE_V4_FRAME *)frame;
    set_frame->cmdClass        = COMMAND_CLASS_CONFIGURATION_V4;
    set_frame->cmd             = CONFIGURATION_SET_V4;
    set_frame->parameterNumber = (uint8_t)parameter_id;
    set_frame->level           = (uint8_t)(size & CONFIGURATION_SIZE_MASK);
    set_parameter_value_in_buffer(&frame[CONFIGURATION_SET_VALUE_INDEX],
                                  value,
                                  size,
                                  format);

    *frame_length = CONFIGURATION_SET_VALUE_INDEX + size;
    return SL_STATUS_OK;
  }

  // For parameters > 255, we need bulk support
  if (false == is_bulk_supported(node)) {
    attribute_store_delete_node(parameter_id_node);
    *frame_length = 0;
    return SL_STATUS_ALREADY_EXISTS;
  }

  // We are good to send a Bulk Set:
  ZW_CONFIGURATION_BULK_SET_1BYTE_V4_FRAME *set_frame
    = (ZW_CONFIGURATION_BULK_SET_1BYTE_V4_FRAME *)frame;
  set_frame->cmdClass           = COMMAND_CLASS_CONFIGURATION_V4;
  set_frame->cmd                = CONFIGURATION_BULK_SET_V4;
  set_frame->parameterOffset1   = (uint8_t)(parameter_id >> 8);
  set_frame->parameterOffset2   = (uint8_t)(parameter_id & 0xFF);
  set_frame->numberOfParameters = 1;
  set_frame->properties1        = (uint8_t)(size & CONFIGURATION_SIZE_MASK);
  set_parameter_value_in_buffer(&frame[CONFIGURATION_BULK_SET_VALUE_INDEX],
                                value,
                                size,
                                format);
  *frame_length = CONFIGURATION_BULK_SET_VALUE_INDEX + size;
  return SL_STATUS_OK;
}

/**
 * @brief Generates a Configuration Properties Get or Configuration Get Command
 * to find out if a parameter ID is supported.
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_configuration_get_next_parameter(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Find the parameter ID that we are trying to discover
  configuration_parameter_id_t parameter_id = 0;
  attribute_store_get_desired(node, &parameter_id, sizeof(parameter_id));

  zwave_cc_version_t supporting_node_version
    = zwave_command_class_get_version_from_node(node,
                                                COMMAND_CLASS_CONFIGURATION_V4);

  // For version 3 and higher, discover using the Properties Get command:
  if (supporting_node_version > 2) {
    // Create a Properties Get frame.
    ZW_CONFIGURATION_PROPERTIES_GET_V4_FRAME *get_frame
      = (ZW_CONFIGURATION_PROPERTIES_GET_V4_FRAME *)frame;
    get_frame->cmdClass         = COMMAND_CLASS_CONFIGURATION_V4;
    get_frame->cmd              = CONFIGURATION_PROPERTIES_GET_V4;
    get_frame->parameterNumber1 = (uint8_t)(parameter_id >> 8);
    get_frame->parameterNumber2 = (uint8_t)(parameter_id & 0xFF);
    *frame_length = sizeof(ZW_CONFIGURATION_PROPERTIES_GET_V4_FRAME);
    return SL_STATUS_OK;
  }

  // If version is 1-2, use a Configuration Get/Configuration Bulk Get:
  if (parameter_id <= 255) {
    ZW_CONFIGURATION_GET_V4_FRAME *get_frame
      = (ZW_CONFIGURATION_GET_V4_FRAME *)frame;
    get_frame->cmdClass        = COMMAND_CLASS_CONFIGURATION_V4;
    get_frame->cmd             = CONFIGURATION_GET_V4;
    get_frame->parameterNumber = (uint8_t)parameter_id;
    *frame_length              = sizeof(ZW_CONFIGURATION_GET_V4_FRAME);
    return SL_STATUS_OK;
  }

  // At that point, we can't use Get or Properties Get, we need Bulk Get
  // or we are out of luck.
  if (true == is_bulk_supported(node)) {
    ZW_CONFIGURATION_BULK_GET_V4_FRAME *get_frame
      = (ZW_CONFIGURATION_BULK_GET_V4_FRAME *)frame;
    get_frame->cmdClass           = COMMAND_CLASS_CONFIGURATION_V4;
    get_frame->cmd                = CONFIGURATION_BULK_GET_V4;
    get_frame->numberOfParameters = 1;
    get_frame->parameterOffset1   = (uint8_t)(parameter_id >> 8);
    get_frame->parameterOffset2   = (uint8_t)(parameter_id & 0xFF);
    *frame_length                 = sizeof(ZW_CONFIGURATION_BULK_GET_V4_FRAME);
    return SL_STATUS_OK;
  }
  // We can't resolve this parameter id.
  sl_log_debug(LOG_TAG,
               "Cannot resolve next Parameter ID %d, "
               "supporting node has no capabilities for it. ",
               parameter_id);
  parameter_id = 0;
  attribute_store_set_desired(node, &parameter_id, sizeof(parameter_id));
  attribute_store_set_reported(node, &parameter_id, sizeof(parameter_id));
  return SL_STATUS_ALREADY_EXISTS;
}

/**
 * @brief Generates a Configuration Name Get or fills up data automatically
 * if the command is not supported.
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 *                       Expects a ATTRIBUTE(PARAMETER_NAME) type
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_configuration_name_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Find the parameter ID that we are trying to discover the name of
  configuration_parameter_id_t parameter_id = 0;
  attribute_store_node_t parameter_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE(PARAMETER_ID));
  attribute_store_get_reported(parameter_id_node,
                               &parameter_id,
                               sizeof(parameter_id));

  zwave_cc_version_t supporting_node_version
    = zwave_command_class_get_version_from_node(node,
                                                COMMAND_CLASS_CONFIGURATION_V4);

  // Just fill a default string if the node cannot be asked.
  if (supporting_node_version < 3) {
    char parameter_name[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
    snprintf(parameter_name,
             sizeof(parameter_name),
             "Parameter %d",
             parameter_id);
    attribute_store_set_reported_string(node, parameter_name);
    *frame_length = 0;
    return SL_STATUS_ALREADY_EXISTS;
  }

  // Create a Name get frame.
  ZW_CONFIGURATION_NAME_GET_V4_FRAME *get_frame
    = (ZW_CONFIGURATION_NAME_GET_V4_FRAME *)frame;
  get_frame->cmdClass         = COMMAND_CLASS_CONFIGURATION_V4;
  get_frame->cmd              = CONFIGURATION_NAME_GET_V4;
  get_frame->parameterNumber1 = (uint8_t)(parameter_id >> 8);
  get_frame->parameterNumber2 = (uint8_t)(parameter_id & 0xFF);
  *frame_length               = sizeof(ZW_CONFIGURATION_NAME_GET_V4_FRAME);
  return SL_STATUS_OK;
}

/**
 * @brief Generates a Configuration Info Get or fills up data automatically
 * if the command is not supported.
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 *                       Expects a ATTRIBUTE(PARAMETER_INFO) type
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_configuration_info_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Find the parameter ID that we are trying to discover the information of
  configuration_parameter_id_t parameter_id = 0;
  attribute_store_node_t parameter_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE(PARAMETER_ID));
  attribute_store_get_reported(parameter_id_node,
                               &parameter_id,
                               sizeof(parameter_id));

  zwave_cc_version_t supporting_node_version
    = zwave_command_class_get_version_from_node(node,
                                                COMMAND_CLASS_CONFIGURATION_V4);

  // Just fill a default string if the node cannot be asked.
  if (supporting_node_version < 3) {
    const char parameter_info[] = "No information available";
    attribute_store_set_reported_string(node, parameter_info);
    *frame_length = 0;
    return SL_STATUS_ALREADY_EXISTS;
  }

  // Create a Info get frame.
  ZW_CONFIGURATION_INFO_GET_V4_FRAME *get_frame
    = (ZW_CONFIGURATION_INFO_GET_V4_FRAME *)frame;
  get_frame->cmdClass         = COMMAND_CLASS_CONFIGURATION_V4;
  get_frame->cmd              = CONFIGURATION_INFO_GET_V4;
  get_frame->parameterNumber1 = (uint8_t)(parameter_id >> 8);
  get_frame->parameterNumber2 = (uint8_t)(parameter_id & 0xFF);
  *frame_length               = sizeof(ZW_CONFIGURATION_INFO_GET_V4_FRAME);
  return SL_STATUS_OK;
}

/**
 * @brief Generates a Configuration Properties Get or fills up data automatically
 * if the command is not supported.
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 *                       Expects a ATTRIBUTE(PARAMETER_MIN_VALUE) type
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_configuration_properties_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Find the parameter ID that we are trying to discover the properties of
  configuration_parameter_id_t parameter_id = 0;
  attribute_store_node_t parameter_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE(PARAMETER_ID));
  attribute_store_get_reported(parameter_id_node,
                               &parameter_id,
                               sizeof(parameter_id));

  zwave_cc_version_t supporting_node_version
    = zwave_command_class_get_version_from_node(node,
                                                COMMAND_CLASS_CONFIGURATION_V4);

  // Just fill a default string if the node cannot be asked.
  if (supporting_node_version < 3) {
    *frame_length = 0;
    return set_default_parameter_properties(parameter_id_node);
  }

  // Create a properties get frame.
  ZW_CONFIGURATION_PROPERTIES_GET_V4_FRAME *get_frame
    = (ZW_CONFIGURATION_PROPERTIES_GET_V4_FRAME *)frame;
  get_frame->cmdClass         = COMMAND_CLASS_CONFIGURATION_V4;
  get_frame->cmd              = CONFIGURATION_PROPERTIES_GET_V4;
  get_frame->parameterNumber1 = (uint8_t)(parameter_id >> 8);
  get_frame->parameterNumber2 = (uint8_t)(parameter_id & 0xFF);
  *frame_length = sizeof(ZW_CONFIGURATION_PROPERTIES_GET_V4_FRAME);
  return SL_STATUS_OK;
}

/**
 * @brief Generates a Configuration Default Reset
 *
 * @param node           Attribute Store node that the resolver tries to resolve.
 *                       Expects a ATTRIBUTE(DEFAULT_RESET_REQUESTED) type
 * @param frame          Pointer at which frame data can be written.
 * @param frame_length   Pointer at which the frame data length can be written.
 *
 * @returns sl_status_t following the \ref attribute_resolver_function_t return codes.
 */
static sl_status_t zwave_command_class_configuration_default_reset(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  zwave_cc_version_t supporting_node_version
    = zwave_command_class_get_version_from_node(node,
                                                COMMAND_CLASS_CONFIGURATION_V4);

  // Double check that we are not doing anything wrong here:
  if (supporting_node_version < 4) {
    attribute_store_delete_node(node);
    *frame_length = 0;
    return SL_STATUS_ALREADY_EXISTS;
  }

  // Create a Set Default frame.
  zwave_minimum_frame_t *get_frame = (zwave_minimum_frame_t *)frame;
  get_frame->command_class         = COMMAND_CLASS_CONFIGURATION_V4;
  get_frame->command               = CONFIGURATION_DEFAULT_RESET_V4;
  *frame_length                    = sizeof(zwave_minimum_frame_t);
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Resolver callback functions
///////////////////////////////////////////////////////////////////////////////
static void on_default_reset_send_data_complete(attribute_store_node_t node,
                                                resolver_rule_type_t rule_type,
                                                zpc_resolver_event_t event)
{
  command_status_values_t state = FINAL_STATE;
  if (rule_type == RESOLVER_GET_RULE) {
    sl_log_warning(LOG_TAG,
                   "Default Reset Requested attribute "
                   "should not be 'Get Resolved'");
    attribute_store_set_desired(node, &state, sizeof(state));
    attribute_store_set_reported(node, &state, sizeof(state));
    return;
  }

  switch (event) {
    case FRAME_SENT_EVENT_OK_SUPERVISION_WORKING:
      // Wait for more. Weird to have a working Set Default.
      return;

    case FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS:
      set_all_parameters_to_their_default_value(node);
      break;

    case FRAME_SENT_EVENT_OK_SUPERVISION_NO_SUPPORT:
      // This is strange. Just give up and avoid "default resets" again.
      sl_log_debug(LOG_TAG,
                   "Configuration Default Reset is not supported. "
                   "Deleting attribute store node %d",
                   node);
      attribute_store_delete_node(node);
      break;

    //FRAME_SENT_EVENT_OK_NO_SUPERVISION
    //FRAME_SENT_EVENT_OK_SUPERVISION_FAIL
    default:
      undefine_all_parameter_values(node);
      break;
  }

  // Do not try to resolve this again: (only 1 set default command)
  attribute_store_set_desired(node, &state, sizeof(state));
  attribute_store_set_reported(node, &state, sizeof(state));
}

/**
 * @brief Callback function for when a Get did not succeed in finding the
 * next parameter id. We may have more to search for, so we mark it as defined
 * in this case to trigger a new get resolution with a new parameter id.
 *
 * @param node    Attribute store node for the NEXT_SUPPORTED_PARAMETER_ID
 */
static void
  on_next_supported_parameter_id_not_found(attribute_store_node_t node)
{
  configuration_parameter_id_t next_parameter = 0;
  attribute_store_set_reported(node, &next_parameter, sizeof(next_parameter));
}

/**
 * @brief Deletes the Configuration Parameter ID if we could not get its value
 * for v1-v2 nodes.
 *
 * @param parameter_value_node    Attribute store node for the Parameter Value
 */
static void
  on_configuration_get_failure(attribute_store_node_t parameter_value_node)
{
  zwave_cc_version_t supporting_node_version
    = zwave_command_class_get_version_from_node(parameter_value_node,
                                                COMMAND_CLASS_CONFIGURATION_V4);

  if ((supporting_node_version > 0) && (supporting_node_version < 3)) {
    attribute_store_node_t parameter_id_node
      = attribute_store_get_first_parent_with_type(parameter_value_node,
                                                   ATTRIBUTE(PARAMETER_ID));
    attribute_store_delete_node(parameter_id_node);
    sl_log_debug(LOG_TAG,
                 "Failed to Get parameter value for Attribute ID %d, "
                 "deleting Parameter ID (ID %d).",
                 parameter_value_node,
                 parameter_id_node);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Attribute update callbacks
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Attribute callback function listening for Configuration Paramter ID
 *        creations and creating all the sub-attributes
 *
 * @param parameter_id_node    Attribute Store node that was modified.
 * @param change               Type of change applied to the node.
 */
static void
  zwave_command_class_configuration_on_configuration_parameter_created(
    attribute_store_node_t parameter_id_node, attribute_store_change_t change)
{
  if (change != ATTRIBUTE_CREATED) {
    return;
  }

  attribute_store_add_if_missing(parameter_id_node,
                                 configuration_parameter_attributes,
                                 COUNT_OF(configuration_parameter_attributes));

  zwave_cc_version_t supporting_node_version
    = zwave_command_class_get_version_from_node(parameter_id_node,
                                                COMMAND_CLASS_CONFIGURATION_V4);

  if (supporting_node_version < 3) {
    // Proactively fill default values if the node cannot be asked.
    set_default_parameter_properties(parameter_id_node);
  }
}

/**
 * @brief Attribute callback function listening for Configuration version
 *        attribute updates.
 *
 * @param updated_node    Attribute Store node that was modified.
 * @param change          Type of change applied to the node.
 */
static void zwave_command_class_configuration_on_version_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  if (is_zwave_command_class_filtered_for_root_device(
        COMMAND_CLASS_CONFIGURATION_V4,
        updated_node)
      == true) {
    return;
  }

  zwave_cc_version_t supporting_node_version = 0;
  attribute_store_get_reported(updated_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  if (supporting_node_version == 0) {
    // Wait for the version to be known.
    return;
  }

  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  // All versions have these attributes.
  attribute_store_add_if_missing(endpoint_node,
                                 default_attributes,
                                 COUNT_OF(default_attributes));

  // Set the bulk support based on the version, if not known:
  attribute_store_node_t bulk_support_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(BULK_SUPPORT));
  if (false == attribute_store_is_reported_defined(bulk_support_node)) {
    configuration_bulk_support_t bulk_support = false;
    if (supporting_node_version == 2 || supporting_node_version == 3) {
      bulk_support = true;
    }
    // Start off assuming no support, when a >= v4 reports its parameters
    // it will tell us about it and we will overwrite it
    attribute_store_set_reported(bulk_support_node,
                                 &bulk_support,
                                 sizeof(bulk_support));
  }

  // Set the desired value of the next parameter id to 0, to get started
  configuration_parameter_id_t next_parameter = 0;
  attribute_store_set_child_desired(endpoint_node,
                                    ATTRIBUTE(NEXT_SUPPORTED_PARAMETER_ID),
                                    &next_parameter,
                                    sizeof(next_parameter));

  // for v1-v2 nodes, we cannot do anything so mark the next parameter as resolved.
  if (supporting_node_version < 3) {
    attribute_store_set_child_reported(endpoint_node,
                                       ATTRIBUTE(NEXT_SUPPORTED_PARAMETER_ID),
                                       &next_parameter,
                                       sizeof(next_parameter));
  }

  // Create a "default reset requested"
  if (supporting_node_version >= 4) {
    command_status_values_t status = FINAL_STATE;
    attribute_store_set_child_desired(endpoint_node,
                                      ATTRIBUTE(DEFAULT_RESET_REQUESTED),
                                      &status,
                                      sizeof(status));
    attribute_store_set_child_reported(endpoint_node,
                                       ATTRIBUTE(DEFAULT_RESET_REQUESTED),
                                       &status,
                                       sizeof(status));
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_configuration_init()
{
  // Attribute resolver rules
  attribute_resolver_register_rule(ATTRIBUTE(PARAMETER_VALUE),
                                   &zwave_command_class_configuration_set,
                                   &zwave_command_class_configuration_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(NEXT_SUPPORTED_PARAMETER_ID),
    NULL,
    &zwave_command_class_configuration_get_next_parameter);

  attribute_resolver_register_rule(ATTRIBUTE(PARAMETER_NAME),
                                   NULL,
                                   &zwave_command_class_configuration_name_get);

  attribute_resolver_register_rule(ATTRIBUTE(PARAMETER_INFO),
                                   NULL,
                                   &zwave_command_class_configuration_info_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(DEFAULT_RESET_REQUESTED),
    &zwave_command_class_configuration_default_reset,
    NULL);

  attribute_resolver_register_rule(
    ATTRIBUTE(PARAMETER_MIN_VALUE),
    NULL,
    &zwave_command_class_configuration_properties_get);

  // Attribute Resolver event listener:
  register_send_event_handler(ATTRIBUTE(DEFAULT_RESET_REQUESTED),
                              &on_default_reset_send_data_complete);

  // Attribute Resolver give up listener:
  attribute_resolver_set_resolution_give_up_listener(
    ATTRIBUTE(NEXT_SUPPORTED_PARAMETER_ID),
    &on_next_supported_parameter_id_not_found);

  attribute_resolver_set_resolution_give_up_listener(
    ATTRIBUTE(PARAMETER_VALUE),
    &on_configuration_get_failure);

  // Attribute store callbacks
  attribute_store_register_callback_by_type(
    zwave_command_class_configuration_on_version_attribute_update,
    ATTRIBUTE(VERSION));

  attribute_store_register_callback_by_type(
    &zwave_command_class_configuration_on_configuration_parameter_created,
    ATTRIBUTE(PARAMETER_ID));

  // Register our handler to the Z-Wave CC framework:
  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler = &zwave_command_class_configuration_control_handler;
  // Not supported, so this does not really matter
  handler.minimal_scheme             = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.manual_security_validation = false;
  handler.command_class              = COMMAND_CLASS_CONFIGURATION_V4;
  handler.version                    = CONFIGURATION_VERSION_V4;
  handler.command_class_name         = "Configuration";
  handler.comments                   = "";

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}
