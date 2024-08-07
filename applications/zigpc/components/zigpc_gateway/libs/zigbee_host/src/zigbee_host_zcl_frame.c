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
sl_zigbee_aps_frame_t globalApsFrame;
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

sl_status_t zigbeeHostFillZclFrame(const uint8_t *buffer,
                                   size_t bufferSize,
                                   size_t bufferSequenceIndex)
{
  sl_status_t status = SL_STATUS_OK;

  if (buffer == NULL) {
    status = SL_STATUS_INVALID_PARAMETER;
  } else if ((bufferSize < SL_ZIGBEE_AF_ZCL_OVERHEAD)
             || (bufferSize > ZCL_BUFFER_SIZE)) {
    status = SL_STATUS_INVALID_INDEX;
    appDebugPrint("Error: Invalid ZCL buffer size: 0x%X\n", status);
  } else {
    memcpy(zclMessageBuffer, buffer, bufferSize);
    zclMessageBuffer[bufferSequenceIndex] = sl_zigbee_af_next_sequence();
    zclMessageBufferLen                   = bufferSize;
  }

  return status;
}

sl_status_t zigbeeHostSendZclFrameUnicast(const sl_802154_long_addr_t eui64,
                                          uint8_t endpoint,
                                          uint16_t clusterId)
{
  sl_status_t status = SL_STATUS_OK;
  static sl_zigbee_aps_frame_t localApsFrame;

  if ((eui64 == NULL) || (clusterId == ZCL_NULL_CLUSTER_ID)) {
    status = SL_STATUS_INVALID_PARAMETER;
    appDebugPrint("Error: Invalid EUI64 or ZCL clusterId provided: 0x%X\n",
                  status);
  } else {
    uint8_t gatewayEndpoint = sl_zigbee_af_primary_endpoint_for_current_network_index();

    localApsFrame.options             = SL_ZIGBEE_AF_DEFAULT_APS_OPTIONS;
    localApsFrame.clusterId           = clusterId;
    localApsFrame.sourceEndpoint      = gatewayEndpoint;
    localApsFrame.destinationEndpoint = endpoint;

    sl_802154_long_addr_t eui64Dup;
    memcpy(eui64Dup, eui64, sizeof(sl_802154_long_addr_t));

    status = sl_zigbee_af_send_unicast_to_eui64(eui64Dup,
                                       &localApsFrame,
                                       zclMessageBufferLen,
                                       zclMessageBuffer);
    if (status == SL_STATUS_INVALID_STATE) {
      status = SL_STATUS_NOT_FOUND;
      appDebugPrint(LOG_FMTSTR_EUI64_NODEID_RES_FAIL,
                    "EUI64 to NodeId",
                    "Unicast ZCL Frame Send");

    } else if (status != SL_STATUS_OK) {
      appDebugPrint(
        "Error: Failed to send ZCL frame as unicast to destination: 0x%X\n",
        status);
    }
  }

  return status;
}

sl_status_t zigbeeHostSendZclFrameMulticast(sl_zigbee_multicast_id_t multicastId,
                                            uint16_t clusterId)
{
  sl_status_t status = SL_STATUS_OK;
  static sl_zigbee_aps_frame_t localApsFrame;
  sl_802154_short_addr_t alias = SL_ZIGBEE_NULL_NODE_ID;
  uint8_t nwkSeq = 0;

  if (clusterId == ZCL_NULL_CLUSTER_ID) {
    status = SL_STATUS_INVALID_PARAMETER;
    appDebugPrint("Error: Invalid ZCL clusterId provided: 0x%X\n", status);
  } else {
    uint8_t gatewayEndpoint = sl_zigbee_af_primary_endpoint_for_current_network_index();

    localApsFrame.options        = SL_ZIGBEE_AF_DEFAULT_APS_OPTIONS;
    localApsFrame.sourceEndpoint = gatewayEndpoint;
    localApsFrame.clusterId      = clusterId;

    status = sl_zigbee_af_send_multicast(multicastId,
                                  alias,
                                  nwkSeq,
                                  &localApsFrame,
                                  zclMessageBufferLen,
                                  zclMessageBuffer);
    if (status != SL_STATUS_OK) {
      appDebugPrint("Error: Failed to send ZCL buffer as multicast to groupID: "
                    "0x%X: 0x%X\n",
                    multicastId,
                    status);
    }
  }

  return status;
}
