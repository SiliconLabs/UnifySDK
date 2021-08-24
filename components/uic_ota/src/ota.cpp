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
#include "ota_image_validation.hpp"

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
static image_available_func_t image_available_function_callback = nullptr;
static std::string base_path_images;
static uint32_t image_received_timeout            = 0;
constexpr unsigned long int max_image_cache_size  = 256 * 1024 * 1024;
constexpr std::string_view ota_info_base_topic    = "ucl/OTA/info";
constexpr std::string_view ota_data_base_topic    = "ucl/OTA/data";
constexpr std::string_view ota_by_unid_base_topic = "ucl/by-unid";
static unsigned long int image_cache_size         = max_image_cache_size;
static uint32_t configured_cluster_revision       = DEFAULT_CLUSTER_REVISION;

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
// Internal Function
////////////////////////////////////////////////////////////////////////////////
static sl_status_t image_write_file(const std::string &filepath,
                                    const char *data,
                                    const size_t data_length);
static void image_download_timeout(void *data);
static std::string
  construct_status_update_topic(const dotdot_unid_t &unid,
                                const dotdot_endpoint_id_t &endpoint,
                                const ota_uiid_t &uiid,
                                const std::string &status_type);
static std::vector<std::string> split_topic(const char *topic);
static std::string get_unid_by_topic(const char *topic);
static time_t parse_time_from_meta_info(const bpt::ptree &pt);
static std::string parse_image_key(const std::string &unid,
                                   const std::string &uiid);
static void
  parse_json(const char *message, const size_t message_length, bpt::ptree &pt);
static std::string get_image_filepath(const std::string &unid,
                                      const std::string &uiid);
static void info_callback(const char *topic,
                          const char *message,
                          const size_t message_length);
static void image_ready_callback(const char *topic,
                                 const char *message,
                                 const size_t message_length);
template<typename value> static std::string format_status_payload(value val);
static void publish_desired_and_reported(const std::string &topic,
                                         const std::string &payload);
template<class value>
static void publish_status(const dotdot_unid_t &unid,
                           const dotdot_endpoint_id_t &endpoint,
                           const ota_uiid_t &uiid,
                           value val,
                           const std::string &status);
static void get(const std::string &image_key,
                const std::string &subscribe_topic,
                const std::string &publish_topic,
                const image_ready_funct_t &image_ready_cb);

////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////
sl_status_t init(const image_available_func_t &image_available_cb,
                 const std::string &image_base_path,
                 unsigned long int cache_size,
                 uint32_t timeout_s,
                 uint16_t cluster_revision)
{
  base_path_images                  = image_base_path;
  image_available_function_callback = image_available_cb;
  image_received_timeout            = CLOCK_SECOND * timeout_s;
  configured_cluster_revision       = cluster_revision;

  sl_log_info(LOG_TAG, "OTA image cache directory %s", image_base_path.c_str());
  if (!std::filesystem::exists(image_base_path)) {
    try {
      std::filesystem::create_directories(image_base_path);
    } catch (std::filesystem::filesystem_error &e) {
      sl_log_error(LOG_TAG,
                   "Unable to create OTA image cache directory %s",
                   e.what());
      return SL_STATUS_FAIL;
    }
  }

  if (cache_size <= max_image_cache_size) {
    image_cache_size = cache_size;
  } else {
    image_cache_size = max_image_cache_size;
    sl_log_warning(LOG_TAG,
                   "Cache size exceeds maximum limit setting to 256 MB");
  }
  return SL_STATUS_OK;
}

void subscribe_unid(const std::string &unid, const std::string &uiid)
{
  std::string ota_info_topic_unid
    = std::string(ota_info_base_topic) + "/" + uiid + "/" + unid;
  std::string ota_info_topic_all
    = std::string(ota_info_base_topic) + "/" + uiid + "/all";
  uic_mqtt_subscribe(ota_info_topic_unid.c_str(), info_callback);

  if (get_unids_listening_count(uiid) == 0) {
    uic_mqtt_subscribe(ota_info_topic_all.c_str(), info_callback);
  }
  set_unid_listening(uiid, unid);
}

void unsubscribe_unid(const std::string &unid, const std::string &uiid)
{
  std::string ota_info_topic_unid
    = std::string(ota_info_base_topic) + "/" + uiid + "/" + unid;
  std::string ota_info_topic_all
    = std::string(ota_info_base_topic) + "/" + uiid + "/all";
  uic_mqtt_unsubscribe(ota_info_topic_unid.c_str(), info_callback);

  remove_unid_listening(uiid, unid);

  if (get_unids_listening_count(uiid) == 0) {
    uic_mqtt_unsubscribe(ota_info_topic_all.c_str(), info_callback);
  }
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
  bool meta_info_received = check_if_meta_info_received(cache_key);
  if (!meta_info_received) {
    cache_key          = uiid;
    meta_info_received = check_if_meta_info_received(
      cache_key);  // Checking if meta info is received on all for UIID.

    if (!meta_info_received) {
      sl_log_warning(LOG_TAG,
                     "No meta info received on unid: %s",
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

  if (is_image_received_and_cached(cache_key)) {
    std::string filepath = get_image_cached_filepath(image_key);
    image_ready_cb(image_ready_result_t::OK, filepath);
    return;
  }

  if (ota_data_subscribe_topic.find("all") != std::string::npos) {
    // Save unid lisetening to uiid
    set_image_listening_unid_on_uiid(uiid, unid, image_ready_cb);
  }

  get(image_key,
      ota_data_subscribe_topic,
      ota_data_publish_topic,
      image_ready_cb);
}

void update_status(const dotdot_unid_t &unid,
                   const dotdot_endpoint_id_t &endpoint,
                   const ota_uiid_t &uiid,
                   const status_t status)
{
  const auto &status_string = status_conversion[static_cast<int>(status)];
  publish_status(unid, endpoint, uiid, status_string, "Status");
}

void update_size(const dotdot_unid_t &unid,
                 const dotdot_endpoint_id_t &endpoint,
                 const ota_uiid_t &uiid,
                 const size_t size_file)
{
  publish_status(unid, endpoint, uiid, size_file, "Size");
}

void update_offset(const dotdot_unid_t &unid,
                   const dotdot_endpoint_id_t &endpoint,
                   const ota_uiid_t &uiid,
                   const size_t offset)
{
  publish_status(unid, endpoint, uiid, offset, "Offset");
}

void update_last_error(const dotdot_unid_t &unid,
                       const dotdot_endpoint_id_t &endpoint,
                       const ota_uiid_t &uiid,
                       const last_error_t last_error)
{
  const auto &last_error_string
    = last_error_conversion[static_cast<int>(last_error)];
  publish_status(unid, endpoint, uiid, last_error_string, "LastError");
}

void update_apply_after(const dotdot_unid_t &unid,
                        const dotdot_endpoint_id_t &endpoint,
                        const ota_uiid_t &uiid,
                        const time_t apply_after)
{
  std::string time_string = get_utc_time_string_from_utc_time(apply_after);
  publish_status(unid, endpoint, uiid, time_string, "ApplyAfter");
}

void update_current_version(const dotdot_unid_t &unid,
                            const dotdot_endpoint_id_t &endpoint,
                            const ota_uiid_t &uiid,
                            const std::string &current_version)
{
  publish_status(unid, endpoint, uiid, current_version, "CurrentVersion");
}

void update_target_version(const dotdot_unid_t &unid,
                           const dotdot_endpoint_id_t &endpoint,
                           const ota_uiid_t &uiid,
                           const std::string &target_version)
{
  publish_status(unid, endpoint, uiid, target_version, "TargetVersion");
}

void clear_cache()
{
  clear_images_cache();
  sl_log_info(LOG_TAG, "Cleared out cache of images");
}

void unretain_ota_status()
{
  std::map<std::string, std::vector<endpoint_cache_t>, std::less<>>
    status_map_to_unretain = get_map_status_published_cached_and_clear();

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
    = get_endpoints_status_published_and_pop(unid);
  for (auto const &endpoint: endpoints) {
    std::string base_topic = std::string(ota_by_unid_base_topic) + "/"
                             + std::string(unid) + "/ep"
                             + std::to_string(endpoint) + "/OTA";
    uic_mqtt_unretain(base_topic.c_str());
  }
}

////////////////////////////////////////////////////////////////////////////////
// Internal functions definitions
////////////////////////////////////////////////////////////////////////////////
static sl_status_t image_write_file(const std::string &filepath,
                                    const char *data,
                                    const size_t data_length)
{
  try {
    std::ofstream image_file(filepath.c_str(),
                             std::ios::out | std::ios::binary);

    if (!image_file.is_open()) {
      sl_log_error(LOG_TAG, "Failed to open file");
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
    sl_log_warning(LOG_TAG, "Could not get uiid in topic");
    return "";
  }

  return topic_elements[3];
}

static time_t parse_time_from_meta_info(const bpt::ptree &pt)
{
  std::string time_str;
  try {
    time_str = pt.get<std::string>("ApplyAfter");
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

static void
  parse_json(const char *message, const size_t message_length, bpt::ptree &pt)
{
  std::stringstream ss;
  ss << std::string(message, message_length);
  bpt::json_parser::read_json(ss, pt);
}

static std::string parse_filename_json(const bpt::ptree &pt,
                                       const std::string &unid,
                                       std::string &uiid)
{
  try {
    return pt.get<std::string>("Filename");
  } catch (...) {
    return "Firmware_image_" + unid + "_" + uiid;
  }
}

static std::string get_image_filepath(const std::string &unid,
                                      const std::string &uiid)
{
  return base_path_images + "/" + uiid + "_" + unid + ".dat";
}

static void info_callback(const char *topic,
                          const char *message,
                          const size_t message_length)
{
  if (image_available_function_callback == nullptr) {
    sl_log_error(LOG_TAG, "Callback for available image is a nullptr");
    return;
  }
  meta_t meta_info;

  // Extract unid and uiid
  meta_info.unid = get_unid_by_topic(topic);
  meta_info.uiid = get_uiid_by_topic(topic);
  if (meta_info.uiid == "") {
    sl_log_error(LOG_TAG, "UIID was empty, wrong topic");
    return;
  }

  // Image key
  bool unid_in_topic = !meta_info.unid.empty();
  std::string image_key
    = unid_in_topic ? meta_info.uiid + "/" + meta_info.unid : meta_info.uiid;

  if (message_length == 0) {
    sl_log_debug(LOG_TAG,
                 "Removing image meta information for uiid: %s (unid: %s)",
                 meta_info.uiid.c_str(),
                 meta_info.unid.c_str());
    remove_image_meta_info(image_key);
    return;
  }

  // Parse payload data
  bpt::ptree pt;
  try {
    parse_json(message, message_length, pt);
  } catch (std::exception const &ex) {
    sl_log_error(LOG_TAG,
                 "Unable to parse json payload on topic %s. Error is: %s",
                 topic,
                 ex.what());
    return;
  } catch (...) {
    sl_log_error(LOG_TAG,
                 "Unable to parse json payload on topic %s. Unknown error");
  }

  try {
    meta_info.apply_after = parse_time_from_meta_info(pt);
    meta_info.version     = pt.get<std::string>("Version");
    meta_info.filename
      = parse_filename_json(pt, meta_info.unid, meta_info.uiid);
  } catch (std::invalid_argument const &ex) {
    sl_log_error(LOG_TAG,
                 "UIID: %s UNID: %s: %s",
                 meta_info.uiid.c_str(),
                 meta_info.unid.c_str(),
                 ex.what());
    return;
  } catch (const std::exception &ex) {
    sl_log_error(LOG_TAG, "Unable to parse meta info, error: %s", ex.what());
    return;
  } catch (...) {
    sl_log_error(LOG_TAG, "Unable to parse meta info, unknown error");
    return;
  }

  set_image_meta_info(image_key, meta_info);

  if (meta_info.unid.empty()) {
    std::vector<std::string> unids = get_unids_listening(meta_info.uiid);

    for (auto &unid: unids) {
      meta_info.unid = unid;
      image_available_function_callback(meta_info);
    }

    return;
  }

  image_available_function_callback(meta_info);
  return;
}

static void image_download_timeout(void *data)
{
  const auto image_data = static_cast<TimeoutHandler *>(data);
  std::string image_key = std::string(image_data->get_image_key());

  // Unsubscribe
  std::string ota_data_topic
    = (image_key.find("/") != std::string::npos)
        ? std::string(ota_data_base_topic) + "/" + image_key
        : std::string(ota_data_base_topic) + "/" + image_key + "/all";
  uic_mqtt_unsubscribe(ota_data_topic.c_str(), image_ready_callback);

  if (get_image_key_poll_state(image_key) == true) {
    return;
  }

  get_image_callback(image_key)(image_ready_result_t::TIMED_OUT, "");
  pop_image_key_poll(image_key);
  sl_log_warning(
    LOG_TAG,
    "Image download timed out after %d ms waiting for UIID(/UNID): %s",
    image_received_timeout,
    image_key.c_str());
  return;
}

static void image_ready_callback(const char *topic,
                                 const char *message,
                                 const size_t message_length)
{
  std::string unid = get_unid_by_topic(topic);
  std::string uiid = get_uiid_by_topic(topic);
  if (uiid == "") {
    sl_log_error(LOG_TAG, "UIID was empty, wrong topic");
    return;
  }
  std::vector<std::pair<std::string, image_ready_funct_t>> list_unids;
  std::string cache_key;
  if (unid == "") {
    list_unids = get_image_listening_list(uiid);
    cache_key  = uiid;
  } else {
    std::pair<std::string, image_ready_funct_t> single_element;
    single_element.first  = unid;
    cache_key             = parse_image_key(unid, uiid);
    single_element.second = get_image_callback(cache_key);
    list_unids.push_back(single_element);
  }

  for (const auto &element: list_unids) {
    // Get image key
    std::string image_key = parse_image_key(element.first, uiid);

    // Image ready callback is fired update key poll state
    set_image_key_poll_state_true(image_key);

    // Filepath
    // Validate image
    std::string filepath = get_image_filepath(element.first, uiid);
    if (is_image_valid_size(message_length, image_cache_size)
        && (SL_STATUS_OK
            == image_write_file(filepath, message, message_length))) {
      set_images_received_cached_filepath(image_key, filepath);

      // Now the protocol controller can take over the firmware upgrade
      element.second(image_ready_result_t::OK, filepath);

    } else {
      element.second(image_ready_result_t::ERROR, "");
      sl_log_warning(LOG_TAG,
                     "Invalid image provided for UIID(/UNID) %s",
                     image_key.c_str());
    }
  }

  remove_unids_listening_on_uiid(uiid);
}

template<typename value> static std::string format_status_payload(value val)
{
  bpt::ptree doc;

  // Needed for removing qotation marks "" around an integer on payload
  bool atomic_type
    = typeid(value) == typeid(uint32_t) || typeid(value) == typeid(size_t);

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

void publish_cluster_revision(const dotdot_unid_t &unid,
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

template<class value>
static void publish_status(const dotdot_unid_t &unid,
                           const dotdot_endpoint_id_t &endpoint,
                           const ota_uiid_t &uiid,
                           value val,
                           const std::string &status)
{
  std::string topic
    = construct_status_update_topic(unid, endpoint, uiid, status);
  std::string payload = format_status_payload(val);
  publish_desired_and_reported(topic, payload);

  if (is_status_published(unid, endpoint) == false) {
    publish_supported_commands(unid, endpoint);
    publish_cluster_revision(unid, endpoint, configured_cluster_revision);
    set_status_published_cached(unid, endpoint);
  }
}

static void get(const std::string &image_key,
                const std::string &subscribe_topic,
                const std::string &publish_topic,
                const image_ready_funct_t &image_ready_cb)
{
  // Subscribe
  uic_mqtt_subscribe(subscribe_topic.c_str(), image_ready_callback);

  // Ask image provider to provide image
  std::string empty_payload = "{}";
  bool retain               = false;
  uic_mqtt_publish(publish_topic.c_str(),
                   empty_payload.c_str(),
                   empty_payload.length(),
                   retain);

  // Poll on download
  timeout_handler_t timeout_data = {image_key, false, image_ready_cb};
  auto timeout_handler = std::make_shared<TimeoutHandler>(timeout_data);
  set_image_key_poll(timeout_handler);

  ctimer_set(timeout_handler.get()->get_ctimer_pointer(),
             image_received_timeout,
             image_download_timeout,
             timeout_handler.get());
}

}  // namespace uic_ota