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
#include "attribute_transitions.h"

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

// CC Capabilities Bitmask
const uint16_t HUE_SATURATION_BITMASK          = 0x01;
const uint16_t ENHANCED_HUE_SATURATION_BITMASK = 0x02;
const uint16_t COLOR_LOOP_BITMASK              = 0x04;
const uint16_t COLOR_XY_BITMASK                = 0x08;
const uint16_t TEMPERATURE_BITMASK             = 0x10;

const uint8_t COLOR_COLOR_MODE       = 0;
const uint8_t TEMPERATURE_COLOR_MODE = 2;

const uint16_t DEFAULT_MAX_ENHANCED_COLOR = 0xffff;
const uint8_t DEFAULT_MAX_COLOR           = 0xfe;
const uint8_t DEFAULT_MIN_COLOR           = 0x00;
const uint16_t DEFAULT_MIN_X_Y            = 0;
const uint16_t DEFAULT_MAX_X_Y            = 65279;

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

// Return true if mask_offset matches the option 
static bool is_option_active(uint8_t option, uint8_t mask_offset) {
  return (option & (1 << mask_offset)) > 0;
}

/**
 * @brief Check if ExecuteIfOff option allows command execution according to the ZCL specification (5.2.2.3.3).
 *
 * @param unid              UNID of the node to set.
 * @param endpoint          Endpoint under which the color mode is set
 * @param options_mask      Option mask
 * @param options_override  Option override
 * 
 * The function first checks ExecuteIfOff bit in option_color_control or in options_override if options_mask contains ExecuteIfOff bit.
 *  The function then also evaluates if command can proceed based on ExecuteIfOff option check along with OnOff state
 * 
 * @return bool  false if command is not allowed to proceed as ExecuteIfOff option check, true otherwise
 */
bool execute_if_off_option_check(dotdot_unid_t unid,
                                 dotdot_endpoint_id_t endpoint,
                                 uint8_t options_mask,
                                 uint8_t options_override)
{
  uint8_t temp_option
    = dotdot_get_color_control_options(unid,
                                       endpoint,
                                       DESIRED_OR_REPORTED_ATTRIBUTE);

  // If option_mask bit for ExecuteIfOff is set we take the option_override value instead of the color_control options attribute
  if (is_option_active(options_mask, CC_COLOR_OPTIONS_EXECUTE_IF_OFF_OFFSET)) {
    temp_option = options_override;
  }
  bool execute_even_if_off_option
    = is_option_active(temp_option, CC_COLOR_OPTIONS_EXECUTE_IF_OFF_OFFSET);

  // Command has no effect if OnOff::OnOff is Off and execute_even_if_off_option is not specified
  if (!execute_even_if_off_option
      && dotdot_get_on_off_on_off(unid, endpoint, DESIRED_OR_REPORTED_ATTRIBUTE)
           == false) {
    return false;
  }
  return true;
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
  if (
    color_mode
    <= ZCL_COLOR_CONTROL_ENHANCED_COLOR_MODE_ENHANCED_CURRENT_HUE_AND_CURRENT_SATURATION) {
    dotdot_set_color_control_enhanced_color_mode(unid,
                                                 endpoint,
                                                 DESIRED_ATTRIBUTE,
                                                 color_mode);
    dotdot_set_color_control_enhanced_color_mode(unid,
                                                 endpoint,
                                                 REPORTED_ATTRIBUTE,
                                                 color_mode);
  }
  if (
    color_mode
    == ZCL_COLOR_CONTROL_ENHANCED_COLOR_MODE_ENHANCED_CURRENT_HUE_AND_CURRENT_SATURATION)
    color_mode = ZCL_COLOR_CONTROL_COLOR_MODE_CURRENT_HUE_AND_CURRENT_SATURATION;
  if (color_mode <= ZCL_COLOR_CONTROL_COLOR_MODE_COLOR_TEMPERATURE_MIREDS) {
    dotdot_set_color_control_color_mode(unid,
                                        endpoint,
                                        DESIRED_ATTRIBUTE,
                                        color_mode);
    dotdot_set_color_control_color_mode(unid,
                                        endpoint,
                                        REPORTED_ATTRIBUTE,
                                        color_mode);
  }
}

static void dotdot_set_color_control_temperature_mireds_remaining_time(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uint16_t color_temperature_mireds,
  float rate,
  uint16_t remaining_time)
{
  attribute_store_node_t endpoint_node
    = unify_dotdot_attributes_get_endpoint_node()(unid, endpoint);
  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(LOG_TAG,
                 "Updating ZCL desired values after "
                 "ColorControl:TemperatureMireds command");
    attribute_store_node_t node = attribute_store_get_first_child_by_type(
      endpoint_node,
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_TEMPERATURE_MIREDS);
    if (rate == 0) {
      attribute_stop_transition(node);
      dotdot_set_color_control_color_temperature_mireds(
        unid,
        endpoint,
        DESIRED_ATTRIBUTE,
        color_temperature_mireds);
    }
    else {
      dotdot_set_color_control_color_temperature_mireds(
        unid,
        endpoint,
        DESIRED_ATTRIBUTE,
        dotdot_get_color_control_color_temperature_mireds(unid,
                                                          endpoint,
                                                          REPORTED_ATTRIBUTE));
      attribute_start_fixed_transition(node,
                                       DESIRED_ATTRIBUTE,
                                       color_temperature_mireds,
                                       rate,
                                       1000);
    }
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
    attribute_store_node_t node = attribute_store_get_first_child_by_type(
      endpoint_node,
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_REMAINING_TIME);
    dotdot_set_color_control_remaining_time(unid,
                                            endpoint,
                                            DESIRED_ATTRIBUTE,
                                            remaining_time);
    if (remaining_time > 0) {
      attribute_start_fixed_transition(node, DESIRED_ATTRIBUTE, 0, -10, 1000);
    }
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
  bool hue_valid,
  float hue_rate,
  uint8_t saturation,
  float sat_rate,
  uint16_t remaining_time)
{
  attribute_store_node_t endpoint_node = unify_dotdot_attributes_get_endpoint_node()(unid, endpoint);

  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(
      LOG_TAG,
      "Updating ZCL desired values after ColorControl:CurrentHue command");
    sl_log_info(
      LOG_TAG,
      "DEBUG!! CurrentHue command: Rate: %d", hue_rate);
    attribute_store_node_t node
      = attribute_store_get_first_child_by_type(
        endpoint_node,
        DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_HUE);
    if (hue_rate == 0) {
      // stop any ongoing transition to ensure ongoing transition don't override
      attribute_stop_transition(node);
      dotdot_set_color_control_current_hue(unid,
                                          endpoint,
                                          DESIRED_ATTRIBUTE,
                                          hue);
    } else if (hue_valid) {
      uint8_t current_hue
        = dotdot_get_color_control_current_hue(unid,
                                               endpoint,
                                               DESIRED_ATTRIBUTE);
      dotdot_set_color_control_current_hue(unid,
                                           endpoint,
                                           DESIRED_ATTRIBUTE,
                                           current_hue);
      attribute_start_fixed_cyclic_transition(node,
                                              DESIRED_ATTRIBUTE,
                                              hue,
                                              hue_rate,
                                              0,
                                              0xFE,
                                              1000);
    }
    else {
      attribute_store_set_desired_as_reported(node);
      attribute_start_cyclic_transition(node,
                                              DESIRED_ATTRIBUTE,
                                              hue_rate,
                                              0,
                                              0xFE,
                                              1000);
    }
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG, "Clearing ColorControl:CurrentHue reported value");
      dotdot_color_control_current_hue_undefine_reported(unid, endpoint);
    }
  }
  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(
      LOG_TAG,
      "Updating ZCL desired values after ColorControl:RemainingTime command");
    attribute_store_node_t node
      = attribute_store_get_first_child_by_type(
        endpoint_node,
        DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_REMAINING_TIME);
    dotdot_set_color_control_remaining_time(unid,
                                            endpoint,
                                            DESIRED_ATTRIBUTE,
                                            remaining_time);
    // decrease by 1 every second until 0 i.e. until command transition complete
    attribute_start_fixed_transition(node, DESIRED_ATTRIBUTE, 0, -10, 1000);
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
    attribute_store_node_t node
      = attribute_store_get_first_child_by_type(
        endpoint_node,
        DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_SATURATION);
    if (sat_rate == 0) {
      // stop any ongoing transition to ensure ongoing transition don't override
      attribute_stop_transition(node);
      dotdot_set_color_control_current_saturation(unid,
                                          endpoint,
                                          DESIRED_ATTRIBUTE,
                                          saturation);
    } else {
      uint8_t current_saturation
        = dotdot_get_color_control_current_saturation(unid,
                                               endpoint,
                                               DESIRED_ATTRIBUTE);
      dotdot_set_color_control_current_saturation(unid,
                                           endpoint,
                                           DESIRED_ATTRIBUTE,
                                           current_saturation);
      attribute_start_fixed_transition(node,
                                       DESIRED_ATTRIBUTE,
                                       saturation,
                                       sat_rate,
                                       1000);
    }
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG,
                   "Clearing ColorControl:RemainingTime reported value");
      dotdot_color_control_current_saturation_undefine_reported(unid, endpoint);
    }
  }
}

static void dotdot_set_color_control_enhanced_hue_saturation_remaining_time(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uint16_t enhanced_hue,
  bool hue_valid,
  float hue_rate,
  uint8_t saturation,
  float sat_rate,
  uint16_t remaining_time)
{
  attribute_store_node_t endpoint_node = unify_dotdot_attributes_get_endpoint_node()(unid, endpoint);

  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(
      LOG_TAG,
      "Updating ZCL desired values after ColorControl:EnhancedCurrentHue command");
    attribute_store_node_t node
      = attribute_store_get_first_child_by_type(
        endpoint_node,
        DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_ENHANCED_CURRENT_HUE);
    if (hue_rate == 0) {
      // stop any ongoing transition to ensure ongoing transition don't override
      attribute_stop_transition(node);
      dotdot_set_color_control_enhanced_current_hue(unid,
                                          endpoint,
                                          DESIRED_ATTRIBUTE,
                                          enhanced_hue);
    } else if (hue_valid) {
      uint16_t current_enhanced_hue
        = dotdot_get_color_control_enhanced_current_hue(unid,
                                               endpoint,
                                               DESIRED_ATTRIBUTE);
      dotdot_set_color_control_enhanced_current_hue(unid,
                                           endpoint,
                                           DESIRED_ATTRIBUTE,
                                           current_enhanced_hue);
      attribute_start_fixed_cyclic_transition(node,
                                              DESIRED_ATTRIBUTE,
                                              enhanced_hue,
                                              hue_rate,
                                              0,
                                              DEFAULT_MAX_ENHANCED_COLOR,
                                              1000);
    }
    else {
      attribute_store_set_desired_as_reported(node);
      attribute_start_cyclic_transition(node,
                                        DESIRED_ATTRIBUTE,
                                        hue_rate,
                                        0,
                                        DEFAULT_MAX_ENHANCED_COLOR,
                                        1000);
    }
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG, "Clearing ColorControl:CurrentHue reported value");
      dotdot_color_control_enhanced_current_hue_undefine_reported(unid, endpoint);
    }
  }
  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(
      LOG_TAG,
      "Updating ZCL desired values after ColorControl:RemainingTime command");
    attribute_store_node_t node
      = attribute_store_get_first_child_by_type(
        endpoint_node,
        DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_REMAINING_TIME);
    dotdot_set_color_control_remaining_time(unid,
                                            endpoint,
                                            DESIRED_ATTRIBUTE,
                                            remaining_time);
    // decrease by 1 every second until 0 i.e. until command transition complete
    attribute_start_fixed_transition(node, DESIRED_ATTRIBUTE, 0, -10, 1000);
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
    attribute_store_node_t node
      = attribute_store_get_first_child_by_type(
        endpoint_node,
        DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_SATURATION);
    if (sat_rate == 0) {
      // stop any ongoing transition to ensure ongoing transition don't override
      attribute_stop_transition(node);
      dotdot_set_color_control_current_saturation(unid,
                                          endpoint,
                                          DESIRED_ATTRIBUTE,
                                          saturation);
    } else {
      uint8_t current_saturation
        = dotdot_get_color_control_current_saturation(unid,
                                               endpoint,
                                               DESIRED_ATTRIBUTE);
      dotdot_set_color_control_current_saturation(unid,
                                           endpoint,
                                           DESIRED_ATTRIBUTE,
                                           current_saturation);
      attribute_start_fixed_transition(node,
                                       DESIRED_ATTRIBUTE,
                                       saturation,
                                       sat_rate,
                                       1000);
    }
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG,
                   "Clearing ColorControl:RemainingTime reported value");
      dotdot_color_control_current_saturation_undefine_reported(unid, endpoint);
    }
  }
}

static void dotdot_set_color_control_colorx_colory_remaining_time(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uint16_t color_x,
  float rate_x,
  uint16_t color_y,
  float rate_y,
  uint16_t remaining_time)
{
  attribute_store_node_t endpoint_node = unify_dotdot_attributes_get_endpoint_node()(unid, endpoint);

  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(
      LOG_TAG,
      "Updating ZCL desired values after ColorControl:CurrentX command");
    attribute_store_node_t node
      = attribute_store_get_first_child_by_type(
        endpoint_node,
        DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENTX);
    if (rate_x == 0) {
      // stop any ongoing transition to ensure ongoing transition don't override
      attribute_stop_transition(node);
      dotdot_set_color_control_currentx(unid,
                                          endpoint,
                                          DESIRED_ATTRIBUTE,
                                          color_x);
    } else {
      uint16_t current_x
        = dotdot_get_color_control_currentx(unid,
                                               endpoint,
                                               DESIRED_ATTRIBUTE);
      dotdot_set_color_control_currentx(unid,
                                           endpoint,
                                           DESIRED_ATTRIBUTE,
                                           current_x);
      attribute_start_fixed_transition(node,
                                              DESIRED_ATTRIBUTE,
                                              color_x,
                                              rate_x,
                                              1000);
    }
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG, "Clearing ColorControl:CurrentX reported value");
      dotdot_color_control_currentx_undefine_reported(unid, endpoint);
    }
  }
  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(
      LOG_TAG,
      "Updating ZCL desired values after ColorControl:RemainingTime command");
    attribute_store_node_t node
      = attribute_store_get_first_child_by_type(
        endpoint_node,
        DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_REMAINING_TIME);
    dotdot_set_color_control_remaining_time(unid,
                                            endpoint,
                                            DESIRED_ATTRIBUTE,
                                            remaining_time);
    // decrease by 1 every second until 0 i.e. until command transition complete
    attribute_start_fixed_transition(node, DESIRED_ATTRIBUTE, 0, -10, 1000);
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG,
                   "Clearing ColorControl:RemainingTime reported value");
      dotdot_color_control_remaining_time_undefine_reported(unid, endpoint);
    }
  }

  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(
      LOG_TAG,
      "Updating ZCL desired values after ColorControl:CurrentY command");
    attribute_store_node_t node
      = attribute_store_get_first_child_by_type(
        endpoint_node,
        DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENTY);
    if (rate_y == 0) {
      // stop any ongoing transition to ensure ongoing transition don't override
      attribute_stop_transition(node);
      dotdot_set_color_control_currenty(unid,
                                          endpoint,
                                          DESIRED_ATTRIBUTE,
                                          color_y);
    } else {
      uint16_t current_y
        = dotdot_get_color_control_currenty(unid,
                                            endpoint,
                                            DESIRED_ATTRIBUTE);
      dotdot_set_color_control_currenty(unid,
                                           endpoint,
                                           DESIRED_ATTRIBUTE,
                                           current_y);
      attribute_start_fixed_transition(node,
                                       DESIRED_ATTRIBUTE,
                                       color_y,
                                       rate_y,
                                       1000);
    }
    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG,
                   "Clearing ColorControl:CurrentY reported value");
      dotdot_color_control_currenty_undefine_reported(unid, endpoint);
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

  if (!execute_if_off_option_check(unid,
                                   endpoint,
                                   options_mask,
                                   options_override)) {
    // Command is should not continue as per ZCL specification (5.2.2.2.1.10 & 5.2.2.3.3)
    return SL_STATUS_OK;
  }

  // Warning: The CCDirection is just too ninja hard for us, we will move
  // in whatever direction does not force us to wrap around a uint8_t
  // during a transition.
  if (hue >= 0xFE) {
    hue = 0xFE;
  }
  int16_t desired_hue_change = 0;
  uint8_t current_hue
    = dotdot_get_color_control_current_hue(unid, endpoint, REPORTED_ATTRIBUTE);

  switch (direction) {
    case ZCL_CC_DIRECTION_SHORTEST_DISTANCE:
      desired_hue_change = hue - current_hue;
      break;
    case ZCL_CC_DIRECTION_LONGEST_DISTANCE:
      if (hue < current_hue) {
        desired_hue_change = 0xFF - current_hue + hue;  // go upwards if smaller
      } else if (hue > current_hue) {
        desired_hue_change = hue - current_hue - 0xFF;  // go downwards if bigger
      }
      break;
    case ZCL_CC_DIRECTION_DOWN:
      desired_hue_change = (hue < current_hue) ? (hue - current_hue)
                                               : (hue - current_hue - 0xFF);
      break;
    case ZCL_CC_DIRECTION_UP:
      desired_hue_change = (hue < current_hue) ? (0xFF - current_hue + hue)
                                               : (hue - current_hue);
      break;
    default: 
      return SL_STATUS_FAIL;
  }

  set_color_modes(unid, endpoint, COLOR_COLOR_MODE);

  dotdot_set_color_control_hue_saturation_remaining_time(
    unid,
    endpoint,
    hue,
    true,
    ((transition_time != 0) ? (desired_hue_change / (transition_time / 10.0)) : 0),
    dotdot_get_color_control_current_saturation(unid,
                                                endpoint,
                                                REPORTED_ATTRIBUTE),
    0,
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

  if (!execute_if_off_option_check(unid,
                                   endpoint,
                                   options_mask,
                                   options_override)) {
    // Command is should not continue as per ZCL specification (5.2.2.2.1.10 & 5.2.2.3.3)
    return SL_STATUS_OK;
  }

  // Received a Hue/Saturation command, change to Hue/Saturation color mode
  set_color_modes(unid, endpoint, COLOR_COLOR_MODE);

  if (rate == 0x00 && move_mode != ZCL_CC_MOVE_MODE_STOP)
    return SL_STATUS_FAIL;

  float hue_rate;
  switch (move_mode) {
    case ZCL_CC_MOVE_MODE_STOP:
      hue_rate = 0;
      break;
    case ZCL_CC_MOVE_MODE_UP:
      hue_rate = rate;
      break;
    case ZCL_CC_MOVE_MODE_DOWN:
      hue_rate = -rate;
      break;
    default:  // move_mode = 0x02 - Reserved
      return SL_STATUS_FAIL;
  }

  dotdot_set_color_control_hue_saturation_remaining_time(
    unid,
    endpoint,
    dotdot_get_color_control_current_hue(unid, endpoint, REPORTED_ATTRIBUTE),
    (hue_rate == 0) ? true : false,
    hue_rate,
    dotdot_get_color_control_current_saturation(unid,
                                                endpoint,
                                                REPORTED_ATTRIBUTE),
    0,
    0);
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

  if (!execute_if_off_option_check(unid,
                                   endpoint,
                                   options_mask,
                                   options_override)) {
    // Command is should not continue as per ZCL specification (5.2.2.2.1.10 & 5.2.2.3.3)
    return SL_STATUS_OK;
  }

  // Received a Hue/Saturation command, change to Hue/Saturation color mode
  set_color_modes(unid, endpoint, COLOR_COLOR_MODE);

  // Step size 0 will not get us moving.
  if (step_size == 0) {
    return SL_STATUS_OK;
  }

  uint8_t desired_hue;
  float change;
  uint8_t current_hue
    = dotdot_get_color_control_current_hue(unid, endpoint, REPORTED_ATTRIBUTE);

  if (step_mode == ZCL_CC_STEP_MODE_UP) {
    desired_hue = (current_hue + step_size) % 0xFF; //wrap around
    change = step_size;
  } else if (step_mode == ZCL_CC_STEP_MODE_DOWN) {
    desired_hue = (current_hue < step_size) ? 0xFF - step_size + current_hue
                                            : current_hue - step_size;
    change = -step_size;
  } else
    return SL_STATUS_FAIL;  // step_mode = 0x00, 0x02 - Reserved

  desired_hue = validated_color_value(desired_hue); 

  dotdot_set_color_control_hue_saturation_remaining_time(
    unid,
    endpoint,
    desired_hue,
    true,
    ((transition_time != 0) ? (change / (transition_time / 10.0)) : 0),
    dotdot_get_color_control_current_saturation(unid,
                                                endpoint,
                                                REPORTED_ATTRIBUTE),
    0,
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

  if (!execute_if_off_option_check(unid,
                                   endpoint,
                                   options_mask,
                                   options_override)) {
    // Command is should not continue as per ZCL specification (5.2.2.2.1.10 & 5.2.2.3.3)
    return SL_STATUS_OK;
  }

  // Received a Hue/Saturation command, change to Hue/Saturation color mode
  set_color_modes(unid, endpoint, COLOR_COLOR_MODE);

  if (hue >= 0xFE) {
    hue = 0xFE;
  }
  if (saturation >= 0xFE) {
    saturation = 0xFE;
  }
  uint8_t current_hue
    = dotdot_get_color_control_current_hue(unid, endpoint, REPORTED_ATTRIBUTE);
  uint8_t current_sat
    = dotdot_get_color_control_current_saturation(unid,
                                                  endpoint,
                                                  REPORTED_ATTRIBUTE);
  dotdot_set_color_control_hue_saturation_remaining_time(
    unid,
    endpoint,
    hue,
    true,
    ((transition_time != 0) ? ((hue - current_hue) / (transition_time / 10.0))
                            : 0),
    saturation,
    ((transition_time != 0)
       ? ((saturation - current_sat) / (transition_time / 10.0))
       : 0),
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

  if (!execute_if_off_option_check(unid,
                                   endpoint,
                                   options_mask,
                                   options_override)) {
    // Command is should not continue as per ZCL specification (5.2.2.2.1.10 & 5.2.2.3.3)
    return SL_STATUS_OK;
  }

  // Received a Hue/Saturation command, change to Hue/Saturation color mode
  set_color_modes(unid, endpoint, COLOR_COLOR_MODE);

  if (saturation >= 0xFE) {
    saturation = 0xFE;
  }
  uint8_t current_sat
    = dotdot_get_color_control_current_saturation(unid,
                                                  endpoint,
                                                  REPORTED_ATTRIBUTE);
  dotdot_set_color_control_hue_saturation_remaining_time(
    unid,
    endpoint,
    dotdot_get_color_control_current_hue(unid, endpoint, REPORTED_ATTRIBUTE),
    true,
    0,
    saturation,
    ((transition_time != 0) ? ((saturation - current_sat) / (transition_time / 10.0)) : 0),
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

  if (!execute_if_off_option_check(unid,
                                   endpoint,
                                   options_mask,
                                   options_override)) {
    // Command is should not continue as per ZCL specification (5.2.2.2.1.10 & 5.2.2.3.3)
    return SL_STATUS_OK;
  }

  // Received a Hue/Saturation command, change to Hue/Saturation color mode
  set_color_modes(unid, endpoint, COLOR_COLOR_MODE);

  if (rate == 0x00 && move_mode != ZCL_CC_MOVE_MODE_STOP) {
    return SL_STATUS_FAIL;
  }
  float sat_rate;

  uint8_t desired_saturation;
  uint8_t current_saturation
    = dotdot_get_color_control_current_saturation(unid,
                                                  endpoint,
                                                  REPORTED_ATTRIBUTE);
  switch (move_mode) {
    case ZCL_CC_MOVE_MODE_STOP:
      desired_saturation = current_saturation;
      sat_rate = 0;
      break;
    case ZCL_CC_MOVE_MODE_UP:
      desired_saturation = DEFAULT_MAX_COLOR;
      sat_rate = rate;
      break;
    case ZCL_CC_MOVE_MODE_DOWN:
      desired_saturation = DEFAULT_MIN_COLOR;
      sat_rate = -rate;
      break;
    default:  // move_mode = 0x02 - Reserved
      return SL_STATUS_FAIL;
  }

  uint16_t remaining_time = 0;
  if (rate != 0) {
    remaining_time 
      = (uint16_t)abs(current_saturation - desired_saturation) / rate;
  }

  dotdot_set_color_control_hue_saturation_remaining_time(
    unid,
    endpoint,
    dotdot_get_color_control_current_hue(unid, endpoint, REPORTED_ATTRIBUTE),
    true,
    0,
    desired_saturation,
    sat_rate,
    remaining_time * 10 /* in 1/10ths of second */);
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

  if (!execute_if_off_option_check(unid,
                                   endpoint,
                                   options_mask,
                                   options_override)) {
    // Command is should not continue as per ZCL specification (5.2.2.2.1.10 & 5.2.2.3.3)
    return SL_STATUS_OK;
  }

  // Received a Hue/Saturation command, change to Hue/Saturation color mode
  set_color_modes(unid, endpoint, COLOR_COLOR_MODE);

  // Step size 0 will not get us moving.
  if (step_size == 0) {
    return SL_STATUS_OK;
  }

  uint8_t desired_saturation;
  float change;
  uint8_t current_saturation
    = dotdot_get_color_control_current_saturation(unid,
                                                  endpoint,
                                                  REPORTED_ATTRIBUTE);

  if (step_mode == ZCL_CC_STEP_MODE_UP) {
    desired_saturation = ((current_saturation + step_size) > 0xFE)
                           ? 0xFE
                           : current_saturation + step_size;
    change = step_size;
  } else if (step_mode == ZCL_CC_STEP_MODE_DOWN) {
    desired_saturation = ((current_saturation - step_size) < 0)
                           ? 0
                           : current_saturation - step_size;
    change = -step_size;
  } else {  // step_mode = 0x00, 0x02 - Reserved
    return SL_STATUS_FAIL;
  }

  desired_saturation = validated_color_value(desired_saturation);

  dotdot_set_color_control_hue_saturation_remaining_time(
    unid,
    endpoint,
    dotdot_get_color_control_current_hue(unid, endpoint, REPORTED_ATTRIBUTE),
    true,
    0,
    desired_saturation,
    ((transition_time != 0) ? (change / (transition_time / 10.0)) : 0),
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

  if (!execute_if_off_option_check(unid,
                                   endpoint,
                                   options_mask,
                                   options_override)) {
    return SL_STATUS_OK;
  }

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
  float rate = 0.0;
  if (transition_time) {
    rate = (color_temperature_mireds
            - dotdot_get_color_control_color_temperature_mireds(
              unid,
              endpoint,
              REPORTED_ATTRIBUTE))
           / (transition_time / 10.0);
  }

  dotdot_set_color_control_temperature_mireds_remaining_time(
    unid,
    endpoint,
    color_temperature_mireds,
    rate,
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

  if (!execute_if_off_option_check(unid,
                                   endpoint,
                                   options_mask,
                                   options_override)) {
    return SL_STATUS_OK;
  }

  // Received a Temperature command, change to Temperature color mode
  set_color_modes(unid, endpoint, TEMPERATURE_COLOR_MODE);

  if (rate == 0x00 && move_mode != ZCL_CC_MOVE_MODE_STOP) {
    return SL_STATUS_OK;
  }

  uint16_t reported_temperature
    = dotdot_get_color_control_color_temperature_mireds(unid,
                                                        endpoint,
                                                        REPORTED_ATTRIBUTE);
  uint16_t desired_temperature = 0;
  float change_rate = 0.0;
  switch (move_mode) {
    case ZCL_CC_MOVE_MODE_STOP:
      desired_temperature = reported_temperature;
      change_rate = 0.0;
      break;
    case ZCL_CC_MOVE_MODE_UP:
      desired_temperature = color_temperature_maximum_mireds;
      change_rate = 1.0;
      break;
    case ZCL_CC_MOVE_MODE_DOWN:
      desired_temperature = color_temperature_minimum_mireds;
      change_rate = -1.0;
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

  uint16_t remaining_time = 0;
  if (rate != 0) {
    remaining_time
      = (uint16_t)abs(reported_temperature - desired_temperature) / rate;
  }
  change_rate *= rate;

  dotdot_set_color_control_temperature_mireds_remaining_time(
    unid,
    endpoint,
    desired_temperature,
    change_rate,
    remaining_time * 10);
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

  if (!execute_if_off_option_check(unid,
                                   endpoint,
                                   options_mask,
                                   options_override)) {
    return SL_STATUS_OK;
  }

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
  float rate = 0.0;
  if (step_mode == ZCL_CC_STEP_MODE_UP) {
    desired_temperature
      = ((reported_temperature + step_size) > color_temperature_maximum_mireds)
          ? color_temperature_maximum_mireds
          : reported_temperature + step_size;
    rate = 1.0;
  } else if (step_mode == ZCL_CC_STEP_MODE_DOWN) {
    desired_temperature
      = ((reported_temperature - step_size) < color_temperature_minimum_mireds)
          ? color_temperature_minimum_mireds
          : reported_temperature - step_size;
    rate = -1.0;
  } else {  // step_mode = 0x00, 0x02 - Reserved
    return SL_STATUS_FAIL;
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

  if (transition_time) {
    rate *= (step_size / (transition_time / 10));
  }
  else {
    rate = 0.0;
  }

  dotdot_set_color_control_temperature_mireds_remaining_time(
    unid,
    endpoint,
    desired_temperature,
    rate,
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
                       (TEMPERATURE_BITMASK | HUE_SATURATION_BITMASK
                        | COLOR_XY_BITMASK | ENHANCED_HUE_SATURATION_BITMASK))

  if (!execute_if_off_option_check(unid,
                                   endpoint,
                                   options_mask,
                                   options_override)) {
    return SL_STATUS_OK;
  }

  dotdot_set_color_control_colorx_colory_remaining_time(
    unid,
    endpoint,
    dotdot_get_color_control_currentx(unid, endpoint, REPORTED_ATTRIBUTE),
    0,
    dotdot_get_color_control_currenty(unid, endpoint, REPORTED_ATTRIBUTE),
    0,
    0);

  dotdot_set_color_control_temperature_mireds_remaining_time(
    unid,
    endpoint,
    dotdot_get_color_control_color_temperature_mireds(unid,
                                                      endpoint,
                                                      REPORTED_ATTRIBUTE),
    0,
    0);

  dotdot_set_color_control_enhanced_hue_saturation_remaining_time(
    unid,
    endpoint,
    dotdot_get_color_control_enhanced_current_hue(unid,
                                                  endpoint,
                                                  REPORTED_ATTRIBUTE),
    true,
    0,
    dotdot_get_color_control_current_saturation(unid,
                                                endpoint,
                                                REPORTED_ATTRIBUTE),
    0,
    0);

  // Stop any transition, remaining time is 0.
  dotdot_set_color_control_hue_saturation_remaining_time(
    unid,
    endpoint,
    dotdot_get_color_control_current_hue(unid, endpoint, REPORTED_ATTRIBUTE),
    true,
    0,
    dotdot_get_color_control_current_saturation(unid,
                                                endpoint,
                                                REPORTED_ATTRIBUTE),
    0,
    0);

  return SL_STATUS_OK;
}

sl_status_t color_control_move_to_color_callback(
    dotdot_unid_t unid,
    dotdot_endpoint_id_t endpoint,
    uic_mqtt_dotdot_callback_call_type_t call_type,
    uint16_t colorx,
    uint16_t colory,
    uint16_t transition_time,
    uint8_t options_mask,
    uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, COLOR_XY_BITMASK)

  if (!execute_if_off_option_check(unid,
                                   endpoint,
                                   options_mask,
                                   options_override)) {
    return SL_STATUS_OK;
  }
  
  set_color_modes(unid,
                 endpoint,
                 ZCL_COLOR_CONTROL_COLOR_MODE_CURRENT_X_AND_CURRENT_Y);

  uint16_t current_x
    = dotdot_get_color_control_currentx(unid, endpoint, REPORTED_ATTRIBUTE);
  uint16_t current_y
    = dotdot_get_color_control_currenty(unid, endpoint, REPORTED_ATTRIBUTE);
  
  float rate_x = 0.0;
  float rate_y = 0.0;
  if (transition_time) {
    rate_x = (current_x - colorx) / (transition_time / 10);
    rate_y = (current_y - colory) / (transition_time / 10);
  }

  dotdot_set_color_control_colorx_colory_remaining_time(
    unid,
    endpoint,
    colorx,
    rate_x,
    colory,
    rate_y,
    transition_time);

  return SL_STATUS_OK;
}

sl_status_t color_control_move_color_callback(
    dotdot_unid_t unid,
    dotdot_endpoint_id_t endpoint,
    uic_mqtt_dotdot_callback_call_type_t call_type,
    int16_t ratex,
    int16_t ratey,
    uint8_t options_mask,
    uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, COLOR_XY_BITMASK)

  if (!execute_if_off_option_check(unid,
                                   endpoint,
                                   options_mask,
                                   options_override)) {
    return SL_STATUS_OK;
  }
  
  set_color_modes(unid,
                 endpoint,
                 ZCL_COLOR_CONTROL_COLOR_MODE_CURRENT_X_AND_CURRENT_Y);
  
  float rate_x = (float)ratex;
  float rate_y = (float)ratey;
  uint16_t current_x
    = dotdot_get_color_control_currentx(unid, endpoint, REPORTED_ATTRIBUTE);
  uint16_t current_y
    = dotdot_get_color_control_currenty(unid, endpoint, REPORTED_ATTRIBUTE);
  uint16_t desired_x = 0;
  uint16_t desired_y = 0;
  if (ratex < 0)
  {
    desired_x = DEFAULT_MIN_X_Y;
  }
  else if (ratex > 0)
  {
    desired_x = DEFAULT_MAX_X_Y;
  }
  if (ratey < 0)
  {
    desired_y = DEFAULT_MIN_X_Y;
  }
  else if (ratey > 0)
  {
    desired_y = DEFAULT_MAX_X_Y;
  }
  
  uint16_t time_x = 0;
  uint16_t time_y = 0;
  if (ratex) {
    time_x = (desired_x - current_x) / ratex;
  }
  if (ratey) {
    time_y = (desired_y - current_y) / ratey;
  }

  dotdot_set_color_control_colorx_colory_remaining_time(
    unid,
    endpoint,
    desired_x,
    rate_x,
    desired_y,
    rate_y,
    (time_x > time_y) ? time_x : time_y);

  return SL_STATUS_OK;
}

sl_status_t color_control_step_color_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  int16_t stepx,
  int16_t stepy,
  uint16_t transition_time,
  uint8_t options_mask,
  uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, COLOR_XY_BITMASK)

  if (!execute_if_off_option_check(unid,
                                   endpoint,
                                   options_mask,
                                   options_override)) {
    return SL_STATUS_OK;
  }

  set_color_modes(unid,
                 endpoint,
                 ZCL_COLOR_CONTROL_COLOR_MODE_CURRENT_X_AND_CURRENT_Y);

  uint16_t current_x
    = dotdot_get_color_control_currentx(unid, endpoint, REPORTED_ATTRIBUTE);
  uint16_t current_y
    = dotdot_get_color_control_currenty(unid, endpoint, REPORTED_ATTRIBUTE);

  uint16_t desired_x = 0;
  uint16_t desired_y = 0;
  if (current_x + stepx < DEFAULT_MIN_X_Y)
    desired_x = DEFAULT_MIN_X_Y;
  else if (current_x + stepx > DEFAULT_MAX_X_Y)
    desired_x = DEFAULT_MAX_X_Y;
  else
    desired_x = current_x + stepx;
  if (current_y + stepy < DEFAULT_MIN_X_Y)
    desired_y = DEFAULT_MIN_X_Y;
  else if (current_y + stepy > DEFAULT_MAX_X_Y)
    desired_y = DEFAULT_MAX_X_Y;
  else
    desired_y = current_y + stepy;

  float rate_x = 0.0;
  float rate_y = 0.0;
  if (transition_time) {
    rate_x = stepx / (transition_time / 10);
    rate_y = stepy / (transition_time / 10);
  }

  dotdot_set_color_control_colorx_colory_remaining_time(
    unid,
    endpoint,
    desired_x,
    rate_x,
    desired_y,
    rate_y,
    transition_time);

  return SL_STATUS_OK;
}

sl_status_t color_control_enhanced_move_to_hue_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint16_t enhanced_hue,
  CCDirection direction,
  uint16_t transition_time,
  uint8_t options_mask,
  uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid,
                       endpoint,
                       call_type,
                       ENHANCED_HUE_SATURATION_BITMASK)

  if (!execute_if_off_option_check(unid,
                                   endpoint,
                                   options_mask,
                                   options_override)) {
    // Command is should not continue as per ZCL specification (5.2.2.2.1.10 & 5.2.2.3.3)
    return SL_STATUS_OK;
  }

  int32_t desired_hue_change = 0;
  uint16_t current_hue
    = dotdot_get_color_control_enhanced_current_hue(unid,
                                                    endpoint,
                                                    REPORTED_ATTRIBUTE);

  switch (direction) {
    case ZCL_CC_DIRECTION_SHORTEST_DISTANCE:
      desired_hue_change = enhanced_hue - current_hue;
      break;
    case ZCL_CC_DIRECTION_LONGEST_DISTANCE:
      if (enhanced_hue < current_hue) {
        desired_hue_change
          = DEFAULT_MAX_ENHANCED_COLOR - current_hue + enhanced_hue;  // go upwards if smaller
      } else if (enhanced_hue > current_hue) {
        desired_hue_change
          = enhanced_hue - current_hue
            - DEFAULT_MAX_ENHANCED_COLOR;  // go downwards if bigger
      }
      break;
    case ZCL_CC_DIRECTION_DOWN:
      desired_hue_change
        = (enhanced_hue < current_hue)
            ? (enhanced_hue - current_hue)
            : (enhanced_hue - current_hue - DEFAULT_MAX_ENHANCED_COLOR);
      break;
    case ZCL_CC_DIRECTION_UP:
      desired_hue_change
        = (enhanced_hue < current_hue)
            ? (DEFAULT_MAX_ENHANCED_COLOR - current_hue + enhanced_hue)
            : (enhanced_hue - current_hue);
      break;
    default:
      return SL_STATUS_FAIL;
  }

  set_color_modes(
    unid,
    endpoint,
    ZCL_COLOR_CONTROL_ENHANCED_COLOR_MODE_ENHANCED_CURRENT_HUE_AND_CURRENT_SATURATION);

  dotdot_set_color_control_enhanced_hue_saturation_remaining_time(
    unid,
    endpoint,
    enhanced_hue,
    true,
    ((transition_time != 0) ? (desired_hue_change / (transition_time / 10.0))
                            : 0),
    dotdot_get_color_control_current_saturation(unid,
                                                endpoint,
                                                REPORTED_ATTRIBUTE),
    0,
    transition_time);
  return SL_STATUS_OK;
}

sl_status_t color_control_enhanced_move_hue_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  CCMoveMode move_mode,
  uint16_t rate,
  uint8_t options_mask,
  uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, ENHANCED_HUE_SATURATION_BITMASK)

  if (!execute_if_off_option_check(unid,
                                   endpoint,
                                   options_mask,
                                   options_override)) {
    // Command is should not continue as per ZCL specification (5.2.2.2.1.10 & 5.2.2.3.3)
    return SL_STATUS_OK;
  }

  if (rate == 0x00 && move_mode != ZCL_CC_MOVE_MODE_STOP)
    return SL_STATUS_FAIL;

  uint16_t desired_hue = 0;
  float hue_rate;
  uint16_t current_hue
    = dotdot_get_color_control_enhanced_current_hue(unid, endpoint, REPORTED_ATTRIBUTE);
  switch (move_mode) {
    case ZCL_CC_MOVE_MODE_STOP:
      desired_hue = current_hue;
      hue_rate = 0;
      break;
    case ZCL_CC_MOVE_MODE_UP:
      hue_rate = rate;
      break;
    case ZCL_CC_MOVE_MODE_DOWN:
      hue_rate = -rate;
      break;
    default:  // move_mode = 0x02 - Reserved
      return SL_STATUS_FAIL;
  }

  // Received a Enhanced Hue command, change to Enhanced Hue color mode
  set_color_modes(
    unid,
    endpoint,
    ZCL_COLOR_CONTROL_ENHANCED_COLOR_MODE_ENHANCED_CURRENT_HUE_AND_CURRENT_SATURATION);

  dotdot_set_color_control_enhanced_hue_saturation_remaining_time(
    unid,
    endpoint,
    desired_hue,
    (hue_rate == 0) ? true : false,
    hue_rate,
    dotdot_get_color_control_current_saturation(unid,
                                                endpoint,
                                                REPORTED_ATTRIBUTE),
    0,
    0);
  return SL_STATUS_OK;
}

sl_status_t color_control_enhanced_step_hue_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  CCStepMode step_mode,
  uint16_t step_size,
  uint16_t transition_time,
  uint8_t options_mask,
  uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, ENHANCED_HUE_SATURATION_BITMASK)

  if (!execute_if_off_option_check(unid,
                                   endpoint,
                                   options_mask,
                                   options_override)) {
    // Command is should not continue as per ZCL specification (5.2.2.2.1.10 & 5.2.2.3.3)
    return SL_STATUS_OK;
  }

  // Step size 0 will not get us moving.
  if (step_size == 0) {
    return SL_STATUS_OK;
  }

  uint16_t desired_hue;
  float change;
  uint16_t current_hue
    = dotdot_get_color_control_enhanced_current_hue(unid, endpoint, REPORTED_ATTRIBUTE);

  if (step_mode == ZCL_CC_STEP_MODE_UP) {
    desired_hue = current_hue + step_size;
    change = step_size;
  } else if (step_mode == ZCL_CC_STEP_MODE_DOWN) {
    desired_hue = current_hue - step_size;
    change = -step_size;
  } else
    return SL_STATUS_FAIL;  // step_mode = 0x00, 0x02 - Reserved
  
  // Received a Enhanced Hue command, change to Enhanced Hue color mode
  set_color_modes(
    unid,
    endpoint,
    ZCL_COLOR_CONTROL_ENHANCED_COLOR_MODE_ENHANCED_CURRENT_HUE_AND_CURRENT_SATURATION);

  dotdot_set_color_control_enhanced_hue_saturation_remaining_time(
    unid,
    endpoint,
    desired_hue,
    true,
    ((transition_time != 0) ? (change / (transition_time / 10.0)) : 0),
    dotdot_get_color_control_current_saturation(unid,
                                                endpoint,
                                                REPORTED_ATTRIBUTE),
    0,
    transition_time);
  return SL_STATUS_OK;
}

sl_status_t color_control_enhanced_move_to_hue_and_saturation_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint16_t hue,
  uint8_t saturation,
  uint16_t transition_time,
  uint8_t options_mask,
  uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, ENHANCED_HUE_SATURATION_BITMASK)

  if (!execute_if_off_option_check(unid,
                                   endpoint,
                                   options_mask,
                                   options_override)) {
    // Command is should not continue as per ZCL specification (5.2.2.2.1.10 & 5.2.2.3.3)
    return SL_STATUS_OK;
  }

  // Received a Enhanced Hue/Saturation command, change to Enhanced Hue/Saturation color mode
  set_color_modes(
    unid,
    endpoint,
    ZCL_COLOR_CONTROL_ENHANCED_COLOR_MODE_ENHANCED_CURRENT_HUE_AND_CURRENT_SATURATION);

  if (saturation >= 0xFE) {

    saturation = 0xFE;
  }
  uint8_t current_hue
    = dotdot_get_color_control_current_hue(unid, endpoint, REPORTED_ATTRIBUTE);
  uint8_t current_sat
    = dotdot_get_color_control_current_saturation(unid,
                                                  endpoint,
                                                  REPORTED_ATTRIBUTE);
  dotdot_set_color_control_enhanced_hue_saturation_remaining_time(
    unid,
    endpoint,
    hue,
    true,
    ((transition_time != 0) ? ((hue - current_hue) / (transition_time / 10.0))
                            : 0),
    saturation,
    ((transition_time != 0)
       ? ((saturation - current_sat) / (transition_time / 10.0))
       : 0),
    transition_time);
  return SL_STATUS_OK;
}

sl_status_t
  color_control_color_loop_set(dotdot_unid_t unid,
                               dotdot_endpoint_id_t endpoint,
                               uic_mqtt_dotdot_callback_call_type_t call_type,
                               uint8_t update_flags,
                               ColorLoopSetAction action,
                               CCColorLoopDirection direction,
                               uint16_t time,
                               uint16_t start_hue,
                               uint8_t options_mask,
                               uint8_t options_override)
{
  HANDLE_SUPPORT_CHECK(unid, endpoint, call_type, COLOR_LOOP_BITMASK)

  if (!execute_if_off_option_check(unid,
                                   endpoint,
                                   options_mask,
                                   options_override)) {
    // Command is should not continue as per ZCL specification (5.2.2.2.1.10 & 5.2.2.3.3)
    return SL_STATUS_OK;
  }

  sl_log_info(LOG_TAG, "Color control loop set processing in core");

  if (update_flags & COLOR_LOOP_SET_UPDATE_FLAGS_UPDATE_ACTION)
  {
    dotdot_set_color_control_color_loop_active(
      unid,
      endpoint,
      DESIRED_ATTRIBUTE,
      action > ZCL_COLOR_LOOP_SET_ACTION_DEACTIVATE_COLOR_LOOP);
  }
  if (update_flags & COLOR_LOOP_SET_UPDATE_FLAGS_UPDATE_DIRECTION)
  {
    dotdot_set_color_control_color_loop_direction(unid,
                                                  endpoint,
                                                  DESIRED_ATTRIBUTE,
                                                  direction);
  }
  if (update_flags & COLOR_LOOP_SET_UPDATE_FLAGS_UPDATE_START_HUE)
  {
    dotdot_set_color_control_color_loop_start_enhanced_hue(unid,
                                                           endpoint,
                                                           DESIRED_ATTRIBUTE,
                                                           start_hue);
  }
  if (update_flags & COLOR_LOOP_SET_UPDATE_FLAGS_UPDATE_TIME)
  {
    dotdot_set_color_control_color_loop_time(unid,
                                             endpoint,
                                             DESIRED_ATTRIBUTE,
                                             time);
  }

  // Note: The actual color loop action cannot be handled by core, 
  //  it is up to PC and end device to handle the action required
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

  // ColorXY commands
  uic_mqtt_dotdot_color_control_move_to_color_callback_set(
    &color_control_move_to_color_callback);
  uic_mqtt_dotdot_color_control_move_color_callback_set(
    &color_control_move_color_callback);
  uic_mqtt_dotdot_color_control_step_color_callback_set(
    &color_control_step_color_callback);

  // Color Control Enhanced Hue commands
  uic_mqtt_dotdot_color_control_enhanced_move_to_hue_callback_set(
    &color_control_enhanced_move_to_hue_callback);
  uic_mqtt_dotdot_color_control_enhanced_move_hue_callback_set(
    &color_control_enhanced_move_hue_callback);
  uic_mqtt_dotdot_color_control_enhanced_step_hue_callback_set(
    &color_control_enhanced_step_hue_callback);

  // Enhanced Hue and Saturation commands
  uic_mqtt_dotdot_color_control_enhanced_move_to_hue_and_saturation_callback_set(
    &color_control_enhanced_move_to_hue_and_saturation_callback);

  // Color loop command
  uic_mqtt_dotdot_color_control_color_loop_set_callback_set(
    &color_control_color_loop_set);
}
