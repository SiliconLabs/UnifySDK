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

#ifndef Z3GATEWAY_TEST
#include PLATFORM_HEADER
#else /* Z3GATEWAY_TEST defined */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "debug-printing.h"
#endif /* Z3GATEWAY_TEST */

#include "z3gw.h"
#include EMBER_AF_API_AF_HEADER
#include EMBER_AF_API_DEVICE_TABLE
#include "app/framework/util/util.h"
#include "stack/include/ember-types.h"
#include "app/framework/include/af-types.h"

#include "z3gateway.h"
#include "z3gateway_common.h"

static const uint8_t Z3GATEWAY_DEFAULT_SRC_ENDPOINT
  = Z3GATEWAY_FRAME_SRC_ENDPOINT;

/**
 * Globals defined in CLI-related functions needed by Z3Gateway
 */
uint8_t emAfCliNetworkIndex = EMBER_AF_DEFAULT_NETWORK_INDEX;
EmberApsFrame globalApsFrame;
bool zclCmdIsBuilt = false;
uint16_t appZclBufferLen;
uint8_t appZclBuffer[ZCL_BUFFER_SIZE];

/**
 * @brief Setup source and destination endpoint for APS Frame
 *
 * NOTE: This function was copied from app/framework/cli/zcl-cli.c
 * in order to decouple Z3gateway from CLI functionality
 *
 * @param srcEndpoint source endpoint number
 * @param dstEndpoint destination endpoint number
 */
void emAfApsFrameEndpointSetup(uint8_t srcEndpoint, uint8_t dstEndpoint)
{
  globalApsFrame.sourceEndpoint
    = (srcEndpoint == 0 ? emberAfPrimaryEndpointForCurrentNetworkIndex()
                        : srcEndpoint);
  globalApsFrame.destinationEndpoint = dstEndpoint;
}

/**
 * END OF Globals defined in CLI-related functions needed by Z3Gateway
 */

EmberStatus z3gatewayFillZclFrame(const uint8_t *buffer,
                                  size_t bufferSize,
                                  size_t bufferSequenceIndex)
{
  EmberStatus status = EMBER_SUCCESS;

  if (buffer == NULL) {
    status = EMBER_BAD_ARGUMENT;
  } else if ((bufferSize < EMBER_AF_ZCL_OVERHEAD)
             || (bufferSize > ZCL_BUFFER_SIZE)) {
    status = EMBER_INDEX_OUT_OF_RANGE;
    emberAfCorePrintln("Error: Invalid ZCL buffer size: 0x%X", status);
  } else {
    memcpy(appZclBuffer, buffer, bufferSize);
    appZclBuffer[bufferSequenceIndex] = emberAfNextSequence();
    appZclBufferLen                   = bufferSize;
  }

  return status;
}

EmberStatus z3gatewaySendZclFrameUnicast(const EmberEUI64 eui64,
                                         uint8_t endpoint,
                                         uint16_t clusterId)
{
  EmberStatus status = EMBER_SUCCESS;
  static EmberApsFrame localApsFrame;
  EmberNodeId nodeId;

  if ((eui64 == NULL) || (clusterId == ZCL_NULL_CLUSTER_ID)) {
    status = EMBER_BAD_ARGUMENT;
    emberAfCorePrintln("Error: Invalid EUI64 or ZCL clusterId provided: 0x%X",
                       status);
  } else {
    nodeId = emberAfDeviceTableGetNodeIdFromEui64(eui64);
    if (nodeId == EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
      status = EMBER_NOT_FOUND;
      emberAfCorePrintln(
        "Error: Failed to find device to send ZCL buffer on the network: 0x%X",
        status);
    }
  }

  if (status == EMBER_SUCCESS) {
    localApsFrame.options             = EMBER_AF_DEFAULT_APS_OPTIONS;
    localApsFrame.clusterId           = clusterId;
    localApsFrame.sourceEndpoint      = Z3GATEWAY_DEFAULT_SRC_ENDPOINT;
    localApsFrame.destinationEndpoint = endpoint;

    status = emberAfSendUnicast(EMBER_OUTGOING_DIRECT,
                                nodeId,
                                &localApsFrame,
                                appZclBufferLen,
                                appZclBuffer);
    if (status != EMBER_SUCCESS) {
      emberAfCorePrintln(
        "Error: Failed to send ZCL buffer as unicast to destination: 0x%X",
        status);
    }
  }

  return status;
}

EmberStatus z3gatewaySendZclFrameMulticast(EmberMulticastId multicastId,
                                           uint16_t clusterId)
{
  EmberStatus status = EMBER_SUCCESS;
  static EmberApsFrame localApsFrame;

  if (clusterId == ZCL_NULL_CLUSTER_ID) {
    status = EMBER_BAD_ARGUMENT;
    emberAfCorePrintln("Error: Invalid ZCL clusterId provided: 0x%X", status);
  } else {
    localApsFrame.options        = EMBER_AF_DEFAULT_APS_OPTIONS;
    localApsFrame.sourceEndpoint = Z3GATEWAY_DEFAULT_SRC_ENDPOINT;
    localApsFrame.clusterId      = clusterId;

    status = emberAfSendMulticast(multicastId,
                                  &localApsFrame,
                                  appZclBufferLen,
                                  appZclBuffer);
    if (status != EMBER_SUCCESS) {
      emberAfCorePrintln(
        "Error: Failed to send ZCL buffer as multicast to groupID: 0x%X: 0x%X",
        multicastId,
        status);
    }
  }

  return status;
}
