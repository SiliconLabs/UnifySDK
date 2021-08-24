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
 * @file ota_cache.hpp
 * @brief Caching of image information.
 *
 * This component exposes tools for caching image information for the OTA. 
 *
 * @{
 */

#ifndef OTA_CACHE_HPP
#define OTA_CACHE_HPP

// This component
#include "ota.hpp"

// Standard library
#include <memory>
#include <map>
#include <vector>

// UIC
#include "sys/ctimer.h"

/**
 * @brief Setting the image callback for a specific image key.
 * 
 * @param key image key.
 * @param state if the image callback has been activated.
 * @param image_ready_cb the image ready callback.
 * @param timeout_timer the ctimer.
 */
typedef struct timeout_handler {
  std::string key;
  bool state = false;
  uic_ota::image_ready_funct_t image_ready_cb;
  struct ctimer timeout_timer;
} timeout_handler_t;

/**
 * 
 * @class TimeoutHandler.
 * @brief Handling timeout data and ctimers sharing data.
 */
class TimeoutHandler
{
  std::shared_ptr<timeout_handler_t> timeout_data;

  public:
  /**
   * @brief TimeoutHandler constructor.
   * @param input_timeout_data A struct containing all data needed for timeout callbacks.
   */
  TimeoutHandler() = default;
  TimeoutHandler(timeout_handler_t input_timeout_data) :
    timeout_data {std::make_unique<timeout_handler_t>(input_timeout_data)} {};
  TimeoutHandler &operator=(const TimeoutHandler &) = delete;
  TimeoutHandler(TimeoutHandler &&)                 = delete;
  TimeoutHandler &operator=(TimeoutHandler &&) = delete;

  /**
   * @brief TimeoutHandler deconstructor. Will stop ctimer in shared ptr.
   */
  ~TimeoutHandler();

  std::string get_image_key() const;
  bool get_image_poll_state() const;
  void set_image_poll_state(bool state);
  uic_ota::image_ready_funct_t get_image_ready_cb() const;
  ctimer *get_ctimer_pointer() const;
};

/**
 * @brief Type definition for endpoint caching.
 *  
 */
using endpoint_cache_t = uint8_t;

/**
 * @brief Setting the image callback for a specific image key.
 * 
 * @param key image key for setting image ready callback.
 * @param image_ready_cb image ready callback.
 */
void set_image_callback_function(
  const std::string &key, const uic_ota::image_ready_funct_t image_ready_cb);

/**
 * @brief Setting image status of a specific image key.
 * 
 * @param timeout_handler handler
 */
void set_image_key_poll(std::shared_ptr<TimeoutHandler> timeout_handler);

/**
 * @brief Pop key from image key poll.
 * 
 * @param key Image key in que.
 */
void pop_image_key_poll(const std::string &key);

/**
 * @brief Get image key poll state.
 * 
 * @param key Image key in poll.
 * 
 * @returns boolean true if the image key has been activated.
 */
bool get_image_key_poll_state(const std::string &key);

/**
 * @brief Set image key poll state.
 * 
 * @param key Image key in poll.
 */
void set_image_key_poll_state_true(const std::string &key);

/**
 * @brief Set unid to listening on uiid.
 * 
 * @param uiid unique image identifier ID.
 * @param unid end-note id.
 */
void set_unid_listening(const std::string &uiid, const std::string unid);

/**
 * @brief Get all unids listening to a specific uiid.
 * 
 * @param uiid unique image identifier ID.
 * 
 * @returns vector of unids listening to uiid.
 */
std::vector<std::string> get_unids_listening(const std::string &uiid);

/**
 * @brief Get count of listening unids to a uiid.
 * 
 * @param uiid unique image identifier ID.
 * 
 * @returns count of listening unids to uiid.
 */
uint32_t get_unids_listening_count(const std::string &uiid);

/**
 * @brief Remove unid listening on uiid.
 * 
 * @param uiid unique image identifier ID.
 * @param unid end-note id.
 * 
 */
void remove_unid_listening(const std::string &uiid, const std::string &unid);

/**
 * @brief Setting image filename.
 * 
 * @param key image key for setting image filename.
 * @param meta_info meta info to set
 */
void set_image_meta_info(const std::string &key,
                         const uic_ota::meta_t &meta_info);

/**
 * @brief Get image meta info.
 * 
 * @param key image key for getting meta info.
 * 
 * @returns meta info structure of image.
 * 
 * @throws std::invalid_argument if meta info could not be found.
 */
uic_ota::meta_t get_image_meta_info(const std::string &key);

/**
 * @brief Remove image meta info.
 * 
 * @param key image key removing meta info for.
 */
void remove_image_meta_info(const std::string &key);

/**
 * @brief Checks if meta info is received for image key.
 * 
 * @param key image key identifier.
 * 
 * @returns if image meta info is recieved.
 */
bool check_if_meta_info_received(const std::string &key);

/**
 * @brief Get the image callback for a specific image key.
 * 
 * @param key image key for getting image ready callback.
 * 
 * @returns image ready callback for image.
 */
uic_ota::image_ready_funct_t get_image_callback(const std::string &key);

/**
 * @brief Remove image callback for image key.
 * 
 * @param key image key for removing image callback .
 * 
 */
void remove_image_callback(const std::string &key);

/**
 * @brief Check if image is received and cached.
 * 
 * @param key image key or search.
 * 
 * @returns true if received and cached.
 */
bool is_image_received_and_cached(const std::string &key);

/**
 * @brief Get image which is received and cached.
 * 
 * @param key image key which is received and cached.
 * 
 * @returns filepath for image.
 */
std::string get_image_cached_filepath(const std::string &key);

/**
 * @brief Set image which is received and cache it.
 * 
 * @param key image key which is received.
 * @param filepath for image which is received.
 * 
 */
void set_images_received_cached_filepath(const std::string &key,
                                         const std::string &filepath);

/**
 * @brief Clear out cache of images.
 * 
 */
void clear_images_cache();

/**
 * @brief Check if unid and ep has been published to.
 * 
 * @param unid UNID to check.
 * @param ep endpoint ID to check.
 * 
 */
bool is_status_published(const dotdot_unid_t &unid,
                         const dotdot_endpoint_id_t &ep);

/**
 * @brief Set unid and endpoint as published to.
 * 
 * @param unid UNID to set as published.
 * @param ep Endpoint ID to set as published to.
 * 
 */
void set_status_published_cached(const dotdot_unid_t &unid,
                                 const dotdot_endpoint_id_t &ep);

/**
 * @brief Get map of published unid and endpoints.
 * 
 * @returns map of published unid and endpoints.
 */
std::map<std::string, std::vector<endpoint_cache_t>, std::less<>>
  get_map_status_published_cached_and_clear();

/**
 * @brief Get published status updates on UNID and pop.
 * 
 * @returns vector of endpoints belonging to specific UNID.
 */
std::vector<endpoint_cache_t>
  get_endpoints_status_published_and_pop(const dotdot_unid_t &unid);

/**
 * @brief Set a unid listening to a uiid and it's image callback.
 * 
 * @param uiid image identifier.
 * @param unid unid listening on uiid.
 * @param image_ready_cb image ready callback listening on uiid.
 */
void set_image_listening_unid_on_uiid(
  const std::string &uiid,
  const std::string &unid,
  uic_ota::image_ready_funct_t image_ready_cb);

/**
 * @brief Get all a unids listening to uiid and image callbacks.
 * 
 * @param uiid image identifier.
 */
std::vector<std::pair<std::string, uic_ota::image_ready_funct_t>>
  get_image_listening_list(const std::string &uiid);

/**
 * @brief Remove all unids listening to uiid and callbacks.
 * 
 * @param uiid image identifier.
 */
void remove_unids_listening_on_uiid(const std::string &uiid);

#endif  //OTA_CACHE_HPP
/** @} end ota_cache */
