
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
#include "unify_dotdot_attribute_store_command_callbacks_zwave_fan_control.h"

// Other Unify components.
// uic_mqtt_dotdot_xxx_callback_set()
#include "dotdot_mqtt.h"
// dotdot_is_supported_xxx
#include "unify_dotdot_attribute_store_helpers.h"
// is_automatic_deduction_of_supported_commands_enabled
#include "unify_dotdot_attribute_store_configuration.h"
#include "sl_log.h"

#define LOG_TAG \
  "unify_dotdot_attribute_store_command_callbacks_zwave_fan_control"

sl_status_t
  zwave_fan_control_set_fan_mode(dotdot_unid_t unid,
                                 dotdot_endpoint_id_t endpoint,
                                 uic_mqtt_dotdot_callback_call_type_t call_type,
                                 ZWaveFanModeEnum fan_mode)
{
  // First check the call type. If this is a support check support call,
  // we check the attributes
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    // Check user option automatic_deduction_of_supported_commands
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return (dotdot_is_supported_unify_fan_control_z_wave_fan_mode(unid, endpoint))
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(
      LOG_TAG,
      "Updating ZCL desired values after ZWave_FanControl::SetFanMode command");
    sl_status_t result
      = dotdot_set_unify_fan_control_z_wave_fan_mode(unid,
                                               endpoint,
                                               DESIRED_ATTRIBUTE,
                                               fan_mode);

    if (result != SL_STATUS_OK) {
      sl_log_warning(LOG_TAG,
                     "Can't set fan mode for ZWave Fan Control cluster");
      return SL_STATUS_FAIL;
    }

    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG, "Clearing ZWave_FanControl reported values");
      dotdot_unify_fan_control_z_wave_fan_mode_undefine_reported(unid, endpoint);
    }

  } else {
    sl_log_debug(
      LOG_TAG,
      "No Updating ZCL desired values after ZWave_FanControl::SetFanMode "
      "command since is_desired_value_update_on_commands_enabled() is off.");
  }

  return SL_STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// Internal component function that register callbacks to DotDot MQTT
////////////////////////////////////////////////////////////////////////////////
void zwave_fan_control_cluster_mapper_init(void)
{
  // Unify Sound Commands.
  sl_log_debug(LOG_TAG, "ZWave Fan Control Cluster mapper initialization\n");

  uic_mqtt_dotdot_unify_fan_control_set_fan_mode_callback_set(
    &zwave_fan_control_set_fan_mode);
}