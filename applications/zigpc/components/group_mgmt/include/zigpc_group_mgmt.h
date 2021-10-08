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
 * @file zigpc_group_mgmt.h
 * @defgroup zigpc_group_mgmt ZigPC Group Management Interface functions
 * @ingroup zigpc_components
 * @brief This header contains the definitions and functionality associated with
 * the Zigpc Group Management. This is the intended public interface for the
 * entire component. All definitions and functions here are required for
 * manipulating groups
 *
 * @{
 */

#ifndef ZIGPC_GROUP_MGMT_H
#define ZIGPC_GROUP_MGMT_H

#include "sl_status.h"
#include "zigpc_common_zigbee.h"
#include "zcl_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief zigpc_group_member_t
 * A structure representing the member of a zigbee group
 **/
typedef struct {
  zigbee_eui64_t eui64;
  zigbee_endpoint_id_t endpoint_id;
} zigpc_group_member_t;

/**
 * @brief zigpc_group_get_cluster_id
 * Retrieve the cluster ID for groups. Useful for handling group clusters
 *
 * @ return zcl_cluster_id_t - the cluster ID of the ZCL Groups Cluster
 */
static inline zcl_cluster_id_t zigpc_group_get_cluster_id(void)
{
  return ZIGPC_ZCL_CLUSTER_GROUPS;
}

/**
 * @brief zigpc_group_get_add_id
 * Retrieve the command ID for the add_group command
 *
 * @ return zcl_command_id_t - the zcl command id of the groups - add group command
 */
static inline zcl_command_id_t zigpc_group_add_get_id(void)
{
  return ZIGPC_ZCL_CLUSTER_GROUPS_COMMAND_ADD_GROUP;
}

/**
 * @brief zigpc_group_get_remove_id
 * Retrieve the command ID for the remove group command.
 *
 * @ return zcl_command_id_t - the zcl command id of the groups - remove group command
 */
static inline zcl_command_id_t zigpc_group_remove_get_id(void)
{
  return ZIGPC_ZCL_CLUSTER_GROUPS_COMMAND_REMOVE_GROUP;
}

/**
 * @brief zigpc_group_get_removeall_id
 * Retrieve the command ID for the removeall groups command.
 *
 * @ return zcl_command_id_t - the zcl command ID of the groups - removeall command
 */
static inline zcl_command_id_t zigpc_group_removeall_get_id(void)
{
  return ZIGPC_ZCL_CLUSTER_GROUPS_COMMAND_REMOVE_ALL_GROUPS;
}

/**
 * @brief zigpc_groups_init
 * Initialize the groups component. Sets up all the parts required for groups
 * operation, including all callbacks
 *
 */
sl_status_t zigpc_group_init();

/**
 * @brief zigpc_group_add_desired
 * Add a new "desired" or expected group to a given node and endpoint
 *
 * @param group_id      - the id of the expected group
 * @param eui64         - the eui64 of the node
 * @param endpoint_id   - the endpoint id to add to the group
 *
 * @return SL_STATUS_OK if able to successfully add the group
 */
sl_status_t zigpc_group_add_desired(const zigbee_group_id_t group_id,
                                    const zigbee_eui64_t eui64,
                                    const zigbee_endpoint_id_t endpoint_id);

/**
 * @brief zigpc_group_add_desired_with_name
 * Add a new group with a specified name
 *
 * @param group_id  - the id of the expected group
 * @param group_name- the expected name of the group
 * @param eui64         - the eui64 of the node
 * @param endpoint_id   - the endpoint id to add to the group
 *
 * @return SL_STATUS_OK if able to successfully add the group
**/
sl_status_t
  zigpc_group_add_desired_with_name(const zigbee_group_id_t group_id,
                                    const char *group_name,
                                    const zigbee_eui64_t eui64,
                                    const zigbee_endpoint_id_t endpoint_id);

/**
 * @brief zigpc_group_add_reported
 * Add a new "reported" or actual group to a given node and endpoint
 *
 * @param group_id      - the id of the reported group
 * @param eui64         - the eui64 of the node
 * @param endpoint_id   - the endpoint id to add to the group
 *
 * @return SL_STATUS_OK if able to successfully add the group
 */
sl_status_t zigpc_group_add_reported(const zigbee_group_id_t group_id,
                                     const zigbee_eui64_t eui64,
                                     const zigbee_endpoint_id_t endpoint_id);

/**
 * @brief zigpc_group_remove
 * Remove an eui64/endpoint from a given group
 *
 * @param group_id      - the id of the group
 * @param eui64         - the eui64 of the node
 * @param endpoint_id   - the endpoint id remove from the group
 * @param direction     - the direction (reported/desired)
 *
 * @return SL_STATUS_OK if able to successfully remove the group
 */
sl_status_t zigpc_group_remove(const zigbee_group_id_t group_id,
                               const zigbee_eui64_t eui64,
                               const zigbee_endpoint_id_t endpoint_id,
                               bool is_reported);

/**
 * @brief zigpc_group_remove_all
 * Remove the eui64/endpoint from all associated groups
 *
 * @param eui64         - the eui64 of the node
 * @param endpoint_id   - the endpoint id to add to the group
 *
 * @return SL_STATUS_OK if able to successfully remove all groups
 */
sl_status_t zigpc_group_remove_all(const zigbee_eui64_t eui64,
                                   const zigbee_endpoint_id_t endpoint_id);

/**
 * @brief Retrieve the count of EUI64/endpoint combinations as part of a
 * Group ID.
 *
 * @param group_id Group ID to lookup.
 * @return size_t Number of zigpc_group_member_t structures.
 */
size_t zigpc_groupmgmt_get_member_count(zigbee_group_id_t group_id);

/**
 * @brief Retrieve the list of group members corresponding to a Group ID.
 *
 * @param group_id          Group ID to lookup.
 * @param member_list       List of member structures to populate.
 * @param member_list_count Capacity of member list passed in.
 * @return sl_status_t SL_STATUS_OK if the list is copied,
 * SL_STATUS_NOT_FOUND if the Group ID does not contain any members,
 * or SL_STATUS_WOULD_OVERFLOW if the provided list is insufficient.
 */
sl_status_t
  zigpc_groupmgmt_get_member_list(zigbee_group_id_t group_id,
                                  zigpc_group_member_t *const member_list,
                                  size_t member_list_count);

/** @} end zigpc_group_mgmt */
#ifdef __cplusplus
}
#endif

#endif  //ZIGPC_GROUP_MGMT_H
