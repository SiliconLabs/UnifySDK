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
#include "network_monitor.h"
#include "network_monitor_span_persistence.h"

// Unify Includes
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "sl_log.h"

// ZPC Includes
#include "zpc_attribute_store.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"

// Z-Wave Controller includes
#include "zwave_s2_nonce_management.h"
#include "zwave_tx_groups.h"
#include "zwave_network_management.h"

#define LOG_TAG "network_monitor_span_persistence"

void network_monitor_store_span_table_data()
{
  // Find all nodes in our network
  for (zwave_node_id_t node_id = ZW_MIN_NODE_ID; node_id <= ZW_LR_MAX_NODE_ID;
       node_id++) {
    if (!ZW_IS_NODE_IN_MASK(node_id,
                            network_monitor_get_cached_current_node_list())) {
      continue;
    }

    // Do not do anything for our own NodeID
    if (node_id == zwave_network_management_get_node_id()) {
      sl_log_debug(LOG_TAG,
                   "Skipping SPAN back-up for ZPC NodeID %d.",
                   node_id);
      continue;
    }

    span_entry_t span_data  = {};
    sl_status_t span_status = zwave_s2_get_span_data(node_id, &span_data);

    if (span_status != SL_STATUS_OK) {
      sl_log_debug(LOG_TAG, "No SPAN data for NodeID %d.", node_id);
      continue;
    }

    // Save the SPAN object in the attribute store.
    attribute_store_node_t node_id_node
      = attribute_store_network_helper_get_zwave_node_id_node(node_id);

    attribute_store_node_t span_node
      = attribute_store_get_first_child_by_type(node_id_node,
                                               ATTRIBUTE_S2_SPAN_ENTRY);

    if (span_node == ATTRIBUTE_STORE_INVALID_NODE) {
      span_node
        = attribute_store_add_node(ATTRIBUTE_S2_SPAN_ENTRY, node_id_node);
    }

    // Save this value.
    sl_log_debug(LOG_TAG, "Backing up SPAN data for NodeID %d.", node_id);
    attribute_store_set_reported(span_node, &span_data, sizeof(span_data));
  }
}

void network_monitor_restore_span_table_data()
{
  // Find all nodes in our network
  for (zwave_node_id_t node_id = ZW_MIN_NODE_ID; node_id <= ZW_LR_MAX_NODE_ID;
       node_id++) {
    if (!ZW_IS_NODE_IN_MASK(node_id,
                            network_monitor_get_cached_current_node_list())) {
      continue;
    }

    // Do not do anything for our own NodeID
    if (node_id == zwave_network_management_get_node_id()) {
      continue;
    }

    // Find the SPAN object in the attribute store.
    attribute_store_node_t node_id_node
      = attribute_store_network_helper_get_zwave_node_id_node(node_id);

    attribute_store_node_t span_node
      = attribute_store_get_first_child_by_type(node_id_node,
                                               ATTRIBUTE_S2_SPAN_ENTRY);

    span_entry_t span_data = {};
    sl_status_t span_status
      = attribute_store_get_reported(span_node, &span_data, sizeof(span_data));

    if (span_status != SL_STATUS_OK) {
      continue;
    }

    // Tell S2 about our data.
    sl_log_debug(LOG_TAG, "Restoring SPAN for NodeID %d", node_id);
    zwave_s2_set_span_table(node_id, &span_data);
  }
}

/**
 * @brief Saves a Multicast Group under each nodeID that are part of that group
 * @param group_id the Group ID to save.
 */
static void store_group_membership(zwave_multicast_group_id_t group_id)
{
  zwave_nodemask_t node_list = {};
  sl_status_t group_status   = zwave_tx_get_nodes(node_list, group_id);
  if (group_status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Cannot find NodeID list for group %d", group_id);
    return;
  }

  // We also need to save the group membership of each node.
  // Find all nodes in our network
  zwave_node_id_t zpc_node_id = zwave_network_management_get_node_id();
  for (zwave_node_id_t node_id = ZW_MIN_NODE_ID; node_id <= ZW_LR_MAX_NODE_ID;
       node_id++) {
    // Do not do anything for our own NodeID
    if (node_id == zpc_node_id) {
      continue;
    }

    attribute_store_node_t node_id_node
      = attribute_store_network_helper_get_zwave_node_id_node(node_id);

    attribute_store_node_t group_list_node
      = attribute_store_get_first_child_by_type(node_id_node,
                                               ATTRIBUTE_MULTICAST_GROUP_LIST);

    if (group_list_node == ATTRIBUTE_STORE_INVALID_NODE) {
      group_list_node = attribute_store_add_node(ATTRIBUTE_MULTICAST_GROUP_LIST,
                                                 node_id_node);
    }

    if (!ZW_IS_NODE_IN_MASK(node_id, node_list)) {
      // Make sure group membership is removed if not part of the group.
      attribute_store_delete_node(
        attribute_store_get_node_child_by_value(group_list_node,
                                                ATTRIBUTE_MULTICAST_GROUP,
                                                REPORTED_ATTRIBUTE,
                                                &group_id,
                                                sizeof(group_id),
                                                0));
    } else {
      // Make sure group membership is saved if part of the group.
      attribute_store_node_t group_node
        = attribute_store_get_node_child_by_value(group_list_node,
                                                  ATTRIBUTE_MULTICAST_GROUP,
                                                  REPORTED_ATTRIBUTE,
                                                  &group_id,
                                                  sizeof(group_id),
                                                  0);
      if (group_node == ATTRIBUTE_STORE_INVALID_NODE) {
        group_node = attribute_store_add_node(ATTRIBUTE_MULTICAST_GROUP,
                                              group_list_node);
        attribute_store_set_reported(group_node, &group_id, sizeof(group_id));
      }
    }
  }
}

void network_monitor_store_mpan_table_data()
{
  // Locate the MPAN table in the attribute store
  zwave_node_id_t zpc_node_id = zwave_network_management_get_node_id();
  attribute_store_node_t zpc_node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(zpc_node_id);

  attribute_store_node_t mpan_table_node
    = attribute_store_get_first_child_by_type(zpc_node_id_node,
                                             ATTRIBUTE_S2_MPAN_TABLE);
  if (mpan_table_node == ATTRIBUTE_STORE_INVALID_NODE) {
    mpan_table_node
      = attribute_store_add_node(ATTRIBUTE_S2_MPAN_TABLE, zpc_node_id_node);
  }

  // Wipe the previous MPAN data
  while (attribute_store_get_node_child_count(mpan_table_node)) {
    attribute_store_delete_node(
      attribute_store_get_node_child(mpan_table_node, 0));
  }

  // For all possibly existing groups
  for (zwave_multicast_group_id_t group_id = 1; group_id < 255; group_id++) {
    // Do we have an MPAN for this group?
    mpan_entry_t mpan_data  = {};
    sl_status_t mpan_status = zwave_s2_get_mpan_data(0, group_id, &mpan_data);

    if (mpan_status != SL_STATUS_OK) {
      continue;
    }

    // Save the MPAN object in the attribute store.
    sl_log_debug(LOG_TAG, "Backing up MPAN Group ID %d", group_id);
    attribute_store_node_t mpan_entry_node
      = attribute_store_add_node(ATTRIBUTE_S2_MPAN_ENTRY, mpan_table_node);
    attribute_store_set_reported(mpan_entry_node,
                                 &mpan_data,
                                 sizeof(mpan_data));

    // We also need to save NodeID group memberships
    store_group_membership(group_id);
  }
}

void network_monitor_restore_mpan_table_data()
{
  // Locate the MPAN table in the attribute store
  zwave_node_id_t zpc_node_id = zwave_network_management_get_node_id();
  attribute_store_node_t zpc_node_id_node
    = attribute_store_network_helper_get_zwave_node_id_node(zpc_node_id);

  attribute_store_node_t mpan_table_node
    = attribute_store_get_first_child_by_type(zpc_node_id_node,
                                             ATTRIBUTE_S2_MPAN_TABLE);
  // No MPAN table, we don't do anything
  if (mpan_table_node == ATTRIBUTE_STORE_INVALID_NODE) {
    sl_log_debug(LOG_TAG, "No MPAN table saved, skipping MPAN table reload");
    return;
  }

  size_t mpan_entry_index = 0;
  attribute_store_node_t mpan_entry_node
    = attribute_store_get_node_child_by_type(mpan_table_node,
                                             ATTRIBUTE_S2_MPAN_ENTRY,
                                             mpan_entry_index);
  mpan_entry_index += 1;
  while (mpan_entry_node != ATTRIBUTE_STORE_INVALID_NODE) {
    // Take the data from the entry and restore it
    mpan_entry_t mpan_entry = {};
    attribute_store_get_reported(mpan_entry_node,
                                 &mpan_entry,
                                 sizeof(mpan_entry));
    sl_log_debug(LOG_TAG,
                 "Restoring MPAN for Group ID %d",
                 mpan_entry.group_id);
    zwave_s2_set_mpan_data(0, mpan_entry.group_id, &mpan_entry);

    // Get the next entry.
    mpan_entry_node
      = attribute_store_get_node_child_by_type(mpan_table_node,
                                               ATTRIBUTE_S2_MPAN_ENTRY,
                                               mpan_entry_index);
    mpan_entry_index += 1;
  }

  // Now restore the Z-Wave Tx Groups
  for (zwave_node_id_t node_id = ZW_MIN_NODE_ID; node_id <= ZW_LR_MAX_NODE_ID;
       node_id++) {
    // Do not do anything for our own NodeID
    if (node_id == zpc_node_id) {
      continue;
    }

    // Get the group list for that node
    attribute_store_node_t node_id_node
      = attribute_store_network_helper_get_zwave_node_id_node(node_id);

    attribute_store_node_t group_list_node
      = attribute_store_get_first_child_by_type(node_id_node,
                                               ATTRIBUTE_MULTICAST_GROUP_LIST);

    size_t group_entry_index = 0;
    attribute_store_node_t group_node
      = attribute_store_get_node_child_by_type(group_list_node,
                                               ATTRIBUTE_MULTICAST_GROUP,
                                               group_entry_index);
    group_entry_index += 1;
    while (group_node != ATTRIBUTE_STORE_INVALID_NODE) {
      // Take the data from the entry and restore it
      zwave_multicast_group_id_t group_id = ZWAVE_TX_INVALID_GROUP;
      attribute_store_get_reported(group_node, &group_id, sizeof(group_id));
      zwave_tx_add_node_to_group(node_id, group_id);

      // Get the next entry.
      group_node
        = attribute_store_get_node_child_by_type(group_list_node,
                                                 ATTRIBUTE_MULTICAST_GROUP,
                                                 group_entry_index);
      group_entry_index += 1;
    }
  }
}