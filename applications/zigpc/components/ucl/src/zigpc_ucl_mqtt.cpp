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
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

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

static constexpr char TOPIC_UCL[]     = "ucl/";
static constexpr char TOPIC_BY_UNID[] = "by-unid/";
static constexpr char TOPIC_PC_NWMGMT[]
  = "/ProtocolController/NetworkManagement";

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
  topic_ss << TOPIC_UCL;

  std::string unid = zigpc_ucl::mqtt::build_unid(topic_data.eui64);
  switch (topic_type) {
    case topic_type_t::BY_UNID_PC_NWMGMT:
      topic_ss << TOPIC_BY_UNID << unid << TOPIC_PC_NWMGMT;
      break;
    case topic_type_t::BY_UNID_PC_NWMGMT_WRITE:
      topic_ss << TOPIC_BY_UNID << unid << TOPIC_PC_NWMGMT << "/Write";
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

sl_status_t zigpc_ucl::mqtt::parse_payload(const char *payload, bpt::ptree &pt)
{
  sl_status_t status = SL_STATUS_OK;
  std::stringstream payload_ss;

  if (payload == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    std::string payload_str(payload);

    if (payload_str.empty()) {
      status = SL_STATUS_EMPTY;
    } else {
      payload_ss << payload_str;
    }
  }

  if (status == SL_STATUS_OK) {
    try {
      bpt::json_parser::read_json(payload_ss, pt);
    } catch (const bpt::json_parser_error &err) {
      sl_log_warning(zigpc_ucl::LOG_TAG,
                     "Failed to parse payload: %s",
                     err.what());
      pt.clear();
      status = SL_STATUS_OBJECT_READ;
    }
  }

  return status;
}
