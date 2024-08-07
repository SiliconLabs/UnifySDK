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
#include "zwave_command_class_switch_multilevel.h"
#include "zwave_command_classes_utils.h"

// Generic includes
#include <stdlib.h>

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
#include "attribute_mapper.h"
#include "sl_log.h"

// Log tag
#define LOG_TAG "zwave_command_class_switch_multilevel"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_##type

#define SET_VALUE_INDEX                   2
#define SET_DURATION_INDEX                3
#define START_LEVEL_CHANGE_UP_DOWN_INDEX  2
#define START_LEVEL_CHANGE_UP_DOWN_MASK   0x40
#define START_LEVEL_CHANGE_DURATION_INDEX 4

// Constant values for our Multilevel Switch
#define MAX_LEVEL 0x63
#define MIN_LEVEL 0x00

/**
 * @brief Full state of a multilevel Switch supporting node.
 */
typedef struct multilevel_switch_state {
  command_status_values_t desired_state;
  command_status_values_t reported_state;
  uint32_t desired_value;
  uint32_t reported_value;
  uint32_t desired_duration;
  uint32_t reported_duration;
} multilevel_switch_state_t;

static const attribute_store_type_t v1_sub_attributes[]
  = {ATTRIBUTE(VALUE), ATTRIBUTE(DURATION)};

static const attribute_store_type_t v3_attributes[]
  = {ATTRIBUTE(CAPABILITIES_REQUESTED)};

// Frame parsing indices:
#define REPORT_VALUE_INDEX        2
#define REPORT_TARGET_VALUE_INDEX 3
#define REPORT_DURATION_INDEX     4

// Constants
#define IGNORE_START_LEVEL 1 << 5
#define UP                 0
#define DOWN               1 << 6

///////////////////////////////////////////////////////////////////////////////
// Prototypes
///////////////////////////////////////////////////////////////////////////////
static void zwave_command_class_switch_multilevel_on_desired_sub_state_update(
  attribute_store_node_t node, attribute_store_change_t change);

///////////////////////////////////////////////////////////////////////////////
// Local helper functions
///////////////////////////////////////////////////////////////////////////////
static void get_state(attribute_store_node_t state_node,
                      multilevel_switch_state_t *state)
{
  attribute_store_get_desired_else_reported(state_node,
                                            &state->desired_state,
                                            sizeof(command_status_values_t));
  attribute_store_get_reported(state_node,
                               &state->reported_state,
                               sizeof(command_status_values_t));

  attribute_store_node_t duration_node
    = attribute_store_get_first_child_by_type(state_node, ATTRIBUTE(DURATION));

  attribute_store_get_desired_else_reported(duration_node,
                                            &state->desired_duration,
                                            sizeof(uint32_t));
  attribute_store_get_reported(duration_node,
                               &state->reported_duration,
                               sizeof(uint32_t));

  attribute_store_node_t value_node
    = attribute_store_get_first_child_by_type(state_node, ATTRIBUTE(VALUE));

  attribute_store_get_desired_else_reported(value_node,
                                            &state->desired_value,
                                            sizeof(uint32_t));
  attribute_store_get_reported(value_node,
                               &state->reported_value,
                               sizeof(uint32_t));
}

static void set_desired_value(attribute_store_node_t state_node, uint32_t value)
{
  attribute_store_node_t value_node
    = attribute_store_get_first_child_by_type(state_node, ATTRIBUTE(VALUE));
  attribute_store_set_desired(value_node, &value, sizeof(value));
}

static void set_reported_value(attribute_store_node_t state_node,
                               uint32_t value)
{
  attribute_store_node_t value_node
    = attribute_store_get_first_child_by_type(state_node, ATTRIBUTE(VALUE));
  attribute_store_set_reported(value_node, &value, sizeof(value));
}

static void set_desired_duration(attribute_store_node_t state_node,
                                 uint32_t duration)
{
  attribute_store_node_t duration_node
    = attribute_store_get_first_child_by_type(state_node, ATTRIBUTE(DURATION));
  attribute_store_set_desired(duration_node, &duration, sizeof(duration));
}

static void set_reported_duration(attribute_store_node_t state_node,
                                  uint32_t duration)
{
  attribute_store_node_t duration_node
    = attribute_store_get_first_child_by_type(state_node, ATTRIBUTE(DURATION));
  attribute_store_set_reported(duration_node, &duration, sizeof(duration));
}

///////////////////////////////////////////////////////////////////////////////
// Resolution functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_switch_multilevel_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  ZW_SWITCH_MULTILEVEL_GET_V4_FRAME *get_frame
    = (ZW_SWITCH_MULTILEVEL_GET_V4_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_SWITCH_MULTILEVEL_V4;
  get_frame->cmd      = SWITCH_MULTILEVEL_GET_V4;
  *frame_length       = sizeof(ZW_SWITCH_MULTILEVEL_GET_V4_FRAME);
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_switch_multilevel_supported_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  ZW_SWITCH_MULTILEVEL_SUPPORTED_GET_V4_FRAME *get_frame
    = (ZW_SWITCH_MULTILEVEL_SUPPORTED_GET_V4_FRAME *)frame;
  get_frame->cmdClass = COMMAND_CLASS_SWITCH_MULTILEVEL_V4;
  get_frame->cmd      = SWITCH_MULTILEVEL_SUPPORTED_GET_V4;
  *frame_length       = sizeof(ZW_SWITCH_MULTILEVEL_SUPPORTED_GET_V4_FRAME);
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_switch_multilevel_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  multilevel_switch_state_t state = {};
  get_state(node, &state);

  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);

  attribute_store_node_t version_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(VERSION));
  attribute_store_node_t value_node
    = attribute_store_get_first_child_by_type(node, ATTRIBUTE(VALUE));

  zwave_cc_version_t supporting_node_version = 0;
  attribute_store_get_reported(version_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  // Are we modifying a transition target in favor of a new one
  // without changing the duration ? In this case, adjust
  // the desired duration to what's remaining in the transition.
  clock_time_t remaining_duration
    = attribute_transition_get_remaining_duration(value_node);

  if ((remaining_duration != 0)
      && (state.desired_duration == state.reported_duration)) {
    state.desired_duration  = time_to_zwave_duration(remaining_duration);
    state.reported_duration = state.desired_duration;
    sl_log_debug(LOG_TAG,
                 "Adjusted desired duration to remaining: %d",
                 state.desired_duration);
  }
  if (supporting_node_version < 2) {
    state.desired_duration  = 0;
    state.reported_duration = state.desired_duration;
  }

  // For simplicity, we avoid using Start level Change / Stop level change.
  ZW_SWITCH_MULTILEVEL_SET_V4_FRAME *set_frame
    = (ZW_SWITCH_MULTILEVEL_SET_V4_FRAME *)frame;
  set_frame->cmdClass = COMMAND_CLASS_SWITCH_MULTILEVEL_V4;
  set_frame->cmd      = SWITCH_MULTILEVEL_SET_V4;
  *frame_length       = sizeof(ZW_SWITCH_MULTILEVEL_SET_V4_FRAME);

  set_frame->value           = (uint8_t)state.desired_value;
  set_frame->duration        = (uint8_t)state.desired_duration;

  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Frame parsing functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_switch_multilevel_handle_set(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // We expect to have at least 1 byte of value.
  if (frame_length <= SET_VALUE_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  unid_t node_unid;
  zwave_unid_from_node_id(connection_info->remote.node_id, node_unid);
  dotdot_endpoint_id_t endpoint_id = connection_info->remote.endpoint_id;

  uic_mqtt_dotdot_level_command_move_to_level_fields_t fields = {};
  fields.level = frame_data[SET_VALUE_INDEX];
  if (frame_length > SET_DURATION_INDEX) {
    // Let's not sweat it too much with duration, we just take the value and multiply by 10.
    // We ignore the minutes (if duration > 127)
    if (frame_data[SET_DURATION_INDEX] != 0xFF) {
      fields.transition_time = frame_data[SET_DURATION_INDEX] * 10;
    }
  }

  uic_mqtt_dotdot_level_publish_generated_move_to_level_command(node_unid,
                                                                endpoint_id,
                                                                &fields);
  return SL_STATUS_NOT_SUPPORTED;
}

static sl_status_t
  zwave_command_class_switch_multilevel_handle_start_level_change(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  // We expect to have at least 1 byte of value.
  if (frame_length <= START_LEVEL_CHANGE_UP_DOWN_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  unid_t node_unid;
  zwave_unid_from_node_id(connection_info->remote.node_id, node_unid);
  dotdot_endpoint_id_t endpoint_id = connection_info->remote.endpoint_id;

  // Indicate if the node is trying to set on or off.
  uint8_t up_down = frame_data[START_LEVEL_CHANGE_UP_DOWN_INDEX]
                    & START_LEVEL_CHANGE_UP_DOWN_MASK;
  uint8_t rate = 0xFF;
  if (frame_length > START_LEVEL_CHANGE_DURATION_INDEX) {
    rate = frame_data[START_LEVEL_CHANGE_DURATION_INDEX];
  }
  if (rate != 0xFF) {
    rate = 99 / rate;
  }

  uic_mqtt_dotdot_level_command_move_fields_t fields = {};
  fields.move_mode
    = (up_down == 0) ? ZCL_MOVE_STEP_MODE_UP : ZCL_MOVE_STEP_MODE_DOWN;
  fields.rate = rate;

  uic_mqtt_dotdot_level_publish_generated_move_command(node_unid,
                                                       endpoint_id,
                                                       &fields);
  return SL_STATUS_NOT_SUPPORTED;
}

static sl_status_t
  zwave_command_class_switch_multilevel_handle_stop_level_change(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  unid_t node_unid;
  zwave_unid_from_node_id(connection_info->remote.node_id, node_unid);
  dotdot_endpoint_id_t endpoint_id = connection_info->remote.endpoint_id;

  uic_mqtt_dotdot_level_command_stop_fields_t fields = {};
  uic_mqtt_dotdot_level_publish_generated_stop_command(node_unid,
                                                       endpoint_id,
                                                       &fields);
  return SL_STATUS_NOT_SUPPORTED;
}

static sl_status_t zwave_command_class_switch_multilevel_handle_report(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // We expect to have at least 1 byte of value
  if (frame_length <= REPORT_VALUE_INDEX) {
    return SL_STATUS_FAIL;
  }

  // Find the Endpoint
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  attribute_store_node_t state_node
    = attribute_store_get_first_child_by_type(endpoint_node, ATTRIBUTE(STATE));
  attribute_store_node_t value_node
    = attribute_store_get_first_child_by_type(state_node, ATTRIBUTE(VALUE));

  multilevel_switch_state_t state = {};
  get_state(state_node, &state);

  uint32_t current_value = frame_data[REPORT_VALUE_INDEX];
  if (current_value == 0xFF) {  // 0xFF is deprecated and means 100%.
    current_value = MAX_LEVEL;
  }

  // Save the reported value first.
  set_reported_value(state_node, current_value);

  // Align back the desired if no transition ongoing
  if (is_attribute_transition_ongoing(value_node) == false) {
    set_desired_value(state_node, current_value);
  }

  // Duration:
  uint32_t duration = 0;  // Assumed value if no duration is provided.
  if (frame_length > REPORT_DURATION_INDEX) {
    duration = frame_data[REPORT_DURATION_INDEX];
  }
  set_reported_duration(state_node, duration);
  set_desired_duration(state_node, duration);

  // Is there a target value:
  if (frame_length > REPORT_TARGET_VALUE_INDEX) {
    uint32_t target_value = frame_data[REPORT_TARGET_VALUE_INDEX];
    if (target_value != MIN_LEVEL || duration != 0) {
      set_desired_value(state_node, target_value);
    }
  }

  if (duration == 0) {
    // Ensure no mismatch between reported/desired if duration is 0.
    attribute_store_set_reported_as_desired(value_node);
    attribute_stop_transition(value_node);
  }
  if (duration > 0 && false == attribute_store_is_value_matched(value_node)
      && false == is_attribute_transition_ongoing(value_node)) {
    clock_time_t clock_duration = zwave_duration_to_time((uint8_t)duration);
    attribute_start_transition(value_node, clock_duration);
    // Probe again after that duration.
    attribute_timeout_set_callback(state_node,
                                   clock_duration + PROBE_BACK_OFF,
                                   &attribute_store_undefine_reported);
  }

  // Set the reported of the state node, so that it does not "get" again.
  set_command_status_value(state_node, FINAL_STATE, FINAL_STATE);

  return SL_STATUS_OK;
}

static sl_status_t
  zwave_command_class_switch_multilevel_handle_supported_report(
    const zwave_controller_connection_info_t *connection_info,
    const uint8_t *frame_data,
    uint16_t frame_length)
{
  // We do nothing with this frame.
  // Mark in the attribute store that the capabilities have been requested.

  // Find the Endpoint
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection_info);

  bool capabilities_requested = true;
  attribute_store_set_child_reported_only_if_exists(
    endpoint_node,
    ATTRIBUTE(CAPABILITIES_REQUESTED),
    &capabilities_requested,
    sizeof(capabilities_requested));

  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Resolver callback functions
///////////////////////////////////////////////////////////////////////////////
static void on_state_send_data_complete(attribute_store_node_t state_node,
                                        resolver_rule_type_t rule_type,
                                        zpc_resolver_event_t event)
{
  if (rule_type == RESOLVER_GET_RULE) {
    return;
  }

  multilevel_switch_state_t state;
  get_state(state_node, &state);
  attribute_store_node_t value_node
    = attribute_store_get_first_child_by_type(state_node, ATTRIBUTE(VALUE));
  clock_time_t zwave_desired_duration
    = zwave_duration_to_time((uint8_t)state.desired_duration);

  switch (event) {
    case FRAME_SENT_EVENT_OK_SUPERVISION_WORKING:
      // Initiate a transition for the value, following the duration
      set_reported_duration(state_node, state.desired_duration);

      // Start our transition for the first WORKING status.
      if (!is_attribute_transition_ongoing(value_node)
          && (zwave_desired_duration > 0)) {
        attribute_start_transition(value_node, zwave_desired_duration);
      }
      break;

    case FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS:
      if (state.desired_duration > 0) {
        sl_log_debug(LOG_TAG,
                     "Node reports Supervision Success with a "
                     "non-instantaneous command. "
                     "Assuming we received a WORKING status.");
        // Duration was accepted but success? (should have been working)
        // Be tolerant to this and accept that they say success to a transition
        set_reported_duration(state_node, state.desired_duration);
        // Start a transition
        attribute_stop_transition(value_node);
        attribute_start_transition(value_node, zwave_desired_duration);
        // Probe again when the transition is over.
        attribute_timeout_set_callback(state_node,
                                       zwave_desired_duration + PROBE_BACK_OFF,
                                       &attribute_store_undefine_reported);
      } else {
        // Align the Value to the initial desired:
        set_reported_value(state_node, state.desired_value);
        attribute_stop_transition(value_node);
        // Success only means that a transition is over. Set the duration to 0.
        set_reported_duration(state_node, 0);
        set_desired_duration(state_node, 0);
      }

      // In any case, mark that we don't need more commands:
      set_command_status_value(state_node, FINAL_STATE, FINAL_STATE);
      break;

    case FRAME_SENT_EVENT_OK_NO_SUPERVISION:
      // We assumed it worked in case of no-supervision.
      // We only probe after the expected duration.
      if (state.desired_duration > 0) {
        // Start an alleged transition.
        attribute_start_transition(value_node, zwave_desired_duration);
        // Assume final state until the next probe
        set_command_status_value(state_node, FINAL_STATE, FINAL_STATE);

        // Probe again after this duration
        attribute_timeout_set_callback(state_node,
                                       zwave_desired_duration + PROBE_BACK_OFF,
                                       &attribute_store_undefine_reported);

      } else {
        // It was an instantaneous change, check if the node is happy:
        attribute_store_undefine_reported(state_node);
      }

      break;
    case FRAME_SENT_EVENT_OK_SUPERVISION_NO_SUPPORT:
    case FRAME_SENT_EVENT_OK_SUPERVISION_FAIL:
    default:
      // Ensure any transition is stopped
      attribute_stop_transition(value_node);
      // Probe again, see what the node is up to.
      attribute_store_undefine_reported(state_node);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Store callback functions
///////////////////////////////////////////////////////////////////////////////
static void zwave_command_class_switch_multilevel_on_version_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  if (is_zwave_command_class_filtered_for_root_device(
        COMMAND_CLASS_SWITCH_MULTILEVEL_V4,
        updated_node)
      == true) {
    return;
  }

  // Do not create the attributes until we are sure of the version
  zwave_cc_version_t supporting_node_version = 0;
  attribute_store_get_reported(updated_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  if (supporting_node_version == 0) {
    // Wait that the version becomes non-zero.
    return;
  }

  // Now we know we have a Multilevel Switch supporting endpoint.
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  // Create the v3 attributes first, so it gets resolved first
  if (supporting_node_version >= 3) {
    attribute_store_add_if_missing(endpoint_node,
                                   v3_attributes,
                                   COUNT_OF(v3_attributes));
  }

  if (supporting_node_version >= 1) {
    attribute_store_node_t state_node
      = attribute_store_get_first_child_by_type(endpoint_node,
                                                ATTRIBUTE(STATE));
    if (state_node == ATTRIBUTE_STORE_INVALID_NODE) {
      state_node = attribute_store_add_node(ATTRIBUTE(STATE), endpoint_node);
    }
    attribute_store_add_if_missing(state_node,
                                   v1_sub_attributes,
                                   COUNT_OF(v1_sub_attributes));
  }
}

/**
 * @brief Sets the state node desired/reported values to match the sub-state
 */
static void update_node_state(attribute_store_node_t state_node)
{
  attribute_store_node_t duration_node
    = attribute_store_get_first_child_by_type(state_node, ATTRIBUTE(DURATION));
  attribute_store_node_t value_node
    = attribute_store_get_first_child_by_type(state_node, ATTRIBUTE(VALUE));

  if (!attribute_store_is_value_defined(duration_node, REPORTED_ATTRIBUTE)
      || !attribute_store_is_value_defined(value_node, REPORTED_ATTRIBUTE)) {
    // Undefine the state as well, so the state gets resolved.
    attribute_store_undefine_desired(state_node);
    attribute_store_undefine_reported(state_node);
    return;
  }
}

static void zwave_command_class_switch_multilevel_on_reported_sub_state_update(
  attribute_store_node_t node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  attribute_store_node_t state_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE(STATE));

  update_node_state(state_node);
}

static void zwave_command_class_switch_multilevel_on_desired_sub_state_update(
  attribute_store_node_t node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  // If one of the sub-state desired value just changed and it's mismatched with
  // the reported, meaning that we want to issue a new command.
  if (attribute_store_is_value_defined(node, DESIRED_ATTRIBUTE)
      && !attribute_store_is_value_matched(node)) {
    attribute_store_node_t state_node
      = attribute_store_get_first_parent_with_type(node, ATTRIBUTE(STATE));
    set_command_status_value(state_node, FINAL_STATE, NEEDS_ONE_COMMAND);
    attribute_store_walk_tree_with_return_value(state_node,
                                                &attribute_stop_transition);

    if (true == is_node_pending_set_resolution(state_node)) {
      sl_log_debug(LOG_TAG, "Restarting Set resolution on node %d", state_node);
      attribute_resolver_restart_set_resolution(state_node);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Incoming commands handler
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_switch_multilevel_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // Frame too short, it should have not come here.
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  switch (frame_data[COMMAND_INDEX]) {
    case SWITCH_MULTILEVEL_START_LEVEL_CHANGE:
      // Note: This command is not supported, but we want to indicate
      // to the application that we received it.
      return zwave_command_class_switch_multilevel_handle_start_level_change(
        connection_info,
        frame_data,
        frame_length);
    case SWITCH_MULTILEVEL_STOP_LEVEL_CHANGE:
      // Note: This command is not supported, but we want to indicate
      // to the application that we received it.
      return zwave_command_class_switch_multilevel_handle_stop_level_change(
        connection_info,
        frame_data,
        frame_length);
    case SWITCH_MULTILEVEL_SET:
      // Note: This command is not supported, but we want to indicate
      // to the application that we received it.
      return zwave_command_class_switch_multilevel_handle_set(connection_info,
                                                              frame_data,
                                                              frame_length);

    case SWITCH_MULTILEVEL_REPORT_V4:
      return zwave_command_class_switch_multilevel_handle_report(
        connection_info,
        frame_data,
        frame_length);

    case SWITCH_MULTILEVEL_SUPPORTED_REPORT_V4:
      return zwave_command_class_switch_multilevel_handle_supported_report(
        connection_info,
        frame_data,
        frame_length);

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_switch_multilevel_init()
{
  // Attribute resolver rules
  attribute_resolver_register_rule(ATTRIBUTE(STATE),
                                   zwave_command_class_switch_multilevel_set,
                                   zwave_command_class_switch_multilevel_get);

  attribute_resolver_register_rule(
    ATTRIBUTE(CAPABILITIES_REQUESTED),
    NULL,
    zwave_command_class_switch_multilevel_supported_get);

  // Attribute store callbacks
  attribute_store_register_callback_by_type(
    zwave_command_class_switch_multilevel_on_version_attribute_update,
    ATTRIBUTE(VERSION));

  // Listening to updates for the nodes under the state.
  attribute_store_register_callback_by_type_and_state(
    zwave_command_class_switch_multilevel_on_reported_sub_state_update,
    ATTRIBUTE(DURATION),
    REPORTED_ATTRIBUTE);

  attribute_store_register_callback_by_type_and_state(
    zwave_command_class_switch_multilevel_on_reported_sub_state_update,
    ATTRIBUTE(VALUE),
    REPORTED_ATTRIBUTE);

  // Listening to updates for the nodes under the state.
  attribute_store_register_callback_by_type_and_state(
    zwave_command_class_switch_multilevel_on_desired_sub_state_update,
    ATTRIBUTE(DURATION),
    DESIRED_ATTRIBUTE);

  attribute_store_register_callback_by_type_and_state(
    zwave_command_class_switch_multilevel_on_desired_sub_state_update,
    ATTRIBUTE(VALUE),
    DESIRED_ATTRIBUTE);

  // Attribute Resolver event listener:
  register_send_event_handler(ATTRIBUTE(STATE), &on_state_send_data_complete);

  // The support side of things: Register our handler to the Z-Wave CC framework:
  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler
    = &zwave_command_class_switch_multilevel_control_handler;
  // Not supported, so this does not really matter
  handler.minimal_scheme             = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.manual_security_validation = false;
  handler.command_class              = COMMAND_CLASS_SWITCH_MULTILEVEL_V4;
  handler.version                    = SWITCH_MULTILEVEL_VERSION_V4;
  handler.command_class_name         = "Multilevel Switch";
  handler.comments                   = "";

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}
