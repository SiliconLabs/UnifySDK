/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include "zigpc_gateway.h"
#include "sl_status.h"

#include "zigpc_gateway_int.h"
#include <unordered_map>

uint8_t zigpc_gateway_get_neighbor_count(){
  return zigbeeHostGetNeighborCount();
}

sl_status_t zigpc_gateway_get_neighbor_eui64(uint8_t index,zigbee_eui64_t eui64){
  sl_status_t status = SL_STATUS_OK;
  if (eui64 == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  }else{
    status = zigbeeHostGetNeighborEUI64(index, eui64);
  }
  return status;
}

