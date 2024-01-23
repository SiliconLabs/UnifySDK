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
#include "zwave_command_class_switch_color.h"

// Interfaces
#include "zwave_command_class_color_switch_types.h"
#include "zwave_command_class_generic_types.h"

// Includes from other ZPC Components
#include "zwave_command_class_indices.h"
#include "zwave_unid.h"
#include "zwave_command_handler.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"
#include "zpc_attribute_resolver.h"
#include "zwave_command_classes_utils.h"

// Includes from other Unify Components
#include "dotdot_mqtt.h"
#include "dotdot_mqtt_generated_commands.h"
#include "attribute_store_helper.h"
#include "attribute_resolver.h"
#include "attribute_transitions.h"
#include "attribute_timeouts.h"
#include "sl_log.h"

#define LOG_TAG "zwave_command_class_switch_color"

static void set_duration(attribute_store_node_t state_node,
                         attribute_store_node_value_state_t state,
                         uint32_t duration)
{
  attribute_store_node_t duration_node
    = attribute_store_get_first_child_by_type(
      state_node,
      ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_DURATION);

  attribute_store_set_node_attribute_value(duration_node,
                                           state,
                                           (uint8_t *)&duration,
                                           sizeof(duration));
}

[[maybe_unused]]
static void
  set_all_color_switch_durations(attribute_store_node_t state_node,
                                 attribute_store_node_value_state_t value_state,
                                 color_component_id_duration_t duration)
{
  uint32_t index = 0;
  attribute_store_node_t component_node
    = attribute_store_get_first_child_by_type(
      state_node,
      ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_COLOR_COMPONENT_ID);
  while (component_node != ATTRIBUTE_STORE_INVALID_NODE) {
    index += 1;

    set_duration(component_node, value_state, duration);

    component_node = attribute_store_get_node_child_by_type(
      state_node,
      ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_COLOR_COMPONENT_ID,
      index);
  }
}

static void switch_color_undefine_reported(attribute_store_node_t state_node)
{
  zwave_command_class_switch_color_invoke_on_all_attributes_with_return_value(
    state_node,
    ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_VALUE,
    attribute_stop_transition);

  attribute_store_node_t duration_node
      = attribute_store_get_first_child_by_type(
        state_node,
        ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_DURATION);
  color_component_id_duration_t duration = 0;
  attribute_store_undefine_desired(duration_node);
  attribute_store_set_reported(duration_node,
                               &duration,
                               sizeof(duration));

  sl_log_debug(LOG_TAG, "Transition time expired, probe color");
  zwave_command_class_switch_color_invoke_on_all_attributes(
    state_node,
    ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_VALUE,
    attribute_store_undefine_desired);
  zwave_command_class_switch_color_invoke_on_all_attributes(
    state_node,
    ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_VALUE,
    attribute_store_undefine_reported);
}

// FIXME: UIC-1901 This function belongs to zwave_command_class_switch_color.rs, but this
// component really cannot interact with the attribute resolver.
static void
  on_color_switch_state_send_data_complete(attribute_store_node_t state_node,
                                           resolver_rule_type_t rule_type,
                                           zpc_resolver_event_t event)
{
  if (rule_type == RESOLVER_GET_RULE) {
    sl_log_error("zwave_command_class_switch_color_control",
                 "Received a Get Rule type callback for State node %d "
                 "whereas no Get rule is registered. Something is broken.");
    return;
  }

  attribute_store_node_t duration_node
      = attribute_store_get_first_child_by_type(
        state_node,
        ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_DURATION);

  color_component_id_duration_t duration = 0;
  attribute_store_get_desired(duration_node,
                              &duration,
                              sizeof(duration));

  clock_time_t zwave_desired_duration
    = zwave_duration_to_time((uint8_t)duration);

  switch (event) {
    case FRAME_SENT_EVENT_OK_SUPERVISION_WORKING:
      attribute_store_set_reported_as_desired(duration_node);
      break;

    case FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS:
      zwave_command_class_switch_color_invoke_on_all_attributes_with_return_value(
        state_node,
        ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_VALUE,
        &attribute_store_set_reported_as_desired);
      zwave_command_class_switch_color_invoke_on_all_attributes(
        state_node,
        ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_VALUE,
        &attribute_store_undefine_desired);
      set_duration(state_node, REPORTED_ATTRIBUTE, 0);
      attribute_store_undefine_desired(duration_node);
      break;

    case FRAME_SENT_EVENT_OK_NO_SUPERVISION:
      zwave_command_class_switch_color_invoke_on_all_attributes_with_return_value(
        state_node,
        ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_VALUE,
        attribute_stop_transition);
      if(zwave_desired_duration > 0) {
        // Should we estimate reported color values during transition
        // and publish them as reported, like we did for level cluster?

        zwave_command_class_switch_color_invoke_on_all_attributes(
          state_node,
          ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_VALUE,
          attribute_store_undefine_desired);

        attribute_store_set_reported_as_desired(duration_node);
        attribute_store_undefine_desired(duration_node);

        // Probe again after this duration
        attribute_timeout_set_callback(state_node,
                                       zwave_desired_duration + PROBE_BACK_OFF,
                                       &switch_color_undefine_reported);
      } else {
        zwave_command_class_switch_color_invoke_on_all_attributes(
          state_node,
          ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_VALUE,
          attribute_store_undefine_desired);
        zwave_command_class_switch_color_invoke_on_all_attributes(
          state_node,
          ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_VALUE,
          attribute_store_undefine_reported);
        attribute_store_undefine_desired(duration_node);
      }
      break;

    // FRAME_SENT_EVENT_OK_SUPERVISION_NO_SUPPORT:
    // FRAME_SENT_EVENT_OK_SUPERVISION_FAIL:
    default:
      zwave_command_class_switch_color_invoke_on_all_attributes_with_return_value(
        state_node,
        ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_VALUE,
        &attribute_stop_transition);
      zwave_command_class_switch_color_invoke_on_all_attributes(
        state_node,
        ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_VALUE,
        &attribute_store_undefine_desired);
      zwave_command_class_switch_color_invoke_on_all_attributes(
        state_node,
        ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_VALUE,
        &attribute_store_undefine_reported);
      attribute_store_undefine_desired(duration_node);
      attribute_store_undefine_reported(duration_node);
      break;
  }

  command_status_values_t state = FINAL_STATE;
  attribute_store_set_reported(state_node, &state, sizeof(state));
  attribute_store_set_desired(state_node, &state, sizeof(state));
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_switch_color_init()
{
  // FIXME: UIC-1901
  register_send_event_handler(ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_STATE,
                              &on_color_switch_state_send_data_complete);

  // zwave_command_handler_t is registered using rust, see zwave_command_class_switch_color.rs
  return SL_STATUS_OK;
}

void zwave_command_class_switch_color_invoke_on_all_attributes(
  attribute_store_node_t state_node,
  attribute_store_type_t child_node_type,
  void (*function)(attribute_store_node_t))
{
  uint32_t index = 0;
  attribute_store_node_t component_node
    = attribute_store_get_first_child_by_type(
      state_node,
      ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_COLOR_COMPONENT_ID);
  while (component_node != ATTRIBUTE_STORE_INVALID_NODE) {
    index += 1;
    attribute_store_node_t child_node
      = attribute_store_get_first_child_by_type(component_node,
                                                child_node_type);

    function(child_node);
    component_node = attribute_store_get_node_child_by_type(
      state_node,
      ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_COLOR_COMPONENT_ID,
      index);
  }
}

void zwave_command_class_switch_color_invoke_on_all_attributes_with_return_value(
  attribute_store_node_t state_node,
  attribute_store_type_t child_node_type,
  sl_status_t (*function)(attribute_store_node_t))
{
  uint32_t index = 0;
  attribute_store_node_t component_node
    = attribute_store_get_first_child_by_type(
      state_node,
      ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_COLOR_COMPONENT_ID);
  while (component_node != ATTRIBUTE_STORE_INVALID_NODE) {
    index += 1;
    attribute_store_node_t child_node
      = attribute_store_get_first_child_by_type(component_node,
                                                child_node_type);

    function(child_node);
    component_node = attribute_store_get_node_child_by_type(
      state_node,
      ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_COLOR_COMPONENT_ID,
      index);
  }
}
