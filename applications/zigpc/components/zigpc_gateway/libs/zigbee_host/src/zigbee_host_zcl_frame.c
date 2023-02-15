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
#include "app/framework/util/config.h"
#include "app/framework/plugin/address-table/address-table.h"

#include "zigbee_host.h"
#include "zigbee_host_common.h"

/**
 * Globals defined in CLI-related functions needed by ZigbeeHost
 */
/*
EmberApsFrame globalApsFrame;
bool zclCmdIsBuilt = false;
*/
uint16_t zclMessageBufferLen;
uint8_t zclMessageBuffer[ZCL_BUFFER_SIZE];
/**
 * @brief Setup source and destination endpoint for APS Frame
 *
 * NOTE: This function was copied from app/framework/cli/zcl-cli.c
 * in order to decouple ZigbeeHost from CLI functionality
 *
 * @param srcEndpoint source endpoint number
 * @param dstEndpoint destination endpoint number
 */
/*void emAfApsFrameEndpointSetup(uint8_t srcEndpoint, uint8_t dstEndpoint)
{
  globalApsFrame.sourceEndpoint
    = (srcEndpoint == 0 ? emberAfPrimaryEndpointForCurrentNetworkIndex()
                        : srcEndpoint);
  globalApsFrame.destinationEndpoint = dstEndpoint;
}*/

/**
 * END OF Globals defined in CLI-related functions needed by ZigbeeHost
 */

EmberStatus zigbeeHostFillZclFrame(const uint8_t *buffer,
                                   size_t bufferSize,
                                   size_t bufferSequenceIndex)
{
  EmberStatus status = EMBER_SUCCESS;

  if (buffer == NULL) {
    status = EMBER_BAD_ARGUMENT;
  } else if ((bufferSize < EMBER_AF_ZCL_OVERHEAD)
             || (bufferSize > ZCL_BUFFER_SIZE)) {
    status = EMBER_INDEX_OUT_OF_RANGE;
    appDebugPrint("Error: Invalid ZCL buffer size: 0x%X\n", status);
  } else {
    memcpy(zclMessageBuffer, buffer, bufferSize);
    zclMessageBuffer[bufferSequenceIndex] = emberAfNextSequence();
    zclMessageBufferLen                   = bufferSize;
  }

  return status;
}

EmberStatus zigbeeHostSendZclFrameUnicast(const EmberEUI64 eui64,
                                          uint8_t endpoint,
                                          uint16_t clusterId)
{
  EmberStatus status = EMBER_SUCCESS;
  static EmberApsFrame localApsFrame;

  if ((eui64 == NULL) || (clusterId == ZCL_NULL_CLUSTER_ID)) {
    status = EMBER_BAD_ARGUMENT;
    appDebugPrint("Error: Invalid EUI64 or ZCL clusterId provided: 0x%X\n",
                  status);
  } else {
    uint8_t gatewayEndpoint = emberAfPrimaryEndpointForCurrentNetworkIndex();

    localApsFrame.options             = EMBER_AF_DEFAULT_APS_OPTIONS;
    localApsFrame.clusterId           = clusterId;
    localApsFrame.sourceEndpoint      = gatewayEndpoint;
    localApsFrame.destinationEndpoint = endpoint;

    EmberEUI64 eui64Dup;
    memcpy(eui64Dup, eui64, sizeof(EmberEUI64));

    status = emberAfSendUnicastToEui64(eui64Dup,
                                       &localApsFrame,
                                       zclMessageBufferLen,
                                       zclMessageBuffer);
    if (status == EMBER_INVALID_CALL) {
      status = EMBER_NOT_FOUND;
      appDebugPrint(LOG_FMTSTR_EUI64_NODEID_RES_FAIL,
                    "EUI64 to NodeId",
                    "Unicast ZCL Frame Send");

    } else if (status != EMBER_SUCCESS) {
      appDebugPrint(
        "Error: Failed to send ZCL frame as unicast to destination: 0x%X\n",
        status);
    }
  }

  return status;
}

EmberStatus zigbeeHostSendZclFrameMulticast(EmberMulticastId multicastId,
                                            uint16_t clusterId)
{
  EmberStatus status = EMBER_SUCCESS;
  static EmberApsFrame localApsFrame;

  if (clusterId == ZCL_NULL_CLUSTER_ID) {
    status = EMBER_BAD_ARGUMENT;
    appDebugPrint("Error: Invalid ZCL clusterId provided: 0x%X\n", status);
  } else {
    uint8_t gatewayEndpoint = emberAfPrimaryEndpointForCurrentNetworkIndex();

    localApsFrame.options        = EMBER_AF_DEFAULT_APS_OPTIONS;
    localApsFrame.sourceEndpoint = gatewayEndpoint;
    localApsFrame.clusterId      = clusterId;

    status = emberAfSendMulticast(multicastId,
                                  &localApsFrame,
                                  zclMessageBufferLen,
                                  zclMessageBuffer);
    if (status != EMBER_SUCCESS) {
      appDebugPrint("Error: Failed to send ZCL buffer as multicast to groupID: "
                    "0x%X: 0x%X\n",
                    multicastId,
                    status);
    }
  }

  return status;
}
