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

/**
 * @defgroup zwave_tx_groups Z-Wave TX Groups
 * @ingroup components
 * @brief Assigns GroupIDs to a list of nodes.
 *
 * Group Identifiers are assigned to a set of nodes, and can be used to trigger
 * transmissions to the list of nodes.
 *
 * @{
 */

#ifndef ZWAVE_TX_GROUPS_H
#define ZWAVE_TX_GROUPS_H

#include "sl_status.h"

#include "zwave_controller_types.h"

/// Group ID returned in case of error
#define ZWAVE_TX_INVALID_GROUP ((zwave_multicast_group_id_t)0)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Assigns/returns a Group ID to the list of NodeIDs.
 *
 * The following checks are performed:
 * 1. Are the nodes all included with the same protocol
 * 2. Do the nodes share the same highest granted class
 * (S0/Non-secure are accepted)
 *
 * @param nodes  Nodemask for the list of nodes for which a GroupID is needed.
 * @param [out] group_id the Group ID assigned for the list of nodes.
 *
 * @returns SL_STATUS_OK if the group was created/found and returned in the
 *          group_id pointer. SL_STATUS_FAIL if the nodes
 *          cannot be part of the same group and the group_id variable is
 *          not to be used.
 */
sl_status_t zwave_tx_assign_group(const zwave_nodemask_t nodes,
                                  zwave_multicast_group_id_t *group_id);

/**
 * @brief Returns the list of NodeIDs part of a group.
 *
 * @param [out] nodes   Nodemask for the list of nodes that are part of a group
 * @param group_id      The Group ID for which we want the list of nodes.
 *
 * @returns SL_STATUS_OK if the list of nodes was copied to the nodes nodemask.
 *          SL_STATUS_NOT_FOUND if the GroupID is empty or does not exist.
 */
sl_status_t zwave_tx_get_nodes(zwave_nodemask_t nodes,
                               zwave_multicast_group_id_t group_id);

/**
 * @brief Instructs the Z-Wave TX Group not to alter the list of NodeIDs
 * part of a group, until the group is unlocked again.
 *
 * This function should be called when before transmitting the first Multicast
 * message. The unlock function should be called after the last follow up has
 * been sent out.
 *
 * @param group_id The multicast group to lock for changes.
 *
 * @returns SL_STATUS_OK if the group is locked. SL_STATUS_NOT_FOUND if there
 *          is an issue with the Group ID.
 */
sl_status_t zwave_tx_lock_group(zwave_multicast_group_id_t group_id);

/**
 * @brief Instructs the Z-Wave TX Group that the list of NodeIDs
 * part of a group can be modified.
 *
 * This function should be called when before transmitting the first Multicast
 * message. The unlock function should be called after the last follow up has
 * been sent out.
 *
 * @param group_id The multicast group to unlock and allow to be updated
 *
 * @returns SL_STATUS_OK if the group is unlocked. SL_STATUS_NOT_FOUND if
 *          there is an error with the Group ID.
 */
sl_status_t zwave_tx_unlock_group(zwave_multicast_group_id_t group_id);

/**
   * @brief Verifies if a group is locked.
   *
   * @param group_id The multicast group id to verify
   *
   * @returns true if the group is locked. false if the group does not exist or
   *          is unlocked.
   */
bool zwave_tx_is_group_locked(zwave_multicast_group_id_t group_id);

/**
   * @brief Logs the content / state of the group using sl_log
   *
   * @param group_id The multicast group to log
  */
void zwave_tx_group_log(zwave_multicast_group_id_t group_id);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_TX_GROUPS_H
/** @} end zwave_tx_groups */
