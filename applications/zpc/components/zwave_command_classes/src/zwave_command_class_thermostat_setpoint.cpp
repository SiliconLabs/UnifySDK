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

// Includes from this component
#include "zwave_command_class_thermostat_setpoint.h"
#include "zwave_command_class_thermostat_setpoint_types.h"
#include "zwave_command_classes_utils.h"

// Includes from other components
#include "sl_log.h"
#include "sl_status.h"
#include "zwave_command_class_indices.h"
#include "ZW_classcmd.h"
#include "zwave_command_handler.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"

#include "attribute_store_type_registration.h"
#include "attribute_store_helper.h"
#include "attribute_store.h"
#include "attribute_resolver.h"

// Generic includes
#include <assert.h>
#include <map>
#include <vector>
#include <cmath>

// Log define
#define LOG_TAG "zwave_command_class_thermostat_setpoint"

#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_##type

// Max values in Thermostat Setpoint Supported Report values
constexpr uint8_t MAX_SUPPORTED_SETPOINT_MODES = 16;
///////////////////////////////////////////////////////////////////////////////
// Data type
///////////////////////////////////////////////////////////////////////////////

// Used to describe a zwave setpoint value
struct zwave_setpoint_value {
  thermostat_setpoint_scale_t scale;
  thermostat_setpoint_precision_t precision;
  thermostat_setpoint_value_t value;

  bool is_valid = false;
};

///////////////////////////////////////////////////////////////////////////////
// Misc
///////////////////////////////////////////////////////////////////////////////
/**
   * @brief Log the setpoint value.
   * 
   * @param message  Message to be logged
   * @param setpoint_value  Setpoint value to be logged
   */
void log_setpoint_value(const char *message,
                        const zwave_setpoint_value &setpoint_value)
{
  sl_log_debug(LOG_TAG,
               "%s: value = %d, scale = %d, precision = %d",
               message,
               setpoint_value.value,
               setpoint_value.scale,
               setpoint_value.precision);
}

///////////////////////////////////////////////////////////////////////////////
// Capabilities check function
///////////////////////////////////////////////////////////////////////////////
bool is_thermostat_setpoint_mode_compatible_with_version(
  uint8_t supported_setpoint_mode_type, zwave_cc_version_t current_version)
{
  bool compatibility = false;

  switch (current_version) {
    case 1:
      compatibility = (supported_setpoint_mode_type <= 0x0A);
      break;
    case 2:
      compatibility = (supported_setpoint_mode_type <= 0x0D);
      break;
    case 3:
      compatibility = (supported_setpoint_mode_type <= 0x0F);
      break;
    default:
      compatibility = false;
  }

  if (!compatibility) {
    sl_log_warning(LOG_TAG,
                   "Thermostat SetPoint mode %#04x is not compatible with "
                   "Thermostat SetPoint Version %d",
                   supported_setpoint_mode_type,
                   current_version);
  }

  return compatibility;
}

bool is_using_b_interpretation(
  attribute_store_node_t supported_setpoint_types_node)
{
  uint8_t use_b_interpretation = 0;
  attribute_store_get_child_reported(supported_setpoint_types_node,
                                     ATTRIBUTE(USE_B_INTERPRETATION),
                                     &use_b_interpretation,
                                     sizeof(use_b_interpretation));

  return use_b_interpretation > 0;
}

///////////////////////////////////////////////////////////////////////////////
// Attribute creation functions
///////////////////////////////////////////////////////////////////////////////
attribute_store_node_t zwave_command_class_thermostat_setpoint_create_type(
  attribute_store_node_t endpoint_node, uint8_t type)
{
  zwave_cc_version_t current_version;
  attribute_store_get_child_reported(endpoint_node,
                                     ATTRIBUTE(VERSION),
                                     &current_version,
                                     sizeof(current_version));

  // Check compatibility
  if (!is_thermostat_setpoint_mode_compatible_with_version(type,
                                                           current_version)) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  sl_log_debug(LOG_TAG, "Creating setpoint type %d", type);

  // Create a new node if it doesn't exists
  attribute_store_node_t type_node = attribute_store_emplace(endpoint_node,
                                                             ATTRIBUTE(TYPE),
                                                             &type,
                                                             sizeof(type));

  // We only create MIN_VALUE since it will trigger the creation of the other
  // attributes thanks to the get capabilities command. If version < 3 and this
  // command is not supported, we will set default values.
  const attribute_store_type_t additional_nodes[] = {ATTRIBUTE(MIN_VALUE)};

  attribute_store_add_if_missing(type_node,
                                 additional_nodes,
                                 COUNT_OF(additional_nodes));
  return type_node;
}

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Get the double representation of a setpoint value (based on the precision)
 * 
 * @param setpoint_value  Setpoint value (scale is ignored)
 * 
 * @return double Current value based on the precision
 */
double get_double_setpoint_value(const zwave_setpoint_value &setpoint_value)
{
  double value = setpoint_value.value;
  for (uint8_t i = 0; i < setpoint_value.precision; i++) {
    value /= 10;
  }

  return value;
}

/**
 * @brief Get node associated with setpoint type
 * 
 * @param endpoint_node  Endpoint node
 * @param setpoint_type  Setpoint type
 * 
 * @return attribute_store_node_t Node associated with the setpoint type, or ATTRIBUTE_STORE_INVALID_NODE if not found
 */
attribute_store_node_t
  get_setpoint_type_node(attribute_store_node_t endpoint_node,
                         thermostat_setpoint_type_t setpoint_type)
{
  attribute_store_node_t setpoint_type_node
    = attribute_store_get_node_child_by_value(endpoint_node,
                                              ATTRIBUTE(TYPE),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&setpoint_type,
                                              sizeof(setpoint_type),
                                              0);

  if (setpoint_type_node == ATTRIBUTE_STORE_INVALID_NODE) {
    sl_log_warning(LOG_TAG, "Can't find setpoint type : %d", setpoint_type);
  }

  return setpoint_type_node;
}

/**
 * @brief Set setpoint value structure associated with a setpoint type (can set value, min value or max value)
 * 
 * @param setpoint_type_node  Setpoint type node, must exists
 * @param value_node_type     Value node type (one of ATTRIBUTE(VALUE), ATTRIBUTE(MIN_VALUE), ATTRIBUTE(MAX_VALUE))
 * @param value_node_state    Value node state (REPORTED_ATTRIBUTE or DESIRED_ATTRIBUTE)
 * 
 * @return zwave_setpoint_value struct. Check is_valid to know if the values are valid
 */
sl_status_t set_setpoint_value(attribute_store_node_t setpoint_type_node,
                               attribute_store_type_t value_node_type,
                               const zwave_setpoint_value &setpoint_value)
{
  sl_status_t status = attribute_store_set_child_reported(
    setpoint_type_node,
    value_node_type + SETPOINT_SCALE_ATTRIBUTE_ID_OFFSET,
    &setpoint_value.scale,
    sizeof(setpoint_value.scale));
  status |= attribute_store_set_child_reported(
    setpoint_type_node,
    value_node_type + SETPOINT_PRECISION_ATTRIBUTE_ID_OFFSET,
    &setpoint_value.precision,
    sizeof(setpoint_value.precision));

  // Set value last so we have an precision and scale for this value
  status = attribute_store_set_child_reported(setpoint_type_node,
                                              value_node_type,
                                              &setpoint_value.value,
                                              sizeof(setpoint_value.value));

  return status;
}

/**
 * @brief Set setpoint value structure associated with a setpoint type (can set value, min value or max value)
 * 
 * @param endpoint_node  Endpoint node
 * @param setpoint_type  Setpoint type, must exists
 * @param value_node_type  Value node type (one of ATTRIBUTE(VALUE), ATTRIBUTE(MIN_VALUE), ATTRIBUTE(MAX_VALUE))
 * @param value_node_state  Value node state (REPORTED_ATTRIBUTE or DESIRED_ATTRIBUTE)
 * 
 * @return zwave_setpoint_value struct. Check is_valid to know if the values are valid
 */
sl_status_t set_setpoint_value(attribute_store_node_t endpoint_node,
                               thermostat_setpoint_type_t setpoint_type,
                               attribute_store_type_t value_node_type,
                               const zwave_setpoint_value &setpoint_value)
{
  attribute_store_node_t setpoint_type_node
    = get_setpoint_type_node(endpoint_node, setpoint_type);

  if (setpoint_type_node == ATTRIBUTE_STORE_INVALID_NODE) {
    return SL_STATUS_FAIL;
  }

  return set_setpoint_value(setpoint_type_node,
                            value_node_type,
                            setpoint_value);
}

/**
 * @brief Get setpoint value structure associated with a setpoint type (can get value, min value or max value)
 * 
 * @param setpoint_type_node  Setpoint type node, must exists
 * @param value_node_type  Value node type (one of ATTRIBUTE(VALUE), ATTRIBUTE(MIN_VALUE), ATTRIBUTE(MAX_VALUE))
 * @param value_node_state  Value node state of VALUE attribute (precision & scale will always be reported) (REPORTED_ATTRIBUTE or DESIRED_ATTRIBUTE)
 * 
 * @return zwave_setpoint_value struct. Check is_valid to know if the values are valid
 */
static zwave_setpoint_value
  get_setpoint_value(attribute_store_node_t setpoint_type_node,
                     attribute_store_type_t value_node_type,
                     attribute_store_node_value_state_t value_node_state
                     = REPORTED_ATTRIBUTE)
{
  zwave_setpoint_value setpoint_value;
  setpoint_value.is_valid = false;

  if (value_node_type != ATTRIBUTE(VALUE)
      && value_node_type != ATTRIBUTE(MIN_VALUE)
      && value_node_type != ATTRIBUTE(MAX_VALUE)) {
    sl_log_critical(LOG_TAG,
                    "Can't get setpoint value. Invalid attribute type.");
    return setpoint_value;
  }

  if (setpoint_type_node == ATTRIBUTE_STORE_INVALID_NODE) {
    return setpoint_value;
  }

  auto value_node = attribute_store_get_first_child_by_type(setpoint_type_node,
                                                            value_node_type);

  sl_status_t status = attribute_store_read_value(value_node,
                                                  value_node_state,
                                                  &setpoint_value.value,
                                                  sizeof(setpoint_value.value));

  status |= attribute_store_get_child_reported(
    setpoint_type_node,
    value_node_type + SETPOINT_SCALE_ATTRIBUTE_ID_OFFSET,
    &setpoint_value.scale,
    sizeof(setpoint_value.scale));
  status |= attribute_store_get_child_reported(
    setpoint_type_node,
    value_node_type + SETPOINT_PRECISION_ATTRIBUTE_ID_OFFSET,
    &setpoint_value.precision,
    sizeof(setpoint_value.precision));

  if (status == SL_STATUS_OK) {
    setpoint_value.is_valid = true;
  }

  return setpoint_value;
}

/**
 * @brief Checks the value of a setpoint and validate if it is within the valid range.
 *
 * @note In case of errors (e.g. min/max not valid, scale not aligned,...) it will not perform any check and return the value in setpoint_value_struct
 * 
 * @param setpoint_value_struct Setpoint value to check
 * @param setpoint_min_struct Minimum value of the setpoint
 * @param setpoint_max_struct Maximum value of the setpoint
 * 
 * @returns thermostat_setpoint_value_t Valid value based on the capabilites. 
 */
thermostat_setpoint_value_t
  adjust_setpoint_value_if_needed(zwave_setpoint_value setpoint_value_struct,
                                  zwave_setpoint_value setpoint_min_struct,
                                  zwave_setpoint_value setpoint_max_struct)
{
  // Get the value to set
  thermostat_setpoint_value_t value_to_set = setpoint_value_struct.value;

  if (!setpoint_value_struct.is_valid) {
    sl_log_error(
      LOG_TAG,
      "Setpoint value is not reported as valid. No check will be performed");
    return value_to_set;
  }

  if (!setpoint_min_struct.is_valid) {
    sl_log_warning(
      LOG_TAG,
      "Can't perform check on setpoint value. Min value are not valid.");
    return value_to_set;
  }

  if (!setpoint_max_struct.is_valid) {
    sl_log_warning(
      LOG_TAG,
      "Can't perform check on setpoint value. Max value are not valid.");
    return value_to_set;
  }

  // Check for unit mismatch
  if (setpoint_min_struct.scale != setpoint_max_struct.scale
      || setpoint_max_struct.scale != setpoint_value_struct.scale) {
    sl_log_info(LOG_TAG,
                "Max, min and current value doesn't have the same unit base. "
                "No check will be performed on the value sent.");
    return value_to_set;
  }

  // Perform check
  double current_min_value = get_double_setpoint_value(setpoint_min_struct);
  double current_max_value = get_double_setpoint_value(setpoint_max_struct);
  double current_value     = get_double_setpoint_value(setpoint_value_struct);

  // Lower bound validation:
  if (current_value < current_min_value) {
    sl_log_info(LOG_TAG,
                "Attempting to set a setpoint (%f) lower than the "
                "minimum (%f). Using the minimum value.",
                current_value,
                current_min_value);

    if (setpoint_min_struct.precision == setpoint_value_struct.precision) {
      return setpoint_min_struct.value;
    } else {
      double coeff = pow(
        10,
        (setpoint_value_struct.precision - setpoint_min_struct.precision));
      thermostat_setpoint_value_t new_value = setpoint_min_struct.value * coeff;
      sl_log_info(LOG_TAG,
                  "Value and minimum value doesn't have the same precision "
                  "adjusting value to %d instead of %d",
                  new_value,
                  setpoint_min_struct.value);
      return new_value;
    }
  }

  // Upper bound validation:
  if (current_value > current_max_value) {
    sl_log_debug(LOG_TAG,
                 "Attempting to set a setpoint (%f) higher than the "
                 "maximum (%f). Using the maximum value.",
                 current_value,
                 current_max_value);

    if (setpoint_max_struct.precision == setpoint_value_struct.precision) {
      return setpoint_max_struct.value;
    } else {
      double coeff = pow(
        10,
        (setpoint_value_struct.precision - setpoint_max_struct.precision));
      thermostat_setpoint_value_t new_value = setpoint_max_struct.value * coeff;
      sl_log_info(LOG_TAG,
                  "Value and maximum value doesn't have the same precision "
                  "adjusting value to %d instead of %d",
                  new_value,
                  setpoint_max_struct.value);
      return new_value;
    }
  }

  return value_to_set;
}

/**
 * @brief Remove all ATTRIBUTE(TYPE) from attribute store associated with the given endpoint node
 * 
 * @param endpoint_node  Endpoint node
 */
void remove_all_thermostat_setpoint_type_attributes(
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

/**
 * @brief Create ATTRIBUTE(TYPE) based on bitmask in supported_types_bitmask
 * 
 * @param endpoint_node  Endpoint node
 * @param supported_types_bitmask  Bitmask of supported types
 * @param using_b_interpretation  If true, we use the B interpretation of the bitmask
 */

void create_all_supported_setpoint_type(attribute_store_node_t endpoint_node,
                                        uint32_t supported_types_bitmask,
                                        bool using_b_interpretation)
{
  // Contains current bit tested
  uint32_t setpoint_mode_current_bit = 0x0;
  // Start with 1 since bit 0 is N/A
  for (uint8_t i = 1; i < MAX_SUPPORTED_SETPOINT_MODES; i++) {
    setpoint_mode_current_bit = 1 << i;
    setpoint_mode_current_bit &= supported_types_bitmask;
    uint8_t current_type;

    // This bit is marked as not supported so we continue
    if (setpoint_mode_current_bit == 0) {
      continue;
      // THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_COOLING_1 && THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_HEATING_1
      // are the same interpretation.
    } else if (i <= THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_COOLING_1
               || using_b_interpretation) {
      current_type = i;
    } else {  // Shift 4 for standard interpretation for all values after 0x02
      current_type = i + 4;
    }

    // Those values are undefined so we ignore them
    if (current_type >= THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_NOT_SUPPORTED1
        && current_type
             <= THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_NOT_SUPPORTED4) {
      continue;
    }

    // If we are here that means we want to create a setpoint type
    zwave_command_class_thermostat_setpoint_create_type(endpoint_node,
                                                        current_type);
  }
}

/**
 * @brief Create all setpoint type attributes based on the supported setpoint types bitmask
 * 
 * @note This function will use is_using_b_interpretation to interpret the bitmask
 * 
 * Create the structure : 
 * |___SETPOINT_TYPE
 *    |___VALUE
 *    |___VALUE_SCALE
 *    |___VALUE_PRECISION
 *    |___MIN_VALUE
 *    |___MIN_VALUE_SCALE
 *    |___MIN_VALUE_PRECISION
 *    |___MAX_VALUE
 *    |___MAX_VALUE_SCALE
 *    |___MAX_VALUE_PRECISION
 * |___SETPOINT_TYPE
 *    |___VALUE
 *    |___VALUE_SCALE
 *    |___VALUE_PRECISION
 *    |___MIN_VALUE
 *    |___MIN_VALUE_SCALE
 *    |___MIN_VALUE_PRECISION
 *    |___MAX_VALUE
 *    |___MAX_VALUE_SCALE
 *    |___MAX_VALUE_PRECISION
 * |___...
 * 
 * @param endpoint_node  Endpoint node
 * 
 * @return SL_STATUS_OK if all attributes are created, SL_STATUS_IS_WAITING if some attributes are not ready yet
 */
sl_status_t
  create_setpoint_type_attributes(attribute_store_node_t endpoint_node)
{
  attribute_store_node_t supported_setpoint_types_node
    = attribute_store_get_first_child_by_type(
      endpoint_node,
      ATTRIBUTE(SUPPORTED_SETPOINT_TYPES));

  // We might not be ready yet, no worries
  if (supported_setpoint_types_node == ATTRIBUTE_STORE_INVALID_NODE) {
    return SL_STATUS_IS_WAITING;
  }

  // Reported bitmask
  uint32_t supported_setpoint_mode_bitmask;
  sl_status_t status
    = attribute_store_get_reported(supported_setpoint_types_node,
                                   &supported_setpoint_mode_bitmask,
                                   sizeof(supported_setpoint_mode_bitmask));

  // We might not be ready yet, no worries
  if (status != SL_STATUS_OK) {
    return SL_STATUS_IS_WAITING;
  }

  // First remove all existing type node
  // This is done to easily update the bitmask or bitmask interpretation so
  // we don't have any leftovers.
  remove_all_thermostat_setpoint_type_attributes(endpoint_node);

  // Check if we need to use B Interpretation
  const bool using_b_interpretation
    = is_using_b_interpretation(supported_setpoint_types_node);
  // Then create attributed based on supported types
  create_all_supported_setpoint_type(endpoint_node,
                                     supported_setpoint_mode_bitmask,
                                     using_b_interpretation);
  return SL_STATUS_OK;
}

/**
 * @brief Create ATTRIBUTE(VALUE) under the setpoint type node if it doesn't exists
 * 
 * @note This will trigger the Setpoint Get for this attribute
 * 
 * @param setpoint_type_node  Setpoint type node
 */
void create_value_attribute(attribute_store_node_t setpoint_type_node)
{
  const attribute_store_type_t additional_nodes[] = {ATTRIBUTE(VALUE)};

  attribute_store_add_if_missing(setpoint_type_node,
                                 additional_nodes,
                                 COUNT_OF(additional_nodes));
}

/**
 * @brief Set default capabilities for a setpoint type node.
 * 
 * @note Used for version < 3 that doesn't have this information 
 *   
 * @param endpoint_node  Endpoint node
 * @param type_node  Setpoint type node
 * 
 */
void set_default_capabilities(attribute_store_node_t setpoint_type_node)
{
  const std::map<attribute_store_type_t, zwave_setpoint_value>
    default_setpoint_values = {
      {ATTRIBUTE(MIN_VALUE),
       {
         .scale     = DEFAULT_SCALE,
         .precision = DEFAULT_PRECISION,
         .value     = DEFAULT_MIN_VALUE,
         .is_valid  = true,
       }},
      {ATTRIBUTE(MAX_VALUE),
       {
         .scale     = DEFAULT_SCALE,
         .precision = DEFAULT_PRECISION,
         .value     = DEFAULT_MAX_VALUE,
         .is_valid  = true,
       }},
    };

  for (const auto &[attribute_type, setpoint_value]: default_setpoint_values) {
    set_setpoint_value(setpoint_type_node, attribute_type, setpoint_value);
  }

  // Create value attribute to trigger Setpoint Get
  create_value_attribute(setpoint_type_node);
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Callback functions
///////////////////////////////////////////////////////////////////////////////
static void
  zwave_command_class_thermostat_setpoint_on_b_interpretation_attribute_update(
    attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  if (endpoint_node == ATTRIBUTE_STORE_INVALID_NODE) {
    sl_log_warning(LOG_TAG,
                   "Can't get endpoint node associated with b interpretation "
                   "flag. Aborting generation.");
    return;
  }

  // Store flag value
  uint8_t flag;
  sl_status_t status
    = attribute_store_get_desired(updated_node, &flag, sizeof(flag));

  // No desired value so we stop here
  if (status != SL_STATUS_OK) {
    return;
  }

  // Update b interpretation value
  attribute_store_set_reported(updated_node, &flag, sizeof(flag));
  attribute_store_undefine_desired(updated_node);

  // Might fail if some attribute are not ready yet and it's ok, we'll try that later
  create_setpoint_type_attributes(endpoint_node);
}

// Zigbee data model is only accepting value with precision of 2 in °C.
// So we need to do that here
static void on_zwave_value_update(attribute_store_node_t value_node,
                                  attribute_store_change_t change)
{
  // On the creation of the attribute we don't need to update the value yet
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  // Attributes nodes
  attribute_store_node_t type_node
    = attribute_store_get_node_parent(value_node);
  attribute_store_node_t endpoint_node
    = attribute_store_get_node_parent(type_node);

  // Current attribute type
  attribute_store_type_t attribute_type
    = attribute_store_get_node_type(value_node);
  const char *attribute_type_name
    = attribute_store_get_type_name(attribute_type);
  // Set point type (debug purpose)
  thermostat_setpoint_type_t setpoint_type;
  attribute_store_get_reported(type_node,
                               &setpoint_type,
                               sizeof(setpoint_type));

  zwave_setpoint_value current_value
    = get_setpoint_value(type_node, attribute_type);

  // We don't have a value yet, it's ok waiting for one
  if (!current_value.is_valid) {
    return;
  }

  sl_log_debug(LOG_TAG,
               "on_zwave_value_update called on %s (setpoint type %d)",
               attribute_type_name,
               setpoint_type);

  // Attributes bindings
  // bind the setpoint type to their corresponding ZCL attributes
  std::map<uint8_t, std::map<attribute_store_type_t, attribute_store_type_t>>
    bindings;

  // Heating
  bindings[SETPOINT_TYPE_HEATING]
    = {{ATTRIBUTE(VALUE),
        DOTDOT_ATTRIBUTE_ID_THERMOSTAT_OCCUPIED_HEATING_SETPOINT},
       {ATTRIBUTE(MIN_VALUE),
        DOTDOT_ATTRIBUTE_ID_THERMOSTAT_MIN_HEAT_SETPOINT_LIMIT},
       {ATTRIBUTE(MAX_VALUE),
        DOTDOT_ATTRIBUTE_ID_THERMOSTAT_MAX_HEAT_SETPOINT_LIMIT}};

  // Cooling
  bindings[SETPOINT_TYPE_COOLING]
    = {{ATTRIBUTE(VALUE),
        DOTDOT_ATTRIBUTE_ID_THERMOSTAT_OCCUPIED_COOLING_SETPOINT},
       {ATTRIBUTE(MIN_VALUE),
        DOTDOT_ATTRIBUTE_ID_THERMOSTAT_MIN_COOL_SETPOINT_LIMIT},
       {ATTRIBUTE(MAX_VALUE),
        DOTDOT_ATTRIBUTE_ID_THERMOSTAT_MAX_COOL_SETPOINT_LIMIT}};

  // If Setpoint Type if found, we proceed. Otherwise it's not a big deal. It can happen and we don't really care
  if (bindings.find(setpoint_type) != bindings.end()) {
    auto attributes = bindings[setpoint_type];

    // If we don't find our attribute node we need to stop here and report the error
    // This should not happen, and if it is happening you need to update the bindings table
    if (attributes.find(attribute_type) == attributes.end()) {
      sl_log_critical(
        LOG_TAG,
        "on_zwave_value_update is bounded to an unknown attribute");
      return;
    }

    attribute_store_type_t ucl_type            = attributes[attribute_type];
    thermostat_setpoint_scale_t scale          = current_value.scale;
    thermostat_setpoint_precision_t precision  = current_value.precision;
    thermostat_setpoint_value_t setpoint_value = current_value.value;

    int16_t ucl_value
      = zwave_temperature_to_ucl_temperature(setpoint_value, precision, scale);

    // Update (or create) ucl value
    sl_status_t status = attribute_store_set_child_reported(endpoint_node,
                                                            ucl_type,
                                                            &ucl_value,
                                                            sizeof(ucl_value));

    const char *ucl_type_name = attribute_store_get_type_name(ucl_type);

    if (status != SL_STATUS_OK) {
      sl_log_warning(LOG_TAG,
                     "Can't set ucl value (%s) for attribute : %s",
                     ucl_type_name,
                     attribute_type_name);
      return;
    }

    sl_log_debug(LOG_TAG,
                 "Update reported attribute %s to : %d",
                 ucl_type_name,
                 ucl_value);

    // Once we've updated the UCL value, we set discard their desired value
    attribute_store_node_t ucl_node
      = attribute_store_get_first_child_by_type(endpoint_node, ucl_type);

    sl_log_debug(LOG_TAG, "Undefine desired value of %s", ucl_type_name);
    attribute_store_undefine_desired(ucl_node);
  }
}

// Zigbee data model is only accepting value with precision of 2 in °C.
// So we need to do that here
static void on_ucl_value_update(attribute_store_node_t ucl_value_node,
                                attribute_store_change_t change)
{
  // We don't care about attribute creation since the Z-Wave stuff is changed first
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  attribute_store_type_t node_type
    = attribute_store_get_node_type(ucl_value_node);
  const char *ucl_attribute_type_name
    = attribute_store_get_type_name(node_type);

  sl_log_debug(LOG_TAG,
               "on_ucl_value_update called on %s",
               ucl_attribute_type_name);

  thermostat_setpoint_type_t setpoint_type = 0;
  if (node_type == DOTDOT_ATTRIBUTE_ID_THERMOSTAT_OCCUPIED_HEATING_SETPOINT) {
    setpoint_type = SETPOINT_TYPE_HEATING;
  } else if (node_type
             == DOTDOT_ATTRIBUTE_ID_THERMOSTAT_OCCUPIED_COOLING_SETPOINT) {
    setpoint_type = SETPOINT_TYPE_COOLING;
  } else {
    // You should not arrive there, if it is the case check the bindings
    sl_log_critical(
      LOG_TAG,
      "Can't found setpoint type mapped with given dotdot attribute : %s",
      ucl_attribute_type_name);
    return;
  }

  attribute_store_node_t endpoint_node
    = attribute_store_get_node_parent(ucl_value_node);

  attribute_store_node_t setpoint_type_node
    = get_setpoint_type_node(endpoint_node, setpoint_type);

  if (setpoint_type_node == ATTRIBUTE_STORE_INVALID_NODE) {
    return;
  }

  auto setpoint_value
    = get_setpoint_value(setpoint_type_node, ATTRIBUTE(VALUE));

  thermostat_setpoint_precision_t precision       = setpoint_value.precision;
  thermostat_setpoint_scale_t scale               = setpoint_value.scale;
  thermostat_setpoint_value_t current_zwave_value = setpoint_value.value;

  if (!setpoint_value.is_valid) {
    sl_log_warning(LOG_TAG,
                   "Can't get setpoint value value for UCL attribute %s. Won't "
                   "update Zwave attribute",
                   ucl_attribute_type_name);
    return;
  }

  // Get the requested desired value and map it to the zwave side
  int16_t ucl_value;
  sl_status_t status = attribute_store_get_desired(ucl_value_node,
                                                   &ucl_value,
                                                   sizeof(ucl_value));
  if (status != SL_STATUS_OK) {
    sl_log_debug(LOG_TAG,
                 "Defined value of UCL attribute %s is not defined. Not "
                 "updating anything.",
                 ucl_attribute_type_name);
    return;
  }

  thermostat_setpoint_value_t new_zwave_value
    = ucl_temperature_to_zwave_temperature(ucl_value, precision, scale);

  // If we have the same converted value (e.g C° to F) we set the state ourself
  if (current_zwave_value == new_zwave_value) {
    sl_log_info(LOG_TAG,
                "Trying to set the same desired z-wave value as the reported "
                "one. No changes will be set on the device. This behavior can "
                "happen when °C is converted to F based on the device "
                "precision. Try to set more/less °C in SetpointRaiseOrLower.");
    attribute_store_set_reported_as_desired(ucl_value_node);
    attribute_store_undefine_desired(ucl_value_node);
    return;
  }

  auto zwave_value_node
    = attribute_store_get_first_child_by_type(setpoint_type_node,
                                              ATTRIBUTE(VALUE));
  status = attribute_store_set_desired(zwave_value_node,
                                       &new_zwave_value,
                                       sizeof(new_zwave_value));

  if (status != SL_STATUS_OK) {
    sl_log_warning(
      LOG_TAG,
      "Can't set desired setpoint value (zwave) for UCL attribute %s. "
      "Won't update Zwave attribute",
      ucl_attribute_type_name);
    return;
  }

  sl_log_debug(LOG_TAG,
               "Update desired Z-Wave Value for setpoint type %d to : %d",
               setpoint_type,
               new_zwave_value);
}

static void zwave_command_class_thermostat_setpoint_on_version_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  if (is_zwave_command_class_filtered_for_root_device(
        COMMAND_CLASS_THERMOSTAT_SETPOINT_V3,
        updated_node)
      == true) {
    return;
  }

  // Check that we have the right type of attribute.
  assert(ATTRIBUTE(VERSION) == attribute_store_get_node_type(updated_node));

  uint8_t supporting_node_version = 0;
  attribute_store_read_value(updated_node,
                             REPORTED_ATTRIBUTE,
                             &supporting_node_version,
                             sizeof(supporting_node_version));

  if (supporting_node_version == 0) {
    // Wait for the version to be known.
    return;
  }

  // Now we know we have a supporting node.
  // Just check that the suported setpoint type attribute is present.
  attribute_store_node_t endpoint_node
    = attribute_store_get_node_parent(updated_node);

  // Let the rest of the command class perform the job.
  attribute_store_type_t supported_sensor_types[]
    = {ATTRIBUTE(SUPPORTED_SETPOINT_TYPES)};

  attribute_store_add_if_missing(endpoint_node,
                                 supported_sensor_types,
                                 COUNT_OF(supported_sensor_types));

  // Create B interpretation flag and set default value
  // It is located underneath ATTRIBUTE(SUPPORTED_SETPOINT_TYPES)
  attribute_store_type_t supported_node
    = attribute_store_get_node_child_by_type(
      endpoint_node,
      ATTRIBUTE(SUPPORTED_SETPOINT_TYPES),
      0);

  // Create the attribute and set default value if not already there
  if (attribute_store_get_node_child_by_type(supported_node,
                                             ATTRIBUTE(USE_B_INTERPRETATION),
                                             0)
      == ATTRIBUTE_STORE_INVALID_NODE) {
    // Default value for b interpretation (off)
    uint8_t default_b_interpretation_value = 0;
    // Will create the node if not existant
    attribute_store_set_child_reported(supported_node,
                                       ATTRIBUTE(USE_B_INTERPRETATION),
                                       &default_b_interpretation_value,
                                       sizeof(default_b_interpretation_value));
  }
}

///////////////////////////////////////////////////////////////////////////////
// Thermostat Setpoint Supported Get/Report
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_thermostat_setpoint_supported_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Check that we have the right type of attribute.
  assert(ATTRIBUTE(SUPPORTED_SETPOINT_TYPES)
         == attribute_store_get_node_type(node));

  // Default frame length in case of error
  *frame_length = 0;

  sl_log_debug(LOG_TAG, "Thermostat Setpoint Supported Get");

  // Supported Get is the same for all versions.
  ZW_THERMOSTAT_SETPOINT_SUPPORTED_GET_V3_FRAME *supported_get_frame
    = (ZW_THERMOSTAT_SETPOINT_SUPPORTED_GET_V3_FRAME *)frame;

  supported_get_frame->cmdClass = COMMAND_CLASS_THERMOSTAT_SETPOINT_V3;
  supported_get_frame->cmd      = THERMOSTAT_SETPOINT_SUPPORTED_GET_V3;

  *frame_length = sizeof(ZW_THERMOSTAT_SETPOINT_SUPPORTED_GET_V3_FRAME);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_thermostat_setpoint_handle_supported_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // We expect to have at least 1 bitmask
  if (frame_length <= SUPPORTED_REPORT_BITMASK_INDEX) {
    return SL_STATUS_FAIL;
  }

  // First up, save our supported types in the Attribute Store.
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  attribute_store_node_t supported_bitmask_node
    = attribute_store_get_first_child_by_type(
      endpoint_node,
      ATTRIBUTE(SUPPORTED_SETPOINT_TYPES));

  uint32_t supported_setpoint_mode_bits = 0x0000;
  uint8_t bitmask_length                = frame_length - 2;

  // Since we are using uint32_t we can't have more that 4 bit mask
  if (bitmask_length > 4) {
    sl_log_error(
      LOG_TAG,
      "Supported Thermostat SetPoint type Bit Mask length is not supported\n");
    return SL_STATUS_FAIL;
  }

  for (int i = bitmask_length; i > 0; i--) {
    supported_setpoint_mode_bits
      = (supported_setpoint_mode_bits << 8) | frame_data[1 + i];
  }

  attribute_store_set_reported(supported_bitmask_node,
                               &supported_setpoint_mode_bits,
                               sizeof(supported_setpoint_mode_bits));

  sl_log_debug(LOG_TAG,
               "Thermostat Setpoint Supported Report : %b",
               supported_setpoint_mode_bits);

  return create_setpoint_type_attributes(endpoint_node);
}

///////////////////////////////////////////////////////////////////////////////
// Thermostat Setpoint Set/Get/Report
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_thermostat_setpoint_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Check that we have the right type of attribute.
  assert(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VALUE
         == attribute_store_get_node_type(node));

  // Default frame length in case of error
  *frame_length = 0;

  // Get version
  zwave_cc_version_t supporting_node_version
    = zwave_command_class_get_version_from_node(
      node,
      COMMAND_CLASS_THERMOSTAT_SETPOINT);

  attribute_store_node_t type_node = attribute_store_get_node_parent(node);
  // Get setpoint type
  thermostat_setpoint_type_t setpoint_type = 0;
  sl_status_t status = attribute_store_get_reported(type_node,
                                                    &setpoint_type,
                                                    sizeof(setpoint_type));
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Setpoint Set : Can't get setpoint type.");
    return SL_STATUS_NOT_SUPPORTED;
  }

  auto setpoint_value
    = get_setpoint_value(type_node, ATTRIBUTE(VALUE), DESIRED_ATTRIBUTE);

  if (!setpoint_value.is_valid) {
    sl_log_error(LOG_TAG, "Setpoint Set : Can't get setpoint value.");
    return SL_STATUS_NOT_SUPPORTED;
  }

  sl_log_debug(LOG_TAG,
               "Thermostat Setpoint Set for setpoint %d",
               setpoint_type);
  log_setpoint_value("Setpoint value : ", setpoint_value);

  // Reuse the same scale as current value.
  thermostat_setpoint_scale_t setpoint_value_scale = setpoint_value.scale;
  // Reuse the same precision as current value.
  thermostat_setpoint_precision_t setpoint_value_precision
    = setpoint_value.precision;
  // By default we get the desired value
  thermostat_setpoint_value_t setpoint_value_integer = setpoint_value.value;

  // See if value needs adjustments
  if (supporting_node_version > 2) {
    setpoint_value_integer = adjust_setpoint_value_if_needed(
      setpoint_value,
      get_setpoint_value(type_node, ATTRIBUTE(MIN_VALUE)),
      get_setpoint_value(type_node, ATTRIBUTE(MAX_VALUE)));
  }

  const uint8_t level2_properties_field
    = (setpoint_value_precision << 5) | (setpoint_value_scale << 3);

  if (setpoint_value_integer >= INT8_MIN
      && setpoint_value_integer <= INT8_MAX) {
    ZW_THERMOSTAT_SETPOINT_SET_1BYTE_V3_FRAME *set_frame
      = (ZW_THERMOSTAT_SETPOINT_SET_1BYTE_V3_FRAME *)frame;
    *frame_length = sizeof(ZW_THERMOSTAT_SETPOINT_SET_1BYTE_V3_FRAME);

    set_frame->cmdClass = COMMAND_CLASS_THERMOSTAT_SETPOINT_V3;
    set_frame->cmd      = THERMOSTAT_SETPOINT_SET_V3;
    set_frame->level    = setpoint_type;
    set_frame->level2   = level2_properties_field | 1;
    set_frame->value1   = (uint8_t)(setpoint_value_integer & 0x000000FF);
  } else if (setpoint_value_integer >= INT16_MIN
             && setpoint_value_integer <= INT16_MAX) {
    ZW_THERMOSTAT_SETPOINT_SET_2BYTE_V3_FRAME *set_frame
      = (ZW_THERMOSTAT_SETPOINT_SET_2BYTE_V3_FRAME *)frame;
    *frame_length = sizeof(ZW_THERMOSTAT_SETPOINT_SET_2BYTE_V3_FRAME);

    set_frame->cmdClass = COMMAND_CLASS_THERMOSTAT_SETPOINT_V3;
    set_frame->cmd      = THERMOSTAT_SETPOINT_SET_V3;
    set_frame->level    = setpoint_type;
    set_frame->level2   = level2_properties_field | 2;

    set_frame->value1 = (setpoint_value_integer & 0x0000FF00) >> 8;  // MSB
    set_frame->value2 = (setpoint_value_integer & 0x000000FF);       // LSB

  } else if (setpoint_value_integer >= INT32_MIN
             && setpoint_value_integer <= INT32_MAX) {
    ZW_THERMOSTAT_SETPOINT_SET_4BYTE_V3_FRAME *set_frame
      = (ZW_THERMOSTAT_SETPOINT_SET_4BYTE_V3_FRAME *)frame;
    *frame_length = sizeof(ZW_THERMOSTAT_SETPOINT_SET_4BYTE_V3_FRAME);

    set_frame->cmdClass = COMMAND_CLASS_THERMOSTAT_SETPOINT_V3;
    set_frame->cmd      = THERMOSTAT_SETPOINT_SET_V3;
    set_frame->level    = setpoint_type;
    set_frame->level2   = level2_properties_field | 4;

    set_frame->value1 = (setpoint_value_integer & 0xFF000000) >> 24;  // MSB
    set_frame->value2 = (setpoint_value_integer & 0x00FF0000) >> 16;
    set_frame->value3 = (setpoint_value_integer & 0x0000FF00) >> 8;
    set_frame->value4 = (setpoint_value_integer & 0x000000FF);  // LSB

    sl_log_warning(
      LOG_TAG,
      "UCL will convert this value to int16, value might be truncated.");
  } else {
    sl_log_error(LOG_TAG, "Invalid desired value size");
    return SL_STATUS_NOT_SUPPORTED;
  }

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_thermostat_setpoint_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Check that we have the right type of attribute.
  assert(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VALUE
         == attribute_store_get_node_type(node));

  // Default frame length in case of error
  *frame_length = 0;

  attribute_store_node_t type_node = attribute_store_get_node_parent(node);

  ZW_THERMOSTAT_SETPOINT_GET_V3_FRAME *get_frame
    = (ZW_THERMOSTAT_SETPOINT_GET_V3_FRAME *)frame;

  get_frame->cmdClass = COMMAND_CLASS_THERMOSTAT_SETPOINT_V3;
  get_frame->cmd      = THERMOSTAT_SETPOINT_GET_V3;
  // get_frame->level ? This is called 4 bits reserved / 4 bits setpoint type.
  attribute_store_read_value(type_node,
                             REPORTED_ATTRIBUTE,
                             &get_frame->level,
                             sizeof(get_frame->level));

  sl_log_debug(LOG_TAG,
               "Thermostat Setpoint Get on setpoint %d",
               get_frame->level);

  *frame_length = sizeof(ZW_THERMOSTAT_SETPOINT_GET_V3_FRAME);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_thermostat_setpoint_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // We expect to have at least 1 byte of value
  if (frame_length <= REPORT_VALUE_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  sl_log_debug(LOG_TAG,
               "NodeID %d:%d - Thermostat Setpoint Report",
               (int)connection_info->remote.node_id,
               (int)connection_info->remote.endpoint_id);

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  thermostat_setpoint_type_t received_type
    = frame_data[REPORT_SETPOINT_TYPE_INDEX] & SETPOINT_TYPE_MASK;

  attribute_store_node_t type_node
    = get_setpoint_type_node(endpoint_node, received_type);

  // Add guard in case we don't find it
  if (type_node == ATTRIBUTE_STORE_INVALID_NODE) {
    sl_log_warning(
      LOG_TAG,
      "Device send setpoint type %d but can't find it in the attribute store",
      received_type);
    return SL_STATUS_NOT_SUPPORTED;
  }

  zwave_cc_version_t supporting_node_version
    = zwave_command_class_get_version_from_node(
      type_node,
      COMMAND_CLASS_THERMOSTAT_SETPOINT);

  zwave_setpoint_value setpoint_value = {.is_valid = true};
  setpoint_value.scale
    = (frame_data[REPORT_PRECISION_SCALE_SIZE_INDEX] & SCALE_MASK) >> 3;
  setpoint_value.precision
    = (frame_data[REPORT_PRECISION_SCALE_SIZE_INDEX] & PRECISION_MASK) >> 5;
  uint8_t size = frame_data[REPORT_PRECISION_SCALE_SIZE_INDEX] & SIZE_MASK;
  setpoint_value.value
    = get_signed_value_from_frame_and_size(&frame_data[REPORT_VALUE_INDEX],
                                           size);

  sl_log_debug(LOG_TAG, "Setpoint type : %d", received_type);
  log_setpoint_value("Received setpoint value", setpoint_value);

  // Check reported value
  if (supporting_node_version > 2) {
    thermostat_setpoint_value_t adjusted_value
      = adjust_setpoint_value_if_needed(
        setpoint_value,
        get_setpoint_value(type_node, ATTRIBUTE(MIN_VALUE)),
        get_setpoint_value(type_node, ATTRIBUTE(MAX_VALUE)));

    if (adjusted_value != setpoint_value.value) {
      sl_log_warning(
        LOG_TAG,
        "Received value is out of bounds. Ignoring thermostat report.");
    }
  }

  return set_setpoint_value(endpoint_node,
                            received_type,
                            ATTRIBUTE(VALUE),
                            setpoint_value);
}

//////////////////////////////////////////////////////////////////////////////
// Thermostat Setpoint Capabilities Get/Report
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_thermostat_setpoint_capabilities_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Check that we have the right type of attribute.
  assert(ATTRIBUTE(MIN_VALUE) == attribute_store_get_node_type(node));
  // Default frame length in case of error
  *frame_length = 0;

  attribute_store_node_t type_node = attribute_store_get_node_parent(node);
  attribute_store_node_t endpoint_node
    = attribute_store_get_node_parent(type_node);
  attribute_store_node_t version_node = attribute_store_get_first_child_by_type(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VERSION);

  uint8_t supporting_node_version = 0;
  attribute_store_read_value(version_node,
                             REPORTED_ATTRIBUTE,
                             &supporting_node_version,
                             sizeof(supporting_node_version));

  if (supporting_node_version == 0) {
    // Something weird happened. Bail out.
    sl_log_warning(LOG_TAG, "Cannot read supporting node version.");
    return SL_STATUS_FAIL;
  }

  thermostat_setpoint_type_t setpoint_type = 0;
  sl_status_t status = attribute_store_get_reported(type_node,
                                                    &setpoint_type,
                                                    sizeof(setpoint_type));

  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Can't get setpoint type when "
                 "trying to get capabilities.");
    return SL_STATUS_NOT_SUPPORTED;
  }

  if (supporting_node_version <= 2) {
    sl_log_debug(LOG_TAG,
                 "Thermostat Setpoint set "
                 "default capabilites for setpoint %d.",
                 setpoint_type);
    // In version 1-2, they do not have a supported get.
    // here there is no substitution, we we will just return no
    // frame and fill up dummy values ourselves
    set_default_capabilities(type_node);
    return SL_STATUS_ALREADY_EXISTS;
  } else {
    sl_log_debug(LOG_TAG,
                 "Thermostat Setpoint Get Capabilities for setpoint %d.",
                 setpoint_type);

    // From version 3, we can ask nodes what they support.
    ZW_THERMOSTAT_SETPOINT_CAPABILITIES_GET_V3_FRAME
    *capabilities_get_frame
      = (ZW_THERMOSTAT_SETPOINT_CAPABILITIES_GET_V3_FRAME *)frame;

    capabilities_get_frame->cmdClass = COMMAND_CLASS_THERMOSTAT_SETPOINT_V3;
    capabilities_get_frame->cmd      = THERMOSTAT_SETPOINT_CAPABILITIES_GET_V3;
    capabilities_get_frame->properties1 = setpoint_type;

    *frame_length = sizeof(ZW_THERMOSTAT_SETPOINT_CAPABILITIES_GET_V3_FRAME);
    return SL_STATUS_OK;
  }
}

static sl_status_t
  zwave_command_class_thermostat_setpoint_handle_capabilities_report(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  // Frame too short, ignore
  if (frame_length <= CAPABILITIES_REPORT_MIN_PRECISION_SCALE_SIZE_INDEX) {
    return SL_STATUS_FAIL;
  }

  // Just retrieve the data and save it.
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  thermostat_setpoint_type_t received_setpoint_type
    = frame_data[CAPABILITIES_REPORT_SETPOINT_TYPE_INDEX] & SETPOINT_TYPE_MASK;
  auto type_node
    = get_setpoint_type_node(endpoint_node, received_setpoint_type);

  sl_log_debug(LOG_TAG,
               "Thermostat Setpoint Report Capabilities for setpoint %d.",
               received_setpoint_type);

  if (type_node == ATTRIBUTE_STORE_INVALID_NODE) {
    sl_log_warning(
      LOG_TAG,
      "Can't find setpoint type %d in the attribute store. Creating it.",
      received_setpoint_type);
    // Hmm it seems that it's a type that we don't know of.
    // Let's be nice and create it.
    zwave_command_class_thermostat_setpoint_create_type(endpoint_node,
                                                        received_setpoint_type);
  }

  auto set_frame_reported_setpoint = [&](uint8_t precision_scale_size_index,
                                         uint8_t value_index,
                                         attribute_store_type_t node_type) {
    zwave_setpoint_value setpoint_value = {.is_valid = true};

    setpoint_value.scale
      = (frame_data[precision_scale_size_index] & SCALE_MASK) >> 3;

    setpoint_value.precision
      = (frame_data[precision_scale_size_index] & PRECISION_MASK) >> 5;

    uint8_t received_value_size
      = frame_data[precision_scale_size_index] & SIZE_MASK;
    setpoint_value.value
      = get_signed_value_from_frame_and_size(&frame_data[value_index],
                                             received_value_size);

    sl_log_debug(LOG_TAG,
                 "Received setpoint value for %s",
                 attribute_store_get_type_name(node_type));
    log_setpoint_value("Setpoint value : ", setpoint_value);

    set_setpoint_value(endpoint_node,
                       received_setpoint_type,
                       node_type,
                       setpoint_value);

    return received_value_size;
  };

  // Set min value attributes
  auto received_min_value_size = set_frame_reported_setpoint(
    CAPABILITIES_REPORT_MIN_PRECISION_SCALE_SIZE_INDEX,
    CAPABILITIES_REPORT_MIN_VALUE_INDEX,
    ATTRIBUTE(MIN_VALUE));

  uint8_t max_value_precision_scale_size_index
    = CAPABILITIES_REPORT_MIN_PRECISION_SCALE_SIZE_INDEX
      + received_min_value_size + 1;
  set_frame_reported_setpoint(max_value_precision_scale_size_index,
                              max_value_precision_scale_size_index + 1,
                              ATTRIBUTE(MAX_VALUE));

  // Create value attribute to trigger Setpoint Get now that we have the value
  create_value_attribute(type_node);

  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Command Handler functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_thermostat_setpoint_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // Frame too short, it should have not come here.
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  assert(frame_data[COMMAND_CLASS_INDEX]
         == COMMAND_CLASS_THERMOSTAT_SETPOINT_V3);

  switch (frame_data[COMMAND_INDEX]) {
    case THERMOSTAT_SETPOINT_REPORT_V3:
      return zwave_command_class_thermostat_setpoint_handle_report(
        connection_info,
        frame_data,
        frame_length);

    case THERMOSTAT_SETPOINT_SUPPORTED_REPORT_V3:
      return zwave_command_class_thermostat_setpoint_handle_supported_report(
        connection_info,
        frame_data,
        frame_length);

    case THERMOSTAT_SETPOINT_CAPABILITIES_REPORT_V3:
      return zwave_command_class_thermostat_setpoint_handle_capabilities_report(
        connection_info,
        frame_data,
        frame_length);

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Init functions
///////////////////////////////////////////////////////////////////////////////

sl_status_t zwave_command_class_thermostat_setpoint_init()
{
  // Resolver functions.
  attribute_resolver_register_rule(
    ATTRIBUTE(SUPPORTED_SETPOINT_TYPES),
    NULL, // NOSONAR
    &zwave_command_class_thermostat_setpoint_supported_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(MIN_VALUE),
    NULL, // NOSONAR
    &zwave_command_class_thermostat_setpoint_capabilities_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(VALUE),
    &zwave_command_class_thermostat_setpoint_set,
    &zwave_command_class_thermostat_setpoint_get);

  // We only listen to desired attribute change, to prevent triggering this when zpc starts.
  // When zpc start it will call the update() callback on every attribute.
  // By only listening to desired attribute change, we prevent the update callback to be called and control the process.
  attribute_store_register_callback_by_type_and_state(
    &zwave_command_class_thermostat_setpoint_on_b_interpretation_attribute_update,
    ATTRIBUTE(USE_B_INTERPRETATION),
    DESIRED_ATTRIBUTE);

  // Listening for supporting nodes
  attribute_store_register_callback_by_type(
    &zwave_command_class_thermostat_setpoint_on_version_attribute_update,
    ATTRIBUTE(VERSION));

  // UAM files are not powerful enough for this case at the moment so we do that here instead
  // Mapping zwave <-> ucl
  const std::vector<attribute_store_type_t> zwave_attributes = {
    ATTRIBUTE(VALUE),
    ATTRIBUTE(MIN_VALUE),
    ATTRIBUTE(MAX_VALUE),
  };
  const std::vector<attribute_store_type_t> ucl_attributes
    = {DOTDOT_ATTRIBUTE_ID_THERMOSTAT_OCCUPIED_HEATING_SETPOINT,
       DOTDOT_ATTRIBUTE_ID_THERMOSTAT_OCCUPIED_COOLING_SETPOINT};

  for (auto &zwave_attribute: zwave_attributes) {
    attribute_store_register_callback_by_type_and_state(&on_zwave_value_update,
                                                        zwave_attribute,
                                                        REPORTED_ATTRIBUTE);
  }

  for (auto &ucl_attribute: ucl_attributes) {
    attribute_store_register_callback_by_type_and_state(&on_ucl_value_update,
                                                        ucl_attribute,
                                                        DESIRED_ATTRIBUTE);
  }

  // Register Thermostat Setpoint CC handler to the Z-Wave CC framework
  zwave_command_handler_t handler = {};
  handler.control_handler
    = &zwave_command_class_thermostat_setpoint_control_handler;
  handler.minimal_scheme             = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.command_class              = COMMAND_CLASS_THERMOSTAT_SETPOINT_V3;
  handler.version                    = THERMOSTAT_SETPOINT_VERSION_V3;
  handler.manual_security_validation = false;
  handler.command_class_name         = "Thermostat Setpoint";
  handler.comments = "Only Cool & Heat setpoint can be controlled from MQTT";

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}