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

extern struct zigbeeHostState z3gwState;

void zigbeeHostZdoActiveEndpointsResponseCallback(
  const sl_zigbee_af_service_discovery_result_t *result)
{
  if (result == NULL) {
    return;
  } else if (!sl_zigbee_af_have_discovery_response_status(result->status)) {
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

  sl_802154_long_addr_t eui64;
  sl_status_t status = sl_zigbee_af_get_current_sender_eui64(eui64);
  if (status != SL_STATUS_OK) {
    appDebugPrint(LOG_FMTSTR_SOURCE_EUI64_RES_FAIL, "active-endpoint response");
    return;
  }

  const sl_zigbee_af_endpoint_list_t *listStruct
    = (const sl_zigbee_af_endpoint_list_t *)(result->responseData);

  z3gwState.callbacks->onZdoActiveEndpointsResponse(eui64,
                                                    listStruct->count,
                                                    listStruct->list);
}

void zigbeeHostZdoSimpleDescriptorResponseCallback(
  const sl_zigbee_af_service_discovery_result_t *result)
{
  if ((result == NULL) || (result->responseData == NULL)) {
    return;
  } else if (!sl_zigbee_af_have_discovery_response_status(result->status)) {
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

  sl_802154_long_addr_t eui64;
  sl_status_t status = sl_zigbee_af_get_current_sender_eui64(eui64);
  if (status != SL_STATUS_OK) {
    appDebugPrint(LOG_FMTSTR_SOURCE_EUI64_RES_FAIL, "active-endpoint response");
    return;
  }

  z3gwState.callbacks->onZdoSimpleDescriptorResponse(
    eui64,
    (const sl_zigbee_af_cluster_list_t *)result->responseData);
}

sl_status_t zigbeeHostZdoActiveEndpointsRequest(const sl_802154_long_addr_t eui64)
{
  if (eui64 == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_802154_short_addr_t nodeId;

  sl_status_t status = zigbeeHostGetAddressTableEntry(eui64, &nodeId);
  if (status != SL_STATUS_OK) {
    appDebugPrint(LOG_FMTSTR_EUI64_NODEID_RES_FAIL,
                  "EUI64 to NodeId",
                  "endpoint discovery");
    return status;
  }

  return sl_zigbee_af_find_active_endpoints(
    nodeId,
    zigbeeHostZdoActiveEndpointsResponseCallback);
}

sl_status_t zigbeeHostZdoSimpleDescriptorRequest(const sl_802154_long_addr_t eui64,
                                                 uint8_t endpointId)
{
  if (eui64 == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_802154_short_addr_t nodeId;

  sl_status_t status = zigbeeHostGetAddressTableEntry(eui64, &nodeId);
  if (status != SL_STATUS_OK) {
    appDebugPrint(LOG_FMTSTR_EUI64_NODEID_RES_FAIL,
                  "EUI64 to NodeId",
                  "cluster discovery");
    return status;
  }

  return sl_zigbee_af_find_clusters_by_device_and_endpoint(
    nodeId,
    endpointId,
    zigbeeHostZdoSimpleDescriptorResponseCallback);
}
