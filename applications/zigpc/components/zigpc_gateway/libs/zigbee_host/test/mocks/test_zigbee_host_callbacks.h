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

#ifndef TEST_ZIGBEE_HOST_CALLBACKS_H
#define TEST_ZIGBEE_HOST_CALLBACKS_H

#include <stdbool.h>

#include "stack/include/sl_zigbee_types.h"

#include "zigbee_host.h"

/**
 * The following callbacks were made for mocking the `struct zigbeeHostCallbacks`
 * in zigbee_host.h
 */

void callback_onNetworkInitialized(const sl_zigbee_network_parameters_t *network);

void callback_onNetworkDeviceJoin(const sl_802154_long_addr_t eui64);

void callback_onTrustCenterDeviceJoinComplete(const sl_802154_long_addr_t eui64);

void callback_onDeviceEndpointsDiscovered(const sl_802154_long_addr_t eui64,
                                          uint8_t endpointCount,
                                          const uint8_t *endpointList);

void callback_onEndpointClustersDiscovered(
  const sl_802154_long_addr_t eui64, const sl_zigbee_af_cluster_list_t  *endpointInfo);

void callback_onNetworkDeviceLeaveResponse(const sl_802154_long_addr_t eui64);

sl_zigbee_af_status_t
  callback_onClusterCommandReceived(const sl_802154_long_addr_t eui64,
                                    uint8_t endpoint,
                                    uint16_t clusterId,
                                    uint8_t commandId,
                                    bool commandFromServerToClient,
                                    const uint8_t *buffer,
                                    uint16_t bufferLength,
                                    uint8_t bufferPayloadStartIndex);

void callback_onReportedAttributeChange(const sl_802154_long_addr_t eui64,
                                        uint8_t endpoint,
                                        uint16_t clusterId,
                                        uint8_t *attribute_report_records,
                                        uint16_t attribute_report_records_size);

void callback_onReadAttributesResponse(const sl_802154_long_addr_t eui64,
                                       uint8_t endpoint,
                                       uint16_t clusterId,
                                       uint8_t *attribute_status_records,
                                       uint16_t attribute_status_records_size);

void callback_onConfigureReportingResponse(
  const sl_802154_long_addr_t eui64,
  uint8_t endpoint,
  uint16_t clusterId,
  uint8_t *configure_status_records,
  uint16_t configure_status_records_size);

void callback_onOtaUpdateStarted(const sl_802154_long_addr_t eui64,
                                 uint16_t manufacturerId,
                                 uint16_t imageTypeId,
                                 uint32_t firmwareVersion);

void callback_onOtaUpdateCompleted(const sl_802154_long_addr_t eui64,
                                   uint16_t manufacturerId,
                                   uint16_t imageTypeId,
                                   uint32_t firmwareVersion,
                                   sl_zigbee_af_status_t status);

#endif  // TEST_ZIGBEE_HOST_CALLBACKS_H
