/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include <string>
#include <set>

/* Boost libraries */
#include <boost/optional/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

/* UIC libraries */
#include "sl_log.h"
#include "sl_status.h"

#include "zigpc_common_zigbee.h"

#include "zcl_profiles.h"

#include "zcl_profiles_command_fields.h"

namespace bpt = boost::property_tree;

static const char *LOG_TAG = "zcl_profiles_parse_build_command_fields";

/**
 * @brief List of standard underlying data types for ZCL command fields.
 *
 */
static std::set<std::string> STANDARD_DATA_FORMATS {
  "bool", "uint8_t", "uint16_t", "uint32_t", "int8_t", "int16_t", "int32_t"};

sl_status_t zcl_profiles_parse_build_command_fields(
  const char *message,
  size_t message_length,
  size_t command_field_count,
  const zigpc_zcl_profiles_command_field_t *command_fields,
  zcl_frame_t *const zcl_frame)
{
  sl_status_t status = SL_STATUS_OK;
  std::stringstream write_ss;
  std::string message_str(message);
  bpt::iptree json_payload;

  if ((message = NULL) || (command_fields == NULL) || (message_length == 0)) {
    status = SL_STATUS_NULL_POINTER;
  }

  if ((status == SL_STATUS_OK) && (message_str.length() == 0)) {
    status = SL_STATUS_EMPTY;
  }

  if (status == SL_STATUS_OK) {
    write_ss << message_str;
    try {
      bpt::json_parser::read_json(write_ss, json_payload);
    } catch (const bpt::json_parser_error &err) {
      sl_log_warning(LOG_TAG,
                     "Failed to parse command field object: %s",
                     err.what());
      json_payload.clear();
      status = SL_STATUS_OBJECT_READ;
    }
  }

  if ((status == SL_STATUS_OK) && (json_payload.empty())) {
    status = SL_STATUS_EMPTY;
  }

  if (status == SL_STATUS_OK) {
    for (size_t i = 0; (i < command_field_count) && (status == SL_STATUS_OK);
         i++) {
      std::string command_format(command_fields[i].format);

      // Convert any non-standard data types from the enum table if possible.
      if (STANDARD_DATA_FORMATS.find(command_format)
          == STANDARD_DATA_FORMATS.end()) {
        command_format = zigpc_zcl_profiles_get_enum_underlying_type(
          command_fields[i].format);
        if (command_format == "unknown") {
          sl_log_error(LOG_TAG,
                       "Failed to find enum '%s' mapping to data type",
                       command_fields[i].format);
        } else {
          sl_log_debug(LOG_TAG,
                       "Converted enum type '%s' to '%s'",
                       command_fields[i].name,
                       command_fields[i].format,
                       command_format.c_str());
        }
      }

      if (command_format == "bool") {
        boost::optional<bool> bool_data;
        bool_data = json_payload.get_optional<bool>(command_fields[i].name);
        if (bool_data) {
          *bool_data = *bool_data != false;
          status     = zigpc_common_zcl_frame_fill_data(zcl_frame,
                                                    ZCL_DATA_TYPE_UINT8,
                                                    &*bool_data);
        } else {
          status = SL_STATUS_NOT_FOUND;
        }
      } else if (command_format == "int8_t") {
        boost::optional<int8_t> i8_data;
        i8_data = json_payload.get_optional<int8_t>(command_fields[i].name);
        if (i8_data) {
          status = zigpc_common_zcl_frame_fill_data(zcl_frame,
                                                    ZCL_DATA_TYPE_INT8,
                                                    &*i8_data);
        } else {
          status = SL_STATUS_NOT_FOUND;
        }
      } else if (command_format == "uint8_t") {
        boost::optional<uint8_t> u8_data;
        u8_data = json_payload.get_optional<uint8_t>(command_fields[i].name);
        if (u8_data) {
          status = zigpc_common_zcl_frame_fill_data(zcl_frame,
                                                    ZCL_DATA_TYPE_UINT8,
                                                    &*u8_data);
        } else {
          status = SL_STATUS_NOT_FOUND;
        }
      } else if (command_format == "int16_t") {
        boost::optional<int16_t> i16_data;
        i16_data = json_payload.get_optional<int16_t>(command_fields[i].name);
        if (i16_data) {
          status = zigpc_common_zcl_frame_fill_data(zcl_frame,
                                                    ZCL_DATA_TYPE_INT16,
                                                    &*i16_data);
        } else {
          status = SL_STATUS_NOT_FOUND;
        }
      } else if (command_format == "uint16_t") {
        boost::optional<uint16_t> u16_data;
        u16_data = json_payload.get_optional<uint16_t>(command_fields[i].name);
        if (u16_data) {
          status = zigpc_common_zcl_frame_fill_data(zcl_frame,
                                                    ZCL_DATA_TYPE_UINT16,
                                                    &*u16_data);
        } else {
          status = SL_STATUS_NOT_FOUND;
        }
      } else if (command_format == "int32_t") {
        boost::optional<int32_t> i32_data;
        i32_data = json_payload.get_optional<int32_t>(command_fields[i].name);
        if (i32_data) {
          status = zigpc_common_zcl_frame_fill_data(zcl_frame,
                                                    ZCL_DATA_TYPE_INT32,
                                                    &*i32_data);
        } else {
          status = SL_STATUS_NOT_FOUND;
        }
      } else if (command_format == "uint32_t") {
        boost::optional<uint32_t> u32_data;
        u32_data = json_payload.get_optional<uint32_t>(command_fields[i].name);
        if (u32_data) {
          status = zigpc_common_zcl_frame_fill_data(zcl_frame,
                                                    ZCL_DATA_TYPE_UINT32,
                                                    &*u32_data);
        } else {
          status = SL_STATUS_NOT_FOUND;
        }
      } else {
        status = SL_STATUS_INVALID_TYPE;
        sl_log_warning(LOG_TAG,
                       "Unable to parse field format '%s': 0x%X",
                       command_fields[i].format,
                       status);
      }
    }
  }
  return status;
}
