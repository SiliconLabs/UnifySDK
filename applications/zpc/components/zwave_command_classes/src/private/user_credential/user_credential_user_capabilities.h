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

#ifndef USER_CREDENTIAL_USER_CAPABILITIES_H
#define USER_CREDENTIAL_USER_CAPABILITIES_H

// Attribute store
#include "attribute.hpp"
// User credential types
#include "zwave_command_class_user_credential_types.h"


namespace user_credential
{

class user_capabilities
{
  public:
  explicit user_capabilities(attribute_store::attribute endpoint_node);
  ~user_capabilities() = default;

  /**
   * @brief Check if the user proprieties are valid
   * 
   * @note Will return false if is_data_valid is false
   * 
   * @param user_id          User ID
   * @param user_type        User type
   * @param credential_rule  Credential rule
   * @param user_name        User name
   * 
   * @return true  User is valid
   * @return false User is not valid
  */
  bool is_user_valid(user_credential_user_unique_id_t user_id,
                     user_credential_type_t user_type,
                     user_credential_rule_t credential_rule,
                     const char *user_name) const;

  /**
   * @brief Checks if the given user name is valid.
   * @param user_name The user name to be validated.
   * @return true User name is valid
   * @return false User name is not valid
   */
  bool is_user_name_valid(const char *user_name) const;

  /**
   * @brief Check if a user id is valid
   * @param user_id User ID to check
   * @return true User ID is valid
   * @return false User ID is not valid
  */
  bool is_user_id_valid(user_credential_user_unique_id_t user_id) const;
  /** 
   * @brief Check if a user type is supported
   * @param user_type User type to check
   * @return true User type is supported
   * @return false User type is not supported
  */
  bool is_user_type_supported(user_credential_type_t user_type) const;

  /**
   * @brief Check if a credential rule is supported
   * @param credential_rule Credential rule to check
   * @return true Credential rule is supported
   * @return false Credential rule is not supported
  */
  bool
    is_credential_rule_supported(user_credential_rule_t credential_rule) const;

  /**
   * @brief Check if all users checksum is supported
   * @return true All users checksum is supported
   * @return false All users checksum is not supported
   */
  bool is_all_users_checksum_supported() const;

  /**
   * @brief Check if user checksum is supported
   * @return true User checksum is supported
   * @return false User checksum is not supported
   */
  bool is_user_checksum_supported() const;
  /**
  * @brief Check if user schedule is supported
  * @return true User schedule is supported
  * @return false User schedule is not supported
  */
  bool is_user_schedule_supported() const;

  private:
  // Maximum number of users that can be stored in the device
  uint16_t max_user_count = 0;
  // Credential rules supported
  uint8_t supported_credential_rules_bitmask = 0;
  // User types supported
  uint32_t supported_user_types_bitmask = 0;
  // Max length for the user names
  uint8_t max_user_name_length = 0;
  // Device support for scheduling users
  uint8_t support_user_schedule = 0;
  // Device support for getting the checksum of all users
  uint8_t support_all_users_checksum = 0;
  // Device support for getting the checksum of a specific user
  uint8_t support_by_user_checksum = 0;

  // True if the data is valid inside this struct
  bool is_data_valid = false;
};

}  // namespace user_credential

#endif  // USER_CREDENTIAL_USER_CAPABILITIES_H