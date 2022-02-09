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
#include "zpc_attribute_resolver.h"
#include "zpc_attribute_resolver_callbacks.h"

// UIC Components
#include "sl_log.h"

// ZPC Components includes
#include "zwave_command_class_supervision.h"
#include "ZW_classcmd.h"

// Generic includes
#include <map>

constexpr char LOG_TAG[] = "zpc_attribute_resolver";

typedef struct node_data {
  resolver_rule_type_t rule_type;
  zwave_tx_session_id_t tx_session;
  bool valid_tx_session;
} node_data_t;

// Local map where we remember which nodes are being resolved
static std::map<attribute_store_node_t, node_data_t> nodes_under_resolution;

// Local list of functions to call when resolution events happened.
static std::map<attribute_store_type_t,
                zpc_resolver_event_notification_function_t>
  send_event_handlers;

void add_node_in_resolution_list(attribute_store_node_t new_node,
                                 resolver_rule_type_t rule_type)
{
  node_data_t node_data      = {};
  node_data.rule_type        = rule_type;
  node_data.valid_tx_session = false;
  nodes_under_resolution.insert(
    std::pair<attribute_store_node_t, node_data_t>(new_node, node_data));
}

bool is_node_in_resolution_list(attribute_store_node_t node)
{
  return nodes_under_resolution.count(node) > 0;
}

void remove_node_from_resolution_list(attribute_store_node_t node)
{
  nodes_under_resolution.erase(node);
}

///////////////////////////////////////////////////////////////////////////////
// Function shared among the component. Used for send_data callbacks
///////////////////////////////////////////////////////////////////////////////
void on_resolver_zwave_send_data_complete(uint8_t status,
                                          const zwapi_tx_report_t *tx_info,
                                          void *user)
{
  attribute_store_node_t current_node = (intptr_t)user;
  if (nodes_under_resolution.find(current_node)
      == nodes_under_resolution.end()) {
    sl_log_debug(LOG_TAG,
                 "Send Data Callback for node %d not under resolution",
                 current_node);
    return;
  }

  zpc_resolver_event_t event = FRAME_SENT_EVENT_OK_NO_SUPERVISION;
  resolver_send_status_t rs;
  switch (status) {
    case TRANSMIT_COMPLETE_OK:
      rs = RESOLVER_SEND_STATUS_OK;
      break;
    case TRANSMIT_COMPLETE_VERIFIED:
      rs = RESOLVER_SEND_STATUS_OK_EXECUTION_VERIFIED;
      break;
    default:
      rs    = RESOLVER_SEND_STATUS_FAIL;
      event = FRAME_SENT_EVENT_FAIL;
      break;
  }

  resolver_rule_type_t current_rule_type
    = nodes_under_resolution[current_node].rule_type;
  clock_time_t transmission_time = tx_info ? tx_info->transmit_ticks * 10 : 0;

  // Do we have a custom handler for this type?
  auto it
    = send_event_handlers.find(attribute_store_get_node_type(current_node));
  if (it == send_event_handlers.end()) {
    on_resolver_send_data_complete(rs,
                                   transmission_time,
                                   current_node,
                                   current_rule_type);
  } else {
    it->second(current_node, current_rule_type, event);
    on_resolver_send_data_complete(RESOLVER_SEND_STATUS_ALREADY_HANDLED,
                                   transmission_time,
                                   current_node,
                                   current_rule_type);
  }

  // Clear up the node resolution slot
  remove_node_from_resolution_list(current_node);
}

void on_resolver_zwave_supervision_complete(uint8_t supervision_status,
                                            const zwapi_tx_report_t *tx_info,
                                            void *user)
{
  attribute_store_node_t current_node = (intptr_t)user;
  if (nodes_under_resolution.find(current_node)
      == nodes_under_resolution.end()) {
    sl_log_debug(LOG_TAG,
                 "Supervision Callback for node %d not under resolution",
                 current_node);
    return;
  }

  zpc_resolver_event_t event;
  resolver_send_status_t rs;
  switch (supervision_status) {
    case SUPERVISION_REPORT_SUCCESS:
      event = FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS;
      rs    = RESOLVER_SEND_STATUS_OK_EXECUTION_VERIFIED;
      break;
    case SUPERVISION_REPORT_NO_SUPPORT:
      event = FRAME_SENT_EVENT_OK_SUPERVISION_NO_SUPPORT;
      rs    = RESOLVER_SEND_STATUS_OK_EXECUTION_FAILED;
      break;
    case SUPERVISION_REPORT_FAIL:
      event = FRAME_SENT_EVENT_OK_SUPERVISION_FAIL;
      rs    = RESOLVER_SEND_STATUS_OK_EXECUTION_FAILED;
      break;
    case SUPERVISION_REPORT_WORKING:
      event = FRAME_SENT_EVENT_OK_SUPERVISION_WORKING;
      rs    = RESOLVER_SEND_STATUS_OK_EXECUTION_PENDING;
      break;
    default:
      event = FRAME_SENT_EVENT_OK_SUPERVISION_FAIL;
      rs    = RESOLVER_SEND_STATUS_FAIL;
      break;
  }

  resolver_rule_type_t current_rule_type
    = nodes_under_resolution[current_node].rule_type;
  clock_time_t transmission_time = tx_info ? tx_info->transmit_ticks * 10 : 0;

  // Do we have a custom handler for this type?
  auto it
    = send_event_handlers.find(attribute_store_get_node_type(current_node));
  if (it == send_event_handlers.end()) {
    on_resolver_send_data_complete(rs,
                                   transmission_time,
                                   current_node,
                                   current_rule_type);
  } else {
    // Call the custom handler
    it->second(current_node, current_rule_type, event);
    // Tell the regular handler that it's been taken care of
    on_resolver_send_data_complete(RESOLVER_SEND_STATUS_ALREADY_HANDLED,
                                   transmission_time,
                                   current_node,
                                   current_rule_type);
  }

  // Clear up the node resolution slot, if the session is over
  if (supervision_status != SUPERVISION_REPORT_WORKING) {
    remove_node_from_resolution_list(current_node);
  }
}

sl_status_t attribute_resolver_associate_node_with_tx_sessions_id(
  attribute_store_node_t node, zwave_tx_session_id_t tx_session_id)
{
  auto it = nodes_under_resolution.find(node);
  if (it == nodes_under_resolution.end()) {
    return SL_STATUS_NOT_FOUND;
  }
  it->second.valid_tx_session = true;
  it->second.tx_session       = tx_session_id;
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Init/reset function
///////////////////////////////////////////////////////////////////////////////
void attribute_resolver_callbacks_reset()
{
  nodes_under_resolution.clear();
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t register_send_event_handler(
  attribute_store_type_t type,
  zpc_resolver_event_notification_function_t function)
{
  if (send_event_handlers.count(type) == 0) {
    send_event_handlers[type] = function;
    return SL_STATUS_OK;
  }
  return SL_STATUS_ALREADY_EXISTS;
}

sl_status_t unregister_send_event_handler(
  attribute_store_type_t type,
  zpc_resolver_event_notification_function_t function)
{
  send_event_handlers.erase(type);
  return SL_STATUS_OK;
}

sl_status_t
  attribute_resolver_abort_pending_resolution(attribute_store_node_t node)
{
  if (is_node_in_resolution_list(node) == false) {
    sl_log_debug(LOG_TAG,
                 "Node %d is not under resolution. Ignoring abort request",
                 node);
    return SL_STATUS_NOT_FOUND;
  }

  sl_log_debug(LOG_TAG, "Aborting resolution for node %d", node);
  // Clear up the node resolution slot, and make sure that supervision stops
  // following up on it.
  if ((nodes_under_resolution[node].valid_tx_session == true)
      && (nodes_under_resolution[node].rule_type == RESOLVER_SET_RULE)) {
    zwave_command_class_supervision_close_session_by_tx_session(
      nodes_under_resolution[node].tx_session);
  }
  // Tell the uic_resolver_rule to remove this node from the pending resolutions.
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_ABORTED,
                                 0,
                                 node,
                                 nodes_under_resolution[node].rule_type);
  remove_node_from_resolution_list(node);
  return SL_STATUS_OK;
}
