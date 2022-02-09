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

#ifndef ATTRIBUTE_STORE_NODE_H
#define ATTRIBUTE_STORE_NODE_H

// Includes from this component
#include "attribute_store.h"

// Generic includes
#include <vector>

/**
 * @defgroup attribute_store_node Attribute Store Node class
 * @ingroup attribute_store
 * @brief Node Class and definition for the attribute store
 *
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A node in the attribute store tree
 *
 */
class attribute_store_node
{
  public:
  /// Type used to identify what the attribute represents.
  /// (e.g. Binary Switch value, HomeID, Version of a Comamand Class, etc.)
  attribute_store_type_t type;
  /// Unique ID used by the public interface to identify this
  /// instance of a node
  attribute_store_node_t id;
  // An attribute value can be anything. From a simple boolean/byte
  // to byte array or string.
  // We have strings in configuration CC or
  // byte arrays in Association or Indicator CCs
  /// Desired value for the node attribute
  std::vector<uint8_t> desired_value;
  /// Reported value for the node attribute
  std::vector<uint8_t> reported_value;
  /// Pointer to the parent node in the tree.
  attribute_store_node *parent_node;
  /// Pointers to child nodes. There will be 0 to N child nodes.
  std::vector<attribute_store_node *> child_nodes;
  /// Node is being deleted.
  bool undergoing_deletion;

  /**
   * @brief Node class constructor
   *
   * Create a node object under the parent node indicated by the pointer.
   * The attribute ID is also assigned immediately.
   * @param _parent_node    Pointer to the parent node.
   * @param _type           The attribute Type to assign to the new node.
   * @param _id             The unique ID used to identify to assign
   *                        to the new node in the persistent datastore/
   *                        public interface.
   */
  attribute_store_node(attribute_store_node *_parent_node,
                       attribute_store_type_t _type,
                       attribute_store_node_t _id);
  /**
   * @brief Node class destructor
   *
   * Destroy the Node object and all its children.
   */
  ~attribute_store_node();

  /**
   * @brief Add a child for the current node.
   *
   * This function will instanciance a new child under the current
   * node.
   * @param _type The attribute Type to assign to the new node.
   * @param _id   The unique ID used to identify to assign
   *              to the new node.
   *
   * @returns     pointer to the newly created node.
   */
  attribute_store_node *add_child(attribute_store_type_t _type,
                                  attribute_store_node_t _id);

  /**
   * @brief Finds if this node or one of its children have a given public interface ID
   *
   * This functions returns the pointer to the node found with the given
   * id. If no node has this ID assigned under the tree, it will return
   * NULL.
   *
   * @param _id  The unique ID to search for under the node.
   *
   * @returns   A pointer to the node having this ID, if it is in the tree.
   * @returns   NULL if the ID is not part of the sub-tree
   */
  attribute_store_node *find_id(attribute_store_node_t _id);

  /**
   * @brief Logs the content of the a Node using \ref sl_log
  */
  void log();
  /**
   * @brief Logs the content of the a Node using \ref sl_log, indicating an indent
   * to use for the current element
   *
   * @param indent Number of spaces to insert before the current entry.
  */
  void log(uint8_t indent);
  /**
   * @brief Logs the content of the a Node and all its children using \ref sl_log
   *
   * @param indent Number of spaces to insert before the children.
  */
  void log_children(uint8_t indent);

  private:
  /**
   * @brief Removes a child from the list of children for the current node.
   */
  void remove_child_link(attribute_store_node *_child_node);
};

#ifdef __cplusplus
}
#endif

/** @} end attribute_store_node */

#endif  // ATTRIBUTE_STORE_NODE_H
