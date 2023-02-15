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

// Includes from other Unify Components
#include "dotdot_mqtt.h"
#include "dotdot_mqtt_generated_commands.h"
#include "attribute_store_helper.h"
#include "attribute_resolver.h"
#include "attribute_transitions.h"
#include "attribute_timeouts.h"
#include "sl_log.h"

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

    attribute_store_node_t duration_node
      = attribute_store_get_first_child_by_type(
        component_node,
        ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_DURATION);

    attribute_store_set_node_attribute_value(duration_node,
                                             value_state,
                                             (uint8_t *)&duration,
                                             sizeof(duration));

    component_node = attribute_store_get_node_child_by_type(
      state_node,
      ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_COLOR_COMPONENT_ID,
      index);
  }
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

  switch (event) {
    case FRAME_SENT_EVENT_OK_SUPERVISION_WORKING:
      zwave_command_class_switch_color_invoke_on_all_attributes_with_return_value(
        state_node,
        ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_DURATION,
        &attribute_store_set_reported_as_desired);
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
      set_all_color_switch_durations(state_node, REPORTED_ATTRIBUTE, 0);
      set_all_color_switch_durations(state_node, DESIRED_ATTRIBUTE, 0);
      break;

    // FRAME_SENT_EVENT_OK_NO_SUPERVISION:
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
      zwave_command_class_switch_color_invoke_on_all_attributes(
        state_node,
        ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_DURATION,
        &attribute_store_undefine_desired);
      zwave_command_class_switch_color_invoke_on_all_attributes(
        state_node,
        ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_DURATION,
        &attribute_store_undefine_reported);
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