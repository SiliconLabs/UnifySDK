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
#include "unify_dotdot_attribute_store_command_callbacks.h"
#include "unify_dotdot_attribute_store_command_callbacks_level.h"
#include "unify_dotdot_attribute_store_configuration.h"
#include "unify_dotdot_attribute_store_helpers.h"
#include "unify_dotdot_defined_attribute_types.h"
#include "attribute_timeouts.h"
#include "dotdot_mqtt_send_commands.h"

// Other Unify components.
#include "dotdot_mqtt.h"
#include "sl_log.h"

#define LOG_TAG "unify_dotdot_attribute_store_on_off_commands_callbacks"

#define ACCEPT_ONLY_WHEN_ON_MASK 1

////////////////////////////////////////////////////////////////////////////////
// Private helper functions
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Updates the desired value of the CurrentLevel attribute if supported
 * when we receive an On Command.
 *
 * @param unid        UNID for which the CurrentLevel desired value is to be adjusted
 * @param endpoint    Endpoint for which the CurrentLevel desired value is to be adjusted
 */
static void update_current_level_attribute_after_on_command(
  const dotdot_unid_t unid, const dotdot_endpoint_id_t endpoint)
{
  attribute_store_node_t endpoint_node
    = unify_dotdot_attributes_get_endpoint_node()(unid, endpoint);
  attribute_store_node_t current_level_node
    = attribute_store_get_first_child_by_type(
      endpoint_node,
      DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL);
  if (attribute_store_is_reported_defined(current_level_node)
      && attribute_store_get_reported_number(current_level_node) != 0) {
    return;
  }
  if (attribute_store_is_desired_defined(current_level_node)
      && attribute_store_get_desired_number(current_level_node) != 0) {
    return;
  }

  uint8_t desired_level = 0xFF;
  if (dotdot_is_supported_level_on_level(unid, endpoint)) {
    desired_level
      = dotdot_get_level_on_level(unid, endpoint, REPORTED_ATTRIBUTE);
  }
  if (desired_level == 0xFF) {
    // try the last non-zero value cache:
    attribute_store_node_t endpoint_node
      = unify_dotdot_attributes_get_endpoint_node()(unid, endpoint);
    attribute_store_node_t zcl_last_non_zero_level
      = attribute_store_get_first_child_by_type(endpoint_node, 0x00080f01);
    attribute_store_get_reported(zcl_last_non_zero_level,
                                 &desired_level,
                                 sizeof(desired_level));
  }

  // Issue a move to level command internally.
  level_cluster_mapper_move_to_level(unid,
                                     endpoint,
                                     UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                     desired_level,
                                     0,
                                     0,
                                     0);
}

static void update_current_level_attribute_after_off_command(
  const dotdot_unid_t unid, const dotdot_endpoint_id_t endpoint)
{
  // Issue a move to level command internally, no harm if the command is not supported, nothing will happen.
  level_cluster_mapper_move_to_level(unid,
                                     endpoint,
                                     UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                     0,
                                     0,
                                     0,
                                     0);
}

////////////////////////////////////////////////////////////////////////////////
// Private component callback functions for DotDot MQTT
////////////////////////////////////////////////////////////////////////////////
sl_status_t
  on_off_cluster_on_command(const dotdot_unid_t unid,
                            const dotdot_endpoint_id_t endpoint,
                            uic_mqtt_dotdot_callback_call_type_t callback_type)
{
  if (callback_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return dotdot_is_supported_on_off_on_off(unid, endpoint) ? SL_STATUS_OK
                                                               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  //1.5.7.2.1. Effect on ReceiptOn receipt of the On command, a server SHALL
  //set the OnOff attribute to TRUE.
  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(LOG_TAG,
                 "Updating ZCL desired values after OnOff::On command");
    dotdot_set_on_off_on_off(unid, endpoint, DESIRED_ATTRIBUTE, true);

    // If we are turning on, level is supported, the device should go
    // back to the ON_LEVEL attribute, and if set to 0xFF, to the last cached non-zero level.
    update_current_level_attribute_after_on_command(unid, endpoint);

    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG, "Clearing OnOff::OnOff reported value");
      dotdot_on_off_on_off_undefine_reported(unid, endpoint);
    }
  }

  //Additionally, when the OnTime and OffWaitTime attributes are both
  //supported, if the value of theOnTime attribute is equal to 0,
  //the server SHALL set the OffWaitTime attribute to 0.
  if (dotdot_is_supported_on_off_on_time(unid, endpoint)
      && dotdot_is_supported_on_off_off_wait_time(unid, endpoint)) {
    if (dotdot_get_on_off_on_time(unid, endpoint, REPORTED_ATTRIBUTE) == 0) {
      dotdot_set_on_off_off_wait_time(unid, endpoint, REPORTED_ATTRIBUTE, 0);
    }
  }

  return SL_STATUS_OK;
}

sl_status_t
  on_off_cluster_off_command(const dotdot_unid_t unid,
                             const dotdot_endpoint_id_t endpoint,
                             uic_mqtt_dotdot_callback_call_type_t callback_type)
{
  if (callback_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return dotdot_is_supported_on_off_on_off(unid, endpoint) ? SL_STATUS_OK
                                                               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  if (is_desired_value_update_on_commands_enabled()) {
    sl_log_debug(LOG_TAG,
                 "Updating ZCL desired values after OnOff::Off command");
    dotdot_set_on_off_on_off(unid, endpoint, DESIRED_ATTRIBUTE, false);

    // If we are turning off, level is supported, the device should go to 0
    update_current_level_attribute_after_off_command(unid, endpoint);

    if (is_clear_reported_enabled()) {
      sl_log_debug(LOG_TAG, "Clearing OnOff::OnOff reported value");
      dotdot_on_off_on_off_undefine_reported(unid, endpoint);
    }
  }

  if (dotdot_is_supported_on_off_on_time(unid, endpoint)) {
    dotdot_set_on_off_on_time(unid, endpoint, REPORTED_ATTRIBUTE, 0);
  }

  return SL_STATUS_OK;
}

sl_status_t on_off_cluster_toggle_command(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t callback_type)
{
  if (callback_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return dotdot_is_supported_on_off_on_off(unid, endpoint) ? SL_STATUS_OK
                                                               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  if (dotdot_get_on_off_on_off(unid, endpoint, REPORTED_ATTRIBUTE)) {
    on_off_cluster_off_command(unid, endpoint, callback_type);
  } else {
    on_off_cluster_on_command(unid, endpoint, callback_type);
  }

  return SL_STATUS_OK;
}

sl_status_t on_off_cluster_off_with_effect_command(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  OffWithEffectEffectIdentifier effect_identifier,
  uint8_t effect_variant)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    //All we care about is if the  move_to_lvel is supported
    return level_cluster_mapper_move_to_level_with_on_off(unid,
                                                          endpoint,
                                                          call_type,
                                                          0,
                                                          0,
                                                          0,
                                                          0);
  }

  // On receipt of the OffWithEffect command the server SHALL check the value
  // of the GlobalSceneControl attribute. If the GlobalSceneControl
  // attribute is equal to TRUE, the server SHALL store its settings in its
  // global scene then set the GlobalSceneControl attribute to FALSE,
  // and if the OnTime attribute is supported set the OnTime attribute to 0.
  if (dotdot_get_on_off_global_scene_control(unid,
                                             endpoint,
                                             REPORTED_ATTRIBUTE)) {
    uic_mqtt_dotdot_scenes_command_store_scene_fields_t feilds = {
      .groupid = 0,
      .sceneid = 0,
    };
    uic_mqtt_dotdot_scenes_publish_store_scene_command(unid, endpoint, &feilds);
    dotdot_set_on_off_global_scene_control(unid,
                                           endpoint,
                                           REPORTED_ATTRIBUTE,
                                           false);
    dotdot_set_on_off_on_time(unid, endpoint, REPORTED_ATTRIBUTE, 0);
  }

  switch (effect_identifier) {
    case ZCL_OFF_WITH_EFFECT_EFFECT_IDENTIFIER_DELAYED_ALL_OFF:
      if (effect_variant == 0x00) {
        // Fade to off in 0.8 seconds
        return level_cluster_mapper_move_to_level_with_on_off(unid,
                                                              endpoint,
                                                              call_type,
                                                              0,
                                                              8,
                                                              0,
                                                              0);
      } else if (effect_variant == 0x01) {
        // No fade
        return level_cluster_mapper_move_to_level_with_on_off(unid,
                                                              endpoint,
                                                              call_type,
                                                              0,
                                                              0,
                                                              0,
                                                              0);
      } else if (effect_variant == 0x02) {
        // 50% dim down in 0.8 seconds then fade to off in 12 seconds
        return level_cluster_mapper_move_to_level_with_on_off(unid,
                                                              endpoint,
                                                              call_type,
                                                              128,
                                                              8,
                                                              0,
                                                              0);
      }
      break;
    case ZCL_OFF_WITH_EFFECT_EFFECT_IDENTIFIER_DYING_LIGHT:
      if (effect_variant == 0x00) {
        //20% dim up in 0.5s then fade to off in 1 second

        /// FIXME this is not quite right
        return level_cluster_mapper_move_to_level_with_on_off(unid,
                                                              endpoint,
                                                              call_type,
                                                              0,
                                                              12,
                                                              0,
                                                              0);
      }
      break;
    default:
      break;
  }
  return SL_STATUS_OK;
}

sl_status_t on_off_cluster_off_with_recall_global_scene_command(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    //All we care about is if the  move_to_lvel is supported
    return level_cluster_mapper_move_to_level_with_on_off(unid,
                                                          endpoint,
                                                          call_type,
                                                          0,
                                                          0,
                                                          0,
                                                          0);
  }

  // On receipt of the OnWithRecallGlobalScene command, if the GlobalSceneControl
  // attribute is equal to TRUE, the server SHALL discard the command. If the
  // GlobalSceneControl attribute is equal to FALSE, the Scene cluster server on
  // the same endpoint SHALL recall its global scene, updating the OnOff attribute
  // accordingly. The OnOff server SHALL then set the GlobalSceneControl attribute
  // to TRUE.
  if (!dotdot_get_on_off_global_scene_control(unid,
                                              endpoint,
                                              REPORTED_ATTRIBUTE)) {
    uic_mqtt_dotdot_scenes_command_recall_scene_fields_t feilds = {

      .groupid         = 0,
      .sceneid         = 0,
      .transition_time = 0,
    };

    dotdot_set_on_off_on_off(unid, endpoint, DESIRED_ATTRIBUTE, true);

    uic_mqtt_dotdot_scenes_publish_recall_scene_command(unid,
                                                        endpoint,
                                                        &feilds);
    dotdot_set_on_off_global_scene_control(unid,
                                           endpoint,
                                           REPORTED_ATTRIBUTE,
                                           true);
  }
  return SL_STATUS_OK;
}

static void
  on_off_on_with_timed_off_timeout(attribute_store_node_t endpoint_node)
{
  attribute_store_node_t off_wait_time
    = attribute_store_get_first_child_by_type(
      endpoint_node,
      DOTDOT_ATTRIBUTE_ID_ON_OFF_OFF_WAIT_TIME);
  attribute_store_node_t on_time = attribute_store_get_first_child_by_type(
    endpoint_node,
    DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_TIME);
  attribute_store_node_t on_off = attribute_store_get_first_child_by_type(
    endpoint_node,
    DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF);

  if (attribute_store_get_reported_number(on_off) > 0.0) {
    //If the value of the OnOff attribute is equal to TRUE and the value of the
    //OnTime attribute is greater than zero, the server SHALL decrement the value of
    //the OnTime attribute. If the value of the OnTime attribute reaches 0, the
    //server SHALL set the OffWaitTime and OnOff attributes to 0 and FALSE,
    //respectively.
    number_t on_time_value = attribute_store_get_reported_number(on_time);
    if (on_time_value > 0.0) {
      attribute_store_set_reported_number(on_time, on_time_value - 1.0);
      attribute_timeout_set_callback(endpoint_node,
                                     100,
                                     on_off_on_with_timed_off_timeout);
    } else {
      attribute_store_set_reported_number(off_wait_time, 0.0);
      attribute_store_set_desired_number(on_off, 0.0);
      attribute_store_set_reported_number(on_off, 0.0);
    }
  } else {
    // If the value of the OnOff attribute is equal to FALSE and the value of the
    // OffWaitTime attribute is greater than zero, the server SHALL decrement the
    // value of the OffWaitTime attribute. If the value of the OffWaitTime attribute
    // reaches 0, the server SHALL terminate the update.
    number_t off_wait_time_value
      = attribute_store_get_reported_number(off_wait_time);
    if (off_wait_time_value > 0.0) {
      attribute_store_set_reported_number(off_wait_time,
                                          off_wait_time_value - 1.0);
      attribute_timeout_set_callback(endpoint_node,
                                     100,
                                     on_off_on_with_timed_off_timeout);
    }
  }
}

sl_status_t on_off_on_with_timed_off_callback_set_command(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint8_t on_off_control,
  uint16_t on_time,
  uint16_t off_wait_time)
{
  if (call_type == UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK) {
    if (is_automatic_deduction_of_supported_commands_enabled()) {
      return dotdot_is_supported_on_off_on_off(unid, endpoint) ? SL_STATUS_OK
                                                               : SL_STATUS_FAIL;
    } else {
      return SL_STATUS_FAIL;
    }
  }

  if (!dotdot_is_supported_on_off_on_time(unid, endpoint)) {
    return SL_STATUS_OK;
  }

  // The AcceptOnlyWhenOn sub-field is 1 bit in length and specifies whether the
  // OnWithTimedOff command is to be processed unconditionally or only when the
  // OnOff attribute is equal to TRUE. If this sub-field is set to 1, the
  // OnWithTimedOff command SHALL only be accepted if the OnOff attribute is
  // equal to TRUE. If this sub-field is set to 0, the OnWithTimedOff command
  // SHALL be processed unconditionally.
  if (on_off_control & ACCEPT_ONLY_WHEN_ON_MASK) {
    if (dotdot_get_on_off_on_off(unid, endpoint, REPORTED_ATTRIBUTE) == false) {
      return SL_STATUS_OK;
    }
  }

  int current_wait_time
    = dotdot_get_on_off_off_wait_time(unid, endpoint, REPORTED_ATTRIBUTE);
  int current_on_time
    = dotdot_get_on_off_on_time(unid, endpoint, REPORTED_ATTRIBUTE);

  //If the value of the OffWaitTime attribute is greater than zero and the value
  //of the OnOff attribute is equal to FALSE, then the server SHALL set the
  //OffWaitTime attribute to the minimum of the OffWaitTime attribute and the
  //value specified in the OffWaitTime field.
  if ((current_wait_time > 0)
      && (dotdot_get_on_off_on_off(unid, endpoint, REPORTED_ATTRIBUTE) == false)
      && (off_wait_time < current_wait_time)) {
    dotdot_set_on_off_off_wait_time(unid,
                                    endpoint,
                                    REPORTED_ATTRIBUTE,
                                    off_wait_time);
  } else {
    //In all other cases, the server SHALL set the OnTime attribute to the
    //maximum of the OnTime attribute and the value specified in the OnTime
    //field, set the OffWaitTime attribute to the value specified in the
    //OffWaitTime field and set the OnOff attribute to TRUE.
    if (on_time > current_on_time) {
      dotdot_set_on_off_on_time(unid, endpoint, REPORTED_ATTRIBUTE, on_time);
    }
    if (is_desired_value_update_on_commands_enabled()) {
      dotdot_set_on_off_on_off(unid, endpoint, DESIRED_ATTRIBUTE, true);
    }
    dotdot_set_on_off_off_wait_time(unid,
                                    endpoint,
                                    REPORTED_ATTRIBUTE,
                                    off_wait_time);
  }
  //If the values of the OnTime and OffWaitTime attributes are both not equal
  //to null, the server SHALL then update these attributes every 1/10th second
  //until both the OnTime and OffWaitTime attributes are equal to 0, as
  //follows:
  if ((dotdot_get_on_off_off_wait_time(unid, endpoint, REPORTED_ATTRIBUTE) > 0)
      && (dotdot_get_on_off_on_time(unid, endpoint, REPORTED_ATTRIBUTE) > 0)) {
    attribute_store_node_t endpoint_node
      = unify_dotdot_attributes_get_endpoint_node()(unid, endpoint);
    attribute_timeout_set_callback(endpoint_node,
                                   100,
                                   on_off_on_with_timed_off_timeout);
  }

  return SL_STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// Internal component function that register callbacks to DotDot MQTT
////////////////////////////////////////////////////////////////////////////////
sl_status_t on_off_cluster_mapper_init()
{
  // OnOff Cluster Commands.
  sl_log_debug(LOG_TAG, "On off Cluster mapper initialization\n");

  uic_mqtt_dotdot_on_off_on_callback_set(&on_off_cluster_on_command);
  uic_mqtt_dotdot_on_off_off_callback_set(&on_off_cluster_off_command);
  uic_mqtt_dotdot_on_off_toggle_callback_set(&on_off_cluster_toggle_command);
  uic_mqtt_dotdot_on_off_off_with_effect_callback_set(
    &on_off_cluster_off_with_effect_command);
  uic_mqtt_dotdot_on_off_on_with_recall_global_scene_callback_set(
    &on_off_cluster_off_with_recall_global_scene_command);
  uic_mqtt_dotdot_on_off_on_with_timed_off_callback_set(
    &on_off_on_with_timed_off_callback_set_command);
  return SL_STATUS_OK;
}
