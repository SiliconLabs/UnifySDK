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

#ifndef ATTRIBUTE_STORE_HELPER_H
#define ATTRIBUTE_STORE_HELPER_H

// Includes from this component
#include "attribute_store.h"

// Generic includes
#include <stdbool.h>

/**
 * @defgroup attribute_store_value_helpers Attribute Store Value Helpers
 * @ingroup attribute_store
 * @brief Helper functions to perform actions on the attribute store.
 *
 * This module contains various helper function to ease the access of the
 * attribute store.
 *
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Indicate whether a value is defined in the attribute store.
 *
 * @param node        The Attribute store node under
 *                    which the value must be retrieved
 * @param value_state The value_state to retrieve
 *
 * @returns true  If the value is defined (its size > 0)
 * @returns false If the value is undefined (its size == 0)
 */
bool attribute_store_is_value_defined(
  attribute_store_node_t node, attribute_store_node_value_state_t value_state);

/**
 * @brief Indicate whether reported and desired values are matched in the attribute store.
 *
 * @param node        The Attribute store node under
 *                    which the value must be retrieved
 *
 * @returns true  If the values are matched
 * @returns false If the value are unmatched
 */
bool attribute_store_is_value_matched(attribute_store_node_t node);

/**
 * @brief Set the Desired value to the same as the Reported value.
 *
 * @param node        The Attribute store node for which DESIRED
 *                    must be aligned to REPORTED
 *
 * @returns sl_status_t
 */
sl_status_t
  attribute_store_set_desired_as_reported(attribute_store_node_t node);

/**
 * @brief Set the Reported value to the same as the Desired value.
 *
 * @param node        The Attribute store node for which REPORTED
 *                    must be aligned to DESIRED
 *
 * @returns sl_status_t
 */
sl_status_t
  attribute_store_set_reported_as_desired(attribute_store_node_t node);

/**
 * @brief Set the Reported value to "undefined" (0 size no value)
 *
 * @param node        The Attribute store node for which REPORTED
 *                    must be set to nothing
 */
void attribute_store_undefine_reported(attribute_store_node_t node);

/**
 * @brief Set the Reported value of an Attribute Store node
 *
 * @param node        The Attribute store node for which REPORTED
 *                    must be updated
 * @param value       A pointer to a variable containing
 *                    the data to be written for the node.
 * @param value_size  The number of bytes to read from the value pointer.
 *
 * @returns sl_status_t returned by @ref attribute_store_set_node_attribute_value
 */
sl_status_t attribute_store_set_reported(attribute_store_node_t node,
                                         const void *value,
                                         uint8_t value_size);

/**
 * @brief Reads the reported value of an attribute store node
 *
 * @param node          The Attribute store node under
 *                      which the value must be retrieved
 * @param value         Pointer where to copy the value
 * @param expected_size  The size of the pointer of read_value
 *
 * @returns SL_STATUS_OK    if the read value has the expected size and
 *                          has been copied to the pointer.
 * @returns SL_STATUS_FAIL  if the value is undefined, has not the expected
 *                          size or has not been copied to the pointer.
 */
sl_status_t attribute_store_get_reported(attribute_store_node_t node,
                                         void *value,
                                         size_t expected_size);

/**
 * @brief Safely copies a String in the Attribute Store
 *
 * @param node          The Attribute store node under
 *                      which the value must be written
 * @param string        C char array containing a String
 *
 * @returns sl_status_t returned by @ref attribute_store_set_node_attribute_value
 */
sl_status_t attribute_store_set_reported_string(attribute_store_node_t node,
                                                const char *string);

/**
 * @brief Safely retrieves a String from the Attribute Store
 *
 *
 * @param node          The Attribute store node where a string is saved.
 * @param string [out]  C char array pointer where the string data will be copied
 *                      NULL termination is guaranteed.
 * @param maximum_size  Capacity of the string pointer
 *
 * @returns SL_STATUS_OK if the entire string was retrived. ("" string would work)
 * @returns SL_STATUS_FAIL if the attribute does not exist, value was undefined or
 * the string could not be copied.
 */
sl_status_t attribute_store_get_reported_string(attribute_store_node_t node,
                                                char *string,
                                                size_t maximum_size);

/**
 * @brief Set the Desired value to "undefined" (0 size no value)
 *
 * @param node        The Attribute store node for which DESIRED
 *                    must be set to nothing
 */
void attribute_store_undefine_desired(attribute_store_node_t node);

/**
 * @brief Set the Desired value of an Attribute Store node
 *
 * @param node        The Attribute store node for which DESIRED
 *                    must be updated
 * @param value       A pointer to a variable containing
 *                    the data to be written for the node.
 * @param value_size  The number of bytes to read from the value pointer.
 *
 * @returns sl_status_t returned by @ref attribute_store_set_node_attribute_value
 */
sl_status_t attribute_store_set_desired(attribute_store_node_t node,
                                        const void *value,
                                        uint8_t value_size);

/**
 * @brief Reads the desired value of an attribute store node
 *
 * @param node          The Attribute store node under
 *                      which the value must be retrieved
 * @param value         Pointer where to copy the value
* @param expected_size  The size of the pointer of read_value
 *
 * @returns SL_STATUS_OK    if the read value has the expected size and
 *                          has been copied to the pointer.
 * @returns SL_STATUS_FAIL  if the value is undefined, has not the expected
 *                          size or has not been copied to the pointer.
 */
sl_status_t attribute_store_get_desired(attribute_store_node_t node,
                                        void *value,
                                        size_t expected_size);

/**
 * @brief Copy the value from a node to another.
 *
 * @param source_node       The node that contains a value to be copied to
 *                          another node.
 * @param destination_node  The node that the source_node's value will be copied
 *                          into.
 * @param value_state       The value state to be copied from source_node to
 *                          destination_node

 * @returns SL_STATUS_OK if the value_state value was copied from source_node
 *                       to destination_node. Empty value copy will be considered
 *                        successful.
 * @returns SL_STATUS_NOT_FOUND if one of the attribute store nodes does not exist
 * @returns SL_FAIL if the value cannot be copied
 *
 */
sl_status_t
  attribute_store_copy_value(attribute_store_node_t source_node,
                             attribute_store_node_t destination_node,
                             attribute_store_node_value_state_t value_state);

/**
 * @brief Read an arbitrary sized value from the attribute store
 * and writes it to a pointer.
 *
 * @param node          The Attribute store node under
 *                      which the value must be retrieved
 * @param value_state   The value_state to retrieve
 * @param read_value    A pointer where to copy the read
 *                      value from the attribute store.
* @param expected_size  The size of the pointer of read_value
 *
 * @returns SL_STATUS_OK    if the read value has the expected size and
 *                          has been copied to the pointer.
 * @returns SL_STATUS_FAIL  if the value is undefined, has not the expected
 *                          size or has not been copied to the pointer.
 */
sl_status_t
  attribute_store_read_value(attribute_store_node_t node,
                             attribute_store_node_value_state_t value_state,
                             void *read_value,
                             size_t expected_size);

/**
 * @brief Adds if missing and sets the value of the first child of a node with a given type.
 * and writes its reported value.
 *
 * If there is no child of the given type for the parent, it will be created.
 *
 * @param parent        The Attribute store node under
 *                      which the child's reported value must be written
 * @param type          The type of the child
 * @param value         A pointer to a variable containing
 *                      the data to be written for the node.
 * @param value_size    The number of bytes to read from the value pointer.
 *
 * @returns sl_status_t returned by @ref attribute_store_set_node_attribute_value

 */
sl_status_t attribute_store_set_child_reported(attribute_store_node_t parent,
                                               attribute_store_type_t type,
                                               const void *value,
                                               uint8_t value_size);

/**
 * @brief Adds if missing and sets the value of the first child of a node with a given type.
 * and writes its desired value.
 *
 * If there is no child of the given type for the parent, it will be created.
 *
 * @param parent        The Attribute store node under
 *                      which the child's desired value must be written
 * @param type          The type of the child
 * @param value         A pointer to a variable containing
 *                      the data to be written for the node.
 * @param value_size    The number of bytes to read from the value pointer.
 *
 * @returns sl_status_t returned by @ref attribute_store_set_node_attribute_value

 */
sl_status_t attribute_store_set_child_desired(attribute_store_node_t parent,
                                              attribute_store_type_t type,
                                              const void *value,
                                              uint8_t value_size);

/**
 * @brief Set the value of the first child with a given type.
 *
 * This function sets the value of the first attribute child
 * matching a given type.
 *
 * @param parent Parent node of children
 * @param type   Type of child
 * @param state  The value_state
 * @param value  Value
 * @return sl_status_t
 */
sl_status_t attribute_store_set_uint32_child_by_type(
  attribute_store_node_t parent,
  attribute_store_type_t type,
  attribute_store_node_value_state_t state,
  uint32_t value);

/**
 * See @ref attribute_store_set_uint32_child_by_type
 * @param parent Parent node of children
 * @param type   Type of child
 * @param state  The value_state
 * @param value  Value
 * @return sl_status_t
 */
sl_status_t attribute_store_set_uint8_child_by_type(
  attribute_store_node_t parent,
  attribute_store_type_t type,
  attribute_store_node_value_state_t state,
  uint8_t value);

/**
 * @brief Visit all nodes in the tree.
 *
 * This function visits all tree nodes going from the top and down.
 * It will continue to the node children if the status returned by
 * the function is SL_STATUS_OK. If the function returns SL_STATUS_ABORT,
 * no more nodes are visited. Otherwise, siblings will be visited.
 *
 * @param top First node to be visited
 * @param func function to execute
 * @param depth The current depth of the top node.
 */
sl_status_t
  attribute_store_walk_tree(attribute_store_node_t top,
                            sl_status_t (*func)(attribute_store_node_t, int),
                            int depth);

/**
 * @brief Add multiple nodes to a parent if they are not already present.
 *
 * This is a helper function that will add  a list of nodes to the parent
 * if they are not already present.
 *
 * @param parent_node Parent node for the nodes to add.
 * @param attributes array of attribute types
 * @param count number of attributes to add.
 */
void attribute_store_add_if_missing(attribute_store_node_t parent_node,
                                    const attribute_store_type_t attributes[],
                                    uint32_t count);

/**
 * @brief Removes all children of an attribute store node.
 *
 * This is a helper function that will make sure a node has no more children.
 *
 * @param node Node for which all the children should be deleted
 *
 * @returns SL_STATUS_OK if no more children, SL_STATUS_FAIL if an error occurred
 *          and a child could not be deleted.
 */
sl_status_t attribute_store_delete_all_children(attribute_store_node_t node);


#ifdef __cplusplus
}
#endif

/** @} end attribute_store_value_helpers */

#endif  //attribute_store_helper_H
