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
#include "stack/include/sl_zigbee_types.h"

/*  Refer to protocol/zigbee/app/util/zigbee-framework/zigbee-device-common.h
    for more details.
*/

sl_status_t sl_zigbee_leave_request(sl_802154_short_addr_t target,
                              sl_802154_long_addr_t deviceAddress,
                              uint8_t leaveRequestFlags,
                              sl_zigbee_aps_option_t options);

sl_status_t sl_zigbee_bind_request(sl_802154_short_addr_t target,
                             sl_802154_long_addr_t source,
                             uint8_t sourceEndpoint,
                             uint16_t clusterId,
                             uint8_t type,
                             sl_802154_long_addr_t destination,
                             sl_zigbee_multicast_id_t groupAddress,
                             uint8_t destinationEndpoint,
                             sl_zigbee_aps_option_t options);

sl_status_t sl_zigbee_unbind_request(sl_802154_short_addr_t target,
                             sl_802154_long_addr_t source,
                             uint8_t sourceEndpoint,
                             uint16_t clusterId,
                             uint8_t type,
                             sl_802154_long_addr_t destination,
                             sl_zigbee_multicast_id_t groupAddress,
                             uint8_t destinationEndpoint,
                             sl_zigbee_aps_option_t options);

uint8_t sl_zigbee_get_last_zig_dev_request_sequence();
#endif  // SILABS_ZIGBEE_DEVICE_COMMON_H
