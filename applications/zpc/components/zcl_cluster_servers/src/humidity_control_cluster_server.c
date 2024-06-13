
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
// Includes from this component
#include "humidity_control_cluster_server.h"

// Includes from Unify
#include "sl_log.h"
#include "sl_status.h"
#include "attribute_store_helper.h"
#include "zpc_attribute_store_network_helper.h"
#include "zwave_command_class_humidity_control_types.h"

#include "attribute_store_defined_attribute_types.h"
#include "unify_dotdot_defined_attribute_types.h"
#include "unify_dotdot_attribute_store.h"
#include "unify_dotdot_attribute_store_node_state.h"

// Includes from auto-generated files
#include "dotdot_mqtt.h"

// Setup Log ID
#define LOG_TAG "unify_humidity_control_cluster_server"

sl_status_t unify_humidity_control_mode_set(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  ModeType mode)
{
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);

  attribute_store_node_t current_mode_node
    = attribute_store_get_first_child_by_type(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_CURRENT_MODE);

  // First check the call type. If this is a support check support call,
  // we check the attributes
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    // Check user option automatic_deduction_of_supported_commands
    return attribute_store_node_exists(current_mode_node) ? SL_STATUS_OK
                                                          : SL_STATUS_FAIL;
  }

  humidity_control_mode_t mode_value = mode;
  return attribute_store_set_desired(current_mode_node,
                                     &mode_value,
                                     sizeof(mode_value));
}

sl_status_t unify_humidity_control_setpoint_set(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  SetpointType type,
  uint8_t precision,
  uint8_t scale,
  int32_t value)
{
  // First check the call type. This command have too many requirements
  // so we support it by default
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return  SL_STATUS_OK;
  }

  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);

  humidity_control_setpoint_type_t setpoint_type = type;

  attribute_store_node_t setpoint_type_node
    = attribute_store_get_node_child_by_value(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_TYPE,
      REPORTED_ATTRIBUTE,
      &setpoint_type,
      sizeof(setpoint_type),
      0);

  if (setpoint_type == ATTRIBUTE_STORE_INVALID_NODE) {
    sl_log_warning(LOG_TAG,
                   "Can't find humidity setpoint type %d",
                   setpoint_type);
    return SL_STATUS_FAIL;
  }

  attribute_store_set_child_reported(
    setpoint_type_node,
    ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_VALUE_PRECISION,
    &precision,
    sizeof(precision));

  attribute_store_set_child_reported(
    setpoint_type_node,
    ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_VALUE_SCALE,
    &scale,
    sizeof(scale));

  return attribute_store_set_child_desired(
    setpoint_type_node,
    ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_VALUE,
    &value,
    sizeof(value));
}

///////////////////////////////////////////////////////////////////////////////
// Init and teardown functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t humidity_control_cluster_server_init()
{
  sl_log_debug(LOG_TAG, "Humidity Control cluster (ZWave) server initialization");

  // Listen to the BASIC Value attribute is created
  uic_mqtt_dotdot_unify_humidity_control_mode_set_callback_set(
    &unify_humidity_control_mode_set);

  uic_mqtt_dotdot_unify_humidity_control_setpoint_set_callback_set(
    &unify_humidity_control_setpoint_set);

  return SL_STATUS_OK;
}
