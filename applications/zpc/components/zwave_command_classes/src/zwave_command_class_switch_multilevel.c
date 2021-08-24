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
#include "zwave_command_class_switch_multilevel.h"
#include "zwave_command_classes_utils.h"

// Generic includes
#include <stdlib.h>
#include <assert.h>

// Includes from other ZPC Components
#include "zwave_controller_command_class_indices.h"
#include "zwave_unid.h"
#include "zwave_command_handler.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"
#include "zpc_attribute_resolver.h"

// Includes from other UIC Components
#include "attribute_store_helper.h"
#include "attribute_resolver.h"
#include "attribute_transitions.h"
#include "attribute_timeouts.h"
#include "sl_log.h"

// Log tag
#define LOG_TAG "zwave_command_class_switch_multilevel"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_MULTILEVEL_SWITCH_##type

// Constant values for our OnOff virtual attribute, taken from the OnOff ZCL cluster
#define ON  0xFF
#define OFF 0x00

// Constant values for our Multilevel Switch
#define MAX_LEVEL 0x63
#define MIN_LEVEL 0x00

///< Indicates which of the sub-states attributes if modified when the
///< last command is executed.
///> The last prepared command payload intends to modify the ATTRIBUTE(ON_OFF)
///> of the node
#define SUBSTATE_ON_OFF 1 << 0
///> The last prepared command payload intends to modify the ATTRIBUTE(VALUE)
///> of the node
#define SUBSTATE_VALUE 1 << 1
///> The last prepared command payload intends to modify the ATTRIBUTE(DURATION)
///> of the node
#define SUBSTATE_DURATION 1 << 2

///< Values for the ATTRIBUTE(STATE) of the multilevel Switch
typedef enum {
  FINAL_STATE             = 0,
  NEEDS_ONE_COMMAND       = 1,
  NEEDS_MULTIPLE_COMMANDS = 2,
} state_values_t;

/**
 * @brief Full state of a multilevel Switch supporting node.
 */
typedef struct multilevel_switch_state {
  state_values_t desired_state;
  state_values_t reported_state;
  uint32_t desired_value;
  uint32_t reported_value;
  uint32_t desired_duration;
  uint32_t reported_duration;
  uint32_t desired_on_off;
  uint32_t reported_on_off;
  uint8_t desired_substate_update;
} multilevel_switch_state_t;

static const attribute_store_type_t v1_sub_attributes[]
  = {ATTRIBUTE(VALUE),
     ATTRIBUTE(DURATION),
     ATTRIBUTE(ON_OFF),
     ATTRIBUTE(SUBSTATE_UPDATE)};

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
                      multilevel_switch_state_t *state)
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

  attribute_store_node_t on_off_node
    = attribute_store_get_node_child_by_type(state_node, ATTRIBUTE(ON_OFF), 0);

  attribute_store_read_value(on_off_node,
                             DESIRED_OR_REPORTED_ATTRIBUTE,
                             (uint8_t *)&state->desired_on_off,
                             sizeof(uint32_t));
  attribute_store_read_value(on_off_node,
                             REPORTED_ATTRIBUTE,
                             (uint8_t *)&state->reported_on_off,
                             sizeof(uint32_t));

  attribute_store_node_t substate_update_node
    = attribute_store_get_node_child_by_type(state_node,
                                             ATTRIBUTE(SUBSTATE_UPDATE),
                                             0);

  attribute_store_read_value(substate_update_node,
                             DESIRED_ATTRIBUTE,
                             &state->desired_substate_update,
                             sizeof(uint8_t));
}

static void set_desired_on_off(attribute_store_node_t state_node,
                               uint32_t on_off)
{
  attribute_store_node_t on_off_node
    = attribute_store_get_node_child_by_type(state_node, ATTRIBUTE(ON_OFF), 0);

  attribute_store_set_node_attribute_value(on_off_node,
                                           DESIRED_ATTRIBUTE,
                                           (uint8_t *)&on_off,
                                           sizeof(on_off));
}

static void set_reported_on_off(attribute_store_node_t state_node,
                                uint32_t on_off)
{
  attribute_store_node_t on_off_node
    = attribute_store_get_node_child_by_type(state_node, ATTRIBUTE(ON_OFF), 0);

  attribute_store_set_node_attribute_value(on_off_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&on_off,
                                           sizeof(on_off));
}

static void set_desired_value(attribute_store_node_t state_node, uint32_t value)
{
  attribute_store_node_t value_node
    = attribute_store_get_node_child_by_type(state_node, ATTRIBUTE(VALUE), 0);

  attribute_store_set_node_attribute_value(value_node,
                                           DESIRED_ATTRIBUTE,
                                           (uint8_t *)&value,
                                           sizeof(value));
}

static void set_reported_value(attribute_store_node_t state_node,
                               uint32_t value)
{
  attribute_store_node_t value_node
    = attribute_store_get_node_child_by_type(state_node, ATTRIBUTE(VALUE), 0);

  attribute_store_set_node_attribute_value(value_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&value,
                                           sizeof(value));
}

static void set_desired_duration(attribute_store_node_t state_node,
                                 uint32_t duration)
{
  attribute_store_node_t duration_node
    = attribute_store_get_node_child_by_type(state_node,
                                             ATTRIBUTE(DURATION),
                                             0);

  attribute_store_set_node_attribute_value(duration_node,
                                           DESIRED_ATTRIBUTE,
                                           (uint8_t *)&duration,
                                           sizeof(duration));
}

static void set_reported_duration(attribute_store_node_t state_node,
                                  uint32_t duration)
{
  attribute_store_node_t duration_node
    = attribute_store_get_node_child_by_type(state_node,
                                             ATTRIBUTE(DURATION),
                                             0);

  attribute_store_set_node_attribute_value(duration_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&duration,
                                           sizeof(duration));
}

static void set_desired_substate_update(attribute_store_node_t state_node,
                                        uint8_t substate_update)
{
  attribute_store_node_t substate_update_node
    = attribute_store_get_node_child_by_type(state_node,
                                             ATTRIBUTE(SUBSTATE_UPDATE),
                                             0);

  attribute_store_set_node_attribute_value(substate_update_node,
                                           DESIRED_ATTRIBUTE,
                                           &substate_update,
                                           sizeof(substate_update));
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
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(VERSION),
                                             0);
  attribute_store_node_t value_node
    = attribute_store_get_node_child_by_type(node, ATTRIBUTE(VALUE), 0);

  uint8_t supporting_node_version = 0;
  attribute_store_read_value(version_node,
                             REPORTED_ATTRIBUTE,
                             &supporting_node_version,
                             sizeof(supporting_node_version));

  // Are we modifying a transition target in favor of a new one
  // without changing the duration ? In this case, adjust
  // the desired duration to what's remaining in the transition.
  clock_time_t remaining_duration
    = attribute_transition_get_remaining_duration(value_node);

  if (remaining_duration != 0
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

  // If we just switch on during a virtual transition, we will use 2 commands:
  // Set at the current virtual level, then Set to start dimming with the
  // remainder of the duration
  if (state.reported_on_off == OFF && state.desired_on_off == ON
      && state.desired_duration != 0 && remaining_duration > 0) {
    sl_log_debug(LOG_TAG, "Set ON during a virtual transition");
    set_frame->value           = state.reported_value;
    set_frame->dimmingDuration = 0;
    set_desired_substate_update(node, SUBSTATE_ON_OFF);
    set_state_value(node, FINAL_STATE, NEEDS_MULTIPLE_COMMANDS);
    return SL_STATUS_IN_PROGRESS;
  }

  // Now the case where we switch off during a transition:
  if (state.reported_on_off == ON && state.desired_on_off == OFF
      && state.desired_duration != 0 && remaining_duration > 0) {
    sl_log_debug(LOG_TAG, "Set OFF during a transition");
    set_frame->value           = 0;
    set_frame->dimmingDuration = 0;
    set_desired_substate_update(node, SUBSTATE_ON_OFF);
    return SL_STATUS_OK;
  }

  // Are we OFF and we need to make or modify virtual transition ?
  if (state.desired_on_off == OFF && state.reported_on_off == OFF
      && state.desired_duration > 0
      && state.desired_duration != state.reported_duration) {
    sl_log_debug(LOG_TAG,
                 "Transition change while OFF. Starting virtual transition");
    // start a virtual transition
    clock_time_t duration_time
      = zwave_duration_to_time((uint8_t)state.desired_duration);
    attribute_start_transition(value_node, duration_time);
    set_state_value(node, FINAL_STATE, FINAL_STATE);  // Prevent re-resolutions
    set_reported_duration(node, state.desired_duration);
    set_desired_substate_update(node, 0);
    *frame_length = 0;
    return SL_STATUS_ALREADY_EXISTS;
  }

  // Any other case, we send a Set Command, even if the device is in an equivalent state.
  // Capture the intent of the command that we are about to send.
  //sl_log_debug(LOG_TAG, "Regular set command");
  set_frame->value
    = state.desired_on_off == OFF ? OFF : (uint8_t)state.desired_value;
  set_frame->dimmingDuration = (uint8_t)state.desired_duration;
  set_desired_substate_update(
    node,
    (SUBSTATE_VALUE | SUBSTATE_ON_OFF | SUBSTATE_DURATION));
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Frame parsing functions
///////////////////////////////////////////////////////////////////////////////
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
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(STATE),
                                             0);
  attribute_store_node_t value_node
    = attribute_store_get_node_child_by_type(state_node, ATTRIBUTE(VALUE), 0);
  attribute_store_node_t on_off_node
    = attribute_store_get_node_child_by_type(state_node, ATTRIBUTE(ON_OFF), 0);

  multilevel_switch_state_t state = {};
  get_state(state_node, &state);

  uint32_t current_value = frame_data[REPORT_VALUE_INDEX];
  if (current_value == 0xFF) {  // 0xFF is deprecated and means 100%.
    current_value = MAX_LEVEL;
  }

  if (current_value != MIN_LEVEL) {
    set_reported_on_off(state_node, ON);
  } else if (false
             == attribute_store_is_value_defined(on_off_node,
                                                 REPORTED_ATTRIBUTE)) {
    set_reported_on_off(state_node, ON);
  }
  set_reported_value(state_node, current_value);

  // Align back the desired if no transition ongoing
  if (is_attribute_transition_ongoing(value_node) == false) {
    if (current_value != MIN_LEVEL) {
      set_desired_on_off(state_node, ON);
    }
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
    if (target_value != current_value) {
      // The node just told it it is in a transition that we do not know about.
      set_desired_value(state_node, target_value);
    }
  }
  if (duration == 0) {
    // Ensure no mismatch between reported/desired if duration is 0.
    attribute_store_set_reported_as_desired(value_node);
  }
  if (duration > 0 && false == is_attribute_transition_ongoing(value_node)) {
    clock_time_t clock_duration = zwave_duration_to_time((uint8_t)duration);
    attribute_start_transition(value_node, clock_duration);
    // Probe again after that duration.
    attribute_timeout_set_callback(state_node,
                                   clock_duration + PROBE_BACK_OFF,
                                   &attribute_store_undefine_reported);
  }

  // Set the reported of the state node, so that it does not "get" again.
  set_state_value(state_node, FINAL_STATE, FINAL_STATE);

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

  attribute_store_node_t capabilities_node
    = attribute_store_get_node_child_by_type(endpoint_node,
                                             ATTRIBUTE(CAPABILITIES_REQUESTED),
                                             0);

  bool capabilities_requested = true;
  attribute_store_set_node_attribute_value(capabilities_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&capabilities_requested,
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
    = attribute_store_get_node_child_by_type(state_node, ATTRIBUTE(VALUE), 0);

  switch (event) {
    case FRAME_SENT_EVENT_OK_SUPERVISION_WORKING:
      // Initiate a transition for the value, following the duration
      if (state.desired_substate_update & SUBSTATE_DURATION) {
        set_reported_duration(state_node, state.desired_duration);
      }
      if (state.desired_substate_update & SUBSTATE_ON_OFF) {
        set_reported_on_off(state_node, state.desired_on_off);
      }

      // Start our transition for the first WORKING status.
      if (!is_attribute_transition_ongoing(value_node)) {
        attribute_start_transition(
          value_node,
          zwave_duration_to_time((uint8_t)state.desired_duration));
      }
      break;

    case FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS:
      // Successfully arrived at the target value.
      if (state.desired_substate_update & SUBSTATE_VALUE) {
        set_reported_value(state_node, state.desired_value);
      }
      if (state.desired_substate_update & SUBSTATE_ON_OFF) {
        set_reported_on_off(state_node, state.desired_on_off);
      }

      // After a success, we verify if there are still desired mismatches
      //(in case of multiple frames resolution)
      if (state.desired_state == NEEDS_ONE_COMMAND
          || state.desired_state == FINAL_STATE) {
        // Success only means that a transition is over. Set the duration to 0.
        attribute_stop_transition(value_node);
        attribute_store_set_reported_as_desired(value_node);
        set_reported_duration(state_node, 0);
        set_desired_duration(state_node, 0);
        set_state_value(state_node, FINAL_STATE, FINAL_STATE);

        // We do not trust our transition predictions, let's probe again.
        if (state.reported_duration > 0
            && state.desired_substate_update != SUBSTATE_ON_OFF) {
          attribute_store_undefine_reported(state_node);
        }
      } else if (state.desired_state == NEEDS_MULTIPLE_COMMANDS) {
        set_state_value(state_node, FINAL_STATE, NEEDS_ONE_COMMAND);
      }
      break;

    case FRAME_SENT_EVENT_OK_NO_SUPERVISION:
      // We assumed it worked in case of no-supervision.
      // We only probe after the expected duration.
      if (state.desired_substate_update & SUBSTATE_DURATION) {
        set_reported_duration(state_node, state.desired_duration);
      }
      if (state.desired_substate_update & SUBSTATE_ON_OFF) {
        set_reported_on_off(state_node, state.desired_on_off);
      }

      if (state.desired_duration != 0) {
        // Start an alleged transition.
        attribute_start_transition(
          value_node,
          zwave_duration_to_time((uint8_t)state.desired_duration));
        // Assume final state until the next probe
        set_state_value(state_node, FINAL_STATE, FINAL_STATE);

        // Probe again after this duration
        attribute_timeout_set_callback(
          state_node,
          zwave_duration_to_time((uint8_t)state.desired_duration)
            + PROBE_BACK_OFF,
          &attribute_store_undefine_reported);

      } else if (state.desired_substate_update & SUBSTATE_VALUE) {
        // It was an instantaneous change.
        if (state.desired_on_off == ON
            && (state.desired_substate_update & SUBSTATE_DURATION)) {
          // Probe back if it is a not offline
          attribute_store_undefine_reported(state_node);
        } else {
          // Else just assume that it worked, we will check on the state
          // when it gets online again.
          set_reported_duration(state_node, state.desired_duration);
          set_state_value(state_node, FINAL_STATE, FINAL_STATE);
        }
      }

      break;
    case FRAME_SENT_EVENT_OK_SUPERVISION_NO_SUPPORT:
    case FRAME_SENT_EVENT_OK_SUPERVISION_FAIL:
    default:
      // Ensure any transition is stopped
      attribute_stop_transition(value_node);
      // Roll back desired to the reported.
      if (state.desired_substate_update & SUBSTATE_VALUE) {
        set_desired_value(state_node, state.reported_value);
      }
      if (state.desired_substate_update & SUBSTATE_ON_OFF) {
        set_desired_on_off(state_node, state.reported_on_off);
      }
      if (state.desired_substate_update & SUBSTATE_DURATION) {
        set_reported_duration(state_node, 0);
        set_desired_duration(state_node, 0);
      }
      // Probe again, see what the node is up to.
      attribute_store_undefine_reported(state_node);
      break;
  }

  set_desired_substate_update(state_node, 0);
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

  // Confirm that we have a version attribute update
  assert(ATTRIBUTE(VERSION) == attribute_store_get_node_type(updated_node));

  // Do not create the attributes until we are sure of the version
  uint8_t supporting_node_version = 0;
  attribute_store_read_value(updated_node,
                             REPORTED_ATTRIBUTE,
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
      = attribute_store_get_node_child_by_type(endpoint_node,
                                               ATTRIBUTE(STATE),
                                               0);
    if (state_node == ATTRIBUTE_STORE_INVALID_NODE) {
      state_node = attribute_store_add_node(ATTRIBUTE(STATE), endpoint_node);
    }
    attribute_store_add_if_missing(state_node,
                                   v1_sub_attributes,
                                   COUNT_OF(v1_sub_attributes));
    // OnOff does not always get resolved by incoming reports.
    // Ensure that we set it to a default value.
    attribute_store_set_uint32_child_by_type(state_node,
                                             ATTRIBUTE(ON_OFF),
                                             REPORTED_ATTRIBUTE,
                                             ON);
    attribute_store_set_uint32_child_by_type(state_node,
                                             ATTRIBUTE(SUBSTATE_UPDATE),
                                             DESIRED_ATTRIBUTE,
                                             0);
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

  attribute_store_node_t on_off_node
    = attribute_store_get_node_child_by_type(state_node, ATTRIBUTE(ON_OFF), 0);

  if (!attribute_store_is_value_defined(duration_node, REPORTED_ATTRIBUTE)
      || !attribute_store_is_value_defined(value_node, REPORTED_ATTRIBUTE)
      || !attribute_store_is_value_defined(on_off_node, REPORTED_ATTRIBUTE)) {
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
sl_status_t zwave_command_class_switch_multilevel_control_handler(
  const zwave_controller_connection_info_t *connection_info,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  // Frame too short, it should have not come here.
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  assert(frame_data[COMMAND_CLASS_INDEX] == COMMAND_CLASS_SWITCH_MULTILEVEL_V4);

  switch (frame_data[COMMAND_INDEX]) {
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

  attribute_store_register_callback_by_type_and_state(
    zwave_command_class_switch_multilevel_on_reported_sub_state_update,
    ATTRIBUTE(ON_OFF),
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

  attribute_store_register_callback_by_type_and_state(
    zwave_command_class_switch_multilevel_on_desired_sub_state_update,
    ATTRIBUTE(ON_OFF),
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
  handler.command_class_name         = "Switch Multilevel";
  handler.comments                   = "Partial control: <br>"
                     "1. we do not use start/stop level change.<br>"
                     "2. we do not support the 0xFF duration";

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}
