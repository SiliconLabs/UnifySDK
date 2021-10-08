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
// This component
#include "ota.hpp"
#include "ota_time.hpp"
#include "ota_cache.hpp"
#include "ota_mqtt.hpp"
#include "ota_download.hpp"

// UIC Components
#include "uic_mqtt.h"
#include "sl_log.h"
#include "sys/ctimer.h"

// Standard library
#include <map>
#include <typeinfo>

// Third party components
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <filesystem>

// Constants
constexpr char LOG_TAG[] = "uic_ota";

namespace bpt = boost::property_tree;

namespace uic_ota
{
// Download timeout
static uint32_t image_received_timeout = 0;

// MQTT base topics
constexpr std::string_view ota_info_base_topic    = "ucl/OTA/info";
constexpr std::string_view ota_data_base_topic    = "ucl/OTA/data";
constexpr std::string_view ota_by_unid_base_topic = "ucl/by-unid";

// Cluster revision
static uint32_t configured_cluster_revision = DEFAULT_CLUSTER_REVISION;

////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////
sl_status_t init(const image_available_func_t &image_available_cb,
                 const std::string &image_base_path,
                 unsigned long int cache_size,
                 uint32_t timeout_s,
                 uint16_t cluster_revision)
{
  ota_download::set_image_available_callback(image_available_cb);
  image_received_timeout      = CLOCK_SECOND * timeout_s;
  configured_cluster_revision = cluster_revision;

  sl_log_info(LOG_TAG,
              "Using directory %s to store OTA cached images.",
              image_base_path.c_str());
  if (!std::filesystem::exists(image_base_path)) {
    try {
      std::filesystem::create_directories(image_base_path);
    } catch (std::filesystem::filesystem_error &e) {
      sl_log_error(LOG_TAG,
                   "OTA image cache directory %s does not exist and could not "
                   "be created automatically.",
                   e.what());
      return SL_STATUS_FAIL;
    }
  }

  ota_download::set_image_base_path(image_base_path);

  return SL_STATUS_OK;
}

void subscribe_unid(const std::string &unid, const std::string &uiid)
{
  if (!ota_download::available_info_callback_initialized()) {
    sl_log_error(LOG_TAG,
                 "No callback registered for info_callback. Please initialize "
                 "OTA properly with an info callback function. Aborting OTA "
                 "subscription for UNID %s and UIID %s.",
                 unid.c_str(),
                 uiid.c_str());
    return;
  }

  std::string ota_info_topic_unid
    = std::string(ota_info_base_topic) + "/" + uiid + "/" + unid;
  uic_mqtt_subscribe(ota_info_topic_unid.c_str(), ota_download::info_callback);

  if (unid_state_handler::get_unids_listening_count(uiid) == 0) {
    std::string ota_info_topic_all
      = std::string(ota_info_base_topic) + "/" + uiid + "/all";
    uic_mqtt_subscribe(ota_info_topic_all.c_str(), ota_download::info_callback);
  }
  unid_state_handler::set_unid_listening(uiid, unid);
}

void unsubscribe_unid(const std::string &unid, const std::string &uiid)
{
  std::string ota_info_topic_unid
    = std::string(ota_info_base_topic) + "/" + uiid + "/" + unid;
  uic_mqtt_unsubscribe(ota_info_topic_unid.c_str(),
                       ota_download::info_callback);

  unid_state_handler::remove_unid_listening(uiid, unid);

  if (unid_state_handler::get_unids_listening_count(uiid) == 0) {
    std::string ota_info_topic_all
      = std::string(ota_info_base_topic) + "/" + uiid + "/all";
    uic_mqtt_unsubscribe(ota_info_topic_all.c_str(),
                         ota_download::info_callback);
  }
}

void unsubscribe_all_unids_uiid(const std::string &unid)
{
  std::vector<std::string> uiids
    = unid_state_handler::get_uiids_unid_is_listening_to(unid);

  for (const auto &uiid: uiids) {
    sl_log_debug(LOG_TAG,
                 "Unsubscribing unid: %s and uiid %s",
                 unid.c_str(),
                 uiid.c_str());
    unsubscribe_unid(unid, uiid);
  }
  unretain_ota_status_by_unid(unid.c_str());
}

void get_by_unid(const std::string &unid,
                 const std::string &uiid,
                 const image_ready_funct_t &image_ready_cb)
{
  std::string image_key = uiid + "/" + unid;
  std::string cache_key = image_key;
  std::string ota_data_subscribe_topic;
  std::string ota_data_publish_topic;
  // Check specific unid info received
  // Checking if meta info is received for specific UIID and UNID

  if (!image_file_handler::check_if_meta_info_received(cache_key)) {
    cache_key = uiid;
    // Checking if meta info is received on all for UIID.
    if (!image_file_handler::check_if_meta_info_received(cache_key)) {
      sl_log_warning(LOG_TAG,
                     "No meta info received on for UIID: %s UNID: %s",
                     unid.c_str(),
                     uiid.c_str());
      image_ready_cb(image_ready_result_t::ERROR, "");
      return;
    }
    ota_data_subscribe_topic
      = std::string(ota_data_base_topic) + "/" + cache_key + "/all";
    ota_data_publish_topic = ota_data_subscribe_topic + "/get";
  } else {
    ota_data_subscribe_topic
      = std::string(ota_data_base_topic) + "/" + cache_key;
    ota_data_publish_topic = ota_data_subscribe_topic + "/get";
  }

  if (image_file_handler::is_image_received_and_cached(cache_key)) {
    std::string filepath
      = image_file_handler::get_image_cached_filepath(image_key);
    image_ready_cb(image_ready_result_t::OK, filepath);
    return;
  }

  ota_download::get(unid,
                    ota_data_subscribe_topic,
                    ota_data_publish_topic,
                    image_ready_cb,
                    image_received_timeout);
}

void update_status(const dotdot_unid_t &unid,
                   const dotdot_endpoint_id_t &endpoint,
                   const ota_uiid_t &uiid,
                   const status_t status)
{
  const auto &status_string = convert_status_to_label(status);
  publish_status_string(unid,
                        endpoint,
                        uiid,
                        status_string,
                        "Status",
                        configured_cluster_revision);
}

void update_size(const dotdot_unid_t &unid,
                 const dotdot_endpoint_id_t &endpoint,
                 const ota_uiid_t &uiid,
                 const size_t size_file)
{
  publish_status_unsigned_long(unid,
                               endpoint,
                               uiid,
                               size_file,
                               "Size",
                               configured_cluster_revision);
}

void update_offset(const dotdot_unid_t &unid,
                   const dotdot_endpoint_id_t &endpoint,
                   const ota_uiid_t &uiid,
                   const size_t offset)
{
  publish_status_unsigned_long(unid,
                               endpoint,
                               uiid,
                               offset,
                               "Offset",
                               configured_cluster_revision);
}

void update_last_error(const dotdot_unid_t &unid,
                       const dotdot_endpoint_id_t &endpoint,
                       const ota_uiid_t &uiid,
                       const last_error_t last_error)
{
  const auto &last_error_string = convert_last_error_to_label(last_error);
  publish_status_string(unid,
                        endpoint,
                        uiid,
                        last_error_string,
                        "LastError",
                        configured_cluster_revision);
}

void update_apply_after(const dotdot_unid_t &unid,
                        const dotdot_endpoint_id_t &endpoint,
                        const ota_uiid_t &uiid,
                        const time_t apply_after)
{
  std::string time_string = get_utc_time_string_from_utc_time(apply_after);
  publish_status_string(unid,
                        endpoint,
                        uiid,
                        time_string,
                        "ApplyAfter",
                        configured_cluster_revision);
}

void update_current_version(const dotdot_unid_t &unid,
                            const dotdot_endpoint_id_t &endpoint,
                            const ota_uiid_t &uiid,
                            const std::string &current_version)
{
  publish_status_string(unid,
                        endpoint,
                        uiid,
                        current_version,
                        "CurrentVersion",
                        configured_cluster_revision);
}

void update_target_version(const dotdot_unid_t &unid,
                           const dotdot_endpoint_id_t &endpoint,
                           const ota_uiid_t &uiid,
                           const std::string &target_version)
{
  publish_status_string(unid,
                        endpoint,
                        uiid,
                        target_version,
                        "TargetVersion",
                        configured_cluster_revision);
}

void clear_cache()
{
  image_file_handler::clear_images_cache();
  sl_log_info(LOG_TAG, "Cleared out OTA image cache.");
}

void unretain_ota_status()
{
  std::map<std::string, std::vector<endpoint_cache_t>, std::less<>>
    status_map_to_unretain
    = unid_state_handler::get_map_status_published_cached_and_clear();

  for (auto const &[unid, endpoints]: status_map_to_unretain) {
    for (auto const &endpoint: endpoints) {
      std::string base_topic = std::string(ota_by_unid_base_topic) + "/" + unid
                               + "/ep" + std::to_string(endpoint) + "/OTA";
      uic_mqtt_unretain(base_topic.c_str());
    }
  }
}

void unretain_ota_status_by_unid(const dotdot_unid_t &unid)
{
  std::vector<endpoint_cache_t> endpoints
    = unid_state_handler::get_endpoints_status_published_and_pop(unid);
  for (auto const &endpoint: endpoints) {
    std::string base_topic = std::string(ota_by_unid_base_topic) + "/"
                             + std::string(unid) + "/ep"
                             + std::to_string(endpoint) + "/OTA";
    uic_mqtt_unretain(base_topic.c_str());
  }
}

}  // namespace uic_ota