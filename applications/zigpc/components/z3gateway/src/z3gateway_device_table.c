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

#include <stdint.h>
#include <string.h>

#ifndef Z3GATEWAY_TEST
#include PLATFORM_HEADER
#include EMBER_AF_API_AF_HEADER
#include EMBER_AF_API_EMBER_TYPES
#include "app/framework/util/util.h"
#include "gen/callback.h"
#else /* Z3GATEWAY_TEST defined */
#include "debug-printing.h"
#endif

#include EMBER_AF_API_DEVICE_TABLE

#include "z3gateway.h"

static const size_t DEVICE_TABLE_TABLE_CAPCITY
  = EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE;
static const size_t DEVICE_TABLE_CLUSTERID_LIST_CAPACITY
  = EMBER_AF_PLUGIN_DEVICE_TABLE_CLUSTER_SIZE;

uint16_t z3gatewayDeviceTableCount(void)
{
  uint16_t tableSize                        = 0U;
  EmberAfPluginDeviceTableEntry *tableEntry = NULL;
  for (uint16_t i = 0; i < DEVICE_TABLE_TABLE_CAPCITY; i++) {
    /**
     * NOTE: assert on index < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE
     * is made in this function.
     */
    tableEntry = emberAfDeviceTableFindDeviceTableEntry(i);
    if (tableEntry == NULL) {
      // Size returned defaults to 0
      break;
    } else if (tableEntry->nodeId
               == EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID) {
      // set size to accommodate element until (but not including) this element
      tableSize = i;
      break;
    }
  }

  return tableSize;
}

size_t z3gatewayDeviceTableEntryClusterListCount(uint16_t entryIndex)
{
  size_t clusterCount                       = 0U;
  EmberAfPluginDeviceTableEntry *tableEntry = NULL;

  if (entryIndex < DEVICE_TABLE_TABLE_CAPCITY) {
    /**
     * NOTE: assert on index < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE
     * is made in this function.
     */
    tableEntry = emberAfDeviceTableFindDeviceTableEntry(entryIndex);
  }

  if (tableEntry != NULL) {
    while ((clusterCount < DEVICE_TABLE_CLUSTERID_LIST_CAPACITY)
           && (tableEntry->clusterIds[clusterCount] != ZCL_NULL_CLUSTER_ID)) {
      clusterCount++;
    }
  }

  return clusterCount;
}

EmberStatus z3gatewayDeviceTableClear(void)
{
  emberAfDeviceTableClear();
  return EMBER_SUCCESS;
}

EmberStatus z3gatewayDeviceTableImportEntry(
  const struct z3gatewayDeviceTableStorageEntry *const entry,
  const uint16_t entryIndex)
{
  EmberStatus status                        = EMBER_SUCCESS;
  EmberAfPluginDeviceTableEntry *tableEntry = NULL;

  if (entry == NULL) {
    status = EMBER_BAD_ARGUMENT;
  } else if (entryIndex >= DEVICE_TABLE_TABLE_CAPCITY) {
    status = EMBER_INDEX_OUT_OF_RANGE;
    emberAfCorePrintln("Error: Import index exceeds device-table size: 0x%X",
                       status);
  } else if (entry->clusterIdList == NULL) {
    status = EMBER_BAD_ARGUMENT;
    emberAfCorePrintln("Error: Invalid cluster list: 0x%X", status);
  } else if (entry->clusterIdCount > DEVICE_TABLE_CLUSTERID_LIST_CAPACITY) {
    status = EMBER_INDEX_OUT_OF_RANGE;
    emberAfCorePrintln("Error: Import cluster list exceeds device-table "
                       "capacity: 0x%X",
                       status);
  } else {
    /**
     * NOTE: assert on index < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE
     * is made in this function.
     */
    tableEntry = emberAfDeviceTableFindDeviceTableEntry(entryIndex);
    if (tableEntry == NULL) {
      status = EMBER_NOT_FOUND;
    }
  }

  if (status == EMBER_SUCCESS) {
    memcpy(tableEntry->eui64, entry->eui64, sizeof(uint8_t) * EUI64_SIZE);
    tableEntry->nodeId   = entry->nodeId;
    tableEntry->endpoint = entry->endpoint;
    tableEntry->deviceId = entry->deviceId;

    /**
     * NOTE: emberAfDeviceTableClear() should initialize rest of the cluster
     * IDs to ZCL_NULL_CLUSTER_ID
     */
    memcpy(tableEntry->clusterIds,
           entry->clusterIdList,
           sizeof(uint16_t) * entry->clusterIdCount);
    tableEntry->clusterOutStartPosition = entry->clusterIdClientStartIndex;
  }

  return status;
}

EmberStatus z3gatewayDeviceTableExportEntry(
  struct z3gatewayDeviceTableStorageEntry *const entry,
  const uint16_t entryIndex)
{
  EmberStatus status                        = EMBER_SUCCESS;
  EmberAfPluginDeviceTableEntry *tableEntry = NULL;
  size_t clusterCount                       = 0U;

  if (entry == NULL) {
    status = EMBER_BAD_ARGUMENT;
  } else if (entryIndex >= DEVICE_TABLE_TABLE_CAPCITY) {
    status = EMBER_INDEX_OUT_OF_RANGE;
    emberAfCorePrintln(
      "Error: Export entry index exceeds device-table size: 0x%X",
      status);
  } else if (entry->clusterIdList == NULL) {
    status = EMBER_BAD_ARGUMENT;
    emberAfCorePrintln("Error: Invalid cluster list: 0x%X", status);
  } else {
    /**
     * NOTE: assert on index < EMBER_AF_PLUGIN_DEVICE_TABLE_DEVICE_TABLE_SIZE
     * is made in this function.
     */
    tableEntry = emberAfDeviceTableFindDeviceTableEntry(entryIndex);
    if ((tableEntry == NULL)
        || (tableEntry->nodeId == EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_NODE_ID)) {
      status = EMBER_NOT_FOUND;
    }
  }

  if (status == EMBER_SUCCESS) {
    clusterCount = z3gatewayDeviceTableEntryClusterListCount(entryIndex);
    if (entry->clusterIdCount < clusterCount) {
      status = EMBER_INDEX_OUT_OF_RANGE;
      emberAfCorePrintln("Error: Export cluster list size insufficient: 0x%X",
                         status);
    }
  }

  if (status == EMBER_SUCCESS) {
    memcpy(entry->eui64, tableEntry->eui64, sizeof(uint8_t) * EUI64_SIZE);
    entry->nodeId                    = tableEntry->nodeId;
    entry->endpoint                  = tableEntry->endpoint;
    entry->deviceId                  = tableEntry->deviceId;
    entry->clusterIdClientStartIndex = tableEntry->clusterOutStartPosition;
    entry->clusterIdCount            = clusterCount;
    memcpy(entry->clusterIdList,
           tableEntry->clusterIds,
           sizeof(uint16_t) * clusterCount);
  }

  return status;
}
