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
//Includes from this component
#include "zwave_command_class_wakeup.h"
#include "zwave_command_classes_utils.h"

// Includes from other components
#include "zwave_controller_command_class_indices.h"
#include "zwave_controller_utils.h"
#include "attribute_store.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute_store_helper.h"
#include "zpc_attribute_store_network_helper.h"

#include "attribute_resolver.h"
#include "zwave_command_handler.h"
#include "zwave_network_management.h"
#include "zwave_tx_scheme_selector.h"
#include "zwave_tx.h"
#include "zpc_config.h"
#include "sl_status.h"
#include "sys/clock.h"

// Generic includes
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

// Frame parsing indices
#define WAKE_UP_ON_DEMAND_INDEX 14

static const attribute_store_type_t last_wakeup_timestamp_attr[]
  = {ATTRIBUTE_LAST_RECEIVED_FRAME_TIMESTAMP};

static const attribute_store_type_t v1_attributes[]
  = {ATTRIBUTE_COMMAND_CLASS_WAKEUP_INTERVAL,
     ATTRIBUTE_COMMAND_CLASS_WAKEUP_NODEID};

static const attribute_store_type_t v2_attributes[] = {
  ATTRIBUTE_COMMAND_CLASS_WAKEUP_V2_MINIMUM_INTERVAL,
  ATTRIBUTE_COMMAND_CLASS_WAKEUP_V2_MAXIMUM_INTERVAL,
  ATTRIBUTE_COMMAND_CLASS_WAKEUP_V2_DEFAULT_INTERVAL,
  ATTRIBUTE_COMMAND_CLASS_WAKEUP_V2_INTERVAL_STEP,
};

static const attribute_store_type_t v3_attributes[]
  = {ATTRIBUTE_COMMAND_CLASS_WAKEUP_V3_WAKE_ON_DEMAND};

/* Used to send NO_MORE_INFORMATION when a node has been resolved
 * If transmition fails its just too bad
 */
static void on_node_resolved(attribute_store_node_t resolved_node)
{
  zwave_controller_connection_info_t connection_info;
  zwave_node_id_t node_id = 0;
  const uint8_t no_more_info[]
    = {COMMAND_CLASS_WAKE_UP, WAKE_UP_NO_MORE_INFORMATION};

  //We don't need a callback anymore
  attribute_resolver_clear_resolution_listener(resolved_node, on_node_resolved);
  //Prevent the resolver from working on this node from now on.
  attribute_resolver_pause_node_resolution(resolved_node);

  zwave_tx_scheme_get_node_connection_info(node_id,
                                           0,  //Endpoint 0 for Wake Up
                                           &connection_info);

  attribute_store_read_value(resolved_node,
                             REPORTED_ATTRIBUTE,
                             &node_id,
                             sizeof(node_id));
  zwave_tx_scheme_get_node_connection_info(node_id, 0, &connection_info);
  zwave_tx_options_t tx_options = {};
  zwave_tx_scheme_get_node_tx_options(
    ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY,
    0,
    0,
    &tx_options);

  zwave_tx_send_data(&connection_info,
                     sizeof(no_more_info),
                     no_more_info,
                     &tx_options,
                     NULL,
                     NULL,
                     NULL);
  /// This will be visited once when the node is just included and inclusion node interview is done.
  /// we just make sure that we are setting last awake timestamp when zpc instruct the node to sleep
  /// after inclusion interview.
  uint32_t wake_up_timestamp_in_seconds = clock_seconds();
  attribute_store_set_uint32_child_by_type(
    resolved_node,
    ATTRIBUTE_LAST_RECEIVED_FRAME_TIMESTAMP,
    REPORTED_ATTRIBUTE,
    wake_up_timestamp_in_seconds);
}

// Return true if the given node has
// ATTRIBUTE_ZWAVE_ROLE_TYPE==ROLE_TYPE_END_NODE_PORTABLE
static bool is_portable_end_node(attribute_store_node_t node)
{
  zwave_node_id_t node_id = 0x00;
  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(node,
                                                 ATTRIBUTE_NODE_ID);
  attribute_store_get_reported(node_id_node, &node_id, sizeof(node_id));
  if (get_zwave_node_role_type(node_id) == ROLE_TYPE_END_NODE_PORTABLE) {
    return true;
  }
  return false;
}

static void on_version_attribute_update(attribute_store_node_t updated_node,
                                        attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    return;
  }
  assert(ATTRIBUTE_COMMAND_CLASS_WAKEUP_VERSION
         == attribute_store_get_node_type(updated_node));

  uint8_t version      = 0;
  uint8_t version_size = 0;
  attribute_store_node_t endpoint_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_ENDPOINT_ID);

  attribute_store_get_node_attribute_value(updated_node,
                                           REPORTED_ATTRIBUTE,
                                           &version,
                                           &version_size);
  if (version_size != 1) {
    return;
  }

  if (version >= 1) {
    attribute_store_add_if_missing(endpoint_node,
                                   v1_attributes,
                                   COUNT_OF(v1_attributes));
    if (zwave_network_management_is_zpc_sis()) {
      /*
      if the Role Type is ATTRIBUTE_ZWAVE_ROLE_TYPE==ROLE_TYPE_END_NODE_PORTABLE)

      - If the node advertises Wake-Up Capabilities (Wake-Up Command Class,
        version 2 or newer), the Wake Up Interval Set Command Seconds field
        MUST be within the allowed range.
      - If the node does not advertise Wake-Up Capabilities (Wake-Up Command
        Class, version 1), the Wake-Up Interval Set Command Seconds field MUST
        be set to 0
      */
      if (version == 1) {
        attribute_store_set_uint32_child_by_type(
          endpoint_node,
          ATTRIBUTE_COMMAND_CLASS_WAKEUP_INTERVAL,
          DESIRED_ATTRIBUTE,
          is_portable_end_node(endpoint_node)
            ? 0
            : zpc_get_config()->default_wakeup_interval);
      }

      attribute_store_node_t wakeup_nodeid
        = attribute_store_get_node_child_by_type(
          endpoint_node,
          ATTRIBUTE_COMMAND_CLASS_WAKEUP_NODEID,
          0);
      zwave_node_id_t nodeid = zwave_network_management_get_node_id();
      attribute_store_set_desired(wakeup_nodeid, &nodeid, sizeof(nodeid));
    }
  }
  if (version >= 2) {
    attribute_store_add_if_missing(endpoint_node,
                                   v2_attributes,
                                   COUNT_OF(v2_attributes));
  }
  if (version >= 3) {
    attribute_store_add_if_missing(endpoint_node,
                                   v3_attributes,
                                   COUNT_OF(v3_attributes));
  }
  // Register a listener callback on the NodeID, that will allow a WakeUp CC
  // handler to be notified when a wakeup node interview is performed.
  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(updated_node,
                                                 ATTRIBUTE_NODE_ID);
  attribute_resolver_set_resolution_listener(node_id_node, on_node_resolved);
  // adding ATTRIBUTE_LAST_RECEIVED_FRAME_TIMESTAMP under NodeID
  attribute_store_add_if_missing(node_id_node,
                                 last_wakeup_timestamp_attr,
                                 COUNT_OF(last_wakeup_timestamp_attr));
}

static sl_status_t handle_wake_up_notification(
  const zwave_controller_connection_info_t *connection)
{
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection);

  attribute_resolver_set_resolution_listener(
    attribute_store_get_first_parent_with_type(endpoint_node,
                                               ATTRIBUTE_NODE_ID),
    on_node_resolved);
  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(endpoint_node,
                                                 ATTRIBUTE_NODE_ID);

  attribute_resolver_resume_node_resolution(node_id_node);

  uint32_t wake_up_timestamp_in_seconds = clock_seconds();
  attribute_store_set_uint32_child_by_type(
    node_id_node,
    ATTRIBUTE_LAST_RECEIVED_FRAME_TIMESTAMP,
    REPORTED_ATTRIBUTE,
    wake_up_timestamp_in_seconds);
  return SL_STATUS_OK;
}

static sl_status_t handle_wake_up_interval_report(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection);

  ZW_WAKE_UP_INTERVAL_REPORT_V2_FRAME *f
    = (ZW_WAKE_UP_INTERVAL_REPORT_V2_FRAME *)frame_data;
  if (frame_length < sizeof(ZW_WAKE_UP_INTERVAL_REPORT_V2_FRAME)) {
    return SL_STATUS_FAIL;
  }
  uint32_t wakeup_interval
    = f->seconds1 << 16 | f->seconds2 << 8 | f->seconds3 << 0;

  attribute_store_set_uint32_child_by_type(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_INTERVAL,
    REPORTED_ATTRIBUTE,
    wakeup_interval);

  attribute_store_node_t wakeup_nodeid = attribute_store_get_node_child_by_type(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_NODEID,
    0);
  zwave_node_id_t nodeid = f->nodeid;
  attribute_store_set_reported(wakeup_nodeid, &nodeid, sizeof(nodeid));

  return SL_STATUS_OK;
}

static sl_status_t handle_wake_up_interval_capabilities_report(
  const zwave_controller_connection_info_t *connection,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  ZW_WAKE_UP_INTERVAL_CAPABILITIES_REPORT_V2_FRAME *f
    = (ZW_WAKE_UP_INTERVAL_CAPABILITIES_REPORT_V2_FRAME *)frame_data;
  if (frame_length < sizeof(ZW_WAKE_UP_INTERVAL_CAPABILITIES_REPORT_V2_FRAME)) {
    return SL_STATUS_OK;
  }

  attribute_store_node_t endpoint_node
    = zwave_command_class_get_endpoint_node(connection);

  attribute_store_set_uint32_child_by_type(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_V2_MINIMUM_INTERVAL,
    REPORTED_ATTRIBUTE,
    f->minimumWakeUpIntervalSeconds1 << 16
      | f->minimumWakeUpIntervalSeconds2 << 8
      | f->minimumWakeUpIntervalSeconds3 << 0);

  attribute_store_set_uint32_child_by_type(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_V2_MAXIMUM_INTERVAL,
    REPORTED_ATTRIBUTE,
    f->maximumWakeUpIntervalSeconds1 << 16
      | f->maximumWakeUpIntervalSeconds2 << 8
      | f->maximumWakeUpIntervalSeconds3 << 0);

  attribute_store_set_uint32_child_by_type(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_V2_DEFAULT_INTERVAL,
    REPORTED_ATTRIBUTE,
    f->defaultWakeUpIntervalSeconds1 << 16
      | f->defaultWakeUpIntervalSeconds2 << 8
      | f->defaultWakeUpIntervalSeconds3 << 0);

  attribute_store_set_uint32_child_by_type(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_V2_INTERVAL_STEP,
    REPORTED_ATTRIBUTE,
    f->wakeUpIntervalStepSeconds1 << 16 | f->wakeUpIntervalStepSeconds2 << 8
      | f->wakeUpIntervalStepSeconds3 << 0);

  if (frame_length > WAKE_UP_ON_DEMAND_INDEX) {
    attribute_store_set_uint32_child_by_type(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_WAKEUP_V3_WAKE_ON_DEMAND,
      REPORTED_ATTRIBUTE,
      frame_data[WAKE_UP_ON_DEMAND_INDEX]);
  }

  if (zwave_network_management_is_zpc_sis()) {
    //If there is no wakeup interval set, set the desired attribute
    // wakeup attribute to the default value
    attribute_store_node_t wakeup_interval_node
      = attribute_store_get_node_child_by_type(
        endpoint_node,
        ATTRIBUTE_COMMAND_CLASS_WAKEUP_INTERVAL,
        0);
    if (!attribute_store_is_value_defined(wakeup_interval_node,
                                          REPORTED_ATTRIBUTE)) {
      attribute_store_set_uint32_child_by_type(
        endpoint_node,
        ATTRIBUTE_COMMAND_CLASS_WAKEUP_INTERVAL,
        DESIRED_ATTRIBUTE,
        f->defaultWakeUpIntervalSeconds1 << 16
          | f->defaultWakeUpIntervalSeconds2 << 8
          | f->defaultWakeUpIntervalSeconds3 << 0);
    }
  }
  return SL_STATUS_OK;
}

static sl_status_t
  wakeup_control_handler(const zwave_controller_connection_info_t *connection,
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

static sl_status_t wakeup_capabilities_get(attribute_store_node_t node,
                                           uint8_t *frame,
                                           uint16_t *frame_len)
{
  ZW_WAKE_UP_INTERVAL_CAPABILITIES_GET_V2_FRAME *wakeup_interval_get_frame
    = (ZW_WAKE_UP_INTERVAL_CAPABILITIES_GET_V2_FRAME *)frame;
  wakeup_interval_get_frame->cmdClass = COMMAND_CLASS_WAKE_UP;
  wakeup_interval_get_frame->cmd      = WAKE_UP_INTERVAL_CAPABILITIES_GET_V2;
  *frame_len = sizeof(ZW_WAKE_UP_INTERVAL_CAPABILITIES_GET_V2_FRAME);
  return SL_STATUS_OK;
}

static sl_status_t wakeup_interval_get(attribute_store_node_t node,
                                       uint8_t *frame,
                                       uint16_t *frame_len)
{
  ZW_WAKE_UP_INTERVAL_GET_FRAME *wakeup_interval_get_frame
    = (ZW_WAKE_UP_INTERVAL_GET_FRAME *)frame;
  wakeup_interval_get_frame->cmdClass = COMMAND_CLASS_WAKE_UP;
  wakeup_interval_get_frame->cmd      = WAKE_UP_INTERVAL_GET;
  *frame_len                          = sizeof(ZW_WAKE_UP_INTERVAL_GET_FRAME);
  return SL_STATUS_OK;
}

static sl_status_t wakeup_interval_set(attribute_store_node_t node,
                                       uint8_t *frame,
                                       uint16_t *frame_len)
{
  zwave_node_id_t node_id  = 0;
  uint32_t wakeup_interval = 0;

  attribute_store_node_t endpoint_node = attribute_store_get_node_parent(node);

  attribute_store_node_t interval_node = attribute_store_get_node_child_by_type(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_INTERVAL,
    0);

  attribute_store_node_t nodeid_node = attribute_store_get_node_child_by_type(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_NODEID,
    0);

  if (SL_STATUS_OK
      != attribute_store_read_value(nodeid_node,
                                    DESIRED_OR_REPORTED_ATTRIBUTE,
                                    &node_id,
                                    sizeof(zwave_node_id_t))) {
    *frame_len = 0;
    return SL_STATUS_FAIL;
  }

  if (SL_STATUS_OK
      != attribute_store_read_value(interval_node,
                                    DESIRED_OR_REPORTED_ATTRIBUTE,
                                    &wakeup_interval,
                                    sizeof(wakeup_interval))) {
    *frame_len = 0;
    return SL_STATUS_FAIL;
  }
  // Send a Wake Up Interval Set Command.
  ZW_WAKE_UP_INTERVAL_SET_FRAME *wakeup_interval_set_frame
    = (ZW_WAKE_UP_INTERVAL_SET_FRAME *)frame;
  wakeup_interval_set_frame->cmdClass = COMMAND_CLASS_WAKE_UP;
  wakeup_interval_set_frame->cmd      = WAKE_UP_INTERVAL_SET;
  wakeup_interval_set_frame->seconds1 = (wakeup_interval >> 16);
  wakeup_interval_set_frame->seconds2 = (wakeup_interval >> 8);
  wakeup_interval_set_frame->seconds3 = (wakeup_interval >> 0);
  wakeup_interval_set_frame->nodeid   = node_id;

  *frame_len = sizeof(ZW_WAKE_UP_INTERVAL_SET_FRAME);
  return SL_STATUS_OK;
}
static sl_status_t pause_resolution_on_wakeup_nodes(attribute_store_node_t node,
                                                    int depth)
{
  if (attribute_store_get_node_type(node) == ATTRIBUTE_ENDPOINT_ID) {
    if (attribute_store_get_node_child_by_type(
          node,
          ATTRIBUTE_COMMAND_CLASS_WAKEUP_VERSION,
          0)
        != ATTRIBUTE_STORE_INVALID_NODE) {
      attribute_resolver_pause_node_resolution(
        attribute_store_get_node_parent(node));
    }
    return SL_STATUS_EMPTY;  //dont process the sub tree
  }
  return SL_STATUS_OK;  // move on
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
bool zwave_command_class_wakeup_supports_wake_up_on_demand(
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
  attribute_store_node_t wake_up_on_demand_node
    = attribute_store_get_node_child_by_type(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_WAKEUP_V3_WAKE_ON_DEMAND,
      0);
  uint32_t wake_up_on_demand = 0;
  attribute_store_read_value(wake_up_on_demand_node,
                             REPORTED_ATTRIBUTE,
                             &wake_up_on_demand,
                             sizeof(wake_up_on_demand));

  return ((wake_up_on_demand & 1) == 1);
}

sl_status_t zwave_command_class_wakeup_init()
{
  attribute_resolver_register_rule(ATTRIBUTE_COMMAND_CLASS_WAKEUP_INTERVAL,
                                   wakeup_interval_set,
                                   wakeup_interval_get);
  attribute_resolver_register_rule(ATTRIBUTE_COMMAND_CLASS_WAKEUP_NODEID,
                                   wakeup_interval_set,
                                   wakeup_interval_get);

  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_V2_MINIMUM_INTERVAL,
    0,
    wakeup_capabilities_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_V2_MAXIMUM_INTERVAL,
    0,
    wakeup_capabilities_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_V2_DEFAULT_INTERVAL,
    0,
    wakeup_capabilities_get);
  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_V2_INTERVAL_STEP,
    0,
    wakeup_capabilities_get);

  attribute_resolver_register_rule(
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_V3_WAKE_ON_DEMAND,
    0,
    wakeup_capabilities_get);

  // Command class wake up will listen to attributes for
  // 1. ATTRIBUTE_COMMAND_CLASS_WAKEUP_VERSION
  // In which case it will populate its required attributes.
  attribute_store_register_callback_by_type(
    on_version_attribute_update,
    ATTRIBUTE_COMMAND_CLASS_WAKEUP_VERSION);

  // Register our handler to the Z-Wave CC framework:
  zwave_command_handler_t handler = {};
  handler.support_handler         = NULL;
  handler.control_handler         = &wakeup_control_handler;
  handler.minimal_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME;
  handler.command_class      = COMMAND_CLASS_WAKE_UP;
  handler.version            = 3;
  handler.command_class_name = "Wake Up";
  handler.manual_security_validation = false;

  zwave_command_handler_register_handler(handler);

  //FIXME: It's not the job of the CC to do this. THe network monitor should
  // do it after init our network.
  attribute_store_walk_tree(attribute_store_get_root(),
                            pause_resolution_on_wakeup_nodes,
                            0);
  return SL_STATUS_OK;
}