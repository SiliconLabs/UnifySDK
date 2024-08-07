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

#ifndef SILABS_AF_SECURITY_H
#define SILABS_AF_SECURITY_H

#include <stdbool.h>
#define SIGNED_ENUM
#include "stack/include/sl_zigbee_types.h"

/**
 * Refer to <GSDK>/protocol/zigbee/app/framework/plugin/network-creator-security/network-creator-security.h
 * for more details.
 */

/** @brief Sets the network to accept the install code only.
 *
 * This API will open a network for joining. It broadcasts a permit join to
 * the network and adds a specified EUI64 and transient key pair if this
 * device is a trust center. Only the node that matches the specified key pair
 * is allowed to join the network.
 *
 * @param eui64 EUI 64 of the joining node.
 *
 * @param keyData the link key to be used by the joining node.
 *
 * @return An ::sl_status_t value describing the success or failure of the
 * network opening procedure. If this node is not currently on a network,
 * this will return ::SL_STATUS_FAIL.
 */
sl_status_t emberAfPluginNetworkCreatorSecurityOpenNetworkWithKeyPair(
  sl_802154_long_addr_t eui64, sl_zigbee_key_data_t keyData);

#endif  // AF_SECURITY_H
