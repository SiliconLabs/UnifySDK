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
#include EMBER_AF_API_AF_HEADER
#include EMBER_AF_API_EMBER_TYPES
#include "app/framework/plugin/address-table/address-table.h"

#include "z3gateway.h"

EmberStatus z3gatewayAddAddressTableEntry(const EmberEUI64 eui64,
                                          const EmberNodeId nodeId)
{
  EmberStatus status = EMBER_SUCCESS;

  if (eui64 == NULL) {
    status = EMBER_BAD_ARGUMENT;
  } else {
    EmberEUI64 eui64Dup;
    memcpy(eui64Dup, eui64, sizeof(EmberEUI64));

    // NOTE: EUI64 passed in must be non-const
    uint8_t storedIndex = emberAfAddAddressTableEntry(eui64Dup, nodeId);
    if (storedIndex == EMBER_NULL_ADDRESS_TABLE_INDEX) {
      status = EMBER_INVALID_CALL;
    }
  }

  return status;
}

EmberStatus z3gatewayGetAddressTableEntry(const EmberEUI64 eui64,
                                          EmberNodeId *const nodeId)
{
  EmberStatus status = EMBER_SUCCESS;
  uint8_t loadIndex  = EMBER_NULL_ADDRESS_TABLE_INDEX;

  if ((eui64 == NULL) || (nodeId == NULL)) {
    status = EMBER_BAD_ARGUMENT;
  } else {
    EmberEUI64 eui64Dup;
    memcpy(eui64Dup, eui64, sizeof(EmberEUI64));

    // NOTE: EUI64 passed in must be non-const
    loadIndex = emberAfPluginAddressTableLookupByEui64(eui64Dup);
    if (loadIndex == EMBER_NULL_ADDRESS_TABLE_INDEX) {
      status = EMBER_INVALID_CALL;
    }
  }

  if (status == EMBER_SUCCESS) {
    // NOTE: nodeId will be set to EMBER_NULL_NODE_ID if this lookup fails
    *nodeId = emberAfPluginAddressTableLookupNodeIdByIndex(loadIndex);
    if (*nodeId == EMBER_NULL_NODE_ID) {
      *nodeId = EMBER_UNKNOWN_NODE_ID;
    }
  } else if (nodeId != NULL) {
    // All non-success results mean the lookup failed, nodeId set as invalid
    *nodeId = EMBER_UNKNOWN_NODE_ID;
  }

  return status;
}
