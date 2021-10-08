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
// Standard library
#include <string>
#include <array>

// This component
#include "ota.hpp"
#include "ota_mqtt.hpp"
#include "ota_cache.hpp"
#include "uic_mqtt.h"

// Third party
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace bpt = boost::property_tree;

// Base topics of relevant MQTT topics
constexpr std::string_view ota_by_unid_base_topic = "ucl/by-unid";

// OTA Enum conversion variables
static const std::array<std::string, 5> status_conversion
  = {"Idle",
     "DownloadInProgress",
     "WaitingToUpgrade",
     "WaitingToUpgradeViaExternalEvent",
     "WaitForMore"};

static const std::array<std::string, 6> last_error_conversion
  = {"Success",
     "Abort",
     "NotAuthorized",
     "InvalidImage",
     "RequireMoreImage",
     "NotSupported"};

////////////////////////////////////////////////////////////////////////////////
// Public OTA Enum conversion functions
////////////////////////////////////////////////////////////////////////////////
std::string convert_last_error_to_label(uic_ota::last_error_t last_error)
{
  return last_error_conversion[static_cast<int>(last_error)];
}

std::string convert_status_to_label(uic_ota::status_t status)
{
  return status_conversion[static_cast<int>(status)];
}

////////////////////////////////////////////////////////////////////////////////
// Publish functionality
////////////////////////////////////////////////////////////////////////////////

static void publish_supported_commands(const dotdot_unid_t &unid,
                                       dotdot_endpoint_id_t &ep)
{
  std::string payload = R"({"value": []})";
  std::string topic   = std::string(ota_by_unid_base_topic) + "/"
                      + std::string(unid) + "/ep" + std::to_string(ep)
                      + "/OTA/SupportedCommands";
  bool retain = true;
  uic_mqtt_publish(topic.c_str(), payload.c_str(), payload.length(), retain);
}

static void publish_desired_and_reported(const std::string &topic,
                                         const std::string &payload)
{
  std::string topic_desired  = topic + "/Desired";
  std::string topic_reported = topic + "/Reported";
  bool retain                = true;
  uic_mqtt_publish(topic_desired.c_str(),
                   payload.c_str(),
                   payload.length(),
                   retain);
  uic_mqtt_publish(topic_reported.c_str(),
                   payload.c_str(),
                   payload.length(),
                   retain);
}

static void publish_cluster_revision(const dotdot_unid_t &unid,
                                     const dotdot_endpoint_id_t &endpoint_id,
                                     const uint32_t cluster_revision)
{
  std::string base_topic = "ucl/by-unid/" + std::string(unid) + "/ep"
                           + std::to_string(endpoint_id)
                           + "/OTA/Attributes/ClusterRevision";
  std::string payload = std::string(R"({"value": )")
                        + std::to_string(cluster_revision) + std::string("}");

  publish_desired_and_reported(base_topic, payload);
}

template<typename value> static std::string format_status_payload(value val)
{
  bpt::ptree doc;

  // Needed for removing qotation marks "" around an integer on payload
  bool atomic_type = typeid(value) == typeid(uint32_t)
                     || typeid(value) == typeid(unsigned long);

  if (atomic_type) {
    doc.add("value", ":PlaceHolder");
  } else {
    doc.add("value", val);
  }
  std::stringstream ss;
  bpt::write_json(ss, doc);
  std::string payload_str(ss.str());

  if (atomic_type) {
    std::stringstream number;
    number << val;
    std::string number_string(number.str());
    boost::replace_all(payload_str, "\":PlaceHolder\"", number_string);
  }

  return payload_str;
}

static std::string
  construct_status_update_topic(const dotdot_unid_t &unid,
                                const dotdot_endpoint_id_t &endpoint,
                                const ota_uiid_t &uiid,
                                const std::string &status_type)
{
  return std::string(ota_by_unid_base_topic) + "/" + std::string(unid) + "/ep"
         + std::to_string(endpoint) + "/OTA/Attributes/UIID/"
         + std::string(uiid) + "/" + status_type;
}

template<typename value>
static void publish_status(const dotdot_unid_t &unid,
                           const dotdot_endpoint_id_t &endpoint,
                           const ota_uiid_t &uiid,
                           const value &val,
                           const std::string &status,
                           uint32_t cluster_revision)
{
  std::string topic
    = construct_status_update_topic(unid, endpoint, uiid, status);
  std::string payload = format_status_payload(val);
  publish_desired_and_reported(topic, payload);

  if (unid_state_handler::is_status_published(unid, endpoint) == false) {
    publish_supported_commands(unid, endpoint);
    publish_cluster_revision(unid, endpoint, cluster_revision);
    unid_state_handler::set_status_published_cached(unid, endpoint);
  }
};

////////////////////////////////////////////////////////////////////////////////
// Publish general functions
////////////////////////////////////////////////////////////////////////////////

void publish_status_string(const dotdot_unid_t &unid,
                           const dotdot_endpoint_id_t &endpoint,
                           const ota_uiid_t &uiid,
                           const std::string &val,
                           const std::string &status,
                           const uint32_t cluster_revision)
{
  publish_status(unid, endpoint, uiid, val, status, cluster_revision);
}

void publish_status_unsigned_long(const dotdot_unid_t &unid,
                                  const dotdot_endpoint_id_t &endpoint,
                                  const ota_uiid_t &uiid,
                                  const unsigned long val,
                                  const std::string &status,
                                  const uint32_t cluster_revision)
{
  publish_status(unid, endpoint, uiid, val, status, cluster_revision);
}