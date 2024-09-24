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
#include "zwave_frame_generator.hpp"

// Unify
#include "attribute_store_type_registration.h"
#include "sl_log.h"
#include "attribute.hpp"

// Cpp includes
#include <stdexcept>
#include <sstream>  // for ostringstream
#include <algorithm>  // for std::for_each

constexpr char LOG_TAG[] = "zwave_frame_generator";

zwave_frame_generator::zwave_frame_generator(uint8_t zwave_command_class) :
  current_command_class(zwave_command_class)
{}

void zwave_frame_generator::initialize_frame(uint8_t zwave_command,
                                             uint8_t *data,
                                             uint16_t data_size)
{
  // Reset current frame index
  current_zwave_frame_index = 0;

  // Assignments
  current_zwave_frame      = data;
  current_zwave_frame_size = data_size;

  // Create frame header
  add_raw_byte(current_command_class);
  add_raw_byte(zwave_command);
}

void zwave_frame_generator::add_raw_byte(uint8_t byte)
{
  if (current_zwave_frame_index >= current_zwave_frame_size) {
    std::ostringstream out;
    out << "Attempt to set index " << current_zwave_frame_index
        << " when frame size is " << current_zwave_frame_size;
    throw std::out_of_range(out.str());
  }
  current_zwave_frame[current_zwave_frame_index++] = byte;
}

std::vector<uint8_t> zwave_frame_generator::helper_get_raw_data(
  attribute_store_node_t node,
  attribute_store_node_value_state_t node_value_state) const
{
  // Used to get the name of the attribute in case of error
  const attribute_store::attribute node_cpp(node);

  sl_log_debug(LOG_TAG, node_cpp.value_to_string().c_str());

  // First get value size
  uint8_t value_size = 0;

  // The logic isn't implemented for DESIRED_OR_REPORTED_ATTRIBUTE, so we need to do it
  // ourselves.
  if (node_value_state == DESIRED_OR_REPORTED_ATTRIBUTE) {
    value_size = attribute_store_get_node_value_size(node, DESIRED_ATTRIBUTE);
    if (value_size == 0) {
      value_size
        = attribute_store_get_node_value_size(node, REPORTED_ATTRIBUTE);
    }
  } else {
    value_size = attribute_store_get_node_value_size(node, node_value_state);
  }

  if (value_size == 0) {
    throw std::runtime_error(
      "Failed to get value size from attribute store for attribute "
      + node_cpp.name_and_id());
  }

  // Then get raw data
  std::vector<uint8_t> raw_data;
  raw_data.resize(value_size);
  sl_status_t status
    = attribute_store_get_node_attribute_value(node,
                                               node_value_state,
                                               raw_data.data(),
                                               &value_size);

  if (status != SL_STATUS_OK) {
    throw std::runtime_error("Failed to get value for attribute "
                             + node_cpp.name_and_id());
  }

  return raw_data;
}

void zwave_frame_generator::add_value(
  attribute_store_node_t node,
  attribute_store_node_value_state_t node_value_state)
{
  auto raw_data = helper_get_raw_data(node, node_value_state);

  auto storage_type
    = attribute_store_get_storage_type(attribute_store_get_node_type(node));
  // Remove the NULL terminator if we are dealing with a string
  if (storage_type == C_STRING_STORAGE_TYPE) {
    raw_data.pop_back();
  }

  // For non-numeric types we need to send them as is
  if (storage_type == C_STRING_STORAGE_TYPE
      || storage_type == BYTE_ARRAY_STORAGE_TYPE
      || storage_type == FIXED_SIZE_STRUCT_STORAGE_TYPE
      || storage_type == INVALID_STORAGE_TYPE) {
    // Store in order
    std::for_each(raw_data.begin(), raw_data.end(), [this](uint8_t byte) {
      add_raw_byte(byte);
    });
  }  // Otherwise the MSB is always the first in Z-Wave frames (and in attribute store it is last)
  else {
    // Store in reverse order (reverse iterator to start from the MSB)
    std::for_each(raw_data.rbegin(), raw_data.rend(), [this](uint8_t byte) {
      add_raw_byte(byte);
    });
  }
}

void zwave_frame_generator::add_shifted_values(
  const std::vector<shifted_value> &shifted_values)
{
  uint8_t final_value = 0;
  for (const auto &shifted_value: shifted_values) {
    uint8_t current_value = 0;
    // If we don't have a node, take the raw value instead
    if (shifted_value.node == ATTRIBUTE_STORE_INVALID_NODE) {
      current_value = shifted_value.raw_value;
    } else {
      auto raw_data = helper_get_raw_data(shifted_value.node,
                                          shifted_value.node_value_state);
      if (raw_data.size() != 1) {
        throw std::runtime_error("Shifted value should be 1 byte long");
      }
      current_value = raw_data[0];
    }
    final_value |= (current_value << shifted_value.left_shift);
  }
  add_raw_byte(final_value);
}

void zwave_frame_generator::add_shifted_values(const shifted_value &sv)
{
  std::vector<shifted_value> shifted_values = {sv};
  add_shifted_values(shifted_values);
}

void zwave_frame_generator::validate_frame(uint16_t *frame_length) const
{
  if (current_zwave_frame_index == current_zwave_frame_size) {
    *frame_length = current_zwave_frame_index;
  } else {
    std::ostringstream out;
    out << "Frame size (" << current_zwave_frame_index
        << ") does not match the expected size (" << current_zwave_frame_index
        << ")";
    throw std::runtime_error(out.str());
  }
}

sl_status_t zwave_frame_generator::generate_no_args_frame(
  uint8_t zwave_command_id, uint8_t *raw_data, uint16_t *frame_length)
{
  try {
    initialize_frame(zwave_command_id, raw_data, 2);
    validate_frame(frame_length);
  } catch (const std::exception &e) {
    sl_log_error(LOG_TAG, "Failed to generate frame: %s", e.what());
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}