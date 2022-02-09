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
#include <app/framework/util/config.h>
#include <app/framework/include/af.h>
#include <app/framework/plugin/address-table/address-table.h>
#include <app/util/zigbee-framework/zigbee-device-common.h>

// SLC project includes
#include "zap-id.h"
#include "zigbee_zcl_callback_dispatcher.h"

#include "zigbee_host.h"
#include "zigbee_host_common.h"

bool emberAfReportAttributesCallback(EmberAfClusterId clusterId,
                                     uint8_t *buffer,
                                     uint16_t bufLen)
{
  if (buffer == NULL) {
    return false;
  }

  EmberStatus status = EMBER_SUCCESS;
  EmberEUI64 sourceEui64;
  uint8_t sourceEndpoint;

  if (!ZIGBEE_HOST_CALLBACK_EXISTS(z3gwState.callbacks,
                                   onReportedAttributeChange)) {
    appDebugPrint(LOG_FMTSTR_UNREGISTERED_CALLBACK,
                  "Attribute Reporting Response");
    status = EMBER_NOT_FOUND;
  }

  if (status == EMBER_SUCCESS) {
    status = emberAfGetCurrentSenderEui64(sourceEui64);
    if (status != EMBER_SUCCESS) {
      status = EMBER_NOT_FOUND;
      appDebugPrint(LOG_FMTSTR_SOURCE_EUI64_RES_FAIL,
                    "Attribute Reporting Response");
    }
  }

  if (status == EMBER_SUCCESS) {
    sourceEndpoint = emberAfCurrentCommand()->apsFrame->sourceEndpoint;
  }

  if (status == EMBER_SUCCESS) {
    appDebugPrint("Attribute report for cluster: %04X buffer: [", clusterId);
    appDebugPrintBuffer(buffer, bufLen, true);
    appDebugPrint("]\n");

    z3gwState.callbacks->onReportedAttributeChange(sourceEui64,
                                                   sourceEndpoint,
                                                   clusterId,
                                                   buffer,
                                                   bufLen);
  }

  bool success = (status == EMBER_SUCCESS);
  return success;
}

bool emberAfReadAttributesResponseCallback(EmberAfClusterId clusterId,
                                           uint8_t *buffer,
                                           uint16_t bufLen)
{
  if (buffer == NULL) {
    return false;
  }

  EmberStatus status = EMBER_SUCCESS;
  EmberEUI64 sourceEui64;
  uint8_t sourceEndpoint;
  uint8_t sourceCommandId;

  if (!ZIGBEE_HOST_CALLBACK_EXISTS(z3gwState.callbacks,
                                   onReadAttributesResponse)) {
    appDebugPrint(LOG_FMTSTR_UNREGISTERED_CALLBACK, "Read Attribute Response");
    status = EMBER_NOT_FOUND;
  }

  if (status == EMBER_SUCCESS) {
    status = emberAfGetCurrentSenderEui64(sourceEui64);
    if (status != EMBER_SUCCESS) {
      status = EMBER_NOT_FOUND;
      appDebugPrint(LOG_FMTSTR_SOURCE_EUI64_RES_FAIL,
                    "Read Attribute Response");
    }
  }

  if (status == EMBER_SUCCESS) {
    const EmberAfClusterCommand *cmd = emberAfCurrentCommand();
    sourceCommandId                  = cmd->commandId;
    sourceEndpoint                   = cmd->apsFrame->sourceEndpoint;
  }

  if ((status == EMBER_SUCCESS)
      && (sourceCommandId == ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID)) {
    appDebugPrint("Read attributes respond for cluster: %04X buffer: [",
                  clusterId);
    appDebugPrintBuffer(buffer, bufLen, true);
    appDebugPrint("]\n");

    z3gwState.callbacks->onReadAttributesResponse(sourceEui64,
                                                  sourceEndpoint,
                                                  clusterId,
                                                  buffer,
                                                  bufLen);
  }

  // Always return false to indicate that the message was not processed
  return false;
}

bool emberAfConfigureReportingResponseCallback(EmberAfClusterId clusterId,
                                               uint8_t *buffer,
                                               uint16_t bufLen)
{
  if (buffer == NULL) {
    return false;
  }

  EmberStatus status = EMBER_SUCCESS;
  EmberEUI64 sourceEui64;
  uint8_t sourceEndpoint;
  uint8_t sourceCommandId;

  if (!ZIGBEE_HOST_CALLBACK_EXISTS(z3gwState.callbacks,
                                   onConfigureReportingResponse)) {
    appDebugPrint(LOG_FMTSTR_UNREGISTERED_CALLBACK,
                  "Configure Reporting Response");
    status = EMBER_NOT_FOUND;
  }

  if (status == EMBER_SUCCESS) {
    status = emberAfGetCurrentSenderEui64(sourceEui64);

    if (status != EMBER_SUCCESS) {
      appDebugPrint(LOG_FMTSTR_SOURCE_EUI64_RES_FAIL,
                    "Configure Reporting Response");
    }
  }

  if (status == EMBER_SUCCESS) {
    const EmberAfClusterCommand *cmd = emberAfCurrentCommand();
    sourceCommandId                  = cmd->commandId;
    sourceEndpoint                   = cmd->apsFrame->sourceEndpoint;
  }

  if ((status == EMBER_SUCCESS)
      && (sourceCommandId == ZCL_CONFIGURE_REPORTING_RESPONSE_COMMAND_ID)) {
    appDebugPrint("Configure attributes response for cluster: %04X buffer: [",
                  clusterId);
    appDebugPrintBuffer(buffer, bufLen, true);
    appDebugPrint("]\n");

    z3gwState.callbacks->onConfigureReportingResponse(sourceEui64,
                                                      sourceEndpoint,
                                                      clusterId,
                                                      buffer,
                                                      bufLen);
  }

  // Always return false to indicate that the message was not processed
  return false;
}

EmberStatus zigbeeHostInitReporting(const EmberEUI64 eui64,
                                    uint8_t endpoint,
                                    uint16_t clusterId,
                                    const uint8_t *reportRecord,
                                    size_t recordSize)
{
  if ((eui64 == NULL) || (reportRecord == NULL)) {
    return EMBER_BAD_ARGUMENT;
  }

  appDebugPrint("Sending Configure Report Frame: [");
  appDebugPrintBuffer(reportRecord, recordSize, true);
  appDebugPrint("]\n");

  // Assemble "ConfigureReporting" Command
  emberAfFillExternalBuffer(
    (ZCL_GLOBAL_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER),
    clusterId,
    ZCL_CONFIGURE_REPORTING_COMMAND_ID,
    "b",
    reportRecord,
    recordSize);

  uint8_t gatewayEndpoint = zigbeeHostGetPrimaryEndpointId();
  emberAfSetCommandEndpoints(gatewayEndpoint, endpoint);

  EmberEUI64 eui64Dup;
  memcpy(eui64Dup, eui64, sizeof(EmberEUI64));

  // Send command via unicast
  EmberStatus status = emberAfSendCommandUnicastToEui64(eui64Dup);

  return status;
}

/**
 * END OF Globals defined in CLI-related functions needed by ZigbeeHost
 */

EmberStatus zigbeeHostInitBinding(const EmberEUI64 eui64,
                                  uint8_t endpoint,
                                  uint16_t clusterId,
                                  uint16_t group_id)
{
  if (eui64 == NULL) {
    return EMBER_BAD_ARGUMENT;
  }

  EmberEUI64 gatewayEui64;
  EmberNodeId targetNodeId;

  // Get our eui64
  zigbeeHostGetEui64(gatewayEui64);
  uint8_t gatewayEndpoint = emberAfPrimaryEndpointForCurrentNetworkIndex();

  // Get target NodeId
  EmberStatus status = zigbeeHostGetAddressTableEntry(eui64, &targetNodeId);
  if (status != EMBER_SUCCESS) {
    status = EMBER_NOT_FOUND;
    appDebugPrint(LOG_FMTSTR_EUI64_NODEID_RES_FAIL,
                  "EUI64 to NodeId",
                  "Bind Request Send");
  } else {
    //Bind
    EmberApsOption options = EMBER_AF_DEFAULT_APS_OPTIONS;
    uint8_t type           = 0;

    if (group_id == 0) {
      type = UNICAST_BINDING;
    } else {
      type = MULTICAST_BINDING;
    }

    emberBindRequest(targetNodeId,
                     (uint8_t *)eui64,
                     endpoint,
                     clusterId,
                     type,
                     gatewayEui64,
                     group_id,
                     gatewayEndpoint,
                     options);
  }

  return status;
}
