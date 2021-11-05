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

#include <new>
#include <string>

/* UIC shared includes */
#include <datastore.h>
#include <sl_log.h>
#include <sl_status.h>

// ZigPC includes
#include <zigpc_datastore.h>
#include <zigpc_datastore.hpp>

#include "zigpc_gateway_int.h"

static const char *DS_DEVICE_TABLE_SIZE_KEY
  = ZIGPC_GATEWAY_STORE_DEVICE_TABLE_SIZE_KEY;
static const char *DS_DEVICE_TABLE_ARR_KEY
  = ZIGPC_GATEWAY_STORE_DEVICE_TABLE_ARR_KEY;
static const char *DS_DEVICE_TABLE_CLUSTER_ARR_KEY
  = ZIGPC_GATEWAY_STORE_DEVICE_TABLE_CLUSTER_ARR_KEY;

constexpr char DS_NODEID_LIST_COUNT_KEY[] = ZIGPC_GATEWAY_NODEID_COUNT_KEY;
constexpr char DS_NODEID_LIST_KEY_KEY[]   = ZIGPC_GATEWAY_NODEID_LIST_KEY;

sl_status_t zigpc_gateway_load_device_table_info(void)
{
  EmberStatus z3gw_status = EMBER_SUCCESS;
  sl_status_t status      = SL_STATUS_OK;
  uint16_t entries_loaded = 0U;
  uint16_t table_size     = 0U;
  std::string store_key(DS_DEVICE_TABLE_SIZE_KEY);
  unsigned int store_value_size = static_cast<unsigned int>(sizeof(uint16_t));

  status = datastore_fetch_arr(store_key.c_str(),
                               reinterpret_cast<uint8_t *>(&table_size),
                               &store_value_size);
  if ((status != SL_STATUS_OK) && (status != SL_STATUS_NOT_FOUND)) {
    sl_log_error(LOG_TAG,
                 "Unable to fetch stored device table size: 0x%X",
                 status);
  } else if (status == SL_STATUS_NOT_FOUND) {
    table_size = 0U;
    status     = SL_STATUS_OK;
  } else if ((status == SL_STATUS_OK) && (table_size > 0)) {
    status = z3gatewayDeviceTableClear();
  }

  for (uint16_t i = 0U; (status == SL_STATUS_OK) && (i < table_size); i++) {
    struct z3gatewayDeviceTableStorageEntry entry;
    entry.clusterIdList = nullptr;

    store_key = DS_DEVICE_TABLE_ARR_KEY;
    store_key += "_" + std::to_string(i);
    store_value_size = sizeof(struct z3gatewayDeviceTableStorageEntry);
    status           = datastore_fetch_arr(store_key.c_str(),
                                 reinterpret_cast<uint8_t *>(&entry),
                                 &store_value_size);
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "Unable to fetch stored device table entry[%u]: 0x%X",
                   i,
                   status);
    } else {
      entry.clusterIdList = new (std::nothrow) uint16_t[entry.clusterIdCount];
      if (entry.clusterIdList == nullptr) {
        status = SL_STATUS_ALLOCATION_FAILED;
        sl_log_error(LOG_TAG,
                     "Unable to allocate cluster list for stored device table "
                     "entry[%u]: 0x%X",
                     i,
                     status);
      }
    }

    if (status == SL_STATUS_OK) {
      store_key = DS_DEVICE_TABLE_CLUSTER_ARR_KEY;
      store_key += "_" + std::to_string(i);
      store_value_size = sizeof(uint16_t) * entry.clusterIdCount;
      status
        = datastore_fetch_arr(store_key.c_str(),
                              reinterpret_cast<uint8_t *>(entry.clusterIdList),
                              &store_value_size);
      if (status != SL_STATUS_OK) {
        sl_log_error(
          LOG_TAG,
          "Unable to fetch stored device table entry cluster list[%u]: 0x%X",
          i,
          status);
      }
    }

    if (status == SL_STATUS_OK) {
      z3gw_status = z3gatewayDeviceTableImportEntry(&entry, i);
      if (z3gw_status != EMBER_SUCCESS) {
        sl_log_error(LOG_TAG,
                     "Unable to import stored device table entry[%u]: 0x%X",
                     i,
                     z3gw_status);
        status = SL_STATUS_OBJECT_WRITE;
      }
    }

    if (status == SL_STATUS_OK) {
      entries_loaded++;
    }

    if (entry.clusterIdList != nullptr) {
      delete[] entry.clusterIdList;
    }
  }

  sl_log_debug(LOG_TAG, "device table entries loaded: %u", entries_loaded);

  return status;
}

sl_status_t zigpc_gateway_persist_device_table_info(void)
{
  EmberStatus z3gw_status   = EMBER_SUCCESS;
  sl_status_t status        = SL_STATUS_OK;
  uint16_t entries_stored   = 0U;
  const uint16_t table_size = z3gatewayDeviceTableCount();
  std::string store_key;

  for (uint16_t i = 0U; (status == SL_STATUS_OK) && (i < table_size); i++) {
    struct z3gatewayDeviceTableStorageEntry entry;
    uint16_t *cluster_list = nullptr;
    entry.clusterIdCount   = z3gatewayDeviceTableEntryClusterListCount(i);
    cluster_list           = new (std::nothrow) uint16_t[entry.clusterIdCount];
    if (cluster_list == nullptr) {
      status = SL_STATUS_ALLOCATION_FAILED;
      sl_log_error(LOG_TAG,
                   "Unable to allocate cluster list for stored device table "
                   "entry[%u]: 0x%X",
                   i,
                   status);
    }

    if (status == SL_STATUS_OK) {
      entry.clusterIdList = cluster_list;
      z3gw_status         = z3gatewayDeviceTableExportEntry(&entry, i);
      if (z3gw_status != EMBER_SUCCESS) {
        sl_log_error(LOG_TAG,
                     "Unable to export device table entry[%u]: 0x%X",
                     i,
                     z3gw_status);
        status = SL_STATUS_OBJECT_READ;
      }
    }

    if (status == SL_STATUS_OK) {
      store_key = DS_DEVICE_TABLE_CLUSTER_ARR_KEY;
      store_key += "_" + std::to_string(i);
      status
        = datastore_store_arr(store_key.c_str(),
                              reinterpret_cast<uint8_t *>(entry.clusterIdList),
                              sizeof(uint16_t) * entry.clusterIdCount);
      if (status != SL_STATUS_OK) {
        sl_log_error(
          LOG_TAG,
          "Unable to store device table clusters for entry[%u]: 0x%X",
          i,
          status);
      }
    }

    if (status == SL_STATUS_OK) {
      store_key = DS_DEVICE_TABLE_ARR_KEY;
      store_key += "_" + std::to_string(i);
      status
        = datastore_store_arr(store_key.c_str(),
                              reinterpret_cast<uint8_t *>(&entry),
                              sizeof(struct z3gatewayDeviceTableStorageEntry));
      if (status != SL_STATUS_OK) {
        sl_log_error(LOG_TAG,
                     "Unable to store device table cluster entry[%u]: 0x%X",
                     i,
                     status);
      }
    }

    if (status == SL_STATUS_OK) {
      entries_stored++;
    }

    if (cluster_list != nullptr) {
      delete[] cluster_list;
    }
  }

  if (status == SL_STATUS_OK) {
    store_key = DS_DEVICE_TABLE_SIZE_KEY;
    status    = datastore_store_arr(store_key.c_str(),
                                 reinterpret_cast<uint8_t *>(&entries_stored),
                                 sizeof(uint16_t));
  }

  sl_log_debug(LOG_TAG, "device table entries stored: %u", entries_stored);

  return status;
}

sl_status_t zigpc_gateway_load_address_table_info(void)
{
  sl_status_t status = SL_STATUS_OK;

  // Load device-id mapping list count
  int64_t mapping_count = 0;
  status = datastore_fetch_int(DS_NODEID_LIST_COUNT_KEY, &mapping_count);
  if (status != SL_STATUS_OK) {
    mapping_count = 0;
    status        = SL_STATUS_OK;
    sl_log_debug(LOG_TAG,
                 "Unable to load nodeId mapping count, skipping load: 0x%X");
  }

  std::vector<zigpc_gateway_addr_entry_t> mappings(mapping_count);

  if ((status == SL_STATUS_OK) && (mapping_count > 0)) {
    // Load device-id mapping list
    unsigned int fetch_size = static_cast<unsigned int>(
      sizeof(zigpc_gateway_addr_entry_t) * mapping_count);
    status = datastore_fetch_arr(DS_NODEID_LIST_KEY_KEY,
                                 reinterpret_cast<uint8_t *>(mappings.data()),
                                 &fetch_size);
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG, "Unable to fetch nodeId mappings: 0x%X", status);
    }
  }

  for (size_t i = 0; (status == SL_STATUS_OK) && (i < mappings.size()); i++) {
    EmberStatus em_status
      = z3gatewayAddAddressTableEntry(mappings[i].eui64, mappings[i].node_id);
    if (em_status != EMBER_SUCCESS) {
      status = SL_STATUS_NO_MORE_RESOURCE;
      sl_log_error(LOG_TAG,
                   "EmberAf address-table full. NCP address-table capacity "
                   "should be increased: 0x%X",
                   status);
    }
  }

  return status;
}

sl_status_t zigpc_gateway_persist_address_table_info(void)
{
  sl_status_t status = SL_STATUS_OK;

  std::vector<zigpc_gateway_addr_entry_t> mappings;

  for (zigbee_eui64_uint_t eui64_i: zigpc_datastore::device::get_id_list()) {
    zigbee_eui64_t eui64;
    zigpc_gateway_addr_entry_t dev_id;

    status = zigbee_uint_to_eui64(eui64_i, eui64);
    if (status == SL_STATUS_OK) {
      zigbee_eui64_copy_switch_endian(dev_id.eui64, eui64);

      EmberStatus em_status
        = z3gatewayGetAddressTableEntry(dev_id.eui64, &dev_id.node_id);
      if (em_status != EMBER_SUCCESS) {
        dev_id.node_id = EMBER_UNKNOWN_NODE_ID;
        sl_log_warning(LOG_TAG,
                       "EmberAf address-table entry cannot be read, setting "
                       "nodeId to 0x%04X",
                       dev_id.node_id);
      }
    }

    sl_log_debug(LOG_TAG,
                 "Storing EUI64:%016" PRIX64 " to nodeId:0x%04X mapping",
                 eui64_i,
                 dev_id.node_id);

    // Persist NodeID even if it's set to EMBER_UNKNOWN_NODE_ID
    mappings.push_back(dev_id);
  }

  // Store new device-id mapping list count
  status = datastore_store_int(DS_NODEID_LIST_COUNT_KEY, mappings.size());
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Unable to store nodeId mapping count: 0x%X", status);
  }

  if ((status == SL_STATUS_OK) && !mappings.empty()) {
    // Store new device-id mapping list
    unsigned int store_size = static_cast<unsigned int>(
      sizeof(zigpc_gateway_addr_entry_t) * mappings.size());
    status = datastore_store_arr(DS_NODEID_LIST_KEY_KEY,
                                 reinterpret_cast<uint8_t *>(mappings.data()),
                                 store_size);
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG, "Unable to store nodeId mappings: 0x%X", status);
    }
  }

  return status;
}

sl_status_t zigpc_gateway_load_from_datastore(void)
{
  sl_status_t status = zigpc_gateway_load_device_table_info();
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Failed to load device-table info from datastore: 0x%X",
                 status);
  } else {
    status = zigpc_gateway_load_address_table_info();
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "Failed to load address-table info from datastore: 0x%X",
                   status);
    }
  }

  return status;
}

sl_status_t zigpc_gateway_persist_to_datastore(void)
{
  sl_status_t status = zigpc_gateway_persist_device_table_info();
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Failed to store device-table info to datastore: 0x%X",
                 status);
  } else {
    status = zigpc_gateway_persist_address_table_info();
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "Failed to store address-table info to datastore: 0x%X",
                   status);
    }
  }

  return status;
}
