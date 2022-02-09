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

#include <string>
#include <iomanip>
#include <sstream>
#include <nlohmann/json.hpp>

// UIC libraries
#include <sl_log.h>
#include <sl_status.h>
#include <uic_mqtt.h>

// ZigPC includes
#include <zigpc_common_unid.h>

// Component includes
#include "zigpc_ucl_int.hpp"

#define SS_FORMAT_HEX(width) \
  std::hex << std::uppercase << std::setw(width) << std::setfill('0')

namespace topic_part
{
constexpr std::string_view UCL        = "ucl/";
constexpr std::string_view BY_UNID    = "by-unid/";
constexpr std::string_view BY_EP      = "/ep";
constexpr std::string_view NODE_STATE = "/State";
constexpr std::string_view PC_NWMGMT  = "/ProtocolController/NetworkManagement";
}  // namespace topic_part

static const std::string UNID_PREFIX = ZIGPC_UNID_PREFIX;

std::string zigpc_ucl::mqtt::build_unid(zigbee_eui64_uint_t eui64)
{
  std::stringstream eui64_ss;

  eui64_ss << UNID_PREFIX << SS_FORMAT_HEX(16) << eui64;

  return eui64_ss.str();
}

sl_status_t zigpc_ucl::mqtt::parse_unid(std::string unid,
                                        zigbee_eui64_uint_t &eui64)
{
  sl_status_t status = SL_STATUS_OK;
  // Check if prefix is first
  if (unid.size() < UNID_PREFIX.size()) {
    status = SL_STATUS_INVALID_SIGNATURE;
  } else if (UNID_PREFIX == unid.substr(0, UNID_PREFIX.size() + 1)) {
    status = SL_STATUS_INVALID_SIGNATURE;
  } else {
    unid = unid.substr(UNID_PREFIX.size());
    if (unid.size() < (ZIGBEE_EUI64_SIZE * 2)) {
      status = SL_STATUS_INVALID_SIGNATURE;
    }
  }

  // Check if buffer only has hexadecimal characters
  if (status == SL_STATUS_OK) {
    for (char c: unid) {
      if (!std::isxdigit(c)) {
        status = SL_STATUS_INVALID_SIGNATURE;
        break;
      }
    }
  }

  if (status == SL_STATUS_OK) {
    std::stringstream unid_ss(unid);

    unid_ss >> std::hex >> eui64;
    if (unid_ss.fail()) {
      status = SL_STATUS_INVALID_SIGNATURE;
      eui64  = 0U;
    }
  }

  return status;
}

sl_status_t
  zigpc_ucl::mqtt::build_topic(zigpc_ucl::mqtt::topic_type_t topic_type,
                               zigpc_ucl::mqtt::topic_data_t topic_data,
                               std::string &topic)
{
  sl_status_t status = SL_STATUS_OK;

  std::stringstream topic_ss;
  topic_ss << topic_part::UCL;

  std::string unid = zigpc_ucl::mqtt::build_unid(topic_data.eui64);
  switch (topic_type) {
    case topic_type_t::BY_UNID_PC_NWMGMT:
      topic_ss << topic_part::BY_UNID << unid << topic_part::PC_NWMGMT;
      break;
    case topic_type_t::BY_UNID_PC_NWMGMT_WRITE:
      topic_ss << topic_part::BY_UNID << unid << topic_part::PC_NWMGMT
               << "/Write";
      break;
    case topic_type_t::BY_UNID_NODE:
      topic_ss << topic_part::BY_UNID << unid;
      break;
    case topic_type_t::BY_UNID_NODE_EP:
      topic_ss << topic_part::BY_UNID << unid << topic_part::BY_EP << std::dec
               << static_cast<int>(topic_data.endpoint_id);
      break;
    case topic_type_t::BY_UNID_NODE_STATE:
      topic_ss << topic_part::BY_UNID << unid << topic_part::NODE_STATE;
      break;
    default:
      status = SL_STATUS_INVALID_TYPE;
      sl_log_warning(zigpc_ucl::LOG_TAG,
                     "Unknown topic to subscribe (%u): 0x%X",
                     topic_type,
                     status);
      break;
  }

  if (status == SL_STATUS_OK) {
    topic = topic_ss.str();
  }

  return status;
}

sl_status_t zigpc_ucl::mqtt::subscribe(zigpc_ucl::mqtt::topic_type_t topic_type,
                                       zigpc_ucl::mqtt::topic_data_t topic_data,
                                       mqtt_message_callback_t cb)
{
  sl_status_t status = SL_STATUS_OK;

  if (cb == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    std::string topic;

    status = zigpc_ucl::mqtt::build_topic(topic_type, topic_data, topic);
    if (status == SL_STATUS_OK) {
      uic_mqtt_subscribe(topic.c_str(), cb);
    }
  }

  return status;
}

sl_status_t zigpc_ucl::mqtt::publish(zigpc_ucl::mqtt::topic_type_t topic_type,
                                     zigpc_ucl::mqtt::topic_data_t topic_data,
                                     const char *payload,
                                     size_t payload_size,
                                     bool retain)
{
  std::string topic;

  sl_status_t status
    = zigpc_ucl::mqtt::build_topic(topic_type, topic_data, topic);
  if (status == SL_STATUS_OK) {
    uic_mqtt_publish(topic.c_str(), payload, payload_size, retain);
  }

  return status;
}

sl_status_t zigpc_ucl::mqtt::unretain(zigpc_ucl::mqtt::topic_type_t topic_type,
                                      zigpc_ucl::mqtt::topic_data_t topic_data)
{
  std::string topic;

  sl_status_t status
    = zigpc_ucl::mqtt::build_topic(topic_type, topic_data, topic);
  if (status == SL_STATUS_OK) {
    uic_mqtt_unretain(topic.c_str());
  }

  return status;
}

sl_status_t zigpc_ucl::mqtt::parse_payload(const char *payload, nlohmann::json &jsn)
{
  sl_status_t status = SL_STATUS_OK;

  if (payload == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else if (std::char_traits<char>::length(payload) == 0) {
    status = SL_STATUS_EMPTY;
  }

  if (status == SL_STATUS_OK) {
    try {
      jsn = nlohmann::json::parse(payload);
    } catch (const nlohmann::json::parse_error& err) {
      sl_log_warning(zigpc_ucl::LOG_TAG,
                     "Failed to parse payload: %s",
                     err.what());
      status = SL_STATUS_OBJECT_READ;
    }
  }

  return status;
}
