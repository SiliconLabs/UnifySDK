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

//Includes from this component
#include "zwave_command_class_wake_up.h"
#include "zwave_command_classes_utils.h"

// Includes from other components
#include "zwave_command_class_indices.h"
#include "zwave_controller_utils.h"
#include "attribute_store.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute_store_helper.h"
#include "zpc_attribute_store_network_helper.h"
#include "zwave_network_management.h"

// Unify components
#include "attribute_resolver.h"
#include "attribute_timeouts.h"
#include "zpc_attribute_resolver.h"
#include "zwave_command_handler.h"
#include "zwave_network_management.h"
#include "zwave_tx_scheme_selector.h"
#include "zwave_tx.h"
#include "zpc_config.h"
#include "sl_status.h"
#include "zwave_utils.h"

// Interfaces
#include "zwave_command_class_wake_up_types.h"
#include "ZW_classcmd.h"

// Generic includes
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "sl_log.h"
#define LOG_TAG "zwave_command_class_wake_up"

///////////////////////////////////////////////////////////////////////////////
// Private definitions
///////////////////////////////////////////////////////////////////////////////
// Attribute type shortcut
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_WAKE_UP_##type

// Frame parsing indices
#define WAKE_UP_ON_DEMAND_INDEX 14

// Constants
/// Bitmask for the Wake Up On demand capability
#define WAKE_UP_ON_DEMAND_BITMASK 1
/// Minimum possible Wake Up Interval
#define MINIMUM_WAKE_UP_INTERVAL 0
/// Maximum possible Wake Up Interval
#define MAXIMUM_WAKE_UP_INTERVAL 16777215
/// Default possible Wake Up Step
#define DEFAULT_WAKE_UP_STEP 1
/// Default capabilities bitmask (nothing is supported)
#define DEFAULT_WAKE_UP_CAPBILITIES_BITMASK 0

/// Timing to send a Wake Up No More (ms). The node falls asleep after 10s, so
/// no reasons to keep this frame more than e.g 15 seconds in the queue.
#define WAKE_UP_NO_MORE_DISCARD_TIMEOUT 15000
// Time to delay a Wake Up No More message
#define WAKE_UP_NO_MORE_DELAY 1000

static const attribute_store_type_t setting_node_type[] = {ATTRIBUTE(SETTING)};
static const attribute_store_type_t capabilities_node_type[]
  = {ATTRIBUTE(CAPABILITIES)};

static const attribute_store_type_t setting_attributes[]
  = {ATTRIBUTE(INTERVAL), ATTRIBUTE(NODE_ID)};

static const attribute_store_type_t capabilities_attributes[]
  = {ATTRIBUTE(MINIMUM_INTERVAL),
     ATTRIBUTE(MAXIMUM_INTERVAL),
     ATTRIBUTE(DEFAULT_INTERVAL),
     ATTRIBUTE(INTERVAL_STEP),
     ATTRIBUTE(CAPABILITIES_BITMASK)};

///////////////////////////////////////////////////////////////////////////////
// Prototypes
///////////////////////////////////////////////////////////////////////////////
static void send_wake_up_no_more(attribute_store_node_t node_id_node);

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Verifies if we are the Wake Up destination of a NodeID.
 *
 * It will search all endpoints for a Wake Up Configuration containing our
 * NodeID.
 *
 * @param node_id_node    The Attribute Store Node of the NodeID for which
 *                        we want to check if we are the Wake Up Destination
 * @returns true if we are the Wake Up Destination, false otherwise
 */
static bool we_are_the_wake_up_destination(attribute_store_node_t node_id_node)
{
  uint32_t endpoint_id_child_index = 0;
  attribute_store_node_t endpoint_id_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_ENDPOINT_ID,
                                             endpoint_id_child_index);

  zwave_node_id_t zpc_node_id = zwave_network_management_get_node_id();

  while (endpoint_id_node != ATTRIBUTE_STORE_INVALID_NODE) {
    // Find if there is a Wake Up Setting:
    // Set up the Wake Up Interval
    attribute_store_node_t wake_up_setting_node
      = attribute_store_get_first_child_by_type(endpoint_id_node,
                                                ATTRIBUTE(SETTING));
    attribute_store_node_t wake_up_node_id_node
      = attribute_store_get_first_child_by_type(wake_up_setting_node,
                                                ATTRIBUTE(NODE_ID));
    zwave_node_id_t configured_node_id = 0;
    attribute_store_get_reported(wake_up_node_id_node,
                                 &configured_node_id,
                                 sizeof(configured_node_id));

    if (configured_node_id == zpc_node_id) {
      return true;
    }

    // Move to the next Endpoint
    endpoint_id_child_index += 1;
    endpoint_id_node
      = attribute_store_get_node_child_by_type(node_id_node,
                                               ATTRIBUTE_ENDPOINT_ID,
                                               endpoint_id_child_index);
  }

  return false;
}

/**
 * @brief Verifies if either NodeID / Wake Up interval settings are still
 * undefined or mismatched and a resolution is needed
 *
 * If the wake_up_setting_node needs resolution, the wake_up_setting_node
 * value will be adjusted to either undefined value or NEEDS_ONE_COMMAND / FINAL_STATE
 * value mismatch, to trigger resolution.
 *
 * @param wake_up_setting_node    Attribute Store node for the Wake Up Setting.
 */
static void verify_if_wake_up_setting_needs_resolution(
  attribute_store_node_t wake_up_setting_node)
{
  // Do we need a Get ?
  attribute_store_node_t wake_up_node_id_node
    = attribute_store_get_first_child_by_type(wake_up_setting_node,
                                              ATTRIBUTE(NODE_ID));
  attribute_store_node_t wake_up_interval_node
    = attribute_store_get_first_child_by_type(wake_up_setting_node,
                                              ATTRIBUTE(INTERVAL));

  if (false
      == attribute_store_is_value_defined(wake_up_node_id_node,
                                          REPORTED_ATTRIBUTE)) {
    attribute_store_undefine_reported(wake_up_setting_node);
    return;
  }

  if (false
      == attribute_store_is_value_defined(wake_up_interval_node,
                                          REPORTED_ATTRIBUTE)) {
    attribute_store_undefine_reported(wake_up_setting_node);
    return;
  }

  // Do we need a Set?
  command_status_values_t state = FINAL_STATE;
  if ((attribute_store_is_value_defined(wake_up_node_id_node, DESIRED_ATTRIBUTE)
       == true)
      && (attribute_store_is_value_matched(wake_up_node_id_node) == false)) {
    // Set resolution needed. Set to FINAL_STATE/NEEDS_ONE_COMMAND
    attribute_store_set_desired(wake_up_setting_node, &state, sizeof(state));
    state = NEEDS_ONE_COMMAND;
    attribute_store_set_reported(wake_up_setting_node, &state, sizeof(state));
    return;
  }

  if ((attribute_store_is_value_defined(wake_up_interval_node,
                                        DESIRED_ATTRIBUTE)
       == true)
      && (attribute_store_is_value_matched(wake_up_interval_node) == false)) {
    // Set resolution needed. Set to FINAL_STATE/NEEDS_ONE_COMMAND
    attribute_store_set_desired(wake_up_setting_node, &state, sizeof(state));
    state = NEEDS_ONE_COMMAND;
    attribute_store_set_reported(wake_up_setting_node, &state, sizeof(state));
    return;
  }

  // No resolution needed. Set to FINAL_STATE/FINAL_STATE
  attribute_store_set_desired(wake_up_setting_node, &state, sizeof(state));
  attribute_store_set_reported(wake_up_setting_node, &state, sizeof(state));
}

/**
 * @brief Takes the 3 bytes of a wake_up_interval_t and saves it in the
 * attribute store.
 *
 * @param parent          The Attribute Store node under which the child with
 *                        the type reported value will be saved.
 * @param type            Type of the attribute store node to be saved under the
 *                        parent
 * @param interval_1_msb  MSB for the wake_up_interval_t. See the Command Class
 *                        specifications for details.
 * @param interval_2      Middle byte for the wake_up_interval_t.
 *                        See the Command Class specifications for details.
 * @param interval_3_lsb
 *                        specifications for details.
 *
 * @returns wake_up_interval_t value.
 */
static sl_status_t save_reported_interval(attribute_store_node_t parent,
                                          attribute_store_type_t type,
                                          uint8_t interval_1_msb,
                                          uint8_t interval_2,
                                          uint8_t interval_3_lsb)
{
  wake_up_interval_t interval
    = interval_1_msb << 16 | interval_2 << 8 | interval_3_lsb;
  return attribute_store_set_child_reported(parent,
                                            type,
                                            &interval,
                                            sizeof(interval));
}

/**
 * @brief Finds the value that would lead to the largest time between Wake Ups
 *        for a node
 *
 * @param endpoint_node  The Attribute Store node for the Endpoint ID which
 *                       which will be read.
 *
 * @returns wake_up_interval_t value.
 */
static wake_up_interval_t
  get_maximum_possible_interval(attribute_store_node_t endpoint_node)
{
  // Looks into the capabilities and check for the interval that will be the largest.
  // the value 0 is largest (infinite), Else we will fall back on MAX Interval.
  attribute_store_node_t wake_up_capabilities_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(CAPABILITIES));

  attribute_store_node_t minimum_interval_interval_node
    = attribute_store_get_first_child_by_type(wake_up_capabilities_node,
                                              ATTRIBUTE(MINIMUM_INTERVAL));
  wake_up_interval_t interval = 1;  // lowest non-zero.
  attribute_store_get_reported(minimum_interval_interval_node,
                               &interval,
                               sizeof(interval));
  if (interval == 0) {
    return 0;
  }

  attribute_store_node_t maximum_interval_interval_node
    = attribute_store_get_first_child_by_type(wake_up_capabilities_node,
                                              ATTRIBUTE(MAXIMUM_INTERVAL));
  attribute_store_get_reported(maximum_interval_interval_node,
                               &interval,
                               sizeof(interval));
  return interval;
}

/**
 * @brief Finds the closest supported value for a Wake Up Interval.
 *
 * Nodes support between Min and Max allowing certain steps. Not all values
 * are allowed.
 *
 * @param endpoint_node     The Attribute Store node for the Endpoint ID
 * @param wished_interval   The Interval that we wish to set
 *
 * @returns wake_up_interval_t value that can be set for the end node.
 */
static wake_up_interval_t
  get_closest_supported_interval(attribute_store_node_t endpoint_node,
                                 wake_up_interval_t wished_interval)
{
  // Read all the capabilities:
  wake_up_interval_t minimum_interval = MINIMUM_WAKE_UP_INTERVAL;
  wake_up_interval_t maximum_interval = MAXIMUM_WAKE_UP_INTERVAL;
  wake_up_interval_t interval_step    = DEFAULT_WAKE_UP_STEP;
  attribute_store_node_t wake_up_capabilities_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(CAPABILITIES));
  attribute_store_node_t minimum_interval_interval_node
    = attribute_store_get_first_child_by_type(wake_up_capabilities_node,
                                              ATTRIBUTE(MINIMUM_INTERVAL));
  attribute_store_get_reported(minimum_interval_interval_node,
                               &minimum_interval,
                               sizeof(minimum_interval));
  attribute_store_node_t maximum_interval_interval_node
    = attribute_store_get_first_child_by_type(wake_up_capabilities_node,
                                              ATTRIBUTE(MAXIMUM_INTERVAL));
  attribute_store_get_reported(maximum_interval_interval_node,
                               &maximum_interval,
                               sizeof(maximum_interval));

  attribute_store_node_t interval_step_node
    = attribute_store_get_first_child_by_type(wake_up_capabilities_node,
                                              ATTRIBUTE(INTERVAL_STEP));
  attribute_store_get_reported(interval_step_node,
                               &interval_step,
                               sizeof(interval_step));

  // First, check the boundaries:
  if (wished_interval <= minimum_interval) {
    return minimum_interval;
  } else if (wished_interval >= maximum_interval) {
    return maximum_interval;
  }

  // Okay so we are in between.
  // See how many full steps our "wish" covers and returns this

  // Verify that we are not about to divide by 0
  if (interval_step == 0) {
    sl_log_warning(LOG_TAG,
                   "Interval Step is configured to 0 for attribute %d. "
                   "This should not happen",
                   interval_step_node);
    interval_step = 1;
  }
  wake_up_interval_t number_of_steps
    = (wished_interval - minimum_interval) / interval_step;

  return (minimum_interval + (number_of_steps * interval_step));
}

/**
 * @brief Writes down assumed non-advertised capabilities for a v2 node
 *
 * V2, the nodes do not advertise if they support the Wake Up On Demand, so we
 * Set that to false.
 *
 * @param wake_up_capabilities_node  The Attribute Store node for the Wake Up
 *                                   capabilities node.
 */
static void
  set_default_v2_capabilities(attribute_store_node_t wake_up_capabilities_node)
{
  wake_up_bitmask_t bitmask = DEFAULT_WAKE_UP_CAPBILITIES_BITMASK;
  attribute_store_set_child_reported(wake_up_capabilities_node,
                                     ATTRIBUTE(CAPABILITIES_BITMASK),
                                     &bitmask,
                                     sizeof(bitmask));
}

/**
 * @brief Writes down assumed non-advertised capabilities for a v1 node
 *
 * V1, there are no range of capabilities, we just write that everything
 * Is supported. It can be overridden by somebody else (e.g. Mapper) if we
 * know of a v1 device that does not support the full range.
 *
 * @param wake_up_capabilities_node  The Attribute Store node for the Wake Up
 *                                   capabilities node.
 */
static void
  set_default_v1_capabilities(attribute_store_node_t wake_up_capabilities_node)
{
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(wake_up_capabilities_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  wake_up_interval_t interval = MINIMUM_WAKE_UP_INTERVAL;
  attribute_store_set_child_reported(wake_up_capabilities_node,
                                     ATTRIBUTE(MINIMUM_INTERVAL),
                                     &interval,
                                     sizeof(interval));

  interval = MAXIMUM_WAKE_UP_INTERVAL;
  attribute_store_set_child_reported(wake_up_capabilities_node,
                                     ATTRIBUTE(MAXIMUM_INTERVAL),
                                     &interval,
                                     sizeof(interval));

  interval = get_closest_supported_interval(
    endpoint_node,
    zpc_get_config()->default_wake_up_interval);
  attribute_store_set_child_reported(wake_up_capabilities_node,
                                     ATTRIBUTE(DEFAULT_INTERVAL),
                                     &interval,
                                     sizeof(interval));
  interval = DEFAULT_WAKE_UP_STEP;
  attribute_store_set_child_reported(wake_up_capabilities_node,
                                     ATTRIBUTE(INTERVAL_STEP),
                                     &interval,
                                     sizeof(interval));

  // V1 also de-facto does not advertise v2 capabilities.
  set_default_v2_capabilities(wake_up_capabilities_node);

  // V1 nodes do not support the Capabilites Get, so we write down that
  // capabilities are resolved
  command_status_values_t state = FINAL_STATE;
  attribute_store_set_reported(wake_up_capabilities_node,
                               &state,
                               sizeof(state));
}

///////////////////////////////////////////////////////////////////////////////
// Resolution functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t
  wake_up_capabilities_get(attribute_store_node_t wake_up_capabilities_node,
                           uint8_t *frame,
                           uint16_t *frame_len)
{
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(wake_up_capabilities_node,
                                                 ATTRIBUTE_ENDPOINT_ID);
  attribute_store_node_t version_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(VERSION));

  zwave_cc_version_t version = 0;
  attribute_store_get_reported(version_node, &version, sizeof(version));

  // Check the supporting node version
  if (version == 1) {
    // Assume default value for capabilities that come in v2
    set_default_v1_capabilities(wake_up_capabilities_node);
    command_status_values_t state = FINAL_STATE;
    attribute_store_set_reported(wake_up_capabilities_node,
                                 &state,
                                 sizeof(state));
    *frame_len = 0;
    return SL_STATUS_ALREADY_EXISTS;
  } else if (version == 2) {
    // Write down that v3 capabilities are not supported for v2 nodes.
    set_default_v2_capabilities(wake_up_capabilities_node);
  }

  zwave_minimum_frame_t *get_frame = (zwave_minimum_frame_t *)frame;
  get_frame->command_class         = COMMAND_CLASS_WAKE_UP;
  get_frame->command               = WAKE_UP_INTERVAL_CAPABILITIES_GET_V2;
  *frame_len                       = sizeof(zwave_minimum_frame_t);
  return SL_STATUS_OK;
}

static sl_status_t
  wake_up_interval_get(attribute_store_node_t wake_up_setting_node,
                       uint8_t *frame,
                       uint16_t *frame_len)
{
  zwave_minimum_frame_t *get_frame = (zwave_minimum_frame_t *)frame;
  get_frame->command_class         = COMMAND_CLASS_WAKE_UP;
  get_frame->command               = WAKE_UP_INTERVAL_GET;
  *frame_len                       = sizeof(zwave_minimum_frame_t);
  return SL_STATUS_OK;
}

static sl_status_t
  wake_up_interval_set(attribute_store_node_t wake_up_setting_node,
                       uint8_t *frame,
                       uint16_t *frame_len)
{
  zwave_node_id_t node_id             = 0;
  wake_up_interval_t wake_up_interval = 0;

  attribute_store_node_t wake_up_node_id_node
    = attribute_store_get_first_child_by_type(wake_up_setting_node,
                                              ATTRIBUTE(NODE_ID));

  attribute_store_node_t wake_up_interval_node
    = attribute_store_get_first_child_by_type(wake_up_setting_node,
                                              ATTRIBUTE(INTERVAL));

  if (SL_STATUS_OK
      != attribute_store_read_value(wake_up_node_id_node,
                                    DESIRED_OR_REPORTED_ATTRIBUTE,
                                    &node_id,
                                    sizeof(zwave_node_id_t))) {
    sl_log_warning(LOG_TAG,
                   "Cannot read value for Wake Up NodeID (Attribute %d)",
                   wake_up_node_id_node);
    *frame_len = 0;
    return SL_STATUS_FAIL;
  }

  if (SL_STATUS_OK
      != attribute_store_read_value(wake_up_interval_node,
                                    DESIRED_OR_REPORTED_ATTRIBUTE,
                                    &wake_up_interval,
                                    sizeof(wake_up_interval))) {
    sl_log_warning(LOG_TAG,
                   "Cannot read value for Wake Up Interval (Attribute %d)",
                   wake_up_interval_node);
    *frame_len = 0;
    return SL_STATUS_FAIL;
  }

  // Send a Wake Up Interval Set Command.
  ZW_WAKE_UP_INTERVAL_SET_FRAME *wake_up_interval_set_frame
    = (ZW_WAKE_UP_INTERVAL_SET_FRAME *)frame;
  wake_up_interval_set_frame->cmdClass = COMMAND_CLASS_WAKE_UP;
  wake_up_interval_set_frame->cmd      = WAKE_UP_INTERVAL_SET;
  wake_up_interval_set_frame->seconds1 = (uint8_t)(wake_up_interval >> 16);
  wake_up_interval_set_frame->seconds2 = (uint8_t)(wake_up_interval >> 8);
  wake_up_interval_set_frame->seconds3 = (uint8_t)(wake_up_interval >> 0);
  wake_up_interval_set_frame->nodeid   = (uint8_t)node_id;

  *frame_len = sizeof(ZW_WAKE_UP_INTERVAL_SET_FRAME);
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Frame parsing functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t handle_wake_up_notification(
  const zwave_controller_connection_info_t *connection)
{
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection);
  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(endpoint_node,
                                                 ATTRIBUTE_NODE_ID);

  attribute_resolver_set_resolution_listener(node_id_node,
                                             &send_wake_up_no_more);
  attribute_resolver_resume_node_resolution(node_id_node);

  zwave_node_id_t node_id = 0;
  attribute_store_get_reported(node_id_node, &node_id, sizeof(node_id));
  sl_log_debug(LOG_TAG,
               "NodeID %d is now awake (Wake Up Notification)",
               node_id);

  return SL_STATUS_OK;
}

static sl_status_t handle_wake_up_interval_report(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  const ZW_WAKE_UP_INTERVAL_REPORT_V2_FRAME *f
    = (const ZW_WAKE_UP_INTERVAL_REPORT_V2_FRAME *)frame_data;
  if (frame_length < sizeof(ZW_WAKE_UP_INTERVAL_REPORT_V2_FRAME)) {
    return SL_STATUS_FAIL;
  }

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection);

  attribute_store_node_t wake_up_setting_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(SETTING));

  save_reported_interval(wake_up_setting_node,
                         ATTRIBUTE(INTERVAL),
                         f->seconds1,
                         f->seconds2,
                         f->seconds3);

  zwave_node_id_t node_id = f->nodeid;
  attribute_store_set_child_reported(wake_up_setting_node,
                                     ATTRIBUTE(NODE_ID),
                                     &node_id,
                                     sizeof(node_id));

  // Just received a report, check if the setting should still be resolved or
  // is in its final state:
  verify_if_wake_up_setting_needs_resolution(wake_up_setting_node);

  return SL_STATUS_OK;
}

static sl_status_t handle_wake_up_interval_capabilities_report(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  const ZW_WAKE_UP_INTERVAL_CAPABILITIES_REPORT_V2_FRAME *f
    = (const ZW_WAKE_UP_INTERVAL_CAPABILITIES_REPORT_V2_FRAME *)frame_data;
  if (frame_length < sizeof(ZW_WAKE_UP_INTERVAL_CAPABILITIES_REPORT_V2_FRAME)) {
    return SL_STATUS_FAIL;
  }

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection);

  attribute_store_node_t wake_up_capabilities_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(CAPABILITIES));

  // Minimum Interval
  save_reported_interval(wake_up_capabilities_node,
                         ATTRIBUTE(MINIMUM_INTERVAL),
                         f->minimumWakeUpIntervalSeconds1,
                         f->minimumWakeUpIntervalSeconds2,
                         f->minimumWakeUpIntervalSeconds3);

  // Maximum Interval
  save_reported_interval(wake_up_capabilities_node,
                         ATTRIBUTE(MAXIMUM_INTERVAL),
                         f->maximumWakeUpIntervalSeconds1,
                         f->maximumWakeUpIntervalSeconds2,
                         f->maximumWakeUpIntervalSeconds3);

  // Default Interval
  save_reported_interval(wake_up_capabilities_node,
                         ATTRIBUTE(DEFAULT_INTERVAL),
                         f->defaultWakeUpIntervalSeconds1,
                         f->defaultWakeUpIntervalSeconds2,
                         f->defaultWakeUpIntervalSeconds3);

  // Interval step
  save_reported_interval(wake_up_capabilities_node,
                         ATTRIBUTE(INTERVAL_STEP),
                         f->wakeUpIntervalStepSeconds1,
                         f->wakeUpIntervalStepSeconds2,
                         f->wakeUpIntervalStepSeconds3);

  // Wake Up On Demand flag
  if (frame_length > WAKE_UP_ON_DEMAND_INDEX) {
    wake_up_bitmask_t bitmask = frame_data[WAKE_UP_ON_DEMAND_INDEX];
    attribute_store_set_child_reported(wake_up_capabilities_node,
                                       ATTRIBUTE(CAPABILITIES_BITMASK),
                                       &bitmask,
                                       sizeof(bitmask));
  }

  // Finally, mark the capabilities as resolved
  command_status_values_t state = FINAL_STATE;
  attribute_store_set_reported(wake_up_capabilities_node,
                               &state,
                               sizeof(state));

  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Resolver callback functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Check if we need and send a Wake Up No more command to a node and puts
 * it in the Tx Queue
 *
 * @param node_id_node   The Attribute Store node for the NODE_ID to send back to
 *                       sleep.
 */
static void send_wake_up_no_more(attribute_store_node_t node_id_node)
{
  // We don't need a callback anymore
  attribute_resolver_clear_resolution_listener(node_id_node,
                                               &send_wake_up_no_more);

  // If we have set the Wake Up Interval to ourselves, then put the node
  // back to sleep, else just do nothing with this node.
  if (false == we_are_the_wake_up_destination(node_id_node)) {
    sl_log_debug(LOG_TAG,
                 "We are not the Wake Up Destination. Skipping sending a "
                 " Wake Up No More Command for attribute store node %d",
                 node_id_node);
    attribute_resolver_pause_node_resolution(node_id_node);
    return;
  }

  // No more resolution, but we also want to wait a bit more if we are still
  // establishing return routes with the node.
  zwave_node_id_t node_id = 0;
  attribute_store_get_reported(node_id_node, &node_id, sizeof(node_id));

  if (true == we_have_return_routes_to_assign(node_id)) {
    sl_log_debug(LOG_TAG,
                 "Delaying Wake Up No More Command, as we still "
                 "have return routes to establish for NodeID %d",
                 node_id);
    attribute_timeout_set_callback(node_id_node,
                                   WAKE_UP_NO_MORE_DELAY,
                                   &send_wake_up_no_more);
    return;
  }

  // Now we just pause the resolution and queue a Wake Up No More.
  zwave_controller_connection_info_t connection_info = {};
  const uint8_t no_more_info[]
    = {COMMAND_CLASS_WAKE_UP, WAKE_UP_NO_MORE_INFORMATION};

  // Prevent the resolver from working on this node from now on.
  attribute_resolver_pause_node_resolution(node_id_node);

  zwave_tx_scheme_get_node_connection_info(node_id, 0, &connection_info);
  zwave_tx_options_t tx_options = {};
  zwave_tx_scheme_get_node_tx_options(
    ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY,
    0,
    WAKE_UP_NO_MORE_DISCARD_TIMEOUT,
    &tx_options);

  zwave_tx_send_data(&connection_info,
                     sizeof(no_more_info),
                     no_more_info,
                     &tx_options,
                     NULL,
                     NULL,
                     NULL);

  sl_log_debug(LOG_TAG,
               "Sending NodeID %d back to sleep (Wake Up No More Information)",
               node_id);
}

/**
 * @brief Configures the Wake Up Interval setting for a node.
 *
 * @param wake_up_capabilities_node  The Attribute Store node for the Wake Up
 *                                   capabilities
 */
static void
  configure_wake_up_interval(attribute_store_node_t wake_up_capabilities_node)
{
  // Do not configure more than once
  attribute_resolver_clear_resolution_listener(wake_up_capabilities_node,
                                               &configure_wake_up_interval);

  // Look up the NodeID first, for printing
  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(wake_up_capabilities_node,
                                                 ATTRIBUTE_NODE_ID);
  zwave_node_id_t node_id = 0;
  attribute_store_get_reported(node_id_node, &node_id, sizeof(node_id));
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(wake_up_capabilities_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  // Do not set any Wake Up if we are not the SIS.
  if (zwave_network_management_is_zpc_sis() == false) {
    sl_log_debug(LOG_TAG,
                 "We are not the SIS. "
                 "Skipping Wake Up Interval configuration for NodeID %d",
                 node_id);
    return;
  }

  wake_up_interval_t interval = 0;
  // PS / PEN Role Type: We try to establish the highest supported Wake Up Interval.
  // Same for Wake Up On Demand, as we literally do not need the node to Wake Up Periodically
  if ((is_portable_end_node(wake_up_capabilities_node) == true)
      || (zwave_command_class_wake_up_supports_wake_up_on_demand(node_id_node)
          == true)) {
    interval = get_maximum_possible_interval(endpoint_node);
  } else {
    // Find the node's favorite default. If unknown, use the ZPC config
    attribute_store_node_t default_interval_interval_node
      = attribute_store_get_first_child_by_type(wake_up_capabilities_node,
                                                ATTRIBUTE(DEFAULT_INTERVAL));
    if (attribute_store_is_value_defined(default_interval_interval_node,
                                         REPORTED_ATTRIBUTE)) {
      attribute_store_get_reported(default_interval_interval_node,
                                   &interval,
                                   sizeof(interval));
    } else {
      // Pay attention, the ZPC default may not be supported by the node.
      interval = get_closest_supported_interval(
        endpoint_node,
        zpc_get_config()->default_wake_up_interval);
    }
  }

  attribute_store_node_t wake_up_setting_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(SETTING));

  // Write down the interval to set.
  attribute_store_set_child_desired(wake_up_setting_node,
                                    ATTRIBUTE(INTERVAL),
                                    &interval,
                                    sizeof(interval));

  // NodeID is always ours.
  zwave_node_id_t zpc_node_id = zwave_network_management_get_node_id();
  attribute_store_set_child_desired(wake_up_setting_node,
                                    ATTRIBUTE(NODE_ID),
                                    &zpc_node_id,
                                    sizeof(zpc_node_id));

  // Print to the console that we just configured a Wake Up Node
  sl_log_info(LOG_TAG,
              "Configuring Wake Up Interval for NodeID %d. "
              "Destination NodeID %d - Wake Up Interval %d",
              node_id,
              zpc_node_id,
              interval);
}

static void on_wake_up_setting_send_data_complete(
  attribute_store_node_t wake_up_setting_node,
  resolver_rule_type_t rule_type,
  zpc_resolver_event_t event)
{
  if (rule_type == RESOLVER_GET_RULE) {
    return;
  }

  attribute_store_node_t wake_up_node_id_node
    = attribute_store_get_first_child_by_type(wake_up_setting_node,
                                              ATTRIBUTE(NODE_ID));
  attribute_store_node_t wake_up_interval_node
    = attribute_store_get_first_child_by_type(wake_up_setting_node,
                                              ATTRIBUTE(INTERVAL));

  switch (event) {
    case FRAME_SENT_EVENT_OK_SUPERVISION_WORKING:
      // Do nothing and wait.
      break;

    case FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS:
      // Align NodeID / Interval settings
      attribute_store_set_reported_as_desired(wake_up_node_id_node);
      attribute_store_set_reported_as_desired(wake_up_interval_node);
      attribute_store_undefine_desired(wake_up_node_id_node);
      attribute_store_undefine_desired(wake_up_interval_node);

      // Align the main settings node.
      attribute_store_set_reported_as_desired(wake_up_setting_node);
      attribute_store_undefine_desired(wake_up_setting_node);
      break;

    case FRAME_SENT_EVENT_OK_NO_SUPERVISION:
    case FRAME_SENT_EVENT_OK_SUPERVISION_NO_SUPPORT:
    case FRAME_SENT_EVENT_OK_SUPERVISION_FAIL:
    default:
      // Give up on the previous wish
      attribute_store_undefine_desired(wake_up_node_id_node);
      attribute_store_undefine_desired(wake_up_interval_node);

      // Probe the node again.
      attribute_store_undefine_reported(wake_up_setting_node);
      attribute_store_undefine_desired(wake_up_setting_node);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Store callback functions
///////////////////////////////////////////////////////////////////////////////
static void on_version_attribute_update(attribute_store_node_t updated_node,
                                        attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }

  zwave_cc_version_t version = 0;
  attribute_store_get_reported(updated_node, &version, sizeof(version));

  if (version == 0) {
    // Wait for the version to be known.
    return;
  }

  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  // Create settings and capabilities
  attribute_store_add_if_missing(endpoint_node,
                                 setting_node_type,
                                 COUNT_OF(setting_node_type));
  attribute_store_add_if_missing(endpoint_node,
                                 capabilities_node_type,
                                 COUNT_OF(capabilities_node_type));

  attribute_store_node_t wake_up_capabilities_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(CAPABILITIES));
  attribute_store_node_t wake_up_setting_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(SETTING));

  attribute_store_add_if_missing(wake_up_capabilities_node,
                                 capabilities_attributes,
                                 COUNT_OF(capabilities_attributes));
  attribute_store_add_if_missing(wake_up_setting_node,
                                 setting_attributes,
                                 COUNT_OF(setting_attributes));

  // Register a listener callback on the NodeID, so we sent it back to sleep
  // when we are done-done.
  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(endpoint_node,
                                                 ATTRIBUTE_NODE_ID);
  attribute_resolver_set_resolution_listener(node_id_node,
                                             &send_wake_up_no_more);
}

/**
 * @brief Verifies if we are resolving capabilities, and if yes, set a resolutio
 *        listener to set the Wake Up Interval
 *
 * We register a listener callback on the lowest possible level, so we configure
 * the wake up interval as early as possible.
 *
 * @param updated_node    Attribute Store node that was updated (ATTRIBUTE(CAPABILITIES))
 * @param change          Attribute Store change.
 */
static void
  on_capabilities_attribute_update(attribute_store_node_t updated_node,
                                   attribute_store_change_t change)
{
  bool set_wake_up_interval = false;
  if (change == ATTRIBUTE_CREATED) {
    set_wake_up_interval = true;
  } else if ((change == ATTRIBUTE_UPDATED)
             && attribute_store_is_value_defined(updated_node,
                                                 REPORTED_ATTRIBUTE)
                  == false) {
    set_wake_up_interval = true;
  }

  if (set_wake_up_interval == true) {
    attribute_resolver_set_resolution_listener(updated_node,
                                               &configure_wake_up_interval);
  }
}

/**
 * @brief Verifies if either NodeID / Wake Up interval settings have changed
 *        and need resolution in this case, we will mismatch the Setting attribute
 *        to get a resolution
 *
 *
 * @param updated_node    Attribute Store node that was updated Setting->NodeID
 *                        or Setting->Interval
 * @param change          Attribute Store change.
 */
static void
  on_desired_wake_up_setting_update(attribute_store_node_t updated_node,
                                    attribute_store_change_t change)
{
  if (change != ATTRIBUTE_UPDATED) {
    return;
  }

  // Somebody just updated the Desired value of either the Wake Up NodeID or
  // Wake Up Interval.
  // Verify if we should put the parent in a "to set" state
  verify_if_wake_up_setting_needs_resolution(
    attribute_store_get_first_parent_with_type(updated_node,
                                               ATTRIBUTE(SETTING)));
}

///////////////////////////////////////////////////////////////////////////////
// Incoming commands handler
///////////////////////////////////////////////////////////////////////////////
static sl_status_t
  wake_up_control_handler(const zwave_controller_connection_info_t *connection,
                          const uint8_t *frame_data,
                          uint16_t frame_length)
{
  if (frame_length <= COMMAND_INDEX) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  assert(frame_data[COMMAND_CLASS_INDEX] == COMMAND_CLASS_WAKE_UP);

  switch (frame_data[COMMAND_INDEX]) {
    case WAKE_UP_NOTIFICATION:
      return handle_wake_up_notification(connection);

    case WAKE_UP_INTERVAL_REPORT:
      return handle_wake_up_interval_report(connection,
                                            frame_data,
                                            frame_length);
    case WAKE_UP_INTERVAL_CAPABILITIES_REPORT_V2:
      return handle_wake_up_interval_capabilities_report(connection,
                                                         frame_data,
                                                         frame_length);

    default:
      return SL_STATUS_NOT_SUPPORTED;
  }
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
bool zwave_command_class_wake_up_supports_wake_up_on_demand(
  attribute_store_node_t node_id_node)
{
  zwave_node_id_t node_id         = 0;
  zwave_endpoint_id_t endpoint_id = 0;
  attribute_store_node_t endpoint_node
    = attribute_store_get_node_child_by_value(node_id_node,
                                              ATTRIBUTE_ENDPOINT_ID,
                                              REPORTED_ATTRIBUTE,
                                              &endpoint_id,
                                              sizeof(zwave_endpoint_id_t),
                                              0);
  attribute_store_network_helper_get_node_id_from_node(endpoint_node, &node_id);
  if (3 > zwave_node_get_command_class_version(COMMAND_CLASS_WAKE_UP,
                                               node_id,
                                               endpoint_id)) {
    return false;
  }

  if (2 > zwave_node_get_command_class_version(COMMAND_CLASS_SUPERVISION,
                                               node_id,
                                               endpoint_id)) {
    return false;
  }

  attribute_store_node_t wake_up_capabilities_node
    = attribute_store_get_first_child_by_type(endpoint_node,
                                              ATTRIBUTE(CAPABILITIES));

  attribute_store_node_t wake_up_bitmask_node
    = attribute_store_get_first_child_by_type(wake_up_capabilities_node,
                                              ATTRIBUTE(CAPABILITIES_BITMASK));
  wake_up_bitmask_t wake_up_bitmask = 0;
  attribute_store_get_reported(wake_up_bitmask_node,
                               &wake_up_bitmask,
                               sizeof(wake_up_bitmask));

  return (wake_up_bitmask & WAKE_UP_ON_DEMAND_BITMASK);
}

sl_status_t zwave_command_class_wake_up_init()
{
  // Attribute Resolver rules
  attribute_resolver_register_rule(ATTRIBUTE(SETTING),
                                   &wake_up_interval_set,
                                   &wake_up_interval_get);
  attribute_resolver_register_rule(ATTRIBUTE(CAPABILITIES),
                                   NULL,
                                   &wake_up_capabilities_get);

  // Attribute Store callbacks
  attribute_store_register_callback_by_type(&on_version_attribute_update,
                                            ATTRIBUTE(VERSION));

  attribute_store_register_callback_by_type_and_state(
    &on_desired_wake_up_setting_update,
    ATTRIBUTE(INTERVAL),
    DESIRED_ATTRIBUTE);
  attribute_store_register_callback_by_type_and_state(
    &on_desired_wake_up_setting_update,
    ATTRIBUTE(NODE_ID),
    DESIRED_ATTRIBUTE);

  attribute_store_register_callback_by_type(&on_capabilities_attribute_update,
                                            ATTRIBUTE(CAPABILITIES));

  // Attribute Resolver event listener:
  register_send_event_handler(ATTRIBUTE(SETTING),
                              &on_wake_up_setting_send_data_complete);

  // Register our handler to the Z-Wave CC framework:
  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler         = &wake_up_control_handler;
  handler.minimal_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME;
  handler.command_class      = COMMAND_CLASS_WAKE_UP;
  handler.version            = 3;
  handler.command_class_name = "Wake Up";
  handler.manual_security_validation = false;

  zwave_command_handler_register_handler(handler);

  return SL_STATUS_OK;
}