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
#include "attribute_timeouts.h"
#include "attribute_transitions.h"

// Includes from auto-generated files
#include "dotdot_mqtt.h"
#include "zap-types.h"

// Setup Log ID
#define LOG_TAG "unify_dotdot_attribute_store_doorlock_commands_callbacks"

////////////////////////////////////////////////////////////////////////////////
// Attribute Store callbacks
////////////////////////////////////////////////////////////////////////////////
static void set_lock_to_secured(attribute_store_node_t node)
{
  attribute_store_set_desired_number(node, ZCL_DOOR_LOCK_LOCK_STATE_LOCKED);
  attribute_store_set_reported_number(node, ZCL_DOOR_LOCK_LOCK_STATE_LOCKED);
}

static void on_lock_state_reported_update(attribute_store_node_t updated_node,
                                          attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }
  if (false == attribute_store_is_reported_defined(updated_node)) {
    return;
  }
  if (ZCL_DOOR_LOCK_LOCK_STATE_UNLOCKED
      != attribute_store_get_reported_number(updated_node)) {
    return;
  }
  if (true
      == attribute_timeout_is_callback_active(updated_node,
                                              &set_lock_to_secured)) {
    return;
  }

  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(
      updated_node,
      unify_dotdot_attributes_endpoint_attribute());

  attribute_store_node_t auto_relock_node
    = attribute_store_get_first_child_by_type(
      endpoint_node,
      DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_AUTO_RELOCK_TIME);

  if (false == attribute_store_is_reported_defined(auto_relock_node)) {
    return;
  }

  // If the reported is set to a non-zero value, make sure to initiate a countdown.
  number_t auto_relock_time
    = attribute_store_get_reported_number(auto_relock_node);
  if (auto_relock_time > 0) {
    attribute_timeout_set_callback(updated_node,
                                   auto_relock_time * 1000,
                                   &set_lock_to_secured);
  }
}

////////////////////////////////////////////////////////////////////////////////
// MQTT UCL command handlers
////////////////////////////////////////////////////////////////////////////////
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

static sl_status_t
  toggle_command(const dotdot_unid_t unid,
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
                 "DoorLock:Toggle command");
    DoorLockLockState state
      = dotdot_get_door_lock_lock_state(unid, endpoint, REPORTED_ATTRIBUTE);
    if (state == ZCL_DOOR_LOCK_LOCK_STATE_LOCKED) {
      state = ZCL_DOOR_LOCK_LOCK_STATE_UNLOCKED;
    } else {
      state = ZCL_DOOR_LOCK_LOCK_STATE_LOCKED;
    }
    dotdot_set_door_lock_lock_state(unid, endpoint, DESIRED_ATTRIBUTE, state);
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG, "Clearing DoorLock:LockState reported value");
      dotdot_door_lock_lock_state_undefine_reported(unid, endpoint);
    }
  }

  return SL_STATUS_OK;
}

static sl_status_t unlock_with_timeout_command(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t callback_type,
  uint16_t timeout_in_seconds,
  const char *pin_orrfid_code)
{
  if (UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK == callback_type) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return (dotdot_is_supported_door_lock_lock_state(unid, endpoint)
              && dotdot_is_supported_door_lock_auto_relock_time(unid, endpoint))
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
                 "DoorLock:UnlockWithTimeout command");

    attribute_store_node_t endpoint_node
      = unify_dotdot_attributes_get_endpoint_node()(unid, endpoint);

    attribute_store_node_t lock_state_node
      = attribute_store_get_first_child_by_type(
        endpoint_node,
        DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_LOCK_STATE);

    attribute_store_set_desired_number(lock_state_node,
                                       ZCL_DOOR_LOCK_LOCK_STATE_UNLOCKED);
    attribute_timeout_set_callback(lock_state_node,
                                   timeout_in_seconds * 1000,
                                   &set_lock_to_secured);

    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG, "Clearing DoorLock:LockState reported value");
      attribute_store_undefine_reported(lock_state_node);
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
  uic_mqtt_dotdot_door_lock_toggle_callback_set(&toggle_command);
  uic_mqtt_dotdot_door_lock_unlock_with_timeout_callback_set(
    &unlock_with_timeout_command);

  attribute_store_register_callback_by_type_and_state(
    &on_lock_state_reported_update,
    DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_LOCK_STATE,
    REPORTED_ATTRIBUTE);
}
