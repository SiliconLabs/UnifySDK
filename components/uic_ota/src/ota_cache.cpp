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
#include "ota_cache.hpp"
#include "ota.hpp"

// UIC components
#include "sl_log.h"
#include "sys/ctimer.h"

// Standard library
#include <map>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// Type definitions
////////////////////////////////////////////////////////////////////////////////
constexpr char LOG_TAG[] = "uic_ota_cache";

////////////////////////////////////////////////////////////////////////////////
// Timeout class handler of poll data variables and ctimer struct
////////////////////////////////////////////////////////////////////////////////
TimeoutHandler::~TimeoutHandler()
{
  ctimer_stop(&timeout_data->timeout_timer);
}

std::string TimeoutHandler::get_subscribe_topic() const
{
  return timeout_data->subscribe_topic;
}

std::string TimeoutHandler::get_unid() const
{
  return timeout_data->unid;
}

bool TimeoutHandler::get_unid_poll_state() const
{
  return timeout_data->state;
}

void TimeoutHandler::set_unid_poll_state(bool state) const
{
  timeout_data->state = state;
}

uic_ota::image_ready_funct_t TimeoutHandler::get_image_ready_cb() const
{
  return timeout_data->image_ready_cb;
}

ctimer *TimeoutHandler::get_ctimer_pointer() const
{
  return &timeout_data->timeout_timer;
}

////////////////////////////////////////////////////////////////////////////////
// Internal functions
////////////////////////////////////////////////////////////////////////////////
template<typename value_type>
static void set_map(const std::string &key,
                    value_type value,
                    std::map<std::string, value_type, std::less<>> &key_map)
{
  if (key_map.find(key) != key_map.end()) {
    key_map.find(key)->second = value;
    return;
  }
  key_map.insert(std::pair<std::string, value_type>(key, value));
  return;
}

template<typename value_type> static bool check_if_key_is_in_map(
  const std::string &key,
  std::map<std::string, value_type, std::less<>> &key_map)
{
  if (key_map.find(key) != key_map.end()) {
    return true;
  }
  return false;
}

static std::string get_file_name(const std::string &path)
{
  auto fileNameStart = path.find_last_of("/\\");
  auto fileName      = fileNameStart == std::string::npos
                         ? path
                         : path.substr(fileNameStart + 1);
  return fileName;
}

////////////////////////////////////////////////////////////////////////////////
// Image File Handler implementation
////////////////////////////////////////////////////////////////////////////////

namespace image_file_handler
{
// Cache variables
static std::map<std::string, uic_ota::meta_t, std::less<>> image_meta_info;
static std::map<std::string, std::string, std::less<>> image_cache_filepaths;

void set_image_meta_info(const std::string &key,
                         const uic_ota::meta_t &meta_info_data)
{
  set_map(key, meta_info_data, image_meta_info);
}

uic_ota::meta_t get_image_meta_info(const std::string &key)
{
  if (image_meta_info.find(key) != image_meta_info.end()) {
    return image_meta_info.at(key);
  }

  sl_log_debug(LOG_TAG,
               "Could not find meta info for image key %s",
               key.c_str());
  throw std::invalid_argument("Could not find meta info for image key");
}

void remove_image_meta_info(const std::string &key)
{
  image_meta_info.erase(key);
}

bool check_if_meta_info_received(const std::string &key)
{
  return check_if_key_is_in_map(key, image_meta_info);
}

void set_images_received_cached_filepath(const std::string &key,
                                         const std::string &filepath)
{
  set_map(key, filepath, image_cache_filepaths);
}

std::string get_image_cached_filepath(const std::string &key)
{
  return image_cache_filepaths.at(key);
}

bool is_image_received_and_cached(const std::string &key)
{
  if (image_cache_filepaths.find(key) != image_cache_filepaths.end()) {
    bool state = get_file_name(image_cache_filepaths.at(key))
                 != get_image_meta_info(key).filename;
    return state ? false : true;
  }
  return false;
}

void clear_images_cache()
{
  for (const auto &[image_key, filepath]: image_cache_filepaths) {
    remove(filepath.c_str());
  }
  image_cache_filepaths.clear();
}
}  // namespace image_file_handler
////////////////////////////////////////////////////////////////////////////////
// Unid State Handler
////////////////////////////////////////////////////////////////////////////////

namespace unid_state_handler
{
// Cache variables
static std::map<std::string, std::vector<std::string>, std::less<>>
  unids_listening;
static std::map<std::string, std::shared_ptr<TimeoutHandler>, std::less<>>
  unids_in_poll;
static std::map<std::string, std::vector<endpoint_cache_t>, std::less<>>
  status_published_cache;

// Poll unid handling //
void set_unid_poll(std::shared_ptr<TimeoutHandler> timeout_handler)
{
  set_map(timeout_handler->get_unid(), timeout_handler, unids_in_poll);
}

void pop_unid_poll(const std::string &unid)
{
  unids_in_poll.erase(unid);
}

bool get_unid_poll_state(const std::string &unid)
{
  return unids_in_poll.at(unid)->get_unid_poll_state();
}

void set_unid_poll_state_true(const std::string &unid)
{
  unids_in_poll.at(unid)->set_unid_poll_state(true);
}

bool is_unid_in_poll(const std::string &unid)
{
  return check_if_key_is_in_map(unid, unids_in_poll);
}

uic_ota::image_ready_funct_t get_unid_callback(const std::string &unid)
{
  return unids_in_poll.at(unid)->get_image_ready_cb();
}

bool is_poll_topic_subscribed(const std::string &topic)
{
  for (const auto &[unid, poll_data]: unids_in_poll) {
    if (poll_data->get_subscribe_topic() == topic) {
      return true;
    }
  }
  return false;
}

// Unid management //
void set_unid_listening(const std::string &uiid, const std::string &unid)
{
  if (unids_listening.find(uiid) != unids_listening.end()) {
    if (std::find(unids_listening.at(uiid).begin(),
                  unids_listening.at(uiid).end(),
                  unid)
        == unids_listening.at(uiid).end()) {
      unids_listening.at(uiid).emplace_back(unid);
    }
    return;
  }

  std::vector<std::string> unids {unid};
  unids_listening.try_emplace(uiid, unids);
}

std::vector<std::string> get_unids_listening(const std::string &uiid)
{
  std::vector<std::string> unids = {};
  if (unids_listening.find(uiid) != unids_listening.end()) {
    unids = unids_listening.at(uiid);
  }
  return unids;
}

std::size_t get_unids_listening_count(const std::string &uiid)
{
  if (unids_listening.find(uiid) != unids_listening.end()) {
    return unids_listening.at(uiid).size();
  }
  return 0;
}

void remove_unid_listening(const std::string &uiid, const std::string &unid)
{
  if (unids_listening.find(uiid) != unids_listening.end()) {
    sl_log_debug(LOG_TAG,
                 "Removing UNID %s from the list of UNIDs to monitor.",
                 unid.c_str());
    std::vector<std::string>::iterator it
      = std::find(unids_listening.at(uiid).begin(),
                  unids_listening.at(uiid).end(),
                  std::string(unid));

    // Remove element by value
    if (it != unids_listening.at(uiid).end()) {
      unids_listening.at(uiid).erase(it);
    }

    // Remove the element completely if empty
    if (unids_listening.at(uiid).empty()) {
      unids_listening.erase(uiid);
    }
  }
  return;
}

std::vector<std::string> get_uiids_unid_is_listening_to(const std::string &unid)
{
  std::vector<std::string> uiids = {};
  for (const auto &[uiid, unids]: unids_listening) {
    if (std::find(unids.begin(), unids.end(), unid) != unids.end()) {
      uiids.push_back(uiid);
    }
  }
  return uiids;
}

// Unid MQTT management //
void set_status_published_cached(const dotdot_unid_t &unid,
                                 const dotdot_endpoint_id_t &ep)
{
  std::vector<endpoint_cache_t> endpoints;
  if (status_published_cache.find(unid) != status_published_cache.end()) {
    endpoints = status_published_cache.at(unid);
    endpoints.emplace_back(ep);
  } else {
    endpoints = {ep};
  }
  set_map(unid, endpoints, status_published_cache);
  return;
}

bool is_status_published(const dotdot_unid_t &unid,
                         const dotdot_endpoint_id_t &ep)
{
  endpoint_cache_t endpoint = ep;
  if (status_published_cache.find(unid) != status_published_cache.end()) {
    std::vector<endpoint_cache_t> endpoints = status_published_cache.at(unid);

    if (std::find(endpoints.begin(), endpoints.end(), endpoint)
        != endpoints.end()) {
      return true;
    }
  }
  return false;
}

std::map<std::string, std::vector<endpoint_cache_t>, std::less<>>
  get_map_status_published_cached_and_clear()
{
  std::map<std::string, std::vector<endpoint_cache_t>, std::less<>> tmp
    = status_published_cache;
  status_published_cache.clear();
  return tmp;
}

std::vector<endpoint_cache_t>
  get_endpoints_status_published_and_pop(const dotdot_unid_t &unid)
{
  std::vector<endpoint_cache_t> endpoints;
  if (status_published_cache.find(unid) != status_published_cache.end()) {
    endpoints = status_published_cache.at(unid);
    status_published_cache.erase(unid);
    return endpoints;
  }
  return endpoints;
}
}  // namespace unid_state_handler