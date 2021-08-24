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
#include "attribute_store.h"
#include "attribute_store_callbacks.h"

// Generic includes
#include <map>
#include <set>
#include <string.h>

// Includes from other components
#include "sl_log.h"

/// Setup Log tag
#define LOG_TAG "attribute_store_callbacks"

/**
 * @brief A structure that's kept for registering callbacks settings.
 * There are 2 possible settings/filters that can be used in parallels:
 * - Callback when matching the type of a node only
 * - Callback when matching the value-state and type of a node
 */

typedef std::pair<attribute_store_type_t, attribute_store_node_value_state_t>
  attribute_store_value_callback_setting_t;

// We have 3 lists of callback functions:
// 1. For any modification in the attribute store
// 2. For modification of any value for a given node type
// 3. For modification of a value/state (reported/desired) for a given node type
static std::set<attribute_store_node_update_callback_t> generic_callbacks;

static std::map<attribute_store_type_t,
                std::set<attribute_store_node_update_callback_t>>
  type_callbacks;

static std::map<attribute_store_value_callback_setting_t,
                std::set<attribute_store_node_update_callback_t>>
  value_callbacks;

///////////////////////////////////////////////////////////////////////////////
// Private functions, shared in the component
///////////////////////////////////////////////////////////////////////////////
void attribute_store_invoke_callbacks(
  attribute_store_node_t updated_node,
  attribute_store_type_t type,
  attribute_store_node_value_state_t value_state,
  attribute_store_change_t change)
{
  attribute_store_invoke_generic_callbacks(updated_node, change);
  attribute_store_invoke_type_callbacks(updated_node, type, change);
  attribute_store_invoke_value_callbacks(updated_node,
                                         type,
                                         value_state,
                                         change);
}

void attribute_store_invoke_generic_callbacks(
  attribute_store_node_t updated_node, attribute_store_change_t change)
{
  // Call all these functions:
  for (auto callback_function: generic_callbacks) {
    callback_function(updated_node, change);
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
  // Make the type callbacks:
  if (value_callbacks.count(setting)) {
    for (auto callback_function: value_callbacks[setting]) {
      callback_function(updated_node, change);
    }
  }
}

int attribute_store_callbacks_teardown(void)
{
  // Remove all registered callbacks
  generic_callbacks.clear();
  type_callbacks.clear();
  value_callbacks.clear();

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t attribute_store_register_callback(
  attribute_store_node_update_callback_t callback_function)
{
  generic_callbacks.insert(callback_function);
  return SL_STATUS_OK;
}

sl_status_t attribute_store_register_callback_by_type(
  attribute_store_node_update_callback_t callback_function,
  attribute_store_type_t type)
{
  type_callbacks[type].insert(callback_function);
  return SL_STATUS_OK;
}

sl_status_t attribute_store_register_callback_by_type_and_state(
  attribute_store_node_update_callback_t callback_function,
  attribute_store_type_t type,
  attribute_store_node_value_state_t value_state)
{
  attribute_store_value_callback_setting_t setting = {type, value_state};
  value_callbacks[setting].insert(callback_function);
  return SL_STATUS_OK;
}
