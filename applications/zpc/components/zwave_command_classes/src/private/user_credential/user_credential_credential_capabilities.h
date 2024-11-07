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

#ifndef USER_CREDENTIAL_CREDENTIAL_CAPABILITIES_H
#define USER_CREDENTIAL_CREDENTIAL_CAPABILITIES_H

// Attribute store
#include "attribute.hpp"

namespace user_credential
{

class credential_capabilities
{
  public:
  explicit credential_capabilities(
    const attribute_store::attribute &endpoint_node);
  ~credential_capabilities() = default;

  bool is_credential_checksum_supported() const;
  bool is_admin_code_supported() const;
  bool is_admin_code_deactivation_supported() const;

  private:
  uint8_t credential_checksum_support     = 0;
  uint8_t admin_code_support              = 0;
  uint8_t admin_code_deactivation_support = 0;

  bool is_data_valid = false;
};

}  // namespace user_credential

#endif  // USER_CREDENTIAL_CREDENTIAL_CAPABILITIES_H