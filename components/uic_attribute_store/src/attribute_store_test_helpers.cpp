
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

#include "attribute_store_fixt.h"
#include "datastore.h"
#include "attribute_store_type_registration.h"
#include "attribute_store_test_helpers.hpp"

using namespace attribute_store;

struct BranchNode {
  attribute attr;
  uint8_t node_value;
  uint32_t node_type;
  bool is_parent;
};

// Creates attributes based on branch data vector starting from some branch root
// Returns true if all attributes created, false otherwise
static bool create_path(std::vector<BranchNode> *branch_data,
                        attribute branch_root)
{
  attribute branch_parent, parent, node;

  if (branch_root) {
    if (branch_root.is_valid()) {
      branch_parent = branch_root;
    } else {
      std::cout << "Error, unable to locate root node 0x" << std::hex
                << branch_root;
      return false;
    }
  } else {
    branch_parent = attribute::root();
  }

  std::vector<BranchNode>::iterator branch_data_begin = (*branch_data).begin();
  std::vector<BranchNode>::iterator branch_data_end   = (*branch_data).end();

  // Iterate over branch data and create branches
  for (std::vector<BranchNode>::iterator vec_it = branch_data_begin;
       vec_it != branch_data_end;
       vec_it++) {
    if (vec_it == branch_data_begin) {
      parent = branch_parent;
    } else {
      parent = (vec_it - 1)->attr;
    }
    if (vec_it->is_parent) {
      node = parent.child_by_type_and_value<uint8_t>(vec_it->node_type,
                                                     vec_it->node_value);
      if (!node.is_valid()) {
        node = parent.add_node(vec_it->node_type);
        node.set_reported<uint8_t>(vec_it->node_value);
      }
    } else {
      node = parent.add_node(vec_it->node_type);
    }
    if (node.is_valid()) {
      vec_it->attr = node;
    } else {
      return false;
    }
  }
  return true;
}

// Check if attributes based on branch data vector starting from some branch root exists
// Returns true if all attributes exists, false otherwise
static bool validate_path(std::vector<BranchNode> *branch_data,
                          attribute branch_root)
{
  attribute branch_parent, parent, node;

  if (branch_root) {
    if (branch_root.is_valid()) {
      branch_parent = branch_root;
    } else {
      std::cout << "Error, unable to locate root node 0x" << std::hex
                << branch_root;
      return false;
    }
  } else {
    branch_parent = attribute::root();
  }

  std::vector<BranchNode>::iterator branch_data_begin = (*branch_data).begin();
  std::vector<BranchNode>::iterator branch_data_end   = (*branch_data).end();

  // Lets iterate over branch nodes and check, if all required nodes in branch exists
  for (std::vector<BranchNode>::iterator vec_it = branch_data_begin;
       vec_it != branch_data_end;
       vec_it++) {
    if (vec_it == branch_data_begin) {
      parent = branch_parent;
    } else {
      parent = (vec_it - 1)->attr;
    }
    if (vec_it->is_parent) {
      node = parent.child_by_type_and_value<uint8_t>(vec_it->node_type,
                                                     vec_it->node_value);
      if (!node.is_valid()) {
        std::cout << " :Error, unable to locate node 0x" << std::hex
                  << vec_it->node_type;
        std::cout << "[" << (vec_it->node_value) << "]" << std::endl;
        return false;
      }
    } else {
      node = parent.child_by_type(vec_it->node_type);
      if (!node.is_valid()) {
        std::cout << " :Error, unable to locate node 0x" << std::hex
                  << vec_it->node_type << std::endl;
        return false;
      }
    }
    vec_it->attr = node;
  }
  return true;
}

// Converts node type from string or hex string to uint32_t
// return 0x0000000 (INVALID_TYPE) if cannot convert
static uint32_t convert_node_type(std::string s_node_type)
{
  //Decide if node is hex number
  uint32_t converted_value;
  if (s_node_type.find("0x") == 0 || s_node_type.find("0X") == 0) {
    try {
      converted_value = std::stoi(s_node_type, nullptr, 0);
    } catch (std::exception &e) {
      converted_value = 0xffffffff;
    }
  } else {
    converted_value = attribute_store_get_type_by_name(s_node_type.c_str());
  }
  return converted_value;
}

// Parse branch descriptor in put attribute type and value to vector
// Return true if parsed successfully, false otherwise
static bool parse_branch_descriptor(std::string &branch_descriptor,
                                    std::vector<BranchNode> *branch_data)
{
  // Split branch_descriptor to tokens, separated by '.' and add to vector
  const std::regex re_separator {"\\."};
  std::vector<std::string> node_list {
    std::sregex_token_iterator(branch_descriptor.cbegin(),
                               branch_descriptor.cend(),
                               re_separator,
                               -1),
    std::sregex_token_iterator()};

  BranchNode branch_node = {};

  // Iterate over node_list to identify value in []
  for (std::vector<std::string>::iterator it = std::begin(node_list);
       it != std::end(node_list);
       ++it) {
    std::string node = *it;
    // Split to node_type - node_value from tokens (node_type[node_value])
    uint8_t node_value = 0;
    uint32_t node_type = 0;
    bool is_parent     = false;

    size_t open_bracket  = node.find('[');
    size_t close_bracket = node.find(']');

    if (open_bracket == std::string::npos
        && close_bracket == std::string::npos) {
      node_type  = convert_node_type(node);
      node_value = 0;
      is_parent  = false;
    } else {
      node_type = convert_node_type(node.substr(0, open_bracket));
      if (open_bracket != std::string::npos
          && close_bracket != std::string::npos) {
        std::string s_node_value
          = node.substr(open_bracket + 1, close_bracket - open_bracket - 1);
        node_value = (uint8_t)std::stoi(s_node_value, nullptr, 0);
        is_parent  = true;
      } else {
        return false;
      }
    }
    if (node_type != 0xffffffff) {
      branch_node.node_type  = node_type;
      branch_node.node_value = node_value;
      branch_node.is_parent  = is_parent;
      branch_data->push_back(branch_node);
    } else {
      return false;
    }
  }
  return true;
}

attribute check_branch(std::string branch_descriptor, attribute branch_root)
{
  attribute node                       = ATTRIBUTE_STORE_INVALID_NODE;
  std::vector<BranchNode> *branch_data = new std::vector<BranchNode>();

  std::cout << "Checking branch: " << branch_descriptor;

  if (!parse_branch_descriptor(branch_descriptor, branch_data)) {
    return node;
  }
  // Validate branch
  if (!validate_path(branch_data, branch_root)) {
    delete branch_data;
    return node;
  }

  std::cout << " :OK" << std::endl;
  node = (*branch_data).back().attr;
  delete branch_data;
  return node;
}

attribute create_branch(std::string branch_descriptor, attribute branch_root)
{
  attribute node = ATTRIBUTE_STORE_INVALID_NODE;

  std::vector<BranchNode> *branch_data = new std::vector<BranchNode>();

  if (!parse_branch_descriptor(branch_descriptor, branch_data)) {
    delete branch_data;
    return node;
  }

  create_path(branch_data, branch_root);
  node = (*branch_data).back().attr;
  delete branch_data;
  return node;
}
