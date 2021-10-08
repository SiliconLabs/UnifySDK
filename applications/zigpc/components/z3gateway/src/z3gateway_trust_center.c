/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include <stdint.h>
#include <string.h>
#include <malloc.h>

#include "z3gateway.h"
#include "z3gateway_common.h"

#include PLATFORM_HEADER
#include EMBER_AF_API_AF_HEADER
#include EMBER_AF_API_AF_MAIN_HEADER
#include EMBER_AF_API_AF_SECURITY_HEADER
#include EMBER_AF_API_EZSP_PROTOCOL
#include EMBER_AF_API_EZSP
#include EMBER_AF_API_DEVICE_TABLE
#include EMBER_AF_API_DEVICE_TABLE_INTERNAL
#include EMBER_AF_API_NETWORK_CREATOR

#ifndef Z3GATEWAY_TEST
#include "gen/callback.h"
#else /* Z3GATEWAY_TEST defined */
#include "debug-printing.h"
#endif /* Z3GATEWAY_TEST */

/** @brief Z3Gateway handler for Zigbee Key Establishment
 *
 * A callback to the application to notify it of the status of the request for
 * a Link Key. The status will cover stages from responding to a link key
 * request, potential failure paths, to final link key verification. The link
 * key verification stage is used by Z3Gateway to indicate the end of
 * successful device addition to the network any notify the
 * onTrustCenterDeviceJoinComplete callback for that particular device.
 *
 * @param partner partner The IEEE address of the partner device.  Or all zeros
 * if the Key establishment failed.  Ver.: always
 * @param status The status of the key establishment.  Ver.: always
 */
void z3gatewayZigbeeKeyEstablishmentCallback(const EmberEUI64 partner,
                                             EmberKeyStatus status)
{
  if (status == EMBER_TC_REQUESTER_VERIFY_KEY_SUCCESS) {
    bool callbackExists
      = (z3gwState.callbacks != NULL)
        && (z3gwState.callbacks->onTrustCenterDeviceJoinComplete != NULL);
    if (callbackExists == true) {
      z3gwState.callbacks->onTrustCenterDeviceJoinComplete(partner);
    }
  }
}

/**
 * @brief Aggregate endpoint information from the device-table plugin using
 * the EUI64 and endpoint provided.
 *
 * @param newNodeEui64  node device identifier
 * @param endpoint node endpoint
 */
void z3gatewayTrustCenterDeviceEndpointDiscovered(const EmberEUI64 newNodeEui64,
                                                  uint8_t endpoint)
{
  EmberStatus status = EMBER_SUCCESS;
  EmberNodeId newNodeId;
  uint16_t deviceTableIndex;
  const EmberAfPluginDeviceTableEntry *deviceTableEntry;
  static struct z3gatewayEndpointInfo endpointInfo;

  newNodeId = (EmberNodeId)emberAfDeviceTableGetNodeIdFromEui64(newNodeEui64);
  if (newNodeId == EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
    status = EMBER_NOT_FOUND;
  }

  if (status == EMBER_SUCCESS) {
    deviceTableIndex
      = emAfDeviceTableFindIndexNodeIdEndpoint(newNodeId, endpoint);
    if (deviceTableIndex == EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
      status = EMBER_NOT_FOUND;
    }
  }

  if (status == EMBER_SUCCESS) {
    deviceTableEntry = emberAfDeviceTableFindDeviceTableEntry(deviceTableIndex);

    endpointInfo.endpoint           = endpoint;
    endpointInfo.serverClusterCount = deviceTableEntry->clusterOutStartPosition;
    if (endpointInfo.serverClusterCount > 0) {
      endpointInfo.serverClusterList
        = malloc(sizeof(uint16_t) * endpointInfo.serverClusterCount);

      memcpy(endpointInfo.serverClusterList,
             deviceTableEntry->clusterIds,
             sizeof(uint16_t) * endpointInfo.serverClusterCount);
    }

    int clusterListSize = EMBER_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE
                          - endpointInfo.serverClusterCount;
    if (clusterListSize > 0) {
      endpointInfo.clientClusterList
        = malloc(sizeof(uint16_t) * clusterListSize);
      endpointInfo.clientClusterCount = 0;

      for (unsigned int i = 0; i < clusterListSize; i++) {
        unsigned int rawDeviceTableIndex
          = i + deviceTableEntry->clusterOutStartPosition;
        //TODO get this undefined value from somewhere else
        if (deviceTableEntry->clusterIds[rawDeviceTableIndex] != 0xFFFF) {
          endpointInfo.clientClusterList[i]
            = deviceTableEntry->clusterIds[rawDeviceTableIndex];
          endpointInfo.clientClusterCount++;
        }
      }
    }

    bool callbackExists
      = (z3gwState.callbacks != NULL)
        && (z3gwState.callbacks->onTrustCenterDeviceEndpointDiscovered != NULL);
    if (callbackExists == true) {
      z3gwState.callbacks->onTrustCenterDeviceEndpointDiscovered(newNodeEui64,
                                                                 &endpointInfo);
    }

    if (endpointInfo.serverClusterCount > 0) {
      free(endpointInfo.serverClusterList);
    }
    if (clusterListSize > 0) {
      free(endpointInfo.clientClusterList);
    }
  }
}

EmberKeyData z3gatewayGetTrustCenterWellKownKey(void)
{
  EmberKeyData key = ZIGBEE_PROFILE_INTEROPERABILITY_LINK_KEY;

  return key;
}

EmberStatus z3gatewayTrustCenterAddLinkKey(const EmberEUI64 eui64,
                                           const EmberKeyData *key,
                                           bool inTransientTable)
{
  EmberStatus status = EMBER_SUCCESS;

  // Copy to non-const variables
  EmberEUI64 eui64_cp;
  EmberKeyData key_cp;
  memcpy(eui64_cp, eui64, sizeof(EmberEUI64));
  memcpy(&key_cp, key, sizeof(EmberKeyData));

  if (inTransientTable) {
    status = emberAddTransientLinkKey(eui64_cp, &key_cp);
  } else {
    status = emberAddOrUpdateKeyTableEntry(eui64_cp, true, &key_cp);
  }
  return status;
}

EmberStatus z3gatewayTrustCenterAddDeviceInstallCode(const EmberEUI64 eui64,
                                                     uint8_t *installCode,
                                                     uint8_t installCodeLength)
{
  EmberStatus status = EMBER_SUCCESS;
  EmberKeyData linkKey;

  if ((NULL == installCode) || (NULL == eui64) || (installCodeLength == 0)) {
    status = EMBER_BAD_ARGUMENT;
  } else {
    status = emAfInstallCodeToKey(installCode, installCodeLength, &linkKey);
    if (status != EMBER_SUCCESS) {
      emberAfCorePrintln(
        "Error: Failed to convert Install Code to Link Key: 0x%X",
        status);
    }
  }

  if (status == EMBER_SUCCESS) {
    status = z3gatewayTrustCenterAddLinkKey(eui64, &linkKey, true);
    if (status != EMBER_SUCCESS) {
      emberAfCorePrintln("Error: Failed to add Link Key Pair: 0x%X", status);
    }
  }

  return status;
}

EmberStatus z3gatewayTrustCenterJoinOpen(bool openForever)
{
  EmberStatus status = EMBER_SUCCESS;

  if (emberAfNetworkState() != EMBER_JOINED_NETWORK) {
    status = EMBER_NETWORK_DOWN;
    emberAfCorePrintln("Error: Network not up: 0x%X", status);
  } else {
    // 0xFF: forever, 0xFE: 254 seconds
    uint8_t permitDuration = (openForever == true) ? 0xFF : 0xFE;

    status = emAfPermitJoin(permitDuration, true);
  }

  return status;
}

EmberStatus z3gatewayTrustCenterJoinClose(void)
{
  EmberStatus status = EMBER_SUCCESS;

  if (emberAfNetworkState() != EMBER_JOINED_NETWORK) {
    status = EMBER_NETWORK_DOWN;
    emberAfCorePrintln("Error: Network not up: 0x%X", status);
  } else {
    emberClearTransientLinkKeys();
    status = emAfPermitJoin(0, true);
  }

  return status;
}

EmberStatus z3gatewayTrustCenterRemoveDevice(const EmberEUI64 eui64)
{
  EmberStatus status = EMBER_SUCCESS;
  uint16_t deviceIndex;

  if (eui64 == NULL) {
    status = EMBER_BAD_ARGUMENT;
  }

  if (status == EMBER_SUCCESS) {
    deviceIndex = emberAfDeviceTableGetFirstIndexFromEui64(eui64);

    if (deviceIndex == EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX) {
      status = EMBER_NOT_FOUND;
    }
  }

  if (status == EMBER_SUCCESS) {
    status = emberAfPluginDeviceTableSendLeave(deviceIndex);
  }

  return status;
}

EmberStatus z3gatewayTrustCenterStartDiscovery(const EmberEUI64 eui64)
{
  return emAfDeviceTableStartDeviceDiscovery(eui64);
}

EmberStatus z3gatewayTrustCenterInit(void)
{
  EmberStatus status = EMBER_SUCCESS;

  EmberNetworkStatus networkState = emberAfNetworkState();
  if (networkState == EMBER_NO_NETWORK) {
    status = emberAfPluginNetworkCreatorStart(true);
    if (status != EMBER_SUCCESS) {
      emberAfCorePrintln("Error: Failed to create trust center network: 0x%X",
                         status);
    }
  } else if (networkState == EMBER_JOINED_NETWORK) {
    EmberEUI64 ncpEui64;
    EmberNodeType nodeType;
    EmberNetworkParameters network;
    emberAfGetEui64(ncpEui64);
    status = emberAfGetNetworkParameters(&nodeType, &network);
    if (status != EMBER_SUCCESS) {
      emberAfCorePrintln(
        "Error: Failed to get trust center network parameters: 0x%X",
        status);
    } else if (Z3GATEWAY_CALLBACK_EXISTS(z3gwState.callbacks,
                                         onTrustCenterInitialized)) {
      z3gwState.callbacks->onTrustCenterInitialized(ncpEui64, &network);
    }
  }

  return status;
}

/*****************************************************************************/
/** Trust Center callbacks                                                  **/
/*****************************************************************************/

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be notified
 * of changes to the stack status and take appropriate action.  The return code
 * from this callback is ignored by the framework.  The framework will always
 * process the stack status after the callback returns.
 *
 * @param status   Ver.: always
 */
bool emberAfStackStatusCallback(EmberStatus status)
{
  EmberStatus paramStatus;
  EmberNetworkParameters network;
  EmberEUI64 ncpEui64;
  emberAfGetEui64(ncpEui64);

  if (status == EMBER_NETWORK_UP) {
    paramStatus = emberAfGetNetworkParameters(&paramStatus, &network);
    if (paramStatus != EMBER_SUCCESS) {
      emberAfCorePrintln(
        "Error: Failed to get trust center network parameters: 0x%X",
        paramStatus);
    }

    bool callbackExists = Z3GATEWAY_CALLBACK_EXISTS(z3gwState.callbacks,
                                                    onTrustCenterInitialized);
    if ((paramStatus == EMBER_SUCCESS) && callbackExists) {
      z3gwState.callbacks->onTrustCenterInitialized(ncpEui64, &network);
    }
  }

  return false;
}

/** @brief NewDeviceEndpointCount
 *
 * This callback is called to inform amount of endpoints a joined
 * device contains.
 *
 * @param eui64   Ver.: always
 * @param endpointCount   Ver.: always
 */
void emberAfPluginDeviceTableActiveEndpointsCallback(const EmberEUI64 eui64,
                                                     uint8_t endpointCount)
{
  if (z3gwState.callbacks != NULL
      && z3gwState.callbacks->onTrustCenterDeviceDiscovered != NULL) {
    z3gwState.callbacks->onTrustCenterDeviceDiscovered(eui64, endpointCount);
  }
}

/** @brief TrustCenterJoinCallback
 *
 * This callback is called to inform when a device has joined the Zigbee
 * Trust Center.
 *
 * @param eui64   Ver.: always
 */
void emberAfPluginDeviceTableTrustCenterJoinCallback(const EmberEUI64 eui64)
{
  bool callbackExists
    = (z3gwState.callbacks != NULL)
      && (z3gwState.callbacks->onTrustCenterDeviceJoinStart != NULL);
  if (callbackExists == true) {
    z3gwState.callbacks->onTrustCenterDeviceJoinStart(eui64);
  }
}

/**
 * @brief Device Table plugin callback when a device has completed leaving
 * the network.
 *
 *@param newNodeEui64:  EUI64 of the device that left.
 */
void emberAfPluginDeviceTableDeviceLeftCallback(const EmberEUI64 newNodeEui64)
{
  bool callbackExists = Z3GATEWAY_CALLBACK_EXISTS(z3gwState.callbacks,
                                                  onTrustCenterDeviceRemoved);
  if (callbackExists == true) {
    z3gwState.callbacks->onTrustCenterDeviceRemoved(newNodeEui64);
  }
}

/** @brief NewDevice
 *
 * This callback is called when a new device joins the gateway.
 *
 * @param uui64   Ver.: always
 */
void emberAfPluginDeviceTableNewDeviceCallback(const EmberEUI64 newNodeEui64,
                                               uint8_t endpoint)
{
  z3gatewayTrustCenterDeviceEndpointDiscovered(newNodeEui64, endpoint);
}

/** @brief Rejoin device
 *
 * Called when a device rejoins.
 *
 *@param newNodeEui64:  EUI64 of the rejoined device.
 */
void emberAfPluginDeviceTableRejoinDeviceCallback(const EmberEUI64 newNodeEui64,
                                                  uint8_t endpoint)
{
  z3gatewayTrustCenterDeviceEndpointDiscovered(newNodeEui64, endpoint);
}
