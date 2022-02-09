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
#include "attribute_store_node.h"
#include "attribute_store_internals.h"
#include "attribute_store_callbacks.h"
#include "attribute_store_fixt.h"

// Generic includes
#include <stdbool.h>
#include <map>
#include <string.h>
#include <assert.h>

// Includes from other components
#include "sl_log.h"
#include "sl_status.h"
#include "datastore.h"
#include "datastore_attributes.h"

/// Setup Log tag
constexpr char LOG_TAG[] = "attribute_store";

// The base of our attribute store tree
static attribute_store_node *root_node = nullptr;

// Map between ids and pointers for speedy identification
// instead of using node->find_id
static std::map<attribute_store_node_t, attribute_store_node *> id_node_map;

// Keep track of which ID to assign next in the datastore
static attribute_store_node_t last_assigned_id = ATTRIBUTE_STORE_INVALID_NODE;
constexpr char DATASTORE_LAST_ASSIGNED_ID_KEY[]
  = "attribute_store_last_assigned_id";

///////////////////////////////////////////////////////////////////////////////
// Private helper macros
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Saves a node that's in-memory in the datastore.
 */
#define STORE_ATTRIBUTE(node)                            \
  datastore_store_attribute(node->id,                    \
                            node->type,                  \
                            node->parent_node->id,       \
                            &node->reported_value[0],    \
                            node->reported_value.size(), \
                            &node->desired_value[0],     \
                            node->desired_value.size())

/**
 * @brief Saves the root node in the datastore.
 */
#define STORE_ROOT_ATTRIBUTE()                                \
  datastore_store_attribute(root_node->id,                    \
                            root_node->type,                  \
                            ATTRIBUTE_STORE_NULL_ID,          \
                            &root_node->reported_value[0],    \
                            root_node->reported_value.size(), \
                            &root_node->desired_value[0],     \
                            root_node->desired_value.size())
///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
/**
 * @brief Prints out that the attribute store is not initialized.
 */
static inline void log_attribute_store_not_initialized()
{
  sl_log_error(LOG_TAG, "Attribute Store is not initialized.");
}

/**
 * @brief Prints out that the database could not be found in the attribute store.
 */
static inline void
  log_attribute_store_node_not_found(attribute_store_node_t node)
{
  sl_log_warning(LOG_TAG, "Cannot find ID %d in the Attribute Store.", node);
}

/**
 * @brief Find the next available attribute_store_node_t ID
 *
 * Will modify the static last_assigned_id variable
 * @returns the next value that can be used for attribute_store_node_t ID
 */
static attribute_store_node_t attribute_store_get_next_id()
{
  while (last_assigned_id == ATTRIBUTE_STORE_INVALID_NODE
         || last_assigned_id == ATTRIBUTE_STORE_ROOT_ID
         || id_node_map.count(last_assigned_id)) {
    last_assigned_id++;
  }
  return last_assigned_id;
}

/**
 * @brief Finds attribute_store_node corresponding to a attribute_store_node_t
 *
 * @param id The node to find and return its pointer
 *
 * @returns nullptr if the node is not in our attribute store tree
 * @returns attribute_store_node_t pointer if the node is in our attribute store tree
 */
static attribute_store_node *
  attribute_store_get_node_from_id(attribute_store_node_t id)
{
  std::map<attribute_store_node_t, attribute_store_node *>::iterator it;
  it = id_node_map.find(id);
  if (it != id_node_map.end()) {
    return it->second;
  }

  // If the resevered ID was asked for, just return nullptr.
  if (id == ATTRIBUTE_STORE_INVALID_NODE) {
    return nullptr;
  }

  // Else if we get here, we did not find the ID using our map.
  // Give a go at traversing the tree, in case the map got out-of-sync
  attribute_store_node *found_node = root_node->find_id(id);

  // If we found it, something bad happened.
  assert((found_node == nullptr) && "Attribute Store map data cache error");
  if (found_node != nullptr) {
    id_node_map[id] = found_node;
    sl_log_error(
      LOG_TAG,
      "Attribute store Map got out-of-sync with the attribute store tree. "
      "Please verify that the map gets correctly updated at any tree update!");
    attribute_store_log_node(id, false);
  } else {
    log_attribute_store_node_not_found(id);
  }

  return found_node;
}

/**
 * @brief Compares if a node value is identical to the value/value size
 *
 * @param node        The node handle of the node for which the value
 *                    must be compared
 * @param value_state The value state of the node that must be compared.
 * @param value       A unint8_t pointer to a buffer with the value to compare
 *                    to the node's value.
 * @param value_size  The size of the data to compare in the value pointer.
 *
 * @returns true if the value stored in the node is identical
 * @returns false if the value stored in the node is different
 */
static bool attribute_store_is_value_identical(
  const attribute_store_node *node,
  attribute_store_node_value_state_t value_state,
  const uint8_t *value,
  uint8_t value_size)
{
  // Make a vector our of the received uint8_t array:
  std::vector<uint8_t> received_value(value, value + value_size);

  // Compare our received vector with the right value state
  if (value_state == REPORTED_ATTRIBUTE) {
    return node->reported_value == received_value;
  } else if (value_state == DESIRED_ATTRIBUTE) {
    return node->desired_value == received_value;
  }

  // value state was invalid
  return false;
}

/**
 * @brief Takes a node, loads it and all its children from
 * the datastore to the "in-memory" attribute store.
 *
 * @param node  Pointer to the in memory object where to copy the data.
 *              If set to nullptr, it will create a new object.
 * @param id    Unique ID to be found in the datastore.
 *
 * @returns SL_STATUS_OK if the node and its subtree have been loaded
 * @returns SL_STATUS_FAIL if an error occurred.
 * @returns SL_STATUS_NOT_FOUND if the given node_id is not present in the datastore.
 */
static sl_status_t
  attribute_store_load_from_datastore(attribute_store_node *node,
                                      attribute_store_node_t id)
{
  attribute_store_type_t received_type                                  = 0;
  attribute_store_node_t received_parent_id                             = 0;
  uint8_t received_reported_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {0};
  uint8_t received_reported_value_size                                  = 0;
  uint8_t received_desired_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH]  = {0};
  uint8_t received_desired_value_size                                   = 0;

  // Fetch the node first:
  sl_status_t datastore_status
    = datastore_fetch_attribute(id,
                                &received_type,
                                &received_parent_id,
                                received_reported_value,
                                &received_reported_value_size,
                                received_desired_value,
                                &received_desired_value_size);
  // Don't do more if it did not go well
  if (datastore_status != SL_STATUS_OK) {
    log_attribute_store_node_not_found(id);
    return datastore_status;
  }

  // Find the parent node pointer (will be nullptr if does not exist)
  attribute_store_node *parent_node
    = attribute_store_get_node_from_id(received_parent_id);

  if (node == nullptr) {
    // create a new node
    attribute_store_node *new_node = nullptr;

    if (parent_node == nullptr) {
      // If the parent ID is not in the tree, something is wrong
      sl_log_error(LOG_TAG,
                   "Cannot find parent ID %d in the attribute store. "
                   "Aborting load from the datastore.",
                   received_parent_id);
      return SL_STATUS_FAIL;
    }
    // Create the new node and assign values
    new_node                 = parent_node->add_child(received_type, id);
    new_node->reported_value = std::vector<uint8_t>(
      received_reported_value,
      received_reported_value + received_reported_value_size);

    new_node->desired_value = std::vector<uint8_t>(
      received_desired_value,
      received_desired_value + received_desired_value_size);

    // We just created a node, keep the local map updated
    id_node_map[new_node->id] = new_node;

  } else {
    // map the received data to the pointer we received,
    // and accept nullptr pointer for parent_node
    node->type        = received_type;
    node->parent_node = parent_node;
    node->reported_value.resize(received_reported_value_size);
    if (received_reported_value_size > 0) {
      node->reported_value.assign(received_reported_value,
                                  received_reported_value
                                    + received_reported_value_size);
    }
    node->desired_value.resize(received_desired_value_size);
    if (received_desired_value_size > 0) {
      node->desired_value.assign(received_desired_value,
                                 received_desired_value
                                   + received_desired_value_size);
    }
    // Should have happened when the node was created, but just make sure that
    // we have this node in the map
    id_node_map[node->id] = node;
  }

  // This is a recursive function, we now load all its children from the datastore
  attribute_store_node_t child_id = 0;
  uint32_t child_index            = 0;
  while (SL_STATUS_OK
         == datastore_fetch_attribute_child(id,
                                            child_index++,
                                            &child_id,
                                            &received_type,
                                            received_reported_value,
                                            &received_reported_value_size,
                                            received_desired_value,
                                            &received_desired_value_size)) {
    // Now load the child into a new node.
    datastore_status = attribute_store_load_from_datastore(nullptr, child_id);
    // Don't do more if it did not go well
    if (datastore_status != SL_STATUS_OK) {
      log_attribute_store_node_not_found(child_id);
      return datastore_status;
    }
  }

  return datastore_status;
}

/**
 * @brief Takes a node and invokes all callbacks on the node and all its
 *        children
 * @param current_node  Pointer to the in memory object of the node.
 */
static void
  invoke_callbacks_on_node_and_children(attribute_store_node *current_node)
{
  if (current_node == nullptr) {
    return;
  }
  // Making sure all callbacks are called with updates to REPORTED_ATTRIBUTES,
  // Start from the parent and do children next
  attribute_store_invoke_callbacks(current_node->id,
                                   current_node->type,
                                   REPORTED_ATTRIBUTE,
                                   ATTRIBUTE_UPDATED);

  for (uint32_t i = 0; i < current_node->child_nodes.size(); i++) {
    invoke_callbacks_on_node_and_children(current_node->child_nodes.at(i));
  }
}

///////////////////////////////////////////////////////////////////////////////
// Init and teardown functions, declared in the attribute_store_fixt.h file
///////////////////////////////////////////////////////////////////////////////
sl_status_t attribute_store_init(void)
{
  // Remove all registered callbacks
  attribute_store_callbacks_init();

  // Prepare our root node, if not ready
  if (root_node == nullptr) {
    root_node = new attribute_store_node(nullptr,
                                         ATTRIBUTE_TREE_ROOT,
                                         ATTRIBUTE_STORE_ROOT_ID);

    // Add the root as our first node in the id map:
    id_node_map.clear();
    id_node_map[root_node->id] = root_node;

    // Load the root data and all its children from the datastore, in case it's there.
    // Do not reload from SQLite if root_node was already allocated.
    if (attribute_store_load_from_datastore(root_node, root_node->id)
        != SL_STATUS_OK) {
      sl_log_info(
        LOG_TAG,
        "Attribute Store data could not be loaded from the datastore. "
        "Starting with an empty Attribute Store.");
    }
  }

  // Reload our last assigned ID. Start from 0 by default.
  last_assigned_id = 0;
  int64_t last_assigned_id_value;
  if (SL_STATUS_OK
      == datastore_fetch_int(DATASTORE_LAST_ASSIGNED_ID_KEY,
                             &last_assigned_id_value)) {
    last_assigned_id
      = static_cast<attribute_store_node_t>(last_assigned_id_value);
  }

  if (nullptr != root_node) {
    // Save the root in the datastore:
    STORE_ROOT_ATTRIBUTE();
    return SL_STATUS_OK;
  }

  sl_log_critical(LOG_TAG, "Could not initialize the Attribute Store.");
  return SL_STATUS_FAIL;
}

int attribute_store_teardown(void)
{
  sl_log_debug(LOG_TAG, "Teardown of the attribute store");
  // Remove all registered callbacks
  attribute_store_callbacks_teardown();

  // Clear up our map
  id_node_map.clear();

  // The datastore should already be in-sync with the in-memory
  // representation. Do not save it once more.

  // Save the last assigned ID for next time
  datastore_store_int(DATASTORE_LAST_ASSIGNED_ID_KEY,
                      static_cast<int64_t>(last_assigned_id));

  // Delete the root node, which will delete everything.
  if (root_node != nullptr) {
    delete root_node;
    // Reset to NULL, so we can detect un-initialized attribute store
    root_node = nullptr;
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions, declared in attribute_store.h
///////////////////////////////////////////////////////////////////////////////
attribute_store_node_t attribute_store_get_root()
{
  if (root_node == nullptr) {
    log_attribute_store_not_initialized();
    return ATTRIBUTE_STORE_INVALID_NODE;
  }
  return root_node->id;
}

attribute_store_node_t
  attribute_store_add_node(attribute_store_type_t type,
                           attribute_store_node_t parent_node)
{
  if (root_node == nullptr) {
    log_attribute_store_not_initialized();
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  if (type == ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  attribute_store_node *parent = attribute_store_get_node_from_id(parent_node);

  if (parent == nullptr) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  if (parent->undergoing_deletion == true) {
    sl_log_warning(LOG_TAG,
                   "Attempt to add a child node for Attribute Node ID %d "
                   "while it is undergoing deletion. Ignoring.",
                   parent->id);
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  attribute_store_node *new_node = nullptr;
  new_node = parent->add_child(type, attribute_store_get_next_id());

  // If it worked, save it in the datastore and invoke callbacks
  if (new_node != nullptr) {
    // Save the new node in the database:
    STORE_ATTRIBUTE(new_node);

    // Update our local map
    id_node_map[new_node->id] = new_node;

    // Callbacks
    attribute_store_invoke_callbacks(new_node->id,
                                     type,
                                     REPORTED_ATTRIBUTE,
                                     ATTRIBUTE_CREATED);
    return new_node->id;
  }

  return ATTRIBUTE_STORE_INVALID_NODE;
}

sl_status_t attribute_store_delete_node(attribute_store_node_t id)
{
  if (root_node == nullptr) {
    log_attribute_store_not_initialized();
    return SL_STATUS_NOT_INITIALIZED;
  }

  sl_status_t deletion_status          = SL_STATUS_OK;
  attribute_store_node *node_to_delete = attribute_store_get_node_from_id(id);
  if (node_to_delete == nullptr) {
    // Node is not found, we pretend deletion went well since it does not exist.
    return deletion_status;
  }

  // Prevent double deletes
  // If the node is already undergoing deletion, don't try to delete again
  if (node_to_delete->undergoing_deletion == true) {
    return SL_STATUS_OK;
  }

  if (node_to_delete == root_node) {
    // The root node does not get deleted, the attribute store owns it.
    while (node_to_delete->child_nodes.size() != 0) {
      deletion_status
        = attribute_store_delete_node(node_to_delete->child_nodes.at(0)->id);
      if (deletion_status != SL_STATUS_OK) {
        return deletion_status;
      }
    }
    // Clear the desired/reported values from the root when a deletion was requested.
    root_node->reported_value.resize(0);
    root_node->desired_value.resize(0);
    STORE_ROOT_ATTRIBUTE();
  } else {
    // Find the children first
    // (else datastore will not be happy, you cannot delete a node with children)
    while (node_to_delete->child_nodes.size() != 0) {
      deletion_status
        = attribute_store_delete_node(node_to_delete->child_nodes.at(0)->id);
      if (deletion_status != SL_STATUS_OK) {
        return deletion_status;
      }
    }

    // Set the node as read-only. No more write operations allowed at this point
    node_to_delete->undergoing_deletion = true;

    // Then delete from our persistent memory, since no write are allowed.
    if (SL_STATUS_OK != datastore_delete_attribute(node_to_delete->id)) {
      sl_log_warning(LOG_TAG,
                     "Could not delete ID %d from the datastore.",
                     node_to_delete->id);
      return SL_STATUS_FAIL;
    }

    // Fire the callback now, so Attribute Store users have a chance
    // to read the value before the attribute is gone for good.
    attribute_store_invoke_callbacks(node_to_delete->id,
                                     node_to_delete->type,
                                     REPORTED_ATTRIBUTE,
                                     ATTRIBUTE_DELETED);

    // Remove the mapping between id and our pointer:
    id_node_map.erase(node_to_delete->id);
    // finally, delete from our memory
    delete node_to_delete;
  }

  return deletion_status;
}

attribute_store_node_t
  attribute_store_get_node_parent(attribute_store_node_t id)
{
  if (root_node == nullptr) {
    log_attribute_store_not_initialized();
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  const attribute_store_node *current_node
    = attribute_store_get_node_from_id(id);

  if (current_node == nullptr) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  if (current_node->parent_node == nullptr) {  // The root has no parent.
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  return current_node->parent_node->id;
}

attribute_store_node_t
  attribute_store_get_first_parent_with_type(attribute_store_node_t id,
                                             attribute_store_type_t parent_type)
{
  if (root_node == nullptr) {
    log_attribute_store_not_initialized();
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  const attribute_store_node *current_node
    = attribute_store_get_node_from_id(id);

  if (current_node == nullptr) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  do {
    current_node = current_node->parent_node;
  } while ((current_node != nullptr) && (current_node->type != parent_type));

  if (current_node == nullptr) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  return current_node->id;
}

sl_status_t attribute_store_set_node_attribute_value(
  attribute_store_node_t id,
  attribute_store_node_value_state_t value_state,
  const uint8_t *value,
  uint8_t value_size)
{
  if (root_node == nullptr) {
    log_attribute_store_not_initialized();
    return SL_STATUS_NOT_INITIALIZED;
  }

  attribute_store_node *node_to_modify = attribute_store_get_node_from_id(id);

  if (node_to_modify == nullptr) {
    return SL_STATUS_FAIL;
  }

  if (node_to_modify->undergoing_deletion == true) {
    sl_log_warning(LOG_TAG,
                   "Attempt to modify the value of Attribute Node ID %d "
                   "while it is undergoing deletion. Ignoring.",
                   node_to_modify->id);
    return SL_STATUS_FAIL;
  }

  // If the value is identical to the one already stored,
  // we exit and avoid making callbacks
  if (attribute_store_is_value_identical(node_to_modify,
                                         value_state,
                                         value,
                                         value_size)) {
    // Notify that somebody tried to write something.
    attribute_store_invoke_touch_callbacks(node_to_modify->id);
    return SL_STATUS_OK;
  }

  if (value_state == DESIRED_ATTRIBUTE) {
    node_to_modify->desired_value.resize(value_size);
    if (value_size > 0) {
      node_to_modify->desired_value.assign(value, value + value_size);
    }
  } else if (value_state == REPORTED_ATTRIBUTE) {
    node_to_modify->reported_value.resize(value_size);
    if (value_size > 0) {
      node_to_modify->reported_value.assign(value, value + value_size);
    }
  } else {
    sl_log_warning(LOG_TAG,
                   "value_state was neither reported or desired: %u\n",
                   value_state);
    return SL_STATUS_FAIL;
  }

  // Save in the datastore
  if (node_to_modify == root_node) {
    STORE_ROOT_ATTRIBUTE();
  } else {
    STORE_ATTRIBUTE(node_to_modify);
  }

  // Invoke callbacks to indicate that something changed.
  attribute_store_invoke_callbacks(node_to_modify->id,
                                   node_to_modify->type,
                                   value_state,
                                   ATTRIBUTE_UPDATED);

  return SL_STATUS_OK;
}

sl_status_t attribute_store_get_node_attribute_value(
  attribute_store_node_t id,
  attribute_store_node_value_state_t value_state,
  uint8_t *value,
  uint8_t *value_size)
{
  if (root_node == nullptr) {
    log_attribute_store_not_initialized();
    return SL_STATUS_NOT_INITIALIZED;
  }

  attribute_store_node *node_to_read = attribute_store_get_node_from_id(id);

  if (node_to_read == nullptr) {
    *value_size = 0;
    return SL_STATUS_FAIL;
  }

  // Retrieve the value
  if (value_state == DESIRED_OR_REPORTED_ATTRIBUTE) {
    value_state = node_to_read->desired_value.size() > 0 ? DESIRED_ATTRIBUTE
                                                         : REPORTED_ATTRIBUTE;
    return attribute_store_get_node_attribute_value(id,
                                                    value_state,
                                                    value,
                                                    value_size);
  } else if (value_state == DESIRED_ATTRIBUTE) {
    *value_size = node_to_read->desired_value.size();
    memcpy(value, &node_to_read->desired_value[0], *value_size);
    return SL_STATUS_OK;
  } else if (value_state == REPORTED_ATTRIBUTE) {
    *value_size = node_to_read->reported_value.size();
    memcpy(value, &node_to_read->reported_value[0], *value_size);
    return SL_STATUS_OK;
  } else {
    sl_log_warning(LOG_TAG,
                   "Value state was neither REPORTED_ATTRIBUTE or "
                   "DESIRED_ATTRIBUTE: %u\n",
                   value_state);
    *value_size = 0;
    return SL_STATUS_FAIL;
  }
}

attribute_store_type_t attribute_store_get_node_type(attribute_store_node_t id)
{
  if (root_node == nullptr) {
    log_attribute_store_not_initialized();
    return ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE;
  }

  attribute_store_node *node_to_read = attribute_store_get_node_from_id(id);
  if (node_to_read == nullptr) {
    return ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE;
  }

  return node_to_read->type;
}

attribute_store_node_t attribute_store_get_node_child(attribute_store_node_t id,
                                                      uint32_t child_index)
{
  if (root_node == nullptr) {
    log_attribute_store_not_initialized();
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  attribute_store_node *node_to_read = attribute_store_get_node_from_id(id);

  if (node_to_read == nullptr) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  if (node_to_read->child_nodes.size() > child_index) {
    return node_to_read->child_nodes.at(child_index)->id;
  } else {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }
}

size_t attribute_store_get_node_child_count(attribute_store_node_t id)
{
  if (root_node == nullptr) {
    log_attribute_store_not_initialized();
    return 0;
  }

  if (id == ATTRIBUTE_STORE_INVALID_NODE) {
    return 0;
  }

  attribute_store_node *node_to_read = attribute_store_get_node_from_id(id);

  if (node_to_read == nullptr) {
    return 0;
  }

  return node_to_read->child_nodes.size();
}

bool attribute_store_node_exists(attribute_store_node_t id)
{
  if (root_node == nullptr) {
    log_attribute_store_not_initialized();
    return false;
  }

  if (id == ATTRIBUTE_STORE_INVALID_NODE) {
    return false;
  }
  const attribute_store_node *node_to_find
    = attribute_store_get_node_from_id(id);

  if (node_to_find == nullptr) {
    return false;
  }

  return true;
}

attribute_store_node_t
  attribute_store_get_node_child_by_type(attribute_store_node_t id,
                                         attribute_store_type_t child_type,
                                         uint32_t child_index)
{
  if (root_node == nullptr) {
    log_attribute_store_not_initialized();
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  attribute_store_node *node_to_read = attribute_store_get_node_from_id(id);

  if (node_to_read == nullptr) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  uint32_t child_counter = 0;
  for (uint32_t i = 0; i < node_to_read->child_nodes.size(); i++) {
    if (node_to_read->child_nodes.at(i)->type == child_type) {
      if (child_counter == child_index) {
        return node_to_read->child_nodes.at(i)->id;
      }
      child_counter++;
    }
  }
  return ATTRIBUTE_STORE_INVALID_NODE;
}

attribute_store_node_t attribute_store_get_node_child_by_value(
  attribute_store_node_t id,
  attribute_store_type_t child_type,
  attribute_store_node_value_state_t value_state,
  const uint8_t *value,
  uint8_t value_size,
  uint32_t child_index)
{
  if (root_node == nullptr) {
    log_attribute_store_not_initialized();
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  attribute_store_node *node_to_read = attribute_store_get_node_from_id(id);

  if (node_to_read == nullptr) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  uint32_t child_counter = 0;
  for (uint32_t i = 0; i < node_to_read->child_nodes.size(); i++) {
    if (node_to_read->child_nodes.at(i)->type == child_type) {
      // Attribute ID is matching, compare the value:
      if (value_state == DESIRED_ATTRIBUTE) {
        if (node_to_read->child_nodes.at(i)->desired_value.size()
            == value_size) {
          if (0
              == memcmp(value,
                        &node_to_read->child_nodes.at(i)->desired_value[0],
                        value_size)) {
            if (child_counter == child_index) {
              return node_to_read->child_nodes.at(i)->id;
            }
            child_counter++;
          }
        }
      } else if (value_state == REPORTED_ATTRIBUTE) {
        if (node_to_read->child_nodes.at(i)->reported_value.size()
            == value_size) {
          if (0
              == memcmp(value,
                        &node_to_read->child_nodes.at(i)->reported_value[0],
                        value_size)) {
            if (child_counter == child_index) {
              return node_to_read->child_nodes.at(i)->id;
            }
            child_counter++;
          }
        }
      }
    }
  }
  return ATTRIBUTE_STORE_INVALID_NODE;
}

sl_status_t
  attribute_store_refresh_node_and_children_callbacks(attribute_store_node_t id)
{
  if (root_node == nullptr) {
    log_attribute_store_not_initialized();
    return SL_STATUS_FAIL;
  }

  attribute_store_node *current_node = attribute_store_get_node_from_id(id);

  if (current_node == nullptr) {
    return SL_STATUS_FAIL;
  }

  invoke_callbacks_on_node_and_children(current_node);
  return SL_STATUS_OK;
}

void attribute_store_log()
{
  if (root_node != nullptr) {
    root_node->log_children(0);
  } else {
    sl_log_debug(LOG_TAG, "The attribute store is empty\n");
  }
}

void attribute_store_log_node(attribute_store_node_t id, bool log_children)
{
  if (root_node == nullptr) {
    log_attribute_store_not_initialized();
    return;
  }

  attribute_store_node *node_to_read = attribute_store_get_node_from_id(id);
  if (node_to_read != nullptr) {
    if (log_children == true) {
      node_to_read->log_children(0);
    } else {
      node_to_read->log();
    }
  }
}
