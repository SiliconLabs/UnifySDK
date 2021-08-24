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

#include "zigpc_group_map.h"

bool operator==(const zigpc_group_member_t &a, const zigpc_group_member_t &b)
{
  int eui_64_cmp = memcmp(a.eui64, b.eui64, ZIGBEE_EUI64_SIZE);

  bool same_eui64    = (eui_64_cmp == 0);
  bool same_endpoint = (a.endpoint_id == b.endpoint_id);

  return same_eui64 && same_endpoint;
}

template<> struct std::hash<zigpc_group_member_t> {
  std::size_t operator()(const zigpc_group_member_t &key) const
  {
    char eui64_cstr[ZIGBEE_EUI64_HEX_STR_LENGTH];

    zigbee_eui64_to_str(key.eui64, eui64_cstr, ZIGBEE_EUI64_HEX_STR_LENGTH);

    std::string eui64_str(eui64_cstr);
    std::string hash_str = eui64_str + std::to_string(key.endpoint_id);

    return hash<string>()(hash_str);
  }
};

typedef struct group_data {
  group_data() {}
  group_data(zigbee_group_id_t id)
  {
    this->id = id;
  }

  zigbee_group_id_t id;
  std::string name;
} group_data_t;

bool operator==(const group_data_t &a, const group_data_t &b)
{
  return (a.id == b.id);
}

typedef struct {
  std::list<group_data_t> reported;
  std::list<group_data_t> desired;
} group_list_t;

static std::unordered_map<zigpc_group_member_t, group_list_t> group_map;

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
  }

  if (SL_STATUS_OK == status) {
    std::list<group_data_t> *group_list;

    if (is_reported) {
      group_list = &group_map[member].reported;
    } else {
      group_list = &group_map[member].desired;
    }

    group_data_t data;
    data.id   = group;
    data.name = group_name;

    auto found_group = std::find(group_list->begin(), group_list->end(), data);

    if (found_group == group_list->end()) {
      group_list->push_back(data);
    } else {
      // Update the group name if group info is already tracked.
      // This handles the case of renaming a group
      found_group->name = group_name;
    }
  }

  return status;
}

unsigned int zigpc_group_map_retrieve_num(zigpc_group_member_t member,
                                          bool is_reported)
{
  unsigned int list_size = 0;

  auto found_member = group_map.find(member);

  //only get the list size if we found the group!
  if (found_member != group_map.end()) {
    if (is_reported) {
      list_size = found_member->second.reported.size();
    } else {
      list_size = found_member->second.desired.size();
    }
  }

  return list_size;
}

sl_status_t zigpc_group_map_retrieve_grouplist(zigpc_group_member_t member,
                                               zigbee_group_id_t *groups,
                                               unsigned int num_groups,
                                               bool is_reported)
{
  sl_status_t status      = SL_STATUS_FAIL;
  unsigned int max_groups = 0;

  auto found_member = group_map.find(member);

  //check that we found the group, then get the max size
  if (found_member != group_map.end()) {
    max_groups = zigpc_group_map_retrieve_num(member, is_reported);
    status     = SL_STATUS_OK;
  }

  //set the fail conditions
  if ((found_member == group_map.end()) || (max_groups > num_groups)
      || (max_groups == 0) || (groups == NULL)) {
    status = SL_STATUS_FAIL;
  } else {
    group_data_t data_list[num_groups];
    //use copy_n to only copy as many as can be contained
    //treat num_members as the size of members array
    if (is_reported) {
      std::copy_n(found_member->second.reported.begin(), num_groups, data_list);
    } else {
      std::copy_n(found_member->second.desired.begin(), num_groups, data_list);
    }

    for (unsigned int i = 0; i < num_groups; i++) {
      groups[i] = data_list[i].id;
    }
  }

  return status;
}

sl_status_t zigpc_group_map_remove_group(zigbee_group_id_t group,
                                         zigpc_group_member_t member,
                                         bool is_reported)
{
  sl_status_t status = SL_STATUS_OK;
  auto found_member  = group_map.find(member);

  //only get the list size if we found the group!
  if (found_member != group_map.end()) {
    std::list<group_data_t> *group_list;

    if (is_reported) {
      group_list = &found_member->second.reported;
    } else {
      group_list = &found_member->second.desired;
    }

    auto group_index = std::find(group_list->begin(), group_list->end(), group);

    //do not set the status
    //STATUS_OK if the group list does not have the group!
    if (group_index != group_list->end()) {
      group_list->erase(group_index);
    }

    status = SL_STATUS_OK;
  } else {
    status = SL_STATUS_FAIL;
  }

  return status;
}

sl_status_t zigpc_group_map_remove_all(zigpc_group_member_t member)
{
  auto found_member = group_map.find(member);
  if (found_member != group_map.end()) {
    group_map.erase(found_member);
  }

  return SL_STATUS_OK;
}

sl_status_t zigpc_group_map_clear(void)
{
  group_map.clear();
  return SL_STATUS_OK;
}

size_t zigpc_group_map_get_memberlist_count(zigbee_group_id_t group_id)
{
  size_t num_members = 0;

  for (auto it = group_map.begin(); it != group_map.end(); it++) {
    std::list<group_data_t> group_list = it->second.reported;
    auto group_index
      = std::find(group_list.begin(), group_list.end(), group_id);

    if (group_index != group_list.end()) {
      num_members = num_members + 1;
    }
  }

  return num_members;
}

sl_status_t
  zigpc_group_map_get_memberlist_items(zigbee_group_id_t group_id,
                                       zigpc_group_member_t *const member_list,
                                       size_t member_list_count)
{
  sl_status_t status = SL_STATUS_OK;
  std::list<zigpc_group_member_t> members_internal;

  for (auto it = group_map.begin(); it != group_map.end(); it++) {
    std::list<group_data_t> group_list = it->second.reported;
    auto group_index
      = std::find(group_list.begin(), group_list.end(), group_id);

    if (group_index != group_list.end()) {
      members_internal.push_back(it->first);
    }
  }

  if ((members_internal.size() > 0)
      && (members_internal.size() <= member_list_count)) {
    status = SL_STATUS_OK;
    std::copy_n(members_internal.begin(), member_list_count, member_list);
  } else if (members_internal.size() > member_list_count) {
    status = SL_STATUS_WOULD_OVERFLOW;
  } else {
    status = SL_STATUS_NOT_FOUND;
  }

  return status;
}

sl_status_t zigpc_group_map_retrieve_group_name(zigpc_group_member_t member,
                                                zigbee_group_id_t group_id,
                                                bool is_reported,
                                                char *group_name,
                                                size_t group_name_size)
{
  sl_status_t status = SL_STATUS_OK;
  std::string group_name_str;
  auto found_member = group_map.find(member);

  if (found_member != group_map.end()) {
    std::list<group_data_t> group_list;

    if (is_reported) {
      group_list = found_member->second.reported;
    } else {
      group_list = found_member->second.desired;
    }

    auto group_index
      = std::find(group_list.begin(), group_list.end(), group_id);

    if (group_index != group_list.end()) {
      group_data_t data = *group_index;
      group_name_str    = data.name;
    } else {
      status = SL_STATUS_NOT_FOUND;
    }
  } else {
    status = SL_STATUS_NOT_FOUND;
  }

  if ((group_name_str.empty()) && (status == SL_STATUS_OK)) {
    status = SL_STATUS_EMPTY;
  }

  if (SL_STATUS_OK == status) {
    if (group_name_str.size() < group_name_size) {
      snprintf(group_name, group_name_size, "%s", group_name_str.c_str());
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
  }

  if (SL_STATUS_OK == status) {
    auto found_member = group_map.find(member);
    std::list<group_data_t> group_list;

    if (found_member != group_map.end()) {
      if (is_reported) {
        group_list = found_member->second.reported;
      } else {
        group_list = found_member->second.desired;
      }

      auto found_group
        = std::find(group_list.begin(), group_list.end(), group_id);

      if (found_group != group_list.end()) {
        found_group->name = group_name;

        if (is_reported) {
          found_member->second.reported = group_list;
        } else {
          found_member->second.desired = group_list;
        }

      } else {
        status = SL_STATUS_NOT_FOUND;
      }
    } else {
      status = SL_STATUS_NOT_FOUND;
    }
  }

  return status;
}
