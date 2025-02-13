/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "eed_attribute_store.h"
#include "eed_attribute_store_helper.h"

#include "sl_log.h"
#include "sl_status.h"
#include "dotdot_mqtt.h"
#include "zap-types.h"
#include "unify_dotdot_attribute_store_helpers.h"
#include "unify_dotdot_defined_attribute_types.h"
#include "attribute_transitions.h"
#include "attribute_timeouts.h"

#define LOG_TAG "eed_cmd_mapper"

#define CC_IS_OPTION_ACTIVE(option, mask_offset) ((option & (1 << mask_offset)) > 0)

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
bool cc_execute_if_off_option_check(dotdot_unid_t unid,
                                 dotdot_endpoint_id_t endpoint,
                                 uint8_t options_mask,
                                 uint8_t options_override)
{
  uint8_t temp_option
    = dotdot_get_color_control_options(unid,
                                       endpoint,
                                       DESIRED_OR_REPORTED_ATTRIBUTE);

  // If option_mask bit for ExecuteIfOff is set we take the option_override value instead of the color_control options attribute
  if (CC_IS_OPTION_ACTIVE(options_mask, CC_COLOR_OPTIONS_EXECUTE_IF_OFF_OFFSET)) {
    temp_option = options_override;
  }
  bool execute_even_if_off_option
    = CC_IS_OPTION_ACTIVE(temp_option, CC_COLOR_OPTIONS_EXECUTE_IF_OFF_OFFSET);

  // Command has no effect if OnOff::OnOff is Off and execute_even_if_off_option is not specified
  if (!execute_even_if_off_option
      && dotdot_get_on_off_on_off(unid, endpoint, DESIRED_OR_REPORTED_ATTRIBUTE)
           == false) {
    return false;
  }
  return true;
}

sl_status_t eed_color_control_color_loop_set(
  dotdot_unid_t unid,
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
  if (0                                                                   \
      == (dotdot_get_color_control_color_capabilities(unid,               \
                                                      endpoint,           \
                                                      REPORTED_ATTRIBUTE) \
          & COLOR_CONTROL_COLOR_CAPABILITIES_COLOR_LOOP_SUPPORTED)) {                                                   \
    return SL_STATUS_FAIL;                                                \
  } else if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {  \
    return SL_STATUS_OK;
  }

  if (!cc_execute_if_off_option_check(unid,
                                   endpoint,
                                   options_mask,
                                   options_override)) {
    // Command is should not continue as per ZCL specification (5.2.2.2.1.10 & 5.2.2.3.3)
    return SL_STATUS_OK;
  }

  
  if ((update_flags & COLOR_LOOP_SET_UPDATE_FLAGS_UPDATE_ACTION) == 0)
  {
    action = (ColorLoopSetAction)dotdot_get_color_control_color_loop_active(
      unid,
      endpoint,
      REPORTED_ATTRIBUTE);
    // If no loop set action to be taken then return 
    if (action == ZCL_COLOR_LOOP_SET_ACTION_DEACTIVATE_COLOR_LOOP)
      return SL_STATUS_OK;
    // if already active and ongoing then continue with current enhanced hue value
    action = ZCL_COLOR_LOOP_SET_ACTION_ACTIVATE_COLOR_LOOP_FROM_ENHANCED_CURRENT_HUE;
  }
  else if (action != ZCL_COLOR_LOOP_SET_ACTION_DEACTIVATE_COLOR_LOOP)
  {
    dotdot_set_color_control_color_loop_stored_enhanced_hue(
    unid,
    endpoint,
    REPORTED_ATTRIBUTE,
    dotdot_get_color_control_enhanced_current_hue(unid,
                                                  endpoint,
                                                  REPORTED_ATTRIBUTE));
  }
  if ((update_flags & COLOR_LOOP_SET_UPDATE_FLAGS_UPDATE_DIRECTION) == 0)
  {
    direction
      = (CCColorLoopDirection)dotdot_get_color_control_color_loop_direction(
        unid,
        endpoint,
        REPORTED_ATTRIBUTE);
  }
  if ((update_flags & COLOR_LOOP_SET_UPDATE_FLAGS_UPDATE_TIME) == 0)
  {
    time = dotdot_get_color_control_color_loop_time(unid,
                                             endpoint,
                                             REPORTED_ATTRIBUTE);
  } 
  if ((update_flags & COLOR_LOOP_SET_UPDATE_FLAGS_UPDATE_START_HUE) == 0)
  {
    start_hue = dotdot_get_color_control_color_loop_start_enhanced_hue(
      unid,
      endpoint,
      REPORTED_ATTRIBUTE);
  }

  attribute_store_node_t endpoint_node
    = eed_attribute_store_get_endpoint_node(unid, endpoint);
  attribute_store_node_t enhanced_hue_node
    = attribute_store_get_first_child_by_type(
      endpoint_node,
      DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_ENHANCED_CURRENT_HUE);

  // start/update/stop color loop based on action
  float rate = 0xFFFF / time;
  if (direction
      == ZCL_CC_COLOR_LOOP_DIRECTION_COLOR_LOOP_HUE_DECREMENT)
    rate = -rate;
  if (action != ZCL_COLOR_LOOP_SET_ACTION_DEACTIVATE_COLOR_LOOP) {
    if (
      action
      == ZCL_COLOR_LOOP_SET_ACTION_ACTIVATE_COLOR_LOOP_FROM_COLOR_LOOP_START_ENHANCED_HUE)
      dotdot_set_color_control_enhanced_current_hue(unid,
                                                    endpoint,
                                                    REPORTED_ATTRIBUTE,
                                                    start_hue);
    attribute_start_cyclic_transition(
      enhanced_hue_node,
      REPORTED_ATTRIBUTE,
      rate,
      0,
      0xFFFF,
      1000);
  }
  else if (is_attribute_transition_ongoing(enhanced_hue_node)) { 
    sl_log_info(LOG_TAG, "ColorLoop ended, reverting hue to stored value");
    uint16_t stored_hue
      = dotdot_get_color_control_color_loop_stored_enhanced_hue(
        unid,
        endpoint,
        REPORTED_ATTRIBUTE);
    attribute_stop_transition(enhanced_hue_node);
    attribute_store_set_reported(enhanced_hue_node,
                                 &stored_hue,
                                 sizeof(uint16_t));
  }

  return SL_STATUS_OK;
}

sl_status_t eed_attribute_store_command_handler_init()
{
  attribute_timeouts_init();
  // custom handling of Color Control Cluster Color Loop Set action
  uic_mqtt_dotdot_color_control_color_loop_set_callback_set(
    &eed_color_control_color_loop_set);

  return SL_STATUS_OK;
}
