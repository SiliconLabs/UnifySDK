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

// Unify
#include "sys/ctimer.h"

/**
 * @brief Type definition for endpoint caching.
 *
 */
using endpoint_cache_t = uint8_t;

/**
 * @brief Setting the image callback for a specific image key.
 *
 * @param key image key.
 * @param state if the image callback has been activated.
 * @param image_ready_cb the image ready callback.
 * @param timeout_timer the ctimer.
 */
typedef struct timeout_handler {
  std::string unid;
  std::string subscribe_topic;
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

  std::string get_unid() const;
  std::string get_subscribe_topic() const;
  bool get_unid_poll_state() const;
  void set_unid_poll_state(bool state) const;
  uic_ota::image_ready_funct_t get_image_ready_cb() const;
  ctimer *get_ctimer_pointer() const;
};

/**
 * @namespace ImageHandler
 * @brief The ImageFileHandler controls and operates availability of an image
 * file. This class encapsulates following control:
 *  - Image file path.
 *  - Image meta info.
 */
namespace image_file_handler
{
////////////////////////////////////////////////////////////////////////////////
// Image meta info received
////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////
// Cached image files
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Set image which is received and cache it.
 *
 * @param key image key which is received.
 * @param filepath for image which is received.
 */
void set_images_received_cached_filepath(const std::string &key,
                                         const std::string &filepath);

/**
 * @brief Get image which is received and cached.
 *
 * @param key image key which is received and cached.
 *
 * @returns filepath for image.
 */
std::string get_image_cached_filepath(const std::string &key);

/**
 * @brief Check if image is received and cached.
 *
 * @param key image key or search.
 *
 * @returns true if received and cached.
 */
bool is_image_received_and_cached(const std::string &key);

/**
 * @brief Clear out cache of images.
 */
void clear_images_cache();
};  // namespace image_file_handler

/**
 * @namespace UnidStateHandler.
 * @brief Handling UNID state for OTA.
 *
 * Class will encapsulate following:
 * - If an image is provided specifically for the UNID or should accept UIID/all.
 * - The last provided image_ready_cb for UNID in poll.
 * - If the UNID is in poll waiting for an image.
 *
 */
namespace unid_state_handler
{
////////////////////////////////////////////////////////////////////////////////
// Unid poll management
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Setting image status of a specific image key.
 *
 * @param timeout_handler handler
 */
void set_unid_poll(std::shared_ptr<TimeoutHandler> timeout_handler);

/**
 * @brief Pop key from image key poll.
 *
 * @param unid UNID in que.
 */
void pop_unid_poll(const std::string &unid);

/**
 * @brief Get image key poll state.
 *
 * @param unid end-note id.
 *
 * @returns boolean true if the image key has been activated.
 */
bool get_unid_poll_state(const std::string &unid);

/**
 * @brief Set UNID poll state.
 *
 * @param unid end-note id.
 */
void set_unid_poll_state_true(const std::string &unid);

/**
 * @brief Check if UNID in poll.
 *
 * @param unid end-note id.
 */
bool is_unid_in_poll(const std::string &unid);

/**
 * @brief Get the image callback for a specific image key.
 *
 * @param unid Image UNID for getting image ready callback.
 *
 * @returns image ready callback for image.
 */
uic_ota::image_ready_funct_t get_unid_callback(const std::string &unid);

/**
 * @brief Check if the topic is already subscribed.
 *
 * @param topic MQTT topic.
 *
 * @returns true/false.
 */
bool is_poll_topic_subscribed(const std::string &topic);

////////////////////////////////////////////////////////////////////////////////
// Unid management
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Set UNID to listening on UIID.
 *
 * @param uiid unique image identifier ID.
 * @param unid end-note id.
 */
void set_unid_listening(const std::string &uiid, const std::string &unid);

/**
 * @brief Get all UNIDs listening to a specific UIID.
 *
 * @param uiid unique image identifier ID.
 *
 * @returns vector of UNIDs listening to UIID.
 */
std::vector<std::string> get_unids_listening(const std::string &uiid);

/**
 * @brief Get count of listening UNIDs to a UIID.
 *
 * @param uiid unique image identifier ID.
 *
 * @returns count of listening UNIDs to UIID.
 */
std::size_t get_unids_listening_count(const std::string &uiid);

/**
 * @brief Remove UNID listening on UIID.
 *
 * @param uiid unique image identifier ID.
 * @param unid end-note id.
 */
void remove_unid_listening(const std::string &uiid, const std::string &unid);

/**
 * @brief Get all UIIDs which a single UNID is listening to.
 *
 * @param unid end-note id.
 *
 * @returns vector of UIIDs.
 */
std::vector<std::string>
  get_uiids_unid_is_listening_to(const std::string &unid);

/**
 * @brief Get all UIIDs and UNIDs active.
 *
 * @returns map of UIIDs and listening UNIDs.
 */
std::map<std::string, std::vector<std::string>, std::less<>>
  get_uiids_and_unids_listening();

////////////////////////////////////////////////////////////////////////////////
// Unid MQTT management
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Set UNID and endpoint as published to.
 *
 * @param unid end-note id set as published.
 * @param ep Endpoint ID to set as published to.
 */
void set_status_published_cached(const dotdot_unid_t &unid,
                                 const dotdot_endpoint_id_t &ep);

/**
 * @brief Check if UNID and ep has been published to.
 *
 * @param unid end-note id to check.
 * @param ep endpoint ID to check.
 */
bool is_status_published(const dotdot_unid_t &unid,
                         const dotdot_endpoint_id_t &ep);

/**
 * @brief Get map of published UNID and endpoints.
 *
 * @returns map of published UNID and endpoints.
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
};  // namespace unid_state_handler

#endif  //OTA_CACHE_HPP
/** @} end ota_cache */
