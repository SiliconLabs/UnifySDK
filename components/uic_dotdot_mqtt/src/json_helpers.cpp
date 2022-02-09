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
#include "json_helpers.hpp"
#include "zap-types.h"

#include <nlohmann/json.hpp>
#include <boost/algorithm/string.hpp>

#include <string>

[[maybe_unused]] bool get_bool_from_json(nlohmann::json &jsn, const std::string &key)
{
  if(jsn[key].is_boolean()) {
    return jsn[key].get<bool>();
  } else if (jsn[key].is_string()) {
    std::string payload_value = jsn[key].get<std::string>();

    if (std::string("true") == payload_value) {
      return true;
    } else if (std::string("false") == payload_value) {
      return false;
    } else {
      // assume that string payload is an integer number - try to convert it to boolean
      try {
        boost::replace_all(payload_value, "\"", "");
        int value = std::stoi(payload_value, nullptr, 0);
        return static_cast<bool>(value);
      } catch ([[maybe_unused]] const std::invalid_argument &e) {
        // actually, this should never happen
        return false;
      }
    }
  } else if (jsn[key].is_number_integer()) {
    unsigned int payload_value = jsn[key].get<unsigned int>();
    return static_cast<bool>(payload_value);
  }

  return false;
}

[[maybe_unused]] bool get_bool_from_json_array(nlohmann::json &jsn, size_t index)
{
  if(jsn.at(index).is_boolean()) {
    return jsn.at(index).get<bool>();
  } else if (jsn.at(index).is_string()) {
    std::string payload_value = jsn.at(index).get<std::string>();

    if (std::string("true") == payload_value) {
      return true;
    } else if (std::string("false") == payload_value) {
      return false;
    } else {
      // assume that string payload is an integer number - try to convert it to boolean
      try {
        boost::replace_all(payload_value, "\"", "");
        int value = std::stoi(payload_value, nullptr, 0);
        return static_cast<bool>(value);
      } catch ([[maybe_unused]] const std::invalid_argument &e) {
        // actually, this should never happen
        return false;
      }
    }
  } else if (jsn.at(index).is_number_integer()) {
    unsigned int payload_value = jsn.at(index).get<unsigned int>();
    return static_cast<bool>(payload_value);
  }

  return false;
}


[[maybe_unused]] nlohmann::json add_transitions_to_json_obj(const TransitionType *data_array, const uint8_t arr_size)
{
  nlohmann::json jsn_array = nlohmann::json::array();
  TransitionType data;

  for (uint8_t i = 0; i < arr_size; i++) {
    data = data_array[i];

    nlohmann::json jsn;
    jsn["TransitionTime"] = data.TransitionTime;
    jsn["HeatSetPoint"]   = data.HeatSetPoint;
    jsn["CoolSetPoint"]   = data.CoolSetPoint;

    jsn_array.push_back(jsn);
  }

  return jsn_array;
}

[[maybe_unused]] nlohmann::json add_extension_field_to_json_obj(const SExtensionFieldSetList *data_array, const uint8_t arr_size)
{
  nlohmann::json jsn_array = nlohmann::json::array();
  SExtensionFieldSetList data;

  for (uint8_t i = 0; i < arr_size; i++) {
    data = data_array[i];

    nlohmann::json jsn;
    jsn["ClusterId"]         = data.ClusterId;
    jsn["ExtensionFieldSet"] = std::string(data.ExtensionFieldSet);

    jsn_array.push_back(jsn);
  }

  return jsn_array;
}
