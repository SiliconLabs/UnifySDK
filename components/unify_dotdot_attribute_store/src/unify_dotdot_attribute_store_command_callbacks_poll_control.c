/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "unify_dotdot_attribute_store_command_callbacks_poll_control.h"
#include "unify_dotdot_attribute_store_helpers.h"
#include "unify_dotdot_attribute_store_configuration.h"
#include "unify_dotdot_defined_attribute_types.h"

// Other Unify components.
#include "dotdot_mqtt.h"
#include "sl_log.h"

#define LOG_TAG "unify_dotdot_attribute_store_poll_control_commands_callbacks"

// PollControl cluster default values
const uint32_t DEFAULT_LONG_POLL_INTERVAL_MIN = 0x00;

/**
 * @brief Finds the PollControl::LongPollIntervalMin value for a UNID/ep
 *
 * @param unid          UNID of the node we want to get the information for
 * @param endpoint      Endpoint of the node we want to get the information for
 * @return uint8_t MinLevel value to use
 */
static uint32_t get_long_poll_interval_min(dotdot_unid_t unid,
                                           dotdot_endpoint_id_t endpoint)
{
  if (dotdot_is_supported_poll_control_long_poll_interval_min(unid, endpoint)) {
    return dotdot_get_poll_control_long_poll_interval_min(unid,
                                                          endpoint,
                                                          REPORTED_ATTRIBUTE);
  } else {
    return DEFAULT_LONG_POLL_INTERVAL_MIN;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Incoming command/control functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t
  set_long_poll_interval_command(dotdot_unid_t unid,
                                 dotdot_endpoint_id_t endpoint,
                                 uic_mqtt_dotdot_callback_call_type_t call_type,
                                 uint32_t new_long_poll_interval)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return dotdot_is_supported_poll_control_long_poll_interval(unid, endpoint)
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(LOG_TAG,
                 "Updating ZCL desired values after "
                 "PollControl::SetLongPollInterval command");

    uint32_t min = get_long_poll_interval_min(unid, endpoint);
    if (new_long_poll_interval < min) {
      new_long_poll_interval = min;
    }
    dotdot_set_poll_control_long_poll_interval(unid,
                                               endpoint,
                                               DESIRED_ATTRIBUTE,
                                               new_long_poll_interval);

    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG,
                   "Clearing PollControl::LongPollInterval reported value");
      dotdot_poll_control_long_poll_interval_undefine_reported(unid, endpoint);
    }
  }

  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Init and teardown functions
///////////////////////////////////////////////////////////////////////////////
void poll_control_cluster_mapper_init()
{
  sl_log_debug(LOG_TAG, "PollControl cluster mapper initialization");

  uic_mqtt_dotdot_poll_control_set_long_poll_interval_callback_set(
    &set_long_poll_interval_command);
}
