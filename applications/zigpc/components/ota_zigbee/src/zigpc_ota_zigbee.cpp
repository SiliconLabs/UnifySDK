/*******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>

#include "sl_log.h"
#include "ota.hpp"

// ZigPC includes
#include <zigpc_common_unid.h>
#include <zigpc_datastore.h>

#include "zigpc_ota_zigbee.h"
#include "zigpc_ota_zigbee_int.h"

#include <zigpc_config.h>

#define LOG_TAG "zigpc_ota_zigbee"

const int INVALID_OTA_POINTER = -2;

void zigpc_ota_zigbee_image_available(uic_ota::meta_t ota_meta_info)
{
  sl_log_info(LOG_TAG, "OTA Image avialable for UNID %s", ota_meta_info.unid);
  std::string unid_str(ota_meta_info.unid);
  size_t position = unid_str.find(ZIGPC_UNID_PREFIX);

  if (position != std::string::npos) {
    unid_str.erase(position, ZIGPC_UNID_PREFIX_LENGTH);
    zigbee_eui64_t eui64 = {0, 0, 0, 0, 0, 0, 0, 0};
    sl_status_t status
      = str_to_zigbee_eui64(unid_str.c_str(), unid_str.size(), eui64);

    zigbee_endpoint_id_t ota_endpoint_id;
    status = zigpc_datastore_find_endpoint_containing_cluster(
      eui64,
      ZCL_CLUSTER_CLIENT_SIDE,
      ZIGPC_ZCL_CLUSTER_OTA_UPGRADE,
      &ota_endpoint_id);

    if (SL_STATUS_OK == status) {
      uic_ota::get_by_unid(ota_meta_info.unid,
                           ota_meta_info.uiid,
                           zigpc_ota_zigbee_image_ready);

      uic_ota::update_status(
        ota_meta_info.unid.c_str(),
        ota_endpoint_id,
        ota_meta_info.unid.c_str(),
        uic_ota::status_t::WAITING_TO_UPGRADE_VIA_EXTERNAL_EVENT);

      uic_ota::update_last_error(ota_meta_info.unid.c_str(),
                                 ota_endpoint_id,
                                 ota_meta_info.unid.c_str(),
                                 uic_ota::last_error_t::SUCCESS);
    }
  }
}

sl_status_t zigpc_ota_init()
{
  sl_status_t status = SL_STATUS_OK;

  //check or create folder
  //absolutely required to exist when we pass it to the uic_ota_listener

  int folder_check = INVALID_OTA_POINTER;
  if (zigpc_get_config()->ota_path != nullptr)
  {
    folder_check = mkdir(zigpc_get_config()->ota_path,
                           S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  }

  if (folder_check == 0) {
    status = SL_STATUS_OK;
    sl_log_info(LOG_TAG,
                "Successfully created required folder for OTA updates");
  } else if ((folder_check == -1) && (EEXIST == errno)) {
    status = SL_STATUS_OK;
    sl_log_info(LOG_TAG, "Found required folder");
  } else {
    sl_log_warning(
      LOG_TAG,
      "Error accessing or creating folder for OTA with error code: %d",
      folder_check);
    sl_log_warning(LOG_TAG,
                   "Do not continue initialization of the OTA component");

    //set to fail so other functions will not be enabled
    //set to NOT_AVAILABLE to indicate setup failed non-critically
    status = SL_STATUS_NOT_AVAILABLE;
  }

  if (SL_STATUS_OK == status) {
    status = ota_zigbee_register_observers();
  }

  if (SL_STATUS_OK == status) {
    uic_ota::init(zigpc_ota_zigbee_image_available,
                  zigpc_get_config()->ota_path,
                  ZIGPC_DEFAULT_OTA_CACHE_SIZE,
                  ZIGPC_DEFAULT_OTA_TIMEOUT);
  }

  return status;
}

sl_status_t zigpc_ota_configure_node(const zigbee_eui64_t eui64)
{
  sl_status_t status = SL_STATUS_OK;
  char eui64_cstr[ZIGBEE_EUI64_HEX_STR_LENGTH];

  status = zigbee_eui64_to_str(eui64, eui64_cstr, ZIGBEE_EUI64_HEX_STR_LENGTH);

  if (SL_STATUS_OK == status) {
    std::string eui64_str(eui64_cstr);
    std::string prefix(ZIGPC_UNID_PREFIX);
    std::string unid = prefix + eui64_str;

    sl_log_debug(LOG_TAG, "Configure OTA node %s", eui64_cstr);
    //for now, use the UNID as a UIID for a given node
    uic_ota::subscribe_unid(unid, unid);

    zigbee_endpoint_id_t ota_endpoint_id;
    status = zigpc_datastore_find_endpoint_containing_cluster(
      eui64,
      ZCL_CLUSTER_CLIENT_SIDE,
      ZIGPC_ZCL_CLUSTER_OTA_UPGRADE,
      &ota_endpoint_id);

    if (status == SL_STATUS_OK) {
      uic_ota::update_status(unid.c_str(),
                             ota_endpoint_id,
                             unid.c_str(),
                             uic_ota::status_t::IDLE);

      uic_ota::update_last_error(unid.c_str(),
                                 ota_endpoint_id,
                                 unid.c_str(),
                                 uic_ota::last_error_t::SUCCESS);
    }
  }

  return status;
}
