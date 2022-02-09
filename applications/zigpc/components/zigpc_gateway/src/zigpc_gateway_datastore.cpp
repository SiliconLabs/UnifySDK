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

static constexpr char DS_NODEID_LIST_COUNT_KEY[]
  = ZIGPC_GATEWAY_NODEID_COUNT_KEY;
static constexpr char DS_NODEID_LIST_KEY_KEY[] = ZIGPC_GATEWAY_NODEID_LIST_KEY;

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
      = zigbeeHostAddAddressTableEntry(mappings[i].eui64, mappings[i].node_id);
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
        = zigbeeHostGetAddressTableEntry(dev_id.eui64, &dev_id.node_id);
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
  sl_status_t status = zigpc_gateway_load_address_table_info();
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Failed to load address-table info from datastore: 0x%X",
                 status);
  }

  return status;
}

sl_status_t zigpc_gateway_persist_to_datastore(void)
{
  sl_status_t status = zigpc_gateway_persist_address_table_info();
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Failed to store address-table info to datastore: 0x%X",
                 status);
  }
  return status;
}
