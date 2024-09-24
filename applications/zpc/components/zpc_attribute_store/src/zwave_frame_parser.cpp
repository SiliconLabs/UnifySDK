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
#include "zwave_frame_parser.hpp"

// Unify C
#include "attribute_store_helper.h"

// Cpp libraries
#include <stdexcept>
#include <bitset>

#define LOG_TAG "zwave_frame_parser"

zwave_frame_parser::zwave_frame_parser(const uint8_t *data, uint16_t length)
{
  zwave_report_frame.assign(data, data + length);
}

bool zwave_frame_parser::is_frame_size_valid(uint8_t min_size,
                                             uint8_t max_size) const
{
  if (max_size == 0) {
    max_size = min_size;
  }

  uint8_t frame_size = static_cast<uint8_t>(zwave_report_frame.size());
  bool frame_valid   = (frame_size >= min_size) && (frame_size <= max_size);

  if (!frame_valid) {
    if (max_size == min_size) {
      sl_log_warning(LOG_TAG,
                     "Invalid frame size expected %d, got %d",
                     min_size,
                     frame_size);
    } else {
      sl_log_warning(LOG_TAG,
                     "Invalid frame size expected between %d and %d, got %d",
                     min_size,
                     max_size,
                     frame_size);
    }
  }

  return frame_valid;
}

uint8_t zwave_frame_parser::read_byte()
{
  uint8_t value_from_frame = zwave_report_frame.at(current_index);
  current_index++;
  return value_from_frame;
}

uint8_t zwave_frame_parser::read_byte(attribute_store_node_t node)
{
  auto value = read_byte();

  helper_store_value(node, value);

  return value;
}

zwave_report_bitmask_t zwave_frame_parser::read_bitmask()
{
  constexpr uint8_t SUPPORT_BITMASK_SIZE = sizeof(zwave_report_bitmask_t);
  constexpr uint8_t BITMASK_SIZE         = SUPPORT_BITMASK_SIZE * 8;

  uint8_t bitmask_length                    = this->read_byte();
  std::bitset<BITMASK_SIZE> support_bitmask = 0;

  if (bitmask_length > SUPPORT_BITMASK_SIZE) {
    throw std::runtime_error(
      "zwave_report_bitmask_t supports only bitmask of max size of "
      + std::to_string(SUPPORT_BITMASK_SIZE) + " bytes");
  }

  // Implementation node : we don't use sequential read here since for the bitmask the MSB is at the end.
  // While standard read in numeric type assume that the MSB is at the beginning.
  for (int i = 0; i < bitmask_length; i++) {
    std::bitset<BITMASK_SIZE> current_bitmask = this->read_byte();
    support_bitmask |= current_bitmask << (8 * i);
  }

  // Convert bitmask into zwave_report_bitmask_t
  zwave_report_bitmask_t support_bitmask_value
    = static_cast<zwave_report_bitmask_t>(support_bitmask.to_ulong());

  return support_bitmask_value;
}

zwave_report_bitmask_t
  zwave_frame_parser::read_bitmask(attribute_store_node_t node)
{
  zwave_report_bitmask_t value = read_bitmask();

  // Store value if applicable
  helper_store_value(node, value);

  return value;
}

zwave_frame_parser::zwave_parser_bitmask_result
  zwave_frame_parser::read_byte_with_bitmask(
    const std::vector<bitmask_data> &bitmask_data)
{
  zwave_frame_parser::zwave_parser_bitmask_result read_values;

  if (!bitmask_data.empty()) {
    uint8_t value_from_frame = read_byte();
    
    for (const auto &current_bitmask: bitmask_data) {
      // Extract value
      uint8_t current_value = current_bitmask.bitmask & value_from_frame;
      // Compute shift to get the actual value (e.g if we want bit 3 & 4 both at 1 we want to store 0b11 not 0b1100)
      uint8_t shift = get_trailing_zeroes(current_bitmask.bitmask);
      current_value >>= shift;

      read_values.insert({current_bitmask.bitmask, current_value});

      // Guard to avoid printing error message
      if (attribute_store_node_exists(current_bitmask.destination_node)) {
        helper_store_value(current_bitmask.destination_node, current_value);
      }
    }
  }
  return read_values;
}

zwave_frame_parser::zwave_parser_bitmask_result
  zwave_frame_parser::read_byte_with_bitmask(const bitmask_data &data)
{
  std::vector<bitmask_data> bitmask_data {data};
  return read_byte_with_bitmask(bitmask_data);
}

std::string zwave_frame_parser::read_string()
{
  uint8_t string_length = read_byte();

  return read_sequential<std::string>(string_length);
}

std::string zwave_frame_parser::read_string(attribute_store_node_t node)
{
  std::string value = read_string();

  helper_store_value(node, value);

  return value;
}

// Can use C++20 bit manipulation instead, but for now we are in C++17.
// https://en.cppreference.com/w/cpp/numeric/countr_zero
uint8_t
  zwave_frame_parser::get_trailing_zeroes(const std::bitset<8> &bitset) const
{
  std::bitset<8> temp_bitset = bitset;
  uint8_t trailing_zeroes    = 0;

  if (temp_bitset.any()) {
    /* mask the 4 low order bits, add 4 and shift them out if they are all 0 */
    if ((temp_bitset & std::bitset<8>(0xF)).none()) {
      trailing_zeroes += 4;
      temp_bitset >>= 4;
    }
    /* mask the 2 low order bits, add 2 and shift them out if they are all 0 */
    if ((temp_bitset & std::bitset<8>(0x3)).none()) {
      trailing_zeroes += 2;
      temp_bitset >>= 2;
    }
    /* mask the low order bit and add 1 if it is 0 */
    if ((temp_bitset & std::bitset<8>(0x1)).none()) {
      trailing_zeroes += 1;
    }
  }
  return trailing_zeroes;
}