/*******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <list>
#include <string>

#include "zcl_attribute_info.h"
#include "attribute_map.h"
#include "sl_status.h"

template<> struct std::hash<attribute_key> {
  std::size_t operator()(const attribute_key &key) const
  {
    char eui64_cstr[ZIGBEE_EUI64_HEX_STR_LENGTH];

    zigbee_eui64_to_str(key.eui64, eui64_cstr, ZIGBEE_EUI64_HEX_STR_LENGTH);

    std::string eui64_str(eui64_cstr);
    std::string hash_str = eui64_str + std::to_string(key.endpoint_id)
                           + std::to_string(key.cluster_id);

    return hash<string>()(hash_str);
  }
};

bool operator==(const zcl_attribute_t &a, const zcl_attribute_t &b)
{
  return (a.cluster_id == b.cluster_id) && (a.attribute_id == b.attribute_id);
}

bool operator==(const attribute_key &a, const attribute_key &b)
{
  int eui64_cmp   = memcmp(a.eui64, b.eui64, ZIGBEE_EUI64_SIZE);
  bool comparison = (eui64_cmp == 0) && (a.endpoint_id == b.endpoint_id)
                    && (a.cluster_id == b.cluster_id);
  return comparison;
}

static std::unordered_map<attribute_key, std::list<zcl_attribute_t>>
  attribute_map;

sl_status_t register_node_attributes(const zigbee_node_t node)
{
  sl_status_t status = SL_STATUS_OK;

  if (!is_valid_zigbee_node(node)) {
    status = SL_STATUS_FAIL;
  }

  unsigned int i;
  for (i = 0; (i < node.endpoint_count) && (status == SL_STATUS_OK); i++) {
    status = register_endpoint_attributes(node.eui64, node.endpoint_list[i]);
  }

  return status;
}

sl_status_t register_endpoint_attributes(const zigbee_eui64_t eui64,
                                         const zigbee_endpoint_t endpoint)
{
  sl_status_t status = SL_STATUS_OK;

  if (!is_valid_zigbee_endpoint(endpoint)) {
    status = SL_STATUS_FAIL;
  }

  unsigned int i;
  for (i = 0; (i < endpoint.cluster_count) && (status == SL_STATUS_OK); i++) {
    status = register_cluster_attributes(eui64,
                                         endpoint.endpoint_id,
                                         endpoint.cluster_list[i].cluster_id);

    /*
         * Ignore failed calls to register unsupported clusters. Override
         * status to be SL_STATUS_OK in order to let the rest of the clusters
         * to be registered.
         */
    if (status == SL_STATUS_NOT_SUPPORTED) {
      status = SL_STATUS_OK;
    }
  }

  return status;
}

sl_status_t register_cluster_attributes(const zigbee_eui64_t eui64,
                                        const zigbee_endpoint_id_t endpoint_id,
                                        const zcl_cluster_id_t cluster_id)
{
  sl_status_t status    = SL_STATUS_OK;
  size_t num_attributes = 0U;
  num_attributes = zigpc_zclprofiles_get_cluster_attribute_count(cluster_id);

  if (num_attributes == 0U) {
    status = SL_STATUS_NOT_SUPPORTED;
  } else {
    zcl_attribute_t attribute_list[num_attributes];
    status = zigpc_zclprofiles_get_cluster_attribute_list(cluster_id,
                                                          attribute_list);

    if (status == SL_STATUS_OK) {
      attribute_key key;
      memcpy(key.eui64, eui64, ZIGBEE_EUI64_SIZE);
      key.endpoint_id = endpoint_id;
      key.cluster_id  = cluster_id;

      status = register_attributes(key,
                                   attribute_list,
                                   (unsigned int)num_attributes);
    }
  }

  return status;
}

sl_status_t register_attributes(const attribute_key key,
                                zcl_attribute_t *attributes,
                                unsigned int num_attributes)
{
  sl_status_t status = SL_STATUS_OK;
  std::unordered_map<attribute_key, std::list<zcl_attribute_t>>::iterator
    attr_it;
  attr_it = attribute_map.find(key);

  //only insert if the entry does not already exist
  if ((attributes != NULL) && (attr_it == attribute_map.end())) {
    std::list<zcl_attribute_t> attr_list(attributes,
                                         attributes + num_attributes);

    std::pair<attribute_key, std::list<zcl_attribute_t>> key_pair(key,
                                                                  attr_list);
    attribute_map.insert(key_pair);

    status = SL_STATUS_OK;
  } else {
    status = SL_STATUS_FAIL;
  }

  return status;
}

sl_status_t read_attributes(const attribute_key key,
                            zcl_attribute_t *attributes,
                            const unsigned int num_attributes)
{
  sl_status_t status          = SL_STATUS_OK;
  unsigned int attr_list_size = 0;

  std::unordered_map<attribute_key, std::list<zcl_attribute_t>>::iterator
    attr_it;
  attr_it = attribute_map.find(key);

  std::list<zcl_attribute_t> attr_list_raw;

  if (attr_it != attribute_map.end()) {
    attr_list_size = attr_list_raw.size();
    attr_list_raw  = attr_it->second;
  } else {
    status = SL_STATUS_FAIL;
  }

  if ((attr_list_size <= num_attributes) && (attributes != NULL)
      && (status == SL_STATUS_OK)) {
    zcl_attribute_t attr_list[attr_list_raw.size()];

    unsigned int i;
    std::list<zcl_attribute_t>::iterator it = attr_list_raw.begin();

    for (i = 0; i < attr_list_raw.size(); i++) {
      attr_list[i] = *(it);
      it++;
    }

    status = copy_attribute_list(attr_list, attributes, attr_list_raw.size());

  } else {
    status = SL_STATUS_FAIL;
  }

  return status;
}

sl_status_t read_single_attribute(const attribute_key key,
                                  const zcl_attribute_id_t attribute_id,
                                  zcl_attribute_t *dest_attribute)
{
  sl_status_t status = SL_STATUS_OK;

  std::unordered_map<attribute_key, std::list<zcl_attribute_t>>::iterator
    attr_it;
  attr_it = attribute_map.find(key);

  zcl_attribute_t to_find;
  to_find.attribute_id = attribute_id;
  to_find.cluster_id   = key.cluster_id;

  if ((dest_attribute != NULL) && (attr_it != attribute_map.end())) {
    std::list<zcl_attribute_t> attr_list = attr_it->second;
    auto entry = std::find(attr_list.begin(), attr_list.end(), to_find);

    if (entry != attr_list.end()) {
      *dest_attribute = *entry;
    } else {
      status = SL_STATUS_FAIL;
    }
  } else {
    status = SL_STATUS_FAIL;
  }

  return status;
}

sl_status_t update_attributes(const attribute_key key,
                              const zcl_attribute_t *attributes,
                              unsigned int num_attributes)
{
  sl_status_t status = SL_STATUS_OK;

  std::unordered_map<attribute_key, std::list<zcl_attribute_t>>::iterator
    attr_it;
  attr_it = attribute_map.find(key);

  if (attr_it != attribute_map.end()) {
    std::list<zcl_attribute_t> attribute_list = attr_it->second;

    unsigned int i;
    for (i = 0; i < num_attributes; i++) {
      auto index = std::find(attribute_list.begin(),
                             attribute_list.end(),
                             attributes[i]);

      if (index != attribute_list.end()) {
        *index = attributes[i];
      } else {
        attribute_list.push_back(attributes[i]);
      }
    }

    attr_it->second = attribute_list;
    status          = SL_STATUS_OK;
  } else {
    status = SL_STATUS_FAIL;
  }

  return status;
}

sl_status_t remove_attributes(const attribute_key key)
{
  sl_status_t status = SL_STATUS_OK;
  std::unordered_map<attribute_key, std::list<zcl_attribute_t>>::iterator
    attr_it;
  attr_it = attribute_map.find(key);

  if (attr_it != attribute_map.end()) {
    attribute_map.erase(attr_it);
  }

  return status;
}
