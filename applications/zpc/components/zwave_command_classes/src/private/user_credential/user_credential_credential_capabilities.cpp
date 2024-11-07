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

#include "user_credential_credential_capabilities.h"

// Get attribute store names
#include "attribute_store_defined_attribute_types.h"

// Boost
#include <boost/format.hpp>

// Common definitions
#include "user_credential_definitions.hpp"

namespace user_credential
{

credential_capabilities::credential_capabilities(
  const attribute_store::attribute &endpoint_node)
{
  this->is_data_valid = false;

  try {
    this->credential_checksum_support
      = endpoint_node.child_by_type(ATTRIBUTE(SUPPORT_CREDENTIAL_CHECKSUM))
          .reported<uint8_t>();
    this->admin_code_support
      = endpoint_node.child_by_type(ATTRIBUTE(SUPPORT_ADMIN_PIN_CODE))
          .reported<uint8_t>();
    this->admin_code_deactivation_support
      = endpoint_node
          .child_by_type(ATTRIBUTE(SUPPORT_ADMIN_PIN_CODE_DEACTIVATION))
          .reported<uint8_t>();

    this->is_data_valid = true;
  } catch (std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Something was wrong getting credential capabilities : %s",
                 e.what());
  }
}

bool credential_capabilities::is_credential_checksum_supported() const
{
  return is_data_valid && credential_checksum_support > 0;
}
bool credential_capabilities::is_admin_code_supported() const
{
  return is_data_valid && admin_code_support > 0;
}
bool credential_capabilities::is_admin_code_deactivation_supported() const
{
  return is_data_valid && admin_code_deactivation_support > 0;
}

}  // namespace user_credential
