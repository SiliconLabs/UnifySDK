/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
// Get attribute store names
#include "user_credential_helpers.hpp"

// Boost
#include "boost/format.hpp"

// Cpp standard includes
#include <vector>
namespace user_credential_helpers
{


void set_operation_type(attribute_store::attribute base_node,
                        attribute_store_type_t operation_type_node_type,
                        user_credential_operation_type_t operation_type)
{
  auto operation_type_node = base_node.emplace_node(operation_type_node_type);

  // Undefine reported to be sure that we can so the same operation twice in a row
  operation_type_node.clear_reported();
  operation_type_node.set_desired(operation_type);
}

void set_user_operation_type(attribute_store_node_t user_node,
                             user_credential_operation_type_t operation_type)
{
  set_operation_type(
    user_node,
    ATTRIBUTE(USER_OPERATION_TYPE),
    operation_type);
}

void set_credential_operation_type(
  attribute_store_node_t slot_node,
  user_credential_operation_type_t operation_type)
{
  set_operation_type(slot_node,
                     ATTRIBUTE(CREDENTIAL_OPERATION_TYPE),
                     operation_type);
}

void set_credential_learn_operation_type(
  attribute_store_node_t slot_node,
  user_credential_operation_type_t operation_type)
{
  set_operation_type(slot_node,
                     ATTRIBUTE(CREDENTIAL_LEARN_OPERATION_TYPE),
                     operation_type);
}

bool user_exists(attribute_store::attribute endpoint_node,
                 user_credential_user_unique_id_t user_id)
{
  return endpoint_node
    .child_by_type_and_value(ATTRIBUTE(USER_UNIQUE_ID),
                             user_id,
                             REPORTED_ATTRIBUTE)
    .is_valid();
}

attribute_store::attribute
  get_user_unique_id_node(attribute_store::attribute endpoint_node,
                          user_credential_user_unique_id_t user_id,
                          attribute_store_node_value_state_t state)
{
  attribute_store::attribute user_id_node
    = endpoint_node.child_by_type_and_value(ATTRIBUTE(USER_UNIQUE_ID),
                                            user_id,
                                            state);

  if (!user_id_node.is_valid()) {
    throw std::runtime_error(
      (boost::format("User ID %1% not found (state : %2%).") % user_id % state)
        .str());
  }

  return user_id_node;
}


attribute_store::attribute
  get_credential_type_node(attribute_store::attribute user_id_node,
                           user_credential_type_t cred_type,
                           attribute_store_node_value_state_t state)
{
  if (!user_id_node.is_valid()) {
    throw std::runtime_error(
      "get_credential_type_node: User ID node is not valid.");
  }

  attribute_store::attribute cred_type_node
    = user_id_node.child_by_type_and_value(ATTRIBUTE(CREDENTIAL_TYPE),
                                           cred_type,
                                           state);

  if (!cred_type_node.is_valid()) {
    throw std::runtime_error(
      (boost::format("Credential type  %1% (state : %2%) not found for %3%.")
       % cred_type % state % user_id_node.value_to_string())
        .str());
  }

  return cred_type_node;
}

attribute_store::attribute
  get_credential_slot_node(attribute_store::attribute cred_type_node,
                           user_credential_slot_t cred_slot,
                           attribute_store_node_value_state_t state)
{
  if (!cred_type_node.is_valid()) {
    throw std::runtime_error(
      "get_credential_slot_node: Credential Type node is not valid.");
  }

  attribute_store::attribute cred_slot_node
    = cred_type_node.child_by_type_and_value(ATTRIBUTE(CREDENTIAL_SLOT),
                                             cred_slot,
                                             state);

  if (!cred_slot_node.is_valid()) {
    throw std::runtime_error(
      (boost::format(
         "Credential slot  %1% (state : %2%) not found for %3% / %4%.")
       % cred_slot % state % cred_type_node.value_to_string()
       % cred_type_node.parent().value_to_string())
        .str());
  }

  return cred_slot_node;
}

void for_each_credential_type_nodes_for_user(
  attribute_store::attribute user_id_node,
  const attribute_callback &callback,
  user_credential_type_t credential_type)
{
  auto credential_type_nodes
    = user_id_node.children(ATTRIBUTE(CREDENTIAL_TYPE));
  for (auto &credential_type_node: credential_type_nodes) {
    // Call
    if (credential_type == 0
        || (credential_type_node.reported_exists()
            && credential_type_node.reported<user_credential_type_t>()
                 == credential_type)) {
      callback(credential_type_node);
    }
  }
}

void for_each_credential_type_nodes(attribute_store::attribute endpoint_node,
                                    const attribute_callback &callback,
                                    user_credential_type_t credential_type)
{
  auto user_nodes = endpoint_node.children(ATTRIBUTE(USER_UNIQUE_ID));
  for (auto &user_node: user_nodes) {
    for_each_credential_type_nodes_for_user(user_node,
                                            callback,
                                            credential_type);
  }
}


bool is_credential_available(attribute_store_node_t endpoint_node,
                             user_credential_type_t credential_type,
                             user_credential_slot_t credential_slot)
{
  bool credential_available = true;

  for_each_credential_type_nodes(
    endpoint_node,
    [&credential_available, &credential_slot](
      attribute_store::attribute &current_credential_type_node) {
      for (auto &current_credential_slot_node:
           current_credential_type_node.children(ATTRIBUTE(CREDENTIAL_SLOT))) {
        // If this credential slot node doesn't have a reported value, check the next one
        if (!current_credential_slot_node.reported_exists()) {
          continue;
        }

        if (current_credential_slot_node.reported<user_credential_slot_t>()
            == credential_slot) {
          credential_available = false;
          return;
        }
      }
    },
    credential_type);

  return credential_available;
}


credential_id_nodes
  get_credential_identifier_nodes(attribute_store_node_t child_node)
{
  attribute_store::attribute slot_node(child_node);
  slot_node = slot_node.first_parent_or_self(ATTRIBUTE(CREDENTIAL_SLOT));
  attribute_store::attribute type_node
    = slot_node.first_parent(ATTRIBUTE(CREDENTIAL_TYPE));
  attribute_store::attribute user_unique_id_node
    = type_node.first_parent(ATTRIBUTE(USER_UNIQUE_ID));

  if (!slot_node.is_valid()) {
    throw std::runtime_error(
      "get_credential_identifier_nodes: Can't get credential slot node.");
  }

  if (!type_node.is_valid()) {
    throw std::runtime_error(
      "get_credential_identifier_nodes: Can't get credential type node.");
  }

  if (!user_unique_id_node.is_valid()) {
    throw std::runtime_error(
      "get_credential_identifier_nodes: Can't get user unique ID node.");
  }

  return {.slot_node           = slot_node,
          .type_node           = type_node,
          .user_unique_id_node = user_unique_id_node};
}

credential_id_nodes get_credential_identifier_nodes(
  const attribute_store::attribute &endpoint_node,
  identifier_state<user_credential_user_unique_id_t> user_id,
  identifier_state<user_credential_type_t> credential_type,
  identifier_state<user_credential_slot_t> credential_slot)
{
  credential_id_nodes nodes;

  nodes.user_unique_id_node
    = get_user_unique_id_node(endpoint_node, user_id.value, user_id.state);

  if (!nodes.user_unique_id_node.is_valid()) {
    throw std::runtime_error(
      "get_credential_identifier_nodes: Can't get user unique ID "
      + std::to_string(user_id.value));
  }

  nodes.type_node = get_credential_type_node(nodes.user_unique_id_node,
                                             credential_type.value,
                                             credential_type.state);

  if (!nodes.type_node.is_valid()) {
    throw std::runtime_error(
      (boost::format("get_credential_identifier_nodes: Can't get credential "
                     "type %1% for user %2%")
       % credential_type.value % user_id.value)
        .str());
  }

  nodes.slot_node = get_credential_slot_node(nodes.type_node,
                                             credential_slot.value,
                                             credential_slot.state);

  if (!nodes.slot_node.is_valid()) {
    throw std::runtime_error(
      (boost::format("get_credential_identifier_nodes: Can't get credential "
                     "slot %1% for credential type %2% / user %3%")
       % credential_slot.value % credential_type.value % user_id.value)
        .str());
  }
  return nodes;
}

bool is_admin_code_empty(const std::vector<uint8_t>& admin_pin_code)
{
  return admin_pin_code.size() == 1 && admin_pin_code[0] == 0;
}

void set_empty_admin_code(attribute_store::attribute &admin_pin_code_node,
                          attribute_store_node_value_state_t state)
{
  if (!admin_pin_code_node.is_valid()) {
    throw std::runtime_error("Admin code node is not valid.");
  }
  std::vector<uint8_t> empty_code = {0};
  admin_pin_code_node.set(state, empty_code);
}

}  // namespace user_credential_helpers