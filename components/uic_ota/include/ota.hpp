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
#ifndef OTA_H
#define OTA_H

// Unify Components
#include "uic_typedefs.h"
#include "sl_status.h"

// Standard library imports
#include <functional>
#include <string>
#include <ctime>

constexpr uint16_t DEFAULT_CLUSTER_REVISION = 1;

/**
 * @defgroup image_listener OTA (Over-the-Air) Image Listener
 * @ingroup unify_components
 * @brief Component that monitors the image provider for new firmware images
 * and publishes ota information over mqtt.
 *
 * The ota abstracts away the communication with the image provider,
 * the authority supplying new images. Secondly, the ota
 * is responsible for providing state information for OTA devices over mqtt via
 * ucl/by-unid/&lt;UNID&gt;/OTA/&lt;UIID&gt;/+.
 *
 * The ota has an internal cache with images, retrieved from the image
 * provider. Cached images are represented by files on disk.
 *
 * Callers are required to let the image listener know about the progress of the firmware
 * update of its clients, see func `status_update`.
 *
 * Images have an `apply_after` field. it tells when the image is intended to be activated
 * by the clients receiving this image.
 *
@startuml{ota_image_listener_component.png} "OTA Image Listener component" width=10cm
title OTA Image Listener component
legend top
<font color=#0039FB>MQTT Subscription</font>
<font color=#2b3618>MQTT Unsubscribe</font>
<font color=#00003C>Retained MQTT Publication</font>
<font color=#6C2A0D>Unretained MQTT Publication</font>
endlegend

participant "Protocol Controller" as PC
participant "OTA" as IL
participant "MQTT Broker" as MQTT

note over IL
- 'image_available' callback with data containing meta data the newly available image.
- 'image_base_path' path used to download the images to.
- 'cache_size' defines the max amount of images to be cached by the ota component.
- 'timeout' the time before a get function returns with result TIMEOUT
end note
PC-> IL: init(image_available, image_base_path, cache_size, timeout)
note over IL: we have 2 ways to subscribe for images; 1. via uiid 2. via unid's (uiid, unid)
PC->IL:subscribe_uiid(uiid)
activate IL
IL->MQTT:<font color=#0039FB>ucl/OTA/info/<UIID>/all</font>
IL-->PC
deactivate IL
== node discovery ==
loop for n nodes in the network
note over IL
The PC is responsible to obtian nodes and their corresponding fwk versioning info.
Secondly, it needs to tell us when a node is not part of a network anymore.
end note
PC->IL:subscribe_unid(unid, uiid)
activate IL
IL->MQTT:<font color=#0039FB>ucl/OTA/info/<UIID>/<UNID></font>
IL-->PC
deactivate IL
end
...
...
note over MQTT: new image coming in!
MQTT->IL: <font color=#00003C>ucl/OTA/info/<UIID>/all/+</font>
activate IL
IL->PC:image_available(uiid, unid, new_version, apply_after)
deactivate IL
note over PC: protocol controller decides to get the image
PC->IL: get(uiid, image_ready_cb)
activate IL
IL->MQTT:<font color=#0039FB>ucl/OTA/data/<UIID></font>
IL->MQTT:<font color=#6C2A0D>ucl/OTA/data/<UIID>/get</font>
MQTT->IL:<font color=#6C2A0D>ucl/OTA/data/<UIID></font>
IL->MQTT:<font color=#2b3618> ucl/OTA/data/<UIID></font>
IL->PC: image_ready_cb(image_meta)
deactivate IL

...
note over PC: protocol controller sends updates to the image_listener
PC->IL: update_*status*(uiid, unid, endpoint, *status*)
activate IL
IL-> MQTT: <font color=#00003C>ucl/by-unid/<UNID>/OTA/<UIID>/+
IL-->PC
deactivate IL
...

PC->IL:unregister_unid(unid)
activate IL
IL-> MQTT: <font color=#00003C>ucl/by-unid/<UNID>/OTA/<UIID>/+
deactivate IL
PC->IL: unregister(uiid)
PC->IL: clear_cache()
@enduml
 *
 * @{
 */
namespace uic_ota
{
/**
 * @brief Structure used to indicate the firmware upload status.
 * used in function `ota::status_update`.
 */
enum class status_t {
  IDLE,                                  /*< Means the device is not currently
        * participating in any part of an OTA process. */
  DOWNLOAD_IN_PROGRESS,                  /*< One ore more firmware fragments
        * has been sent but the transfer is not complete. */
  WAITING_TO_UPGRADE,                    /*< Indicates that device has
        * transferred the image but has not yet updated. Possibly
        * because of the ApplyAfter timestamp has not yet passed.*/
  WAITING_TO_UPGRADE_VIA_EXTERNAL_EVENT, /*< Device need
        * external/manual activation to complete the update.*/
  WAIT_FOR_MORE,                         /*< Indicates that the client is still
        * waiting to receive more OTA upgrade image files from
        * the server. This is true for a client device that is
        * composed of multiple processors and each processor
        * requires different image or a system which requires
        * multiple images. The client SHALL be in this
        * state until it has received all necessary OTA
        * upgrade images, then it SHALL transition to Download
        * complete state.*/
};

/**
 * @brief Structure used to indicate the last error status.
 * Used by protocol controllers to report firmware download
 * status.
 */
enum class last_error_t {
  SUCCESS            = 0,  /* The operation was successful. */
  ABORT              = 1,  /*< Failed case when a client or a server decides
        * to abort the upgrade process.*/
  NOT_AUTHORIZED     = 2,  ///< no permission to download image
  INVALID_IMAGE      = 3,  /*< Invalid OTA upgrade image (ex. failed
        * signature validation or signer information check or CRC
        * check) */
  REQUIRE_MORE_IMAGE = 4,  /*< The client still requires more OTA
        * upgrade image files in order to successfully upgrade */
  NOT_SUPPORTED      = 5,
};

/**
 * @brief Result enum that describes if an image is ready for the caller
 * to be used to donwload its clients.
 */
enum class image_ready_result_t {
  OK,        ///< success
  TIMED_OUT, /*< image provider can be reached. however it took too long to
        receive and return the response. Request it dropped */
  ERROR,     ///< mqtt client error. the mqtt transport could not get images
};

/**
 * @brief Struct that describes meta for a new available image
 *
 * @param uiid Unique Image Identifier.
 * @param unid Unique Node Identifier.
 * @param apply_after timestamp to firmware upgrade after.
 * @param version version of firmware.
 * @param filename filename of firmware file.
 */
struct meta_t {
  std::string uiid;
  std::string unid;
  time_t apply_after;
  std::string version;
  std::string filename;
};

/**
 * @brief Callback function that is triggered when an new
 * firmware image is detected on the image provider.
 *
 * @param meta meta information of the newly available
 * image.
 */
using image_available_func_t = std::function<void(const meta_t &)>;

/**
 * @brief this callback is returned when the actually firmware image
 * is downloaded and stored to disk, ready to be used for protocol
 * controllers.
 */
using image_ready_funct_t
  = std::function<void(const image_ready_result_t, const std::string &)>;

/**
 * @brief initialize ota_listener
 *
 * @param image_available_cb will be called if there is new
 * image information available for the subscribed nodes.
 * @param image_base_path this path is used to download the images
 * from the imageprovider to.
 * @param cache_size specify a limit of the internal cache
 * size in bytes. note that the maximal image size is not allowed to be
 * bigger as 256 MB.
 * @param timeout_s specify the timeout before a get function
 * returns with a result `TIMEOUT`,
 * @param cluster_revision The ClusterRevision value to advertise on MQTT
 *                         publications. It will default to DEFAULT_CLUSTER_REVISION
 */
sl_status_t init(const image_available_func_t &image_available_cb,
                 const std::string &image_base_path,
                 unsigned long int cache_size,
                 uint32_t timeout_s,
                 uint16_t cluster_revision = DEFAULT_CLUSTER_REVISION);

/**
 * @brief Listen for images that match the UNID + UIID pair/
 * when changes to this key is detected, image_ready will be called.
 *
 * @param unid end-note id.
 * @param uiid unique image id.
 */
void subscribe_unid(const std::string &unid, const std::string &uiid);

/**
 * @brief Stop watching specific images.
 *
 * Will stop watching specific UIID image and will NOT unretain MQTT topics.
 *
 * @param unid end-note id.
 * @param uiid unique image id.
 */
void unsubscribe_unid(const std::string &unid, const std::string &uiid);

/**
 * @brief Stop watching all images for unid and unretain MQTT messages.
 *
 * Will unsubscribe on all image UIIDs and unretain all MQTT topics for the unid.
 *
 * @param unid end-note id.
 */
void unsubscribe_all_unids_uiid(const std::string &unid);

/**
 * @brief After an image_available call is triggered.
 *
 * The get function can be used to get the path to the
 * actual fw image.
 *
 * In this call the actual fw image will be retrieved from the imageprovider
 * component.
 *
 * @param unid end-note id of image.
 * @param uiid unique image id.
 * @param get_ready_cb callback providing a file to the image on disk.
 */
void get_by_unid(const std::string &unid,
                 const std::string &uiid,
                 const image_ready_funct_t &get_ready_cb);

/**
 * @brief Status update of firmware upgrade OTA MQTT for the image_listener
 * component specific for a UIID, UNID, and endpoint device.
 *
 * Upgrading images is the responsibility of the protocol controller. Thus,
 * this function are for the protocol controller to update its state of the
 * firmware upgrade.
 *
 * @param unid end point.
 * @param uiid unique image id.
 * @param endpoint endpoint of device.
 * @param status status of OTA.
 */
void update_status(const dotdot_unid_t &unid,
                   const dotdot_endpoint_id_t &endpoint,
                   const ota_uiid_t &uiid,
                   const status_t status);

/**
 * @brief Size file of firmware upgrade OTA MQTT for the image_listener
 * component specific for a UIID, UNID, and endpoint device.
 *
 * Upgrading images is the responsibility of the protocol controller. Thus,
 * this function are for the protocol controller to update its state of the
 * firmware upgrade.
 *
 * @param unid end point.
 * @param uiid unique image id.
 * @param endpoint endpoint of device.
 * @param size_file Size of file.
 */
void update_size(const dotdot_unid_t &unid,
                 const dotdot_endpoint_id_t &endpoint,
                 const ota_uiid_t &uiid,
                 const size_t size_file);

/**
 * @brief Offset update of firmware upgrade OTA MQTT for the image_listener
 * component specific for a UIID, UNID and endpoint device.
 *
 * Upgrading images is the responsibility of the protocol controller. Thus,
 * this function are for the protocol controller to update it's state of the
 * firmware upgrade.
 *
 * @param unid end point.
 * @param uiid unique image id.
 * @param endpoint endpoint of device.
 * @param offset offset from being done with the upgrade.
 */
void update_offset(const dotdot_unid_t &unid,
                   const dotdot_endpoint_id_t &endpoint,
                   const ota_uiid_t &uiid,
                   const size_t offset);

/**
 * @brief Last error update of firmware upgrade OTA MQTT for the image_listener
 * component specific for a UIID, UNID, and endpoint device.
 *
 * Upgrading images is the responsibility of the protocol controller. Thus,
 * this function are for the protocol controller to update it's state of the
 * firmware upgrade.
 *
 * @param unid end point.
 * @param uiid unique image id.
 * @param endpoint endpoint of device.
 * @param last_error error information.
 */
void update_last_error(const dotdot_unid_t &unid,
                       const dotdot_endpoint_id_t &endpoint,
                       const ota_uiid_t &uiid,
                       const last_error_t last_error);

/**
 * @brief Apply after update of firmware upgrade OTA MQTT for the image_listener
 * component specific for a UIID, UNID, and endpoint device.
 *
 * Upgrading images is the responsibility of the protocol controller. Thus,
 * this function are for the protocol controller to update it's state of the
 * firmware upgrade.
 *
 * @param unid end point.
 * @param uiid unique image id.
 * @param endpoint endpoint of device.
 * @param apply_after Time to apply after, MUST be in UTC time!
 */
void update_apply_after(const dotdot_unid_t &unid,
                        const dotdot_endpoint_id_t &endpoint,
                        const ota_uiid_t &uiid,
                        const time_t apply_after);

/**
 * @brief Current version update of firmware upgrade OTA MQTT for the image_listener
 * component specific for a UIID, UNID, and endpoint device.
 *
 * Upgrading images is the responsibility of the protocol controller. Thus,
 * this function are for the protocol controller to update it's state of the
 * firmware upgrade.
 *
 * @param unid end point.
 * @param uiid unique image id.
 * @param endpoint endpoint of device.
 * @param current_version string representing the current version of device.
 */
void update_current_version(const dotdot_unid_t &unid,
                            const dotdot_endpoint_id_t &endpoint,
                            const ota_uiid_t &uiid,
                            const std::string &current_version);

/**
 * @brief Target version update of firmware upgrade OTA MQTT for the image_listener
 * component specific for a UIID, UNID, and endpoint device.
 *
 * Upgrading images is the responsibility of the protocol controller. Thus,
 * this function are for the protocol controller to update it's state of the
 * firmware upgrade.
 *
 * @param unid end point.
 * @param uiid unique image id.
 * @param endpoint endpoint of device.
 * @param target_version desired version of target.
 */
void update_target_version(const dotdot_unid_t &unid,
                           const dotdot_endpoint_id_t &endpoint,
                           const ota_uiid_t &uiid,
                           const std::string &target_version);

/**
 * @brief Utility function to clear the internal cache.
 */
void clear_cache();

/**
 * @brief Clean up MQTT status for UNIDs and endpoints.
 *
 * When importing this shared component this function should be used on
 * shutdown of the protocol controller. This will clean up MQTT topics.
 */
void unretain_ota_status();

/**
 * @brief Clean up MQTT status for a UNID and related endpoints.
 *
 * This function should be used when a device or endpoint
 * disconnects for cleaning up OTA statuses.
 */
void unretain_ota_status_by_unid(const dotdot_unid_t &unid);

};  // namespace uic_ota
/**@}*/
#endif
