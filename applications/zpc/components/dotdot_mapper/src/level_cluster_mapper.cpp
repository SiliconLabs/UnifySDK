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
#include "level_cluster_mapper.h"
#include "dotdot_attributes.h"

// Includes from UIC
#include "sl_log.h"
#include "sl_status.h"

// Includes from auto-generated files
#include "dotdot_mqtt.h"

// Setup Log ID
constexpr char LOG_TAG[] = "level_cluster_mapper";

// OnOff cluster constants
constexpr uint8_t ON = 0xFF;
// Level Cluster constants
constexpr uint8_t DEFAULT_MOVE_RATE = 0xFF;

// ZigBee normally like to have a 0..255 range, but also allows to advertise a min/max
// Z-Wave does 0..99%, the UAM file advertises this range.
constexpr uint8_t DEFAULT_MIN_LEVEL = 0x00;
constexpr uint8_t DEFAULT_MAX_LEVEL = 0x63;

///////////////////////////////////////////////////////////////////////////////
// Incoming command/control functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t level_cluster_mapper_move_to_level(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint8_t level,
  uint16_t transition_time,
  [[maybe_unused]] uint8_t options_mask,
  [[maybe_unused]] uint8_t options_override)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return (dotdot_is_supported_level_current_level(unid, endpoint)
            && dotdot_is_supported_on_off_on_off(unid, endpoint))
             ? SL_STATUS_OK
             : SL_STATUS_FAIL;
  }
  // Set the level and transition time as received in the command.
  dotdot_set_level_current_level(unid,
                                 endpoint,
                                 DESIRED_ATTRIBUTE,
                                 (uint32_t)level);
  if (dotdot_is_supported_level_on_off_transition_time(unid, endpoint)) {
    dotdot_set_level_on_off_transition_time(unid,
                                            endpoint,
                                            DESIRED_ATTRIBUTE,
                                            (uint32_t)transition_time);
  }

  //FIXME: We ignore OptionsMask / OptionsOverride for now.
  return SL_STATUS_OK;
}

sl_status_t
  level_cluster_mapper_move(dotdot_unid_t unid,
                            uint8_t endpoint,
                            uic_mqtt_dotdot_callback_call_type_t call_type,
                            MoveStepMode move_mode,
                            uint8_t rate,
                            [[maybe_unused]] uint8_t options_mask,
                            [[maybe_unused]] uint8_t options_override)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return (dotdot_is_supported_level_current_level(unid, endpoint)
            && dotdot_is_supported_level_on_off_transition_time(unid, endpoint)
            && dotdot_is_supported_on_off_on_off(unid, endpoint))
             ? SL_STATUS_OK
             : SL_STATUS_FAIL;
  }

  // This is the ZigBee equivalent of Z-Wave Start level change.
  // We don't bother and just do a regular set towards boundaries (DEFAULT_MIN_LEVEL or DEFAULT_MAX_LEVEL)
  // with an estimated transition time.
  uint8_t level = DEFAULT_MIN_LEVEL;
  if (move_mode == ZCL_MOVE_STEP_MODE_DOWN) {
    level = DEFAULT_MIN_LEVEL;
  } else if (move_mode == ZCL_MOVE_STEP_MODE_UP) {
    level = DEFAULT_MAX_LEVEL;
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
  dotdot_set_level_current_level(unid,
                                 endpoint,
                                 DESIRED_ATTRIBUTE,
                                 (uint32_t)level);
  dotdot_set_level_on_off_transition_time(unid,
                                          endpoint,
                                          DESIRED_ATTRIBUTE,
                                          (uint32_t)transition_time);

  //FIXME: We ignore OptionsMask / OptionsOverride for now.
  return SL_STATUS_OK;
}

sl_status_t
  level_cluster_mapper_step(dotdot_unid_t unid,
                            uint8_t endpoint,
                            uic_mqtt_dotdot_callback_call_type_t call_type,
                            MoveStepMode step_mode,
                            uint8_t step_size,
                            uint16_t transition_time,
                            [[maybe_unused]] uint8_t options_mask,
                            [[maybe_unused]] uint8_t options_override)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return (dotdot_is_supported_level_current_level(unid, endpoint)
            && dotdot_is_supported_level_on_off_transition_time(unid, endpoint)
            && dotdot_is_supported_on_off_on_off(unid, endpoint))
             ? SL_STATUS_OK
             : SL_STATUS_FAIL;
  }

  // Step size 0 will not get us moving.
  if (step_size == 0) {
    return SL_STATUS_OK;
  }

  int32_t current_reported_level
    = dotdot_get_level_current_level(unid, endpoint, REPORTED_ATTRIBUTE);

  // Apply the Step to our current level.
  uint8_t level = 0;
  if (step_mode == ZCL_MOVE_STEP_MODE_DOWN) {
    level = current_reported_level - step_size;
  } else if (step_mode == ZCL_MOVE_STEP_MODE_UP) {
    level = current_reported_level + step_size;
  }

  // Boundary verification
  if (level > DEFAULT_MAX_LEVEL) {
    level = DEFAULT_MAX_LEVEL;
  } else if (level < DEFAULT_MIN_LEVEL) {
    level = DEFAULT_MIN_LEVEL;
  }

  // Transition time 0xFFFF will be converted to "as fast as possible."
  if (transition_time == 0xFFFF) {
    transition_time = 0;
  }

  // Make the transition with our calculated target level
  dotdot_set_level_current_level(unid,
                                 endpoint,
                                 DESIRED_ATTRIBUTE,
                                 (uint32_t)level);
  dotdot_set_level_on_off_transition_time(unid,
                                          endpoint,
                                          DESIRED_ATTRIBUTE,
                                          (uint32_t)transition_time);

  //FIXME: We ignore OptionsMask / OptionsOverride for now.
  return SL_STATUS_OK;
}

sl_status_t
  level_cluster_mapper_stop(dotdot_unid_t unid,
                            uint8_t endpoint,
                            uic_mqtt_dotdot_callback_call_type_t call_type,
                            [[maybe_unused]] uint8_t options_mask,
                            [[maybe_unused]] uint8_t options_override)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return (dotdot_is_supported_level_current_level(unid, endpoint)
            && dotdot_is_supported_level_on_off_transition_time(unid, endpoint)
            && dotdot_is_supported_on_off_on_off(unid, endpoint))
             ? SL_STATUS_OK
             : SL_STATUS_FAIL;
  }
  // Set the remaining of the transition time to 0.
  dotdot_set_level_on_off_transition_time(unid, endpoint, DESIRED_ATTRIBUTE, 0);

  // Set the desired value to the current reported.
  int32_t current_reported_level
    = dotdot_get_level_current_level(unid, endpoint, REPORTED_ATTRIBUTE);
  dotdot_set_level_current_level(unid,
                                 endpoint,
                                 DESIRED_ATTRIBUTE,
                                 current_reported_level);

  //FIXME: We ignore OptionsMask / OptionsOverride for now.
  return SL_STATUS_OK;
}

sl_status_t level_cluster_mapper_move_to_level_with_on_off(
  dotdot_unid_t unid,
  uint8_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint8_t level,
  uint16_t transition_time,
  [[maybe_unused]] uint8_t options_mask,
  [[maybe_unused]] uint8_t options_override)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return (dotdot_is_supported_level_current_level(unid, endpoint)
            && dotdot_is_supported_level_on_off_transition_time(unid, endpoint)
            && dotdot_is_supported_on_off_on_off(unid, endpoint))
             ? SL_STATUS_OK
             : SL_STATUS_FAIL;
  }
  // Set the level and transition time as received in the command.
  dotdot_set_level_current_level(unid,
                                 endpoint,
                                 DESIRED_ATTRIBUTE,
                                 (uint32_t)level);
  dotdot_set_level_on_off_transition_time(unid,
                                          endpoint,
                                          DESIRED_ATTRIBUTE,
                                          (uint32_t)transition_time);

  // Do the with OnOff magic:
  if (level > DEFAULT_MIN_LEVEL) {
    dotdot_set_on_off_on_off(unid, endpoint, DESIRED_ATTRIBUTE, ON);
  }

  //FIXME: We ignore OptionsMask / OptionsOverride for now.
  return SL_STATUS_OK;
}

sl_status_t level_cluster_mapper_move_with_on_off(
  dotdot_unid_t unid,
  uint8_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  MoveStepMode move_mode,
  uint8_t rate,
  [[maybe_unused]] uint8_t options_mask,
  [[maybe_unused]] uint8_t options_override)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return (dotdot_is_supported_level_current_level(unid, endpoint)
            && dotdot_is_supported_on_off_on_off(unid, endpoint))
             ? SL_STATUS_OK
             : SL_STATUS_FAIL;
  }

  // This is the ZigBee equivalent of Z-Wave Start level change.
  // We don't bother and just do a regular set towards boundaries (0 or 255)
  // with an estimated transition time.
  uint8_t level = DEFAULT_MIN_LEVEL;
  if (move_mode == ZCL_MOVE_STEP_MODE_DOWN) {
    level = DEFAULT_MIN_LEVEL;
  } else if (move_mode == ZCL_MOVE_STEP_MODE_UP) {
    level = DEFAULT_MAX_LEVEL;
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
  dotdot_set_level_current_level(unid,
                                 endpoint,
                                 DESIRED_ATTRIBUTE,
                                 (uint32_t)level);
  if (dotdot_is_supported_level_on_off_transition_time(unid, endpoint)) {
    dotdot_set_level_on_off_transition_time(unid,
                                            endpoint,
                                            DESIRED_ATTRIBUTE,
                                            (uint32_t)transition_time);
  }

  // Do the with OnOff magic:
  // Do not go off here, the multilevel switch level going to zero will make
  // the OnOff attribute Off.
  if (level > DEFAULT_MIN_LEVEL) {
    dotdot_set_on_off_on_off(unid, endpoint, DESIRED_ATTRIBUTE, ON);
  }

  //FIXME: We ignore OptionsMask / OptionsOverride for now.
  return SL_STATUS_OK;
}

sl_status_t level_cluster_mapper_step_with_on_off(
  dotdot_unid_t unid,
  uint8_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  MoveStepMode step_mode,
  uint8_t step_size,
  uint16_t transition_time,
  [[maybe_unused]] uint8_t options_mask,
  [[maybe_unused]] uint8_t options_override)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return (dotdot_is_supported_level_current_level(unid, endpoint)
            && dotdot_is_supported_level_on_off_transition_time(unid, endpoint)
            && dotdot_is_supported_on_off_on_off(unid, endpoint))
             ? SL_STATUS_OK
             : SL_STATUS_FAIL;
  }

  // Step size 0 will not get us moving.
  if (step_size == 0) {
    return SL_STATUS_OK;
  }

  int32_t current_reported_level
    = dotdot_get_level_current_level(unid, endpoint, REPORTED_ATTRIBUTE);

  // Apply the Step to our current level.
  uint8_t level = 0;
  if (step_mode == ZCL_MOVE_STEP_MODE_DOWN) {
    level = current_reported_level - step_size;
  } else if (step_mode == ZCL_MOVE_STEP_MODE_UP) {
    level = current_reported_level + step_size;
  }

  // Boundary verification
  if (level > DEFAULT_MAX_LEVEL) {
    level = DEFAULT_MAX_LEVEL;
  } else if (level < DEFAULT_MIN_LEVEL) {
    level = DEFAULT_MIN_LEVEL;
  }

  // Transition time 0xFFFF will be converted to "as fast as possible."
  if (transition_time == 0xFFFF) {
    transition_time = 0;
  }

  // Make the transition with our calculated target level
  dotdot_set_level_current_level(unid,
                                 endpoint,
                                 DESIRED_ATTRIBUTE,
                                 (uint32_t)level);
  dotdot_set_level_on_off_transition_time(unid,
                                          endpoint,
                                          DESIRED_ATTRIBUTE,
                                          (uint32_t)transition_time);

  // Do the with OnOff magic:
  // Do not go off here, the multilevel switch level going to zero will make
  // the OnOff attribute Off.
  if (level > DEFAULT_MIN_LEVEL) {
    dotdot_set_on_off_on_off(unid, endpoint, DESIRED_ATTRIBUTE, ON);
  }

  //FIXME: We ignore OptionsMask / OptionsOverride for now.
  return SL_STATUS_OK;
}

sl_status_t level_cluster_mapper_stop_with_on_off(
  dotdot_unid_t unid,
  uint8_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  [[maybe_unused]] uint8_t options_mask,
  [[maybe_unused]] uint8_t options_override)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    return (dotdot_is_supported_level_current_level(unid, endpoint)
            && dotdot_is_supported_level_on_off_transition_time(unid, endpoint)
            && dotdot_is_supported_on_off_on_off(unid, endpoint))
             ? SL_STATUS_OK
             : SL_STATUS_FAIL;
  }
  // FIXME: Here we may need ZCL / ZigBee gurus: If I do a stop with OnOff,
  // when the current level is non-zero and the OnOff is 0,
  // do I switch the OnOff to On ??

  // Set the remaining of the transition time to 0.
  dotdot_set_level_on_off_transition_time(unid, endpoint, DESIRED_ATTRIBUTE, 0);

  // Set the desired value to the current reported.
  int32_t current_reported_level
    = dotdot_get_level_current_level(unid, endpoint, REPORTED_ATTRIBUTE);
  dotdot_set_level_current_level(unid,
                                 endpoint,
                                 DESIRED_ATTRIBUTE,
                                 current_reported_level);

  // Do the with OnOff magic:
  if (current_reported_level > DEFAULT_MIN_LEVEL) {
    dotdot_set_on_off_on_off(unid, endpoint, DESIRED_ATTRIBUTE, ON);
  }

  //FIXME: We ignore OptionsMask / OptionsOverride for now.
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Init and teardown functions
///////////////////////////////////////////////////////////////////////////////
bool level_cluster_mapper_init()
{
  sl_log_debug(LOG_TAG, "Level cluster mapper initialization");

  uic_mqtt_dotdot_level_move_to_level_callback_set(
    level_cluster_mapper_move_to_level);

  uic_mqtt_dotdot_level_move_callback_set(level_cluster_mapper_move);

  uic_mqtt_dotdot_level_step_callback_set(level_cluster_mapper_step);

  uic_mqtt_dotdot_level_stop_callback_set(level_cluster_mapper_stop);

  uic_mqtt_dotdot_level_move_to_level_with_on_off_callback_set(
    level_cluster_mapper_move_to_level_with_on_off);

  uic_mqtt_dotdot_level_move_with_on_off_callback_set(
    level_cluster_mapper_move_with_on_off);

  uic_mqtt_dotdot_level_step_with_on_off_callback_set(
    level_cluster_mapper_step_with_on_off);

  uic_mqtt_dotdot_level_stop_with_on_off_callback_set(
    level_cluster_mapper_stop_with_on_off);

  return true;
}

int level_cluster_mapper_teardown()
{
  sl_log_debug(LOG_TAG, "Teardown of the Level cluster mapper");
  return 0;
}
