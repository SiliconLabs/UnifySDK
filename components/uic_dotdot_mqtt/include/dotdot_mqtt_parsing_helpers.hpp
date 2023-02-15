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

/**
 * @defgroup dotdot_mqtt_parsing_helpers Dotdot MQTT internal shared functions
 * @ingroup dotdot_mqtt
 * @brief Dotdot MQTT internal shared functions (C++)
 *
 *
 * @{
 */

#ifndef DOTDOT_MQTT_PARSING_HELPERS_HPP
#define DOTDOT_MQTT_PARSING_HELPERS_HPP

#include "sl_status.h"
#include "uic_typedefs.h"

#include "dotdot_bitmap.hpp"

#include <string>
#include <map>

namespace uic_dotdot_mqtt
{
/**
 * @brief Helper function to extract unid from a topic string
 *
 * @param topic std::string to parse.
 * @param unid [out] unid variable to write.
 *
 * @returns true if topic could be parsed
 */
bool parse_topic_no_endpoint(const std::string &topic, std::string &unid);

/**
 * @brief Helper function to extract unid and endpoint from a topic string
 *
 * @param topic std::string to parse.
 * @param unid [out] unid variable to write.
 * @param endpoint [out] Endpoint variable to write.
 *
 * @returns true if topic could be parsed
 */
bool parse_topic(const std::string &topic,
                 std::string &unid,
                 dotdot_endpoint_id_t &endpoint);

/**
 * @brief Helper function to extract group ID from a topic string.
 *
 * @param topic     std::string to parse.
 * @param group_id  [out] group ID variable to populate.
 *
 *  @returns true if topic could be parsed
 */
bool parse_topic_group_id(const std::string &&topic,
                          dotdot_group_id_t &group_id);

/**
 * @brief Helper function to extract the last item of an MQTT topic
 *
 * For example, if the topic is ucl/2/ep3/example/test/23more/test
 * the last item is "test"
 *
 * @param topic std::string to parse.
 * @param last_item [out] String containing the last item
 *
 * @returns true if topic could be parsed
 */
sl_status_t get_topic_last_item(const std::string &topic,
                                std::string &last_item);

/**
 * @brief Finds the list of fields that are set to true/false for a bitmap.
 *
 * @returns a Map with keys and true/false values
 */
std::map<std::string, bool>
  get_bitmap_value_names(uint32_t value, dotdot_bitmap bitmap_handler);

/**
 * @brief Get the bitmap decimal value object
 *
 * @param key
 * @param jsn
 * @param bitmap_handler
 * @return uint32_t
 */
uint32_t get_bitmap_decimal_value(std::string key,
                                  nlohmann::json &jsn,
                                  dotdot_bitmap bitmap_handler);

/**
 * @brief Get the bitmap decimal value from array object
 *
 * @param index
 * @param jsn
 * @param bitmap_handler
 * @return uint32_t
 */
uint32_t get_bitmap_decimal_value_from_array(size_t index,
                                             nlohmann::json &jsn,
                                             dotdot_bitmap bitmap_handler);

}  // namespace uic_dotdot_mqtt

#endif  //DOTDOT_MQTT_PARSING_HELPERS_HPP
/** @} end dotdot_mqtt_parsing_helpers */
