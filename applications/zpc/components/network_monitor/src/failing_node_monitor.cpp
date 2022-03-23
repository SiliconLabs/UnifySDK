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

//Standard libraries
#include <list>

//ZPC libraries
#include <zwave_node_id_definitions.h>
#include <zwave_controller_connection_info.h>
#include <sl_log.h>
#include <ctimer.h>
#include <zwave_utils.h>  // For Z-Wave operating mode functionality
// for ATTRIBUTE_ZWAVE_FAILING_NODE_PING_INTERVAL and ATTRIBUTE_NODE_ID
#include <attribute_store_defined_attribute_types.h>
// for attribute_store_network_helper_get_zwave_node_id_node()
#include <zpc_attribute_store_network_helper.h>
#include <zpc_attribute_store.h>  // for get_zpc_network_node
#include <attribute.hpp>
#include <zwave_controller_utils.h>  // for zwave_send_nop_to_node()

//This module
#include "failing_node_monitor.h"

#define LOG_TAG                                 "failing_node_monitor"
#define ALWAYS_LISTENING_PING_TIME_INTERVAL     (4 * CLOCK_SECOND)
#define FREQUENTLY_LISTENING_PING_TIME_INTERVAL (40 * CLOCK_SECOND)
#define MAX_PING_TIME_INTERVAL                  (24 * 60 * CLOCK_SECOND)  // 24 hours

void stop_monitoring_failing_node(zwave_node_id_t node_id);
void start_monitoring_failing_node(zwave_node_id_t node_id);
sl_status_t zwave_remove_failing_node_ping_interval_from_attribute_store(
  zwave_node_id_t node_id);
sl_status_t zwave_store_failing_node_ping_interval(zwave_node_id_t node_id,
                                                   clock_time_t interval);
static void ping_failing_node(void *user);

// last_timer is a timestamp of when the last timer of ping for any node
// occured
// used check if its time to ping the node by comparing
// last timestamp(last_timer) the node was pinged and clock_time and see if its
// more than next_ping_in
// also used to adjust the next_ping_time for all other nodes when some node is
// being pinged
static clock_time_t last_timer = 0;
static struct ctimer ping_timer;

struct failing_node_t {
  zwave_node_id_t node_id;
  clock_time_t next_ping_in;  //Duration to wait before next ping. This will be
                              // changed when adjust is done

  clock_time_t last_ping_in;  //Previous ping duration. This is never changed
                              // and used to calculate next ping interval
  zwave_operating_mode_t zwave_operating_mode;

  bool operator<(const failing_node_t &n) const
  {
    return next_ping_in < n.next_ping_in;
  }
  bool operator>(const failing_node_t &n) const
  {
    return next_ping_in > n.next_ping_in;
  }
};

static std::list<failing_node_t> ping_list;

using namespace attribute_store;
static void send_nop()
{
  if (ping_list.empty()) {
    sl_log_warning(LOG_TAG,
                   "Failing node list empty. No NOP will be sent out.");
    return;
  }

  zwave_node_id_t node_id = 0;
  node_id                 = ping_list.begin()->node_id;

  // No need to setup callback here. If the NOP succeeds, Network Monitor
  // will detect it and will call stop_monitoring_failing_node()
  zwave_send_nop_to_node(node_id, nullptr, nullptr);

  for (auto it = ping_list.begin(); it != ping_list.end(); ++it) {
    // Existing node, double the back off time
    if (it->node_id == node_id) {
      if ((it->zwave_operating_mode == OPERATING_MODE_AL)
          || (it->zwave_operating_mode == OPERATING_MODE_UNKNOWN)) {
        it->next_ping_in = it->last_ping_in * 2;
      } else if (it->zwave_operating_mode == OPERATING_MODE_FL) {
        it->next_ping_in = it->last_ping_in * 4;
      } else {
        it->next_ping_in = it->last_ping_in * 2;
      }
      it->next_ping_in = (it->next_ping_in > MAX_PING_TIME_INTERVAL)
                           ? MAX_PING_TIME_INTERVAL
                           : it->next_ping_in;

      it->last_ping_in = it->next_ping_in;
    }
    if (zwave_store_failing_node_ping_interval(it->node_id, it->next_ping_in)
        != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "Error persisting the failing node interval in Attribute "
                   "Store for NodeID %d\n",
                   it->node_id);
    } else {
      sl_log_debug(LOG_TAG,
                   "Persisted the failing node interval in Attribute Store "
                   "for NodeID %d\n",
                   it->node_id);
    }
  }

  ping_list.sort();

  last_timer = clock_time();

  sl_log_debug(
    LOG_TAG,
    "Next Failing node ping will be sent to NodeID %d in %lu seconds",
    ping_list.begin()->node_id,
    ping_list.begin()->next_ping_in / CLOCK_SECOND);
  ctimer_set(&ping_timer,
             ping_list.begin()->next_ping_in,
             ping_failing_node,
             0);
}

void stop_monitoring_failing_node(zwave_node_id_t node_id)
{
  for (auto it = ping_list.begin(); it != ping_list.end(); ++it) {
    if (it->node_id == node_id) {
      ping_list.erase(it);
      if (ping_list.empty()) {
        ctimer_stop(&ping_timer);
      }
      sl_log_debug(LOG_TAG,
                   "Removing NodeID %d from failing node monitor list.",
                   node_id);
      zwave_remove_failing_node_ping_interval_from_attribute_store(node_id);
      return;
    }
  }
  sl_log_debug(LOG_TAG, "NodeID %d was not failing", node_id);
  return;
}

static void read_all_nodes_failing_node_ping_interval()
{
  failing_node_t n;

  //Read the home_id for ZPC network
  attribute network = get_zpc_network_node();
  for (auto zw_node: network.children(ATTRIBUTE_NODE_ID)) {
    try {
      if (zw_node == get_zpc_node_id_node()) {
        continue;
      }

      zwave_node_id_t node_id = zw_node.reported<zwave_node_id_t>();

      clock_time_t interval
        = zw_node.child_by_type(ATTRIBUTE_ZWAVE_FAILING_NODE_PING_INTERVAL)
            .reported<clock_time_t>();
      zwave_operating_mode_t mode = zwave_get_operating_mode(node_id);
      n.next_ping_in              = interval;
      n.last_ping_in              = interval;
      n.node_id                   = node_id;
      n.zwave_operating_mode      = mode;
      sl_log_info(LOG_TAG,
                  "Node %d was failing and last interval it was pinged at "
                  "was: : %lu seconds. Continuing from there",
                  n.node_id,
                  n.next_ping_in / CLOCK_SECOND);
      ping_list.push_front(n);
    } catch (const std::invalid_argument &) {
    }
  }
}

sl_status_t zwave_remove_failing_node_ping_interval_from_attribute_store(
  zwave_node_id_t node_id)
{
  attribute node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(node_id);

  try {
    attribute failing_node_interval_node
      = node_id_node.child_by_type(ATTRIBUTE_ZWAVE_FAILING_NODE_PING_INTERVAL);
    failing_node_interval_node.delete_node();
  } catch (const std::invalid_argument &) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

sl_status_t zwave_store_failing_node_ping_interval(zwave_node_id_t node_id,
                                                   clock_time_t interval)
{
  attribute node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(node_id);

  if (!node_id_node.is_valid()) {
    sl_log_debug(LOG_TAG,
                 "Error: NodeID %d was not found in attribute store. "
                 "Removing from the monitor list.",
                 node_id);
    stop_monitoring_failing_node(node_id);
    return SL_STATUS_FAIL;
  }
  try {
    attribute failing_node_interval_node
      = node_id_node.child_by_type(ATTRIBUTE_ZWAVE_FAILING_NODE_PING_INTERVAL);
    failing_node_interval_node.delete_node();

    node_id_node.emplace_node<clock_time_t>(
      ATTRIBUTE_ZWAVE_FAILING_NODE_PING_INTERVAL,
      interval);
    failing_node_interval_node
      = node_id_node.child_by_type(ATTRIBUTE_ZWAVE_FAILING_NODE_PING_INTERVAL);
  } catch (const std::invalid_argument &) {
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

static void adjust_next_ping_for_whole_list()
{
  // Adjust the next ping for all nodes by difference between current_timer and last_timer.
  for (auto it = ping_list.begin(); it != ping_list.end(); ++it) {
    if (it->next_ping_in < (clock_time() - last_timer)) {
      it->next_ping_in = 0;
    } else if (clock_time() > last_timer) {
      it->next_ping_in -= (clock_time() - last_timer);
    } else if (clock_time() < last_timer) {
      it->next_ping_in -= (clock_time() + ~last_timer);
    }
  }
  // set last_timer to current_timer
  last_timer = clock_time();
}

static void ping_failing_node(void *user)
{
  adjust_next_ping_for_whole_list();
  // ping the node who was in front of the list
  if (!ping_list.empty()) {
    send_nop();
  }
}

void start_monitoring_failing_node(zwave_node_id_t node_id)
{
  zwave_operating_mode_t mode = zwave_get_operating_mode(node_id);
  if (mode == OPERATING_MODE_NL) {
    sl_log_debug(LOG_TAG,
                 "NodeID %d is Non Listening. Skipping regular NOP retries.",
                 node_id);
    return;
  }

  sl_log_debug(LOG_TAG,
               "Adding NodeID %d to failing node monitor list",
               node_id);

  for (auto it = ping_list.begin(); it != ping_list.end(); ++it) {
    // Existing node, remove it here and it will be added again to the list below
    if (it->node_id == node_id) {
      ping_list.erase(it);
      break;
    }
  }

  //New node, begin from 4 seconds interval for sending NOP
  failing_node_t n = {};
  if ((mode == OPERATING_MODE_AL) || (mode == OPERATING_MODE_UNKNOWN)) {
    n.next_ping_in = ALWAYS_LISTENING_PING_TIME_INTERVAL;
  } else if (mode == OPERATING_MODE_FL) {
    n.next_ping_in = FREQUENTLY_LISTENING_PING_TIME_INTERVAL;
  } else {  // Safety to handle new zwave operating modes
    n.next_ping_in = ALWAYS_LISTENING_PING_TIME_INTERVAL;
  }

  n.last_ping_in         = n.next_ping_in;
  n.node_id              = node_id;
  n.zwave_operating_mode = mode;

  if (zwave_store_failing_node_ping_interval(node_id, n.next_ping_in)
      != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Error writing the failing node interval in Attribute "
                 "Store for NodeID %d\n",
                 node_id);
  } else {
    sl_log_debug(
      LOG_TAG,
      "Saved the failing node interval in Attribute Store for NodeID %d\n",
      node_id);
  }

  ping_list.push_front(n);
  sl_log_debug(LOG_TAG,
               "Failing Node: %d will be pinged after: %lu seconds",
               n.node_id,
               n.next_ping_in / CLOCK_SECOND);
  // sort the list, so that the timer is set for the minimum next_ping_in timeout.
  ping_list.sort();
  adjust_next_ping_for_whole_list();

  ctimer_set(&ping_timer,
             ping_list.begin()->next_ping_in,
             ping_failing_node,
             0);
}

void print_failing_node_monitor_list()
{
  sl_log_debug(LOG_TAG, "Printing the failing node monitor list");
  for (auto it = ping_list.begin(); it != ping_list.end(); ++it) {
    sl_log_warning(LOG_TAG,
                   "Node: %d, next ping in: %lu, last ping at: %lu",
                   it->node_id,
                   it->next_ping_in,
                   it->last_ping_in);
  }
}

void failing_node_monitor_list_clear()
{
  sl_log_debug(LOG_TAG, "Clearing the failing node monitor list");
  ctimer_stop(&ping_timer);
  ping_list.clear();
}

void failing_node_monitor_list_load()
{
  sl_log_debug(LOG_TAG, "Loading the failing node monitor list");
  ping_list.clear();
  read_all_nodes_failing_node_ping_interval();
  last_timer = clock_time();
  if (!ping_list.empty()) {
    ctimer_set(&ping_timer,
               ping_list.begin()->next_ping_in,
               ping_failing_node,
               0);
  }
}
