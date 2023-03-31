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

//The binding response does not contain cluster information
//Use the sequence number and buffer outgoing messages
#define MAX_BUFFER_SIZE 256

extern struct zigbeeHostState z3gwState;

typedef struct {
    EmberEUI64 sourceEui64;
    uint8_t sourceEndpoint;
    uint16_t clusterId;
    EmberEUI64 destEui64;
    uint8_t destEndpoint;
    bool isBindResponse;
} binding_entry_t;

static binding_entry_t binding_buffer[MAX_BUFFER_SIZE];

bool emberAfReportAttributesCallback(EmberAfClusterId clusterId,
                                     uint8_t *buffer,
                                     uint16_t bufLen)
{
  if (buffer == NULL) {
    return false;
  }

  EmberStatus status = EMBER_SUCCESS;
  EmberEUI64 sourceEui64 = {0};
  uint8_t sourceEndpoint = 0;

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
  EmberEUI64 sourceEui64 = {0};
  uint8_t sourceEndpoint = 0;
  uint8_t sourceCommandId = 0;

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
  EmberEUI64 sourceEui64 = {0};
  uint8_t sourceEndpoint = 0;
  uint8_t sourceCommandId = 0;

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

EmberStatus zigbeeHostInitBinding(const EmberEUI64 sourceEui64,
                                  uint8_t sourceEndpoint,
                                  uint16_t clusterId,
                                  uint16_t group_id,
                                  EmberEUI64 destEui64,
                                  uint8_t destEndpoint,
                                  bool isBindRequest)
{
  if ((sourceEui64 == NULL) || (destEui64 == NULL)) {
    return EMBER_BAD_ARGUMENT;
  }
  
  EmberNodeId targetNodeId = 0;

  // Get target NodeId
  EmberStatus status = zigbeeHostGetAddressTableEntry(sourceEui64, &targetNodeId);
  if (status != EMBER_SUCCESS) {
    status = EMBER_NOT_FOUND;
    appDebugPrint(LOG_FMTSTR_EUI64_NODEID_RES_FAIL,
                  "EUI64 to NodeId",
                  "Bind Request Send\n");
  } else {
    //Bind
    EmberApsOption options = EMBER_AF_DEFAULT_APS_OPTIONS;
    uint8_t type           = 0;

    if (group_id == 0) {
      type = UNICAST_BINDING;
    } else {
      type = MULTICAST_BINDING;
    }

    if(isBindRequest)
    {
        appDebugPrint("Sending zdo bind command\n");
        status = emberBindRequest(targetNodeId,
                     (uint8_t *)sourceEui64,
                     sourceEndpoint,
                     clusterId,
                     type,
                     destEui64,
                     group_id,
                     destEndpoint,
                     options);
    }
    else
    {
        appDebugPrint("Sending zdo unbind command\n");
        status = emberUnbindRequest(targetNodeId,
                     (uint8_t *)sourceEui64,
                     sourceEndpoint,
                     clusterId,
                     type,
                     destEui64,
                     group_id,
                     destEndpoint,
                     options);
    }

    //Need to store outgoing message info
    //Response does not contain all binding info
    //Therefore, need to keep track of transaction sequence
    
    //use the sequence number as the index in the buffer
    uint8_t index = emberGetLastAppZigDevRequestSequence(); 

    memcpy(binding_buffer[index].sourceEui64, sourceEui64, sizeof(EmberEUI64));
    memcpy(binding_buffer[index].destEui64, destEui64, sizeof(EmberEUI64));
    binding_buffer[index].sourceEndpoint = sourceEndpoint;
    binding_buffer[index].clusterId = clusterId;
    binding_buffer[index].destEndpoint = destEndpoint;
    binding_buffer[index].isBindResponse = isBindRequest;
        
    appDebugPrint("Storing binding at index: %d\n", index );
    
  }
  return status;
 }

void emAfZDOHandleBindingResponseCallback(EmberNodeId sender, EmberApsFrame* apsFrame, uint8_t* message, uint16_t length)
{
  (void)sender;
  (void)length;
  

  //only respond to bind and unbind response
  uint16_t clusterId = apsFrame->clusterId;
  if( BIND_RESPONSE == clusterId || UNBIND_RESPONSE == clusterId)
  {
    appDebugPrint("Received bind/unbind response\n");
    uint8_t index = emberGetLastAppZigDevRequestSequence();
    uint8_t status = message[1];
 
    //check to see if entry is in the binding buffer
    if( (binding_buffer[index].sourceEui64 != NULL) &&
        (binding_buffer[index].destEui64 != NULL))
    {
    
        appDebugPrint("Pushing bind/unbind response to zigpc_gateway\n");

        EmberEUI64 sourceEui64;
        EmberEUI64 destEui64;

        memcpy(sourceEui64, binding_buffer[index].sourceEui64, sizeof(EmberEUI64));
        memcpy(destEui64, binding_buffer[index].destEui64, sizeof(EmberEUI64));
    
        z3gwState.callbacks->onBindUnbindResponse(
                sourceEui64,
                binding_buffer[index].sourceEndpoint,
                binding_buffer[index].clusterId,
                destEui64,
                binding_buffer[index].destEndpoint,
                binding_buffer[index].isBindResponse,
                status);
    }
  }
}
