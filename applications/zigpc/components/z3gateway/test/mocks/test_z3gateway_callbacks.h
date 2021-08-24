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

#ifndef TEST_Z3GATEWAY_CALLBACKS_H
#define TEST_Z3GATEWAY_CALLBACKS_H

#include <stdbool.h>
#define SIGNED_ENUM
#include "stack/include/ember-types.h"

#include "z3gateway.h"

/**
 * The following callbacks were made for mocking the `struct z3gatewayCallbacks`
 * in z3gateway.h
 */

void callback_onTrustCenterInitialized(const EmberEUI64 eui64,
                                       const EmberNetworkParameters *network);

void callback_onTrustCenterDeviceJoinStart(const EmberEUI64 eui64);

void callback_onTrustCenterDeviceJoinComplete(const EmberEUI64 eui64);

void callback_onTrustCenterDeviceDiscovered(const EmberEUI64 eui64,
                                            uint8_t endpointCount);

void callback_onTrustCenterDeviceEndpointDiscovered(
  const EmberEUI64 eui64, const struct z3gatewayEndpointInfo *endpointInfo);

void callback_onTrustCenterDeviceRemoved(const EmberEUI64 eui64);

EmberAfStatus
  callback_onClusterCommandReceived(const EmberEUI64 eui64,
                                    uint8_t endpoint,
                                    uint16_t clusterId,
                                    uint8_t commandId,
                                    bool commandFromServerToClient,
                                    const uint8_t *buffer,
                                    uint16_t bufferLength,
                                    uint8_t bufferPayloadStartIndex);

#endif  // TEST_Z3GATEWAY_CALLBACKS_H
