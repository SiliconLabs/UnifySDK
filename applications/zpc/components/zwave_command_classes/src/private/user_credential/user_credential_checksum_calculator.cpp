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
// Base class
#include "user_credential_checksum_calculator.h"

// CRC 16
#include "zwave_controller_crc16.h"

// Common definitions
#include "user_credential_definitions.hpp"

namespace user_credential
{

// Used to compute checksums
constexpr uint16_t CRC_INITIALIZATION_VALUE = 0x1D0F;
constexpr uint8_t MAX_CHAR_SIZE             = 64;

std::vector<uint8_t>
  get_raw_data_from_node(const attribute_store::attribute &node)
{
  std::vector<uint8_t> raw_data;
  auto value_size
    = attribute_store_get_node_value_size(node, REPORTED_ATTRIBUTE);

  if (value_size == 0) {
    throw std::runtime_error(
      "Failed to get value size from attribute store for attribute "
      + node.name_and_id());
  }

  raw_data.resize(value_size);
  sl_status_t status
    = attribute_store_get_node_attribute_value(node,
                                               REPORTED_ATTRIBUTE,
                                               raw_data.data(),
                                               &value_size);

  if (status != SL_STATUS_OK) {
    throw std::runtime_error("Failed to get value for attribute "
                             + node.name_and_id());
  }

  return raw_data;
}

void checksum_calculator::add_node(const attribute_store::attribute &node)
{
  if (!node.is_valid()) {
    sl_log_error(LOG_TAG, "Can't find node %d. Not adding to checksum.", node);
    throw std::runtime_error("Invalid node");
  }

  // Will throw and exception if the conversion failed
  auto raw_data = get_raw_data_from_node(node);

  // Add the raw data to the checksum data
  auto storage_type = attribute_store_get_storage_type(node.type());

  // Remove the NULL terminator if we are dealing with a string
  if (storage_type == C_STRING_STORAGE_TYPE) {
    raw_data.pop_back();
  }

  // Send it as is but append the size of the value at the beginning
  if (storage_type == C_STRING_STORAGE_TYPE
      || storage_type == BYTE_ARRAY_STORAGE_TYPE
      || storage_type == FIXED_SIZE_STRUCT_STORAGE_TYPE
      || storage_type == INVALID_STORAGE_TYPE) {
    raw_data.insert(raw_data.begin(), raw_data.size());
    checksum_data.insert(checksum_data.end(), raw_data.begin(), raw_data.end());
  }
  // Otherwise the MSB is always the first in Z-Wave frames
  // (and in attribute store it is last) so we need to reverse the order
  else {
    checksum_data.insert(checksum_data.end(),
                         raw_data.rbegin(),
                         raw_data.rend());
  }
}

user_credential_checksum_t checksum_calculator::compute_checksum()
{
  user_credential_checksum_t computed_checksum = 0;
  // If checksum data is empty, the checksum is 0. The guard is present to avoid
  // zwave_controller_crc16 to return CRC_INITIALIZATION_VALUE if checksum_data is empty.
  // See CC:0083.01.19.11.016 & CC:0083.01.17.11.013
  if (checksum_data.size() > 0) {
    computed_checksum = zwave_controller_crc16(CRC_INITIALIZATION_VALUE,
                                               checksum_data.data(),
                                               checksum_data.size());
  }
  return computed_checksum;
}

}  // namespace user_credential