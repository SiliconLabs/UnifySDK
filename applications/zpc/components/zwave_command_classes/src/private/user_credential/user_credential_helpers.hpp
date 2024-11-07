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

#ifndef USER_CREDENTIAL_HELPERS_H
#define USER_CREDENTIAL_HELPERS_H

// Get attribute store names
#include "attribute_store_defined_attribute_types.h"
// User credential types
#include "zwave_command_class_user_credential_types.h"

// Unify Cpp
#include "attribute.hpp"
#include "user_credential_definitions.hpp"

namespace user_credential_helpers
{

using attribute_callback = std::function<void(attribute_store::attribute &)>;

/////////////////////////////////////////////////////////////////////////////
// Data struct
/////////////////////////////////////////////////////////////////////////////

// Represent a credential ID (slot, type, user unique ID)
struct credential_id_nodes {
  attribute_store::attribute slot_node;
  attribute_store::attribute type_node;
  attribute_store::attribute user_unique_id_node;
};

/**
 * @brief Update desired value if found, or create the node otherwise
 * 
 * Check for the value in the desired value of attribute_type (with parent base_node).
 * If we found it, we update the reported value and clear the desired value.
 * Otherwise we create the node with the given value and set it to reported.
 * 
 * @tparam T Type of the value to set
 * 
 * @param base_node Base node to search for the attribute
 * @param attribute_type Type of the attribute to search for
 * @param value Value to search/set
 * 
 * @return attribute_store::attribute Node that was created/updated
 */
template<typename T> attribute_store::attribute
  create_or_update_desired_value(attribute_store::attribute base_node,
                                 attribute_store_type_t attribute_type,
                                 T value)
{
  auto node = base_node.child_by_type_and_value(attribute_type,
                                                value,
                                                DESIRED_ATTRIBUTE);
  if (!node.is_valid()) {
    node = base_node.emplace_node(attribute_type, value);
  } else {
    node.set_reported(value);
    node.clear_desired();
  }

  return node;
}

/**
 * @brief Create operation_type_node_type if it doesn't exists, and set the desired value to operation_type (and clear reported)
 * 
 * @see set_user_operation_type
 * @see set_credential_operation_type
 * @see set_credential_learn_operation_type
 * 
 * @param base_node Base node to search for the attribute
 * @param operation_type_node_type Type of the operation type node
 * @param operation_type Operation type to set
 *
 */
void set_operation_type(attribute_store::attribute base_node,
                        attribute_store_type_t operation_type_node_type,
                        user_credential_operation_type_t operation_type);
/**
 * @brief Set User Operation
 * 
 * Set the operation type as desired and clear reported to call SET function
 * 
 * @param user_node User node
 * @param operation_type Operation type to set
 * 
 */
void set_user_operation_type(attribute_store_node_t user_node,
                             user_credential_operation_type_t operation_type);

/**
 * @brief Set Credential Operation
 * 
 * Set the operation type as desired and clear reported to call SET function
 * 
 * @param slot_node Slot node
 * @param operation_type Operation type to set
 * 
 */
void set_credential_operation_type(
  attribute_store_node_t slot_node,
  user_credential_operation_type_t operation_type);

/**
 * @brief Set Credential Learn Operation
 * 
 * Set the operation type as desired and clear reported to call SET function
 * 
 * @param slot_node Slot node
 * @param operation_type Operation type to set
 */
void set_credential_learn_operation_type(
  attribute_store_node_t slot_node,
  user_credential_operation_type_t operation_type);

/**
 * @brief Get node associated with user ID (desired or reported)
 * 
 * @param endpoint_node  Current endpoint node
 * @param user_id        User ID
 * 
 * @return True is user exists, false otherwise
 */
bool user_exists(attribute_store::attribute endpoint_node,
                 user_credential_user_unique_id_t user_id);

/**
 * @brief Get node associated with user ID (desired or reported)
 * 
 * @param endpoint_node  Current endpoint node
 * @param user_id        User ID
 * @param state          Check reported or desired value (or desired else reported)
 * 
 * @throws std::runtime_error If User ID does not exist with given state
 * 
 * @return User ID Node
 */
attribute_store::attribute
  get_user_unique_id_node(attribute_store::attribute endpoint_node,
                          user_credential_user_unique_id_t user_id,
                          attribute_store_node_value_state_t state);

/**
 * @brief Get credential type node associated with user ID
 * 
 * @param user_id_node  User ID node
 * @param cred_type     Credential type
 * @param state         Check reported or desired value (or desired else reported)
 * 
 * @throws std::runtime_error If Credential type for given user_id_node does not exist with given state
 * 
 * @return Credential type node
 */
attribute_store::attribute
  get_credential_type_node(attribute_store::attribute user_id_node,
                           user_credential_type_t cred_type,
                           attribute_store_node_value_state_t state);

/**
 * @brief Get credential slot node associated with credential type
 * 
 * @param cred_type_node  Credential type node
 * @param cred_slot       Credential slot
 * @param state           Check reported or desired value (or desired else reported)
 * 
 * @throws std::runtime_error If Credential slot for given cred_type_node does not exist with given state
 * 
 * @return Credential slot node
 */
attribute_store::attribute
  get_credential_slot_node(attribute_store::attribute cred_type_node,
                           user_credential_slot_t cred_slot,
                           attribute_store_node_value_state_t state);
/**
 * @brief Iterate on each credential type nodes for a given user
 * 
 * @param user_id_node      User ID node
 * @param callback          Callback function to call for each credential type node
 * @param credential_type   Credential type to find. If 0, process all credential types
 */
void for_each_credential_type_nodes_for_user(
  attribute_store::attribute user_id_node,
  const attribute_callback &callback,
  user_credential_type_t credential_type = 0);

/**
 * @brief Iterate on each credential type nodes
 * 
 * @param endpoint_node     Endpoint point node
 * @param callback          Callback function to call for each credential type node
 * @param credential_type   Credential type to find. If 0, process all credential types
 */
void for_each_credential_type_nodes(attribute_store::attribute endpoint_node,
                                    const attribute_callback &callback,
                                    user_credential_type_t credential_type = 0);
/**
 * @brief Checks if given credential ID (credential type, credential slot) is available
 * 
 * @param endpoint_node     Endpoint node
 * @param credential_type   Credential type
 * @param credential_slot   Credential slot
 * 
 * @return true  Credential is available
 * @return false Credential is not available : if an user already have the combination of given credential type and slot.
*/
bool is_credential_available(attribute_store_node_t endpoint_node,
                             user_credential_type_t credential_type,
                             user_credential_slot_t credential_slot);

/**
 * @brief Get associated credential identifier nodes 
 * 
 * @param child_node Not that have a CREDENTIAL_SLOT, CREDENTIAL_TYPE and USER_UNIQUE_ID as respective parents
 * 
 * @throws std::runtime_error If one of the nodes is not found
 * 
 * @return credential_id_nodes Credential identifier nodes
 */
credential_id_nodes
  get_credential_identifier_nodes(attribute_store_node_t child_node);

template<typename T> struct identifier_state {
  T value;
  attribute_store_node_value_state_t state;
};
/**
 * @brief Get associated credential identifier nodes
 * 
 * @param endpoint_node Endpoint node
 * @param user_id       User ID with given state
 * @param credential_type Credential type with given state
 * @param credential_slot Credential slot with given state
 * 
 * @throws std::runtime_error If one of the nodes is not found
 * 
 * @return credential_id_nodes Credential identifier nodes
 */
credential_id_nodes get_credential_identifier_nodes(
  const attribute_store::attribute &endpoint_node,
  identifier_state<user_credential_user_unique_id_t> user_id,
  identifier_state<user_credential_type_t> credential_type,
  identifier_state<user_credential_slot_t> credential_slot);

/**
 * @brief Return true if given admin pin code is considered empty
 * 
 * We need this function since the attribute store doesn't support empty vector.
 *
 * @param admin_pin_code Admin pin code
 * 
 * @return true Admin pin code is empty
 * @return false Admin pin code is not empty
 */
bool is_admin_code_empty(const std::vector<uint8_t>& admin_pin_code);
/**
 * @brief Set the admin pin code node to an empty value
 * 
 * We need this function since the attribute store doesn't support empty vector.
 * 
 * @param admin_pin_code_node Admin pin code node
 * @param state State (REPORTED or DESIRED)
 * 
 * @throws std::runtime_error If admin_pin_code_node is not valid
 */
void set_empty_admin_code(attribute_store::attribute &admin_pin_code_node,
                          attribute_store_node_value_state_t state);

}  // namespace user_credential_helpers

#endif  // USER_CREDENTIAL_HELPERS_H