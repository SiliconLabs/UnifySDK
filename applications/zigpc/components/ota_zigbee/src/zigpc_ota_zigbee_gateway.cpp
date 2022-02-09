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

#include <string>

#include "ota.hpp"
#include "sl_log.h"

// ZigPC includes
#include <zcl_definitions.h>
#include <zigpc_datastore.h>
#include <zigpc_gateway_notify.h>
#include <zigpc_gateway.h>
#include <zigpc_common_unid.h>

#include "zigpc_ota_zigbee_int.h"

#define LOG_TAG "ota_zigbee_gateway"

sl_status_t ota_zigbee_register_observers()
{
  sl_status_t result;

  result
    = zigpc_gateway_register_observer(ZIGPC_GATEWAY_NOTIFY_OTA_UPDATE_STARTED,
                                      on_ota_zigbee_upgrade_start);

  if (result == SL_STATUS_OK) {
    result = zigpc_gateway_register_observer(
      ZIGPC_GATEWAY_NOTIFY_OTA_UPDATE_COMPLETED,
      on_ota_zigbee_upgrade_complete);
  }

  if(result == SL_STATUS_OK)
  {
      result = zigpc_gateway_register_observer(
        ZIGPC_GATEWAY_NOTIFY_NODE_REMOVED,
        on_remove_unsubscribe_ota_listeners);
  }

  sl_log_info(LOG_TAG, "Registering observers with result: %d", result);
  return result;
}

void zigpc_ota_zigbee_image_ready(const uic_ota::image_ready_result_t result,
                                  const std::string &file_path)
{
  //check if the result was good and if the file downloaded properly
  if (uic_ota::image_ready_result_t::OK == result) {
    sl_log_info(LOG_TAG,
                "Successfully downloaded ota image to %s",
                file_path.c_str());

    zigpc_gateway_add_ota_image(file_path.c_str(), file_path.size());
  } else {
    sl_log_warning(LOG_TAG, "Error downloading ota image");
  }
}

void on_ota_zigbee_upgrade_start(void *data)
{
  zigpc_gateway_on_ota_started_t *ota_data
    = (zigpc_gateway_on_ota_started_t *)data;

  sl_status_t status = SL_STATUS_OK;
  char eui64_cstr[ZIGBEE_EUI64_HEX_STR_LENGTH];

  status = zigbee_eui64_to_str(ota_data->eui64,
                               eui64_cstr,
                               ZIGBEE_EUI64_HEX_STR_LENGTH);

  if (SL_STATUS_OK == status) {
    std::string eui64_str(eui64_cstr);
    std::string prefix(ZIGPC_UNID_PREFIX);
    std::string unid = prefix + eui64_str;

    zigbee_endpoint_id_t ota_endpoint_id;
    status = zigpc_datastore_find_endpoint_containing_cluster(
      ota_data->eui64,
      ZCL_CLUSTER_CLIENT_SIDE,
      ZIGPC_ZCL_CLUSTER_OTA_UPGRADE,
      &ota_endpoint_id);

    if (status == SL_STATUS_OK) {
      uic_ota::update_status(unid.c_str(),
                             ota_endpoint_id,
                             unid.c_str(),
                             uic_ota::status_t::DOWNLOAD_IN_PROGRESS);

      uic_ota::update_last_error(unid.c_str(),
                                 ota_endpoint_id,
                                 unid.c_str(),
                                 uic_ota::last_error_t::SUCCESS);
    }
  }
}

void on_ota_zigbee_upgrade_complete(void *data)
{
  sl_log_info(LOG_TAG, "OTA complete");

  zigpc_gateway_on_ota_completed_t *ota_data
    = (zigpc_gateway_on_ota_completed_t *)data;

  sl_status_t status = SL_STATUS_OK;
  char eui64_cstr[ZIGBEE_EUI64_HEX_STR_LENGTH];

  status = zigbee_eui64_to_str(ota_data->eui64,
                               eui64_cstr,
                               ZIGBEE_EUI64_HEX_STR_LENGTH);

  if (SL_STATUS_OK == status) {
    sl_log_info(LOG_TAG, "OTA update status");
    std::string eui64_str(eui64_cstr);
    std::string prefix(ZIGPC_UNID_PREFIX);
    std::string unid = prefix + eui64_str;

    zigbee_endpoint_id_t ota_endpoint_id;
    status = zigpc_datastore_find_endpoint_containing_cluster(
      ota_data->eui64,
      ZCL_CLUSTER_CLIENT_SIDE,
      ZIGPC_ZCL_CLUSTER_OTA_UPGRADE,
      &ota_endpoint_id);

    if (status == SL_STATUS_OK) {
      uic_ota::update_status(unid.c_str(),
                             ota_endpoint_id,
                             unid.c_str(),
                             uic_ota::status_t::IDLE);

      uic_ota::last_error_t last_error
        = (ota_data->status == ZIGPC_ZCL_STATUS_SUCCESS)
            ? uic_ota::last_error_t::SUCCESS
            : uic_ota::last_error_t::ABORT;

      uic_ota::update_last_error(unid.c_str(),
                                 ota_endpoint_id,
                                 unid.c_str(),
                                 last_error);
    }
  }
}

void on_remove_unsubscribe_ota_listeners( void* data)
{
   
    zigpc_gateway_on_node_removed_t* removed_data =
        (zigpc_gateway_on_node_removed_t*)data;
 

    if(data == nullptr)
    {
        sl_log_warning(LOG_TAG, "Error unsubscribing OTA topics");
        return;
    }

    sl_status_t status = SL_STATUS_OK;
    char eui64_cstr[ZIGBEE_EUI64_HEX_STR_LENGTH];

    status = zigbee_eui64_to_str(removed_data->eui64,
                                    eui64_cstr,
                                    ZIGBEE_EUI64_HEX_STR_LENGTH);

    if(SL_STATUS_OK == status)
    {
        
        std::string eui64_str(eui64_cstr);
        std::string prefix(ZIGPC_UNID_PREFIX);
        std::string unid = prefix + eui64_str;
        
        sl_log_info(LOG_TAG, "Removing OTA listeners for UNID: %s", unid.c_str());
        
        uic_ota::unsubscribe_all_unids_uiid(unid);
    }
    else
    {
        sl_log_warning(LOG_TAG, "Error unsubscribing OTA topics");
    }

}
