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

#include PLATFORM_HEADER
#include "app/framework/include/af.h"
#include "app/framework/plugin/address-table/address-table.h"
#include "app/framework/plugin/ota-server-policy/ota-server-policy.h"
#include "app/framework/plugin/ota-server/ota-server.h"
#include "app/framework/plugin/ota-storage-common/ota-storage.h"

#include "zigbee_host_common.h"

extern struct zigbeeHostState z3gwState;

/** @brief OTA Server Block Sent Callback
 *
 * This function will be called when a block is sent to a device.
 *
 * @param actualLength      Ver.: always
 * @param manufacturerId    Ver.: always
 * @param imageTypeId       Ver.: always
 * @param firmwareVersion   Ver.: always
 */
void sl_zigbee_af_ota_server_block_sent_cb(uint8_t actualLength,
                                             uint16_t manufacturerId,
                                             uint16_t imageTypeId,
                                             uint32_t firmwareVersion)
{
  // TODO: Investigate if update progress can be calculated in here
}

/** @brief OTA Server Get Client Delay Units
 *
 * Called when the server receives an Image Block Request from a client that
 * supports rate limiting using the Minimum Block Period feature. This callback
 * gives the server a chance to decide which units the client uses for the
 * Minimum Block Period, seconds or milliseconds. The server can also return
 * OTA_SERVER_DISCOVER_CLIENT_DELAY_UNITS, which causes the plugin code to test
 * the client by sending it a preset delay value. The length of time the client
 * delays determines which units it uses. For more information on this feature,
 * please read the plugin option descriptions under the OTA Server plugin.
 *
 * @param clientNodeId     Ver.: always
 * @param clientEui64      Ver.: always
 *
 * For return values, see ota-server-policy.h. An unknown return value will
 * result in the same behavior as if OTA_SERVER_CLIENT_USES_MILLISECONDS had
 * been returned.
 */
uint8_t
  sl_zigbee_af_ota_server_policy_get_client_delay_units(sl_802154_short_addr_t clientNodeId,
                                                  sl_802154_long_addr_t clientEui64)
{
  return OTA_SERVER_DISCOVER_CLIENT_DELAY_UNITS;
}

/** @brief OTA Server Update Started Callback
 *
 * Called when an OTA upgrade starts.
 *
 * @param manufacturerId   Ver.: always
 * @param imageTypeId      Ver.: always
 * @param firmwareVersion  Ver.: always
 * @param maxDataSize      Ver.: always
 * @param offset           Ver.: always
 */
void sl_zigbee_af_ota_server_update_started_cb(uint16_t manufacturerId,
                                                 uint16_t imageTypeId,
                                                 uint32_t firmwareVersion,
                                                 uint8_t maxDataSize,
                                                 uint32_t offset)
{
  if (!ZIGBEE_HOST_CALLBACK_EXISTS(z3gwState.callbacks, onOtaUpdateStarted)) {
    appDebugPrint(LOG_FMTSTR_UNREGISTERED_CALLBACK, "OTA-Server Update Start");

  } else {
    sl_802154_long_addr_t eui64;

    sl_status_t status = sl_zigbee_af_get_current_sender_eui64(eui64);
    if (status != SL_STATUS_OK) {
      appDebugPrint(LOG_FMTSTR_SOURCE_EUI64_RES_FAIL,
                    "OTA-Server Update Start");
    } else {
      z3gwState.callbacks->onOtaUpdateStarted(eui64,
                                              manufacturerId,
                                              imageTypeId,
                                              firmwareVersion);
    }
  }
}

/** @brief OTA Server Update Complete Callback
 *
 * This function will be called when an OTA update has finished.
 *
 * @param manufacturerId  ManufacturerId.
 * @param imageTypeId  Image Type Id.
 * @param firmwareVersion  Firmware Version.
 * @param source  Source node id.
 * @param status  Status of update.
 */
void sl_zigbee_af_ota_server_update_complete_cb(uint16_t manufacturerId,
                                                  uint16_t imageTypeId,
                                                  uint32_t firmwareVersion,
                                                  sl_802154_short_addr_t source,
                                                  uint8_t status)
{
  if (!ZIGBEE_HOST_CALLBACK_EXISTS(z3gwState.callbacks, onOtaUpdateCompleted)) {
    appDebugPrint(LOG_FMTSTR_UNREGISTERED_CALLBACK,
                  "OTA-Server Update Complete");

  } else {
    sl_802154_long_addr_t eui64;

    sl_status_t emStatus = sl_zigbee_af_get_current_sender_eui64(eui64);
    if (emStatus != SL_STATUS_OK) {
      appDebugPrint(LOG_FMTSTR_SOURCE_EUI64_RES_FAIL,
                    "OTA-Server Update Complete");
    } else {
      z3gwState.callbacks->onOtaUpdateCompleted(eui64,
                                                manufacturerId,
                                                imageTypeId,
                                                firmwareVersion,
                                                status);
    }
  }
}

sl_status_t zigbeeHostAddOtaImage(const char *filename)
{
  if (filename == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_zigbee_af_ota_storage_status_t status = sli_zigbee_af_ota_storage_add_image_file(filename);

  return (status == SL_ZIGBEE_AF_OTA_STORAGE_SUCCESS) ? SL_STATUS_OK
                                                  : SL_STATUS_FAIL;
}
