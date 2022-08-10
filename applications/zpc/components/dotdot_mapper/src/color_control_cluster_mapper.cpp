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
#include "color_control_cluster_mapper.h"
#include "dotdot_attributes.h"

// Includes from Unify
#include "sl_log.h"
#include "sl_status.h"

// Generic includes
#include <stdlib.h>

// Includes from auto-generated files
#include "dotdot_mqtt.h"

constexpr char LOG_TAG[]            = "color_switch_cluster_mapper";
constexpr uint8_t DEFAULT_MIN_COLOR = 0x00;
constexpr uint8_t DEFAULT_MAX_COLOR = 0xfe;

constexpr uint8_t COLOR_COLOR_MODE       = 0;
constexpr uint8_t TEMPERATURE_COLOR_MODE = 2;

constexpr uint16_t HUE_SATURATION_BITMASK = 0x1;
constexpr uint16_t TEMPERATURE_BITMASK    = 0x10;

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

// Macro to return if ColorControl is supposed
// Bismask is from the ColorCapabilities
#define HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, bitmask)            \
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {           \
    return (dotdot_get_color_control_color_capabilities(unid,               \
                                                        endpoint,           \
                                                        REPORTED_ATTRIBUTE) \
            & bitmask)                                                      \
             ? SL_STATUS_OK                                                 \
             : SL_STATUS_FAIL;                                              \
  }

///////////////////////////////////////////////////////////////////////////////
// Command Callback functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t uic_mqtt_dotdot_color_control_move_to_hue_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint8_t hue,
  [[maybe_unused]] CCDirection direction,
  uint16_t transition_time,
  [[maybe_unused]] uint8_t options_mask,
  [[maybe_unused]] uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, HUE_SATURATION_BITMASK)

  // Warning: The CCDirection is just too ninja hard for us, we will move
  // in whatever direction does not force us to wrap around a uint8_t
  // during a transition.
  if (hue >= 0xFE) {
    hue = 0xFE;
  }

  // Received a Hue/Saturation command, change to Hue/Saturation color mode
  dotdot_set_color_control_color_mode(unid,
                                      endpoint,
                                      REPORTED_ATTRIBUTE,
                                      COLOR_COLOR_MODE);

  dotdot_set_color_control_current_hue(unid, endpoint, DESIRED_ATTRIBUTE, hue);
  dotdot_set_color_control_remaining_time(unid,
                                          endpoint,
                                          DESIRED_ATTRIBUTE,
                                          transition_time);

  dotdot_set_color_control_current_saturation(
    unid,
    endpoint,
    DESIRED_ATTRIBUTE,
    dotdot_get_color_control_current_saturation(unid,
                                                endpoint,
                                                REPORTED_ATTRIBUTE));

  return SL_STATUS_OK;
}

sl_status_t uic_mqtt_dotdot_color_control_move_hue_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  CCMoveMode move_mode,
  uint8_t rate,
  [[maybe_unused]] uint8_t options_mask,
  [[maybe_unused]] uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, HUE_SATURATION_BITMASK)

  // Received a Hue/Saturation command, change to Hue/Saturation color mode
  dotdot_set_color_control_color_mode(unid,
                                      endpoint,
                                      REPORTED_ATTRIBUTE,
                                      COLOR_COLOR_MODE);

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
  dotdot_set_color_control_remaining_time(unid,
                                          endpoint,
                                          DESIRED_ATTRIBUTE,
                                          remaining_time);
  dotdot_set_color_control_current_hue(unid,
                                       endpoint,
                                       DESIRED_ATTRIBUTE,
                                       desired_hue);

  dotdot_set_color_control_current_saturation(
    unid,
    endpoint,
    DESIRED_ATTRIBUTE,
    dotdot_get_color_control_current_saturation(unid,
                                                endpoint,
                                                REPORTED_ATTRIBUTE));
  return SL_STATUS_OK;
}

sl_status_t uic_mqtt_dotdot_color_control_step_hue_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  CCStepMode step_mode,
  uint8_t step_size,
  uint8_t transition_time,
  [[maybe_unused]] uint8_t options_mask,
  [[maybe_unused]] uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, HUE_SATURATION_BITMASK)

  // Received a Hue/Saturation command, change to Hue/Saturation color mode
  dotdot_set_color_control_color_mode(unid,
                                      endpoint,
                                      REPORTED_ATTRIBUTE,
                                      COLOR_COLOR_MODE);

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

  dotdot_set_color_control_current_hue(unid,
                                       endpoint,
                                       DESIRED_ATTRIBUTE,
                                       desired_hue);
  dotdot_set_color_control_remaining_time(unid,
                                          endpoint,
                                          DESIRED_ATTRIBUTE,
                                          transition_time);

  dotdot_set_color_control_current_saturation(
    unid,
    endpoint,
    DESIRED_ATTRIBUTE,
    dotdot_get_color_control_current_saturation(unid,
                                                endpoint,
                                                REPORTED_ATTRIBUTE));

  return SL_STATUS_OK;
}

sl_status_t uic_mqtt_dotdot_color_control_move_to_saturation_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint8_t saturation,
  uint16_t transition_time,
  [[maybe_unused]] uint8_t options_mask,
  [[maybe_unused]] uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, HUE_SATURATION_BITMASK)

  // Received a Hue/Saturation command, change to Hue/Saturation color mode
  dotdot_set_color_control_color_mode(unid,
                                      endpoint,
                                      REPORTED_ATTRIBUTE,
                                      COLOR_COLOR_MODE);

  if (saturation >= 0xFE) {
    saturation = 0xFE;
  }
  dotdot_set_color_control_current_saturation(unid,
                                              endpoint,
                                              DESIRED_ATTRIBUTE,
                                              saturation);

  dotdot_set_color_control_remaining_time(unid,
                                          endpoint,
                                          DESIRED_ATTRIBUTE,
                                          transition_time);

  dotdot_set_color_control_current_hue(
    unid,
    endpoint,
    DESIRED_ATTRIBUTE,
    dotdot_get_color_control_current_hue(unid, endpoint, REPORTED_ATTRIBUTE));
  return SL_STATUS_OK;
}

sl_status_t uic_mqtt_dotdot_color_control_move_saturation_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  CCMoveMode move_mode,
  uint8_t rate,
  [[maybe_unused]] uint8_t options_mask,
  [[maybe_unused]] uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, HUE_SATURATION_BITMASK)

  // Received a Hue/Saturation command, change to Hue/Saturation color mode
  dotdot_set_color_control_color_mode(unid,
                                      endpoint,
                                      REPORTED_ATTRIBUTE,
                                      COLOR_COLOR_MODE);

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

  dotdot_set_color_control_remaining_time(unid,
                                          endpoint,
                                          DESIRED_ATTRIBUTE,
                                          remaining_time);

  dotdot_set_color_control_current_saturation(unid,
                                              endpoint,
                                              DESIRED_ATTRIBUTE,
                                              desired_saturation);

  dotdot_set_color_control_current_hue(
    unid,
    endpoint,
    DESIRED_ATTRIBUTE,
    dotdot_get_color_control_current_hue(unid, endpoint, REPORTED_ATTRIBUTE));

  return SL_STATUS_OK;
}

sl_status_t uic_mqtt_dotdot_color_control_step_saturation_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  CCStepMode step_mode,
  uint8_t step_size,
  uint8_t transition_time,
  [[maybe_unused]] uint8_t options_mask,
  [[maybe_unused]] uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, HUE_SATURATION_BITMASK)

  // Received a Hue/Saturation command, change to Hue/Saturation color mode
  dotdot_set_color_control_color_mode(unid,
                                      endpoint,
                                      REPORTED_ATTRIBUTE,
                                      COLOR_COLOR_MODE);

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

  dotdot_set_color_control_current_saturation(unid,
                                              endpoint,
                                              DESIRED_ATTRIBUTE,
                                              desired_saturation);
  dotdot_set_color_control_remaining_time(unid,
                                          endpoint,
                                          DESIRED_ATTRIBUTE,
                                          transition_time);

  dotdot_set_color_control_current_hue(
    unid,
    endpoint,
    DESIRED_ATTRIBUTE,
    dotdot_get_color_control_current_hue(unid, endpoint, REPORTED_ATTRIBUTE));

  return SL_STATUS_OK;
}

sl_status_t uic_mqtt_dotdot_color_control_move_to_hue_and_saturation_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint8_t hue,
  uint8_t saturation,
  uint16_t transition_time,
  [[maybe_unused]] uint8_t options_mask,
  [[maybe_unused]] uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, HUE_SATURATION_BITMASK)

  // Received a Hue/Saturation command, change to Hue/Saturation color mode
  dotdot_set_color_control_color_mode(unid,
                                      endpoint,
                                      REPORTED_ATTRIBUTE,
                                      COLOR_COLOR_MODE);

  if (hue >= 0xFE) {
    hue = 0xFE;
  }
  dotdot_set_color_control_current_hue(unid, endpoint, DESIRED_ATTRIBUTE, hue);
  if (saturation >= 0xFE) {
    saturation = 0xFE;
  }
  dotdot_set_color_control_current_saturation(unid,
                                              endpoint,
                                              DESIRED_ATTRIBUTE,
                                              saturation);

  dotdot_set_color_control_remaining_time(unid,
                                          endpoint,
                                          DESIRED_ATTRIBUTE,
                                          transition_time);

  return SL_STATUS_OK;
}

sl_status_t uic_mqtt_dotdot_color_control_move_to_color_temperature_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint16_t color_temperature_mireds,
  uint16_t transition_time,
  [[maybe_unused]] uint8_t options_mask,
  [[maybe_unused]] uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, TEMPERATURE_BITMASK)

  // Received a Temperature command, change to Temperature color mode
  dotdot_set_color_control_color_mode(unid,
                                      endpoint,
                                      REPORTED_ATTRIBUTE,
                                      TEMPERATURE_COLOR_MODE);

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

  dotdot_set_color_control_color_temperature_mireds(unid,
                                                    endpoint,
                                                    DESIRED_ATTRIBUTE,
                                                    color_temperature_mireds);

  dotdot_set_color_control_remaining_time(unid,
                                          endpoint,
                                          DESIRED_ATTRIBUTE,
                                          transition_time);

  return SL_STATUS_OK;
}

sl_status_t uic_mqtt_dotdot_color_control_move_color_temperature_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  CCMoveMode move_mode,
  uint16_t rate,
  CCMinMiredsField color_temperature_minimum_mireds,
  CCMaxMiredsField color_temperature_maximum_mireds,
  [[maybe_unused]] uint8_t options_mask,
  [[maybe_unused]] uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, TEMPERATURE_BITMASK)

  // Received a Temperature command, change to Temperature color mode
  dotdot_set_color_control_color_mode(unid,
                                      endpoint,
                                      REPORTED_ATTRIBUTE,
                                      TEMPERATURE_COLOR_MODE);

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

  dotdot_set_color_control_color_temperature_mireds(unid,
                                                    endpoint,
                                                    DESIRED_ATTRIBUTE,
                                                    desired_temperature);

  dotdot_set_color_control_remaining_time(unid,
                                          endpoint,
                                          DESIRED_ATTRIBUTE,
                                          remaining_time);

  return SL_STATUS_OK;
}

sl_status_t uic_mqtt_dotdot_color_control_step_color_temperature_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  CCStepMode step_mode,
  uint16_t step_size,
  uint16_t transition_time,
  CCMinMiredsField color_temperature_minimum_mireds,
  CCMaxMiredsField color_temperature_maximum_mireds,
  [[maybe_unused]] uint8_t options_mask,
  [[maybe_unused]] uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, TEMPERATURE_BITMASK)

  // Received a Temperature command, change to Temperature color mode
  dotdot_set_color_control_color_mode(unid,
                                      endpoint,
                                      REPORTED_ATTRIBUTE,
                                      TEMPERATURE_COLOR_MODE);

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

  dotdot_set_color_control_color_temperature_mireds(unid,
                                                    endpoint,
                                                    DESIRED_ATTRIBUTE,
                                                    desired_temperature);

  dotdot_set_color_control_remaining_time(unid,
                                          endpoint,
                                          DESIRED_ATTRIBUTE,
                                          transition_time);

  return SL_STATUS_OK;
}

sl_status_t uic_mqtt_dotdot_color_control_stop_move_step_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  [[maybe_unused]] uint8_t options_mask,
  [[maybe_unused]] uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid,
                       endpoint,
                       call_type,
                       (TEMPERATURE_BITMASK | HUE_SATURATION_BITMASK))

  // Stop any transition, remaining time is 0.
  dotdot_set_color_control_remaining_time(unid, endpoint, DESIRED_ATTRIBUTE, 0);

  // Adjust all temperature, hue, saturations to get the desired at the same as reported.
  dotdot_set_color_control_color_temperature_mireds(
    unid,
    endpoint,
    DESIRED_ATTRIBUTE,
    dotdot_get_color_control_color_temperature_mireds(unid,
                                                      endpoint,
                                                      REPORTED_ATTRIBUTE));

  dotdot_set_color_control_current_saturation(
    unid,
    endpoint,
    DESIRED_ATTRIBUTE,
    dotdot_get_color_control_current_saturation(unid,
                                                endpoint,
                                                REPORTED_ATTRIBUTE));

  dotdot_set_color_control_current_hue(
    unid,
    endpoint,
    DESIRED_ATTRIBUTE,
    dotdot_get_color_control_current_hue(unid, endpoint, REPORTED_ATTRIBUTE));

  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Init and Teardown functions
///////////////////////////////////////////////////////////////////////////////
bool color_control_cluster_mapper_init(void)
{
  sl_log_debug(LOG_TAG, "ColorControl cluster mapper initialization");

  // Hue commands
  uic_mqtt_dotdot_color_control_move_to_hue_callback_set(
    &uic_mqtt_dotdot_color_control_move_to_hue_callback);
  uic_mqtt_dotdot_color_control_move_hue_callback_set(
    &uic_mqtt_dotdot_color_control_move_hue_callback);
  uic_mqtt_dotdot_color_control_step_hue_callback_set(
    &uic_mqtt_dotdot_color_control_step_hue_callback);

  // Saturation commands
  uic_mqtt_dotdot_color_control_move_to_saturation_callback_set(
    &uic_mqtt_dotdot_color_control_move_to_saturation_callback);
  uic_mqtt_dotdot_color_control_move_saturation_callback_set(
    &uic_mqtt_dotdot_color_control_move_saturation_callback);
  uic_mqtt_dotdot_color_control_step_saturation_callback_set(
    &uic_mqtt_dotdot_color_control_step_saturation_callback);

  // Hue and Saturation commands
  uic_mqtt_dotdot_color_control_move_to_hue_and_saturation_callback_set(
    &uic_mqtt_dotdot_color_control_move_to_hue_and_saturation_callback);

  // Temperature commands
  uic_mqtt_dotdot_color_control_move_to_color_temperature_callback_set(
    &uic_mqtt_dotdot_color_control_move_to_color_temperature_callback);
  uic_mqtt_dotdot_color_control_move_color_temperature_callback_set(
    &uic_mqtt_dotdot_color_control_move_color_temperature_callback);
  uic_mqtt_dotdot_color_control_step_color_temperature_callback_set(
    &uic_mqtt_dotdot_color_control_step_color_temperature_callback);

  // Generic commands
  uic_mqtt_dotdot_color_control_stop_move_step_callback_set(
    &uic_mqtt_dotdot_color_control_stop_move_step_callback);

  return true;
}

/**
 * @brief Clears the Color Control cluster mapper from all its content.
 *
 */
int color_control_cluster_mapper_teardown(void)
{
  sl_log_debug(LOG_TAG, "Teardown of the Color Control cluster mapper");
  return 0;
}