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
#include "zwave_tx_groups.h"
#include "zwave_tx_groups_internal.hpp"

// Includes from other components
#include "zwave_controller_connection_info.h"
#include "zwave_controller_internal.h"

// Generic includes
#include <cstring>
#include <string>

#include "sl_log.h"
constexpr char LOG_TAG[] = "Z-Wave TX Groups";

///////////////////////////////////////////////////////////////////////////////
// Private functions
///////////////////////////////////////////////////////////////////////////////
void zwave_tx_groups::increment_next_group_id()
{
  if (this->next_group_id_to_reuse >= MAXIMUM_ZWAVE_TX_GROUP_ID) {
    this->next_group_id_to_reuse = 1;
  } else {
    this->next_group_id_to_reuse += 1;
  }
}

zwave_multicast_group_id_t zwave_tx_groups::get_next_group_to_reuse()
{
  //Remember where we start from
  zwave_multicast_group_id_t start_group_id = this->next_group_id_to_reuse;
  this->increment_next_group_id();

  while (this->next_group_id_to_reuse != start_group_id
         && this->is_group_locked(this->next_group_id_to_reuse) == true) {
    this->increment_next_group_id();
  }

  // Did we roll all around back to start_group_id ?
  if (this->next_group_id_to_reuse != start_group_id) {
    zwave_controller_on_multicast_group_deleted(this->next_group_id_to_reuse);
    return this->next_group_id_to_reuse;
  }
  return ZWAVE_TX_INVALID_GROUP;
}

bool zwave_tx_groups::is_node_list_empty(const zwave_nodemask_t nodes) const
{
  for (zwave_node_id_t n = ZW_MIN_NODE_ID; n <= ZW_LR_MAX_NODE_ID; n++) {
    if (ZW_IS_NODE_IN_MASK(n, nodes) == true) {
      return false;
    }
  }
  return true;
}

sl_status_t
  zwave_tx_groups::analyze_group_suitability(const zwave_nodemask_t list,
                                             const zwave_nodemask_t group_nodes,
                                             uint16_t *common_nodes) const
{
  uint16_t common_nodes_count = 0;
  uint16_t total_list_size    = 0;

  //TBD: is there something better than an iteration here ?
  for (zwave_node_id_t n = ZW_MIN_NODE_ID; n <= ZW_LR_MAX_NODE_ID; n++) {
    bool node_in_group = ZW_IS_NODE_IN_MASK(n, group_nodes);
    bool node_in_list  = ZW_IS_NODE_IN_MASK(n, list);
    if (node_in_list == true) {
      total_list_size += 1;
    }
    if (node_in_list && node_in_group) {
      common_nodes_count += 1;
    } else if (!node_in_list && node_in_group) {
      *common_nodes = 0;
      // Do not use this group for this list
      return SL_STATUS_NOT_AVAILABLE;
    }
  }

  *common_nodes = common_nodes_count;
  if (total_list_size == common_nodes_count) {
    // list and group list are identical.
    return SL_STATUS_ALREADY_EXISTS;
  } else {
    // It's okay to expand this group to match the list
    return SL_STATUS_OK;
  }
}

zwave_multicast_group_id_t
  zwave_tx_groups::get_most_similar_subgroup(const zwave_nodemask_t nodes) const
{
  uint16_t best_common_nodes                     = 0;
  zwave_multicast_group_id_t best_matching_group = ZWAVE_TX_INVALID_GROUP;

  for (auto it = this->groups.begin(); it != this->groups.end(); ++it) {
    // We do not edit or even reuse locked groups.
    if (it->locked == true) {
      continue;
    }

    uint16_t common_nodes         = 0;
    sl_status_t group_suitability = SL_STATUS_FAIL;
    group_suitability
      = this->analyze_group_suitability(nodes, it->node_list, &common_nodes);

    if (group_suitability == SL_STATUS_NOT_AVAILABLE) {
      continue;
    }

    if (group_suitability == SL_STATUS_ALREADY_EXISTS) {
      return it->group_id;
    }

    if (common_nodes > best_common_nodes) {
      best_common_nodes   = common_nodes;
      best_matching_group = it->group_id;
    }
  }
  return best_matching_group;
}

sl_status_t
  zwave_tx_groups::get_group_data(zwave_tx_group_t *group_data,
                                  zwave_multicast_group_id_t group_id) const
{
  zwave_tx_group_t group_to_find = {};
  group_to_find.group_id         = group_id;
  auto it                        = this->groups.find(group_to_find);

  if (it != this->groups.end()) {
    memcpy(group_data, &(*it), sizeof(zwave_tx_group_t));
    return SL_STATUS_OK;
  }

  return SL_STATUS_NOT_FOUND;
}

sl_status_t
  zwave_tx_groups::create_or_update_group(const zwave_tx_group_t &new_group)
{
  auto it = this->groups.find(new_group);
  if (it != this->groups.end()) {
    // Remove the old element, if the group is to be updated
    this->groups.erase(it);
  }

  // Insert the new group data.
  this->groups.insert(new_group);
#ifndef ZWAVE_BUILD_SYSTEM
  this->log(new_group.group_id);
#endif  // ZWAVE_BUILD_SYSTEM
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Public functions
///////////////////////////////////////////////////////////////////////////////
// Initialize the group_id counter on contruction
zwave_tx_groups::zwave_tx_groups(void) :
  next_group_id_to_reuse(ZWAVE_TX_INVALID_GROUP)
{}

// Nothing to do on destruction.
zwave_tx_groups::~zwave_tx_groups(void) = default;

sl_status_t
  zwave_tx_groups::add_node_to_group(zwave_node_id_t node_id,
                                     zwave_multicast_group_id_t group_id)
{
  sl_log_debug(LOG_TAG,
               "Manually adding NodeID %d to Group ID %d",
               node_id,
               group_id);

  // Bail out if the Group is locked.
  if (this->is_group_locked(group_id)) {
    sl_log_warning(LOG_TAG,
                   "GroupID %d is locked. NodeID %d will not be added.",
                   group_id,
                   node_id);
    return SL_STATUS_FAIL;
  }

  // Find the group.
  for (auto it = this->groups.begin(); it != this->groups.end(); ++it) {
    if (it->group_id == group_id) {
      zwave_tx_group_t updated_group = *it;
      ZW_ADD_NODE_TO_MASK(node_id, updated_group.node_list);
      return this->create_or_update_group(updated_group);
    }
  }

  // Group does not exist, create it
  zwave_tx_group_t new_group = {};
  new_group.locked           = false;
  new_group.group_id         = group_id;
  ZW_ADD_NODE_TO_MASK(node_id, new_group.node_list);
  return this->create_or_update_group(new_group);
}

sl_status_t zwave_tx_groups::assign_group(const zwave_nodemask_t nodes,
                                          zwave_multicast_group_id_t *group_id)
{
  if (this->is_node_list_empty(nodes) == true) {
    sl_log_warning(
      LOG_TAG,
      "Invalid TX group. You need at least 2 nodes to form a Tx group");
    *group_id = ZWAVE_TX_INVALID_GROUP;
    return SL_STATUS_FAIL;
  }
  zwave_tx_group_t assigned_group = {};
  memcpy(assigned_group.node_list, nodes, sizeof(zwave_nodemask_t));
  assigned_group.locked = false;
  // Find if we extend an existing group
  assigned_group.group_id = this->get_most_similar_subgroup(nodes);
  if (assigned_group.group_id == ZWAVE_TX_INVALID_GROUP) {
    // If not, assign a new ID:
    assigned_group.group_id = this->get_next_group_to_reuse();
  }

  *group_id = assigned_group.group_id;
  if (assigned_group.group_id == ZWAVE_TX_INVALID_GROUP) {
    return SL_STATUS_FAIL;
  }

  this->create_or_update_group(assigned_group);
  return SL_STATUS_OK;
}

sl_status_t zwave_tx_groups::get_nodes(zwave_nodemask_t nodes,
                                       zwave_multicast_group_id_t group_id)
{
  for (auto it = this->groups.begin(); it != this->groups.end(); ++it) {
    if (it->group_id == group_id) {
      memcpy(nodes, it->node_list, sizeof(zwave_nodemask_t));
      return SL_STATUS_OK;
    }
  }
  return SL_STATUS_NOT_FOUND;
}

sl_status_t zwave_tx_groups::lock_group(zwave_multicast_group_id_t group_id)
{
  zwave_tx_group_t group_to_find = {};
  group_to_find.group_id         = group_id;
  auto it                        = this->groups.find(group_to_find);

  if (it != this->groups.end()) {
    zwave_tx_group_t group = *it;
    group.locked           = true;
    this->groups.erase(it);
    this->groups.insert(group);
    return SL_STATUS_OK;
  }

  return SL_STATUS_NOT_FOUND;
}

sl_status_t zwave_tx_groups::unlock_group(zwave_multicast_group_id_t group_id)
{
  zwave_tx_group_t group_to_find = {};
  group_to_find.group_id         = group_id;
  auto it                        = this->groups.find(group_to_find);

  if (it != this->groups.end()) {
    zwave_tx_group_t group = *it;
    group.locked           = false;
    this->groups.erase(it);
    this->groups.insert(group);
    return SL_STATUS_OK;
  }

  return SL_STATUS_NOT_FOUND;
}

bool zwave_tx_groups::is_group_locked(zwave_multicast_group_id_t group_id)
{
  zwave_tx_group_t group_to_find = {};
  group_to_find.group_id         = group_id;
  auto it                        = this->groups.find(group_to_find);

  if (it != this->groups.end()) {
    return it->locked;
  }
  return false;
}

void zwave_tx_groups::log(zwave_multicast_group_id_t group_id)
{
  zwave_tx_group_t group_to_find = {};
  group_to_find.group_id         = group_id;
  auto it                        = this->groups.find(group_to_find);

  if (it != this->groups.end()) {
    std::string message = "Group ID " + std::to_string(it->group_id) + " ";
    if (it->locked) {
      message += "(R)";
    } else {
      message += "(RW)";
    }
    message += " - NodeIDs [";
    for (zwave_node_id_t n = ZW_MIN_NODE_ID; n <= ZW_LR_MAX_NODE_ID; n++) {
      if (ZW_IS_NODE_IN_MASK(n, it->node_list)) {
        message += std::to_string(n) + " ";
      }
    }
    message += "]";
    sl_log_debug(LOG_TAG, "%s", message.c_str());
    return;
  }

  sl_log_debug(LOG_TAG, "Group ID %d does not exist", group_id);
}
