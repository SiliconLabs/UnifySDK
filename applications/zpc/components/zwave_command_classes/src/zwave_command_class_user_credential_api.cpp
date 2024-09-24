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

#include "zwave_command_class_user_credential_api.h"

#include "zap-types.h"

// Private definitions
#include "private/user_credential/user_credential_helpers.hpp"
#include "private/user_credential/user_credential_user_capabilities.h"
#include "private/user_credential/user_credential_credential_capabilities.h"
#include "private/user_credential/user_credential_credential_type_capabilities.h"

// Cpp includes
#include <map>
#include <string>

using namespace user_credential_helpers;

/**
 * @brief Setup user attributes (desired values)
 * 
 * This function sets the desired values of the user attributes in the attribute store.
 * If attributes are not present, they are created.
 */
void setup_user(attribute_store::attribute user_id_node,
                user_credential_type_t user_type,
                user_credential_rule_t credential_rule,
                user_credential_user_active_state_t user_active_state,
                user_credential_expiring_timeout_minutes_t expiring_timeout,
                user_credential_user_name_encoding_t user_name_encoding,
                std::string user_name)
{
  user_id_node.emplace_node(ATTRIBUTE(USER_TYPE)).set_desired(user_type);
  user_id_node.emplace_node(ATTRIBUTE(CREDENTIAL_RULE))
    .set_desired(credential_rule);
  user_id_node.emplace_node(ATTRIBUTE(USER_ACTIVE_STATE))
    .set_desired(user_active_state);
  user_id_node.emplace_node(ATTRIBUTE(USER_EXPIRING_TIMEOUT_MINUTES))
    .set_desired(expiring_timeout);
  user_id_node.emplace_node(ATTRIBUTE(USER_NAME_ENCODING))
    .set_desired(user_name_encoding);
  user_id_node.emplace_node(ATTRIBUTE(USER_NAME)).set_desired(user_name);
}

sl_status_t zwave_command_class_user_credential_add_new_user(
  attribute_store_node_t node,
  user_credential_user_unique_id_t user_id,
  user_credential_type_t user_type,
  user_credential_rule_t credential_rule,
  user_credential_user_active_state_t user_active_state,
  user_credential_expiring_timeout_minutes_t expiring_timeout,
  user_credential_user_name_encoding_t user_name_encoding,
  const char *user_name)
{
  // Debug info
  sl_log_debug(
    LOG_TAG,
    "zwave_command_class_user_credential_add_new_user called with : ");
  sl_log_debug(LOG_TAG, "\tuser_id : %d", user_id);
  sl_log_debug(LOG_TAG, "\tuser_type : %d", user_type);
  sl_log_debug(LOG_TAG, "\tcredential_rule : %d", credential_rule);
  sl_log_debug(LOG_TAG, "\tuser_active_state : %d", user_active_state);
  sl_log_debug(LOG_TAG, "\texpiring_timeout : %d", expiring_timeout);
  sl_log_debug(LOG_TAG, "\tuser_name_encoding : %d", user_name_encoding);
  sl_log_debug(LOG_TAG, "\tuser_name : %s", user_name);

  auto endpoint_node = attribute_store::attribute(node);

  // Check user id
  if (user_id == 0) {
    sl_log_error(LOG_TAG, "User ID 0 is reserved. Not adding user.");
    return SL_STATUS_FAIL;
  }

  // Node already exists, can't create user.
  if (user_exists(endpoint_node, user_id)) {
    sl_log_error(LOG_TAG,
                 "User with ID %d already exists. Not adding user.",
                 user_id);
    return SL_STATUS_FAIL;
  }

  try {
    // Check capabilites
    auto capabilites = user_credential::user_capabilities(endpoint_node);
    if (!capabilites.is_user_valid(user_id,
                                   user_type,
                                   credential_rule,
                                   user_name)) {
      sl_log_error(LOG_TAG,
                   "User capabilities are not valid. Not adding user.");
      return SL_STATUS_FAIL;
    }

    if (user_type != USER_CREDENTIAL_USER_TYPE_EXPIRING_USER) {
      if (expiring_timeout != 0) {
        sl_log_warning(
          LOG_TAG,
          "Expiring timeout set for non-expiring user, set value to 0.");
      }
      expiring_timeout = 0;
    }

    // Create the user node
    auto user_id_node = endpoint_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID),
                                                   user_id,
                                                   DESIRED_ATTRIBUTE);
    // Create user attributes
    setup_user(user_id_node,
               user_type,
               credential_rule,
               user_active_state,
               expiring_timeout,
               user_name_encoding,
               user_name);

    // Finally set operation type add
    set_user_operation_type(user_id_node, USER_CREDENTIAL_OPERATION_TYPE_ADD);

  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG, "Error while adding user : %s", e.what());
    return SL_STATUS_FAIL;
  }
  sl_log_debug(LOG_TAG, "Add user with ID %d", user_id);
  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_delete_user(
  attribute_store_node_t endpoint_node,
  user_credential_user_unique_id_t user_id)
{
  // Debug info
  sl_log_debug(
    LOG_TAG,
    "zwave_command_class_user_credential_delete_user called with : ");
  sl_log_debug(LOG_TAG, "\tuser_id : %d", user_id);

  try {
    if (!user_exists(endpoint_node, user_id)) {
      sl_log_error(LOG_TAG,
                   "User with ID %d doesn't exists. Not deleting user.",
                   user_id);
      return SL_STATUS_FAIL;
    }

    attribute_store_node_t user_id_node
      = get_user_unique_id_node(endpoint_node, user_id, REPORTED_ATTRIBUTE);

    // Finally set operation type delete
    set_user_operation_type(user_id_node,
                            USER_CREDENTIAL_OPERATION_TYPE_DELETE);
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG, "Error while deleting user : %s", e.what());
    return SL_STATUS_FAIL;
  }
  sl_log_debug(LOG_TAG, "Remove user with ID %d", user_id);

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_modify_user(
  attribute_store_node_t endpoint_node,
  user_credential_user_unique_id_t user_id,
  user_credential_type_t user_type,
  user_credential_rule_t credential_rule,
  user_credential_user_active_state_t user_active_state,
  user_credential_expiring_timeout_minutes_t expiring_timeout,
  user_credential_user_name_encoding_t user_name_encoding,
  const char *user_name)
{
  // Debug info
  sl_log_debug(
    LOG_TAG,
    "zwave_command_class_user_credential_modify_user called with : ");
  sl_log_debug(LOG_TAG, "\tuser_id : %d", user_id);
  sl_log_debug(LOG_TAG, "\tuser_type : %d", user_type);
  sl_log_debug(LOG_TAG, "\tcredential_rule : %d", credential_rule);
  sl_log_debug(LOG_TAG, "\tuser_active_state : %d", user_active_state);
  sl_log_debug(LOG_TAG, "\texpiring_timeout : %d", expiring_timeout);
  sl_log_debug(LOG_TAG, "\tuser_name_encoding : %d", user_name_encoding);
  sl_log_debug(LOG_TAG, "\tuser_name : %s", user_name);

  try {
    // Check user id
    if (user_id == 0) {
      sl_log_error(LOG_TAG, "User ID 0 is reserved. Can't modify user.");
      return SL_STATUS_FAIL;
    }

    if (!user_exists(endpoint_node, user_id)) {
      sl_log_error(LOG_TAG,
                   "User with ID %d doesn't exists. Can't modify user.",
                   user_id);
      return SL_STATUS_FAIL;
    }

    // Check if the user already exists
    attribute_store_node_t user_id_node
      = get_user_unique_id_node(endpoint_node, user_id, REPORTED_ATTRIBUTE);

    // Check capabilites
    auto capabilites = user_credential::user_capabilities(endpoint_node);
    if (!capabilites.is_user_valid(user_id,
                                   user_type,
                                   credential_rule,
                                   user_name)) {
      sl_log_error(LOG_TAG,
                   "User capabilities are not valid. Not adding user.");
      return SL_STATUS_FAIL;
    }

    // Modify user attributes
    setup_user(user_id_node,
               user_type,
               credential_rule,
               user_active_state,
               expiring_timeout,
               user_name_encoding,
               user_name);

    set_user_operation_type(user_id_node,
                            USER_CREDENTIAL_OPERATION_TYPE_MODIFY);
    sl_log_debug(LOG_TAG, "Modify user with ID %d", user_id);
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while modifying user %d : %s",
                 user_id,
                 e.what());
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_add_new_credential(
  attribute_store_node_t endpoint_node,
  user_credential_user_unique_id_t user_id,
  user_credential_type_t credential_type,
  user_credential_slot_t credential_slot,
  const char *credential_data)
{
  // Debug info
  sl_log_debug(
    LOG_TAG,
    "zwave_command_class_user_credential_add_new_credential called with : ");
  sl_log_debug(LOG_TAG, "\tuser_id : %d", user_id);
  sl_log_debug(LOG_TAG, "\tcredential_type : %d", credential_type);
  sl_log_debug(LOG_TAG, "\tcredential_slot : %d", credential_slot);
  sl_log_debug(LOG_TAG, "\tcredential_data : %s", credential_data);

  try {
    // Check if parameters are ok
    if (credential_type == 0 || credential_slot == 0) {
      sl_log_error(
        LOG_TAG,
        "Credential Type and Slot 0 are reserved. Not adding credentials.");
      return SL_STATUS_FAIL;
    }

    auto capabilities
      = user_credential::credential_type_capabilities(endpoint_node,
                                                      credential_type);

    if (!capabilities.is_slot_valid(credential_slot)) {
      sl_log_error(LOG_TAG,
                   "Credential slot %d for Credential Type %d is not valid. "
                   "Not adding credentials.",
                   credential_slot,
                   credential_type);
      return SL_STATUS_FAIL;
    }

    if (!is_credential_available(endpoint_node,
                                 credential_type,
                                 credential_slot)) {
      sl_log_error(LOG_TAG,
                   "Credential slot %d for Credential Type %d already exists."
                   "Not adding credentials.",
                   credential_slot,
                   credential_type);
      return SL_STATUS_FAIL;
    }

    auto user_id_node
      = get_user_unique_id_node(endpoint_node, user_id, REPORTED_ATTRIBUTE);
    // Get or create credential type node
    auto credential_type_node
      = user_id_node.emplace_node(ATTRIBUTE(CREDENTIAL_TYPE), credential_type);

    // Process credential data
    std::vector<uint8_t> credential_data_vector
      = capabilities.convert_and_validate_credential_data(credential_data,
                                                          credential_slot);

    auto credential_slot_node
      = credential_type_node.emplace_node(ATTRIBUTE(CREDENTIAL_SLOT),
                                          credential_slot,
                                          DESIRED_ATTRIBUTE);
    credential_slot_node.emplace_node(ATTRIBUTE(CREDENTIAL_DATA),
                                      credential_data_vector,
                                      DESIRED_ATTRIBUTE);
    set_credential_operation_type(credential_slot_node,
                                  USER_CREDENTIAL_OPERATION_TYPE_ADD);
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while trying to add a new credential : %s",
                 e.what());
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_modify_credential(
  attribute_store_node_t endpoint_node,
  user_credential_user_unique_id_t user_id,
  user_credential_type_t credential_type,
  user_credential_slot_t credential_slot,
  const char *credential_data)
{
  // Debug info
  sl_log_debug(
    LOG_TAG,
    "zwave_command_class_user_credential_modify_credential called with : ");
  sl_log_debug(LOG_TAG, "\tuser_id : %d", user_id);
  sl_log_debug(LOG_TAG, "\tcredential_type : %d", credential_type);
  sl_log_debug(LOG_TAG, "\tcredential_slot : %d", credential_slot);
  sl_log_debug(LOG_TAG, "\tcredential_data : %s", credential_data);

  // Check if parameters are ok
  if (user_id == 0 || credential_type == 0 || credential_slot == 0) {
    sl_log_error(LOG_TAG,
                 "User ID, Credential Type and Slot 0 are reserved. Not "
                 "modifying credentials.");
    return SL_STATUS_FAIL;
  }

  try {
    auto nodes
      = get_credential_identifier_nodes(endpoint_node,
                                        {user_id, REPORTED_ATTRIBUTE},
                                        {credential_type, REPORTED_ATTRIBUTE},
                                        {credential_slot, REPORTED_ATTRIBUTE});
    // Get current credential slot node
    auto credential_slot_node = nodes.slot_node;
    // Process credential data
    auto capabilities
      = user_credential::credential_type_capabilities(endpoint_node,
                                                      credential_type);
    std::vector<uint8_t> credential_data_vector
      = capabilities.convert_and_validate_credential_data(credential_data,
                                                          credential_slot);
    // Modify current data
    credential_slot_node.emplace_node(ATTRIBUTE(CREDENTIAL_DATA))
      .set_desired(credential_data_vector);

    // Finally set operation type modify
    set_credential_operation_type(credential_slot_node,
                                  USER_CREDENTIAL_OPERATION_TYPE_MODIFY);

  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while trying to modify a credential : %s",
                 e.what());
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_delete_credential(
  attribute_store_node_t endpoint_node,
  user_credential_user_unique_id_t user_id,
  user_credential_type_t credential_type,
  user_credential_slot_t credential_slot)
{
  sl_log_debug(LOG_TAG,
               "Delete credential slot %d (credential type %d, user id %d)",
               credential_slot,
               credential_type,
               user_id);

  try {
    auto nodes
      = get_credential_identifier_nodes(endpoint_node,
                                        {user_id, REPORTED_ATTRIBUTE},
                                        {credential_type, REPORTED_ATTRIBUTE},
                                        {credential_slot, REPORTED_ATTRIBUTE});

    // Finally set operation type delete
    set_credential_operation_type(nodes.slot_node,
                                  USER_CREDENTIAL_OPERATION_TYPE_DELETE);
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while trying to delete a credential : %s",
                 e.what());
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_delete_all_users(
  attribute_store_node_t endpoint_node)
{
  attribute_store::attribute cpp_endpoint_node(endpoint_node);
  user_credential_user_unique_id_t user_id = 0;
  auto user_id_node
    = cpp_endpoint_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id);

  // Finally set operation type delete
  set_user_operation_type(user_id_node, USER_CREDENTIAL_OPERATION_TYPE_DELETE);

  sl_log_debug(LOG_TAG,
               "Delete all user operation received. Creating user with id %d "
               "to send a User SET.",
               user_id);
  return SL_STATUS_OK;
}

void trigger_credential_deletion(attribute_store::attribute endpoint_node,
                                 user_credential_user_unique_id_t user_id,
                                 user_credential_type_t credential_type,
                                 user_credential_slot_t credential_slot)
{
  auto credential_slot_node
    = endpoint_node.emplace_node(ATTRIBUTE(USER_UNIQUE_ID), user_id)
        .emplace_node(ATTRIBUTE(CREDENTIAL_TYPE), credential_type)
        .emplace_node(ATTRIBUTE(CREDENTIAL_SLOT), credential_slot);

  // Finally set operation type delete
  set_credential_operation_type(credential_slot_node,
                                USER_CREDENTIAL_OPERATION_TYPE_DELETE);

  sl_log_debug(LOG_TAG,
               "Creating user with id %d, credential type %d and slot "
               "both to %d to send a Credential SET.",
               user_id,
               credential_type,
               credential_slot);
}

sl_status_t zwave_command_class_user_credential_delete_all_credentials(
  attribute_store_node_t endpoint_node)
{
  sl_log_debug(LOG_TAG, "Delete all credential operation received");

  trigger_credential_deletion(endpoint_node, 0, 0, 0);

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_delete_all_credentials_by_type(
  attribute_store_node_t endpoint_node, user_credential_type_t credential_type)
{
  sl_log_debug(LOG_TAG,
               "Delete all credential of type %d operation received",
               credential_type);

  trigger_credential_deletion(endpoint_node, 0, credential_type, 0);

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_delete_all_credentials_for_user(
  attribute_store_node_t endpoint_node,
  user_credential_user_unique_id_t user_id)
{
  sl_log_debug(LOG_TAG,
               "Delete all credential for user %d operation received",
               user_id);

  trigger_credential_deletion(endpoint_node, user_id, 0, 0);

  return SL_STATUS_OK;
}

sl_status_t
  zwave_command_class_user_credential_delete_all_credentials_for_user_by_type(
    attribute_store_node_t endpoint_node,
    user_credential_user_unique_id_t user_id,
    user_credential_type_t credential_type)
{
  sl_log_debug(LOG_TAG,
               "Delete all credential for user %d and credential type %d "
               "operation received",
               user_id,
               credential_type);

  trigger_credential_deletion(endpoint_node, user_id, credential_type, 0);

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_credential_learn_start_add(
  attribute_store_node_t endpoint_node,
  user_credential_user_unique_id_t user_id,
  user_credential_type_t credential_type,
  user_credential_slot_t credential_slot,
  user_credential_learn_timeout_t credential_learn_timeout)
{
  // Debug info
  sl_log_debug(
    LOG_TAG,
    "zwave_command_class_user_credential_credential_learn_start (Add)"
    " called with : ");
  sl_log_debug(LOG_TAG, "\tuser_id : %d", user_id);
  sl_log_debug(LOG_TAG, "\tcredential_type : %d", credential_type);
  sl_log_debug(LOG_TAG, "\tcredential_slot : %d", credential_slot);
  sl_log_debug(LOG_TAG,
               "\tcredential_learn_timeout : %d",
               credential_learn_timeout);

  auto credential_type_capabilities
    = user_credential::credential_type_capabilities(endpoint_node,
                                                    credential_type);

  if (!credential_type_capabilities.is_learn_supported()) {
    sl_log_error(LOG_TAG,
                 "Learn is not supported for credential type %d. Not starting "
                 "learn process.",
                 credential_type);
    return SL_STATUS_FAIL;
  }

  if (!credential_type_capabilities.is_slot_valid(credential_slot)) {
    sl_log_error(LOG_TAG,
                 "Credential slot %d is not valid for Credential Type %d. Not "
                 "starting learn process.",
                 credential_slot,
                 credential_type);
    return SL_STATUS_FAIL;
  }

  // Check parameters values
  if (credential_type == 0 || credential_slot == 0) {
    sl_log_error(
      LOG_TAG,
      "Credential Type and Slot 0 are reserved. Not adding credentials.");
    return SL_STATUS_FAIL;
  }

  if (!is_credential_available(endpoint_node,
                               credential_type,
                               credential_slot)) {
    sl_log_error(LOG_TAG,
                 "Credential slot %d for Credential Type %d already exists."
                 "Not adding credentials.",
                 credential_slot,
                 credential_type);
    return SL_STATUS_FAIL;
  }

  try {
    auto user_id_node
      = get_user_unique_id_node(endpoint_node, user_id, REPORTED_ATTRIBUTE);
    auto credential_type_node
      = user_id_node.emplace_node(ATTRIBUTE(CREDENTIAL_TYPE), credential_type);

    // Create credential slot with reported value since we don't want
    // to trigger a Credential Get right away
    auto credential_slot_node
      = credential_type_node.emplace_node(ATTRIBUTE(CREDENTIAL_SLOT),
                                          credential_slot);

    if (credential_learn_timeout == 0) {
      credential_learn_timeout
        = credential_type_capabilities.get_learn_recommended_timeout();
      sl_log_debug(LOG_TAG,
                   "Credential learn timeout is 0. Setting it to default "
                   "reported value (%d seconds).",
                   credential_learn_timeout);
    }

    // Set attributes for Credential Learn
    credential_slot_node.emplace_node(ATTRIBUTE(CREDENTIAL_LEARN_TIMEOUT))
      .set_reported(credential_learn_timeout);

    set_credential_learn_operation_type(credential_slot_node,
                                        USER_CREDENTIAL_OPERATION_TYPE_ADD);
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG, "Error in credential learn start : %s", e.what());
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_credential_learn_start_modify(
  attribute_store_node_t endpoint_node,
  user_credential_user_unique_id_t user_id,
  user_credential_type_t credential_type,
  user_credential_slot_t credential_slot,
  user_credential_learn_timeout_t credential_learn_timeout)
{
  // Debug info
  sl_log_debug(
    LOG_TAG,
    "zwave_command_class_user_credential_credential_learn_start (modify) "
    "called with : ");
  sl_log_debug(LOG_TAG, "\tuser_id : %d", user_id);
  sl_log_debug(LOG_TAG, "\tcredential_type : %d", credential_type);
  sl_log_debug(LOG_TAG, "\tcredential_slot : %d", credential_slot);
  sl_log_debug(LOG_TAG,
               "\tcredential_learn_timeout : %d",
               credential_learn_timeout);

  auto credential_type_capabilities
    = user_credential::credential_type_capabilities(endpoint_node,
                                                    credential_type);

  if (!credential_type_capabilities.is_learn_supported()) {
    sl_log_error(LOG_TAG,
                 "Learn is not supported for credential type %d. Not starting "
                 "learn process.",
                 credential_type);
    return SL_STATUS_FAIL;
  }

  // Check parameters values
  if (credential_type == 0 || credential_slot == 0) {
    sl_log_error(
      LOG_TAG,
      "Credential Type and Slot 0 are reserved. Not adding credentials.");
    return SL_STATUS_FAIL;
  }

  if (credential_learn_timeout == 0) {
    credential_learn_timeout
      = credential_type_capabilities.get_learn_recommended_timeout();
    sl_log_debug(LOG_TAG,
                 "Credential learn timeout is 0. Setting it to default "
                 "reported value (%d seconds).",
                 credential_learn_timeout);
  }

  try {
    auto nodes
      = get_credential_identifier_nodes(endpoint_node,
                                        {user_id, REPORTED_ATTRIBUTE},
                                        {credential_type, REPORTED_ATTRIBUTE},
                                        {credential_slot, REPORTED_ATTRIBUTE});

    auto credential_slot_node = nodes.slot_node;

    // Set attributes for Credential Learn
    credential_slot_node.emplace_node(ATTRIBUTE(CREDENTIAL_LEARN_TIMEOUT))
      .set_reported(credential_learn_timeout);
    set_credential_learn_operation_type(credential_slot_node,
                                        USER_CREDENTIAL_OPERATION_TYPE_MODIFY);
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error in credential learn start modify : %s",
                 e.what());
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_credential_learn_stop(
  attribute_store_node_t endpoint_node)
{
  uint8_t stop_flag = 1;

  return attribute_store_set_child_desired(endpoint_node,
                                           ATTRIBUTE(CREDENTIAL_LEARN_STOP),
                                           &stop_flag,
                                           sizeof(stop_flag));
}

sl_status_t zwave_command_class_user_credential_uuic_association_set(
  attribute_store_node_t endpoint_node,
  user_credential_type_t credential_type,
  user_credential_user_unique_id_t source_user_id,
  user_credential_slot_t source_credential_slot,
  user_credential_user_unique_id_t destination_user_id,
  user_credential_slot_t destination_credential_slot)
{
  try {
    auto nodes = get_credential_identifier_nodes(
      endpoint_node,
      {source_user_id, REPORTED_ATTRIBUTE},
      {credential_type, REPORTED_ATTRIBUTE},
      {source_credential_slot, REPORTED_ATTRIBUTE});

    nodes.slot_node.emplace_node(ATTRIBUTE(ASSOCIATION_DESTINATION_USER_ID))
      .set_desired(destination_user_id);

    // Slot ID last since it's this attribute that is bound to the SET command
    nodes.slot_node
      .emplace_node(ATTRIBUTE(ASSOCIATION_DESTINATION_CREDENTIAL_SLOT))
      .set_desired(destination_credential_slot);
  } catch (std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while setting up uuic asociation set : %s",
                 e.what());
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_get_user_checksum(
  attribute_store_node_t endpoint_node,
  user_credential_user_unique_id_t user_id)
{
  try {
    auto user_id_node
      = get_user_unique_id_node(endpoint_node, user_id, REPORTED_ATTRIBUTE);

    auto checksum_node = user_id_node.emplace_node(ATTRIBUTE(USER_CHECKSUM));
    // If node already exists, we clear its value to trigger the GET
    checksum_node.clear_reported();
    checksum_node.clear_desired();
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while setting up user get checksum : %s",
                 e.what());
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_get_credential_checksum(
  attribute_store_node_t endpoint_node, user_credential_type_t credential_type)
{
  try {
    attribute_store::attribute cpp_endpoint_node(endpoint_node);
    auto supported_credential_type_node
      = cpp_endpoint_node.child_by_type_and_value(
        ATTRIBUTE(SUPPORTED_CREDENTIAL_TYPE),
        credential_type);

    if (!supported_credential_type_node.is_valid()) {
      sl_log_error(LOG_TAG,
                   "Can't find supported credential type %d. Not setting up "
                   "Checksum get.",
                   credential_type);
      return SL_STATUS_FAIL;
    }

    auto checksum_node = supported_credential_type_node.emplace_node(
      ATTRIBUTE(CREDENTIAL_CHECKSUM));
    // If node already exists, we clear its value to trigger the GET
    checksum_node.clear_reported();
    checksum_node.clear_desired();
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while setting up credential get checksum : %s",
                 e.what());
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_get_all_users_checksum(
  attribute_store_node_t endpoint_node)
{
  try {
    attribute_store::attribute cpp_endpoint_node(endpoint_node);
    auto checksum_node
      = cpp_endpoint_node.emplace_node(ATTRIBUTE(ALL_USERS_CHECKSUM));
    checksum_node.clear_reported();
    checksum_node.clear_desired();
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while setting up get all users checksum : %s",
                 e.what());
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_user_credential_set_admin_pin_code(
  attribute_store_node_t endpoint_node, const char *raw_credential_data)
{
  try {
    // Capabilities
    user_credential::credential_type_capabilities credential_type_capabilities(
      endpoint_node,
      ZCL_CRED_TYPE_PIN_CODE);
    user_credential::credential_capabilities credential_capabilities(
      endpoint_node);

    // Base check
    if (!credential_capabilities.is_admin_code_supported()) {
      sl_log_error(
        LOG_TAG,
        "Admin PIN code not supported.");
      return SL_STATUS_FAIL;
    }

    // Get credential data in the right format
    auto credential_data
      = credential_type_capabilities.convert_credential_data(raw_credential_data);

    // Create or get admin pin code node
    auto admin_pin_code_node = attribute_store::attribute(endpoint_node)
                                 .emplace_node(ATTRIBUTE(ADMIN_PIN_CODE));

    // Check if we can deactivate it
    if (credential_data.empty()
        && !credential_capabilities.is_admin_code_deactivation_supported()) {
      sl_log_error(
        LOG_TAG,
        "Admin PIN code deactivation disabled, can't deactivate it.");
      return SL_STATUS_FAIL;
    }

    // Special case if credential data is empty
    // Need this since no value is the same as undefined for the attribute store
    if (credential_data.empty()) {
      sl_log_debug(LOG_TAG, "Admin pin code data is empty. Deactivating it.");
      set_empty_admin_code(admin_pin_code_node, DESIRED_ATTRIBUTE);
    } else if (credential_type_capabilities.is_credential_data_valid(
                 credential_data)) {
      // Set new value for desired node
      admin_pin_code_node.set_desired(credential_data);
    } else {
      sl_log_error(LOG_TAG,
                   "Admin pin code data is not valid. Not setting it.");
      return SL_STATUS_FAIL;
    }

  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Error while setting up set admin pin code : %s",
                 e.what());
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

bool zwave_command_class_user_credential_supports(
  attribute_store_node_t endpoint_node, uint8_t command_id)
{
  // Checks first if the node supports User Credential
  auto cpp_endpoint_node = attribute_store::attribute(endpoint_node);
  if (!cpp_endpoint_node.child_by_type(ATTRIBUTE(VERSION)).is_valid()) {
    return false;
  }

  // Commands that don't need specific capabilities
  // Note : We don't have the granularity to check if the learn is supported for a specific credential at this level
  // so some commands (e.g CREDENTIAL_LEARN_START) are marked as supported even if the credential type doesn't support it.
  // The API call will check that for us instead.
  const std::vector<uint8_t> supported_commands = {
    USER_SET,
    CREDENTIAL_SET,
    CREDENTIAL_LEARN_START, 
    CREDENTIAL_LEARN_CANCEL,
    USER_CREDENTIAL_ASSOCIATION_SET
  };

  // Check if we support the command_id without checking the capabilities
  if (std::find(supported_commands.begin(),
                supported_commands.end(),
                command_id)
      != supported_commands.end()) {
    return true;
  }

  user_credential::user_capabilities user_capabilities(endpoint_node);
  user_credential::credential_capabilities credential_capabilities(
    endpoint_node);

  switch (command_id) {
    case ALL_USERS_CHECKSUM_GET:
      return user_capabilities.is_all_users_checksum_supported();
    case USER_CHECKSUM_GET:
      return user_capabilities.is_user_checksum_supported();
    case CREDENTIAL_CHECKSUM_GET:
      return credential_capabilities.is_credential_checksum_supported();
    case ADMIN_PIN_CODE_SET: 
      return credential_capabilities.is_admin_code_supported();
    default:
      sl_log_critical(
        LOG_TAG,
        "Unknown command %d in zwave_command_class_user_credential_supports",
        command_id);
      return false;
  }
}

bool zwave_command_class_user_credential_supports_admin_pin_code_deactivation(
  attribute_store_node_t endpoint_node)
{
  user_credential::credential_capabilities credential_capabilities(
    endpoint_node);
  return credential_capabilities.is_admin_code_deactivation_supported();
}
