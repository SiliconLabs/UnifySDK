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
 * @file ota_download.hpp
 * @brief MQTT actions supporting downloading images.
 *
 * This module enables functionality for downloading images through the MQTT
 * broker. The module complies with the Unify spec.
 *
 * @{
@startuml{ota_internal.png} "OTA internal" width=10cm
title OTA download from image provider

participant "OTA cache" as OTAC
participant "OTA API" as API
participant "Protocol controller" as PC
participant "Image provider" as IP

PC -> API: subscribe_unid(unid, uiid)

alt If no unids listening to uiid 
API -> IP: Subscribe: ucl/OTA/info/uiid/all
end
API -> IP: Subscribe: ucl/OTA/info/uiid/unid
API -> OTAC: unid listening to uiid
alt #LightBlue specific unid available 
== image availability ==
IP -> API: Publish: ucl/OTA/info/uiid/unid
API -> OTAC: Cache image meta info for uiid/unid
API -> PC: image_available_callback(meta_info)
== image download ==
PC -> API: get_by_unid(unid, uiid, image_ready_cb)
alt #LightBlue image not cached
API -> IP: Subscribe: ucl/OTA/data/uiid/unid
API -> IP: Publish: ucl/OTA/info/uiid/unid/get
alt #LightBlue image received
IP -> API: Publish: image file data message
API -> OTAC: Cache image file
API -> PC: image_ready_cb(OK, filepath)
else no image received
API -> PC: image_ready_cb(ERROR, "")
end
API -> IP: Unsubscribe: ucl/OTA/data/uiid/unid
else #LightBlue image cached
API -> OTAC: get image filepath from cache
OTAC -> API: image filepath
API -> PC: image_ready_cb(OK, filepath)
end

else #LightGreen no specific unid available
== image availability ==
IP -> API: Publish: ucl/OTA/info/uiid/all
API -> OTAC: Cache image meta info for uiid/all
API -> OTAC: get_unids_listening_to_uiid()
OTAC -> API: list of unids listening to the uiid
loop #LightGreen for all unids listening to uiid
API -> PC: image_available_callback(meta_info)
== image download ==
PC -> API: get_by_unid(unid, uiid, image_ready_cb)
alt #LightGreen image not cached
API -> IP: Subscribe: ucl/OTA/data/uiid/all
API -> IP: Publish: ucl/OTA/info/uiid/all/get
alt #LightGreen image received
IP -> API: image file data message
API -> OTAC: cache image file
API -> PC: image_ready_cb(OK, filepath)
else #LightGreen no image received
API -> PC: image_ready_cb(ERROR, "")
end
API -> IP: Unsubscribe: ucl/OTA/data/uiid/all
else #LightGreen image cached
API -> OTAC: get image filepath from cache
OTAC -> API: image filepath
API -> PC: image_ready_cb(OK, filepath)
end
end
end
PC -> API: unsubscribe_unid(unid, uiid)
API -> OTAC: remove unid listening to uiid
API -> IP: Unsubscribe: ucl/OTA/info/uiid/unid
alt If no more unids listening to uiid
API -> IP: Unsubscribe: ucl/OTA/info/uiid/all
end
@enduml
 */

#ifndef OTA_DOWNLOAD_HPP
#define OTA_DOWNLOAD_HPP

#include "ota.hpp"

namespace ota_download
{
/**
 * @brief Set the image cache size for download.
 * 
 * @param cache_size the cache size input. Maximum is 256 Mb.
 */
void set_image_cache_size(unsigned long int cache_size);

/**
 * @brief Set the image base path where image files are saved.
 * 
 * @param image_base_path path to directory where images should be saved.
 */
void set_image_base_path(const std::string &image_base_path);

/**
 * @brief Set the callback called for available images.
 * 
 * @param image_available_cb callback which is called when an image is 
 * available.
 */
void set_image_available_callback(
  const uic_ota::image_available_func_t &image_available_cb);

/**
 * @brief Check if available info callback is initialized.
 * 
 * @returns boolean if available info callback is not a nullptr.
 */
bool available_info_callback_initialized();

/**
 * @brief Callback function to be called when revceiving image files 
 * information on info topic.
 * 
 * @param topic info MQTT topic.
 * @param message image file contents transferred.
 * @param message_length size of image file.
 */
void info_callback(const char *topic,
                   const char *message,
                   const size_t message_length);

/**
 * @brief Function for downloading images.
 * 
 * @param image_key unique image key for downloading for this image.
 * @param subscribe_topic the data topic to subscribe to where image file is 
 * received.
 * @param publish_topic topic to publish to telling image provider to provide 
 * image file.
 * @param image_ready_cb callback for when image is ready for over the air 
 * transfer.
 */
void get(const std::string &image_key,
         const std::string &subscribe_topic,
         const std::string &publish_topic,
         const uic_ota::image_ready_funct_t &image_ready_cb,
         uint32_t timeout_s);
}  // namespace ota_download

#endif  //OTA_DOWNLOAD_HPP
/** @} end ota_download */
