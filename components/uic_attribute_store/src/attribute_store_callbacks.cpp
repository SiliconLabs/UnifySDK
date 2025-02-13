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
#include "attribute_store.h"
#include "attribute_store_callbacks.h"

// Generic includes
#include <map>
#include <set>
#include <string.h>

// Includes from other components
#include "sl_log.h"

// Cpp
#include "attribute_callbacks.hpp"

/// Setup Log tag
constexpr char LOG_TAG[] = "attribute_store_callbacks";

/**
 * @brief A structure that's kept for registering callbacks settings.
 * There are 2 possible settings/filters that can be used in parallels:
 * - Callback when matching the type of a node only
 * - Callback when matching the value-state and type of a node
 */
using attribute_store_value_callback_setting_t
  = std::pair<attribute_store_type_t, attribute_store_node_value_state_t>;

///////////////////////////////////////////////////////////////////////////////
// Private variables
///////////////////////////////////////////////////////////////////////////////
namespace
{
// We have 4 lists of callback functions.

// 1. For any modification in the attribute store
std::set<attribute_store_node_update_callback_t> generic_callbacks;

// 2. For any touch in the attribute store
std::set<attribute_store_node_touch_callback_t> touch_generic_callbacks;

// 3. For modification of any value for a given node type
std::map<attribute_store_type_t,
         std::vector<attribute_store::node_changed_callback>> // NOSONAR: std::vector must have a non-const
  type_callbacks;

// 4. Attribute Deletion callbacks
std::set<attribute_store_node_delete_callback_t> delete_callbacks;

// 5. For modification of a value/state (reported/desired) for a given node type
std::map<attribute_store_value_callback_setting_t, // NOSONAR: std::vector must have a non-const
         std::vector<attribute_store::node_changed_callback>>
  value_callbacks;

// 6. To ensure compatibility with the old C API, that is still in use
// we keep track of C functions that are registered for type callbacks
std::map<attribute_store_type_t,
         std::set<attribute_store_node_changed_callback_t>>
  c_node_changed_callbacks; // NOSONAR: Global variables should be const.

// 7. To ensure compatibility with the old C API, that is still in use
// we keep track of C functions that are registered for value callbacks
std::map<attribute_store_value_callback_setting_t,
         std::set<attribute_store_node_changed_callback_t>>
  c_node_value_callbacks;

}  // namespace

///////////////////////////////////////////////////////////////////////////////
// Private functions, shared in the component
///////////////////////////////////////////////////////////////////////////////
void attribute_store_invoke_touch_callbacks(attribute_store_node_t touched_node)
{
  attribute_store_invoke_touch_generic_callbacks(touched_node);
}

void attribute_store_invoke_callbacks(
  attribute_store_node_t updated_node,
  attribute_store_type_t type,
  attribute_store_node_value_state_t value_state,
  attribute_store_change_t change)
{
  attribute_changed_event_t change_event = {.updated_node = updated_node,
                                            .type         = type,
                                            .value_state  = value_state,
                                            .change       = change};

  attribute_store_invoke_generic_callbacks(&change_event);
  attribute_store_invoke_type_callbacks(updated_node, type, change);
  attribute_store_invoke_value_callbacks(updated_node,
                                         type,
                                         value_state,
                                         change);
  if (change == ATTRIBUTE_DELETED) {
    attribute_store_invoke_delete_callbacks(updated_node);
  }
}

void attribute_store_invoke_generic_callbacks(
  attribute_changed_event_t *change_event)
{
  // Call all these functions:
  std::set<attribute_store_node_update_callback_t> callbacks_to_invoke
    = generic_callbacks;
  for (auto &callback_function: callbacks_to_invoke) {
    callback_function(change_event);
  }
}

void attribute_store_invoke_touch_generic_callbacks(
  attribute_store_node_t touched_node)
{
  // Call all these functions:
  std::set<attribute_store_node_touch_callback_t> callbacks_to_invoke
    = touch_generic_callbacks;
  for (auto &callback_function: callbacks_to_invoke) {
    callback_function(touched_node);
  }
}

void attribute_store_invoke_type_callbacks(attribute_store_node_t updated_node,
                                           attribute_store_type_t type,
                                           attribute_store_change_t change)
{
  // Make the type callbacks:
  if (type_callbacks.count(type)) {
    for (auto callback_function: type_callbacks[type]) {
      callback_function(updated_node, change);
    }
  }
}

void attribute_store_invoke_value_callbacks(
  attribute_store_node_t updated_node,
  attribute_store_type_t type,
  attribute_store_node_value_state_t value_state,
  attribute_store_change_t change)
{
  attribute_store_value_callback_setting_t setting = {type, value_state};

  for (auto callback_function: value_callbacks[setting]) {
    callback_function(updated_node, change);
  }
}

void attribute_store_invoke_delete_callbacks(
  attribute_store_node_t deleted_node)
{
  // Make the delete callbacks:
  std::set<attribute_store_node_delete_callback_t> callbacks_to_invoke
    = delete_callbacks;
  for (auto &callback_function: callbacks_to_invoke) {
    callback_function(deleted_node);
  }
}

sl_status_t attribute_store_callbacks_init(void)
{
  // Remove all registered callbacks
  generic_callbacks.clear();
  type_callbacks.clear();
  delete_callbacks.clear();
  value_callbacks.clear();
  touch_generic_callbacks.clear();
  c_node_changed_callbacks.clear();
  c_node_value_callbacks.clear();
  
  return SL_STATUS_OK;
}

int attribute_store_callbacks_teardown(void)
{
  // Remove all registered callbacks
  generic_callbacks.clear();
  type_callbacks.clear();
  delete_callbacks.clear();
  value_callbacks.clear();
  touch_generic_callbacks.clear();
  c_node_changed_callbacks.clear();
  c_node_value_callbacks.clear();

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t attribute_store_register_touch_notification_callback(
  attribute_store_node_touch_callback_t callback_function)
{
  if (callback_function == nullptr) {
    sl_log_warning(LOG_TAG,
                   "Attempt to register a nullptr touch callback for all "
                   "attributes. Discarding.");
    return SL_STATUS_FAIL;
  }
  touch_generic_callbacks.insert(callback_function);
  return SL_STATUS_OK;
}

sl_status_t attribute_store_register_delete_callback(
  attribute_store_node_delete_callback_t callback_function)
{
  if (callback_function == nullptr) {
    sl_log_warning(LOG_TAG,
                   "Attempt to register a nullptr delete callback "
                   "function. Discarding.");
    return SL_STATUS_FAIL;
  }
  delete_callbacks.insert(callback_function);
  return SL_STATUS_OK;
}

sl_status_t attribute_store_register_callback(
  attribute_store_node_update_callback_t callback_function)
{
  if (callback_function == nullptr) {
    sl_log_warning(LOG_TAG,
                   "Attempt to register a nullptr update callback for all "
                   "attributes. Discarding.");
    return SL_STATUS_FAIL;
  }
  generic_callbacks.insert(callback_function);
  return SL_STATUS_OK;
}

sl_status_t attribute_store_register_callback_by_type(
  attribute_store_node_changed_callback_t callback_function,
  attribute_store_type_t type)
{
  if (callback_function == nullptr) {
    sl_log_warning(
      LOG_TAG,
      "Attempt to register a nullptr update callback for attribute type %d. "
      "Discarding.",
      type);
    return SL_STATUS_FAIL;
  }

  // Here we only register the callback if it's not already registered
  // This match the old behavior of the C API
  // We still return SL_STATUS_OK even if it not registered to keep compatibility
  if (c_node_changed_callbacks.count(type) == 0
      || c_node_changed_callbacks[type].count(callback_function) == 0) {
    c_node_changed_callbacks[type].insert(callback_function);
    attribute_store::register_callback_by_type(callback_function, type);
  }

  return SL_STATUS_OK;
}

sl_status_t attribute_store_register_callback_by_type_and_state(
  attribute_store_node_changed_callback_t callback_function,
  attribute_store_type_t type,
  attribute_store_node_value_state_t value_state)
{
  if (callback_function == nullptr) {
    sl_log_warning(LOG_TAG,
                   "Attempt to register a nullptr update callback for "
                   "attribute type %d and value state %d. "
                   "Discarding.",
                   type,
                   value_state);
    return SL_STATUS_FAIL;
  }

  // Here we only register the callback if it's not already registered
  // This match the old behavior of the C API
  // We still return SL_STATUS_OK even if it not registered to keep compatibility
  attribute_store_value_callback_setting_t setting = {type, value_state};
  if (c_node_value_callbacks.count(setting) == 0
      || c_node_value_callbacks[setting].count(callback_function) == 0) {
    c_node_value_callbacks[setting].insert(callback_function);
    attribute_store::register_callback_by_type_and_state(callback_function,
                                                         type,
                                                         value_state);
  }

  return SL_STATUS_OK;
}



namespace attribute_store {

void register_callback_by_type(node_changed_callback callback_function,
                               attribute_store_type_t type)
{
  type_callbacks[type].push_back(callback_function);
}

void register_callback_by_type_and_state(node_changed_callback callback_function,
                                         attribute_store_type_t type,
                                         attribute_store_node_value_state_t value_state)
{
  attribute_store_value_callback_setting_t setting = {type, value_state};
  value_callbacks[setting].push_back(callback_function);
}

} // namespace attribute_store 
