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
 * @defgroup zwave_tx_groups_internal Z-Wave Tx Group internals
 * @ingroup zwave_tx_groups
 * @brief Internal functions / class definitions for the Tx Group functionality
 *
 * This module comprises a class definition as well as some logic to create
 * and assign Group ID to a list of nodes.
 *
 * @{
 */

#ifndef ZWAVE_TX_GROUPS_INTERNAL_HPP
#define ZWAVE_TX_GROUPS_INTERNAL_HPP

// Generic includes
#include <set>

constexpr zwave_multicast_group_id_t MAXIMUM_ZWAVE_TX_GROUP_ID = 255;

/**
 * @brief Group data contained in a Z-Wave Tx Group.
 */
struct zwave_tx_group_t {
  /// GroupID handle for the group. User components will use this.
  zwave_multicast_group_id_t group_id;
  /// Node list for the group.
  zwave_nodemask_t node_list;
  /// Boolean indicating if the group can be modified by this component.
  bool locked;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Helps to sort groups, based on their zwave_multicast_group_id_t Group ID.
 */
struct zwave_tx_group_compare {
  bool operator()(const zwave_tx_group_t &lhs,
                  const zwave_tx_group_t &rhs) const
  {
    return lhs.group_id > rhs.group_id;
  }
};

// Our private list of groups
class zwave_tx_groups
{
  private:
  // List of assigned groups.
  std::set<zwave_tx_group_t, zwave_tx_group_compare> groups;
  // Variable to keep track of the next group to reuse
  zwave_multicast_group_id_t next_group_id_to_reuse = 1;

  /**
   * @brief Increments the next_group_id_to_reuse private variable and makes sure
   * to roll around when we reached MAXIMUM_ZWAVE_TX_GROUP_ID
   */
  void increment_next_group_id();

  /**
   * @brief Uses the next_group_id_to_reuse to re-use a already used group that is
   * not currently locked
   * @returns ZWAVE_TX_INVALID_GROUP if all groups are locked and none can be reused.
   */
  zwave_multicast_group_id_t get_next_group_to_reuse();

  /**
   * @brief Tells how suitable is a group reuse for a node list
   *
   * @param [in] nodes          List of nodes to fit into a group.
   * @param [in] group_nodes    List of nodes already present in a group
   * @param [out] common_nodes  The number of common nodes in both lists.
   * @returns SL_STATUS_NOT_AVAILABLE : This group must not be used for this
   *          node list.
   *          SL_STATUS_ALREADY_EXISTS: Group node list and node list are
   *          identical, so no need for any group modification
   *          SL_STATUS_OK: The group can be expanded to fit the node list.
   */
  sl_status_t analyze_group_suitability(const zwave_nodemask_t nodes,
                                        const zwave_nodemask_t group_nodes,
                                        uint16_t *common_nodes) const;
  /**
   * @brief Verify if a node list is empty
   *
   * @param [in] nodes          List of nodes to check.
   * @returns true if emtpy, false if at least 1 node is in the node mask
   */
  bool is_node_list_empty(const zwave_nodemask_t nodes) const;

  /**
   * @brief Finds the best group to use or reuse for a given node list
   *
   * @param nodes  List of nodes to fit into a group.
   * @returns The GroupID that is best to reuse for the node list. Returns
   *          ZWAVE_TX_INVALID_GROUP if no groups matches, and a brand new
   *          group should be created for this list.
   */
  zwave_multicast_group_id_t
    get_most_similar_subgroup(const zwave_nodemask_t nodes) const;

  /**
   * @brief Returns the group data for a group
   *
   * @param [out] group_data  Pointer to return group data.
   * @param [in] group_id     Group ID for which the data is to be returned
   * @returns SL_STATUS_OK if the data was written out to the group_data
   *          SL_STATUS_NOT_FOUND if the Group ID is not part of the group list.
   */
  sl_status_t get_group_data(zwave_tx_group_t *group_data,
                             zwave_multicast_group_id_t group_id) const;

  /**
   * @brief Adds this group into the group list. If the Group ID is already in use
   * update the existing group.
   *
   * @param new_group  Group object to add/update in the group list.
   * @returns SL_STATUS_OK
   */
  sl_status_t create_or_update_group(const zwave_tx_group_t &new_group);

  public:
  /**
   * @brief Class constructor
   */
  zwave_tx_groups();
  /**
   * @brief Class destructor
   */
  ~zwave_tx_groups();

  /**
   * @brief Forces to add a NodeID to a group.
   *
   * This should be used only to restore groups, at initialization time.
   *
   * @param node_id  NodeID to be added to the GroupID
   * @param group_id the Group ID to add the NodeID.
   *
   * @returns SL_STATUS_OK if the group was updated or created with the NodeID.
   *          SL_STATUS_FAIL otherwize
   */
  sl_status_t add_node_to_group(zwave_node_id_t node_id,
                                zwave_multicast_group_id_t group_id);

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
  sl_status_t assign_group(const zwave_nodemask_t nodes,
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
  sl_status_t get_nodes(zwave_nodemask_t nodes,
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
   * @returns SL_STATUS_OK if the group is locked. SL_STATUS_FAIL if there is an
   *          issue with the Group ID.
   */
  sl_status_t lock_group(zwave_multicast_group_id_t group_id);

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
   * @returns SL_STATUS_OK if the group is unlocked. SL_STATUS_FAIL if there is an
   *          error with the Group ID.
   */
  sl_status_t unlock_group(zwave_multicast_group_id_t group_id);

  /**
   * @brief Verifies if a group is locked.
   *
   * @param group_id The multicast group id to verify
   *
   * @returns true if the group is locked. false if the group does not exist or
   *          is unlocked.
   */
  bool is_group_locked(zwave_multicast_group_id_t group_id);

  /**
   * @brief Logs the content / state of the group using sl_log
   *
   * @param group_id The multicast group to log
  */
  void log(zwave_multicast_group_id_t group_id);
};

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_TX_GROUPS_INTERNAL_HPP
/** @} end zwave_tx_groups_internal */
