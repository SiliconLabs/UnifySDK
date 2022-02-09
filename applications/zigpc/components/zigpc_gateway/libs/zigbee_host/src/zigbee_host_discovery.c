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

#include "zigbee_host_common.h"

void zigbeeHostZdoActiveEndpointsResponseCallback(
  const EmberAfServiceDiscoveryResult *result)
{
  if (result == NULL) {
    return;
  } else if (!emberAfHaveDiscoveryResponseStatus(result->status)) {
    appDebugPrint("Error: service discovery returned no results\n");
    return;
  } else if (result->zdoRequestClusterId != ACTIVE_ENDPOINTS_REQUEST) {
    // Ignore non-active-endpoint based responses
    return;
  } else if (!ZIGBEE_HOST_CALLBACK_EXISTS(z3gwState.callbacks,
                                          onZdoActiveEndpointsResponse)) {
    // Ignore forming event if not registered
    appDebugPrint(LOG_FMTSTR_UNREGISTERED_CALLBACK,
                  "ZDO Active Endpoints response");
    return;
  }

  EmberEUI64 eui64;
  EmberStatus status = emberAfGetCurrentSenderEui64(eui64);
  if (status != EMBER_SUCCESS) {
    appDebugPrint(LOG_FMTSTR_SOURCE_EUI64_RES_FAIL, "active-endpoint response");
    return;
  }

  const EmberAfEndpointList *listStruct
    = (const EmberAfEndpointList *)(result->responseData);

  z3gwState.callbacks->onZdoActiveEndpointsResponse(eui64,
                                                    listStruct->count,
                                                    listStruct->list);
}

void zigbeeHostZdoSimpleDescriptorResponseCallback(
  const EmberAfServiceDiscoveryResult *result)
{
  if ((result == NULL) || (result->responseData == NULL)) {
    return;
  } else if (!emberAfHaveDiscoveryResponseStatus(result->status)) {
    appDebugPrint("Error: service discovery returned no results\n");
    return;
  } else if (result->zdoRequestClusterId != SIMPLE_DESCRIPTOR_REQUEST) {
    // Ignore non-active-endpoint based responses
    return;
  } else if (!ZIGBEE_HOST_CALLBACK_EXISTS(z3gwState.callbacks,
                                          onZdoSimpleDescriptorResponse)) {
    // Ignore forming event if not registered
    appDebugPrint(LOG_FMTSTR_UNREGISTERED_CALLBACK,
                  "ZDO Simple Descriptor response");
    return;
  }

  EmberEUI64 eui64;
  EmberStatus status = emberAfGetCurrentSenderEui64(eui64);
  if (status != EMBER_SUCCESS) {
    appDebugPrint(LOG_FMTSTR_SOURCE_EUI64_RES_FAIL, "active-endpoint response");
    return;
  }

  z3gwState.callbacks->onZdoSimpleDescriptorResponse(
    eui64,
    (const EmberAfClusterList *)result->responseData);
}

EmberStatus zigbeeHostZdoActiveEndpointsRequest(const EmberEUI64 eui64)
{
  if (eui64 == NULL) {
    return EMBER_BAD_ARGUMENT;
  }

  EmberNodeId nodeId;

  EmberStatus status = zigbeeHostGetAddressTableEntry(eui64, &nodeId);
  if (status != EMBER_SUCCESS) {
    appDebugPrint(LOG_FMTSTR_EUI64_NODEID_RES_FAIL,
                  "EUI64 to NodeId",
                  "endpoint discovery");
    return status;
  }

  return emberAfFindActiveEndpoints(
    nodeId,
    zigbeeHostZdoActiveEndpointsResponseCallback);
}

EmberStatus zigbeeHostZdoSimpleDescriptorRequest(const EmberEUI64 eui64,
                                                 uint8_t endpointId)
{
  if (eui64 == NULL) {
    return EMBER_BAD_ARGUMENT;
  }

  EmberNodeId nodeId;

  EmberStatus status = zigbeeHostGetAddressTableEntry(eui64, &nodeId);
  if (status != EMBER_SUCCESS) {
    appDebugPrint(LOG_FMTSTR_EUI64_NODEID_RES_FAIL,
                  "EUI64 to NodeId",
                  "cluster discovery");
    return status;
  }

  return emberAfFindClustersByDeviceAndEndpoint(
    nodeId,
    endpointId,
    zigbeeHostZdoSimpleDescriptorResponseCallback);
}
