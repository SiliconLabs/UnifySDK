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
#include "unify_dotdot_attribute_store_command_callbacks_door_lock.h"
#include "unify_dotdot_attribute_store_helpers.h"
#include "unify_dotdot_attribute_store_command_callbacks.h"
#include "unify_dotdot_attribute_store_configuration.h"
#include "unify_dotdot_defined_attribute_types.h"

// Includes from Unify
#include "sl_log.h"
#include "sl_status.h"

// Includes from auto-generated files
#include "dotdot_mqtt.h"
#include "zap-types.h"

// Setup Log ID
#define LOG_TAG "unify_dotdot_attribute_store_doorlock_commands_callbacks"

/* Door lock */
static sl_status_t doorlock_lock_command(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  const uic_mqtt_dotdot_callback_call_type_t callback_type,
  const char *pin_code)
{
  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return dotdot_is_supported_door_lock_lock_state(unid, endpoint)
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  if (dotdot_is_supported_door_lock_lock_state(unid, endpoint)
      && (is_desired_value_update_on_commands_enabled())) {
    sl_log_debug(LOG_TAG,
                 "Updating ZCL desired values after "
                 "DoorLock:LockState command");
    dotdot_set_door_lock_lock_state(unid,
                                    endpoint,
                                    DESIRED_ATTRIBUTE,
                                    ZCL_DOOR_LOCK_LOCK_STATE_LOCKED);
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG, "Clearing DoorLock:LockState reported value");
      dotdot_door_lock_lock_state_undefine_reported(unid, endpoint);
    }
  }

  return SL_STATUS_OK;
}

static sl_status_t
  doorlock_unLock_command(const dotdot_unid_t unid,
                          const dotdot_endpoint_id_t endpoint,
                          uic_mqtt_dotdot_callback_call_type_t callback_type,
                          const char *pin_code)
{
  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return dotdot_is_supported_door_lock_lock_state(unid, endpoint)
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  if (dotdot_is_supported_door_lock_lock_state(unid, endpoint)
      && (is_desired_value_update_on_commands_enabled())) {
    sl_log_debug(LOG_TAG,
                 "Updating ZCL desired values after "
                 "DoorLock:LockState command");
    dotdot_set_door_lock_lock_state(unid,
                                    endpoint,
                                    DESIRED_ATTRIBUTE,
                                    ZCL_DOOR_LOCK_LOCK_STATE_UNLOCKED);
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG, "Clearing DoorLock:LockState reported value");
      dotdot_door_lock_lock_state_undefine_reported(unid, endpoint);
    }
  }

  return SL_STATUS_OK;
}

void door_lock_cluster_mapper_init()
{
  sl_log_debug(LOG_TAG, "Door lock cluster mapper initialization");
  // Doorlock: Register the callbacks for receiving commands from IoT service
  uic_mqtt_dotdot_door_lock_lock_door_callback_set(doorlock_lock_command);
  uic_mqtt_dotdot_door_lock_unlock_door_callback_set(doorlock_unLock_command);
}
