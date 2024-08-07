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

#include PLATFORM_HEADER
#include "app/framework/include/af.h"
#include "app/framework/plugin/address-table/address-table.h"

#include "zigbee_host.h"

sl_status_t zigbeeHostAddAddressTableEntry(const sl_802154_long_addr_t eui64,
                                           const sl_802154_short_addr_t nodeId)
{
  sl_status_t status = SL_STATUS_OK;

  if (eui64 == NULL) {
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    sl_802154_long_addr_t eui64Dup;
    memcpy(eui64Dup, eui64, sizeof(sl_802154_long_addr_t));

    // NOTE: EUI64 passed in must be non-const
    uint8_t storedIndex = sl_zigbee_af_add_address_table_entry(eui64Dup, nodeId);
    if (storedIndex == SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX ) {
      status = SL_STATUS_INVALID_STATE;
    }
  }

  return status;
}

sl_status_t zigbeeHostGetAddressTableEntry(const sl_802154_long_addr_t eui64,
                                           sl_802154_short_addr_t *const nodeId)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t loadIndex  = SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX ;

  if ((eui64 == NULL) || (nodeId == NULL)) {
    status = SL_STATUS_INVALID_PARAMETER;
  } else {
    sl_802154_long_addr_t eui64Dup;
    memcpy(eui64Dup, eui64, sizeof(sl_802154_long_addr_t));

    // NOTE: EUI64 passed in must be non-const
    loadIndex = sl_zigbee_af_address_table_lookup_by_eui64(eui64Dup);
    if (loadIndex == SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX ) {
      status = SL_STATUS_INVALID_STATE; 
    }
  }

  if (status == SL_STATUS_OK) {
    // NOTE: nodeId will be set to SL_ZIGBEE_NULL_NODE_ID if this lookup fails
    *nodeId = sl_zigbee_af_address_table_lookup_node_id_by_index(loadIndex);
    if (*nodeId == SL_ZIGBEE_NULL_NODE_ID ) {
      *nodeId = SL_ZIGBEE_UNKNOWN_NODE_ID;
    }
  } else if (nodeId != NULL) {
    // All non-success results mean the lookup failed, nodeId set as invalid
    *nodeId = SL_ZIGBEE_UNKNOWN_NODE_ID;
  }

  return status;
}
