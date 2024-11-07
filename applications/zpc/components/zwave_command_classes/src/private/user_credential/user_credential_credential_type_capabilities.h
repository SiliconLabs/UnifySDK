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

#ifndef USER_CREDENTIAL_CREDENTIAL_TYPE_CAPABILITIES_H
#define USER_CREDENTIAL_CREDENTIAL_TYPE_CAPABILITIES_H

// Cpp definitions
#include <vector>
#include <string>

// Attribute store
#include "attribute.hpp"
// User credential types
#include "zwave_command_class_user_credential_types.h"

namespace user_credential
{

class credential_type_capabilities
{
  public:
  explicit credential_type_capabilities(
    const attribute_store::attribute &endpoint_node,
    user_credential_type_t credential_type);
  ~credential_type_capabilities() = default;

  bool is_learn_supported() const;

  bool is_credential_valid(user_credential_type_t credential_type,
                           user_credential_slot_t credential_slot,
                           const std::vector<uint8_t> &credential_data) const;

  bool is_slot_valid(user_credential_slot_t credential_slot) const;
  bool
    is_credential_data_valid(const std::vector<uint8_t> &credential_data) const;

  /**
 * @brief Convert and validate credential data str to a vector of uint8_t
 * 
 * Convenience function to convert data and check if it is valid as per the credential capabilities.
 * 
 * @param credential_data Credential data to convert
 * @param credential_slot Credential slot
 * 
 * @throws std::runtime_error if the credential data is invalid
 * 
 * @return The converted data
 */
  std::vector<uint8_t> convert_and_validate_credential_data(
    const char *credential_data, user_credential_slot_t credential_slot) const;

  /**
 * @brief Convert credential data str to a vector of uint8_t
 * 
 * @note CC:0083.01.0A.11.021 Passwords MUST be transmitted in Unicode UTF-16 format, in big endian order
 * 
 * @param credential_data Credential data to convert
 * 
 * @throws std::runtime_error if the credential data is invalid
 * 
 * @return The converted data
*/
  std::vector<uint8_t> convert_credential_data(const char *credential_data) const;

  uint8_t get_learn_recommended_timeout() const;

  private:

  std::u16string utf8_to_utf16(const std::string &utf8) const;

  user_credential_type_t credential_type = 0;
  uint16_t max_slot_count                = 0;
  uint8_t learn_support                  = 0;
  uint8_t min_credential_length          = 0;
  uint8_t max_credential_length          = 0;
  uint8_t learn_recommended_timeout      = 0;
  uint8_t learn_number_of_steps          = 0;

  bool is_data_valid = false;
};

}  // namespace user_credential

#endif  // USER_CREDENTIAL_CREDENTIAL_TYPE_CAPABILITIES_H