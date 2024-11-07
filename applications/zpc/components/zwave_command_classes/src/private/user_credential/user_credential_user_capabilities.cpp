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

#include "user_credential_user_capabilities.h"

// Get attribute store names
#include "attribute_store_defined_attribute_types.h"

// Common definitions
#include "user_credential_definitions.hpp"


namespace user_credential
{


user_capabilities::user_capabilities(attribute_store::attribute endpoint_node) {
  try {
    this->max_user_count
      = endpoint_node.child_by_type(ATTRIBUTE(NUMBER_OF_USERS))
          .reported<uint16_t>();
    this->supported_credential_rules_bitmask
        = endpoint_node.child_by_type(ATTRIBUTE(SUPPORTED_CREDENTIAL_RULES))
            .reported<uint8_t>();
    this->supported_user_types_bitmask
        = endpoint_node.child_by_type(ATTRIBUTE(SUPPORTED_USER_TYPES))
            .reported<uint32_t>();  
    this->max_user_name_length
        = endpoint_node.child_by_type(ATTRIBUTE(MAX_USERNAME_LENGTH))
            .reported<uint8_t>();
    this->support_user_schedule
        = endpoint_node.child_by_type(ATTRIBUTE(SUPPORT_USER_SCHEDULE))
            .reported<uint8_t>();
    this->support_all_users_checksum
        = endpoint_node.child_by_type(ATTRIBUTE(SUPPORT_ALL_USERS_CHECKSUM))
            .reported<uint8_t>();
    this->support_by_user_checksum
        = endpoint_node.child_by_type(ATTRIBUTE(SUPPORT_USER_CHECKSUM))
            .reported<uint8_t>();   


    this->is_data_valid = true;
  } catch (std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Something was wrong getting user capabilities : %s",
                 e.what());
    attribute_store_log();
    this->is_data_valid = false;
  }
}


bool user_capabilities::is_user_valid(user_credential_user_unique_id_t user_id,
                                      user_credential_type_t user_type,
                                      user_credential_rule_t credential_rule,
                                      const char *user_name) const
{
  if (!is_data_valid) {
    sl_log_error(LOG_TAG,
                 "User capabilities are not valid. Try restarting the device.");
    return false;
  }

  if (!is_user_id_valid(user_id)) {
    sl_log_error(LOG_TAG, "User ID is not valid.");
    return false;
  }

  if (!is_user_type_supported(user_type)) {
    sl_log_error(LOG_TAG, "User type is not supported.");
    return false;
  }

  if (!is_credential_rule_supported(credential_rule)) {
    sl_log_error(LOG_TAG, "Credential rule is not supported.");
    return false;
  }

  if (!is_user_name_valid(user_name)) {
    sl_log_error(LOG_TAG, "User name is not valid.");
    return false;
  }
  return true;
}

bool user_capabilities::is_user_name_valid(const char *user_name) const
{
  std::string str_user_name(user_name);
  return str_user_name.length() <= max_user_name_length;
}

bool user_capabilities::is_user_id_valid(
  user_credential_user_unique_id_t user_id) const
{
  return user_id <= max_user_count;
}

bool user_capabilities::is_user_type_supported(
  user_credential_type_t user_type) const
{
  return (supported_user_types_bitmask & (1 << user_type));
}

bool user_capabilities::is_credential_rule_supported(
  user_credential_rule_t credential_rule) const
{
  return (supported_credential_rules_bitmask & (1 << credential_rule));
}

bool user_capabilities::is_all_users_checksum_supported() const
{
  return is_data_valid && support_all_users_checksum > 0;
}

bool user_capabilities::is_user_checksum_supported() const
{
  return is_data_valid && support_by_user_checksum > 0;
}

bool user_capabilities::is_user_schedule_supported() const
{
  return is_data_valid && support_user_schedule > 0;
}

}  // namespace user_credential