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
#include "unify_dotdot_attribute_store_command_callbacks_color_control.h"
#include "unify_dotdot_attribute_store_helpers.h"
#include "unify_dotdot_attribute_store_configuration.h"
#include "unify_dotdot_defined_attribute_types.h"

// Includes from Unify
#include "sl_log.h"
#include "sl_status.h"

// Includes from auto-generated files
#include "dotdot_mqtt.h"

// Generic includes
#include <stdlib.h>

// Setup Log ID
#define LOG_TAG "unify_dotdot_attribute_store_color_control_commands_callbacks"
//
// Macro to return if ColorControl is supposed
// Bismask is from the ColorCapabilities
// Return no matter what if the functi
#define HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, bitmask)          \
  if (0                                                                   \
      == (dotdot_get_color_control_color_capabilities(unid,               \
                                                      endpoint,           \
                                                      REPORTED_ATTRIBUTE) \
          & bitmask)) {                                                   \
    return SL_STATUS_FAIL;                                                \
  } else if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {  \
    return is_automatic_deduction_of_supported_commands_enabled()         \
             ? SL_STATUS_OK                                               \
             : SL_STATUS_FAIL;                                            \
  }

const uint16_t HUE_SATURATION_BITMASK = 0x1;
const uint8_t COLOR_COLOR_MODE        = 0;
const uint8_t DEFAULT_MAX_COLOR       = 0xfe;
const uint8_t DEFAULT_MIN_COLOR       = 0x00;
const uint16_t TEMPERATURE_BITMASK    = 0x10;
const uint8_t TEMPERATURE_COLOR_MODE  = 2;

/**
 * @brief Verifies that the color is within the allowed range and returns a valid
 * value from it.
 *
 * @param color     The value that we received
 * @return uint8_t  The validated value that we should apply.
 */
static uint8_t validated_color_value(uint8_t color)
{
  if (color > DEFAULT_MAX_COLOR) {
    return DEFAULT_MAX_COLOR;
  }

  if (color < DEFAULT_MIN_COLOR) {
    return DEFAULT_MIN_COLOR;
  }

  return color;
}

/**
 * @brief Sets all the Color modes attributes of a device to the new value.
 * Will set both desired and reported values.
 *
 * @param unid               UNID of the node to set.
 * @param endpoint           Endpoint under which the color mode is set
 * @param color_mode         New color mode
 */
static void set_color_modes(dotdot_unid_t unid,
                            dotdot_endpoint_id_t endpoint,
                            uint8_t color_mode)
{
  if (color_mode <= 2) {
    dotdot_set_color_control_color_mode(unid,
                                        endpoint,
                                        DESIRED_ATTRIBUTE,
                                        color_mode);
    dotdot_set_color_control_color_mode(unid,
                                        endpoint,
                                        REPORTED_ATTRIBUTE,
                                        color_mode);
  }
  if (color_mode <= 3) {
    dotdot_set_color_control_enhanced_color_mode(unid,
                                                 endpoint,
                                                 DESIRED_ATTRIBUTE,
                                                 color_mode);
    dotdot_set_color_control_enhanced_color_mode(unid,
                                                 endpoint,
                                                 REPORTED_ATTRIBUTE,
                                                 color_mode);
  }
}

static void dotdot_set_color_control_temperature_mireds_remaining_time(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uint16_t color_temperature_mireds,
  uint16_t remaining_time)
{
  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(LOG_TAG,
                 "Updating ZCL desired values after "
                 "ColorControl:TemperatureMireds command");
    dotdot_set_color_control_color_temperature_mireds(unid,
                                                      endpoint,
                                                      DESIRED_ATTRIBUTE,
                                                      color_temperature_mireds);
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG,
                   "Clearing ColorControl:TemperatureMireds reported value");
      dotdot_color_control_color_temperature_mireds_undefine_reported(unid,
                                                                      endpoint);
    }
  }
  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(
      LOG_TAG,
      "Updating ZCL desired values after ColorControl:RemainingTime command");
    dotdot_set_color_control_remaining_time(unid,
                                            endpoint,
                                            DESIRED_ATTRIBUTE,
                                            remaining_time);
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG,
                   "Clearing ColorControl:RemainingTime reported value");
      dotdot_color_control_remaining_time_undefine_reported(unid, endpoint);
    }
  }
}

static void dotdot_set_color_control_hue_saturation_remaining_time(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uint8_t hue,
  uint8_t saturation,
  uint16_t remaining_time)
{
  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(
      LOG_TAG,
      "Updating ZCL desired values after ColorControl:CurrentHue command");
    dotdot_set_color_control_current_hue(unid,
                                         endpoint,
                                         DESIRED_ATTRIBUTE,
                                         hue);
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG, "Clearing ColorControl:CurrentHue reported value");
      dotdot_color_control_current_hue_undefine_reported(unid, endpoint);
    }
  }
  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(
      LOG_TAG,
      "Updating ZCL desired values after ColorControl:RemainingTime command");
    dotdot_set_color_control_remaining_time(unid,
                                            endpoint,
                                            DESIRED_ATTRIBUTE,
                                            remaining_time);
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG,
                   "Clearing ColorControl:RemainingTime reported value");
      dotdot_color_control_remaining_time_undefine_reported(unid, endpoint);
    }
  }

  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(
      LOG_TAG,
      "Updating ZCL desired values after ColorControl:Saturation command");
    dotdot_set_color_control_current_saturation(unid,
                                                endpoint,
                                                DESIRED_ATTRIBUTE,
                                                saturation);
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG,
                   "Clearing ColorControl:RemainingTime reported value");
      dotdot_color_control_current_saturation_undefine_reported(unid, endpoint);
    }
  }
}

sl_status_t color_control_move_to_hue_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint8_t hue,
  CCDirection direction,
  uint16_t transition_time,
  uint8_t options_mask,
  uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, HUE_SATURATION_BITMASK)

  // Warning: The CCDirection is just too ninja hard for us, we will move
  // in whatever direction does not force us to wrap around a uint8_t
  // during a transition.
  if (hue >= 0xFE) {
    hue = 0xFE;
  }

  set_color_modes(unid, endpoint, COLOR_COLOR_MODE);

  dotdot_set_color_control_hue_saturation_remaining_time(
    unid,
    endpoint,
    hue,
    dotdot_get_color_control_current_saturation(unid,
                                                endpoint,
                                                REPORTED_ATTRIBUTE),
    transition_time);
  return SL_STATUS_OK;
}

sl_status_t color_control_move_hue_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  CCMoveMode move_mode,
  uint8_t rate,
  uint8_t options_mask,
  uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, HUE_SATURATION_BITMASK)

  // Received a Hue/Saturation command, change to Hue/Saturation color mode
  set_color_modes(unid, endpoint, COLOR_COLOR_MODE);

  if (rate == 0x00)
    return SL_STATUS_FAIL;

  uint8_t desired_hue;
  uint8_t current_hue
    = dotdot_get_color_control_current_hue(unid, endpoint, REPORTED_ATTRIBUTE);
  switch (move_mode) {
    case ZCL_CC_MOVE_MODE_STOP:
      desired_hue = current_hue;
      break;
    case ZCL_CC_MOVE_MODE_UP:
      desired_hue = DEFAULT_MAX_COLOR;
      break;
    case ZCL_CC_MOVE_MODE_DOWN:
      desired_hue = DEFAULT_MIN_COLOR;
      break;
    default:  // move_mode = 0x02 - Reserved
      return SL_STATUS_FAIL;
  }

  uint16_t remaining_time = 0;
  if (current_hue > desired_hue) {
    remaining_time = (current_hue - desired_hue) / rate;
  } else {
    remaining_time = (desired_hue - current_hue) / rate;
  }
  dotdot_set_color_control_hue_saturation_remaining_time(
    unid,
    endpoint,
    desired_hue,
    dotdot_get_color_control_current_saturation(unid,
                                                endpoint,
                                                REPORTED_ATTRIBUTE),
    remaining_time);
  return SL_STATUS_OK;
}

sl_status_t color_control_step_hue_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  CCStepMode step_mode,
  uint8_t step_size,
  uint8_t transition_time,
  uint8_t options_mask,
  uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, HUE_SATURATION_BITMASK)

  // Received a Hue/Saturation command, change to Hue/Saturation color mode
  set_color_modes(unid, endpoint, COLOR_COLOR_MODE);

  // Step size 0 will not get us moving.
  if (step_size == 0) {
    return SL_STATUS_OK;
  }

  uint8_t desired_hue;
  uint8_t current_hue
    = dotdot_get_color_control_current_hue(unid, endpoint, REPORTED_ATTRIBUTE);

  if (step_mode == ZCL_CC_STEP_MODE_UP)
    desired_hue = current_hue + step_size;
  else if (step_mode == ZCL_CC_STEP_MODE_DOWN)
    desired_hue = current_hue - step_size;
  else
    return SL_STATUS_FAIL;  // step_mode = 0x00, 0x02 - Reserved

  desired_hue = validated_color_value(desired_hue);

  dotdot_set_color_control_hue_saturation_remaining_time(
    unid,
    endpoint,
    desired_hue,
    dotdot_get_color_control_current_saturation(unid,
                                                endpoint,
                                                REPORTED_ATTRIBUTE),
    transition_time);
  return SL_STATUS_OK;
}

sl_status_t color_control_move_to_hue_and_saturation_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint8_t hue,
  uint8_t saturation,
  uint16_t transition_time,
  uint8_t options_mask,
  uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, HUE_SATURATION_BITMASK)

  // Received a Hue/Saturation command, change to Hue/Saturation color mode
  set_color_modes(unid, endpoint, COLOR_COLOR_MODE);

  if (hue >= 0xFE) {
    hue = 0xFE;
  }
  if (saturation >= 0xFE) {
    saturation = 0xFE;
  }
  dotdot_set_color_control_hue_saturation_remaining_time(unid,
                                                         endpoint,
                                                         hue,
                                                         saturation,
                                                         transition_time);
  return SL_STATUS_OK;
}

sl_status_t color_control_move_to_saturation_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint8_t saturation,
  uint16_t transition_time,
  uint8_t options_mask,
  uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, HUE_SATURATION_BITMASK)

  // Received a Hue/Saturation command, change to Hue/Saturation color mode
  set_color_modes(unid, endpoint, COLOR_COLOR_MODE);

  if (saturation >= 0xFE) {
    saturation = 0xFE;
  }
  dotdot_set_color_control_hue_saturation_remaining_time(
    unid,
    endpoint,
    dotdot_get_color_control_current_hue(unid, endpoint, REPORTED_ATTRIBUTE),
    saturation,
    transition_time);
  return SL_STATUS_OK;
}

sl_status_t color_control_move_saturation_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  CCMoveMode move_mode,
  uint8_t rate,
  uint8_t options_mask,
  uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, HUE_SATURATION_BITMASK)

  // Received a Hue/Saturation command, change to Hue/Saturation color mode
  set_color_modes(unid, endpoint, COLOR_COLOR_MODE);

  if (rate == 0x00) {
    return SL_STATUS_FAIL;
  }

  uint8_t desired_saturation;
  uint8_t current_saturation
    = dotdot_get_color_control_current_saturation(unid,
                                                  endpoint,
                                                  REPORTED_ATTRIBUTE);
  switch (move_mode) {
    case ZCL_CC_MOVE_MODE_STOP:
      desired_saturation = current_saturation;
      break;
    case ZCL_CC_MOVE_MODE_UP:
      desired_saturation = DEFAULT_MAX_COLOR;
      break;
    case ZCL_CC_MOVE_MODE_DOWN:
      desired_saturation = DEFAULT_MIN_COLOR;
      break;
    default:  // move_mode = 0x02 - Reserved
      return SL_STATUS_FAIL;
  }

  uint16_t remaining_time
    = (uint16_t)abs(current_saturation - desired_saturation) / rate;

  dotdot_set_color_control_hue_saturation_remaining_time(
    unid,
    endpoint,
    dotdot_get_color_control_current_hue(unid, endpoint, REPORTED_ATTRIBUTE),
    desired_saturation,
    remaining_time);
  return SL_STATUS_OK;
}

sl_status_t color_control_step_saturation_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  CCStepMode step_mode,
  uint8_t step_size,
  uint8_t transition_time,
  uint8_t options_mask,
  uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, HUE_SATURATION_BITMASK)

  // Received a Hue/Saturation command, change to Hue/Saturation color mode
  set_color_modes(unid, endpoint, COLOR_COLOR_MODE);

  // Step size 0 will not get us moving.
  if (step_size == 0) {
    return SL_STATUS_OK;
  }

  uint8_t desired_saturation;
  uint8_t current_saturation
    = dotdot_get_color_control_current_saturation(unid,
                                                  endpoint,
                                                  REPORTED_ATTRIBUTE);

  if (step_mode == ZCL_CC_STEP_MODE_UP) {
    desired_saturation = current_saturation + step_size;
  } else if (step_mode == ZCL_CC_STEP_MODE_DOWN) {
    desired_saturation = current_saturation - step_size;
  } else {  // step_mode = 0x00, 0x02 - Reserved
    return SL_STATUS_FAIL;
  }

  desired_saturation = validated_color_value(desired_saturation);

  dotdot_set_color_control_hue_saturation_remaining_time(
    unid,
    endpoint,
    dotdot_get_color_control_current_hue(unid, endpoint, REPORTED_ATTRIBUTE),
    desired_saturation,
    transition_time);
  return SL_STATUS_OK;
}

sl_status_t color_control_move_to_color_temperature_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint16_t color_temperature_mireds,
  uint16_t transition_time,
  uint8_t options_mask,
  uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, TEMPERATURE_BITMASK)

  // Received a Temperature command, change to Temperature color mode
  set_color_modes(unid, endpoint, TEMPERATURE_COLOR_MODE);

  // Range check:
  if (color_temperature_mireds
      > dotdot_get_color_control_color_temp_physical_max_mireds(
        unid,
        endpoint,
        REPORTED_ATTRIBUTE)) {
    color_temperature_mireds
      = dotdot_get_color_control_color_temp_physical_max_mireds(
        unid,
        endpoint,
        REPORTED_ATTRIBUTE);
  } else if (color_temperature_mireds
             < dotdot_get_color_control_color_temp_physical_min_mireds(
               unid,
               endpoint,
               REPORTED_ATTRIBUTE)) {
    color_temperature_mireds
      = dotdot_get_color_control_color_temp_physical_min_mireds(
        unid,
        endpoint,
        REPORTED_ATTRIBUTE);
  }

  dotdot_set_color_control_temperature_mireds_remaining_time(
    unid,
    endpoint,
    color_temperature_mireds,
    transition_time);

  return SL_STATUS_OK;
}

sl_status_t color_control_move_color_temperature_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  CCMoveMode move_mode,
  uint16_t rate,
  CCMinMiredsField color_temperature_minimum_mireds,
  CCMaxMiredsField color_temperature_maximum_mireds,
  uint8_t options_mask,
  uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, TEMPERATURE_BITMASK)

  // Received a Temperature command, change to Temperature color mode
  set_color_modes(unid, endpoint, TEMPERATURE_COLOR_MODE);

  if (rate == 0x00) {
    return SL_STATUS_OK;
  }

  uint16_t reported_temperature
    = dotdot_get_color_control_color_temperature_mireds(unid,
                                                        endpoint,
                                                        REPORTED_ATTRIBUTE);
  uint16_t desired_temperature = 0;
  switch (move_mode) {
    case ZCL_CC_MOVE_MODE_STOP:
      desired_temperature = reported_temperature;
      break;
    case ZCL_CC_MOVE_MODE_UP:
      desired_temperature = color_temperature_maximum_mireds;
      break;
    case ZCL_CC_MOVE_MODE_DOWN:
      desired_temperature = color_temperature_minimum_mireds;
      break;
    default:  // move_mode = 0x02 - Reserved
      desired_temperature = reported_temperature;
  }
  // Range check
  if (desired_temperature
      > dotdot_get_color_control_color_temp_physical_max_mireds(
        unid,
        endpoint,
        REPORTED_ATTRIBUTE)) {
    desired_temperature
      = dotdot_get_color_control_color_temp_physical_max_mireds(
        unid,
        endpoint,
        REPORTED_ATTRIBUTE);
  } else if (desired_temperature
             < dotdot_get_color_control_color_temp_physical_min_mireds(
               unid,
               endpoint,
               REPORTED_ATTRIBUTE)) {
    desired_temperature
      = dotdot_get_color_control_color_temp_physical_min_mireds(
        unid,
        endpoint,
        REPORTED_ATTRIBUTE);
  }

  uint16_t remaining_time
    = (uint16_t)abs(reported_temperature - desired_temperature) / rate;

  dotdot_set_color_control_temperature_mireds_remaining_time(
    unid,
    endpoint,
    desired_temperature,
    remaining_time);
  return SL_STATUS_OK;
}

sl_status_t color_control_step_color_temperature_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  CCStepMode step_mode,
  uint16_t step_size,
  uint16_t transition_time,
  CCMinMiredsField color_temperature_minimum_mireds,
  CCMaxMiredsField color_temperature_maximum_mireds,
  uint8_t options_mask,
  uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, TEMPERATURE_BITMASK)

  // Received a Temperature command, change to Temperature color mode
  set_color_modes(unid, endpoint, TEMPERATURE_COLOR_MODE);

  // Step size 0 will not get us moving.
  if (step_size == 0) {
    return SL_STATUS_OK;
  }
  uint16_t reported_temperature
    = dotdot_get_color_control_color_temperature_mireds(unid,
                                                        endpoint,
                                                        REPORTED_ATTRIBUTE);
  uint16_t desired_temperature = 0;

  if (step_mode == ZCL_CC_STEP_MODE_UP) {
    desired_temperature = reported_temperature + step_size;
  } else if (step_mode == ZCL_CC_STEP_MODE_DOWN) {
    desired_temperature = reported_temperature - step_size;
  } else {  // step_mode = 0x00, 0x02 - Reserved
    return SL_STATUS_FAIL;
  }

  if (desired_temperature < color_temperature_minimum_mireds) {
    desired_temperature = color_temperature_minimum_mireds;
  } else if (desired_temperature > color_temperature_maximum_mireds) {
    desired_temperature = color_temperature_maximum_mireds;
  }

  // Range check
  if (desired_temperature
      > dotdot_get_color_control_color_temp_physical_max_mireds(
        unid,
        endpoint,
        REPORTED_ATTRIBUTE)) {
    desired_temperature
      = dotdot_get_color_control_color_temp_physical_max_mireds(
        unid,
        endpoint,
        REPORTED_ATTRIBUTE);
  } else if (desired_temperature
             < dotdot_get_color_control_color_temp_physical_min_mireds(
               unid,
               endpoint,
               REPORTED_ATTRIBUTE)) {
    desired_temperature
      = dotdot_get_color_control_color_temp_physical_min_mireds(
        unid,
        endpoint,
        REPORTED_ATTRIBUTE);
  }

  dotdot_set_color_control_temperature_mireds_remaining_time(
    unid,
    endpoint,
    desired_temperature,
    transition_time);
  return SL_STATUS_OK;
}

sl_status_t color_control_stop_move_step_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint8_t options_mask,
  uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid,
                       endpoint,
                       call_type,
                       (TEMPERATURE_BITMASK | HUE_SATURATION_BITMASK))
  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(
      LOG_TAG,
      "Updating ZCL desired values after ColorControl:RemainingTime command");
    dotdot_set_color_control_color_temperature_mireds(
      unid,
      endpoint,
      DESIRED_ATTRIBUTE,
      dotdot_get_color_control_color_temperature_mireds(unid,
                                                        endpoint,
                                                        REPORTED_ATTRIBUTE));
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG,
                   "Clearing ColorControl:RemainingTime reported value");
      dotdot_color_control_color_temperature_mireds_undefine_reported(unid,
                                                                      endpoint);
    }
  }

  // Stop any transition, remaining time is 0.
  dotdot_set_color_control_hue_saturation_remaining_time(
    unid,
    endpoint,
    dotdot_get_color_control_current_hue(unid, endpoint, REPORTED_ATTRIBUTE),
    dotdot_get_color_control_current_saturation(unid,
                                                endpoint,
                                                REPORTED_ATTRIBUTE),
    0);

  return SL_STATUS_OK;
}

void color_control_cluster_mapper_init()
{
  sl_log_debug(LOG_TAG, "Color control cluster mapper initialization");
  // Color Control Hue commands
  uic_mqtt_dotdot_color_control_move_to_hue_callback_set(
    &color_control_move_to_hue_callback);
  uic_mqtt_dotdot_color_control_move_hue_callback_set(
    &color_control_move_hue_callback);
  uic_mqtt_dotdot_color_control_step_hue_callback_set(
    &color_control_step_hue_callback);

  // Hue and Saturation commands
  uic_mqtt_dotdot_color_control_move_to_hue_and_saturation_callback_set(
    &color_control_move_to_hue_and_saturation_callback);

  // Saturation commands
  uic_mqtt_dotdot_color_control_move_to_saturation_callback_set(
    &color_control_move_to_saturation_callback);
  uic_mqtt_dotdot_color_control_move_saturation_callback_set(
    &color_control_move_saturation_callback);
  uic_mqtt_dotdot_color_control_step_saturation_callback_set(
    &color_control_step_saturation_callback);

  // Temperature commands
  uic_mqtt_dotdot_color_control_move_to_color_temperature_callback_set(
    &color_control_move_to_color_temperature_callback);
  uic_mqtt_dotdot_color_control_move_color_temperature_callback_set(
    &color_control_move_color_temperature_callback);
  uic_mqtt_dotdot_color_control_step_color_temperature_callback_set(
    &color_control_step_color_temperature_callback);

  // Generic commands
  uic_mqtt_dotdot_color_control_stop_move_step_callback_set(
    &color_control_stop_move_step_callback);
}
