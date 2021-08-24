#include PLATFORM_HEADER
#include EMBER_AF_API_AF_HEADER
#include EMBER_AF_API_DEVICE_TABLE
#include EMBER_AF_API_DEVICE_TABLE_INTERNAL
#include "app/util/zigbee-framework/zigbee-device-common.h"
#include "gen/callback.h"
#include "z3gateway.h"
#include "z3gateway_common.h"

static const uint8_t Z3GATEWAY_DEFAULT_SRC_ENDPOINT
  = Z3GATEWAY_FRAME_SRC_ENDPOINT;

bool emberAfReportAttributesCallback(EmberAfClusterId clusterId,
                                     uint8_t *buffer,
                                     uint16_t bufLen)
{
  EmberStatus status = EMBER_SUCCESS;
  EmberEUI64 sourceEui64;
  EmberNodeId sourceNodeId;
  uint8_t sourceEndpoint;

  if (!Z3GATEWAY_CALLBACK_EXISTS(z3gwState.callbacks,
                                 onReportedAttributeChange)) {
    emberAfCorePrintln(
      "Ignoring reported attributes data due to missing callback");
    status = EMBER_NOT_FOUND;
  }

  if (status == EMBER_SUCCESS) {
    sourceNodeId = emberAfCurrentCommand()->source;

    bool euiStatus
      = emberAfDeviceTableGetEui64FromNodeId(sourceNodeId, sourceEui64);
    if (!euiStatus) {
      status = EMBER_NOT_FOUND;
      emberAfCorePrintln(
        "Error: Failed to find device eui64 in report attributes command");
    }
  }

  if (status == EMBER_SUCCESS) {
    sourceEndpoint = emberAfCurrentCommand()->apsFrame->sourceEndpoint;
  }

  if (status == EMBER_SUCCESS) {
    emberAfCorePrint("Attribute report for cluster: %04X buffer: [", clusterId);
    for (size_t i = 0; i < bufLen; i++) {
      emberAfCorePrint(" %02X", buffer[i]);
    }
    emberAfCorePrintln(" ]");

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
  EmberStatus status = EMBER_SUCCESS;
  EmberEUI64 sourceEui64;
  EmberNodeId sourceNodeId;
  uint8_t sourceEndpoint;
  uint8_t sourceCommandId;

  if (!Z3GATEWAY_CALLBACK_EXISTS(z3gwState.callbacks,
                                 onReadAttributesResponse)) {
    emberAfCorePrintln(
      "Ignoring read attribute response data due to missing callback");
    status = EMBER_NOT_FOUND;
  }

  if (status == EMBER_SUCCESS) {
    sourceNodeId = emberAfCurrentCommand()->source;

    bool euiStatus
      = emberAfDeviceTableGetEui64FromNodeId(sourceNodeId, sourceEui64);
    if (!euiStatus) {
      status = EMBER_NOT_FOUND;
      emberAfCorePrintln(
        "Error: Failed to find device eui64 in read attribute response");
    }
  }

  if (status == EMBER_SUCCESS) {
    sourceCommandId = emberAfCurrentCommand()->commandId;
    sourceEndpoint  = emberAfCurrentCommand()->apsFrame->sourceEndpoint;
  }

  if ((status == EMBER_SUCCESS)
      && (sourceCommandId == ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID)) {
    emberAfCorePrint("Read attributes respond for cluster: %04X buffer: [",
                     clusterId);
    for (size_t i = 0; i < bufLen; i++) {
      emberAfCorePrint(" %02X", buffer[i]);
    }
    emberAfCorePrintln(" ]");

    z3gwState.callbacks->onReadAttributesResponse(sourceEui64,
                                                  sourceEndpoint,
                                                  clusterId,
                                                  buffer,
                                                  bufLen);
  }
  // Always return false to indicate that the message was not processed
  return false;
}

EmberStatus z3gatewayInitReporting(const EmberEUI64 eui64,
                                   uint8_t endpoint,
                                   uint16_t clusterId,
                                   const uint8_t *reportRecord,
                                   size_t recordSize)
{
  EmberStatus status = EMBER_SUCCESS;

  //get our eui64
  EmberEUI64 ncpEui64;
  emberAfGetEui64(ncpEui64);

  //get target NodeId
  EmberNodeId targetNodeId
    = (EmberNodeId)emberAfDeviceTableGetNodeIdFromEui64(eui64);
  if (targetNodeId == EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
    status = EMBER_NOT_FOUND;
    emberAfCorePrintln(
      "Error: Failed to find device to send ZCL buffer on the network: 0x%X",
      status);
  }

  if (status == EMBER_SUCCESS) {

    //Assemble "ConfigureReporting" Command
    emberAfCorePrint("Sending Report Frame: [ ");
    for (size_t i = 0; i < recordSize; i++) {
      emberAfCorePrint("%02X ", reportRecord[i]);
    }
    emberAfCorePrintln("]");

    emberAfFillExternalBuffer(
      (ZCL_GLOBAL_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER),
      clusterId,
      ZCL_CONFIGURE_REPORTING_COMMAND_ID,
      "b",
      reportRecord,
      recordSize);

    emberAfSetCommandEndpoints(1, endpoint);

    //Send command via unicast
    status = emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, targetNodeId);
  }
  return status;
}

/**
 * END OF Globals defined in CLI-related functions needed by Z3Gateway
 */

EmberStatus z3gatewayInitBinding(const EmberEUI64 eui64,
                                 uint8_t endpoint,
                                 uint16_t clusterId,
                                 uint16_t group_id)
{
  EmberStatus status = EMBER_SUCCESS;

  //get our eui64
  EmberEUI64 ncpEui64;
  emberAfGetEui64(ncpEui64);

  //get target NodeId
  EmberNodeId targetNodeId
    = (EmberNodeId)emberAfDeviceTableGetNodeIdFromEui64(eui64);
  if (targetNodeId == EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
    status = EMBER_NOT_FOUND;
    emberAfCorePrintln(
      "Error: Failed to find device to send ZCL buffer on the network: 0x%X",
      status);
  }

  if (status == EMBER_SUCCESS) {
    //Bind
    EmberApsOption options = EMBER_AF_DEFAULT_APS_OPTIONS;
    uint8_t type = 0;

    if(group_id == 0)
    {
        type = UNICAST_BINDING;
    }
    else
    {
        type = MULTICAST_BINDING;
    }

    emberBindRequest(targetNodeId,
                     (uint8_t *)eui64,
                     endpoint,
                     clusterId,
                     type,
                     ncpEui64,
                     group_id,
                     Z3GATEWAY_DEFAULT_SRC_ENDPOINT,
                     options);
  }

  return status;
}
