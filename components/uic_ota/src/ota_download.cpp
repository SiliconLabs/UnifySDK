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
#include <filesystem>
#include <fstream>

// This component
#include "ota.hpp"
#include "ota_download.hpp"
#include "ota_time.hpp"
#include "ota_cache.hpp"

// Unify components
#include "sl_log.h"
#include "sys/ctimer.h"
#include "uic_mqtt.h"

// Third party library
#include <boost/algorithm/string.hpp>
#include <nlohmann/json.hpp>

// Constants
constexpr char LOG_TAG[] = "uic_ota_download";

// General image info available callback
static uic_ota::image_available_func_t image_available_function_callback
  = nullptr;

// Base path to save images at.
static std::string base_path_images;

// Cache size
constexpr unsigned long int max_image_cache_size = 256 * 1024 * 1024;
static unsigned long int image_cache_size        = max_image_cache_size;

////////////////////////////////////////////////////////////////////////////////
// Internal functions
////////////////////////////////////////////////////////////////////////////////
sl_status_t is_image_valid_size(const size_t message_length,
                                const unsigned long desired_cache_size)
{
  if ((message_length > desired_cache_size) || (message_length == 0)) {
    return false;
  }
  return true;
}

static sl_status_t image_write_file(const std::string &filepath,
                                    const char *data,
                                    const size_t data_length)
{
  try {
    std::ofstream image_file(filepath.c_str(),
                             std::ios::out | std::ios::binary);

    if (!image_file.is_open()) {
      sl_log_error(LOG_TAG,
                   "Failed to save the OTA image data. Could not open file %s. "
                   "Aborting OTA operations for this image.",
                   filepath.c_str());
      return SL_STATUS_ABORT;
    }
    image_file.write(data, data_length);

    image_file.close();

    return SL_STATUS_OK;
  } catch (std::exception const &ex) {
    sl_log_error(LOG_TAG,
                 "Unable to write image file %s. Error is: %s",
                 filepath.c_str(),
                 ex.what());
    return SL_STATUS_ABORT;
  } catch (...) {
    sl_log_error(LOG_TAG,
                 "Unable to write image file %s. Unknown error",
                 filepath.c_str());
    return SL_STATUS_ABORT;
  }
}

static std::vector<std::string> split_topic(const char *topic)
{
  std::string topic_string(topic);
  std::vector<std::string> topic_elements;

  boost::algorithm::split(topic_elements, topic_string, boost::is_any_of("/"));

  return topic_elements;
}

static std::string get_unid_by_topic(const char *topic)
{
  // Assuming the topic subscribed to will always be of structure
  // ucl/OTA/+/<UIID>/<UNID> this returns all if it is non specified UNID
  std::vector<std::string> topic_elements = split_topic(topic);

  if (topic_elements.size() < 5 || topic_elements[4] == "all") {
    return "";
  }

  return topic_elements[4];
}

static std::string get_uiid_by_topic(const char *topic)
{
  std::vector<std::string> topic_elements = split_topic(topic);

  if (topic_elements.size() < 4) {
    sl_log_warning(LOG_TAG,
                   "Could not parse the UIID from the MQTT topic: %s",
                   topic);
    return "";
  }

  return topic_elements[3];
}

static time_t parse_time_from_meta_info(nlohmann::json &jsn)
{
  std::string time_str;
  try {
    time_str = jsn["ApplyAfter"].get<std::string>();
  } catch (...) {
    return 0;
  }
  return calculate_utc_time_from_rfc_3339(time_str);
}

static std::string parse_image_key(const std::string &unid,
                                   const std::string &uiid)
{
  bool unid_in_topic = !unid.empty();
  return unid_in_topic ? uiid + "/" + unid : uiid;
}

static std::string parse_filename_json(nlohmann::json &jsn,
                                       const std::string &unid,
                                       const std::string &uiid)
{
  try {
    return jsn["Filename"].get<std::string>();
  } catch (...) {
    return "Firmware_image_" + unid + "_" + uiid;
  }
}

static std::string get_image_filepath(const std::string &unid,
                                      const std::string &uiid)
{
  if (unid != "") {
    return base_path_images + "/" + uiid + "_" + unid + ".dat";
  }
  return base_path_images + "/" + uiid + ".dat";
}

static std::vector<std::string> get_unids_to_activate(const std::string &unid,
                                                      const std::string &uiid)
{
  std::vector<std::string> list_unids = {};
  if (unid == "") {
    auto tmp_list_unids = unid_state_handler::get_unids_listening(uiid);

    // Emplace all unids where there exists no meta information on specific uiid/unid
    for (const auto &unid_listening: tmp_list_unids) {
      if (!image_file_handler::check_if_meta_info_received(uiid + "/"
                                                           + unid_listening)
          && unid_state_handler::is_unid_in_poll(unid_listening)) {
        list_unids.emplace_back(unid_listening);
      }
    }
  } else {
    list_unids.push_back(unid);
  }
  return list_unids;
}

static void image_ready_callback(const char *topic,
                                 const char *message,
                                 const size_t message_length)
{
  std::string unid = get_unid_by_topic(topic);
  std::string uiid = get_uiid_by_topic(topic);
  if (uiid == "") {
    return;
  }

  std::string image_key               = parse_image_key(unid, uiid);
  std::vector<std::string> list_unids = get_unids_to_activate(unid, uiid);

  for (const auto &unid_to_update: list_unids) {
    // Check if unid has been given an update.
    if (unid_state_handler::get_unid_poll_state(unid_to_update)) {
      continue;
    }
    unid_state_handler::set_unid_poll_state_true(unid_to_update);

    // Validate image
    std::string filepath = get_image_filepath(unid, uiid);
    if (is_image_valid_size(message_length, image_cache_size)
        && (SL_STATUS_OK
            == image_write_file(filepath, message, message_length))) {
      image_file_handler::set_images_received_cached_filepath(image_key,
                                                              filepath);

      // Now the protocol controller can take over the firmware upgrade
      unid_state_handler::get_unid_callback(
        unid_to_update)(uic_ota::image_ready_result_t::OK, filepath);

    } else {
      unid_state_handler::get_unid_callback(
        unid_to_update)(uic_ota::image_ready_result_t::ERROR, "");
      sl_log_warning(LOG_TAG,
                     "Invalid image provided for UNID %s, UIID %s",
                     unid_to_update.c_str(),
                     uiid.c_str());
    }
  }
}

static void image_download_timeout(void *data)
{
  const auto image_data        = static_cast<TimeoutHandler *>(data);
  std::string subscribed_topic = std::string(image_data->get_subscribe_topic());
  std::string unid             = image_data->get_unid();

  // Unsubscribe
  uic_mqtt_unsubscribe(subscribed_topic.c_str(), image_ready_callback);

  if (unid_state_handler::get_unid_poll_state(unid) == true) {
    unid_state_handler::pop_unid_poll(unid);
    return;
  }

  unid_state_handler::get_unid_callback(
    unid)(uic_ota::image_ready_result_t::TIMED_OUT, "");
  unid_state_handler::pop_unid_poll(unid);
  sl_log_warning(
    LOG_TAG,
    "Image download timed out after waiting for unid %s on topic %s",
    unid.c_str(),
    subscribed_topic.c_str());
  return;
}

////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////
namespace ota_download
{
void set_image_cache_size(unsigned long int cache_size)
{
  if (cache_size <= max_image_cache_size) {
    image_cache_size = cache_size;
  } else {
    image_cache_size = max_image_cache_size;
    sl_log_warning(
      LOG_TAG,
      "Configured cache size exceeds maximum limit. Setting to 256 MB");
  }
}

void set_image_base_path(const std::string &image_base_path)
{
  base_path_images = image_base_path;
}

void set_image_available_callback(
  const uic_ota::image_available_func_t &image_available_cb)
{
  image_available_function_callback = image_available_cb;
}

bool available_info_callback_initialized()
{
  return image_available_function_callback != nullptr;
}

void info_callback(const char *topic,
                   const char *message,
                   const size_t message_length)
{
  uic_ota::meta_t meta_info;

  // Extract unid and uiid
  meta_info.unid = get_unid_by_topic(topic);
  meta_info.uiid = get_uiid_by_topic(topic);
  if (meta_info.uiid == "") {
    return;
  }

  // Image key
  bool unid_in_topic = !meta_info.unid.empty();
  std::string image_key
    = unid_in_topic ? meta_info.uiid + "/" + meta_info.unid : meta_info.uiid;

  if (message_length == 0) {
    sl_log_info(LOG_TAG,
                "Removing meta information for UIID: %s (UNID: %s)",
                meta_info.uiid.c_str(),
                meta_info.unid.c_str());
    image_file_handler::remove_image_meta_info(image_key);
    return;
  }

  // Parse payload data
  nlohmann::json jsn;
  try {
    jsn = nlohmann::json::parse(std::string(message, message_length));
  } catch (const nlohmann::json::parse_error& e) {
    sl_log_error(LOG_TAG, "Unable to parse JSON payload on topic '%s', error: '%s'.", topic, e.what());
    return;
  }

  try {
    meta_info.apply_after = parse_time_from_meta_info(jsn);
    meta_info.version     = jsn["Version"].get<std::string>();
    meta_info.filename    = parse_filename_json(jsn, meta_info.unid, meta_info.uiid);
  } catch (...) {
    sl_log_error(LOG_TAG,
                 "Error parsing OTA meta data JSON payload %s - unknown error",
                 message);
    return;
  }

  image_file_handler::set_image_meta_info(image_key, meta_info);

  if (meta_info.unid.empty()) {
    std::vector<std::string> unids
      = unid_state_handler::get_unids_listening(meta_info.uiid);

    for (const auto &unid: unids) {
      meta_info.unid = unid;
      image_available_function_callback(meta_info);
    }

    return;
  }

  image_available_function_callback(meta_info);
  return;
}

void get(const std::string &unid,
         const std::string &subscribe_topic,
         const std::string &publish_topic,
         const uic_ota::image_ready_funct_t &image_ready_cb,
         uint32_t timeout_s)
{
  // Subscribe
  if (!unid_state_handler::is_poll_topic_subscribed(subscribe_topic)) {
    uic_mqtt_subscribe(subscribe_topic.c_str(), image_ready_callback);
  }

  // Ask image provider to provide image
  std::string empty_payload = "{}";
  bool retain               = false;
  uic_mqtt_publish(publish_topic.c_str(),
                   empty_payload.c_str(),
                   empty_payload.length(),
                   retain);

  // Poll on download
  timeout_handler_t timeout_data
    = {unid, subscribe_topic, false, image_ready_cb};
  auto timeout_handler = std::make_shared<TimeoutHandler>(timeout_data);
  unid_state_handler::set_unid_poll(timeout_handler);

  ctimer_set(timeout_handler.get()->get_ctimer_pointer(),
             timeout_s,
             image_download_timeout,
             timeout_handler.get());
}
}  // namespace ota_download