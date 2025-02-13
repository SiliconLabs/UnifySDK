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
const uint8_t DEFAULT_MIN_FREQUENCY = 0x00;
const uint8_t DEFAULT_MAX_FREQUENCY = 0x00;

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
 * @brief Finds the Level::MinFrequency value for a UNID/ep
 *
 * @param unid          UNID of the node we want to get the information for
 * @param endpoint      Endpoint of the node we want to get the information for
 * @return uint16_t MinFrequency value to use
 */
static uint16_t get_min_frequency(dotdot_unid_t unid,
                                  dotdot_endpoint_id_t endpoint)
{
  if (dotdot_is_supported_level_min_frequency(unid, endpoint)) {
    return dotdot_get_level_min_frequency(unid, endpoint, REPORTED_ATTRIBUTE);
  } else {
    return DEFAULT_MIN_FREQUENCY;
  }
}

/**
 * @brief Finds the Level::MaxFrequency value for a UNID/ep
 *
 * @param unid          UNID of the node we want to get the information for
 * @param endpoint      Endpoint of the node we want to get the information for
 * @return uint16_t MaxFrequency value to use
 */
static uint16_t get_max_frequency(dotdot_unid_t unid,
                                  dotdot_endpoint_id_t endpoint)
{
  if (dotdot_is_supported_level_max_frequency(unid, endpoint)) {
    return dotdot_get_level_max_frequency(unid, endpoint, REPORTED_ATTRIBUTE);
  } else {
    return DEFAULT_MAX_FREQUENCY;
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
  int8_t step,
  uint16_t transition_time)
{
  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(LOG_TAG,
                 "Updating ZCL desired values after "
                 "Level:CurrentLevel command");
    attribute_store_node_t endpoint_node = unify_dotdot_attributes_get_endpoint_node()(unid, endpoint);

    attribute_store_node_t node
      = attribute_store_get_first_child_by_type(
        endpoint_node,
        DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL);

    if (step != 0) {
      // transitions won't start if the node is undefined, so define
      //  for the first step of transition
      uint8_t next_level
        = dotdot_get_level_current_level(unid, endpoint, REPORTED_ATTRIBUTE);
      next_level += step;
      dotdot_set_level_current_level(unid,
                                     endpoint,
                                     DESIRED_ATTRIBUTE,
                                     next_level);
      attribute_start_fixed_transition(node,
                                       DESIRED_ATTRIBUTE,
                                       level,
                                       step,
                                       1000);
    } else {
      attribute_stop_transition(node);
      dotdot_set_level_current_level(unid, endpoint, DESIRED_ATTRIBUTE, level);
    }

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

// Return true if mask_offset matches the option 
bool is_option_active(uint8_t option, uint8_t mask_offset) {
  return (option & (1 << mask_offset)) > 0;
}



 /**
 * @brief Check if ExecuteIfOff option is set according to the ZCL specification (3.10.2.3.1.2).
 *
 * @param option_level      ZCL option attribute
 * @param options_mask      Option mask
 * @param options_override  Option override
 * 
 * The function checks ExecuteIfOffOn bit in option_level or in options_override if options_mask contains ExecuteIfOffOn bit.
 * 
 * @return bool  True if ExecuteIfOff option is activated, false otherwise
 */
bool has_execute_if_off_option(uint8_t option_level, uint8_t options_mask, uint8_t options_override) {
  uint8_t temp_option = option_level;
  // If option_mask bit for ExecuteIfOff is set we take the option_override value instead of the level_option
  if (is_option_active(options_mask, LEVEL_OPTIONS_EXECUTE_IF_OFF_OFFSET))  {
    temp_option = options_override;
  }
  return is_option_active( temp_option, LEVEL_OPTIONS_EXECUTE_IF_OFF_OFFSET);
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
                                                                  endpoint))
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }
  
  uint8_t option_level = dotdot_get_level_options(unid, endpoint, DESIRED_OR_REPORTED_ATTRIBUTE);
  bool execute_even_if_off_option = has_execute_if_off_option(option_level, options_mask, options_override);

  // Command has no effect if OnOff::OnOff is Off and execute_even_if_off_option is not specified
  if (!execute_even_if_off_option && 
        dotdot_get_on_off_on_off(unid,
                                  endpoint,
                                  DESIRED_OR_REPORTED_ATTRIBUTE) == false) {
    return SL_STATUS_OK;
  }

  // Set the level and transition time as received in the command.
  level = validated_level_value(unid, endpoint, level);
  uint8_t current_level
    = dotdot_get_level_current_level(unid, endpoint, REPORTED_ATTRIBUTE);
  dotdot_set_level_current_level_and_on_off_transition_time(
    unid,
    endpoint,
    level,
    (transition_time) ? ((level - current_level) / (transition_time / 10.0f))
                      : 0,
    transition_time);
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
                                                                  endpoint))
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  uint8_t option_level = dotdot_get_level_options(unid, endpoint, DESIRED_OR_REPORTED_ATTRIBUTE);
  bool execute_even_if_off_option = has_execute_if_off_option(option_level, options_mask, options_override);

  // Command has no effect if OnOff::OnOff is Off and execute_even_if_off_option is not specified
  if (!execute_even_if_off_option &&  
      dotdot_get_on_off_on_off(unid,
                               endpoint,
                               DESIRED_OR_REPORTED_ATTRIBUTE) == false) {
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
                                                            (move_mode == ZCL_MOVE_STEP_MODE_DOWN) ? -rate : rate,
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
                                                                  endpoint))
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  uint8_t option_level = dotdot_get_level_options(unid, endpoint, DESIRED_OR_REPORTED_ATTRIBUTE);
  bool execute_even_if_off_option = has_execute_if_off_option(option_level, options_mask, options_override);

  // Command has no effect if OnOff::OnOff is Off and execute_even_if_off_option is not specified
  if (!execute_even_if_off_option && 
        dotdot_get_on_off_on_off(unid,
                                  endpoint,
                                  DESIRED_OR_REPORTED_ATTRIBUTE) == false) {
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
  uint8_t step  = 0;
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
    step = level;
  }
  else
  {
    step = step_size / (transition_time / 10);
  }
  

  // Make the transition with our calculated target level
  dotdot_set_level_current_level_and_on_off_transition_time(unid,
                                                            endpoint,
                                                            level,
                                                            (step_mode == ZCL_MOVE_STEP_MODE_DOWN) ? -step : step,
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
                                                                  endpoint))
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  uint8_t option_level = dotdot_get_level_options(unid, endpoint, DESIRED_OR_REPORTED_ATTRIBUTE);
  bool execute_even_if_off_option = has_execute_if_off_option(option_level, options_mask, options_override);

  // Command has no effect if OnOff::OnOff is Off and execute_even_if_off_option is not specified
  if (!execute_even_if_off_option && 
        dotdot_get_on_off_on_off(unid,
                                  endpoint,
                                  DESIRED_OR_REPORTED_ATTRIBUTE) == false) {
    return SL_STATUS_OK;
  }

  // Set the remaining of the transition time to 0.
  // Set the desired value to the current reported.
  int32_t current_reported_level
    = dotdot_get_level_current_level(unid, endpoint, REPORTED_ATTRIBUTE);
  dotdot_set_level_current_level_and_on_off_transition_time(
    unid,
    endpoint,
    current_reported_level,
    0,
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
  uint8_t current_level
    = dotdot_get_level_current_level(unid, endpoint, REPORTED_ATTRIBUTE);

  dotdot_set_level_current_level_and_on_off_transition_time(
    unid,
    endpoint,
    level,
    (transition_time) ? ((level - current_level) / (transition_time / 10.0f))
                      : 0,
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
                                                            rate,
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
  uint8_t step = 0;
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
    step = level;
  }
  else if (transition_time != 0)
  {
    step = step_size / (transition_time / 10);
  }

  // Make the transition with our calculated target level
  dotdot_set_level_current_level_and_on_off_transition_time(unid,
                                                            endpoint,
                                                            level,
                                                            (step_mode == ZCL_MOVE_STEP_MODE_DOWN) ? -step : step,
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
    0,
    0);

  // Do the with OnOff magic:
  if (current_reported_level > get_min_level(unid, endpoint)) {
    dotdot_set_on_off_on_off(unid, endpoint, DESIRED_ATTRIBUTE, ON);
  }

  // We ignore OptionsMask / OptionsOverride.
  return SL_STATUS_OK;
}

static sl_status_t level_move_to_closest_frequency_command(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint16_t frequency)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return dotdot_is_supported_level_current_frequency(unid, endpoint)
               ? SL_STATUS_OK
               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  // Align desired values:
  if (dotdot_is_supported_level_current_frequency(unid, endpoint)
      && (is_desired_value_update_on_commands_enabled())) {
    uint16_t min_frequency = get_min_frequency(unid, endpoint);
    uint16_t max_frequency = get_max_frequency(unid, endpoint);
    if (frequency < min_frequency) {
      frequency = min_frequency;
    } else if (frequency > max_frequency) {
      frequency = max_frequency;
    }

    sl_log_debug(LOG_TAG,
                 "Updating ZCL desired values after "
                 "Level:MoveToClosestFrequency command");
    dotdot_set_level_current_frequency(unid,
                                       endpoint,
                                       DESIRED_ATTRIBUTE,
                                       frequency);
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG, "Clearing Level:CurrentFrequency reported value");
      dotdot_level_current_frequency_undefine_reported(unid, endpoint);
    }
  }

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

  uic_mqtt_dotdot_level_move_to_closest_frequency_callback_set(
    &level_move_to_closest_frequency_command);

  attribute_store_register_callback_by_type_and_state(
    &on_remaining_time_reported_update,
    DOTDOT_ATTRIBUTE_ID_LEVEL_REMAINING_TIME,
    REPORTED_ATTRIBUTE);
}
