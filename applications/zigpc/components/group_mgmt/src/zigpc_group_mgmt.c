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

#include <string.h>

#include "sl_log.h"
#include "zigpc_group_map.h"

#include "zigpc_group_mgmt.h"
#include "zigpc_group_mgmt_mqtt.h"
#include "zcl_command_parser.h"

static const char LOG_TAG[] = "zigpc_groups_interface";

void zigpc_group_handle_add_response(
  const zigbee_eui64_t eui64,
  const zigbee_endpoint_id_t endpoint,
  const zigpc_zclcmdparse_callback_data_t *add_data)
{
  sl_status_t status = SL_STATUS_OK;

  if (add_data->groups_add_group_response.status == 0) {
    zigpc_group_add_reported(add_data->groups_add_group_response.group_id,
                             eui64,
                             endpoint);
  } else {
    sl_log_warning(LOG_TAG, "Error handling add group response");
  }

  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG, "Error managing a newly added group");
  }
}

void zigpc_group_handle_remove_response(
  const zigbee_eui64_t eui64,
  const zigbee_endpoint_id_t endpoint,
  const zigpc_zclcmdparse_callback_data_t *remove_data)
{
  sl_status_t status = SL_STATUS_OK;

  if (remove_data->groups_remove_group_response.status == 0) {
    zigpc_group_remove(remove_data->groups_remove_group_response.group_id,
                       eui64,
                       endpoint,
                       ATTR_DIRECTION_REPORTED);
  } else {
    sl_log_warning(LOG_TAG, "Error handling remove group response");
  }

  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG, "Error removing a managed group");
  }
}

sl_status_t zigpc_group_init()
{
  zigpc_zclcmdparse_register_callback(
    ZIGPC_ZCL_CLUSTER_GROUPS,
    ZIGPC_ZCL_CLUSTER_GROUPS_COMMAND_ADD_GROUP_RESPONSE,
    zigpc_group_handle_add_response);

  zigpc_zclcmdparse_register_callback(
    ZIGPC_ZCL_CLUSTER_GROUPS,
    ZIGPC_ZCL_CLUSTER_GROUPS_COMMAND_REMOVE_GROUP_RESPONSE,
    zigpc_group_handle_remove_response);

  return SL_STATUS_OK;
}

sl_status_t zigpc_group_add_desired(const zigbee_group_id_t group_id,
                                    const zigbee_eui64_t eui64,
                                    const zigbee_endpoint_id_t endpoint_id)
{
  zigpc_group_member_t new_member;
  new_member.endpoint_id = endpoint_id;
  memcpy(new_member.eui64, eui64, ZIGBEE_EUI64_SIZE);

  sl_status_t status
    = zigpc_group_map_add_new(group_id, new_member, ATTR_DIRECTION_DESIRED);

  if (SL_STATUS_OK == status) {
    status = zigpc_group_mqttpub_grouplist(eui64,
                                           endpoint_id,
                                           ATTR_DIRECTION_DESIRED);
  }

  return status;
}

sl_status_t
  zigpc_group_add_desired_with_name(const zigbee_group_id_t group_id,
                                    const char *group_name,
                                    const zigbee_eui64_t eui64,
                                    const zigbee_endpoint_id_t endpoint_id)
{
  zigpc_group_member_t new_member;
  new_member.endpoint_id = endpoint_id;
  memcpy(new_member.eui64, eui64, ZIGBEE_EUI64_SIZE);

  sl_status_t status
    = zigpc_group_map_add_new_with_name(group_id,
                                        group_name,
                                        new_member,
                                        ATTR_DIRECTION_DESIRED);

  if (status == SL_STATUS_OK) {
    status = zigpc_group_mqttpub_member_groupname(eui64,
                                                  endpoint_id,
                                                  group_id,
                                                  ATTR_DIRECTION_DESIRED);
  }

  return status;
}

sl_status_t zigpc_group_add_reported(const zigbee_group_id_t group_id,
                                     const zigbee_eui64_t eui64,
                                     const zigbee_endpoint_id_t endpoint_id)
{
  zigpc_group_member_t new_member;
  new_member.endpoint_id = endpoint_id;
  memcpy(new_member.eui64, eui64, ZIGBEE_EUI64_SIZE);

  //required to get the "expected" group name
  //we dont recieve the status of the group name without sending
  //another explicit "ViewGroup" command
  //therefore, we have to check what the expected name is
  size_t group_name_size = ZCL_DEFAULT_STR_LENGTH;
  char group_name[group_name_size];
  sl_status_t status
    = zigpc_group_map_retrieve_group_name(new_member,
                                          group_id,
                                          ATTR_DIRECTION_DESIRED,
                                          group_name,
                                          &group_name_size);

  //if the group name is empty, only add new
  if (SL_STATUS_EMPTY == status) {
    status
      = zigpc_group_map_add_new(group_id, new_member, ATTR_DIRECTION_REPORTED);
  }
  //otherwise, add it with the name
  else if (SL_STATUS_OK == status) {
    status = zigpc_group_map_add_new_with_name(group_id,
                                               group_name,
                                               new_member,
                                               ATTR_DIRECTION_REPORTED);
  }

  if (SL_STATUS_OK == status) {
    status = zigpc_group_mqttpub_grouplist(eui64,
                                           endpoint_id,
                                           ATTR_DIRECTION_REPORTED);
  }
  if (SL_STATUS_OK == status) {
    status = zigpc_group_mqttpub_member_groupname(eui64,
                                                  endpoint_id,
                                                  group_id,
                                                  ATTR_DIRECTION_REPORTED);
  }

  return status;
}

sl_status_t zigpc_group_remove(const zigbee_group_id_t group_id,
                               const zigbee_eui64_t eui64,
                               const zigbee_endpoint_id_t endpoint_id,
                               bool is_reported)
{
  zigpc_group_member_t new_member;
  new_member.endpoint_id = endpoint_id;
  memcpy(new_member.eui64, eui64, ZIGBEE_EUI64_SIZE);

  sl_status_t status
    = zigpc_group_map_remove_group(group_id, new_member, is_reported);

  if (SL_STATUS_OK == status) {
    status = zigpc_group_mqttpub_grouplist(eui64, endpoint_id, is_reported);
  }
  if (SL_STATUS_OK == status) {
    status = zigpc_group_mqttpub_member_groupname(eui64,
                                                  endpoint_id,
                                                  group_id,
                                                  is_reported);
  }

  return status;
}

sl_status_t zigpc_group_remove_all(const zigbee_eui64_t eui64,
                                   const zigbee_endpoint_id_t endpoint_id)
{
  zigpc_group_member_t new_member;
  new_member.endpoint_id = endpoint_id;
  memcpy(new_member.eui64, eui64, ZIGBEE_EUI64_SIZE);

  zigpc_group_map_remove_all(new_member);

  (void)zigpc_group_mqttpub_grouplist(eui64,
                                      endpoint_id,
                                      ATTR_DIRECTION_DESIRED);
  (void)zigpc_group_mqttpub_grouplist(eui64,
                                      endpoint_id,
                                      ATTR_DIRECTION_REPORTED);

  return SL_STATUS_OK;
}

size_t zigpc_groupmgmt_get_member_count(zigbee_group_id_t group_id)
{
  return zigpc_group_map_get_memberlist_count(group_id);
}

sl_status_t
  zigpc_groupmgmt_get_member_list(zigbee_group_id_t group_id,
                                  zigpc_group_member_t *const member_list,
                                  size_t member_list_count)
{
  return zigpc_group_map_get_memberlist_items(group_id,
                                              member_list,
                                              member_list_count);
}
