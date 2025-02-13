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
#include "unify_dotdot_attribute_store_command_callbacks.h"
#include "unify_dotdot_attribute_store_command_callbacks_barrier_control.h"
#include "unify_dotdot_attribute_store_configuration.h"
#include "unify_dotdot_attribute_store_helpers.h"
#include "unify_dotdot_defined_attribute_types.h"
#include "attribute_timeouts.h"
#include "dotdot_mqtt_send_commands.h"

// Other Unify components.
#include "dotdot_mqtt.h"
#include "sl_log.h"

#define LOG_TAG \
  "unify_dotdot_attribute_store_barrier_control_commands_callbacks"

////////////////////////////////////////////////////////////////////////////////
// Private component callback functions for DotDot MQTT
////////////////////////////////////////////////////////////////////////////////
sl_status_t barrier_control_go_to_percent_command(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint8_t percent_open)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return dotdot_is_supported_barrier_control_barrier_position(unid,
                                                                  endpoint)
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }
  
  // Value validation
  uint8_t capabilities = dotdot_get_barrier_control_capabilities(unid, endpoint, REPORTED_ATTRIBUTE);
  if(capabilities == 0) {
    if ((percent_open!=0) && (percent_open!=100)) {
      sl_log_debug(LOG_TAG,
                   "BarrierOperator::BarrierPosition Invalid value, only 0 and 100 value are supported");
    return SL_STATUS_INVALID_PARAMETER;
    }
  }
  
  //Bound Validation
  if (percent_open > 100) {
    sl_log_debug(LOG_TAG,
                   "BarrierOperator::BarrierPosition value out of bounds");
    return SL_STATUS_INVALID_RANGE;
  }
  
  // Update the desired values:
  if (is_desired_value_update_on_commands_enabled()) {
    dotdot_set_barrier_control_barrier_position(unid,
                                                endpoint,
                                                DESIRED_ATTRIBUTE,
                                                percent_open);
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG,
                   "Clearing BarrierOperator::BarrierPosition reported value");
      dotdot_barrier_control_barrier_position_undefine_reported(unid, endpoint);
    }
  }

  return SL_STATUS_OK;
}

sl_status_t
  barrier_control_stop_command(dotdot_unid_t unid,
                               dotdot_endpoint_id_t endpoint,
                               uic_mqtt_dotdot_callback_call_type_t call_type)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return dotdot_is_supported_barrier_control_moving_state(unid, endpoint)
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  // Update the desired values:
  if (is_desired_value_update_on_commands_enabled()) {
    dotdot_set_barrier_control_moving_state(unid,
                                            endpoint,
                                            DESIRED_ATTRIBUTE,
                                            0);
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG,
                   "Clearing BarrierOperator::MovingState reported value");
      dotdot_barrier_control_moving_state_undefine_reported(unid, endpoint);
    }
  }

  return SL_STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// Internal component function that register callbacks to DotDot MQTT
////////////////////////////////////////////////////////////////////////////////
void barrier_control_cluster_mapper_init()
{
  // OnOff Cluster Commands.
  sl_log_debug(LOG_TAG, "BarrierControl Cluster mapper initialization\n");

  uic_mqtt_dotdot_barrier_control_go_to_percent_callback_set(
    &barrier_control_go_to_percent_command);
  uic_mqtt_dotdot_barrier_control_stop_callback_set(
    &barrier_control_stop_command);
}
