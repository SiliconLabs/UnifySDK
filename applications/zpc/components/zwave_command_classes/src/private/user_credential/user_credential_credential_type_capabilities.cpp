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

#include "user_credential_credential_type_capabilities.h"

// Get attribute store names
#include "attribute_store_defined_attribute_types.h"

// UTF16 conversion (deprecated in C++17 but we don't have a better alternative yet)
// Needed for credential data (password) per specification
#include <locale>
#include <codecvt>

// Boost
#include <boost/format.hpp>

// Common definitions
#include "user_credential_definitions.hpp"

namespace user_credential
{

credential_type_capabilities::credential_type_capabilities(
  const attribute_store::attribute &endpoint_node,
  user_credential_type_t credential_type)
{
  this->is_data_valid = false;

  try {
    auto supported_credential_type_node = endpoint_node.child_by_type_and_value(
      ATTRIBUTE(SUPPORTED_CREDENTIAL_TYPE),
      credential_type);

    if (!supported_credential_type_node.is_valid()) {
      sl_log_error(
        LOG_TAG,
        "Credential type %d not supported. Can't get capabilities for it.",
        credential_type);
      return;
    }

    this->max_slot_count
      = supported_credential_type_node
          .child_by_type(ATTRIBUTE(CREDENTIAL_SUPPORTED_SLOT_COUNT))
          .reported<uint16_t>();
    this->learn_support = supported_credential_type_node
                            .child_by_type(ATTRIBUTE(CREDENTIAL_LEARN_SUPPORT))
                            .reported<uint8_t>();
    this->min_credential_length
      = supported_credential_type_node
          .child_by_type(ATTRIBUTE(CREDENTIAL_MIN_LENGTH))
          .reported<uint8_t>();
    this->max_credential_length
      = supported_credential_type_node
          .child_by_type(ATTRIBUTE(CREDENTIAL_MAX_LENGTH))
          .reported<uint8_t>();
    this->learn_recommended_timeout
      = supported_credential_type_node
          .child_by_type(ATTRIBUTE(CREDENTIAL_LEARN_RECOMMENDED_TIMEOUT))
          .reported<uint8_t>();
    this->learn_number_of_steps
      = supported_credential_type_node
          .child_by_type(ATTRIBUTE(CREDENTIAL_LEARN_NUMBER_OF_STEPS))
          .reported<uint8_t>();

    this->credential_type = credential_type;
    this->is_data_valid   = true;
  } catch (std::exception &e) {
    sl_log_error(LOG_TAG,
                 "Something was wrong getting credential capabilities : %s",
                 e.what());
  }
}

bool credential_type_capabilities::is_learn_supported() const
{
  return is_data_valid && learn_support > 0;
}

bool credential_type_capabilities::is_credential_valid(
  user_credential_type_t credential_type,
  user_credential_slot_t credential_slot,
  const std::vector<uint8_t> &credential_data) const
{
  if (!is_data_valid) {
    sl_log_error(
      LOG_TAG,
      "Credential capabilities are not valid. Try restarting the device.");
    return false;
  }

  if (credential_type != this->credential_type) {
    sl_log_error(LOG_TAG, "Credential type mismatch.");
    return false;
  }

  if (!is_slot_valid(credential_slot)) {
    sl_log_error(
      LOG_TAG,
      "Slot ID is not valid. Given : %d, Max Supported Slot count : %d",
      credential_slot,
      max_slot_count);
    return false;
  }

  if (!is_credential_data_valid(credential_data)) {
    sl_log_error(LOG_TAG,
                 "Credential data size is not valid. Should be between %d "
                 "and %d, given : %d",
                 min_credential_length,
                 max_credential_length,
                 credential_data.size());
    return false;
  }

  return true;
}

bool credential_type_capabilities::is_slot_valid(
  user_credential_slot_t credential_slot) const
{
  return credential_slot <= max_slot_count;
}

bool credential_type_capabilities::is_credential_data_valid(
  const std::vector<uint8_t> &credential_data) const
{
  return (credential_data.size() >= min_credential_length
          && credential_data.size() <= max_credential_length);
}

std::vector<uint8_t>
  credential_type_capabilities::convert_and_validate_credential_data(
    const char *credential_data, user_credential_slot_t credential_slot) const
{
  std::vector<uint8_t> credential_data_vector
    = convert_credential_data(credential_data);

  // Credential not valid, we are not adding it
  if (!this->is_credential_valid(this->credential_type,
                                 credential_slot,
                                 credential_data_vector)) {
    throw std::runtime_error("Credential capabilities are not valid.");
  }

  return credential_data_vector;
}

std::vector<uint8_t>
  credential_type_capabilities::convert_credential_data(const char *credential_data) const
{
  std::vector<uint8_t> credential_data_vector;
  std::string credential_data_str(credential_data);
  switch (credential_type) {
    case CREDENTIAL_REPORT_PASSWORD: {
      // CC:0083.01.0A.11.021 Passwords MUST be transmitted in Unicode UTF-16 format, in big endian order
      auto credential_data_utf16 = utf8_to_utf16(credential_data_str);
      for (const auto &c: credential_data_utf16) {
        credential_data_vector.push_back((uint8_t)(c >> 8));
        credential_data_vector.push_back((uint8_t)c);
      }
    } break;
    case CREDENTIAL_REPORT_PIN_CODE:
      for (const auto &c: credential_data_str) {
        if (c < '0' || c > '9') {
          throw std::runtime_error(
            (boost::format("Invalid character in PIN code : %1%. Only digits "
                           "are allowed.")
             % c)
              .str());
        }
        credential_data_vector.push_back(c);
      }
      break;
    default:
      for (const auto &c: credential_data_str) {
        credential_data_vector.push_back(c);
      }
  }

  return credential_data_vector;
}

uint8_t credential_type_capabilities::get_learn_recommended_timeout() const
{
  return learn_recommended_timeout;
}

std::u16string credential_type_capabilities::utf8_to_utf16(const std::string &utf8) const
{
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> cnv;
  std::u16string s = cnv.from_bytes(utf8);
  if (cnv.converted() < utf8.size())
    throw std::runtime_error("Something went wrong converting UTF8 to UTF16");
  return s;
}

}  // namespace user_credential