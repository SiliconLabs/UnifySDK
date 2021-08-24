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

std::string TimeoutHandler::get_image_key() const
{
  return timeout_data->key;
}

bool TimeoutHandler::get_image_poll_state() const
{
  return timeout_data->state;
}

void TimeoutHandler::set_image_poll_state(bool state)
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
// Cache variables
////////////////////////////////////////////////////////////////////////////////
static std::map<std::string, uic_ota::meta_t, std::less<>> image_meta;
static std::map<std::string, std::shared_ptr<TimeoutHandler>, std::less<>>
  image_keys_poll;
static std::map<std::string, std::string, std::less<>> image_cache_filepaths;
static std::map<std::string, std::vector<endpoint_cache_t>, std::less<>>
  status_published_cache;
static std::map<std::string, std::vector<std::string>, std::less<>>
  listening_unids;
static std::map<
  std::string,
  std::vector<std::pair<std::string, uic_ota::image_ready_funct_t>>>
  listening_unid_on_uiid_all;

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

static std::string get_file_name(const std::string &path)
{
  auto fileNameStart = path.find_last_of("/\\");
  auto fileName      = fileNameStart == std::string::npos
                         ? path
                         : path.substr(fileNameStart + 1);
  return fileName;
}

////////////////////////////////////////////////////////////////////////////////
// Component private cache functions
////////////////////////////////////////////////////////////////////////////////
void set_image_listening_unid_on_uiid(
  const std::string &uiid,
  const std::string &unid,
  uic_ota::image_ready_funct_t image_ready_cb)
{
  std::pair<std::string, uic_ota::image_ready_funct_t> value;
  value.first  = unid;
  value.second = image_ready_cb;
  if (listening_unid_on_uiid_all.find(uiid)
      != listening_unid_on_uiid_all.end()) {
    listening_unid_on_uiid_all.at(uiid).emplace_back(value);
  }
  std::vector<std::pair<std::string, uic_ota::image_ready_funct_t>> values {
    value};
  listening_unid_on_uiid_all.emplace(uiid, values);
}

std::vector<std::pair<std::string, uic_ota::image_ready_funct_t>>
  get_image_listening_list(const std::string &uiid)
{
  std::vector<std::pair<std::string, uic_ota::image_ready_funct_t>>
    unids_callbacks;
  if (listening_unid_on_uiid_all.find(uiid)
      != listening_unid_on_uiid_all.end()) {
    unids_callbacks = listening_unid_on_uiid_all.at(uiid);
    return unids_callbacks;
  }
  return unids_callbacks;
}

void remove_unids_listening_on_uiid(const std::string &uiid)
{
  listening_unid_on_uiid_all.erase(uiid);
}

void set_image_meta_info(const std::string &key,
                         const uic_ota::meta_t &meta_info_data)
{
  set_map(key, meta_info_data, image_meta);
}

uic_ota::meta_t get_image_meta_info(const std::string &key)
{
  if (image_meta.find(key) != image_meta.end()) {
    return image_meta.at(key);
  }

  sl_log_error(LOG_TAG, "Could not find meta info");
  throw std::invalid_argument("Could not find meta info for image key");
}

void remove_image_meta_info(const std::string &key)
{
  image_meta.erase(key);
}

bool check_if_meta_info_received(const std::string &key)
{
  if (image_meta.find(key) != image_meta.end()) {
    return true;
  }
  return false;
}

void set_unid_listening(const std::string &uiid, const std::string unid)
{
  if (listening_unids.find(uiid) != listening_unids.end()) {
    listening_unids.at(uiid).emplace_back(unid);
    return;
  }
  std::vector<std::string> unids {unid};
  listening_unids.emplace(uiid, unids);
}

void remove_unid_listening(const std::string &uiid, const std::string &unid)
{
  if (listening_unids.find(uiid) != listening_unids.end()) {
    sl_log_debug(LOG_TAG, "Removing unid: %s", unid.c_str());
    std::vector<std::string>::iterator it
      = std::find(listening_unids.at(uiid).begin(),
                  listening_unids.at(uiid).end(),
                  std::string(unid));

    // Remove element by value
    if (it != listening_unids.at(uiid).end()) {
      listening_unids.at(uiid).erase(it);
    }

    // Remove the element completely if empty
    if (listening_unids.at(uiid).empty()) {
      listening_unids.erase(uiid);
    }
  }
  return;
}

std::vector<std::string> get_unids_listening(const std::string &uiid)
{
  std::vector<std::string> unids = {};
  if (listening_unids.find(uiid) != listening_unids.end()) {
    unids = listening_unids.at(uiid);
  }
  return unids;
}

uint32_t get_unids_listening_count(const std::string &uiid)
{
  if (listening_unids.find(uiid) != listening_unids.end()) {
    return listening_unids.at(uiid).size();
  }
  return 0;
}

void set_image_key_poll(std::shared_ptr<TimeoutHandler> timeout_handler)
{
  set_map(timeout_handler->get_image_key(), timeout_handler, image_keys_poll);
}

void pop_image_key_poll(const std::string &key)
{
  image_keys_poll.erase(key);
}

bool get_image_key_poll_state(const std::string &key)
{
  return image_keys_poll.at(key)->get_image_poll_state();
}

void set_image_key_poll_state_true(const std::string &key)
{
  image_keys_poll.at(key)->set_image_poll_state(true);
}

uic_ota::image_ready_funct_t get_image_callback(const std::string &key)
{
  return image_keys_poll.at(key)->get_image_ready_cb();
}

void set_images_received_cached_filepath(const std::string &key,
                                         const std::string &filepath)
{
  set_map(key, filepath, image_cache_filepaths);
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

std::string get_image_cached_filepath(const std::string &key)
{
  return image_cache_filepaths.at(key);
}

void clear_images_cache()
{
  image_cache_filepaths.clear();
}

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
