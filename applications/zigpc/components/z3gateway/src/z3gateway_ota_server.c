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
#include EMBER_AF_API_AF_HEADER
#include EMBER_AF_API_EMBER_TYPES
#include EMBER_AF_API_DEVICE_TABLE
#include EMBER_AF_API_OTA_SERVER_POLICY
#include EMBER_AF_API_OTA_SERVER
#include EMBER_AF_API_OTA_STORAGE


#include "z3gateway_common.h"

/** @brief OTA Server Block Sent Callback
 *
 * This function will be called when a block is sent to a device.
 *
 * @param actualLength      Ver.: always
 * @param manufacturerId    Ver.: always
 * @param imageTypeId       Ver.: always
 * @param firmwareVersion   Ver.: always
 */
void emberAfPluginOtaServerBlockSentCallback(uint8_t actualLength,
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
  emberAfPluginOtaServerPolicyGetClientDelayUnits(EmberNodeId clientNodeId,
                                                  EmberEUI64 clientEui64)
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
void emberAfPluginOtaServerUpdateStartedCallback(uint16_t manufacturerId,
                                                 uint16_t imageTypeId,
                                                 uint32_t firmwareVersion,
                                                 uint8_t maxDataSize,
                                                 uint32_t offset)
{
  if (!Z3GATEWAY_CALLBACK_EXISTS(z3gwState.callbacks, onOtaUpdateStarted)) {
    emberAfCorePrintln("Ignoring uninitialized OTAUpdateComplete callback");

  } else {
    EmberEUI64 eui64;
    EmberNodeId source = emberAfCurrentCommand()->source;

    bool eui64Found = emberAfDeviceTableGetEui64FromNodeId(source, eui64);
    if (!eui64Found) {
      emberAfCorePrintln("Error: Failed to find device EUI64");
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
void emberAfPluginOtaServerUpdateCompleteCallback(uint16_t manufacturerId,
                                                  uint16_t imageTypeId,
                                                  uint32_t firmwareVersion,
                                                  EmberNodeId source,
                                                  uint8_t status)
{
  if (!Z3GATEWAY_CALLBACK_EXISTS(z3gwState.callbacks, onOtaUpdateCompleted)) {
    emberAfCorePrintln("Ignoring uninitialized OTAUpdateComplete callback");

  } else {
    EmberEUI64 eui64;

    bool eui64Found = emberAfDeviceTableGetEui64FromNodeId(source, eui64);
    if (!eui64Found) {
      emberAfCorePrintln("Error: Failed to find device EUI64");
    } else {
      z3gwState.callbacks->onOtaUpdateCompleted(eui64,
                                                manufacturerId,
                                                imageTypeId,
                                                firmwareVersion,
                                                status);
    }
  }
}

EmberStatus z3gatewayAddOtaImage(const char* filename)
{

    EmberAfOtaStorageStatus status =
        emAfOtaStorageAddImageFile(filename);

    return (status == EMBER_AF_OTA_STORAGE_SUCCESS)?
            EMBER_SUCCESS:
            EMBER_ERR_FATAL; 
}
