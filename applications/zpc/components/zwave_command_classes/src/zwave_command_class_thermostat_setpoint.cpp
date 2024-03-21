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

///////////////////////////////////////////////////////////////////////////////
// Data type
///////////////////////////////////////////////////////////////////////////////

// Used to describe a zwave setpoint value
struct zwave_setpoint_value {
  thermostat_setpoint_scale_t scale;
  thermostat_setpoint_precision_t precision;
  thermostat_setpoint_value_t value;
};

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
static void zwave_command_class_thermostat_setpoint_set_default_capabilities(
  attribute_store_node_t type_node)
{
  attribute_store_node_t min_value_scale_node
    = attribute_store_get_first_child_by_type(type_node,
                                              ATTRIBUTE(MIN_VALUE_SCALE));

  thermostat_setpoint_scale_t scale = DEFAULT_SCALE;
  attribute_store_set_reported(min_value_scale_node, &scale, sizeof(scale));

  attribute_store_node_t max_value_scale_node
    = attribute_store_get_first_child_by_type(type_node,
                                              ATTRIBUTE(MAX_VALUE_SCALE));

  attribute_store_set_reported(max_value_scale_node, &scale, sizeof(scale));

  thermostat_setpoint_precision_t precision = DEFAULT_PRECISION;
  attribute_store_node_t min_value_precision_node
    = attribute_store_get_first_child_by_type(type_node,
                                              ATTRIBUTE(MIN_VALUE_PRECISION));
  attribute_store_set_reported(min_value_precision_node,
                               &precision,
                               sizeof(precision));

  attribute_store_node_t max_value_precision_node
    = attribute_store_get_first_child_by_type(type_node,
                                              ATTRIBUTE(MAX_VALUE_PRECISION));
  attribute_store_set_reported(max_value_precision_node,
                               &precision,
                               sizeof(precision));

  // Set min and max value last so we can get their actual value
  attribute_store_node_t min_value_node
    = attribute_store_get_first_child_by_type(type_node, ATTRIBUTE(MIN_VALUE));

  thermostat_setpoint_value_t min_value = DEFAULT_MIN_VALUE;
  attribute_store_set_reported(min_value_node, &min_value, sizeof(min_value));

  attribute_store_node_t max_value_node
    = attribute_store_get_first_child_by_type(type_node, ATTRIBUTE(MAX_VALUE));

  int32_t max_value = DEFAULT_MAX_VALUE;
  attribute_store_set_reported(max_value_node, &max_value, sizeof(max_value));
}

/**
 * @brief Get the current value of a Setpoint Value 
 * 
 * @param setpoint_value  Setpoint value (scale is ignored)
 * 
 * @return double Current value based on the precision
 */
static double get_current_value(const zwave_setpoint_value &setpoint_value)
{
  double value = setpoint_value.value;
  for (uint8_t i = 0; i < setpoint_value.precision; i++) {
    value /= 10;
  }

  return value;
}

/**
 * @brief Fill setpoint_value with Scale, Precision and Value of value_node
 * 
 * @param value_node     One of ATTRIBUTE(VALUE), ATTRIBUTE(MIN_VALUE), ATTRIBUTE(MAX_VALUE)
 * @param setpoint_value Empty object that will be filled (by reference)
 * 
 * @return sl_status_t   SL_STATUS_OK if every field have been filed correctly, SL_STATUS_FAIL otherwise
 */
static sl_status_t
  get_zwave_setpoint_value(attribute_store_node_t value_node,
                           zwave_setpoint_value &setpoint_value,
                           attribute_store_node_value_state_t value_node_state
                           = REPORTED_ATTRIBUTE)
{
  sl_status_t status = SL_STATUS_OK;

  attribute_store_node_t setpoint_type_node
    = attribute_store_get_first_parent_with_type(value_node, ATTRIBUTE(TYPE));

  attribute_store_type_t value_node_type
    = attribute_store_get_node_type(value_node);

  status |= attribute_store_read_value(value_node,
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
  return status;
}

/**
 * @brief Checks the desired value of a setpoint and validate if it is within
 * the valid range.
 *
 * @param value_node      Attribute Store node for the value to apply.
 * @returns int32_t valid desired value
 */
static int32_t thermostat_setpoint_get_valid_desired_setpoint_value(
  attribute_store_node_t value_node)
{
  int32_t value_to_set = 0;
  attribute_store_get_desired_else_reported(value_node,
                                            &value_to_set,
                                            sizeof(value_to_set));

  zwave_cc_version_t supporting_node_version
    = zwave_command_class_get_version_from_node(
      value_node,
      COMMAND_CLASS_THERMOSTAT_SETPOINT_V3);

  // V1-v2, we don't know the capabilities, so we allow to try anything:
  if (3 > supporting_node_version) {
    return value_to_set;
  }

  attribute_store_node_t setpoint_type_node
    = attribute_store_get_first_parent_with_type(value_node, ATTRIBUTE(TYPE));
  attribute_store_node_t min_value_node
    = attribute_store_get_first_child_by_type(setpoint_type_node,
                                              ATTRIBUTE(MIN_VALUE));
  attribute_store_node_t max_value_node
    = attribute_store_get_first_child_by_type(setpoint_type_node,
                                              ATTRIBUTE(MAX_VALUE));

  zwave_setpoint_value setpoint_min_struct;
  zwave_setpoint_value setpoint_max_struct;
  zwave_setpoint_value setpoint_value_struct;

  sl_status_t status
    = get_zwave_setpoint_value(min_value_node, setpoint_min_struct);
  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG,
                   "Can't get setpoint MIN value. No check will be performed "
                   "on the value sent.");
    return value_to_set;
  }
  status = get_zwave_setpoint_value(max_value_node, setpoint_max_struct);
  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG,
                   "Can't get setpoint MAX value. No check will be performed "
                   "on the value sent.");
    return value_to_set;
  }
  status = get_zwave_setpoint_value(value_node,
                                    setpoint_value_struct,
                                    DESIRED_ATTRIBUTE);
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Can't get setpoint CURRENT value. Something went wrong");
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
  double current_min_value = get_current_value(setpoint_min_struct);
  double current_max_value = get_current_value(setpoint_max_struct);
  double current_value     = get_current_value(setpoint_value_struct);

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

///////////////////////////////////////////////////////////////////////////////
// Attribute Callback functions
///////////////////////////////////////////////////////////////////////////////

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

  // Get Setpoint type
  uint8_t setpoint_type = 0;
  attribute_store_get_reported(type_node,
                               &setpoint_type,
                               sizeof(setpoint_type));
  // Get Setpoint value
  thermostat_setpoint_value_t setpoint_value = 0;
  sl_status_t status = attribute_store_get_reported(value_node,
                                                    &setpoint_value,
                                                    sizeof(setpoint_value));

  if (status != SL_STATUS_OK) {
    sl_log_debug(LOG_TAG,
                 "No reported value for setpoint type %d. Waiting for one.",
                 setpoint_type);
    return;
  }

  // Check if all attributes are there (value, precision and scale)
  for (int i = 0; i < 3; i++) {
    auto current_attribute_type = attribute_type + i;
    attribute_store_node_t zwave_node
      = attribute_store_get_first_child_by_type(type_node,
                                                current_attribute_type);
    const char *zwave_attribute_name
      = attribute_store_get_type_name(current_attribute_type);
    if (!attribute_store_node_exists(zwave_node)
        || !attribute_store_is_value_defined(zwave_node, REPORTED_ATTRIBUTE)) {
      sl_log_debug(LOG_TAG,
                   "Setpoint type %d not ready yet. Missing : %s",
                   setpoint_type,
                   zwave_attribute_name);
      return;
    }
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

    attribute_store_type_t ucl_type           = attributes[attribute_type];
    thermostat_setpoint_scale_t scale         = 0;
    thermostat_setpoint_precision_t precision = 0;

    // Scale ID is base attribute + 1
    status = attribute_store_get_child_reported(
      type_node,
      attribute_type + SETPOINT_SCALE_ATTRIBUTE_ID_OFFSET,
      &scale,
      sizeof(scale));

    if (status != SL_STATUS_OK) {
      sl_log_warning(LOG_TAG,
                     "Can't get scale for attribute : %s",
                     attribute_type_name);
      attribute_store_log_node(type_node, true);
      return;
    }

    // Precision ID is base attribute + 2
    status = attribute_store_get_child_reported(
      type_node,
      attribute_type + SETPOINT_PRECISION_ATTRIBUTE_ID_OFFSET,
      &precision,
      sizeof(precision));

    if (status != SL_STATUS_OK) {
      sl_log_warning(LOG_TAG,
                     "Can't get precision for attribute : %s",
                     attribute_type_name);
      return;
    }

    int16_t ucl_value = zwave_temperature_to_ucl_temperature(setpoint_value,
                                                             (uint8_t)precision,
                                                             (uint8_t)scale);

    // Update (or create) ucl value
    status = attribute_store_set_child_reported(endpoint_node,
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
    = attribute_store_get_node_child_by_value(endpoint_node,
                                              ATTRIBUTE(TYPE),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&setpoint_type,
                                              sizeof(setpoint_type),
                                              0);

  if (setpoint_type_node == ATTRIBUTE_STORE_INVALID_NODE) {
    sl_log_warning(LOG_TAG, "Can't find setpoint type : %d", setpoint_type);
    return;
  }

  thermostat_setpoint_precision_t precision;
  sl_status_t status
    = attribute_store_get_child_reported(setpoint_type_node,
                                         ATTRIBUTE(VALUE_PRECISION),
                                         &precision,
                                         sizeof(precision));

  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG,
                   "Can't get precision value for UCL attribute %s. Won't "
                   "update Zwave attribute",
                   ucl_attribute_type_name);
    return;
  }

  thermostat_setpoint_scale_t scale;
  status = attribute_store_get_child_reported(setpoint_type_node,
                                              ATTRIBUTE(VALUE_SCALE),
                                              &scale,
                                              sizeof(scale));

  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG,
                   "Can't get scale value for UCL attribute %s. Won't update "
                   "Zwave attribute",
                   ucl_attribute_type_name);
    return;
  }

  // Get the requested desired value and map it to the zwave side
  int16_t ucl_value;
  status = attribute_store_get_desired(ucl_value_node,
                                       &ucl_value,
                                       sizeof(ucl_value));
  if (status != SL_STATUS_OK) {
    sl_log_debug(LOG_TAG,
                 "Defined value of UCL attribute %s is not defined. Not "
                 "updating anything.",
                 ucl_attribute_type_name);
    return;
  }

  // Read current zwave value
  attribute_store_node_t zwave_value_node
    = attribute_store_get_first_child_by_type(setpoint_type_node,
                                              ATTRIBUTE(VALUE));
  thermostat_setpoint_value_t current_zwave_value;
  status = attribute_store_get_reported(zwave_value_node,
                                        &current_zwave_value,
                                        sizeof(current_zwave_value));
  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG,
                   "Can't get current zwave current value. Not updating it.",
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
}

///////////////////////////////////////////////////////////////////////////////
// Attribute creation functions
///////////////////////////////////////////////////////////////////////////////
static attribute_store_node_t
  zwave_command_class_thermostat_setpoint_create_type(
    attribute_store_node_t endpoint_node, uint8_t type)
{
  // Do we already have the node ?
  attribute_store_node_t type_node = attribute_store_emplace(endpoint_node,
                                                             ATTRIBUTE(TYPE),
                                                             &type,
                                                             sizeof(type));

  // Add the six other nodes under the type.
  const attribute_store_type_t additional_nodes[]
    = {ATTRIBUTE(VALUE),
       ATTRIBUTE(VALUE_SCALE),
       ATTRIBUTE(VALUE_PRECISION),
       ATTRIBUTE(MIN_VALUE),
       ATTRIBUTE(MIN_VALUE_SCALE),
       ATTRIBUTE(MIN_VALUE_PRECISION),
       ATTRIBUTE(MAX_VALUE),
       ATTRIBUTE(MAX_VALUE_SCALE),
       ATTRIBUTE(MAX_VALUE_PRECISION)};
  attribute_store_add_if_missing(type_node,
                                 additional_nodes,
                                 COUNT_OF(additional_nodes));
  return type_node;
}

///////////////////////////////////////////////////////////////////////////////
// Attribute resolution functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_thermostat_setpoint_supported_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Check that we have the right type of attribute.
  assert(ATTRIBUTE(SUPPORTED_SETPOINT_TYPES)
         == attribute_store_get_node_type(node));

  // Default frame length in case of error
  *frame_length = 0;

  // Supported Get is the same for all versions.
  ZW_THERMOSTAT_SETPOINT_SUPPORTED_GET_V3_FRAME *supported_get_frame
    = (ZW_THERMOSTAT_SETPOINT_SUPPORTED_GET_V3_FRAME *)frame;

  supported_get_frame->cmdClass = COMMAND_CLASS_THERMOSTAT_SETPOINT_V3;
  supported_get_frame->cmd      = THERMOSTAT_SETPOINT_SUPPORTED_GET_V3;

  *frame_length = sizeof(ZW_THERMOSTAT_SETPOINT_SUPPORTED_GET_V3_FRAME);
  return SL_STATUS_OK;
}

// FIXME: This will be failing CL:0043.01.21.02.1
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

  if (supporting_node_version <= 2) {
    // In version 1-2, they do not have a supported get.
    // here there is no substitution, we we will just return no
    // frame and fill up dummy values ourselves
    zwave_command_class_thermostat_setpoint_set_default_capabilities(type_node);
    return SL_STATUS_ALREADY_EXISTS;

  } else {
    // From version 3, we can ask nodes what they support.
    ZW_THERMOSTAT_SETPOINT_CAPABILITIES_GET_V3_FRAME
    *capabilities_get_frame
      = (ZW_THERMOSTAT_SETPOINT_CAPABILITIES_GET_V3_FRAME *)frame;

    capabilities_get_frame->cmdClass = COMMAND_CLASS_THERMOSTAT_SETPOINT_V3;
    capabilities_get_frame->cmd      = THERMOSTAT_SETPOINT_CAPABILITIES_GET_V3;
    attribute_store_read_value(type_node,
                               REPORTED_ATTRIBUTE,
                               &capabilities_get_frame->properties1,
                               sizeof(capabilities_get_frame->properties1));

    *frame_length = sizeof(ZW_THERMOSTAT_SETPOINT_CAPABILITIES_GET_V3_FRAME);
    return SL_STATUS_OK;
  }
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

  *frame_length = sizeof(ZW_THERMOSTAT_SETPOINT_GET_V3_FRAME);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_thermostat_setpoint_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Check that we have the right type of attribute.
  assert(ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VALUE
         == attribute_store_get_node_type(node));

  // Default frame length in case of error
  *frame_length = 0;

  attribute_store_node_t type_node = attribute_store_get_node_parent(node);

  // Get setpoint type
  uint8_t setpoint_type = 0;
  attribute_store_get_reported(type_node,
                               &setpoint_type,
                               sizeof(setpoint_type));

  // Reuse the same scale as current value.
  thermostat_setpoint_scale_t setpoint_value_scale = 0;
  attribute_store_get_child_reported(
    type_node,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VALUE_SCALE,
    &setpoint_value_scale,
    sizeof(setpoint_value_scale));

  // Reuse the same precision as current value.
  thermostat_setpoint_precision_t setpoint_value_precision = 0;
  attribute_store_get_child_reported(
    type_node,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VALUE_PRECISION,
    &setpoint_value_precision,
    sizeof(setpoint_value_precision));

  uint8_t level2_properties_field
    = (setpoint_value_precision << 5) | (setpoint_value_scale << 3);

  thermostat_setpoint_value_t setpoint_value_integer
    = thermostat_setpoint_get_valid_desired_setpoint_value(node);

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

///////////////////////////////////////////////////////////////////////////////
// Command Handler functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_thermostat_setpoint_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // We expect to have at least 1 byte of value
  if (frame_length <= REPORT_VALUE_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  thermostat_setpoint_type_t received_type
    = frame_data[REPORT_SETPOINT_TYPE_INDEX] & SETPOINT_TYPE_MASK;

  attribute_store_node_t type_node = attribute_store_get_node_child_by_value(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_TYPE,
    REPORTED_ATTRIBUTE,
    (uint8_t *)&received_type,
    sizeof(received_type),
    0);

  // Add guard in case we don't find it
  if (type_node == ATTRIBUTE_STORE_INVALID_NODE) {
    sl_log_warning(
      LOG_TAG,
      "Device send setpoint type %d but can't find it in the attribute store",
      received_type);
    return SL_STATUS_NOT_SUPPORTED;
  }

  uint8_t size = frame_data[REPORT_PRECISION_SCALE_SIZE_INDEX] & SIZE_MASK;
  thermostat_setpoint_scale_t scale
    = (frame_data[REPORT_PRECISION_SCALE_SIZE_INDEX] & SCALE_MASK) >> 3;
  thermostat_setpoint_precision_t precision
    = (frame_data[REPORT_PRECISION_SCALE_SIZE_INDEX] & PRECISION_MASK) >> 5;

  // FIXME: Here we just ignore this requirement from the CC spec:
  // A receiving node MUST ignore values outside the range advertised in the Thermostat Setpoint
  // Capabilities Report Command for the actual Setpoint type.
  // Save the scale directly
  attribute_store_node_t scale_node = attribute_store_get_first_child_by_type(
    type_node,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VALUE_SCALE);
  attribute_store_set_reported(scale_node, &scale, sizeof(scale));

  // Save precision
  attribute_store_set_child_reported(
    type_node,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VALUE_PRECISION,
    &precision,
    sizeof(precision));

  thermostat_setpoint_value_t setpoint_value
    = get_signed_value_from_frame_and_size(&frame_data[REPORT_VALUE_INDEX],
                                           size);

  sl_log_debug(
    LOG_TAG,
    "NodeID %d:%d - Thermostat current setpoint value: %d (precision %d)",
    (int)connection_info->remote.node_id,
    (int)connection_info->remote.endpoint_id,
    setpoint_value,
    precision);

  // Save it in the attribute store, save the value after the scale, that's what the
  // mapper is listening to and the scale has to be correct when they get the callback.
  attribute_store_node_t setpoint_value_node
    = attribute_store_get_first_child_by_type(
      type_node,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VALUE);

  attribute_store_set_reported(setpoint_value_node,
                               &setpoint_value,
                               sizeof(setpoint_value));
  // We have a set function for this attribute, so we also align the desired value
  //attribute_store_set_desired_as_reported(setpoint_value_node);

  return SL_STATUS_OK;
}

static sl_status_t
  zwave_command_class_thermostat_setpoint_handle_supported_report(
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

  attribute_store_set_reported(supported_bitmask_node,
                               &frame_data[SUPPORTED_REPORT_BITMASK_INDEX],
                               frame_length - SUPPORTED_REPORT_BITMASK_INDEX);

  // Now create a type attribute for each supported type
  for (uint8_t byte = 0; byte < frame_length - SUPPORTED_REPORT_BITMASK_INDEX;
       byte++) {
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (frame_data[SUPPORTED_REPORT_BITMASK_INDEX + byte] & (1 << bit)) {
        // This type is supported.
        uint8_t type = byte * 8 + bit;
        if (type > 2) {
          // Use Bit Interpretation A see Thermostat Setpoint Command Class in
          // Z-Wave Application Command Class Specification
          type += 4;
        }
        zwave_command_class_thermostat_setpoint_create_type(endpoint_node,
                                                            type);
      }
    }
  }

  return SL_STATUS_OK;
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
  attribute_store_node_t type_node = attribute_store_get_node_child_by_value(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_TYPE,
    REPORTED_ATTRIBUTE,
    (uint8_t *)&received_setpoint_type,
    sizeof(int8_t),
    0);

  if (type_node == ATTRIBUTE_STORE_INVALID_NODE) {
    // Hmm it seems that it's a type that we don't know of.
    // Let's be nice and create it.
    zwave_command_class_thermostat_setpoint_create_type(endpoint_node,
                                                        received_setpoint_type);
  }

  // Save the Min value
  attribute_store_node_t supported_min_value_scale_node
    = attribute_store_get_first_child_by_type(
      type_node,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_MIN_VALUE_SCALE);

  if (supported_min_value_scale_node == ATTRIBUTE_STORE_INVALID_NODE) {
    return SL_STATUS_FAIL;
  }

  thermostat_setpoint_scale_t received_min_value_scale
    = (frame_data[CAPABILITIES_REPORT_MIN_PRECISION_SCALE_SIZE_INDEX]
       & SCALE_MASK)
      >> 3;
  attribute_store_set_reported(supported_min_value_scale_node,
                               &received_min_value_scale,
                               sizeof(received_min_value_scale));

  attribute_store_node_t supported_min_value_node
    = attribute_store_get_first_child_by_type(
      type_node,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_MIN_VALUE);
  uint8_t received_min_value_size
    = frame_data[CAPABILITIES_REPORT_MIN_PRECISION_SCALE_SIZE_INDEX]
      & SIZE_MASK;
  thermostat_setpoint_precision_t received_min_value_precision
    = (frame_data[CAPABILITIES_REPORT_MIN_PRECISION_SCALE_SIZE_INDEX]
       & PRECISION_MASK)
      >> 5;

  thermostat_setpoint_value_t received_min_value
    = command_class_get_int32_value(
      received_min_value_size,
      received_min_value_precision,
      &frame_data[CAPABILITIES_REPORT_MIN_VALUE_INDEX]);

  sl_log_debug(LOG_TAG,
               "NodeID %d:%d - Thermostat Min supported temperature "
               "(3 decimal digits): %d",
               (int)connection_info->remote.node_id,
               (int)connection_info->remote.endpoint_id,
               received_min_value);

  attribute_store_set_reported(supported_min_value_node,
                               &received_min_value,
                               sizeof(received_min_value));

  // Save the Max value
  int32_t max_value_precision_scale_size_index
    = CAPABILITIES_REPORT_MIN_PRECISION_SCALE_SIZE_INDEX
      + received_min_value_size + 1;
  attribute_store_node_t supported_max_value_scale_node
    = attribute_store_get_first_child_by_type(type_node,
                                              ATTRIBUTE(MAX_VALUE_SCALE));

  if (supported_max_value_scale_node == ATTRIBUTE_STORE_INVALID_NODE) {
    return SL_STATUS_FAIL;
  }

  int32_t received_max_value_scale
    = (frame_data[max_value_precision_scale_size_index] & SCALE_MASK) >> 3;
  attribute_store_set_reported(supported_max_value_scale_node,
                               &received_max_value_scale,
                               sizeof(received_max_value_scale));

  attribute_store_node_t supported_max_value_node
    = attribute_store_get_first_child_by_type(type_node, ATTRIBUTE(MAX_VALUE));
  int32_t received_max_value_size
    = frame_data[max_value_precision_scale_size_index] & SIZE_MASK;
  int32_t received_max_value_precision
    = (frame_data[max_value_precision_scale_size_index] & PRECISION_MASK) >> 5;

  int32_t received_max_value = command_class_get_int32_value(
    received_max_value_size,
    received_max_value_precision,
    &frame_data[max_value_precision_scale_size_index + 1]);

  sl_log_debug(LOG_TAG,
               "NodeID %d:%d - Thermostat Max supported temperature "
               "(3 decimal digits): %d",
               (int)connection_info->remote.node_id,
               (int)connection_info->remote.endpoint_id,
               received_max_value);

  attribute_store_set_reported(supported_max_value_node,
                               &received_max_value,
                               sizeof(received_max_value));

  return SL_STATUS_OK;
}

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
    NULL,
    &zwave_command_class_thermostat_setpoint_supported_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(MIN_VALUE),
    NULL,
    &zwave_command_class_thermostat_setpoint_capabilities_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(VALUE),
    &zwave_command_class_thermostat_setpoint_set,
    &zwave_command_class_thermostat_setpoint_get);

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
  handler.comments                   = "Partial Control: <br>"
                                       "1. No discovery of ambiguous types in v1-v2 <br>"
                                       "2. Only a few setpoints can be configured. <br>";

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}