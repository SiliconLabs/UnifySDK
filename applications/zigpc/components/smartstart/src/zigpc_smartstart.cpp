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

#include <string>
#include <cstring>

/* Shared component includes */
#include "sl_log.h"
#include "smartstart_management.hpp"

/* ZigPC includes */
#include <zigpc_ucl.hpp>
#include <zigpc_net_mgmt.h>
#include <zigpc_net_mgmt_notify.h>

#include "zigpc_smartstart_fixt.h"
#include "zigpc_smartstart_int.hpp"

constexpr char LOG_TAG[] = "zigpc_smartstart";

/**
 * @brief Key structure used by map to store DSKs used.
 *
 */
typedef struct dsk_key {
  zigbee_eui64_t eui64;
} dsk_key_t;

/**
 * @brief Comparison operator for DSK key structure
 *
 * @param lhs
 * @param rhs
 * @return true if EUI64 matches, false otherwise
 */
bool operator==(const dsk_key_t &lhs, const dsk_key_t &rhs)
{
  return !memcmp(lhs.eui64, rhs.eui64, sizeof(zigbee_eui64_t));
}

/**
 * @brief Hashing operator for DSK key structure
 *
 * @param key
 * @return hash of EUI64 string representation
 */
template<> struct std::hash<dsk_key_t> {
  std::size_t operator()(const dsk_key_t &key) const
  {
    std::array<char, ZIGBEE_EUI64_HEX_STR_LENGTH> eui64_cstr;

    zigbee_eui64_to_str(key.eui64,
                        eui64_cstr.data(),
                        ZIGBEE_EUI64_HEX_STR_LENGTH);

    return hash<string>()(eui64_cstr.data());
  }
};

/**
 * @brief Map of DSKs parsed and processed by ZigPC.
 *
 */
std::unordered_map<dsk_key_t, zigpc_common::dsk_install_code_t> //NOSONAR - this cannot be const as we need to add entries to it
  stored_dsk_entries;

void zigpc_smartstart_on_network_init(void *event_data)
{
  const struct zigpc_net_mgmt_on_network_init *net_init
    = (struct zigpc_net_mgmt_on_network_init *)event_data;

  std::string unid
    = zigpc_ucl::mqtt::build_unid(zigbee_eui64_to_uint(net_init->pc_eui64));
  if (!unid.empty()) {
    smartstart::Management::get_instance()->init(
      unid,
      zigpc_smartstart_on_list_update);
  }
}

void zigpc_smartstart_on_list_update(bool entries_pending_inclusion)
{
  if (entries_pending_inclusion == false) {
    sl_log_debug(LOG_TAG, "No more entries to include in the SmartStart list.");
    return;
  }

  smartstart::Query q = smartstart::Query(smartstart::QueryType::exact,
                                          smartstart::QueryKey::include,
                                          true);
  std::vector<smartstart::Entry> include_entries
    = smartstart::Management::get_instance()->get_cache_entries(q);

  size_t install_codes_added = 0U;

  for (smartstart::Entry entry: include_entries) {
    if (!entry.device_unid.empty()) {
      continue;
    }
    zigpc_common::dsk_install_code_t dsk_content;
    sl_status_t status
      = zigpc_smartstart_dsk_parse_install_code(entry.dsk, dsk_content);
    if (status == SL_STATUS_OK) {
      dsk_key_t key;
      memcpy(key.eui64, dsk_content.eui64, sizeof(zigbee_eui64_t));
      auto found_entry = stored_dsk_entries.find(key);

      if ((found_entry == stored_dsk_entries.end())
          || !(found_entry->second == dsk_content)) {
        stored_dsk_entries.insert({key, dsk_content});
        status = zigpc_net_mgmt_add_node(dsk_content.eui64,
                                         dsk_content.install_code,
                                         dsk_content.install_code_length);
        if (status != SL_STATUS_OK) {
          sl_log_warning(LOG_TAG,
                         "Failed to add DSK-based Install Code: 0x%X",
                         status);
        } else {
          ++install_codes_added;
        }
      }
      sl_log_debug(LOG_TAG, "Install code add status: 0x%X", status);
    }
  }

  if (install_codes_added > 0U) {
    sl_status_t status = zigpc_netmgmt_network_permit_joins(true);
    if (status != SL_STATUS_OK) {
      sl_log_warning(LOG_TAG, "Failed to open network: 0x%X", status);
    }
  }
}

void zigpc_smartstart_on_node_added(void *event_data)
{
  sl_status_t status = SL_STATUS_OK;
  const struct zigpc_net_mgmt_on_node_added *node_added
    = (struct zigpc_net_mgmt_on_node_added *)event_data;

  dsk_key_t key;
  memcpy(key.eui64, node_added->eui64, sizeof(zigbee_eui64_t));
  auto found_entry = stored_dsk_entries.find(key);
  if (found_entry == stored_dsk_entries.end()) {
    status = SL_STATUS_NOT_FOUND;
    sl_log_debug(LOG_TAG,
                 "Unable to find smartstart entry for node added: 0x%X",
                 status);
  }

  if (status == SL_STATUS_OK) {
    std::string unid
      = zigpc_ucl::mqtt::build_unid(zigbee_eui64_to_uint(node_added->eui64));
    if (unid.empty()) {
      status = SL_STATUS_OBJECT_WRITE;
    } else {
      status = smartstart::Management::get_instance()->notify_node_added(
        found_entry->second.dsk,
        unid);
    }
  }

  if (status == SL_STATUS_OK) {
    stored_dsk_entries.erase(key);
    if (stored_dsk_entries.empty()) {
      status = zigpc_netmgmt_network_permit_joins(false);
    }
  }

  sl_log_debug(LOG_TAG, "Node add event handler status: 0x%X", status);
}

void zigpc_smartstart_on_node_removed(void *event_data)
{
  sl_status_t status;
  const zigpc_net_mgmt_on_node_removed_t *node_removed
    = (zigpc_net_mgmt_on_node_removed_t *)event_data;

  std::string unid
    = zigpc_ucl::mqtt::build_unid(zigbee_eui64_to_uint(node_removed->eui64));
  if (unid.empty()) {
    status = SL_STATUS_OBJECT_WRITE;
  } else {
    status = smartstart::Management::get_instance()->notify_node_removed(unid);
  }

  sl_log_debug(LOG_TAG,
               "Status of smartstart list node remove complete: 0x%X",
               status);
}

sl_status_t zigpc_smartstart_init_observers(void)
{
  sl_status_t status;

  status = zigpc_net_mgmt_register_observer(ZIGPC_NET_MGMT_NOTIFY_NETWORK_INIT,
                                            zigpc_smartstart_on_network_init);

  if (status == SL_STATUS_OK) {
    status = zigpc_net_mgmt_register_observer(ZIGPC_NET_MGMT_NOTIFY_NODE_ADDED,
                                              zigpc_smartstart_on_node_added);
  }

  if (status == SL_STATUS_OK) {
    status
      = zigpc_net_mgmt_register_observer(ZIGPC_NET_MGMT_NOTIFY_NODE_REMOVED,
                                         zigpc_smartstart_on_node_removed);
  }

  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Network management observer registrations failed: 0x%X",
                 status);
  }

  return status;
}

sl_status_t zigpc_smartstart_setup(void)
{
  sl_status_t status;

  status = zigpc_smartstart_init_observers();

  return status;
}

int zigpc_smartstart_shutdown(void)
{
  smartstart::Management::get_instance()->teardown();
  return 0;
}
