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

#include "zigpc_common_zigbee.h"

#ifndef ZIGPC_NET_MGMT_EUI64_H
#define ZIGPC_NET_MGMT_EUI64_H
/**
 * @brief Used to locally store the zigpc protocol controller EUI64
 * 
 * @param eui64 
 * 
 */
void zigpc_net_mgmt_set_protocol_controller_eui64(const zigbee_eui64_t eui64);

#endif