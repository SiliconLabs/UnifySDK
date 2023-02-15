/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "dotdot_mqtt_parsing_helpers.hpp"
#include "dotdot_mqtt.hpp"

// Generic includes
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

// Unify includes
#include "sl_log.h"
#include "uic_typedefs.h"

static constexpr char LOG_TAG[] = "dotdot_mqtt_parsing_helpers";

bool uic_dotdot_mqtt::parse_topic_no_endpoint(const std::string &topic,
                                              std::string &unid)
{
  std::vector<std::string> topic_elements;
  boost::algorithm::split(topic_elements, topic, boost::is_any_of("/"));

  if (topic_elements.size() < 3) {
    return false;
  }
  unid = topic_elements[2];
  return true;
}

bool uic_dotdot_mqtt::parse_topic(const std::string &topic,
                                  std::string &unid,
                                  dotdot_endpoint_id_t &endpoint)
{
  std::vector<std::string> topic_elements;
  boost::algorithm::split(topic_elements, topic, boost::is_any_of("/"));

  if (topic_elements.size() < 5) {
    return false;
  }
  unid                  = topic_elements[2];
  const std::string &ep = topic_elements[3];
  if ((ep.size() < 3) || ep.at(0) != 'e' || ep.at(1) != 'p')
    return false;
  int n = std::stoi(topic_elements[3].substr(2));
  if ((n < 0) || (n > 255)) {
    return false;
  }
  endpoint = static_cast<dotdot_endpoint_id_t>(n);
  return true;
}

bool uic_dotdot_mqtt::parse_topic_group_id(const std::string &&topic,
                                           dotdot_group_id_t &group_id)
{
  std::vector<std::string> topic_elements;
  boost::algorithm::split(topic_elements, topic, boost::is_any_of("/"));

  if (topic_elements.size() < 3) {
    return false;
  }

  const std::string &group = topic_elements[2];
  if (group.size() == 0) {
    return false;
  }

  int n;
  try {
    n = std::stoi(group);
  } catch (...) {
    sl_log_debug(LOG_TAG, "Failed to parse GroupId");
    return false;
  }

  if ((n < 1) || (n > 65535)) {
    sl_log_debug(LOG_TAG, "GroupID outside valid range");
    return false;
  }

  group_id = n;
  return true;
}

sl_status_t uic_dotdot_mqtt::get_topic_last_item(const std::string &topic,
                                                 std::string &last_item)
{
  std::vector<std::string> topic_elements;
  boost::algorithm::split(topic_elements, topic, boost::is_any_of("/"));

  if (topic_elements.empty()) {
    return SL_STATUS_FAIL;
  }

  // Just return the last element.
  last_item = topic_elements.back();
  return SL_STATUS_OK;
}

std::map<std::string, bool>
  uic_dotdot_mqtt::get_bitmap_value_names(uint32_t value,
                                          dotdot_bitmap bitmap_handler)
{
  // Get it parsed by the bitmap_handler
  std::vector<std::pair<std::string, std::string>> values
    = bitmap_handler.get_bitmap_values(value);

  std::map<std::string, bool> result;
  for (auto const &element: values) {
    if (element.second.compare("true") == 0) {
      result[element.first] = true;
    } else {
      result[element.first] = false;
    }
  }
  return result;
}

uint32_t uic_dotdot_mqtt::get_bitmap_decimal_value(std::string key,
                                                   nlohmann::json &jsn,
                                                   dotdot_bitmap bitmap_handler)
{
  if (jsn[key].is_number_integer()) {
    return static_cast<uint32_t>(jsn[key]);
  } else if (jsn[key].is_string()) {
    std::string val_str;
    try {
      val_str        = jsn[key].get<std::string>();
      uint32_t value = std::stoi(val_str, nullptr, 0);
      sl_log_warning(LOG_TAG,
                     "Converted string '%s' to integer value and accepted it "
                     "as bitmap argument.",
                     val_str.c_str());
      return value;
    } catch (const std::invalid_argument &e) {
      sl_log_error(LOG_TAG,
                   "Provided invalid '%s' value :'%s'.",
                   key.c_str(),
                   val_str.c_str());
      return std::numeric_limits<uint32_t>::max();
    }
  } else if (jsn[key].is_object()) {
    // Json object passed - so we have to parse it and get bitmap value from passed key:value pairs
    auto retval
      = bitmap_handler.get_bitmap_state_value_from_json_tree(jsn[key]);
    return retval;
  } else {
    sl_log_error(LOG_TAG, "Provided invalid Json payload.");
    return std::numeric_limits<uint32_t>::max();
  }
}

uint32_t uic_dotdot_mqtt::get_bitmap_decimal_value_from_array(
  size_t index, nlohmann::json &jsn, dotdot_bitmap bitmap_handler)
{
  if (jsn.at(index).is_number_integer()) {
    return static_cast<uint32_t>(jsn.at(index));
  } else if (jsn.at(index).is_string()) {
    std::string val_str;
    try {
      val_str        = jsn.at(index).get<std::string>();
      uint32_t value = std::stoi(val_str, nullptr, 0);
      sl_log_warning(LOG_TAG,
                     "Converted string '%s' to integer value and accepted it "
                     "as bitmap argument.",
                     val_str.c_str());
      return value;
    } catch (const std::invalid_argument &e) {
      sl_log_error(LOG_TAG,
                   "Provided invalid index (%i) for JSON array.",
                   index);
      return std::numeric_limits<uint32_t>::max();
    }
  } else if (jsn.at(index).is_object()) {
    // Json object passed - so we have to parse it and get bitmap value from passed key:value pairs
    auto retval
      = bitmap_handler.get_bitmap_state_value_from_json_tree(jsn.at(index));
    return retval;
  } else {
    sl_log_error(LOG_TAG, "Provided invalid Json payload.");
    return std::numeric_limits<uint32_t>::max();
  }
}
