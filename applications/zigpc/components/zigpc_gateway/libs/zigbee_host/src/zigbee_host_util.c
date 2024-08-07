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

// stack includes
#include PLATFORM_HEADER
#include "app/framework/include/af.h"

// public includes
#include "zigbee_host.h"

void zigbeeHostGetEui64(sl_802154_long_addr_t eui64)
{
  return sl_zigbee_af_get_eui64(eui64);
}

uint8_t zigbeeHostGetPrimaryEndpointId(void)
{
  return sl_zigbee_af_primary_endpoint_for_current_network_index();
}
