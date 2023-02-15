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
 * @file json_helpers.hpp
 * @brief Helper functions for nlohmann::json library
 * @{
 */

#ifndef JSON_HELPERS_HPP
#define JSON_HELPERS_HPP

#include "zap-types.h"

#include <nlohmann/json.hpp>

/**
 * @brief Converts json payload (either boolean, string or integer) to bool value.
 *        This function is used because nlohmann::json library can't automatically convert
 *        string or numerical json payload to boolean value and it fails in such cases.
 *
 * @returns Boolean value that represents the payload.
 */
[[maybe_unused]] bool get_bool_from_json(nlohmann::json &jsn,
                                         const std::string &key);

/**
 * @brief Converts json array element (either boolean, string or integer) to bool value.
 *
 * @param jsn   JSON array to inspect (json::array())
 * @param index Index of the element to inspect
 *
 * @returns Boolean value that represents the value of the nth element.
 */
[[maybe_unused]] bool get_bool_from_json_array(nlohmann::json &jsn,
                                               size_t index);

/**
 * @brief The pointer to array of "TransitionType" structs couldn't be automatically converted to nlohmann::json object -
 *        so this function provides a manual conversion.
 *
 * @param data_array - pointer to array of TransitionType structs that should be converted to json object
 * @param arr_size   - the number of elements in @a data_array
 *
 * @returns nlohmann::json object.
 */
[[maybe_unused]] nlohmann::json
  add_transitions_to_json_obj(const TransitionType *data_array,
                              const uint8_t arr_size);

/**
 * @brief The pointer to array of "SExtensionFieldSetList" structs couldn't be automatically converted to nlohmann::json object -
 *        so this function provides a manual conversion.
 *
 * @param data_array - pointer to array of SExtensionFieldSetList structs that should be converted to json object
 * @param arr_size   - the number of elements in @a data_array
 *
 * @returns nlohmann::json object.
 */
[[maybe_unused]] nlohmann::json
  add_extension_field_to_json_obj(const SExtensionFieldSetList *data_array,
                                  const uint8_t arr_size);

#endif  //JSON_HELPERS_HPP
/** @} end json_helpers */
