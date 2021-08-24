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
#include "attribute_resolver_rule_internal.h"
#include "attribute_resolver_rule_internal.hpp"
#include "attribute_resolver_rule.h"
#include "attribute.hpp"
// Generic includes
#include <set>
#include <map>
#include <vector>

// UIC Components
#include "attribute_store_helper.h"
#include "process.h"
#include "sl_log.h"

constexpr char LOG_TAG[] = "attribute_resolver_rule";

struct attribute_rule {
  attribute_resolver_function_t set_func;
  attribute_resolver_function_t get_func;
};

constexpr size_t MAX_FRAME_LEN = 255;

static enum {
  RESOLVER_IDLE,
  RESOLVER_EXECUTING_SET_RULE,
  RESOLVER_EXECUTING_GET_RULE,
} resolver_state;

static std::map<attribute_store_type_t, struct attribute_rule> rule_book;

/* Set of attributes in the same report message, this is deducted form the get function */
static std::multimap<attribute_resolver_function_t, attribute_store_type_t>
  get_group;

/* Set of attribute in the same set function. */
static std::multimap<attribute_resolver_function_t, attribute_store_type_t>
  set_group;

static attribute_rule_complete_t compl_func;
static attribute_store_node_t node_pending_resolution
  = ATTRIBUTE_STORE_INVALID_NODE;
static bool node_needs_more_frames = false;

// We have a list of "callback" functions,
// that we notify when we get new "set" rules
static std::set<resolver_on_set_rule_registered_t> set_rule_listeners;

/**
 * @brief Prints the name of the resolution status code (resolver_send_status_t)

 * @param status resolver_send_status_t that we want a string representation of
 * @return const char*
 */
static const char *
  attribute_resolver_get_send_status_name(resolver_send_status_t status)
{
  switch (status) {
    case RESOLVER_SEND_STATUS_OK:
      return "Transmit OK, No application status";
    case RESOLVER_SEND_STATUS_OK_EXECUTION_PENDING:
      return "Transmit OK, Application working";
    case RESOLVER_SEND_STATUS_FAIL:
      return "Transmit Failed";
    case RESOLVER_SEND_STATUS_OK_EXECUTION_VERIFIED:
      return "Transmit OK, Application OK";
    case RESOLVER_SEND_STATUS_OK_EXECUTION_FAILED:
      return "Transmit OK, Application Failed";
    case RESOLVER_SEND_STATUS_ALREADY_HANDLED:
      return "Already handled (custom handler)";
    case RESOLVER_SEND_STATUS_ABORTED:
      return "Aborted";
    default:
      static std::string message = "Unknown status: " + std::to_string(status);
      return message.c_str();
  }
}

/**
 * @brief Prints the name of a rule type

 * @param status resolver_send_status_t that we want a string representation of
 * @return const char*
 */
static const char *
  attribute_resolver_get_rule_type_name(resolver_rule_type_t rule_type)
{
  switch (rule_type) {
    case RESOLVER_SET_RULE:
      return "Set rule";
    case RESOLVER_GET_RULE:
      return "Get rule";
    default:
      static std::string message
        = "Unknown rule type: " + std::to_string(rule_type);
      return message.c_str();
  }
}

/**
 * @brief Find other attribute belonging to the same rule.

 * @param rule_type
 * @param attribute_type
 * @return std::set<attribute_store_type_t>
 */
static void
  attribute_resolver_rule_get_group(resolver_rule_type_t rule_type,
                                    attribute_store_type_t attribute_type,
                                    std::vector<attribute_store_type_t> &result)
{
  //do we have the attribute in the rule book
  if (rule_book.count(attribute_type)) {
    if (rule_type == RESOLVER_GET_RULE) {
      attribute_resolver_function_t f = rule_book.at(attribute_type).get_func;
      const auto range                = get_group.equal_range(f);
      for (auto i = range.first; i != range.second; i++) {
        result.push_back(i->second);
      }
    } else if (rule_type == RESOLVER_SET_RULE) {
      attribute_resolver_function_t f = rule_book.at(attribute_type).set_func;
      const auto range                = set_group.equal_range(f);
      for (auto i = range.first; i != range.second; i++) {
        result.push_back(i->second);
      }
    }
  }
}

std::vector<attribute_store_node_t>
  attribute_resolver_rule_get_group_nodes(resolver_rule_type_t rule_type,
                                          attribute_store_node_t _node)
{
  std::vector<attribute_store_node_t> result;
  std::vector<attribute_store_type_t> group_types;
  attribute_store::attribute node   = _node;
  attribute_store::attribute parent = node.parent();

  attribute_resolver_rule_get_group(rule_type, node.type(), group_types);
  for (const auto &t: group_types) {
    // UIC-938 fix this hardcoded navigation.
    attribute_store::attribute a = parent.child_by_type(t);

    if (a.is_valid()) {
      result.push_back(a);
    }
  }
  return result;
}

void on_resolver_send_data_complete(resolver_send_status_t status,
                                    clock_time_t transmission_time,
                                    attribute_store_node_t _node,
                                    resolver_rule_type_t rule_type)
{
  using namespace attribute_store;
  attribute node = _node;

  // "Needs more frames" is only saved for the node pending resolution.
  bool needs_more_frames = false;
  if ((_node == node_pending_resolution)
      && (_node != ATTRIBUTE_STORE_INVALID_NODE)) {
    needs_more_frames = node_needs_more_frames;
  }

  sl_log_debug(LOG_TAG,
               "%s send data complete for Attribute ID %d: "
               "Status: %s - more_frames = %d",
               attribute_resolver_get_rule_type_name(rule_type),
               node,
               attribute_resolver_get_send_status_name(status),
               needs_more_frames);

  if (rule_type == RESOLVER_SET_RULE) {
    // Log the state of the node, so we can see the transformation operated here.
    attribute_store_log_node(node, false);
  }

  switch (status) {
    case RESOLVER_SEND_STATUS_OK:
      if (rule_type == RESOLVER_SET_RULE) {
        // We need to update the reported value, undefine it to trigger
        // a get rule.
        attribute_store_undefine_reported(node);
        if (needs_more_frames == false) {
          for (attribute_store::attribute a:
               attribute_resolver_rule_get_group_nodes(rule_type, node)) {
            // Now we can align the reported to the desired value
            a.clear_reported();
            a.clear_desired();
            attribute_store_log_node(node, false);
          }
        }
      }
      break;

    case RESOLVER_SEND_STATUS_FAIL:
      // Roll back the desired, if it was a set rule.
      if (rule_type == RESOLVER_SET_RULE) {
        for (attribute_store::attribute a:
             attribute_resolver_rule_get_group_nodes(rule_type, node)) {
          // Now we can align the reported to the desired value
          a.clear_desired();
          attribute_store_log_node(a, false);
        }
      }
      break;

    case RESOLVER_SEND_STATUS_OK_EXECUTION_PENDING:
      // We just wait for a subsequent callback. Nothing to do here.
      break;

    case RESOLVER_SEND_STATUS_OK_EXECUTION_VERIFIED:
      if (rule_type == RESOLVER_SET_RULE) {
        if (needs_more_frames == false) {
          for (attribute_store::attribute a:
               attribute_resolver_rule_get_group_nodes(rule_type, node)) {
            // Now we can align the reported to the desired value
            a.set_reported(a.desired_or_reported<std::vector<uint8_t>>());
            a.clear_desired();
            attribute_store_log_node(a, false);
          }
        }
      }
      break;
    case RESOLVER_SEND_STATUS_OK_EXECUTION_FAILED:
      if (rule_type == RESOLVER_SET_RULE) {
        if (needs_more_frames == false) {
          for (attribute_store::attribute a:
               attribute_resolver_rule_get_group_nodes(rule_type, node)) {
            // Now we can align the reported to the desired value
            a.clear_reported();
            a.clear_desired();
            attribute_store_log_node(node, false);
          }
        }
      }
      break;
    case RESOLVER_SEND_STATUS_ALREADY_HANDLED:
    case RESOLVER_SEND_STATUS_ABORTED:
      break;
  }

  // It's an initial callback for a node, so that we can execute the next rule
  if (node_pending_resolution == _node) {
    node_pending_resolution = ATTRIBUTE_STORE_INVALID_NODE;
    resolver_state          = RESOLVER_IDLE;
    compl_func(_node, transmission_time);
  }
}

void attribute_resolver_register_set_rule_listener(
  resolver_on_set_rule_registered_t function)
{
  set_rule_listeners.insert(function);
  // If they register themselves late, we notify of already existing rules.
  for (auto it = rule_book.begin(); it != rule_book.end(); ++it) {
    if (it->second.set_func != nullptr) {
      function(it->first);
    }
  }
}

sl_status_t attribute_resolver_rule_execute(attribute_store_node_t node,
                                            bool set_rule)
{
  uint8_t frame[MAX_FRAME_LEN] = {0};
  uint16_t frame_size          = 0;

  attribute_store_type_t attribute_type = attribute_store_get_node_type(node);

  if (rule_book.find(attribute_type) == rule_book.end()) {
    //No rule for this attribute
    return SL_STATUS_NOT_FOUND;
  }

  if (attribute_resolver_rule_busy() == true) {
    return SL_STATUS_BUSY;
  }

  attribute_resolver_function_t func = set_rule
                                         ? rule_book[attribute_type].set_func
                                         : rule_book[attribute_type].get_func;

  if (func) {
    try {
      sl_status_t frame_status = func(node, frame, &frame_size);
      if ((frame_status == SL_STATUS_OK)
          || (frame_status == SL_STATUS_IN_PROGRESS)) {
        // Verify that frame size is not too big.
        if (frame_size > sizeof(frame)) {
          return SL_STATUS_WOULD_OVERFLOW;
        }

        // Transmit the rule payload
        if (attribute_resolver_get_config().send(node,
                                                 frame,
                                                 frame_size,
                                                 set_rule)
            == SL_STATUS_OK) {
          resolver_state          = set_rule ? RESOLVER_EXECUTING_SET_RULE
                                             : RESOLVER_EXECUTING_GET_RULE;
          node_pending_resolution = node;
          if (frame_status == SL_STATUS_IN_PROGRESS) {
            node_needs_more_frames = true;
            return SL_STATUS_IN_PROGRESS;
          } else {
            node_needs_more_frames = false;
            return SL_STATUS_OK;
          }

        } else {
          resolver_state = RESOLVER_IDLE;
          return SL_STATUS_NOT_READY;
        }
      }
      // Other frame status codes
      if (frame_status == SL_STATUS_ALREADY_EXISTS) {
        resolver_state = RESOLVER_IDLE;
        return SL_STATUS_ALREADY_EXISTS;
      } else if (frame_status == SL_STATUS_IS_WAITING) {
        resolver_state = RESOLVER_IDLE;
        return SL_STATUS_IS_WAITING;
      }
      sl_log_info(
        LOG_TAG,
        "Unexpected frame status: 0x%02X. Please verify that the frame "
        "resolution function for node type 0x%X respects the return codes!",
        frame_status,
        attribute_type);
    } catch (const std::exception &e) {
      sl_log_debug("Exception in rule execution %s", e.what());
    }
  }
  return SL_STATUS_NOT_SUPPORTED;
}

void attribute_resolver_rule_register(attribute_store_type_t node_type,
                                      attribute_resolver_function_t set_func,
                                      attribute_resolver_function_t get_func)
{
  rule_book[node_type] = {set_func, get_func};

  if (get_func) {
    get_group.insert(std::make_pair(get_func, node_type));
  }

  if (set_func) {
    set_group.insert(std::make_pair(set_func, node_type));
  }

  // Notify set rule listeners
  if (set_func != nullptr) {
    for (auto it = set_rule_listeners.begin(); it != set_rule_listeners.end();
         ++it) {
      (*it)(node_type);
    }
  }
}

void attribute_resolver_rule_init(attribute_rule_complete_t __compl_func)
{
  get_group.clear();
  set_group.clear();
  compl_func = __compl_func;
  rule_book.clear();
  resolver_state = RESOLVER_IDLE;
}

bool attribute_resolver_rule_busy()
{
  if (resolver_state != RESOLVER_IDLE) {
    sl_log_debug(LOG_TAG,
                 "Resolver busy! waiting for node %d",
                 node_pending_resolution);
    attribute_store_log_node(node_pending_resolution, false);
  }
  return resolver_state != RESOLVER_IDLE;
}

void attribute_resolver_rule_abort(attribute_store_node_t node)
{
  // Abort executing a rule (or waiting for a callback)
  if (node_pending_resolution == node) {
    node_pending_resolution = ATTRIBUTE_STORE_INVALID_NODE;
    resolver_state          = RESOLVER_IDLE;
    compl_func(node, 0);
  }
}

attribute_resolver_function_t
  attribute_resolver_set_function(attribute_store_type_t node_type)
{
  if (rule_book.find(node_type) == rule_book.end()) {
    // No rule at all for this attribute
    return nullptr;
  } else {
    return rule_book[node_type].set_func;
  }
}

attribute_resolver_function_t
  attribute_resolver_get_function(attribute_store_type_t node_type)
{
  if (rule_book.find(node_type) == rule_book.end()) {
    // No rule at all for this attribute
    return nullptr;
  } else {
    return rule_book[node_type].get_func;
  }
}

bool attribute_resolver_has_set_rule(attribute_store_type_t node_type)
{
  return attribute_resolver_set_function(node_type) != nullptr;
}

bool attribute_resolver_has_get_rule(attribute_store_type_t node_type)
{
  return attribute_resolver_get_function(node_type) != nullptr;
}
