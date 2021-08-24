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

#ifndef ATTRIBUTE_STORE_H
#define ATTRIBUTE_STORE_H

// Includes from this component

// Includes from other components
#include "datastore_attributes.h"
#include "sl_status.h"
#include <stddef.h>

/**
 * @brief The macros below are used for defining attribute
 *
 * DEFINE_ATTRIBUTE has 3 deferent implementations which is
 * used in different contexts.
 * - If ATTRIBUTE_TYPE_MAP is define this macro output a key_value pair where the type is the key
 * - If ATTRIBUTE_NAME_MAP is define this macro output a key_value pair where the name is the key
 * - Otherwise this declaes a anonymous enum value. In this way the attribute can be used both expressions
 *   and in switch statements.
 */
#ifdef ATTRIBUTE_TYPE_MAP
#define DEFINE_ATTRIBUTE(name, value) {value, #name},
#else
#ifdef ATTRIBUTE_NAME_MAP
#define DEFINE_ATTRIBUTE(name, value) {#name, value},
#else
#define DEFINE_ATTRIBUTE(name, value) enum { name = value };
#endif
#endif

#define ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE \
  ((attribute_store_type_t)0x0000)  ///< This represents an invalid attribute Type. Shall not be used for nodes in the attribute store.

#define ATTRIBUTE_TREE_ROOT \
  ((attribute_store_type_t)0x0001)  ///< This represents the root of the attribute store tree.

typedef uint32_t attribute_store_type_t;

/**
 * @defgroup attribute_store Attribute Store
 * @ingroup uic_components
 * @brief Storage for DotDot/Z-Wave attributes.
 *
 * The Attribute store is a tree of attributes stored to save the state
 * of an object in the current network/environment.
 *
 * The attribute store is a directed acyclic graph, where there is
 * a root node under which all other nodes will have 1 parent and
 * a number of children.
 *
 * A tree node is identified by an @ref attribute_store_node_t handle
 * Each node has a type, defined by its @ref attribute_store_type_t type.
 *
 * This indicates what the value represents.
 * There are no restrictions regarding which node (type) is a parent or
 * a child.
 *
 * Each node has one or more value states, defined by
 * @ref attribute_store_node_value_state_t.
 * Each value state (REPORTED/DESIRED) contains a value of
 * variable length that can be read and written.
 * Nodes created have, by default, unassigned values in each of their
 * value state. In this case, the value length is set to 0.
 *
 * Finally, an example of attribute store tree organization is shown
 * in the image below.
 * Some Attribute IDs are color-coded for readability
 * and they do not represent any difference between nodes apart from
 * their attribute type value.
 * \image html attribute_store_representation.png "Attribute store tree example"
 *
 */

/**
 * @defgroup attribute_store_api Attribute Store Public Interface
 * @ingroup attribute_store
 * @brief Public functions from the attribute store.
 * It allows other components to manipulate the @ref attribute_store tree.
 * and register callbacks for @ref attribute_store updates.
 *
 * @{
 */

/**
 * @brief Handle to identify attribute store nodes
 */
//typedef void *attribute_store_node_t;
typedef datastore_attribute_id_t attribute_store_node_t;

/// Maximum length for a value associated to an attribute
#define ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH 255
/// Special attribute_store_node_t value indicating that it does not exist.
#define ATTRIBUTE_STORE_INVALID_NODE (attribute_store_node_t)0

/**
 * @brief Attribute Store type to indicate the modification type
 * that triggered a callback
 */
typedef enum {
  ATTRIBUTE_CREATED,
  ATTRIBUTE_UPDATED,
  ATTRIBUTE_DELETED,
} attribute_store_change_t;

/**
 * @brief Attribute store callback type for tree node updates
 */
typedef void (*attribute_store_node_update_callback_t)(
  attribute_store_node_t, attribute_store_change_t);

/**
 * @brief This is the value state of a value.
 */
typedef enum {
  REPORTED_ATTRIBUTE,  ///< This is the value reported by the node or actual value.
  DESIRED_ATTRIBUTE,  ///< This is the value to be applied to the node.
  DESIRED_OR_REPORTED_ATTRIBUTE,  ///< Retrieve the desired value if it exists if not then provide the reported attribute only used for readding.
} attribute_store_node_value_state_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Retrieve the root node of the tree.
 *
 * @returns NULL                    If the tree is not initialized, or there is
 *                                  no root node
 * @returns attribute_store_node_t  handle for the root node
 */
attribute_store_node_t attribute_store_get_root();

/**
 * @brief Add a new node in the current attribute store.
 *
 * @param type        The type to assign to the new node.
 *                    MUST be different than ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE
 * @param parent_node Parent node for the node to add.
 *
 * @returns attribute_store_node_t handle for the newly created node
 * @returns ATTRIBUTE_STORE_INVALID_NODE in case of error
 */
attribute_store_node_t
  attribute_store_add_node(attribute_store_type_t type,
                           attribute_store_node_t parent_node);
/**
 * @brief Delete a node and all its children from the attribute store.
 *
 * @param node The identifier of the node to delete.
 * @note The root node cannot be deleted. If passed as argument,
 *        only its children will be deleted.
 * @returns SL_STATUS_OK if the node and its children were deleted successfully
 *          or if it was not in the attribute store anyway
 * @returns SL_STATUS_FAIL if an error happened.
 * @returns SL_STATUS_NOT_INITIALIZED if the attribute store is not initialized
 */
sl_status_t attribute_store_delete_node(attribute_store_node_t node);

/**
 * @brief Get the node handle of the parent of a node.
 *
 * @param node         The tree handle of the node for which
 *                     the parent identifier will be returned
 * @returns ATTRIBUTE_STORE_INVALID_NODE If the node has no parent or does not exist.
 * @returns attribute_store_node_t       handle for the parent node on success
 */
attribute_store_node_t
  attribute_store_get_node_parent(attribute_store_node_t node);

/**
 * @brief Traverse up the tree from a node and finds the first parent with a given type.
 *
 * @param node              The Attribute Store node for which the parents
 *                          will be searched for the given type
 * @param parent_type       The type of the parent that we search
 *
 * @returns ATTRIBUTE_STORE_INVALID_NODE If the node has no (distant) parent with the given type.
 * @returns attribute_store_node_t       handle for the parent node on success
 */
attribute_store_node_t attribute_store_get_first_parent_with_type(
  attribute_store_node_t node, attribute_store_type_t parent_type);

/**
 * @brief Set the attribute value of a node in the current attribute store tree.
 *
 * @param node        The node handle for which the value
 *                    must be updated.
 * @param value_state The attribute_store_node_value_state_t
 *                    value state to write for the node.
 *                    (Desired or Reported)
 * @param value       A uint8_t pointer to an array containing
 *                    the data to be written for the node. This array
 *                    may contain up to 255 bytes.
 * @param value_size  The number of bytes to read from the value pointer.
 *
 * @returns SL_STATUS_OK         If the tree node value has been set correctly.
 * @returns SL_STATUS_FAIL       If the value state is not understood.
 * @returns SL_STATUS_NOT_INITIALIZED if the attribute store is not initialized
 *                                    properly
 */
sl_status_t attribute_store_set_node_attribute_value(
  attribute_store_node_t node,
  attribute_store_node_value_state_t value_state,
  const uint8_t *value,
  uint8_t value_size);

/**
 * @brief Get the attribute value of a node.
 *
 * @param node        The node handle of the node for which
 *                    attribute value is to be retrieved
 * @param value_state The attribute_store_node_value_state_t
 *                    value state to retrieve from the node.
 *                    (Desired or Reported)
 * @param value       A uint8_t pointer to an array for which the
 *                    value/value state will be written to. This array
 *                    should be able to contain 255 bytes.
 * @param value_size  Pointer to an uint8_t variable where the size
 *                    of the value/payload written to the value pointer
 *                    will be written.
 *
 * @returns SL_STATUS_OK        If the attribute value of the node was successfully
 *                              written to the pointers value/value_size
 * @returns SL_STATUS_FAIL      If an error occurred
 * @returns SL_STATUS_NOT_INITIALIZED if the attribute store is not initialized
 *                                    properly
 */
sl_status_t attribute_store_get_node_attribute_value(
  attribute_store_node_t node,
  attribute_store_node_value_state_t value_state,
  uint8_t *value,
  uint8_t *value_size);

/**
 * @brief Get the type of a node.
 *
 * @param node The node handle of the node for which
 *             the type is to be returned
 * @returns attribute_store_type_t The assigned type for the node
 * @returns ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE if the node does not exist
 */
attribute_store_type_t
  attribute_store_get_node_type(attribute_store_node_t node);

/**
 * @brief Get the node handle of a child of a node.
 *
 * @param node        The tree handle of the node for which
 *                    the child handle will be returned
 * @param child_index Value to indicate which child to find.
 *                    0 will match the first child,
 *                    1 the second child, etc.
 * @returns attribute_store_node_t handle for the child node
 *                                 at the indicated index.
 * @returns ATTRIBUTE_STORE_INVALID_NODE If the child index does not exist
 *          (i.e. there are less children than the child index).
 *          or if node itself does not exist
 */
attribute_store_node_t
  attribute_store_get_node_child(attribute_store_node_t node,
                                 uint32_t child_index);

/**
 * @brief Get the number of children of the given node.
 *
 * @param node        The tree handle of the node for which
 *                    the child handle will be returned
 * @returns int       Number of children
 */
size_t attribute_store_get_node_child_count(attribute_store_node_t node);

/**
 * @brief Get the node handle of a child of a node with a certain type.
 *
 * @param node          The node handle of the node for which
 *                      the child identifier will be written
 * @param child_type    The attribute store type of the child.
 * @param child_index   Value to indicate which child to find.
 *                      0 will match the first child (with that type),
 *                      1 the second child (with that type), etc.
 * @returns attribute_store_node_t handle for the child node at the indicated index
 *                                 with the type.
 * @returns ATTRIBUTE_STORE_INVALID_NODE if the child index does not exist.
 *               (i.e. there are less children with that type than the child index).
 *               or if node itself does not exist
 */
attribute_store_node_t
  attribute_store_get_node_child_by_type(attribute_store_node_t node,
                                         attribute_store_type_t child_type,
                                         uint32_t child_index);

/**
 * @brief Get the node handle of a child of a node with a certain type and value.
 *
 * @param node          The node handle of the node for which
 *                      the child identifier will be written
 * @param child_type    The attribute type of the child.
 * @param value_state   The attribute_store_node_value_state_t
 *                      value state to compare for the child.
 *                      (Desired or Reported)
 * @param value         A uint8_t pointer to an array for which the
 *                      value/value state will be compared.
 * @param value_size    The number of bytes to compare from the
 *                      value pointer.
 * @param child_index   Value to indicate which child to find.
 *                      0 will match the first child (with that type and value),
 *                      1 the second child (with that type and value), etc.
 * @returns attribute_store_node_t handle for the child node at the indicated index
 *                                 with the type and value.
 * @returns ATTRIBUTE_STORE_INVALID_NODE if the child index does not exist.
 *          (i.e. there are less children with that type and value than the
 *          child index). or if node itself does not exist
 *
 */
attribute_store_node_t attribute_store_get_node_child_by_value(
  attribute_store_node_t node,
  attribute_store_type_t child_type,
  attribute_store_node_value_state_t value_state,
  const uint8_t *value,
  uint8_t value_size,
  uint32_t child_index);

/**
 * @brief Register a callback function to any node change.
 *
 * WARNING: Do not use this function, unless you are a component
 * that really wants to know about almost all updates in the attribute
 * store. See \ref attribute_store_register_callback_by_type and
 * \ref attribute_store_register_callback_by_type_and_state
 *
 * @param callback_function The function to invoke when the node's
 *                          value is updated.
 * @returns SL_STATUS_OK    If the callback was registered successfully.
 */
sl_status_t attribute_store_register_callback(
  attribute_store_node_update_callback_t callback_function);

/**
 * @brief Register a callback function to any node with a given type.
 *
 * @param callback_function The function to invoke when the node's
 *                          value is updated.
 * @param type              The attribute node type for which a
 *                          callback will be registered.
 * @returns SL_STATUS_OK    If the callback was registered successfully.
 */
sl_status_t attribute_store_register_callback_by_type(
  attribute_store_node_update_callback_t callback_function,
  attribute_store_type_t type);

/**
 * @brief Register a callback function to any node with a given
 * type for a given value_state update
 *
 * @param callback_function The function to invoke when the node's
 *                          value is updated.
 * @param type              The attribute node type for which a
 *                          callback will be registered.
 * @param value_state       The value-state that will trigger a
 *                          callback when it's modified.
 * @returns SL_STATUS_OK    If the callback was registered successfully.
 */
sl_status_t attribute_store_register_callback_by_type_and_state(
  attribute_store_node_update_callback_t callback_function,
  attribute_store_type_t type,
  attribute_store_node_value_state_t value_state);

/**
 * @brief Verify whether a node is in the Attribute Store.
 *
 * @param   node  The identifier of the node to verify if
 *                it is present in the Attribute Store
 * @returns true  if the Node ID is in the Attribute Store
 * @returns false if the Node ID is in the Attribute Store
 */
bool attribute_store_node_exists(attribute_store_node_t node);

/**
 * @brief Invoking update of callbacks for the given node and all children nodes
 *
 * WARNING: Callbacks registered by DESIRED_ATTRIBUTE or
 * DESIRED_OR_REPORTED_ATTRIBUTE will not be invoked.
 *
 * @param   node  The identifier of the node to update
 *                and all its children
 *
 * @returns SL_STATUS_OK if all nodes callbacks was successfully updated
 * @returns SL_STATUS_FAIL if invoking update callbacks on any node fails
 */
sl_status_t attribute_store_refresh_node_and_children_callbacks(
  attribute_store_node_t node);

/**
 * @brief Log the content of the Attribute Store tree using \ref sl_log.
*/
void attribute_store_log(void);

/**
 * @brief Log the content of the Attribute Store tree using \ref sl_log
 * from a given node.
 * @param node          The node handle of the node for which the tree
 *                      will be printed out.
 * @param log_children  bool value indicating if children should be
 *                      printed out as well.
*/
void attribute_store_log_node(attribute_store_node_t node, bool log_children);

/** @} end attribute_store_api */

#ifdef __cplusplus
}
#endif

#endif  //ATTRIBUTE_STORE_H
