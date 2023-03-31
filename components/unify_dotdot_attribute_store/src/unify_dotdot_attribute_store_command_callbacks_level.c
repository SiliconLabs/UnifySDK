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
#include "unify_dotdot_attribute_store_command_callbacks.h"
#include "unify_dotdot_attribute_store_command_callbacks_level.h"
#include "unify_dotdot_attribute_store_helpers.h"
#include "unify_dotdot_attribute_store_configuration.h"
#include "unify_dotdot_defined_attribute_types.h"

// Includes from Unify
#include "sl_log.h"
#include "sl_status.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_transitions.h"

// Includes from auto-generated files
#include "dotdot_mqtt.h"

// Setup Log ID
const char LOG_TAG[] = "unify_dotdot_attribute_store_level_commands_callbacks";

// OnOff cluster constants
const uint8_t ON = 0xFF;
// Level Cluster constants
const uint8_t DEFAULT_MOVE_RATE = 0xFF;

// ZigBee default values for Level::MinLevel and Level::MaxLevel
const uint8_t DEFAULT_MIN_LEVEL = 0x00;
const uint8_t DEFAULT_MAX_LEVEL = 0xFF;

/**
 * @brief Finds the Level::MinLevel value for a UNID/ep
 *
 * @param unid          UNID of the node we want to get the information for
 * @param endpoint      Endpoint of the node we want to get the information for
 * @return uint8_t MinLevel value to use
 */
static uint8_t get_min_level(dotdot_unid_t unid, dotdot_endpoint_id_t endpoint)
{
  if (dotdot_is_supported_level_min_level(unid, endpoint)) {
    return dotdot_get_level_min_level(unid, endpoint, REPORTED_ATTRIBUTE);
  } else {
    return DEFAULT_MIN_LEVEL;
  }
}

/**
 * @brief Finds the Level::MaxLevel value for a UNID/ep
 *
 * @param unid          UNID of the node we want to get the information for
 * @param endpoint      Endpoint of the node we want to get the information for
 * @return uint8_t MaxLevel value to use
 */
static uint8_t get_max_level(dotdot_unid_t unid, dotdot_endpoint_id_t endpoint)
{
  if (dotdot_is_supported_level_max_level(unid, endpoint)) {
    return dotdot_get_level_max_level(unid, endpoint, REPORTED_ATTRIBUTE);
  } else {
    return DEFAULT_MAX_LEVEL;
  }
}

/**
 * @brief Verifies that the Level is within the allowed range and returns a valid
 * value from it.
 *
 * @param unid          UNID of the node we want to validate the level
 * @param endpoint      Endpoint of the node we want to validate the level
 * @param level         The value that we received for the level
 * @return uint8_t  The validated value that we should apply.
 */
static uint8_t validated_level_value(dotdot_unid_t unid,
                                     dotdot_endpoint_id_t endpoint,
                                     uint8_t level)
{
  uint8_t max_level = get_max_level(unid, endpoint);
  if (level > max_level) {
    return max_level;
  }

  uint8_t min_level = get_min_level(unid, endpoint);
  if (level < min_level) {
    return min_level;
  }

  return level;
}

////////////////////////////////////////////////////////////////////////////////
// Attribute Store callbacks
////////////////////////////////////////////////////////////////////////////////
static void
  on_remaining_time_reported_update(attribute_store_node_t updated_node,
                                    attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }
  if (false == attribute_store_is_reported_defined(updated_node)) {
    return;
  }

  // If the reported is set to a non-zero value, make sure to initiate a countdown.
  number_t remaining_time = attribute_store_get_reported_number(updated_node);
  if ((remaining_time > 0)
      && (false == is_attribute_transition_ongoing(updated_node))) {
    // Attribute is in seconds, so decrement by 1 every second
    attribute_start_fixed_transition(updated_node,
                                     REPORTED_ATTRIBUTE,
                                     0,
                                     -10,
                                     1000);
  }
}

static void dotdot_set_level_current_level_and_on_off_transition_time(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uint8_t level,
  uint16_t transition_time)
{
  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(LOG_TAG,
                 "Updating ZCL desired values after "
                 "Level:CurrentLevel command");
    dotdot_set_level_current_level(unid, endpoint, DESIRED_ATTRIBUTE, level);
    if (is_clear_reported_enabled()) {
      dotdot_level_current_level_undefine_reported(unid, endpoint);

      sl_log_debug(LOG_TAG, "Clearing Level:CurrentLevel reported value");
    }
  }
  if (dotdot_is_supported_level_on_off_transition_time(unid, endpoint)
      && (is_desired_value_update_on_commands_enabled())) {
    sl_log_debug(LOG_TAG,
                 "Updating ZCL desired values after "
                 "Level:OnOffTransitionTime command");
    dotdot_set_level_on_off_transition_time(unid,
                                            endpoint,
                                            DESIRED_ATTRIBUTE,
                                            transition_time);
    if (is_clear_reported_enabled()) {
      dotdot_level_on_off_transition_time_undefine_reported(unid, endpoint);
      sl_log_debug(LOG_TAG,
                   "Clearing Level:OnOffTransitionTime reported value");
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Incoming command/control functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t level_cluster_mapper_move_to_level(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint8_t level,
  uint16_t transition_time,
  uint8_t options_mask,
  uint8_t options_override)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return (dotdot_is_supported_level_current_level(unid, endpoint)
              && dotdot_is_supported_level_on_off_transition_time(unid,
                                                                  endpoint)
              && dotdot_is_supported_on_off_on_off(unid, endpoint))
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  // Command has no effect if OnOff::OnOff is Off.
  if (false
      == dotdot_get_on_off_on_off(unid,
                                  endpoint,
                                  DESIRED_OR_REPORTED_ATTRIBUTE)) {
    return SL_STATUS_OK;
  }

  // Set the level and transition time as received in the command.
  level = validated_level_value(unid, endpoint, level);
  dotdot_set_level_current_level_and_on_off_transition_time(unid,
                                                            endpoint,
                                                            level,
                                                            transition_time);
  // We ignore OptionsMask / OptionsOverride.
  return SL_STATUS_OK;
}

sl_status_t
  level_cluster_mapper_move(dotdot_unid_t unid,
                            uint8_t endpoint,
                            uic_mqtt_dotdot_callback_call_type_t call_type,
                            MoveStepMode move_mode,
                            uint8_t rate,
                            uint8_t options_mask,
                            uint8_t options_override)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return (dotdot_is_supported_level_current_level(unid, endpoint)
              && dotdot_is_supported_level_on_off_transition_time(unid,
                                                                  endpoint)
              && dotdot_is_supported_on_off_on_off(unid, endpoint))
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  // Command has no effect if OnOff::OnOff is Off.
  if (false
      == dotdot_get_on_off_on_off(unid,
                                  endpoint,
                                  DESIRED_OR_REPORTED_ATTRIBUTE)) {
    return SL_STATUS_OK;
  }

  // This is the ZigBee equivalent of Z-Wave Start level change.
  // We don't bother and just do a regular set towards boundaries (DEFAULT_MIN_LEVEL or DEFAULT_MAX_LEVEL)
  // with an estimated transition time.
  uint8_t level = DEFAULT_MIN_LEVEL;
  if (move_mode == ZCL_MOVE_STEP_MODE_DOWN) {
    level = get_min_level(unid, endpoint);
  } else if (move_mode == ZCL_MOVE_STEP_MODE_UP) {
    level = get_max_level(unid, endpoint);
  }

  int32_t current_reported_level
    = dotdot_get_level_current_level(unid, endpoint, REPORTED_ATTRIBUTE);

  uint16_t transition_time = 0;
  if (rate != DEFAULT_MOVE_RATE && rate != 0) {
    uint8_t steps = 0;
    if (current_reported_level > level) {
      steps = (uint8_t)(current_reported_level - level);
    } else {
      steps = (uint8_t)(level - current_reported_level);
    }
    transition_time = steps / rate;
  }

  // Make the transition with our calculated target level /
  dotdot_set_level_current_level_and_on_off_transition_time(unid,
                                                            endpoint,
                                                            level,
                                                            transition_time);

  // We ignore OptionsMask / OptionsOverride.
  return SL_STATUS_OK;
}

sl_status_t
  level_cluster_mapper_step(dotdot_unid_t unid,
                            uint8_t endpoint,
                            uic_mqtt_dotdot_callback_call_type_t call_type,
                            MoveStepMode step_mode,
                            uint8_t step_size,
                            uint16_t transition_time,
                            uint8_t options_mask,
                            uint8_t options_override)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return (dotdot_is_supported_level_current_level(unid, endpoint)
              && dotdot_is_supported_level_on_off_transition_time(unid,
                                                                  endpoint)
              && dotdot_is_supported_on_off_on_off(unid, endpoint))
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  // Command has no effect if OnOff::OnOff is Off.
  if (false
      == dotdot_get_on_off_on_off(unid,
                                  endpoint,
                                  DESIRED_OR_REPORTED_ATTRIBUTE)) {
    return SL_STATUS_OK;
  }

  // Step size 0 will not get us moving.
  if (step_size == 0) {
    return SL_STATUS_OK;
  }

  uint8_t current_reported_level
    = dotdot_get_level_current_level(unid, endpoint, REPORTED_ATTRIBUTE);

  // Apply the Step to our current level.
  uint8_t level = 0;
  if (step_mode == ZCL_MOVE_STEP_MODE_DOWN) {
    level = current_reported_level - step_size;
  } else if (step_mode == ZCL_MOVE_STEP_MODE_UP) {
    level = current_reported_level + step_size;
  }

  // Boundary verification
  level = validated_level_value(unid, endpoint, level);

  // Transition time 0xFFFF will be converted to "as fast as possible."
  if (transition_time == 0xFFFF) {
    transition_time = 0;
  }

  // Make the transition with our calculated target level
  dotdot_set_level_current_level_and_on_off_transition_time(unid,
                                                            endpoint,
                                                            level,
                                                            transition_time);

  // We ignore OptionsMask / OptionsOverride.
  return SL_STATUS_OK;
}

sl_status_t
  level_cluster_mapper_stop(dotdot_unid_t unid,
                            uint8_t endpoint,
                            uic_mqtt_dotdot_callback_call_type_t call_type,
                            uint8_t options_mask,
                            uint8_t options_override)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return (dotdot_is_supported_level_current_level(unid, endpoint)
              && dotdot_is_supported_level_on_off_transition_time(unid,
                                                                  endpoint)
              && dotdot_is_supported_on_off_on_off(unid, endpoint))
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }
  // Set the remaining of the transition time to 0.
  // Set the desired value to the current reported.
  int32_t current_reported_level
    = dotdot_get_level_current_level(unid, endpoint, REPORTED_ATTRIBUTE);
  dotdot_set_level_current_level_and_on_off_transition_time(
    unid,
    endpoint,
    current_reported_level,
    0);

  // We ignore OptionsMask / OptionsOverride.
  return SL_STATUS_OK;
}

sl_status_t level_cluster_mapper_move_to_level_with_on_off(
  dotdot_unid_t unid,
  uint8_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint8_t level,
  uint16_t transition_time,
  uint8_t options_mask,
  uint8_t options_override)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return (dotdot_is_supported_level_current_level(unid, endpoint)
              && dotdot_is_supported_level_on_off_transition_time(unid,
                                                                  endpoint)
              && dotdot_is_supported_on_off_on_off(unid, endpoint))
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }
  // Set the level and transition time as received in the command.
  level = validated_level_value(unid, endpoint, level);

  dotdot_set_level_current_level_and_on_off_transition_time(unid,
                                                            endpoint,
                                                            level,
                                                            transition_time);
  // Do the with OnOff magic:
  if (level > get_min_level(unid, endpoint)) {
    dotdot_set_on_off_on_off(unid, endpoint, DESIRED_ATTRIBUTE, ON);
  } else {
    attribute_store_node_t endpoint_node
      = unify_dotdot_attributes_get_endpoint_node()(unid, endpoint);
    attribute_store_node_t on_off = attribute_store_get_first_child_by_type(
      endpoint_node,
      DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF);
    attribute_start_fixed_transition(on_off,
                                     DESIRED_ATTRIBUTE,
                                     0,
                                     -1.0,
                                     transition_time * 100);
  }

  // We ignore OptionsMask / OptionsOverride.
  return SL_STATUS_OK;
}

sl_status_t level_cluster_mapper_move_with_on_off(
  dotdot_unid_t unid,
  uint8_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  MoveStepMode move_mode,
  uint8_t rate,
  uint8_t options_mask,
  uint8_t options_override)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return (dotdot_is_supported_level_current_level(unid, endpoint)
              && dotdot_is_supported_level_on_off_transition_time(unid,
                                                                  endpoint)
              && dotdot_is_supported_on_off_on_off(unid, endpoint))
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  // This is the ZigBee equivalent of Z-Wave Start level change.
  // We don't bother and just do a regular set towards boundaries (0 or 255)
  // with an estimated transition time.
  uint8_t level = DEFAULT_MIN_LEVEL;
  if (move_mode == ZCL_MOVE_STEP_MODE_DOWN) {
    level = get_min_level(unid, endpoint);
  } else if (move_mode == ZCL_MOVE_STEP_MODE_UP) {
    level = get_max_level(unid, endpoint);
  }

  int32_t current_reported_level
    = dotdot_get_level_current_level(unid, endpoint, REPORTED_ATTRIBUTE);

  uint16_t transition_time = 0;
  if (rate != DEFAULT_MOVE_RATE && rate != 0) {
    uint8_t steps = 0;
    if (current_reported_level > level) {
      steps = (uint8_t)(current_reported_level - level);
    } else {
      steps = (uint8_t)(level - current_reported_level);
    }
    transition_time = steps / rate;
  }

  // Make the transition with our calculated target level /
  dotdot_set_level_current_level_and_on_off_transition_time(unid,
                                                            endpoint,
                                                            level,
                                                            transition_time);

  // Do the with OnOff magic:
  // Do not go off here, the multilevel switch level going to zero will make
  // the OnOff attribute Off.
  if (level > get_min_level(unid, endpoint)) {
    dotdot_set_on_off_on_off(unid, endpoint, DESIRED_ATTRIBUTE, ON);
  }

  // We ignore OptionsMask / OptionsOverride.
  return SL_STATUS_OK;
}

sl_status_t level_cluster_mapper_step_with_on_off(
  dotdot_unid_t unid,
  uint8_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  MoveStepMode step_mode,
  uint8_t step_size,
  uint16_t transition_time,
  uint8_t options_mask,
  uint8_t options_override)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return (dotdot_is_supported_level_current_level(unid, endpoint)
              && dotdot_is_supported_level_on_off_transition_time(unid,
                                                                  endpoint)
              && dotdot_is_supported_on_off_on_off(unid, endpoint))
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  // Step size 0 will not get us moving.
  if (step_size == 0) {
    return SL_STATUS_OK;
  }

  uint8_t current_reported_level
    = dotdot_get_level_current_level(unid, endpoint, REPORTED_ATTRIBUTE);

  // Apply the Step to our current level.
  uint8_t level = 0;
  if (step_mode == ZCL_MOVE_STEP_MODE_DOWN) {
    level = current_reported_level - step_size;
  } else if (step_mode == ZCL_MOVE_STEP_MODE_UP) {
    level = current_reported_level + step_size;
  }

  // Boundary verification
  level = validated_level_value(unid, endpoint, level);

  // Transition time 0xFFFF will be converted to "as fast as possible."
  if (transition_time == 0xFFFF) {
    transition_time = 0;
  }

  // Make the transition with our calculated target level
  dotdot_set_level_current_level_and_on_off_transition_time(unid,
                                                            endpoint,
                                                            level,
                                                            transition_time);
  // Do the with OnOff magic:
  // Do not go off here, the multilevel switch level going to zero will make
  // the OnOff attribute Off.
  if (level > get_min_level(unid, endpoint)) {
    dotdot_set_on_off_on_off(unid, endpoint, DESIRED_ATTRIBUTE, ON);
  }

  // We ignore OptionsMask / OptionsOverride.
  return SL_STATUS_OK;
}

sl_status_t level_cluster_mapper_stop_with_on_off(
  dotdot_unid_t unid,
  uint8_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint8_t options_mask,
  uint8_t options_override)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return (dotdot_is_supported_level_current_level(unid, endpoint)
              && dotdot_is_supported_level_on_off_transition_time(unid,
                                                                  endpoint)
              && dotdot_is_supported_on_off_on_off(unid, endpoint))
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  // Set the remaining of the transition time to 0.
  // Set the desired value to the current reported.
  int32_t current_reported_level
    = dotdot_get_level_current_level(unid, endpoint, REPORTED_ATTRIBUTE);
  dotdot_set_level_current_level_and_on_off_transition_time(
    unid,
    endpoint,
    current_reported_level,
    0);

  // Do the with OnOff magic:
  if (current_reported_level > get_min_level(unid, endpoint)) {
    dotdot_set_on_off_on_off(unid, endpoint, DESIRED_ATTRIBUTE, ON);
  }

  // We ignore OptionsMask / OptionsOverride.
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Init and teardown functions
///////////////////////////////////////////////////////////////////////////////
void level_cluster_mapper_init()
{
  sl_log_debug(LOG_TAG, "Level cluster mapper initialization");

  uic_mqtt_dotdot_level_move_to_level_callback_set(
    &level_cluster_mapper_move_to_level);

  uic_mqtt_dotdot_level_move_callback_set(&level_cluster_mapper_move);

  uic_mqtt_dotdot_level_step_callback_set(&level_cluster_mapper_step);

  uic_mqtt_dotdot_level_stop_callback_set(&level_cluster_mapper_stop);

  uic_mqtt_dotdot_level_move_to_level_with_on_off_callback_set(
    &level_cluster_mapper_move_to_level_with_on_off);

  uic_mqtt_dotdot_level_move_with_on_off_callback_set(
    &level_cluster_mapper_move_with_on_off);

  uic_mqtt_dotdot_level_step_with_on_off_callback_set(
    &level_cluster_mapper_step_with_on_off);

  uic_mqtt_dotdot_level_stop_with_on_off_callback_set(
    &level_cluster_mapper_stop_with_on_off);

  attribute_store_register_callback_by_type_and_state(
    &on_remaining_time_reported_update,
    DOTDOT_ATTRIBUTE_ID_LEVEL_REMAINING_TIME,
    REPORTED_ATTRIBUTE);
}
