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
#include "attribute_resolver.h"
#include "attribute_resolver_internal.h"
#include "attribute_resolver_rule_internal.h"
#include "attribute_resolver_rule_internal.hpp"

#include "attribute.hpp"

// Includes from other components
#include "multi_invoke.hpp"
#include "attribute_store_helper.h"
#include "attribute_store_debug.h"
#include "sl_log.h"
#include "process.h"
#include "clock.h"
#include "etimer.h"

constexpr char LOG_TAG[] = "attribute_resolver";

// Generic includes
#include <unordered_set>
#include <deque>
#include <map>
#include <string>
#include <sstream>

using namespace attribute_store;

typedef enum {
  RESOLVER_NEXT_EVENT,
  RESOLVER_WATCH_EVENT,
  RESOLVER_TIMER_SET_EVENT,
} attribute_resolver_worker_event_t;

static std::deque<std::pair<attribute_store_node_t, uint32_t>> stack;
static std::unordered_set<attribute_store_node_t> paused_nodes;
static multi_invoke<attribute_store_node_t, attribute_store_node_t> listeners;
static attribute_resolver_config_t attribute_resolver_config;

/*
 * Set of nodes for which we have sent a get. The value of the map
 * has information about the number of gets already tried as
 * well as when the get was sent.
 */
typedef struct {
  clock_time_t send_timeout;
  int count;
} pending_get_t;

// Pending Get resolutions
static std::map<attribute_store_node_t, pending_get_t> pending_get_resolutions;
struct etimer pending_get_resume_timer;
// Pending Sets resolutions <map of attribute nodes / needs more frame boolean
static std::map<attribute_store_node_t, bool> pending_set_resolutions;

// Has a full scan of the attribute store been requested
static bool scan_requested;

// Forward declaration for static functions
static bool needs_get(attribute_store_node_t node);
static bool needs_set(attribute_store_node_t node);
static void resolver_find_next_resolve();
static void on_resolver_node_update(attribute_store_node_t node,
                                    attribute_store_change_t change);
static void on_resolver_node_deleted(attribute_store_node_t node);

/**
 * @brief This function traverses attribute store to check if any parent
 *        of the attribute node has a resolution listener
 *
 * @param node  Attribute node
 * @returns   The Attribute Store ID of the first parent with a resolution
 *            listener, if any. It can be the node itself.
 *            ATTRIBUTE_STORE_INVALID_NODE if none if the node and none of its
 *            parents have a resolution listener
 */
static attribute_store_node_t
  get_highest_parent_with_resolution_listener(attribute_store_node_t node);

/**
 * @brief This function returns true if a resolution is ongoing for the node
 *
 * @param node  Attribute node
 * @returns   true if the node is pending a resolution
 *            false if the node is not pending a resolution
 */
static bool is_node_under_resolution(attribute_store_node_t node);

/**
 * @brief Verifies if we gave up trying to resolve a get
 *
 * i.e. we tried up to maximum number of retries and stalled there.
 *
 * @param node  Attribute node
 * @returns     true if the node has reached the maximum number of retries
 *              false if the node has not reached the maximum number of retries
 */
static bool
  is_node_get_resolution_max_retries_reached(attribute_store_node_t node);

/**
 * @brief This function traverses the stack and set the child_index of the common parent to 0.
 *        This enables the attribute resolver to scan updated attribute node.
 *
 * @param node  Attribute node
 */
static void set_common_parent_stack_index_zero(attribute_store_node_t node);

/**
 * @brief This function traverses the list of nodes pending a get resolution
 *        response and restarts a timer expiring when the next node is to be
 *        retried.
 */
static void attribute_resolver_restart_pending_get_nodes_timer();

/**
 * @brief This function traverses the list of nodes pending a get resolution
 *        response and ensures that they get scanned if it's time for them
 *        to get scanned again.
 */
static void attribute_resume_expired_pending_get_nodes();

/**
 * @brief Ensures that a node gets scanned, as part of the current
 *        scan or by starting a new scan.
 */
static void scan_node(attribute_store_node_t node_to_scan);

/**
 * @brief Helper function verifying that all the conditions are met
 *        to schedule a scan for node
 *
 * <!> this only checks for the node, not for its children.
 * It can be that some children need a scan
 *
 * @param node  Attribute node
 * @returns   true if the node must be scanned/resolved.
 *            false if the node does not need to be scanned
 */
static bool is_node_to_be_scanned(attribute_store_node_t node);

// Declare the Contiki Process for the Attribute Resolver
PROCESS(attribute_resolver_process, "attribute_resolver_process");

void attribute_resolver_pause_node_resolution(attribute_store_node_t node)
{
  sl_log_debug(LOG_TAG, "Resolution paused on Attribute ID %d", node);
  paused_nodes.insert(node);
}

void attribute_resolver_resume_node_resolution(attribute_store_node_t node)
{
  paused_nodes.erase(node);

  sl_log_debug(LOG_TAG, "Resolution resumed on Attribute ID %d", node);
  // It can be that the node itself does not need to be scanned, but some
  // unpaused children do. So schedule a scan again unless we are
  // waiting for a response.
  // If a parent of this node is paused, the stack will detect it immediately
  if (is_node_under_resolution(node) == false) {
    scan_node(node);
  }
}

void attribute_resolver_set_resolution_listener(
  attribute_store_node_t node, void (*callback)(attribute_store_node_t))
{
  if (node == ATTRIBUTE_STORE_INVALID_NODE) {
    return;
  }
  listeners.add(node, callback);

  // Ensure that if somebody registers a listener while no scan is ongoing
  // and nothing needs to be resolved, they get a notification for their node
  // as soon as possible
  scan_node(node);
}

void attribute_resolver_clear_resolution_listener(
  attribute_store_node_t node, void (*callback)(attribute_store_node_t))
{
  listeners.remove(node, callback);
}

sl_status_t
  attribute_resolver_restart_set_resolution(attribute_store_node_t node)
{
  if (pending_set_resolutions.count(node) > 0) {
    pending_set_resolutions.erase(node);
    attribute_resolver_resume_node_resolution(node);
    // here it's important to ask the lower layers to abort as well,
    // as some components (e.g. zpc_resolver_group start resolving sets)
    // before we ask them!
    sl_status_t abort_status = attribute_resolver_config.abort(node);
    scan_node(node);
    return abort_status;
  }
  return SL_STATUS_NOT_FOUND;
}

attribute_resolver_config_t attribute_resolver_get_config()
{
  return attribute_resolver_config;
}

bool attribute_resolver_node_or_child_needs_resolution(
  attribute_store_node_t node)
{
  if (needs_get(node)) {
    const auto it = pending_get_resolutions.find(node);
    if ((it == pending_get_resolutions.end())
        || (is_node_get_resolution_max_retries_reached(node) == false)) {
      return true;
    }
  }

  if (needs_set(node)) {
    return true;
  }

  for (uint32_t i = 0; i < attribute_store_get_node_child_count(node); i++) {
    if (true
        == attribute_resolver_node_or_child_needs_resolution(
          attribute_store_get_node_child(node, i))) {
      return true;
    }
  }
  return false;
}

bool is_node_or_parent_paused(attribute_store_node_t node)
{
  for (attribute updated_node = node; updated_node.is_valid();
       updated_node           = updated_node.parent()) {
    if (paused_nodes.count(updated_node)) {
      return true;
    }
  }
  return false;
}

bool is_node_pending_set_resolution(attribute_store_node_t node)
{
  return pending_set_resolutions.count(node) > 0;
}

void attribute_resolver_state_log()
{
  sl_log_debug(LOG_TAG, "Scan Requested: %d", scan_requested);
  if (stack.empty()) {
    sl_log_debug(LOG_TAG, "Stack is empty");
  } else {
    sl_log_debug(LOG_TAG, "Stack next node: %d", stack.back().first);
  }

  std::stringstream stream;
  for (auto it = paused_nodes.begin(); it != paused_nodes.end(); ++it) {
    stream << *it;
    stream << " ";
  }

  std::string message = stream.str();
  sl_log_debug(LOG_TAG,
               "Paused nodes: %lu - [%s]",
               paused_nodes.size(),
               message.c_str());

  sl_log_debug(LOG_TAG,
               "Nodes pending Get resolution: %lu",
               pending_get_resolutions.size());
  for (auto it = pending_get_resolutions.begin();
       it != pending_get_resolutions.end();
       ++it) {
    sl_log_debug(LOG_TAG,
                 "\t Node %d, timeout: %d, retries: %d",
                 it->first,
                 it->second.send_timeout,
                 it->second.count);
  }
  if (!etimer_expired(&pending_get_resume_timer)) {
    sl_log_debug(LOG_TAG, "Get Retry Timer is running ");
  } else {
    sl_log_debug(LOG_TAG, "Get Retry Timer is not running ");
  }

  stream.str("");
  stream.clear();
  for (auto it = pending_set_resolutions.begin();
       it != pending_set_resolutions.end();
       ++it) {
    stream << it->first;
    stream << " (more frames = ";
    stream << it->second;
    stream << "),";
  }
  message = stream.str();
  sl_log_debug(LOG_TAG,
               "Nodes pending Set resolution: %lu - [%s]",
               pending_set_resolutions.size(),
               message.c_str());
}

/**
 * @brief Check if we all ready have a get or set resolution for this attribute node.
 *
 * @param rule_type SET or GET
 * @param node Node to check
 * @return true There is already a pending resolution for this node.
 * @return false  No pending resolution will resolve this.
 */
static bool resolution_already_queued(resolver_rule_type_t rule_type,
                                      attribute node)
{
  std::vector<attribute_store_node_t> group_nodes
    = attribute_resolver_rule_get_group_nodes(rule_type, node);

  for (attribute a: group_nodes) {
    if ((rule_type == RESOLVER_GET_RULE) && pending_get_resolutions.count(a)) {
      sl_log_debug(LOG_TAG,
                   "Already resolving GET %s vs %s id %i",
                   attribute_store_name_by_type(a.type()),
                   attribute_store_name_by_type(node.type()),
                   a);
      return true;
    }
    if ((rule_type == RESOLVER_SET_RULE) && pending_set_resolutions.count(a)) {
      sl_log_debug(LOG_TAG,
                   "Already resolving SET %s id %i",
                   attribute_store_name_by_type(a.type()),
                   a);
      return true;
    }
  }
  return false;
}

/**
 * @brief Helper function executing a Get rule.
 *
 *
 * @param node  Attribute node that needs a get resolution
 * @returns   SL_STATUS_ABORT if we gave up trying to resolve this node
 *            \ref attribute_resolver_rule_execute return code otherwise
 */
static sl_status_t execute_get(attribute_store_node_t node)
{
  if (pending_get_resolutions.count(node)) {
    if (pending_get_resolutions[node].count
        >= attribute_resolver_config.get_retry_count) {
      //If we exceed the retry count don't try this anymore
      sl_log_info(LOG_TAG,
                  "Maximum amount of retries reached for Attribute ID "
                  "%d. Giving up.\n",
                  node);
      return SL_STATUS_ABORT;
    } else if ((clock_time() > pending_get_resolutions[node].send_timeout)
               && (pending_get_resolutions[node].send_timeout != 0)) {
      pending_get_resolutions[node].count++;
      sl_log_debug(LOG_TAG,
                   "Retransmitting Get command for Attribute ID %d "
                   "(attempt %i out of %i)",
                   node,
                   pending_get_resolutions[node].count,
                   attribute_resolver_config.get_retry_count);
    } else {
      // We have to wait before a retry.
      return SL_STATUS_IS_WAITING;
    }
  } else {
    if (resolution_already_queued(RESOLVER_GET_RULE, node)) {
      return SL_STATUS_IS_WAITING;
    } else {
      pending_get_resolutions[node] = {.send_timeout = 0, .count = 1};
    }
  }

  return attribute_resolver_rule_execute(node, false);
}

/**
 * @brief Helper function executing a Set rule.
 *
 *
 * @param node  Attribute node that needs a set resolution
 * @returns   \ref attribute_resolver_rule_execute return code
 *              SL_STATUS_IS_WAITING if the resolver just has to wait and do
 *              nothing about this attribute.
 */
static sl_status_t execute_set(attribute_store_node_t node)
{
  // If it is pending a set (i.e. WORKING), we don't try to resolve again.
  if (pending_set_resolutions.count(node)) {
    return SL_STATUS_IS_WAITING;
  }

  if (resolution_already_queued(RESOLVER_SET_RULE, node)) {
    return SL_STATUS_IS_WAITING;
  }

  // If we are about set this value, reset the pending get resolution.
  pending_get_resolutions.erase(node);

  sl_status_t rule_status = attribute_resolver_rule_execute(node, true);

  if (rule_status == SL_STATUS_OK) {
    pending_set_resolutions[node] = false;
  }
  if (rule_status == SL_STATUS_IN_PROGRESS) {
    pending_set_resolutions[node] = true;
  }

  return rule_status;
}

/**
 * @brief This function traverses attribute store iteratively by using a stack.
 * Given the nature of a stack, the nodes that get pushed will be visited later
 * on, that is simulating backtracking of recursion.
 *
 * During traversal, all the stack elements are considered as root of a subtree
 * and all their children will be visited. Therefore, if visiting the whole is
 * intended, root node should be pushed before calling this function.
 * E.g.
 *    traverse_stack.push_back(ROOT_NODE);
 *    resolver_find_next_resolve();
 *
 * - Why iteratively traversing, not recursively?
 *   In order to be have asynchronous traversal, the plan is to resolve one node
 *   at a time. The possibility of saving/restoring state of traversal is needed
 *   So in this case, iteratively traversing is easier to achieve the goal.
 *
 * A given node in the tree will only be executed when all its children has
 * been executed. Ie for the tree:
 *
 *         A
 *        / \
 *        B   E
 *       / \
 *      C   D
 *
 * The execution order will be: C D B E A
 */
static void resolver_find_next_resolve()
{
  while (!stack.empty()) {
    attribute_store_node_t node = stack.back().first;
    uint32_t index              = stack.back().second;

    //Node is paused skip this and all its children
    if (is_node_or_parent_paused(node)) {
      stack.pop_back();
      continue;
    }

    // Verify that our stack is not out of sync with the Attribute Store
    if (false == attribute_store_node_exists(node)) {
      stack.pop_back();
      continue;
    }
    if (index > attribute_store_get_node_child_count(node)) {
      stack.back().second = 0;
      index               = 0;
    }

    // All children has been visited
    if (index == attribute_store_get_node_child_count(node)) {
      stack.pop_back();
      // Get operations have priority.
      // Some resolvers need proper state information before they
      // can set.
      sl_status_t rule_status = SL_STATUS_NOT_INITIALIZED;
      if (needs_get(node)) {
        sl_log_debug(LOG_TAG, "Attribute ID %d needs a Get\n", node);
        rule_status = execute_get(node);

        if (rule_status == SL_STATUS_ABORT) {
          continue;
        }

      } else if (needs_set(node)) {
        sl_log_debug(LOG_TAG, "Attribute ID %d needs a Set\n", node);
        rule_status = execute_set(node);
      }

      // If a rule was executed successfully, we return and wait for a callback
      if (rule_status == SL_STATUS_OK || rule_status == SL_STATUS_IN_PROGRESS) {
        return;
      }
      // Resolution of this node is done with no frame
      if (rule_status == SL_STATUS_NOT_INITIALIZED
          || rule_status == SL_STATUS_ALREADY_EXISTS) {
        on_resolver_rule_execute_complete(node, 0);
        return;
      }

      if (rule_status == SL_STATUS_IS_WAITING) {
        sl_log_debug(LOG_TAG,
                     "Attribute ID %d resolution is working. Skipping\n",
                     node);
        continue;
      }

      sl_log_info(LOG_TAG,
                  "Unexpected Resolver rule status: 0x%02X for node %d.",
                  rule_status,
                  node);

    } else {
      // Increment the child index
      stack.back().second++;
      // Add the child onto the stack.
      stack.push_back(std::pair<attribute_store_node_t, int>(
        attribute_store_get_node_child(node, index),
        0));
    }
  }
  sl_log_debug(LOG_TAG, "Attribute Store scan completed.");
}

// Check if attribute node needs get
//
static bool needs_get(attribute_store_node_t node)
{
  if (attribute_resolver_has_get_rule(attribute_store_get_node_type(node))
      == false) {
    return false;
  }

  if (attribute_store_is_value_defined(node, REPORTED_ATTRIBUTE)) {
    return false;
  } else {
    return true;
  }
}

// Returns true if attribute needs a set operation
//
static bool needs_set(attribute_store_node_t node)
{
  if (attribute_resolver_has_set_rule(attribute_store_get_node_type(node))
      == false) {
    return false;
  }

  if (!attribute_store_is_value_defined(node, DESIRED_ATTRIBUTE)
      || attribute_store_is_value_matched(node)) {
    return false;
  } else {
    return true;
  }
}

/**
 * @brief Callback function registered to attribute store when node has updates
 */
static void on_resolver_node_update(attribute_store_node_t node,
                                    attribute_store_change_t change)
{
  //If the node has been deleted make sure to clear it from our watch
  if ((change == ATTRIBUTE_DELETED)
      || (attribute_store_get_node_type(node)
          == ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE)) {
    on_resolver_node_deleted(node);
    return;
  }
  // If the node is blocked due to maximum retry count and it was just updated
  // try to see if resolution is needed or if it works now.
  if ((!needs_get(node))
      || (is_node_get_resolution_max_retries_reached(node) == true)) {
    pending_get_resolutions.erase(node);
  }

  // If the node was updated and does not need a set anymore, we remove it from
  // the pending sets
  if (!needs_set(node)) {
    pending_set_resolutions.erase(node);
  }

  // See if we need to scan something.
  // if somebody is waiting for a resolution notification, (possibly above the current node)
  // ensure that the node gets scanned again
  attribute_store_node_t parent_with_listener
    = get_highest_parent_with_resolution_listener(node);
  if (parent_with_listener != ATTRIBUTE_STORE_INVALID_NODE) {
    scan_node(parent_with_listener);
  } else if (is_node_to_be_scanned(node)) {
    scan_node(node);
  }
}

static void on_resolver_node_deleted(attribute_store_node_t node)
{
  // Remove listeners and also from the pause resolution set.
  listeners.erase(node);
  paused_nodes.erase(node);
  pending_get_resolutions.erase(node);
  pending_set_resolutions.erase(node);
  // Don't remove the node from the stack, the stack will
  // detect missing nodes.
  // Tell the Resolver Rule part to stop waiting for a callback for deleted nodes.
  attribute_resolver_rule_abort(node);
}

static bool is_node_under_resolution(attribute_store_node_t node)
{
  if (pending_get_resolutions.count(node) == 0
      && pending_set_resolutions.count(node) == 0) {
    return false;
  } else {
    return true;
  }
}

static bool
  is_node_get_resolution_max_retries_reached(attribute_store_node_t node)
{
  const auto it = pending_get_resolutions.find(node);
  if (it != pending_get_resolutions.end()
      && it->second.count >= attribute_resolver_config.get_retry_count) {
    return true;
  }
  return false;
}

static attribute_store_node_t
  get_highest_parent_with_resolution_listener(attribute_store_node_t node)
{
  attribute_store_node_t parent_with_listener = ATTRIBUTE_STORE_INVALID_NODE;
  while (node != ATTRIBUTE_STORE_INVALID_NODE) {
    if (listeners.contains(node)) {
      parent_with_listener = node;
    }
    node = attribute_store_get_node_parent(node);
  }
  return parent_with_listener;
}

static void set_common_parent_stack_index_zero(attribute_store_node_t node)
{
  for (attribute updated_node = node; updated_node.is_valid();
       updated_node           = updated_node.parent()) {
    for (uint16_t i = 0; i < stack.size(); i++) {
      if (stack[i].first == updated_node) {
        stack[i].second = 0;
        return;
      }
    }
  }
}

static void scan_node(attribute_store_node_t node_to_scan)
{
  // If a scan is progress, we search the
  // common parent node in the stack and set he child_index of the common parent to 0,
  // this will cause do updated node to be visited.
  if (!stack.empty()) {
    set_common_parent_stack_index_zero(node_to_scan);
    //process_post(&attribute_resolver_process, RESOLVER_NEXT_EVENT, nullptr);
  } else if (scan_requested == false) {
    scan_requested = true;
    process_post(&attribute_resolver_process, RESOLVER_NEXT_EVENT, nullptr);
  }
}

static void attribute_resume_expired_pending_get_nodes()
{
  // Go through all the nodes pending a Get Command response
  for (auto it = pending_get_resolutions.begin();
       it != pending_get_resolutions.end();
       ++it) {
    attribute_store_node_t node = it->first;

    if (is_node_to_be_scanned(node) == true) {
      scan_node(node);
    }
  }

  // Ensure that the timer is running, if needed
  attribute_resolver_restart_pending_get_nodes_timer();
}

static void attribute_resolver_restart_pending_get_nodes_timer()
{
  // If the timer is already running, we trust that it's running against the
  // closest deadline (when inserting new nodes, they will have a later deadline
  // for retry). If it's not the case, it's not too bad, there will just be
  // an extra delay for the node that has a closer deadline
  if (!etimer_expired(&pending_get_resume_timer)) {
    return;
  }

  // Find out among all candidates, the node that is next to "expire"
  // and can be retried.
  clock_time_t next_deadline = 0;
  clock_time_t now           = clock_time();

  // Go through all the nodes pending a Get Command response
  for (auto it = pending_get_resolutions.begin();
       it != pending_get_resolutions.end();
       ++it) {
    pending_get_t pending_data  = it->second;
    attribute_store_node_t node = it->first;

    // Do not restart a timer if the node should not be retried.
    if (pending_data.count >= attribute_resolver_config.get_retry_count
        || is_node_or_parent_paused(node) == true || !needs_get(node)) {
      continue;
    }

    if (pending_data.send_timeout != 0) {
      if (pending_data.send_timeout <= now) {
        // Deadline is past, just schedule a scan directly
        scan_node(node);
      } else if (pending_data.send_timeout < next_deadline
                 || next_deadline == 0) {
        // Save the next deadline in the list
        next_deadline = pending_data.send_timeout;
      }
    }
  }

  // Do we have a next deadline?
  if (next_deadline != 0 && now < next_deadline) {
    //restart the timer for 1ms, if the next deadline is in the past.
    clock_time_t time_until_deadline = next_deadline - now;
    process_post(&attribute_resolver_process,
                 RESOLVER_TIMER_SET_EVENT,
                 (void *)time_until_deadline);
  }
}

static bool is_node_to_be_scanned(attribute_store_node_t node)
{
  // Verify if it is paused
  if (is_node_or_parent_paused(node) == true) {
    return false;
  }

  // If it needs a get. Get operations have priority
  if (needs_get(node)) {
    // Is it pending a get-response
    if (pending_get_resolutions.count(node)) {
      if ((pending_get_resolutions[node].count
           < attribute_resolver_config.get_retry_count)
          && (clock_time() >= pending_get_resolutions[node].send_timeout)
          && (pending_get_resolutions[node].send_timeout != 0)) {
        return true;
      }
    } else {
      // if it needs a get and not pending a response, we need to scan
      return true;
    }
  } else if (needs_set(node) && pending_set_resolutions.count(node) == 0) {
    return true;
  }

  return false;
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Resolver private functions
///////////////////////////////////////////////////////////////////////////////
void on_resolver_rule_execute_complete(attribute_store_node_t node,
                                       clock_time_t transmission_time)
{
  if (pending_get_resolutions.count(node) && needs_get(node)) {
    clock_time_t retry_time
      = transmission_time + attribute_resolver_config.get_retry_timeout;
    pending_get_resolutions[node].send_timeout = clock_time() + retry_time;

    // Ensure that the timer is running
    attribute_resolver_restart_pending_get_nodes_timer();
  }
  if (pending_set_resolutions.count(node)
      && (pending_set_resolutions[node] == true)) {
    // Multi-frame resolutions, just remove it from the pending sets and try again.
    pending_set_resolutions.erase(node);
    scan_node(node);
  }
  // Resolution of this node is done (everything is resolved or we gave up trying)
  if (listeners.contains(node)
      && !attribute_resolver_node_or_child_needs_resolution(node)) {
    process_post(&attribute_resolver_process,
                 RESOLVER_WATCH_EVENT,
                 reinterpret_cast<void *>(static_cast<uintptr_t>(node)));
  }

  // Process the next element in our scan
  if (!stack.empty()) {
    process_post(&attribute_resolver_process, RESOLVER_NEXT_EVENT, nullptr);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Attribute Resolver public functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t
  attribute_resolver_register_rule(attribute_store_type_t node_type,
                                   attribute_resolver_function_t set_func,
                                   attribute_resolver_function_t get_func)
{
  attribute_resolver_rule_register(node_type, set_func, get_func);

  // Just verify the tree, in case some new rules can be applied.
  if (stack.empty() && (scan_requested == false)) {
    scan_requested = true;
    process_post(&attribute_resolver_process, RESOLVER_NEXT_EVENT, nullptr);
  }

  return SL_STATUS_OK;
}

static sl_status_t
  attribute_resolver_config_init(attribute_resolver_config_t resolver_config)
{
  // Allow send_init to be NULL
  if (resolver_config.send == NULL) {
    return SL_STATUS_FAIL;
  }
  attribute_resolver_config
    = {.send_init         = resolver_config.send_init,
       .send              = resolver_config.send,
       .abort             = resolver_config.abort,
       .get_retry_timeout = resolver_config.get_retry_timeout,
       .get_retry_count   = resolver_config.get_retry_count};
  return SL_STATUS_OK;
}

sl_status_t attribute_resolver_init(attribute_resolver_config_t resolver_config)
{
  // Set everything to 0
  listeners.clear();
  paused_nodes.clear();
  pending_get_resolutions.clear();
  pending_set_resolutions.clear();
  stack.clear();
  etimer_stop(&pending_get_resume_timer);
  scan_requested = true;

  attribute_resolver_rule_init(on_resolver_rule_execute_complete);
  if (attribute_resolver_config_init(resolver_config) == SL_STATUS_FAIL) {
    return SL_STATUS_FAIL;
  }
  if (attribute_resolver_config.send_init != NULL) {
    attribute_resolver_config.send_init();
  }

  // Tell the Attribute Store that we want to be notified of everything
  attribute_store_register_callback(on_resolver_node_update);

  process_start(&attribute_resolver_process, 0);

  return SL_STATUS_OK;
}

int attribute_resolver_teardown()
{
  process_exit(&attribute_resolver_process);
  return 0;
}

/************************ process stuff ****************************/

static void attribute_resolver_ev_init()
{
  sl_log_debug(LOG_TAG, "Init of attribute resolver");
  process_post(&attribute_resolver_process, RESOLVER_NEXT_EVENT, nullptr);
}

static void attribute_resolver_ev_next()
{
  if (stack.empty() && scan_requested) {
    sl_log_debug(LOG_TAG, "Starting scan from the top");
    scan_requested = false;
    stack.push_back(
      std::pair<attribute_store_node_t, int>(attribute_store_get_root(), 0));
  }
  // Find next node to resolve
  if (!attribute_resolver_rule_busy()) {
    resolver_find_next_resolve();
  }
}

static void attribute_resolver_ev_watch(const attribute_store_node_t node)
{
  listeners(node, node);
}

///////////////////////////////////////////////////////////////////////////////
// Contiki Process thread
///////////////////////////////////////////////////////////////////////////////
PROCESS_THREAD(attribute_resolver_process, ev, data)
{
  PROCESS_BEGIN();
  while (1) {
    if (ev == PROCESS_EVENT_INIT) {
      attribute_resolver_ev_init();
    } else if (ev == PROCESS_EVENT_EXIT) {
      sl_log_debug(LOG_TAG, "Teardown of attribute resolver");
    } else if (ev == RESOLVER_NEXT_EVENT) {
      attribute_resolver_ev_next();
    } else if (ev == RESOLVER_WATCH_EVENT) {
      attribute_resolver_ev_watch(
        static_cast<attribute_store_node_t>(reinterpret_cast<uintptr_t>(data)));
    } else if ((ev == PROCESS_EVENT_TIMER)
               && (data == &pending_get_resume_timer)) {
      attribute_resume_expired_pending_get_nodes();
    } else if (ev == RESOLVER_TIMER_SET_EVENT) {
      sl_log_debug(LOG_TAG,
                   "Restarting timer for pending Get resolutions. Next "
                   "attempt in %lu ms",
                   (clock_time_t)data);
      etimer_set(&pending_get_resume_timer, (clock_time_t)data);
    }

    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}
