/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file group_map.h
 * @defgroup zigpc_group_map ZigPC Group Map functions
 * @ingroup zigpc_group_mgmt
 * @brief This header contains the definitions and functionality associated with
 * the Zigpc Group Map. This is the internal mechanism for storing and manipulating
 * groups data.
 *
 * @{
 */

#ifndef ZIGPC_GROUP_MAP_H
#define ZIGPC_GROUP_MAP_H

#include "sl_status.h"
#include "zigpc_common_zigbee.h"
#include "zigpc_group_mgmt.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief zigpc_group_map_add_new
 * Add a member to a zigbee group in the map.
 *
 * @param group         - the group to add a new member to
 * @param member        - the new member to add
 * @param is_reported   - the direction (REPORTED/DESIRED) of the operation
 *
 * @return SL_STATUS_OK if able to add the group/member properly
 */
sl_status_t zigpc_group_map_add_new(zigbee_group_id_t group,
                                    zigpc_group_member_t member,
                                    bool is_reported);

/**
 * @brief zigpc_group_map_add_new_with_name
 * Add a member to a zigbee group in the map, with a name.
 *
 * @param group         - the group to add a new member to
 * @param member        - the new member to add
 * @param group_name    - the name of the group
 * @param is_reported   - the direction (REPORTED/DESIRED) of the operation
 *
 * @return SL_STATUS_OK if able to add the group/member properly
 */
sl_status_t zigpc_group_map_add_new_with_name(zigbee_group_id_t group,
                                              const char *group_name,
                                              zigpc_group_member_t member,
                                              bool is_reported);

/**
 * @brief zigpc_group_map_retrieve_num
 * Retrieve the number of groups associated with a member
 *
 * @param member        - the group member
 * @param is_reported   - the direction to retrieve
 *
 * @return the size of the grouplist of a given member.
 * 0 if unable to retrieve the grouplist or the grouplist
 * is empty
 */
unsigned int zigpc_group_map_retrieve_num(zigpc_group_member_t member,
                                          bool is_reported);

/**
 * @brief zigpc_group_map_retrieve_grouplist
 * Retrieve the group list of a given member
 *
 * @param member        - the endpoit/eui64 combo associated with a number of groups
 * @param group_list    - the destination to store the list of groups
 * @param num_groups    - the size of the group_list
 * @param is_reported   - the direction of the group_list desired (reported/desired)
 *
 * @return SL_STATUS_OK if able to retrive the group_list, SL_STATUS_FAIL if an error occured
 * or the group_list does not exist
 */
sl_status_t zigpc_group_map_retrieve_grouplist(zigpc_group_member_t member,
                                               zigbee_group_id_t *group_list,
                                               unsigned int num_groups,
                                               bool is_reported);

/**
 * @brief zigpc_group_map_remove_group
 * Remove the given member from its associated group
 *
 * @param group - the group selected
 * @param member- the member to remove from the group
 * @param is_reported - the direction of the group
 *
 * @return SL_STATUS_OK if able to successfully remove the group. NOTE
 * this will return OK if the member exists but was not part of the group
 * in the first place
 */
sl_status_t zigpc_group_map_remove_group(zigbee_group_id_t group,
                                         zigpc_group_member_t member,
                                         bool is_reported);

/**
 * @brief zigpc_group_map_remove_all
 * Remove all associated groups from a given member
 *
 * @param member - eui64/endpoint combo associated with a group
 *
 * @return SL_STATUS_OK if able to remove the member from all its associated groups
 */
sl_status_t zigpc_group_map_remove_all(zigpc_group_member_t member);

/**
 * @brief Retrieve the size of the member(EUI64/EP pair) list corresponding
 * to a group_id.
 *
 * @param group_id  Group ID to lookup.
 * @return size_t 0U if the member list is not found or the number of members
 * corresponding to that Group ID.
 */
size_t zigpc_group_map_get_memberlist_count(zigbee_group_id_t group_id);

/**
 * @brief Retrieve the member(EUI64/EP pair) list corresponding
 * to a group_id via the provided list pointer.
 *
 * @param group_id          Group ID to lookup.
 * @param member_list       Pointer to member list to populate.
 * @param member_list_count Capacity of the provided list.
 * @return sl_status_t SL_STATUS_OK if the list is copied,
 * SL_STATUS_NOT_FOUND if the Group ID does not contain any members,
 * or SL_STATUS_WOULD_OVERFLOW if the provided list is insufficient.
 */
sl_status_t
  zigpc_group_map_get_memberlist_items(zigbee_group_id_t group_id,
                                       zigpc_group_member_t *const member_list,
                                       size_t member_list_count);

/**
 * @brief zigpc_group_map_clear
 * Clears the group map of all contents
 *
 * @return SL_STATUS_OK if able to empty the map of all contents
 */
sl_status_t zigpc_group_map_clear(void);

/**
 * @brief zigpc_group_map_retrieve_group_name
 * Retrieve the name of a group on a given endpoint
 *
 * @param member        - the new member to add
 * @param group_id      - the group to add a new member to
 * @param is_reported   - the direction (REPORTED/DESIRED) of the operation
 * @param group_name    - Reference to buffer to populate name.
 * @param group_name_size - Reference to buffer size:
 *                          IN: provided buffer capacity.
 *                          OUT: actual size of buffer used.
 *
 * @return SL_STATUS_OK if able to retrieve the group name properly
 * SL_STATUS_WOULD_OVERFLOW if the name requested is too large
 * to fit in the destination. SL_STATUS_EMPTY if the group is
 * found but has no name. SL_STATUS_NULL_POINTER on invalid args passed in.
 * SL_STATUS_FAIL on all other failures.
 */
sl_status_t zigpc_group_map_retrieve_group_name(zigpc_group_member_t member,
                                                zigbee_group_id_t group_id,
                                                bool is_reported,
                                                char *const group_name,
                                                size_t *const group_name_size);

/**
 * @brief zigpc_group_map_set_group_name
 * Set the name of a group on a given endpoint
 *
 * @param member        - the new member to add
 * @param group_id      - the group to add a new member to
 * @param is_reported   - the direction (REPORTED/DESIRED) of the operation
 * @param group_name    - the destination where to copy the name
 *
 * @return SL_STATUS_OK if able to set the group name properly
 * SL_STATUS_NOT_FOUND if unable to retrieve the member or the group
 * SL_STATUS_FAIL in all other failures
 */
sl_status_t zigpc_group_map_set_group_name(zigpc_group_member_t member,
                                           zigbee_group_id_t group_id,
                                           bool is_reported,
                                           const char *group_name);

/** @} end zigpc_group_mgmt */

#ifdef __cplusplus
}
#endif
#endif  //ZIGPC_GROUP_MGMT_H
