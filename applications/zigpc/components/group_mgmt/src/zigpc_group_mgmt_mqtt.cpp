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
#include <vector>
#include <string>
#include <cstring>

#include "sl_log.h"
#include "uic_mqtt.h"
#include "zigpc_common_zigbee.h"
#include "zigpc_common_unid.h"

#include "zigpc_datastore.h"
#include "zigpc_datastore.hpp"

#include "zigpc_group_map.h"
#include "zigpc_group_mgmt_mqtt.h"


static const char LOG_TAG[] = "zigpc_groups_mqtt";

/**
 * @brief MQTT JSON payload format for group name publishing.
 *
 */
const std::string GROUP_NAME_FMTSTR = R"({"value":"%s"})";

sl_status_t zigpc_group_mqttpub_republish()
{
    sl_status_t status = SL_STATUS_OK;

    size_t num_devices = 
        zigpc_datastore_get_device_count();

    if(num_devices == 0)
    {
        sl_log_warning(LOG_TAG, "No devices found when republishing groups");
        return SL_STATUS_OK;
    }
 

    std::vector<zigbee_eui64_uint_t> eui64_list =
        zigpc_datastore::device::get_id_list();
    
    for(auto eui_iter = eui64_list.begin();
            eui_iter != eui64_list.end();
            eui_iter++)
    {
        zigbee_eui64_t eui64;

        status = zigbee_uint_to_eui64(
                    *eui_iter, eui64);

        if(status != SL_STATUS_OK)
        {
            continue;
        }

        std::vector<zigbee_endpoint_id_t> 
            endpoint_list = 
                zigpc_datastore::endpoint::get_id_list(eui64);

        for(auto endp_iter = endpoint_list.begin();
                endp_iter != endpoint_list.end();
                endp_iter++)
        {
            status = zigpc_group_mqttpub_grouplist(
                        eui64,
                        *endp_iter,
                        false);
            
            if(status != SL_STATUS_OK)
            {
                sl_log_warning(LOG_TAG, 
                        "Error republishing desired grouplist");
            }
            
            status = zigpc_group_mqttpub_grouplist(
                        eui64,
                        *endp_iter,
                        true);
            
            if(status != SL_STATUS_OK)
            {
                sl_log_warning(LOG_TAG, 
                        "Error republishing reported grouplist");
            }
        }
    }   

    return SL_STATUS_OK;
}

sl_status_t
  zigpc_group_mqttpub_grouplist(const zigbee_eui64_t eui64,
                                const zigbee_endpoint_id_t endpoint_id,
                                bool is_reported)
{
  size_t payload_size = ZCL_DEFAULT_STR_LENGTH;
  char topic_string[ZCL_DEFAULT_STR_LENGTH];
  char payload_string[payload_size];

  char base_group_topic[] = "/Groups/Attributes/GroupList/";

  sl_status_t status = zigpc_group_mqtt_assemble_topic(eui64,
                                                       endpoint_id,
                                                       base_group_topic,
                                                       strlen(base_group_topic),
                                                       is_reported,
                                                       topic_string,
                                                       ZCL_DEFAULT_STR_LENGTH);

  if (status == SL_STATUS_OK) {
    status = zigpc_group_mqtt_assemble_group_payload(eui64,
                                                     endpoint_id,
                                                     is_reported,
                                                     payload_string,
                                                     &payload_size);
  }

  if (status == SL_STATUS_OK) {
    uic_mqtt_publish(topic_string, payload_string, payload_size, true);
  } else {
    status = SL_STATUS_FAIL;
    sl_log_warning(LOG_TAG, "Error assembling GroupList: 0x%0X", status);
  }

  return status;
}

sl_status_t
  zigpc_group_mqttpub_member_groupname(const zigbee_eui64_t eui64,
                                       const zigbee_endpoint_id_t endpoint_id,
                                       const zigbee_group_id_t group_id,
                                       bool is_reported)
{
  size_t payload_size = ZCL_DEFAULT_STR_LENGTH;
  char topic_string[ZCL_DEFAULT_STR_LENGTH];
  char payload_string[payload_size];

  std::string base_group_topic = "/Groups/Attributes/";
  base_group_topic = base_group_topic + std::to_string(group_id) + "/Name/";

  sl_status_t status = zigpc_group_mqtt_assemble_topic(eui64,
                                                       endpoint_id,
                                                       base_group_topic.c_str(),
                                                       base_group_topic.size(),
                                                       is_reported,
                                                       topic_string,
                                                       ZCL_DEFAULT_STR_LENGTH);

  if (status == SL_STATUS_OK) {
    status = zigpc_group_mqtt_assemble_name_payload(eui64,
                                                    endpoint_id,
                                                    group_id,
                                                    is_reported,
                                                    payload_string,
                                                    &payload_size);
  }

  if (status == SL_STATUS_OK) {
    uic_mqtt_publish(topic_string, payload_string, payload_size, true);
  } else {
    status = SL_STATUS_FAIL;
    sl_log_warning(LOG_TAG, "Error assembling GroupName: 0x%0X", status);
  }

  return status;
}

sl_status_t
  zigpc_group_mqtt_assemble_topic(const zigbee_eui64_t eui64,
                                  const zigbee_endpoint_id_t endpoint_id,
                                  const char *base_cstr,
                                  unsigned int base_cstr_size,
                                  bool is_reported,
                                  char *topic_string,
                                  unsigned int topic_size)
{
  std::string topic;

  size_t unid_len = ZIGBEE_EUI64_HEX_STR_LENGTH + strlen(ZIGPC_UNID_PREFIX);
  std::vector<char> unid_cstr(unid_len);

  sl_status_t status
    = zigpc_common_eui64_to_unid(eui64, unid_cstr.data(), unid_len);

  bool is_valid_base_string = (base_cstr != NULL) && (base_cstr_size > 0);

  if ((status == SL_STATUS_OK) && is_valid_base_string) {
    std::string eui64_str(unid_cstr.data());
    std::string base_string(base_cstr);

    topic = "ucl/by-unid/" + eui64_str + "/ep" + std::to_string(endpoint_id)
            + base_string + (is_reported ? "Reported" : "Desired");
  }

  if ((status == SL_STATUS_OK) && (topic.size() < topic_size)) {
    snprintf(topic_string, topic_size, "%s", topic.c_str());
  } else {
    status = SL_STATUS_WOULD_OVERFLOW;
    sl_log_debug(LOG_TAG,
                 "Insufficient space for topic assembly: 0x%0X",
                 status);
  }

  return status;
}

sl_status_t zigpc_group_mqtt_assemble_group_payload(
  const zigbee_eui64_t eui64,
  const zigbee_endpoint_id_t endpoint_id,
  bool is_reported,
  char *payload_string,
  size_t *payload_size)
{
  zigpc_group_member_t member;
  member.endpoint_id = endpoint_id;
  memcpy(member.eui64, eui64, ZIGBEE_EUI64_SIZE);

  unsigned int num_groups = zigpc_group_map_retrieve_num(member, is_reported);

  zigbee_group_id_t group_list[num_groups];

  sl_status_t status = zigpc_group_map_retrieve_grouplist(member,
                                                          group_list,
                                                          num_groups,
                                                          is_reported);

  std::string payload;
  if (status == SL_STATUS_OK) {
    payload.append(R"({"value":[)");
    unsigned int i;
    for (i = 0; i < num_groups; i++) {
      payload = payload + std::to_string(group_list[i]);

      if (i != num_groups - 1) {
        payload.append(",");
      }
    }
    payload.append("]}");
  }

  if (payload.size() == 0) {
    *payload_size = 0U;
    status        = SL_STATUS_OK;
  } else if (payload.size() < *payload_size) {
    snprintf(payload_string, *payload_size, "%s", payload.c_str());
    *payload_size = payload.size();
    status        = SL_STATUS_OK;
  } else {
    status        = SL_STATUS_WOULD_OVERFLOW;
    *payload_size = 0;
    sl_log_warning(LOG_TAG,
                   "Insufficient space for payload assembly: 0x%0X",
                   status);
  }

  return status;
}

sl_status_t
  zigpc_group_mqtt_assemble_name_payload(const zigbee_eui64_t eui64,
                                         const zigbee_endpoint_id_t endpoint_id,
                                         const zigbee_group_id_t group_id,
                                         bool is_reported,
                                         char *const payload_string,
                                         size_t *const payload_size)
{
  if ((eui64 == nullptr) || (payload_string == nullptr)
      || (payload_size == nullptr)) {
    return SL_STATUS_NULL_POINTER;
  }

  size_t group_name_size = ZCL_DEFAULT_STR_LENGTH;
  char group_name[group_name_size];

  zigpc_group_member_t member;
  member.endpoint_id = endpoint_id;
  memcpy(member.eui64, eui64, ZIGBEE_EUI64_SIZE);

  sl_status_t status = zigpc_group_map_retrieve_group_name(member,
                                                           group_id,
                                                           is_reported,
                                                           group_name,
                                                           &group_name_size);

  if ((status == SL_STATUS_OK) || (status == SL_STATUS_EMPTY)) {
    // Ensure provided payload is sufficient
    const size_t formatted_payload_size
      = group_name_size + GROUP_NAME_FMTSTR.size() - 2U;
    if (formatted_payload_size >= *payload_size) {
      sl_log_warning(LOG_TAG, "Insufficient space for name payload assembly");
      return SL_STATUS_WOULD_OVERFLOW;
    }

    // Ensure empty group name is null-terminated
    if (status == SL_STATUS_EMPTY) {
      group_name[0] = '\0';
    }

    *payload_size = snprintf(payload_string,
                             *payload_size,
                             GROUP_NAME_FMTSTR.c_str(),
                             group_name);
    return SL_STATUS_OK;
  } else if (status == SL_STATUS_NOT_FOUND) {
    // Group name not found, populate empty payload
    payload_string[0] = '\0';
    *payload_size     = 0U;
    return SL_STATUS_OK;
  } else {
    // Other name lookup failure case
    return SL_STATUS_FAIL;
  }
}
