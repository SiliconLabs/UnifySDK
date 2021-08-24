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

#include "zigpc_net_mgmt.h"
#include "zigpc_net_mgmt_eui64.h"

static zigbee_eui64_t protocol_controller_eui64 = {0};

void zigpc_net_mgmt_set_protocol_controller_eui64(const zigbee_eui64_t eui64)
{
  for (int i = 0; i < ZIGBEE_EUI64_SIZE; i++) {
    protocol_controller_eui64[i] = eui64[i];
  }
}

sl_status_t zigpc_net_mgmt_get_protocol_controller_eui64(zigbee_eui64_t eui64)
{
  sl_status_t status = SL_STATUS_OK;

  if (eui64 == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    for (int i = 0; i < ZIGBEE_EUI64_SIZE; i++) {
      eui64[i] = protocol_controller_eui64[i];
    }
  }
  return status;
}
