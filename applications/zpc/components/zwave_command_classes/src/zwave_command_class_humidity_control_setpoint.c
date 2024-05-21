
/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

// System
#include <stdlib.h>

#include "zwave_command_class_humidity_control_setpoint.h"
#include "zwave_command_class_humidity_control_types.h"
#include "zwave_command_classes_utils.h"
#include "ZW_classcmd.h"

// Includes from other ZPC Components
#include "zwave_command_class_indices.h"
#include "zwave_command_handler.h"
#include "zwave_command_class_version_types.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"

// Unify
#include "attribute_resolver.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "sl_log.h"

#define LOG_TAG         "zwave_command_class_humidity_control_setpoint"
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_##type

// Aliases
#define MASK_PRECISION \
  HUMIDITY_CONTROL_SETPOINT_CAPABILITIES_REPORT_PROPERTIES2_PRECISION1_MASK
#define MASK_SCALE \
  HUMIDITY_CONTROL_SETPOINT_CAPABILITIES_REPORT_PROPERTIES2_SCALE1_MASK
#define MASK_SIZE \
  HUMIDITY_CONTROL_SETPOINT_CAPABILITIES_REPORT_PROPERTIES2_SIZE1_MASK

// Max values in Humidity Control Setpoint Supported Report values
#define MAX_SUPPORTED_SETPOINT_MODES 4

enum humidity_control_setpoint_value_type {
  SETPOINT_CURRENT_VALUE,
  SETPOINT_MIN_VALUE,
  SETPOINT_MAX_VALUE
};

/////////////////////////////////////////////////////////////////////////////
// Utils
/////////////////////////////////////////////////////////////////////////////

/**
 * @brief Get setpoint real value from raw value and precision
 * 
 * get_setpoint_value(10, 0) => 10.0
 * get_setpoint_value(10, 1) => 1.0
 * get_setpoint_value(10, 2) => 0.1
 * 
 * @param value Raw value
 * @param precision Precision of value
 * 
 * @return double Real value
 */
double get_setpoint_value(humidity_control_setpoint_value_t value,
                          humidity_control_setpoint_precision_t precision)
{
  double current_value = value;
  for (uint8_t i = 0; i < precision; i++) {
    current_value /= 10.;
  }
  return current_value;
}
/**
 * @brief Check if current value can be set (in bounds of value_min and value_max)
 * 
 * If value_min and value_max field doesn't exists, it will return false
 * 
 * @param setpoint_node Current setpoint node
 * @param value_raw Value to check (precision will be applied to it)
 * @param precision Precision of value
 * 
 * @return true  Value in bounds
 * @return false Value not in bounds or bounds cannot be found
 */
bool check_setpoint_value_bounds(
  attribute_store_node_t setpoint_node,
  humidity_control_setpoint_value_t value_raw,
  humidity_control_setpoint_precision_t precision)
{
  humidity_control_setpoint_value_t min_value_raw;
  humidity_control_setpoint_precision_t min_value_precision;
  humidity_control_setpoint_value_t max_value_raw;
  humidity_control_setpoint_precision_t max_value_precision;

  attribute_store_node_t min_value_node
    = attribute_store_get_first_child_by_type(setpoint_node,
                                              ATTRIBUTE(MIN_VALUE));

  sl_status_t status = attribute_store_get_reported(min_value_node,
                                                    &min_value_raw,
                                                    sizeof(min_value_raw));

  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Can't get MIN_VALUE attribute value");
    return false;
  }

  status = attribute_store_get_child_reported(min_value_node,
                                              ATTRIBUTE(MIN_VALUE_PRECISION),
                                              &min_value_precision,
                                              sizeof(min_value_precision));

  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Can't get MIN_VALUE_PRECISION attribute value");
    return false;
  }

  attribute_store_node_t max_value_node
    = attribute_store_get_first_child_by_type(setpoint_node,
                                              ATTRIBUTE(MAX_VALUE));

  status = attribute_store_get_reported(max_value_node,
                                        &max_value_raw,
                                        sizeof(max_value_raw));

  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Can't get MAX_VALUE attribute value");
    return false;
  }

  status = attribute_store_get_child_reported(max_value_node,
                                              ATTRIBUTE(MAX_VALUE_PRECISION),
                                              &max_value_precision,
                                              sizeof(max_value_precision));

  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Can't get MAX_VALUE_PRECISION attribute value");
    return false;
  }

  double min_value = get_setpoint_value(min_value_raw, min_value_precision);
  double max_value = get_setpoint_value(max_value_raw, max_value_precision);
  double value     = get_setpoint_value(value_raw, precision);

  bool result = value >= min_value && value <= max_value;

  if (!result) {
    sl_log_error(LOG_TAG,
                 "Value %f is out of bounds. Should be between %f, %f.",
                 value,
                 min_value,
                 max_value);
  }

  return result;
}

/**
 * @brief Interpret value from a report frame and store it in the attribute store
 * 
 * @warning This function doesn't check any bounds in frame_data
 * 
 * @param setpoint_node Current setpoint node 
 * @param value_type Determines the attribute field of value report (min_value, max_value or value)
 * @param frame_data Frame raw data 
 * @param start_index Starting index in the frame raw data
 * 
 * @return uint8_t Byte read count 
 */
uint8_t interpret_value_from_report(
  attribute_store_node_t setpoint_node,
  enum humidity_control_setpoint_value_type value_type,
  const uint8_t *frame_data,
  uint8_t start_index)
{
  // Save current index
  uint8_t current_index = start_index;
  // Determine used types
  attribute_store_type_t value_store_type;
  attribute_store_type_t value_scale_type;
  attribute_store_type_t value_precision_type;
  switch (value_type) {
    case SETPOINT_CURRENT_VALUE:
      value_store_type     = ATTRIBUTE(VALUE);
      value_scale_type     = ATTRIBUTE(VALUE_SCALE);
      value_precision_type = ATTRIBUTE(VALUE_PRECISION);
      break;
    case SETPOINT_MIN_VALUE:
      value_store_type     = ATTRIBUTE(MIN_VALUE);
      value_scale_type     = ATTRIBUTE(MIN_VALUE_SCALE);
      value_precision_type = ATTRIBUTE(MIN_VALUE_PRECISION);
      break;
    case SETPOINT_MAX_VALUE:
      value_store_type     = ATTRIBUTE(MAX_VALUE);
      value_scale_type     = ATTRIBUTE(MAX_VALUE_SCALE);
      value_precision_type = ATTRIBUTE(MAX_VALUE_PRECISION);
      break;
    default:
      sl_log_error(LOG_TAG, "Invalid humidity_control_setpoint_value_type");
      return 0;
  }

  humidity_control_setpoint_precision_t precision
    = (frame_data[current_index] & MASK_PRECISION) >> 5;

  humidity_control_setpoint_scale_t scale
    = (frame_data[current_index] & MASK_SCALE) >> 3;

  humidity_control_setpoint_size_t size = frame_data[current_index] & MASK_SIZE;

  if (size > 4) {
    sl_log_error(LOG_TAG, "Incorrect reported size for setpoint capabilities");
    return 0;
  }

  // Next field(s) (value)
  current_index++;

  humidity_control_setpoint_value_t value
    = get_signed_value_from_frame_and_size(&frame_data[current_index], size);

  // Check bounds
  if (value_type == SETPOINT_CURRENT_VALUE
      && !check_setpoint_value_bounds(setpoint_node, value, precision)) {
    return 0;
  }

  // Create the value node if missing
  attribute_store_node_t value_node
    = attribute_store_create_child_if_missing(setpoint_node, value_store_type);

  // Set computed value
  attribute_store_set_reported(value_node, &value, sizeof(value));

  // Set scale value
  attribute_store_set_child_reported(value_node,
                                     value_scale_type,
                                     &scale,
                                     sizeof(scale));
  // Set precision value
  attribute_store_set_child_reported(value_node,
                                     value_precision_type,
                                     &precision,
                                     sizeof(precision));
  // Return # of bytes read
  return 1 + size;
}

// Remove all ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_TYPE from attribute store
void remove_all_humidity_setpoint_type_attributes(
  attribute_store_node_t endpoint_node)
{
  attribute_store_node_t type_node;
  do {
    type_node = attribute_store_get_node_child_by_type(endpoint_node,
                                                       ATTRIBUTE(TYPE),
                                                       0);
    attribute_store_delete_node(type_node);
  } while (type_node != ATTRIBUTE_STORE_INVALID_NODE);
}

// Create humidity setpoint
void create_humidity_setpoint_type_node(
  attribute_store_node_t endpoint_node,
  humidity_control_setpoint_type_t setpoint_type)
{
  // Check compatibility
  // if (!is_thermostat_setpoint_mode_compatible_with_version(type, version)) {
  //   return ATTRIBUTE_STORE_INVALID_NODE;
  // }

  attribute_store_node_t type_node
    = attribute_store_emplace(endpoint_node,
                              ATTRIBUTE(TYPE),
                              &setpoint_type,
                              sizeof(setpoint_type));

  // Add the six other nodes under the type.
  const attribute_store_type_t additional_nodes[]
    = {// Scale support to check value reported (Not supported for now)
       // ATTRIBUTE(SUPPORTED_SCALE),
       // Min and Max attribute first to check value reported
       ATTRIBUTE(MIN_VALUE),
       ATTRIBUTE(MAX_VALUE),
       // Finally, actual value
       ATTRIBUTE(VALUE)};

  attribute_store_add_if_missing(type_node,
                                 additional_nodes,
                                 COUNT_OF(additional_nodes));
}

// Create ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_TYPE based on bitmask in supported_types
// This function will also check version
void create_all_supported_humidity_setpoint_type(
  attribute_store_node_t endpoint_node,
  humidity_control_setpoint_supported_types_t supported_types)
{
  // Contains current bit tested
  uint8_t setpoint_mode_current_bit = 0x0;

  for (uint8_t i = 0; i <= MAX_SUPPORTED_SETPOINT_MODES; i++) {
    setpoint_mode_current_bit = 1 << i;
    setpoint_mode_current_bit &= supported_types;
    humidity_control_setpoint_type_t current_type;

    // Check if current bit tested correspond to a registered type
    switch (setpoint_mode_current_bit) {
      case 0x02:
        current_type
          = HUMIDITY_CONTROL_SETPOINT_REPORT_SETPOINT_TYPE_HUMIDIFIER;
        break;
      case 0x04:
        current_type
          = HUMIDITY_CONTROL_SETPOINT_REPORT_SETPOINT_TYPE_DEHUMIDIFIER;
        break;
      case 0x08:
        current_type = HUMIDITY_CONTROL_SETPOINT_REPORT_SETPOINT_TYPE_AUTO_V2;
        break;
      default:
        sl_log_error(
          LOG_TAG,
          "Invalid bit in Humidity Control SetPoint supported types");
        continue;
    }

    create_humidity_setpoint_type_node(endpoint_node, current_type);
  }
}

// Get associated ATTRIBUTE(TYPE) from current_node.
// The ATTRIBUTE(TYPE) should be a parent of current_node
humidity_control_setpoint_type_t
  get_associated_setpoint_type(attribute_store_node_t current_node)
{
  attribute_store_node_t type_node
    = attribute_store_get_first_parent_with_type(current_node, ATTRIBUTE(TYPE));

  if (type_node == ATTRIBUTE_STORE_INVALID_NODE) {
    sl_log_error(LOG_TAG, "Can't get type node.");
    return 0;
  }

  humidity_control_setpoint_type_t current_type;
  sl_status_t status = attribute_store_get_reported(type_node,
                                                    &current_type,
                                                    sizeof(current_type));

  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Can't get type node reported value.");
    return 0;
  }

  return current_type;
}

// Get setpoint node with specified value
attribute_store_node_t
  get_setpoint_node(attribute_store_node_t endpoint_node,
                    humidity_control_setpoint_type_t setpoint_type)
{
  return attribute_store_get_node_child_by_value(endpoint_node,
                                                 ATTRIBUTE(TYPE),
                                                 REPORTED_ATTRIBUTE,
                                                 &setpoint_type,
                                                 sizeof(setpoint_type),
                                                 0);
}

/////////////////////////////////////////////////////////////////////////////
// Version & Attribute Creation
/////////////////////////////////////////////////////////////////////////////
static void
  zwave_command_class_humidity_control_setpoint_on_version_attribute_update(
    attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  zwave_cc_version_t version = 0;
  attribute_store_get_reported(updated_node, &version, sizeof(version));

  if (version == 0) {
    return;
  }

  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  // The order of the attribute matter since it defines the order of the
  // Z-Wave get command order.
  const attribute_store_type_t attributes[] = {ATTRIBUTE(SUPPORTED_TYPES)};

  attribute_store_add_if_missing(endpoint_node,
                                 attributes,
                                 COUNT_OF(attributes));
}

static void
  zwave_command_class_humidity_control_setpoint_on_scale_attribute_update(
    attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  humidity_control_setpoint_scale_t scale;
  attribute_store_get_reported(updated_node, &scale, sizeof(scale));

  // Nothing to do here
  if (scale <= 1) {
    return;
  }

  sl_log_warning(LOG_TAG, "Unsupported scale value, setting scale to 0.");

  // Default value
  scale = 0;
  // Else if value is not supported
  attribute_store_set_reported(updated_node, &scale, sizeof(scale));
}

/////////////////////////////////////////////////////////////////////////////
// Humidity Control SetPoint Supported Types Get/Report
/////////////////////////////////////////////////////////////////////////////
static sl_status_t
  zwave_command_class_humidity_control_setpoint_supported_types_get(
    attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  (void)node;  // unused.
  ZW_HUMIDITY_CONTROL_SETPOINT_SUPPORTED_GET_FRAME *get_frame
    = (ZW_HUMIDITY_CONTROL_SETPOINT_SUPPORTED_GET_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT;
  get_frame->cmd      = HUMIDITY_CONTROL_SETPOINT_SUPPORTED_GET;
  *frame_length = sizeof(ZW_HUMIDITY_CONTROL_SETPOINT_SUPPORTED_GET_FRAME);
  return SL_STATUS_OK;
}

sl_status_t
  zwave_command_class_humidity_control_setpoint_supported_handle_report(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  // We expect a frame length of 3 since the bitmask is supposed to be on a single byte
  if (frame_length != 3) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  humidity_control_setpoint_supported_types_t supported_types = frame_data[2];

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  attribute_store_set_child_reported(endpoint_node,
                                     ATTRIBUTE(SUPPORTED_TYPES),
                                     &supported_types,
                                     sizeof(supported_types));

  // First remove all existing type nodes
  // This is done to easily update the bitmask or bitmask interpretation so we don't have any leftovers.
  remove_all_humidity_setpoint_type_attributes(endpoint_node);
  // Then create all type nodes
  create_all_supported_humidity_setpoint_type(endpoint_node, supported_types);

  return SL_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Humidity Control SetPoint Capabilities Get/Report
/////////////////////////////////////////////////////////////////////////////
static sl_status_t
  zwave_command_class_humidity_control_setpoint_capabilities_get(
    attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  humidity_control_setpoint_type_t setpoint_type
    = get_associated_setpoint_type(node);

  if (setpoint_type == 0) {
    sl_log_error(LOG_TAG, "Can't get setpoint type for GET Capabilities.");
    return SL_STATUS_NOT_SUPPORTED;
  }

  ZW_HUMIDITY_CONTROL_SETPOINT_CAPABILITIES_GET_FRAME *get_frame
    = (ZW_HUMIDITY_CONTROL_SETPOINT_CAPABILITIES_GET_FRAME *)frame;
  get_frame->cmdClass    = COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT;
  get_frame->cmd         = HUMIDITY_CONTROL_SETPOINT_CAPABILITIES_GET;
  get_frame->properties1 = setpoint_type;
  *frame_length = sizeof(ZW_HUMIDITY_CONTROL_SETPOINT_CAPABILITIES_GET_FRAME);
  return SL_STATUS_OK;
}

sl_status_t
  zwave_command_class_humidity_control_setpoint_capabilities_handle_report(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  if (frame_length < 5) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  humidity_control_setpoint_type_t setpoint_type
    = frame_data[2]
      & HUMIDITY_CONTROL_SETPOINT_CAPABILITIES_REPORT_PROPERTIES1_SETPOINT_TYPE_MASK;

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  attribute_store_node_t setpoint_node
    = get_setpoint_node(endpoint_node, setpoint_type);

  if (setpoint_node == ATTRIBUTE_STORE_INVALID_NODE) {
    sl_log_error(
      LOG_TAG,
      "Can't get setpoint type node for setpoint capabilities report");
    return SL_STATUS_NOT_SUPPORTED;
  }

  const uint8_t current_index = 3;
  uint8_t byte_read_count     = interpret_value_from_report(setpoint_node,
                                                        SETPOINT_MIN_VALUE,
                                                        frame_data,
                                                        current_index);

  // If we didn't read anything or if we will go OOB
  if (byte_read_count == 0
      || (current_index + byte_read_count + 1) >= frame_length) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  byte_read_count
    = interpret_value_from_report(setpoint_node,
                                  SETPOINT_MAX_VALUE,
                                  frame_data,
                                  current_index + byte_read_count);

  if (byte_read_count == 0) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  return SL_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Humidity Control SetPoint Capabilities Set/Get/Report
/////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_humidity_control_setpoint_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  humidity_control_setpoint_type_t setpoint_type
    = get_associated_setpoint_type(node);

  if (setpoint_type == 0) {
    sl_log_error(LOG_TAG, "Can't get setpoint type for SET.");
    return SL_STATUS_NOT_SUPPORTED;
  }

  humidity_control_setpoint_value_t desired_value;
  attribute_store_get_desired(node, &desired_value, sizeof(desired_value));

  attribute_store_node_t precision_node
    = attribute_store_get_first_child_by_type(node, ATTRIBUTE(VALUE_PRECISION));

  humidity_control_setpoint_precision_t precision = 0;
  sl_status_t status
    = attribute_store_get_desired_else_reported(precision_node,
                                                &precision,
                                                sizeof(precision));
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Can't get precision value for SET.");
    return SL_STATUS_NOT_SUPPORTED;
  }

  attribute_store_node_t setpoint_node = attribute_store_get_node_parent(node);
  if (!check_setpoint_value_bounds(setpoint_node, desired_value, precision)) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Can't get precision for SET.");
    return SL_STATUS_NOT_SUPPORTED;
  }

  humidity_control_setpoint_scale_t scale;
  attribute_store_node_t scale_node
    = attribute_store_get_first_child_by_type(node, ATTRIBUTE(VALUE_SCALE));

  status = attribute_store_get_desired_else_reported(scale_node,
                                                     &scale,
                                                     sizeof(scale));

  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Can't get scale value for SET.");
    return SL_STATUS_NOT_SUPPORTED;
  }

  uint8_t value_properties_field = (precision << 5) | (scale << 3);

  if (abs(desired_value) < INT8_MAX) {
    ZW_HUMIDITY_CONTROL_SETPOINT_SET_1BYTE_FRAME *set_frame
      = (ZW_HUMIDITY_CONTROL_SETPOINT_SET_1BYTE_FRAME *)frame;
    set_frame->cmdClass    = COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT;
    set_frame->cmd         = HUMIDITY_CONTROL_SETPOINT_SET;
    set_frame->properties1 = setpoint_type;
    set_frame->properties2 = value_properties_field | 1;

    set_frame->value1 = (uint8_t)(desired_value & 0x000000FF);
    *frame_length     = sizeof(ZW_HUMIDITY_CONTROL_SETPOINT_SET_1BYTE_FRAME);

  } else if (abs(desired_value) < INT16_MAX) {
    ZW_HUMIDITY_CONTROL_SETPOINT_SET_2BYTE_FRAME *set_frame
      = (ZW_HUMIDITY_CONTROL_SETPOINT_SET_2BYTE_FRAME *)frame;
    set_frame->cmdClass    = COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT;
    set_frame->cmd         = HUMIDITY_CONTROL_SETPOINT_SET;
    set_frame->properties1 = setpoint_type;
    set_frame->properties2 = value_properties_field | 2;

    set_frame->value1 = (uint8_t)((desired_value & 0x0000FF00) >> 8);
    set_frame->value2 = (uint8_t)(desired_value & 0x000000FF);

    *frame_length = sizeof(ZW_HUMIDITY_CONTROL_SETPOINT_SET_2BYTE_FRAME);
  } else if (abs(desired_value) < INT32_MAX) {
    ZW_HUMIDITY_CONTROL_SETPOINT_SET_4BYTE_FRAME *set_frame
      = (ZW_HUMIDITY_CONTROL_SETPOINT_SET_4BYTE_FRAME *)frame;
    set_frame->cmdClass    = COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT;
    set_frame->cmd         = HUMIDITY_CONTROL_SETPOINT_SET;
    set_frame->properties1 = setpoint_type;
    set_frame->properties2 = value_properties_field | 4;

    set_frame->value1 = (uint8_t)((desired_value & 0xFF000000) >> 24);  // MSB
    set_frame->value2 = (uint8_t)((desired_value & 0x00FF0000) >> 16);
    set_frame->value3 = (uint8_t)((desired_value & 0x0000FF00) >> 8);
    set_frame->value4 = (uint8_t)(desired_value & 0x000000FF);  // LSB

    *frame_length = sizeof(ZW_HUMIDITY_CONTROL_SETPOINT_SET_4BYTE_FRAME);
  } else {
    sl_log_error(LOG_TAG, "Invalid desired value size");
    return SL_STATUS_NOT_SUPPORTED;
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_humidity_control_setpoint_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  humidity_control_setpoint_type_t setpoint_type
    = get_associated_setpoint_type(node);

  if (setpoint_type == 0) {
    sl_log_error(LOG_TAG, "Can't get setpoint type for GET.");
    return SL_STATUS_NOT_SUPPORTED;
  }

  ZW_HUMIDITY_CONTROL_SETPOINT_GET_FRAME *set_frame
    = (ZW_HUMIDITY_CONTROL_SETPOINT_GET_FRAME *)frame;
  set_frame->cmdClass    = COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT;
  set_frame->cmd         = HUMIDITY_CONTROL_SETPOINT_GET;
  set_frame->properties1 = setpoint_type;
  *frame_length          = sizeof(ZW_HUMIDITY_CONTROL_SETPOINT_GET_FRAME);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_humidity_control_setpoint_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length < 5) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  humidity_control_setpoint_type_t setpoint_type
    = frame_data[2]
      & HUMIDITY_CONTROL_SETPOINT_REPORT_PROPERTIES1_SETPOINT_TYPE_MASK;

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  attribute_store_node_t setpoint_node
    = get_setpoint_node(endpoint_node, setpoint_type);

  if (setpoint_node == ATTRIBUTE_STORE_INVALID_NODE) {
    sl_log_error(LOG_TAG, "Can't get setpoint type node for setpoint report");
    return SL_STATUS_NOT_SUPPORTED;
  }

  const uint8_t current_index = 3;
  uint8_t byte_read_count     = interpret_value_from_report(setpoint_node,
                                                        SETPOINT_CURRENT_VALUE,
                                                        frame_data,
                                                        current_index);

  // If we didn't read anything
  if (byte_read_count == 0) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  return SL_STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Class logic
/////////////////////////////////////////////////////////////////////////////

// Control handler
sl_status_t zwave_command_class_humidity_control_setpoint_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case HUMIDITY_CONTROL_SETPOINT_SUPPORTED_REPORT:
      return zwave_command_class_humidity_control_setpoint_supported_handle_report(
        connection_info,
        frame_data,
        frame_length);
    case HUMIDITY_CONTROL_SETPOINT_CAPABILITIES_REPORT:
      return zwave_command_class_humidity_control_setpoint_capabilities_handle_report(
        connection_info,
        frame_data,
        frame_length);
    case HUMIDITY_CONTROL_SETPOINT_REPORT:
      return zwave_command_class_humidity_control_setpoint_handle_report(
        connection_info,
        frame_data,
        frame_length);
    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

// Entry point
sl_status_t zwave_command_class_humidity_control_setpoint_init()
{
  attribute_store_register_callback_by_type(
    &zwave_command_class_humidity_control_setpoint_on_version_attribute_update,
    ATTRIBUTE(VERSION));

  attribute_resolver_register_rule(
    ATTRIBUTE(SUPPORTED_TYPES),
    NULL,
    &zwave_command_class_humidity_control_setpoint_supported_types_get);

  // Only monitor one of ATTRIBUTE(MIN_VALUE) or ATTRIBUTE(MAX_VALUE) we don't need both
  attribute_resolver_register_rule(
    ATTRIBUTE(MIN_VALUE),
    NULL,
    &zwave_command_class_humidity_control_setpoint_capabilities_get);

  // Only monitor one of ATTRIBUTE(MIN_VALUE) or ATTRIBUTE(MAX_VALUE) we don't need both
  attribute_resolver_register_rule(
    ATTRIBUTE(VALUE),
    &zwave_command_class_humidity_control_setpoint_set,
    &zwave_command_class_humidity_control_setpoint_get);

  // Scale limits
  attribute_store_register_callback_by_type(
    &zwave_command_class_humidity_control_setpoint_on_scale_attribute_update,
    ATTRIBUTE(VALUE_SCALE));
  attribute_store_register_callback_by_type(
    &zwave_command_class_humidity_control_setpoint_on_scale_attribute_update,
    ATTRIBUTE(MIN_VALUE_SCALE));
  attribute_store_register_callback_by_type(
    &zwave_command_class_humidity_control_setpoint_on_scale_attribute_update,
    ATTRIBUTE(MAX_VALUE_SCALE));

  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler
    = zwave_command_class_humidity_control_setpoint_control_handler;
  handler.minimal_scheme             = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.manual_security_validation = false;
  handler.command_class              = COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT;
  handler.version                    = 2;
  handler.command_class_name         = "Humidity Control SetPoint";
  handler.comments = "Experimental. Supported Scale command not supported "
                     "since the report doesn't contain Setpoint type.";

  return zwave_command_handler_register_handler(handler);
}