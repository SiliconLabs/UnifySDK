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
    sl_802154_long_addr_t sourceEui64;
    uint8_t sourceEndpoint;
    uint16_t clusterId;
    sl_802154_long_addr_t destEui64;
    uint8_t destEndpoint;
    bool isBindResponse;
} binding_entry_t;

static binding_entry_t binding_buffer[MAX_BUFFER_SIZE];

bool emberAfReportAttributesCallback(sl_zigbee_af_cluster_id_t clusterId,
                                     uint8_t *buffer,
                                     uint16_t bufLen)
{
  if (buffer == NULL) {
    return false;
  }

  sl_status_t status = SL_STATUS_OK;
  sl_802154_long_addr_t sourceEui64 = {0};
  uint8_t sourceEndpoint = 0;

  if (!ZIGBEE_HOST_CALLBACK_EXISTS(z3gwState.callbacks,
                                   onReportedAttributeChange)) {
    appDebugPrint(LOG_FMTSTR_UNREGISTERED_CALLBACK,
                  "Attribute Reporting Response");
    status = SL_STATUS_NOT_FOUND;
  }

  if (status == SL_STATUS_OK) {
    status = sl_zigbee_af_get_current_sender_eui64(sourceEui64);
    if (status != SL_STATUS_OK) {
      status = SL_STATUS_NOT_FOUND;
      appDebugPrint(LOG_FMTSTR_SOURCE_EUI64_RES_FAIL,
                    "Attribute Reporting Response");
    }
  }

  if (status == SL_STATUS_OK) {
    sourceEndpoint = sl_zigbee_af_current_command()->apsFrame->sourceEndpoint;
  }

  if (status == SL_STATUS_OK) {
    appDebugPrint("Attribute report for cluster: %04X buffer: [", clusterId);
    appDebugPrintBuffer(buffer, bufLen, true);
    appDebugPrint("]\n");

    z3gwState.callbacks->onReportedAttributeChange(sourceEui64,
                                                   sourceEndpoint,
                                                   clusterId,
                                                   buffer,
                                                   bufLen);
  }

  bool success = (status == SL_STATUS_OK);
  return success;
}

bool emberAfReadAttributesResponseCallback(sl_zigbee_af_cluster_id_t clusterId,
                                           uint8_t *buffer,
                                           uint16_t bufLen)
{
  if (buffer == NULL) {
    return false;
  }

  sl_status_t status = SL_STATUS_OK;
  sl_802154_long_addr_t sourceEui64 = {0};
  uint8_t sourceEndpoint = 0;
  uint8_t sourceCommandId = 0;

  if (!ZIGBEE_HOST_CALLBACK_EXISTS(z3gwState.callbacks,
                                   onReadAttributesResponse)) {
    appDebugPrint(LOG_FMTSTR_UNREGISTERED_CALLBACK, "Read Attribute Response");
    status = SL_STATUS_NOT_FOUND;
  }

  if (status == SL_STATUS_OK) {
    status = sl_zigbee_af_get_current_sender_eui64(sourceEui64);
    if (status != SL_STATUS_OK) {
      status = SL_STATUS_NOT_FOUND;
      appDebugPrint(LOG_FMTSTR_SOURCE_EUI64_RES_FAIL,
                    "Read Attribute Response");
    }
  }

  if (status == SL_STATUS_OK) {
    const sl_zigbee_af_cluster_command_t *cmd = sl_zigbee_af_current_command();
    sourceCommandId                  = cmd->commandId;
    sourceEndpoint                   = cmd->apsFrame->sourceEndpoint;
  }

  if ((status == SL_STATUS_OK)
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

bool emberAfConfigureReportingResponseCallback(sl_zigbee_af_cluster_id_t clusterId,
                                               uint8_t *buffer,
                                               uint16_t bufLen)
{
  if (buffer == NULL) {
    return false;
  }

  sl_status_t status = SL_STATUS_OK;
  sl_802154_long_addr_t sourceEui64 = {0};
  uint8_t sourceEndpoint = 0;
  uint8_t sourceCommandId = 0;

  if (!ZIGBEE_HOST_CALLBACK_EXISTS(z3gwState.callbacks,
                                   onConfigureReportingResponse)) {
    appDebugPrint(LOG_FMTSTR_UNREGISTERED_CALLBACK,
                  "Configure Reporting Response");
    status = SL_STATUS_NOT_FOUND;
  }

  if (status == SL_STATUS_OK) {
    status = sl_zigbee_af_get_current_sender_eui64(sourceEui64);

    if (status != SL_STATUS_OK) {
      appDebugPrint(LOG_FMTSTR_SOURCE_EUI64_RES_FAIL,
                    "Configure Reporting Response");
    }
  }

  if (status == SL_STATUS_OK) {
    const sl_zigbee_af_cluster_command_t *cmd = sl_zigbee_af_current_command();
    sourceCommandId                  = cmd->commandId;
    sourceEndpoint                   = cmd->apsFrame->sourceEndpoint;
  }

  if ((status == SL_STATUS_OK)
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

sl_status_t zigbeeHostInitReporting(const sl_802154_long_addr_t eui64,
                                    uint8_t endpoint,
                                    uint16_t clusterId,
                                    const uint8_t *reportRecord,
                                    size_t recordSize)
{
  if ((eui64 == NULL) || (reportRecord == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  appDebugPrint("Sending Configure Report Frame: [");
  appDebugPrintBuffer(reportRecord, recordSize, true);
  appDebugPrint("]\n");

  // Assemble "ConfigureReporting" Command
  sl_zigbee_af_fill_external_buffer(
    (ZCL_GLOBAL_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER),
    clusterId,
    ZCL_CONFIGURE_REPORTING_COMMAND_ID,
    "b",
    reportRecord,
    recordSize);

  uint8_t gatewayEndpoint = zigbeeHostGetPrimaryEndpointId();
  sl_zigbee_af_set_command_endpoints(gatewayEndpoint, endpoint);

  sl_802154_long_addr_t eui64Dup;
  memcpy(eui64Dup, eui64, sizeof(sl_802154_long_addr_t));

  // Send command via unicast
  sl_status_t status = sl_zigbee_af_send_command_unicast_to_eui64(eui64Dup);

  return status;
}

/**
 * END OF Globals defined in CLI-related functions needed by ZigbeeHost
 */

sl_status_t zigbeeHostInitBinding(const sl_802154_long_addr_t sourceEui64,
                                  uint8_t sourceEndpoint,
                                  uint16_t clusterId,
                                  uint16_t group_id,
                                  sl_802154_long_addr_t destEui64,
                                  uint8_t destEndpoint,
                                  bool isBindRequest)
{
  if ((sourceEui64 == NULL) || (destEui64 == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  
  sl_802154_short_addr_t targetNodeId = 0;

  // Get target NodeId
  sl_status_t status = zigbeeHostGetAddressTableEntry(sourceEui64, &targetNodeId);
  if (status != SL_STATUS_OK) {
    status = SL_STATUS_NOT_FOUND;
    appDebugPrint(LOG_FMTSTR_EUI64_NODEID_RES_FAIL,
                  "EUI64 to NodeId",
                  "Bind Request Send\n");
  } else {
    //Bind
    sl_zigbee_aps_option_t options = SL_ZIGBEE_AF_DEFAULT_APS_OPTIONS ;
    uint8_t type           = 0;

    if (group_id == 0) {
      type = UNICAST_BINDING;
    } else {
      type = MULTICAST_BINDING;
    }

    if(isBindRequest)
    {
        appDebugPrint("Sending zdo bind command\n");
        status = sl_zigbee_bind_request(targetNodeId,
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
        status = sl_zigbee_unbind_request(targetNodeId,
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
    uint8_t index = sl_zigbee_get_last_zig_dev_request_sequence(); 

    memcpy(binding_buffer[index].sourceEui64, sourceEui64, sizeof(sl_802154_long_addr_t));
    memcpy(binding_buffer[index].destEui64, destEui64, sizeof(sl_802154_long_addr_t));
    binding_buffer[index].sourceEndpoint = sourceEndpoint;
    binding_buffer[index].clusterId = clusterId;
    binding_buffer[index].destEndpoint = destEndpoint;
    binding_buffer[index].isBindResponse = isBindRequest;
        
    appDebugPrint("Storing binding at index: %d\n", index );
    
  }
  return status;
 }

void emAfZDOHandleBindingResponseCallback(sl_802154_short_addr_t sender, sl_zigbee_aps_frame_t* apsFrame, uint8_t* message, uint16_t length)
{
  (void)sender;
  (void)length;
  

  //only respond to bind and unbind response
  uint16_t clusterId = apsFrame->clusterId;
  if( BIND_RESPONSE == clusterId || UNBIND_RESPONSE == clusterId)
  {
    appDebugPrint("Received bind/unbind response\n");
    uint8_t index = sl_zigbee_get_last_zig_dev_request_sequence();
    uint8_t status = message[1];
 
// TODO, may be reworked in GCC 13+
// Check if binding_buffer[index].sourceEui64 and binding_buffer[index].destEui64 can be NULL in any ways
#if defined(__GNUC__) && (__GNUC__ == 12)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress"
#endif
    //check to see if entry is in the binding buffer
    if( (binding_buffer[index].sourceEui64 != NULL) &&
        (binding_buffer[index].destEui64 != NULL))
    {
    
        appDebugPrint("Pushing bind/unbind response to zigpc_gateway\n");

        sl_802154_long_addr_t sourceEui64;
        sl_802154_long_addr_t destEui64;

        memcpy(sourceEui64, binding_buffer[index].sourceEui64, sizeof(sl_802154_long_addr_t));
        memcpy(destEui64, binding_buffer[index].destEui64, sizeof(sl_802154_long_addr_t));
    
        z3gwState.callbacks->onBindUnbindResponse(
                sourceEui64,
                binding_buffer[index].sourceEndpoint,
                binding_buffer[index].clusterId,
                destEui64,
                binding_buffer[index].destEndpoint,
                binding_buffer[index].isBindResponse,
                status);
    }
#if defined(__GNUC__) && (__GNUC__ == 12)
#pragma GCC diagnostic pop
#endif
  }
}
