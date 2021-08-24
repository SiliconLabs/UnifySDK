/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "dotdot_bitmap.hpp"

// Boost
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

// standard library
#include <algorithm>

namespace bpt = boost::property_tree;

uint32_t
  dotdot_bitmap::convert_hex_string_to_uint32_t(const std::string &hex) const
{
  return std::stoi(hex, nullptr, 16);
}

std::vector<std::pair<std::string, std::string>>
  dotdot_bitmap::get_bitmap_values(uint32_t bitmap_value) const
{
  using namespace std;

  vector<pair<string, string>> bitmap_representation;

  for (const auto &[element, enum_data]: this->bitmap_data) {
    std::string type = element[this->idx_type];
    std::string name = element[this->idx_name];
    uint32_t mask    = convert_hex_string_to_uint32_t(element[this->idx_mask]);
    uint32_t bitmap_result
      = (bitmap_value & mask) >> std::stoi(element[this->idx_shiftright]);
    if (type == "bool") {
      string state = bitmap_result ? "true" : "false";

      bitmap_representation.emplace_back(pair<string, string>(name, state));
    } else if (type.find("enum") != std::string::npos) {
      string state
        = this->get_enum_name_of_element_by_value(enum_data, bitmap_result);
      bitmap_representation.emplace_back(pair<string, string>(name, state));
    } else if (type.find("uint") != std::string::npos) {
      uint32_t value = bitmap_result;
      bitmap_representation.emplace_back(
        pair<string, string>(name, to_string(value)));
    }
  }

  return bitmap_representation;
}

bpt::ptree dotdot_bitmap::get_bitmap_values_as_json_tree(
  uint32_t bitmap_decimal_value) const
{
  bpt::ptree resulting_tree;
  std::vector<std::pair<std::string, std::string>> bitmap_values
    = this->get_bitmap_values(bitmap_decimal_value);

  for (const auto &[value_name, value]: bitmap_values) {
    resulting_tree.add(value_name, value);
  }
  return resulting_tree;
}

uint32_t dotdot_bitmap::get_bitmap_state_value_from_json_tree(
  const bpt::ptree &bitmap_tree) const
{
  uint32_t resulting_value = 0;
  for (const auto &[element, enum_data]: this->bitmap_data) {
    std::string type  = element[idx_type];
    std::string value = bitmap_tree.get<std::string>(element[idx_name]);

    if (type == "bool" && value == "true") {
      resulting_value |= convert_hex_string_to_uint32_t(element[idx_mask]);
    } else if (type.find("enum") != std::string::npos) {
      resulting_value |= get_enum_value_of_element_by_name(enum_data, value);
    }
  }

  return resulting_value;
}

std::string dotdot_bitmap::get_enum_name_of_element_by_value(
  std::vector<std::vector<std::string>> enum_element, uint32_t value) const
{
  for (const auto &enum_data: enum_element) {
    if (value == convert_hex_string_to_uint32_t(enum_data[1])) {
      return enum_data[0];
    }
  }
  std::string error_string = "Enum value " + std::to_string(value)
                             + " does not exist for element "
                             + this->attribute_name;
  throw std::invalid_argument(error_string);
}

uint32_t dotdot_bitmap::get_enum_value_of_element_by_name(
  std::vector<std::vector<std::string>> enum_element,
  const std::string &name) const
{
  for (const auto &enum_data: enum_element) {
    if (enum_data[0] == name) {
      return this->convert_hex_string_to_uint32_t(enum_data[1]);
    }
  }
  std::string error_string = "Enum value " + name
                             + " does not exist for element "
                             + this->attribute_name;
  throw std::invalid_argument(error_string);
}
