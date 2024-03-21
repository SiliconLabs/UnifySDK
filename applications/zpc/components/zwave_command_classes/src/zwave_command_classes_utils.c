
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
#include "zwave_command_classes_utils.h"
#include "zwave_command_class_version.h"

// Includes from other components
#include "ZW_classcmd.h"
#include "zwave_unid.h"
#include "zwave_tx.h"
#include "zwave_tx_scheme_selector.h"
#include "zwave_controller_utils.h"
#include "zpc_attribute_store.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"

// Unify Includes
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "sl_log.h"

// Generic includes
#include <assert.h>
#include <math.h>

#define LOG_TAG "zwave_command_class_utils"

///////////////////////////////////////////////////////////////////////////////
// Attribute store helper functions
///////////////////////////////////////////////////////////////////////////////
attribute_store_node_t zwave_command_class_get_endpoint_node(
  const zwave_controller_connection_info_t *connection_info)
{
  if (NULL == connection_info) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }
  // Find out the unid of the sending node:
  unid_t node_unid;
  zwave_unid_from_node_id(connection_info->remote.node_id, node_unid);

  // Get the corresponding Attribute Store entry for this unid / endpoint:
  return attribute_store_network_helper_get_endpoint_node(
    node_unid,
    connection_info->remote.endpoint_id);
}

attribute_store_node_t zwave_command_class_get_node_id_node(
  const zwave_controller_connection_info_t *connection_info)
{
  if (NULL == connection_info) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }
  // Find out the unid of the sending node:
  unid_t node_unid;
  zwave_unid_from_node_id(connection_info->remote.node_id, node_unid);

  // Get the corresponding Attribute Store entry for this unid :
  return attribute_store_network_helper_get_node_id_node(node_unid);
}

zwave_cc_version_t
  zwave_command_class_get_version_from_node(attribute_store_node_t node,
                                            zwave_command_class_t command_class)
{
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);

  attribute_store_node_t version_node = attribute_store_get_first_child_by_type(
    endpoint_node,
    ZWAVE_CC_VERSION_ATTRIBUTE(command_class));

  zwave_cc_version_t version = 0;
  attribute_store_get_reported(version_node, &version, sizeof(version));
  return version;
}

bool has_reports_to_follow(attribute_store_node_t node)
{
  reports_to_follow_t reports = 0;
  attribute_store_get_reported(
    attribute_store_get_first_child_by_type(node, ATTRIBUTE_REPORTS_TO_FOLLOW),
    &reports,
    sizeof(reports));

  return (reports > 0);
}

reports_to_follow_t get_reports_to_follow(attribute_store_node_t node)
{
  reports_to_follow_t reports = 0;
  attribute_store_get_reported(
    attribute_store_get_first_child_by_type(node, ATTRIBUTE_REPORTS_TO_FOLLOW),
    &reports,
    sizeof(reports));

  return reports;
}

sl_status_t set_reports_to_follow(attribute_store_node_t node,
                                  reports_to_follow_t reports_to_follow)
{
  zwave_node_id_t zwave_node_id = 0;
  if ((reports_to_follow > 0)
      && (SL_STATUS_OK
          == attribute_store_network_helper_get_node_id_from_node(
            node,
            &zwave_node_id))) {
    zwave_tx_set_expected_frames(zwave_node_id, reports_to_follow);
  }
  return attribute_store_set_child_reported(node,
                                            ATTRIBUTE_REPORTS_TO_FOLLOW,
                                            &reports_to_follow,
                                            sizeof(reports_to_follow));
}

void set_command_status_value(attribute_store_node_t command_status_node,
                              command_status_values_t reported,
                              command_status_values_t desired)
{
  attribute_store_set_reported(command_status_node,
                               (uint8_t *)&reported,
                               sizeof(command_status_values_t));
  attribute_store_set_desired(command_status_node,
                              (uint8_t *)&desired,
                              sizeof(command_status_values_t));
}

///////////////////////////////////////////////////////////////////////////////
// Tx helper functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_send_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint16_t report_size,
  const uint8_t *const report_data)
{
  assert(report_size > 0);

  zwave_tx_options_t tx_options = {};
  zwave_tx_scheme_get_node_tx_options(
    ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY,
    0,
    0,
    &tx_options);

  return zwave_tx_send_data(connection_info,
                            report_size,
                            report_data,
                            &tx_options,
                            NULL,
                            NULL,
                            NULL);
}

///////////////////////////////////////////////////////////////////////////////
// Generic helper functions
///////////////////////////////////////////////////////////////////////////////
bool is_zwave_command_class_filtered_for_root_device(
  zwave_command_class_t command_class, attribute_store_node_t updated_node)
{
  if (is_actuator_command_class(command_class) == false) {
    return false;
  }

  zwave_node_id_t node_id         = 0;
  zwave_endpoint_id_t endpoint_id = 0;

  attribute_store_network_helper_get_zwave_ids_from_node(updated_node,
                                                         &node_id,
                                                         &endpoint_id);

  if (endpoint_id != 0) {
    return false;
  }

  // CC is to be filtered for ep0 if
  // 1. The node supports Multi Channel AND
  //    a. we don't know the version yet (once we know the version, we may unfilter it)
  //    b. we know the version and it's > 2
  if (zwave_node_supports_command_class(COMMAND_CLASS_MULTI_CHANNEL_V4,
                                        node_id,
                                        endpoint_id)) {
    if (is_version_cc_found(updated_node)) {
      zwave_cc_version_t version
        = zwave_node_get_command_class_version(COMMAND_CLASS_MULTI_CHANNEL_V4,
                                               node_id,
                                               endpoint_id);
      if (version == 0 || version > 2) {
        return true;
      } else {
        return false;
      }

    } else {
      return true;
    }
  }
  return false;
}

bool is_using_zpc_highest_security_class(
  const zwave_controller_connection_info_t *connection)
{
  if (connection == NULL) {
    sl_log_warning(LOG_TAG, "NULL connection info object. Returning false");
    return false;
  }
  zwave_keyset_t zpc_keyset;
  zwave_tx_scheme_get_zpc_granted_keys(&zpc_keyset);
  zwave_controller_encapsulation_scheme_t zpc_scheme
    = zwave_controller_get_highest_encapsulation(zpc_keyset);
  return connection->encapsulation == zpc_scheme;
}

bool is_actuator_command_class(zwave_command_class_t command_class)
{
  switch (command_class) {
    case COMMAND_CLASS_BARRIER_OPERATOR:
    case COMMAND_CLASS_BASIC:
    case COMMAND_CLASS_SWITCH_BINARY:
    case COMMAND_CLASS_SWITCH_COLOR:
    case COMMAND_CLASS_DOOR_LOCK:
    case COMMAND_CLASS_SWITCH_MULTILEVEL:
    case COMMAND_CLASS_SIMPLE_AV_CONTROL:
    case COMMAND_CLASS_SOUND_SWITCH:
    case COMMAND_CLASS_THERMOSTAT_SETPOINT:
    case COMMAND_CLASS_THERMOSTAT_MODE:
    case COMMAND_CLASS_WINDOW_COVERING:
      return true;
    default:
      return false;
  }
}

zwave_role_type_t get_zwave_node_role_type(zwave_node_id_t node_id)
{
  zwave_role_type_t role_type = ROLE_TYPE_UNKNOWN;
  unid_t unid;
  zwave_unid_from_node_id(node_id, unid);
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, 0);

  attribute_store_node_t role_type_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE_ZWAVE_ROLE_TYPE);

  if (role_type_node != ATTRIBUTE_STORE_INVALID_NODE) {
    attribute_store_get_reported(role_type_node, &role_type, sizeof(role_type));
  }

  return role_type;
}

bool is_portable_end_node(attribute_store_node_t node)
{
  zwave_node_id_t node_id = 0;
  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_NODE_ID);
  attribute_store_get_reported(node_id_node, &node_id, sizeof(node_id));
  if (get_zwave_node_role_type(node_id) == ROLE_TYPE_PEN) {
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// Value manipulation functions
///////////////////////////////////////////////////////////////////////////////
int32_t command_class_get_int32_value(uint8_t size,
                                      uint8_t precision,
                                      const uint8_t *value)
{
  int64_t extracted_value = 0;
  extracted_value         = get_signed_value_from_frame_and_size(value, size);

  // We save this value with 3 decimal places in the attribute store.
  extracted_value = extracted_value * 1000;
  for (uint8_t i = 0; i < precision; i++) {
    extracted_value = extracted_value / 10;
  }
  return extracted_value;
}

int32_t
  get_signed_value_from_frame_and_size(const uint8_t *frame, uint8_t size)
{
  int32_t extracted_value = get_unsigned_value_from_frame_and_size(frame, size);

  // Check if that was a negative value
  if (extracted_value & (1 << (size * 8 - 1))) {
    extracted_value |= ~((1 << (size * 8 - 1)) - 1);
  }

  return extracted_value;
}

uint32_t get_unsigned_value_from_frame_and_size(const uint8_t *frame,
                                                uint8_t size)
{
  uint32_t extracted_value = 0;
  for (int i = 0; i < size; i++) {
    extracted_value = (extracted_value << 8) | frame[i];
  }

  return extracted_value;
}

// ZCL use precision of 2
int16_t zwave_temperature_to_ucl_temperature(int32_t value,
                                             uint8_t precision,
                                             uint8_t scale)
{
  // First convert value with precision of 2
  double coeff = pow(10, (2-precision));
  int32_t ucl_value = value * coeff;

  while (ucl_value > INT16_MAX || ucl_value < INT16_MIN ) {
      ucl_value /= 10;
      sl_log_warning(LOG_TAG, "Reducing precision of temperature value");
  }

  // Need to convert to C°
  if (scale == 1) {
    double real_value = ucl_value / 100.0;
    double f_value = FAHRENHEIT_TO_DEGREES(real_value);
    ucl_value = f_value * 100;
  }

  return ucl_value;
}

// ZCL use precision of 2
int32_t ucl_temperature_to_zwave_temperature(int16_t value,
                                             uint8_t precision,
                                             uint8_t scale)
{
  int32_t zwave_value = value;
  // First Need to convert to C°
  if (scale == 1) {
    double real_value = value / 100.0;
    double f_value = DEGREES_TO_FAHRENHEIT(real_value);
    zwave_value = f_value * 100;
  }

  // Then convert back to our  precision
  double coeff = pow(10, (precision-2));
  zwave_value *= coeff;

  return zwave_value;
}

///////////////////////////////////////////////////////////////////////////////
// Duration conversion
///////////////////////////////////////////////////////////////////////////////
uint8_t time_to_zwave_duration(clock_time_t time)
{
  uint8_t zwave_duration = 0;
  if (time <= 0x7F * CLOCK_SECOND) {
    zwave_duration = (uint8_t)(time / CLOCK_SECOND);
  } else if (time <= 0xFD * CLOCK_SECOND * 60) {
    zwave_duration = 127 + (uint8_t)(time / CLOCK_SECOND / 60);
  } else {
    zwave_duration = 0xFD;
  }
  return zwave_duration;
}

clock_time_t zwave_duration_to_time(uint8_t zwave_duration)
{
  clock_time_t time = 0;
  if (zwave_duration <= 0x7F) {
    time = zwave_duration * CLOCK_SECOND;
  } else if (zwave_duration <= 0xFD) {
    time = (zwave_duration - 127) * CLOCK_SECOND * 60;
  }
  return time;
}
