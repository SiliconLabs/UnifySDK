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

#include <unordered_map>
#include <list>
#include <algorithm>
#include <string.h>
#include <stdio.h>

#include "sl_log.h"

#include "zigpc_datastore.h"
#include "zigpc_datastore.hpp"

#include "zigpc_group_map.h"

#define LOG_TAG "zigpc_group_map"

sl_status_t zigpc_group_map_add_new(zigbee_group_id_t group,
                                    zigpc_group_member_t member,
                                    bool is_reported)
{
  char empty_name[] = "";
  return zigpc_group_map_add_new_with_name(group,
                                           empty_name,
                                           member,
                                           is_reported);
}

sl_status_t zigpc_group_map_add_new_with_name(zigbee_group_id_t group,
                                              const char *group_name,
                                              zigpc_group_member_t member,
                                              bool is_reported)
{
  sl_status_t status = SL_STATUS_OK;

  //retrives member_list if group_map[group] exists
  //group_map[group] creates an empty list if it is not in group_map!
  if (nullptr == group_name) {
    status = SL_STATUS_FAIL;
  } else if (strlen(group_name) >= ZCL_GROUP_NAME_LENGTH) {
    status = SL_STATUS_WOULD_OVERFLOW;
  } else {
    status = zigpc_datastore_create_group(member.eui64,
                                          member.endpoint_id,
                                          is_reported,
                                          group);
  }

  if (SL_STATUS_OK == status) {
    zigpc_group_data_t group_data = {0};
    strcpy(group_data.group_name, group_name);
    group_data.group_name[ZCL_GROUP_NAME_LENGTH - 1] = '\0';

    status = zigpc_datastore_write_group(member.eui64,
                                         member.endpoint_id,
                                         is_reported,
                                         group,
                                         &group_data);
  }

  return status;
}

unsigned int zigpc_group_map_retrieve_num(zigpc_group_member_t member,
                                          bool is_reported)
{
  unsigned int list_size = zigpc_datastore_get_group_count(member.eui64,
                                                           member.endpoint_id,
                                                           is_reported);

  return list_size;
}

sl_status_t zigpc_group_map_retrieve_grouplist(zigpc_group_member_t member,
                                               zigbee_group_id_t *groups,
                                               unsigned int num_groups,
                                               bool is_reported)
{
  sl_status_t status = SL_STATUS_OK;

  if (groups == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  }

  unsigned int expected_size
    = zigpc_group_map_retrieve_num(member, is_reported);
  if (expected_size > num_groups) {
    status = SL_STATUS_WOULD_OVERFLOW;
  }

  if (SL_STATUS_OK == status) {
    std::vector<zigbee_group_id_t> group_vector
      = zigpc_datastore::group::get_group_list(member.eui64,
                                               member.endpoint_id,
                                               is_reported);

    std::copy(group_vector.begin(), group_vector.end(), groups);
  }

  return status;
}

sl_status_t zigpc_group_map_remove_group(zigbee_group_id_t group,
                                         zigpc_group_member_t member,
                                         bool is_reported)
{
  sl_status_t status = SL_STATUS_OK;
  status             = zigpc_datastore_remove_group(member.eui64,
                                        member.endpoint_id,
                                        is_reported,
                                        group);

  return status;
}

sl_status_t zigpc_group_map_remove_all(zigpc_group_member_t member)
{
  std::vector<zigbee_group_id_t> desired_group_list
    = zigpc_datastore::group::get_group_list(member.eui64,
                                             member.endpoint_id,
                                             false);

  std::vector<zigbee_group_id_t> reported_group_list
    = zigpc_datastore::group::get_group_list(member.eui64,
                                             member.endpoint_id,
                                             true);

  for (auto desired_it = desired_group_list.begin();
       desired_it != desired_group_list.end();
       desired_it++) {
    sl_status_t desired_status
      = zigpc_datastore_remove_group(member.eui64,
                                     member.endpoint_id,
                                     false,
                                     *desired_it);

    if (SL_STATUS_OK != desired_status) {
      sl_log_warning(LOG_TAG, "Error removing desired group");
    }
  }

  for (auto reported_it = reported_group_list.begin();
       reported_it != reported_group_list.end();
       reported_it++) {
    sl_status_t reported_status
      = zigpc_datastore_remove_group(member.eui64,
                                     member.endpoint_id,
                                     true,
                                     *reported_it);

    if (SL_STATUS_OK != reported_status) {
      sl_log_warning(LOG_TAG, "Error removing reported group");
    }
  }

  return SL_STATUS_OK;
}

sl_status_t zigpc_group_map_clear(void)
{
  return SL_STATUS_OK;
}

size_t zigpc_group_map_get_memberlist_count(zigbee_group_id_t group_id)
{
  size_t num_members = 0;

  std::vector<zigbee_eui64_uint_t> eui64_list
    = zigpc_datastore::device::get_id_list();

  for (auto eui_iter = eui64_list.begin(); eui_iter != eui64_list.end();
       eui_iter++) {
    zigbee_eui64_t eui64;
    sl_status_t status = zigbee_uint_to_eui64(*eui_iter, eui64);

    if (SL_STATUS_OK != status) {
      sl_log_warning(LOG_TAG, "Error retrieving eui64");
      continue;
    } else {
      std::vector<zigbee_endpoint_id_t> endpoint_list
        = zigpc_datastore::endpoint::get_id_list(eui64);

      for (auto end_iter = endpoint_list.begin();
           end_iter != endpoint_list.end();
           end_iter++) {

        bool has_reported_group
          = zigpc_datastore_contains_group(eui64, *end_iter, true, group_id);

        if (has_reported_group) {
          num_members++;
        }
      }
    }
  }

  return num_members;
}

sl_status_t
  zigpc_group_map_get_memberlist_items(zigbee_group_id_t group_id,
                                       zigpc_group_member_t *const member_list,
                                       size_t member_list_count)
{
  unsigned int expected_num = zigpc_group_map_get_memberlist_count(group_id);

  if (expected_num == 0U) {
    return SL_STATUS_NOT_FOUND;
  }

  if (expected_num > member_list_count) {
    return SL_STATUS_WOULD_OVERFLOW;
  }

  if (member_list == nullptr) {
    return SL_STATUS_NULL_POINTER;
  }

  std::list<zigpc_group_member_t> member_temp;

  std::vector<zigbee_eui64_uint_t> eui64_list
    = zigpc_datastore::device::get_id_list();

  for (zigbee_eui64_uint_t eui64_uint: eui64_list) {
    zigbee_eui64_t eui64;
    sl_status_t status = zigbee_uint_to_eui64(eui64_uint, eui64);

    if (SL_STATUS_OK != status) {
      sl_log_warning(LOG_TAG, "Error retrieving eui64");
      continue;
    } else {
      std::vector<zigbee_endpoint_id_t> endpoint_list
        = zigpc_datastore::endpoint::get_id_list(eui64);

      for( zigbee_endpoint_id_t endpoint_id : endpoint_list)
      {
        bool has_reported_group
          = zigpc_datastore_contains_group(eui64, endpoint_id, true, group_id);

        if (has_reported_group) {
          zigpc_group_member_t next_member;
          memcpy(next_member.eui64, eui64, ZIGBEE_EUI64_SIZE);
          next_member.endpoint_id = endpoint_id;

          member_temp.push_back(next_member);
        }
      }
    }
  }

  std::copy(member_temp.begin(), member_temp.end(), member_list);

  return SL_STATUS_OK;
}

sl_status_t zigpc_group_map_retrieve_group_name(zigpc_group_member_t member,
                                                zigbee_group_id_t group_id,
                                                bool is_reported,
                                                char *const group_name,
                                                size_t *const group_name_size)
{
  sl_status_t status = SL_STATUS_OK;
  std::string group_name_str;

  if ((group_name == nullptr) || (group_name_size == nullptr)) {
    status = SL_STATUS_NULL_POINTER;
  } else if (*group_name_size < 1U) {
    status = SL_STATUS_INVALID_RANGE;
  } else {
    zigpc_group_data_t group_data;

    status = zigpc_datastore_read_group(member.eui64,
                                        member.endpoint_id,
                                        is_reported,
                                        group_id,
                                        &group_data);

    if (SL_STATUS_OK == status) {
      group_name_str = group_data.group_name;
    } else {
      *group_name_size = 0U;
      status           = SL_STATUS_NOT_FOUND;
    }
  }

  // Empty name handling
  if ((status == SL_STATUS_OK) && (group_name_str.empty())) {
    group_name[0]    = '\0';
    *group_name_size = 0U;
    status           = SL_STATUS_EMPTY;
  }

  // Non-empty name handling
  if (SL_STATUS_OK == status) {
    if (group_name_str.size() < *group_name_size) {
      // std::string is guaranteed to be null-terminated
      strcpy(group_name, group_name_str.c_str());
      *group_name_size = group_name_str.size();
    } else {
      status = SL_STATUS_WOULD_OVERFLOW;
    }
  }

  return status;
}

sl_status_t zigpc_group_map_set_group_name(zigpc_group_member_t member,
                                           zigbee_group_id_t group_id,
                                           bool is_reported,
                                           const char *group_name)
{
  sl_status_t status = SL_STATUS_OK;

  if (nullptr == group_name) {
    status = SL_STATUS_FAIL;
  } else if (strlen(group_name) >= ZCL_GROUP_NAME_LENGTH) {
    status = SL_STATUS_WOULD_OVERFLOW;
  }

  bool has_group = zigpc_datastore_contains_group(member.eui64,
                                                  member.endpoint_id,
                                                  is_reported,
                                                  group_id);

  if ((SL_STATUS_OK == status) && (!has_group)) {
    status = SL_STATUS_NOT_FOUND;
  }

  if (SL_STATUS_OK == status) {
    zigpc_group_data_t group_data;

    memcpy(group_data.group_name, group_name, ZCL_GROUP_NAME_LENGTH);
    status = zigpc_datastore_write_group(member.eui64,
                                         member.endpoint_id,
                                         is_reported,
                                         group_id,
                                         &group_data);
  }

  return status;
}
