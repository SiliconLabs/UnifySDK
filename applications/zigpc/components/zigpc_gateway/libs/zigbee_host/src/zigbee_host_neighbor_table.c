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
// GCOVR_EXCL_START should be excluded from gcov coverage
#include PLATFORM_HEADER
#include "app/framework/include/af.h"
#include "app/util/ezsp/ezsp-protocol.h"
#include "app/util/ezsp/ezsp.h"

/* external includes */
#include "zigbee_host.h"


uint8_t zigbeeHostGetNeighborCount(){
    return sl_zigbee_ezsp_neighbor_count();
}

sl_status_t zigbeeHostGetNeighborEUI64(uint8_t index, sl_802154_long_addr_t eui64){
    sl_zigbee_neighbor_table_entry_t entry;
    sl_status_t status = sl_zigbee_ezsp_get_neighbor(index, &entry);
    memcpy(eui64, entry.longId, sizeof(uint8_t) * EUI64_SIZE);
    return status;
}
// GCOVR_EXCL_STOP