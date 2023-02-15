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

/**
 * @file dotdot_bitmap.hpp
 * @brief A class representation of a bitmap
 *
 * Class which is able to represent a bitmap and provides functionality to go
 * from a bitmap uint value to a json representation and the other way around.
 *
 * @{
 */

#ifndef DOTDOT_BITMAP_HPP
#define DOTDOT_BITMAP_HPP

// Standard library
#include <vector>
#include <map>
#include <string>
#include <nlohmann/json.hpp>

class dotdot_bitmap
{
  private:
  const uint8_t idx_name       = 0;
  const uint8_t idx_type       = 1;
  const uint8_t idx_mask       = 2;
  const uint8_t idx_shiftright = 3;

  std::string get_enum_name_of_element_by_value(
    std::vector<std::vector<std::string>> enum_element,
    uint32_t enum_idx) const;
  uint32_t get_enum_value_of_element_by_name(
    std::vector<std::vector<std::string>> enum_element,
    const std::string &name) const;

  const std::string attribute_name;
  const std::string map_type_string;
  const std::vector<
    std::pair<std::vector<std::string>, std::vector<std::vector<std::string>>>>
    bitmap_data;

  uint32_t convert_hex_string_to_uint32_t(const std::string &hex) const;

  public:
  dotdot_bitmap(
    const std::string &input_attribute_name,
    const std::string &input_map_type,
    const std::vector<std::pair<std::vector<std::string>,
                                std::vector<std::vector<std::string>>>>
      &input_bitmap_data) :
    attribute_name(input_attribute_name),
    map_type_string(input_map_type),
    bitmap_data(input_bitmap_data) {};

  std::vector<std::pair<std::string, std::string>>get_bitmap_values(uint32_t bitmap_value) const;

  nlohmann::json get_bitmap_values_as_json_tree(uint32_t bitmap_value) const;

  // bitmap_jsn param can't be a const because of limitations of the "const []" operator,
  // overloaded for nlohmann::json class
  uint32_t get_bitmap_state_value_from_json_tree(nlohmann::json &bitmap_jsn) const;
};

#endif  //DOTDOT_BITMAP_HPP
/** @} end dotdot_bitmap */
