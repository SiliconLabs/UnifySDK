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

// Generic includes
#include <string.h>

// The list of groups.
namespace tx_groups_variables
{
zwave_tx_groups tx_groups;
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_tx_assign_group(const zwave_nodemask_t nodes,
                                  zwave_multicast_group_id_t *group_id)
{
  return tx_groups_variables::tx_groups.assign_group(nodes, group_id);
}

sl_status_t zwave_tx_get_nodes(zwave_nodemask_t nodes,
                               zwave_multicast_group_id_t group_id)
{
  return tx_groups_variables::tx_groups.get_nodes(nodes, group_id);
}

sl_status_t zwave_tx_lock_group(zwave_multicast_group_id_t group_id)
{
  return tx_groups_variables::tx_groups.lock_group(group_id);
}

sl_status_t zwave_tx_unlock_group(zwave_multicast_group_id_t group_id)
{
  return tx_groups_variables::tx_groups.unlock_group(group_id);
}

bool zwave_tx_is_group_locked(zwave_multicast_group_id_t group_id)
{
  return tx_groups_variables::tx_groups.is_group_locked(group_id);
}

void zwave_tx_group_log(zwave_multicast_group_id_t group_id)
{
  return tx_groups_variables::tx_groups.log(group_id);
}