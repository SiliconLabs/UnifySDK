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

// Generic includes
#include <stdio.h>
#include <string.h>
#include <algorithm>  //using std::remove

// Includes from other components
#include "sl_log.h"
#include "attribute_store_debug.h"

/// Setup Log tag
#define LOG_TAG "attribute_store"

// Printing define.
#define MAXIMUM_MESSAGE_SIZE 1000

///////////////////////////////////////////////////////////////////////////////
// Attribute store node constuctor / destructor
///////////////////////////////////////////////////////////////////////////////
attribute_store_node::attribute_store_node(attribute_store_node *_parent_node,
                                           attribute_store_type_t _type,
                                           attribute_store_node_t _id)
{
  // Assign the three required values
  this->type        = _type;
  this->parent_node = _parent_node;
  this->id          = _id;
  // The node is not being deleted... yet
  this->undergoing_deletion = false;
}

attribute_store_node::~attribute_store_node()
{
  // Destroy children first:
  while (this->child_nodes.size() > 0) {
    delete this->child_nodes.at(0);
  }

  // Remove the link at the parent's node
  if (this->parent_node != NULL) {
    this->parent_node->remove_child_link(this);
  }

  // Let the C++ destructor do the clean-up, no need to destroy
  // any of the node content
}

attribute_store_node *
  attribute_store_node::add_child(attribute_store_type_t _type,
                                  attribute_store_node_t _id)
{
  // Instantiate a new child.
  attribute_store_node *child_node = new attribute_store_node(this, _type, _id);
  // Add its pointer to the list of child nodes.
  this->child_nodes.push_back(child_node);

  // Return its address.
  return child_node;
}

attribute_store_node *attribute_store_node::find_id(attribute_store_node_t _id)
{
  // Look at our own ID
  if (this->id == _id) {
    return this;
  }

  // Probe the children:
  for (uint32_t i = 0; i < this->child_nodes.size(); i++) {
    attribute_store_node *child_result = this->child_nodes.at(i)->find_id(_id);
    if (child_result != NULL) {
      return child_result;
    }
  }

  // If no more child and we don't have the right id, return NULL
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Log functions
///////////////////////////////////////////////////////////////////////////////
void attribute_store_node::log()
{
  this->log(0);
}

void attribute_store_node::log(uint8_t indent)
{
  char log_message[MAXIMUM_MESSAGE_SIZE] = {'\0'};
  uint16_t log_index                     = 0;

  // Add the indent (spaces) at the start
  for (uint8_t i = 0; i < indent; i++) {
    log_index += snprintf(log_message + log_index,
                          sizeof(log_message) - log_index,
                          " ");
  }

  log_index += snprintf(log_message + log_index,
                        sizeof(log_message) - log_index,
                        "ID: %d - %s (0x%08X) - ",
                        id,
                        attribute_store_name_by_type(type),
                        type);

  // Desired value
  char desired_value_data[MAXIMUM_MESSAGE_SIZE] = {0};
  uint16_t index                                = 0;
  for (uint8_t i = 0; i < this->desired_value.size(); i++) {
    index += snprintf(desired_value_data + index,
                      sizeof(desired_value_data) - index,
                      "%02X ",
                      this->desired_value.at(i));
  }
  if (index > 1) {
    desired_value_data[index - 1] = '\0';
  }
  // Reported value
  index                                          = 0;
  char reported_value_data[MAXIMUM_MESSAGE_SIZE] = {0};
  for (uint8_t i = 0; i < this->reported_value.size(); i++) {
    index += snprintf(reported_value_data + index,
                      sizeof(reported_value_data) - index,
                      "%02X ",
                      this->reported_value.at(i));
  }
  if (index > 1) {
    reported_value_data[index - 1] = '\0';
  }

  // Add the desired and reported to the list
  snprintf(log_message + log_index,
           sizeof(log_message) - log_index,
           "Desired (hex): [%s] - Reported (hex): [%s]",
           desired_value_data,
           reported_value_data);

  sl_log_debug(LOG_TAG, "%s", log_message);
}

void attribute_store_node::log_children(uint8_t indent)
{
  this->log(indent);
  for (uint32_t i = 0; i < this->child_nodes.size(); i++) {
    this->child_nodes.at(i)->log_children(indent + 2);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Private functions
///////////////////////////////////////////////////////////////////////////////
void attribute_store_node::remove_child_link(attribute_store_node *child_node)
{
  this->child_nodes.erase(
    std::remove(this->child_nodes.begin(), this->child_nodes.end(), child_node),
    this->child_nodes.end());
}