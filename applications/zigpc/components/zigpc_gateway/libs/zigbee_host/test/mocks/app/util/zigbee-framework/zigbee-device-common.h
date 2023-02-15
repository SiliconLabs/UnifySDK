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

#ifndef SILABS_ZIGBEE_DEVICE_COMMON_H
#define SILABS_ZIGBEE_DEVICE_COMMON_H

#include <stdbool.h>
#define SIGNED_ENUM
#include "stack/include/ember-types.h"

/*  Refer to protocol/zigbee/app/util/zigbee-framework/zigbee-device-common.h
    for more details.
*/

EmberStatus emberLeaveRequest(EmberNodeId target,
                              EmberEUI64 deviceAddress,
                              uint8_t leaveRequestFlags,
                              EmberApsOption options);

EmberStatus emberBindRequest(EmberNodeId target,
                             EmberEUI64 source,
                             uint8_t sourceEndpoint,
                             uint16_t clusterId,
                             uint8_t type,
                             EmberEUI64 destination,
                             EmberMulticastId groupAddress,
                             uint8_t destinationEndpoint,
                             EmberApsOption options);

EmberStatus emberUnbindRequest(EmberNodeId target,
                             EmberEUI64 source,
                             uint8_t sourceEndpoint,
                             uint16_t clusterId,
                             uint8_t type,
                             EmberEUI64 destination,
                             EmberMulticastId groupAddress,
                             uint8_t destinationEndpoint,
                             EmberApsOption options);

uint8_t emberGetLastAppZigDevRequestSequence();
#endif  // SILABS_ZIGBEE_DEVICE_COMMON_H
