
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
#include "fan_control_cluster_server.h"

// Includes from Unify
#include "sl_log.h"
#include "sl_status.h"
#include "attribute_store_helper.h"
#include "zpc_attribute_store_network_helper.h"
#include "zwave_command_class_thermostat_fan_types.h"

#include "attribute_store_defined_attribute_types.h"
#include "unify_dotdot_defined_attribute_types.h"
#include "unify_dotdot_attribute_store.h"
#include "unify_dotdot_attribute_store_node_state.h"

// Includes from auto-generated files
#include "dotdot_mqtt.h"

// Setup Log ID
#define LOG_TAG "fan_control_cluster_server"

sl_status_t
  zwave_fan_control_turn_off(dotdot_unid_t unid,
                             dotdot_endpoint_id_t endpoint,
                             uic_mqtt_dotdot_callback_call_type_t call_type)
{
  attribute_store_node_t endpoint_node
    = attribute_store_network_helper_get_endpoint_node(unid, endpoint);

  attribute_store_node_t off_flag_node
    = attribute_store_get_first_child_by_type(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_OFF_FLAG);

  // First check the call type. If this is a support check support call,
  // we check the attributes
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    // Check user option automatic_deduction_of_supported_commands
    return attribute_store_node_exists(off_flag_node) ? SL_STATUS_OK
                                                      : SL_STATUS_FAIL;
  }

  thermostat_fan_mode_off_flag_t off_flag = 1;
  return attribute_store_set_desired(off_flag_node,
                                     &off_flag,
                                     sizeof(off_flag));
}

///////////////////////////////////////////////////////////////////////////////
// Init and teardown functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t fan_control_cluster_server_init()
{
  sl_log_debug(LOG_TAG, "FanControl cluster (ZWave) server initialization");

  // Listen to the BASIC Value attribute is created
  uic_mqtt_dotdot_unify_fan_control_turn_off_callback_set(
    &zwave_fan_control_turn_off);

  return SL_STATUS_OK;
}
