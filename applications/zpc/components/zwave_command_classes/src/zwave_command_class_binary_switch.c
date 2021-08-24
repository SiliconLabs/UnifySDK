/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "zwave_command_class_binary_switch.h"
#include "zwave_command_classes_utils.h"

// Generic includes
#include <stdlib.h>
#include <assert.h>

// Includes from other ZPC Components
#include "zwave_controller_command_class_indices.h"
#include "zwave_command_handler.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"
#include "zpc_attribute_resolver.h"

// Includes from other UIC Components
#include "attribute_store_helper.h"
#include "attribute_resolver.h"
#include "attribute_timeouts.h"
#include "sl_log.h"

// Log tag
#define LOG_TAG "zwave_command_class_binary_switch"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_##type

///< Values for the ATTRIBUTE(STATE) of the binary Switch
typedef enum {
  FINAL_STATE       = 0,
  NEEDS_ONE_COMMAND = 1,
} state_values_t;

/**
 * @brief Full state of a binary Switch supporting node.
 */
typedef struct binary_switch_state {
  state_values_t desired_state;
  state_values_t reported_state;
  uint32_t desired_value;
  uint32_t reported_value;
  uint32_t desired_duration;
  uint32_t reported_duration;
} binary_switch_state_t;

// Version 1 attributes under the state
static const attribute_store_type_t v1_sub_attributes[] = {ATTRIBUTE(VALUE)};
// Version 2 attributes under the state
static const attribute_store_type_t v2_sub_attributes[] = {ATTRIBUTE(DURATION)};

// Frame parsing indices:
#define REPORT_VALUE_INDEX        2
#define REPORT_TARGET_VALUE_INDEX 3
#define REPORT_DURATION_INDEX     4

// Constants
#define ON  0xFF
#define OFF 0x00

///////////////////////////////////////////////////////////////////////////////
// Local helper functions
///////////////////////////////////////////////////////////////////////////////
static void set_state_value(attribute_store_node_t state_node,
                            state_values_t reported,
                            state_values_t desired)
{
  attribute_store_set_node_attribute_value(state_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&reported,
                                           sizeof(state_values_t));
  attribute_store_set_node_attribute_value(state_node,
                                           DESIRED_ATTRIBUTE,
                                           (uint8_t *)&desired,
                                           sizeof(state_values_t));
}

static void get_state(attribute_store_node_t state_node,
                      binary_switch_state_t *state)
{
  attribute_store_read_value(state_node,
                             DESIRED_OR_REPORTED_ATTRIBUTE,
                             (uint8_t *)&state->desired_state,
                             sizeof(state_values_t));
  attribute_store_read_value(state_node,
                             REPORTED_ATTRIBUTE,
                             (uint8_t *)&state->reported_state,
                             sizeof(state_values_t));

  attribute_store_node_t duration_node
    = attribute_store_get_node_child_by_type(state_node,
                                             ATTRIBUTE(DURATION),
                                             0);
  // Default duration, if v1
  state->desired_duration  = 0;
  state->reported_duration = 0;
  attribute_store_read_value(duration_node,
                             DESIRED_OR_REPORTED_ATTRIBUTE,
                             (uint8_t *)&state->desired_duration,
                             sizeof(uint32_t));
  attribute_store_read_value(duration_node,
                             REPORTED_ATTRIBUTE,
                             (uint8_t *)&state->reported_duration,
                             sizeof(uint32_t));

  attribute_store_node_t value_node
    = attribute_store_get_node_child_by_type(state_node, ATTRIBUTE(VALUE), 0);

  attribute_store_read_value(value_node,
                             DESIRED_OR_REPORTED_ATTRIBUTE,
                             (uint8_t *)&state->desired_value,
                             sizeof(uint32_t));
  attribute_store_read_value(value_node,
                             REPORTED_ATTRIBUTE,
                             (uint8_t *)&state->reported_value,
                             sizeof(uint32_t));
}

static void set_desired_value(attribute_store_node_t state_node, uint32_t value)
{
  attribute_store_node_t value_node
    = attribute_store_get_node_child_by_type(state_node, ATTRIBUTE(VALUE), 0);

  attribute_store_set_desired(value_node, &value, sizeof(value));
}

static void set_reported_value(attribute_store_node_t state_node,
                               uint32_t value)
{
  attribute_store_node_t value_node
    = attribute_store_get_node_child_by_type(state_node, ATTRIBUTE(VALUE), 0);

  attribute_store_set_reported(value_node, &value, sizeof(value));
}

static void set_desired_duration(attribute_store_node_t state_node,
                                 uint32_t duration)
{
  attribute_store_node_t duration_node
    = attribute_store_get_node_child_by_type(state_node,
                                             ATTRIBUTE(DURATION),
                                             0);

  attribute_store_set_desired(duration_node, &duration, sizeof(duration));
}

static void set_reported_duration(attribute_store_node_t state_node,
                                  uint32_t duration)
{
  attribute_store_node_t duration_node
    = attribute_store_get_node_child_by_type(state_node,
                                             ATTRIBUTE(DURATION),
                                             0);

  attribute_store_set_reported(duration_node, &duration, sizeof(duration));
}

///////////////////////////////////////////////////////////////////////////////
// Resolution functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_binary_switch_get(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  zwave_minimum_frame_t *get_frame = (zwave_minimum_frame_t *)frame;
  get_frame->command_class         = COMMAND_CLASS_SWITCH_BINARY_V2;
  get_frame->command               = SWITCH_BINARY_GET_V2;
  *frame_length                    = sizeof(zwave_minimum_frame_t);
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_class_binary_switch_set(
  attribute_store_node_t node, uint8_t *frame, uint16_t *frame_length)
{
  // Setting the state to 0 here, so that if we have a v1 node without duration,
  // it sets this to 0.
  binary_switch_state_t state = {};
  get_state(node, &state);

  // For simplicity, we avoid using Start level Change / Stop level change.
  ZW_SWITCH_BINARY_SET_V2_FRAME *set_frame
    = (ZW_SWITCH_BINARY_SET_V2_FRAME *)frame;
  set_frame->cmdClass    = COMMAND_CLASS_SWITCH_BINARY_V2;
  set_frame->cmd         = SWITCH_BINARY_SET_V2;
  set_frame->targetValue = state.desired_value;
  set_frame->duration    = state.desired_duration;
  *frame_length          = sizeof(ZW_SWITCH_BINARY_SET_V2_FRAME);
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Frame parsing functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t zwave_command_class_binary_switch_handle_report(
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
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(STATE),
                                             0);
  attribute_store_node_t value_node
    = attribute_store_get_node_child_by_type(state_node, ATTRIBUTE(VALUE), 0);

  binary_switch_state_t state = {};
  get_state(state_node, &state);

  uint32_t current_value = frame_data[REPORT_VALUE_INDEX];
  // Nodes should only report 0x00, 0xFE or 0xFF, but in case somebody
  // did something funny, we accept values 1..100
  if (current_value >= 1 && current_value <= 100) {
    current_value = ON;
  }
  if ((current_value == OFF) || (current_value == ON)) {
    set_reported_value(state_node, current_value);
  }

  // Duration:
  uint32_t duration = 0;  // Assumed value if no duration is provided.
  if (frame_length > REPORT_DURATION_INDEX) {
    duration = frame_data[REPORT_DURATION_INDEX];
    set_reported_duration(state_node, duration);
    set_desired_duration(state_node, duration);
  }

  // Is there a target value:
  if (frame_length > REPORT_TARGET_VALUE_INDEX) {
    uint32_t target_value = frame_data[REPORT_TARGET_VALUE_INDEX];
    // Same as for current value, sanitize the range.
    if (target_value >= 1 && target_value <= 100) {
      target_value = ON;
    }
    if ((target_value == OFF) || (target_value == ON)) {
      set_desired_value(state_node, target_value);
    }
  }

  // Current/target value adjustments based on the reported duration.
  if (duration == 0) {
    // Ensure no mismatch between reported/desired if duration is 0.
    if (attribute_store_is_value_defined(value_node, DESIRED_ATTRIBUTE)) {
      attribute_store_set_reported_as_desired(value_node);
    }
  }
  if (duration > 0) {
    // here we want to probe again after that duration.
    attribute_timeout_set_callback(state_node,
                                   zwave_duration_to_time((uint8_t)duration)
                                     + PROBE_BACK_OFF,
                                   &attribute_store_undefine_reported);
  }

  // Set the reported of the state node, so that it does not "get" again.
  set_state_value(state_node, FINAL_STATE, FINAL_STATE);

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

  binary_switch_state_t state = {};
  get_state(state_node, &state);

  switch (event) {
    case FRAME_SENT_EVENT_OK_SUPERVISION_WORKING:
      // Duration was accepted, but not the value
      set_reported_duration(state_node, state.desired_duration);
      // Wait for subsequent callbacks now.
      break;

    case FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS:
      // Successfully arrived at the target value.
      set_reported_value(state_node, state.desired_value);
      set_reported_duration(state_node, state.desired_duration);

      // After a success, no more commands to send.
      set_state_value(state_node, FINAL_STATE, FINAL_STATE);
      break;

    case FRAME_SENT_EVENT_OK_NO_SUPERVISION:
      // We assumed it worked in case of no-supervision.
      // We only probe after the expected duration.
      set_reported_duration(state_node, state.desired_duration);
      // Probe again after the duration (can also be 0)
      attribute_timeout_set_callback(
        state_node,
        PROBE_BACK_OFF
          + zwave_duration_to_time((uint8_t)state.desired_duration),
        &attribute_store_undefine_reported);

      break;

    case FRAME_SENT_EVENT_OK_SUPERVISION_NO_SUPPORT:
    case FRAME_SENT_EVENT_OK_SUPERVISION_FAIL:
    default:
      // Roll back desired to the reported.
      set_desired_value(state_node, state.reported_value);
      set_desired_duration(state_node, state.reported_duration);
      // Probe again, see what the node is up to.
      attribute_store_undefine_reported(state_node);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Store callback functions
///////////////////////////////////////////////////////////////////////////////
static void zwave_command_class_binary_switch_on_version_attribute_update(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  if (is_zwave_command_class_filtered_for_root_device(
        COMMAND_CLASS_SWITCH_BINARY_V2,
        updated_node)
      == true) {
    return;
  }

  // Confirm that we have a version attribute update
  assert(ATTRIBUTE(VERSION) == attribute_store_get_node_type(updated_node));

  // Do not create the attributes until we are sure of the version
  uint8_t supporting_node_version = 0;
  attribute_store_get_reported(updated_node,
                               &supporting_node_version,
                               sizeof(supporting_node_version));

  if (supporting_node_version == 0) {
    // Wait that the version becomes non-zero.
    return;
  }

  // Now we know we have a Binary Switch supporting endpoint.
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  if (supporting_node_version >= 1) {
    attribute_store_node_t state_node
      = attribute_store_get_node_child_by_type(endpoint_node,
                                               ATTRIBUTE(STATE),
                                               0);
    if (state_node == ATTRIBUTE_STORE_INVALID_NODE) {
      state_node = attribute_store_add_node(ATTRIBUTE(STATE), endpoint_node);
    }
    attribute_store_add_if_missing(state_node,
                                   v1_sub_attributes,
                                   COUNT_OF(v1_sub_attributes));

    if (supporting_node_version >= 2) {
      attribute_store_add_if_missing(state_node,
                                     v2_sub_attributes,
                                     COUNT_OF(v2_sub_attributes));
    }
  }
}

/**
 * @brief Sets the state node desired/reported values to match the sub-state
 */
static void update_node_state(attribute_store_node_t state_node)
{
  attribute_store_node_t duration_node
    = attribute_store_get_node_child_by_type(state_node,
                                             ATTRIBUTE(DURATION),
                                             0);
  attribute_store_node_t value_node
    = attribute_store_get_node_child_by_type(state_node, ATTRIBUTE(VALUE), 0);

  if (!attribute_store_is_value_defined(duration_node, REPORTED_ATTRIBUTE)
      || !attribute_store_is_value_defined(value_node, REPORTED_ATTRIBUTE)) {
    // Undefine the state as well, so the state gets resolved.
    attribute_store_undefine_desired(state_node);
    attribute_store_undefine_reported(state_node);
    return;
  }
}

static void zwave_command_class_binary_switch_on_reported_sub_state_update(
  attribute_store_node_t node, attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  attribute_store_node_t state_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE(STATE));

  update_node_state(state_node);
}

static void zwave_command_class_binary_switch_on_desired_sub_state_update(
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
    set_state_value(state_node, FINAL_STATE, NEEDS_ONE_COMMAND);

    if (true == is_node_pending_set_resolution(state_node)) {
      sl_log_debug(LOG_TAG, "Restarting Set resolution on node %d", state_node);
      attribute_resolver_restart_set_resolution(state_node);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// Incoming commands handler
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_binary_switch_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // Frame too short, it should have not come here.
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  assert(frame_data[COMMAND_CLASS_INDEX] == COMMAND_CLASS_SWITCH_BINARY_V2);

  switch (frame_data[COMMAND_INDEX]) {
    case SWITCH_BINARY_REPORT_V2:
      return zwave_command_class_binary_switch_handle_report(connection_info,
                                                             frame_data,
                                                             frame_length);
    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_class_binary_switch_init()
{
  // Attribute resolver rules
  attribute_resolver_register_rule(ATTRIBUTE(STATE),
                                   zwave_command_class_binary_switch_set,
                                   zwave_command_class_binary_switch_get);

  // Attribute store callbacks
  attribute_store_register_callback_by_type(
    zwave_command_class_binary_switch_on_version_attribute_update,
    ATTRIBUTE(VERSION));

  // Listening to updates for the nodes under the state.
  attribute_store_register_callback_by_type_and_state(
    zwave_command_class_binary_switch_on_reported_sub_state_update,
    ATTRIBUTE(DURATION),
    REPORTED_ATTRIBUTE);

  attribute_store_register_callback_by_type_and_state(
    zwave_command_class_binary_switch_on_reported_sub_state_update,
    ATTRIBUTE(VALUE),
    REPORTED_ATTRIBUTE);

  // Listening to updates for the nodes under the state.
  attribute_store_register_callback_by_type_and_state(
    zwave_command_class_binary_switch_on_desired_sub_state_update,
    ATTRIBUTE(DURATION),
    DESIRED_ATTRIBUTE);

  attribute_store_register_callback_by_type_and_state(
    zwave_command_class_binary_switch_on_desired_sub_state_update,
    ATTRIBUTE(VALUE),
    DESIRED_ATTRIBUTE);

  // Attribute Resolver event listener:
  register_send_event_handler(ATTRIBUTE(STATE), &on_state_send_data_complete);

  // The support side of things: Register our handler to the Z-Wave CC framework:
  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler = &zwave_command_class_binary_switch_control_handler;
  // Not supported, so this does not really matter
  handler.minimal_scheme             = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  handler.manual_security_validation = false;
  handler.command_class              = COMMAND_CLASS_SWITCH_BINARY_V2;
  handler.version                    = SWITCH_BINARY_VERSION_V2;
  handler.command_class_name         = "Binary Switch";
  handler.comments                   = "";

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}
