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

#include <map>
#include <cstring>
#include <string>

#include "sl_log.h"

#include "zigpc_node_mgmt.h"

#include "zigpc_node_map.h"
#include "zigpc_common_unid.h"
#include "zigpc_node_map.hpp"
#include "zigpc_common_zigbee.h"

static const char LOG_TAG[] = "zigpc_node_map";

static std::map<std::string, zigbee_node_t> node_map;

sl_status_t nodemap_manage_node(const zigbee_node_t node)
{
  sl_status_t status = SL_STATUS_OK;
  char eui64_cstr[ZIGBEE_EUI64_HEX_STR_LENGTH];

  if (is_valid_zigbee_node(node)) {

    zigbee_eui64_to_str(node.eui64,
                        eui64_cstr,
                        ZIGBEE_EUI64_HEX_STR_LENGTH);
    
    std::string eui64_string(eui64_cstr);

    if (node_map.find(eui64_string) != node_map.end()) {
      nodemap_remove_node(node.eui64);
    }

    std::pair<std::string, zigbee_node_t> entry(eui64_string, node);
    node_map.insert(entry);
    
  } else {
    sl_log_warning(LOG_TAG, "Error handling invalid node data");
    status = SL_STATUS_FAIL;
  }

  return status;
}

sl_status_t nodemap_manage_node_eui64(const zigbee_eui64_t eui64)
{
  zigbee_node_t node;

  node.is_active = true;
  memcpy(node.eui64, eui64, ZIGBEE_EUI64_SIZE);
  node.endpoint_count = 0;

  sl_status_t status = nodemap_manage_node(node);

  return status;
}

sl_status_t nodemap_add_node_endpoint(const zigbee_eui64_t eui64,
                                      const zigbee_endpoint_t endpoint)
{
  sl_status_t status = SL_STATUS_OK;
  char eui64_cstr[ZIGBEE_EUI64_HEX_STR_LENGTH];
  zigbee_eui64_to_str(eui64, eui64_cstr, ZIGBEE_EUI64_HEX_STR_LENGTH);

  std::string eui64_str(eui64_cstr);
  std::map<std::string, zigbee_node_t>::iterator node_iterator;

  node_iterator = node_map.find(eui64_str);
  if (node_iterator != node_map.end()) {
    unsigned int new_count = node_iterator->second.endpoint_count + 1;
    sl_status_t copy_status
      = copy_endpoint(endpoint,
                      &(node_iterator->second.endpoint_list[new_count - 1]));

    if (copy_status == SL_STATUS_OK) {
      node_iterator->second.endpoint_count = new_count;
      node_iterator->second.is_sleepy
        = node_iterator->second.is_sleepy
          || zigpc_nodemgmt_endpoint_contains_cluster(endpoint, 0x0020);
      //TODO put in POLL_CONTROL CLUSTER real define from ZAP
    }

    status = copy_status;
  } else {
    zigbee_node_t node;

    memcpy(node.eui64, eui64, ZIGBEE_EUI64_SIZE);
    node.endpoint_count = 1;
    zigbee_endpoint_t endpoint_list[1];
    endpoint_list[0] = endpoint;

    memcpy(node.endpoint_list, endpoint_list, sizeof(zigbee_endpoint_t));

    status = nodemap_manage_node(node);
  }

  return status;
}

sl_status_t nodemap_fetch_node(const zigbee_eui64_t eui64, zigbee_node_t *node)
{
  sl_status_t status = SL_STATUS_OK;
  char eui64_cstr[ZIGBEE_EUI64_HEX_STR_LENGTH];
  status = zigbee_eui64_to_str(eui64, eui64_cstr, ZIGBEE_EUI64_HEX_STR_LENGTH);

  if(SL_STATUS_OK != status)
  {
    sl_log_warning(LOG_TAG, "EUI64 string failed %s with status %X",
                        eui64_cstr,
                        status);
  }

  std::string eui64_str(eui64_cstr);
  std::map<std::string, zigbee_node_t>::iterator node_iterator;
  node_iterator = node_map.find(eui64_str);

  if (node_iterator != node_map.end()) {
    status = copy_node(node_iterator->second, node);
  } else {
    status = SL_STATUS_FAIL;
    sl_log_warning(LOG_TAG, "Requested node with eui64 not found");
  }

  return status;
}

sl_status_t nodemap_fetch_node_ep(const zigbee_eui64_t eui64,
                                  const zigbee_endpoint_id_t endpoint_id,
                                  zigbee_endpoint_t *endpoint)
{
  sl_status_t status = SL_STATUS_OK;
  zigbee_node_t node;
  zigbee_endpoint_t *found_ep = NULL;

  if (endpoint == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    status = nodemap_fetch_node(eui64, &node);
  }
  if (status == SL_STATUS_OK) {
    for (size_t ep_i = 0; (found_ep == NULL) && (ep_i < node.endpoint_count);
         ep_i++) {
      if (node.endpoint_list[ep_i].endpoint_id == endpoint_id) {
        found_ep = &node.endpoint_list[ep_i];
      }
    }
    if (found_ep == NULL) {
      status = SL_STATUS_FAIL;
    } else {
      status = copy_endpoint(*found_ep, endpoint);
    }
  }

  return status;
}

sl_status_t
  nodemap_fetch_node_ep_cluster(const zigbee_eui64_t eui64,
                                const zigbee_endpoint_id_t endpoint_id,
                                const zcl_cluster_id_t cluster_id,
                                zcl_cluster_type_t *cluster)
{
  sl_status_t status = SL_STATUS_OK;
  zigbee_endpoint_t endpoint;
  zcl_cluster_type_t *found_cluster = NULL;

  if (cluster == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    status = nodemap_fetch_node_ep(eui64, endpoint_id, &endpoint);
  }
  if (status == SL_STATUS_OK) {
    for (size_t cl_i = 0;
         (found_cluster == NULL) && (cl_i < endpoint.cluster_count);
         cl_i++) {
      if (endpoint.cluster_list[cl_i].cluster_id == cluster_id) {
        found_cluster = &endpoint.cluster_list[cl_i];
      }
    }
    if (found_cluster == NULL) {
      status = SL_STATUS_FAIL;
    } else {
      status = copy_cluster_list(found_cluster, cluster, 1);
    }
  }

  return status;
}

sl_status_t nodemap_update_node(const zigbee_node_t node)
{
  sl_status_t status = SL_STATUS_OK;

  char eui64_cstr[ZIGBEE_EUI64_HEX_STR_LENGTH];
  zigbee_eui64_to_str(node.eui64, eui64_cstr, ZIGBEE_EUI64_HEX_STR_LENGTH);

  std::string eui64_str(eui64_cstr);
  std::map<std::string, zigbee_node_t>::iterator node_iterator;
  node_iterator = node_map.find(eui64_str);

  if (node_iterator != node_map.end()) {
    status = nodemap_remove_node(node_iterator->second.eui64);
  }

  if (status == SL_STATUS_OK) {
    status = nodemap_manage_node(node);
  }

  return status;
}

sl_status_t nodemap_remove_node(const zigbee_eui64_t eui64)
{
  sl_status_t status = SL_STATUS_OK;

  char eui64_cstr[ZIGBEE_EUI64_HEX_STR_LENGTH];
  zigbee_eui64_to_str(eui64, eui64_cstr, ZIGBEE_EUI64_HEX_STR_LENGTH);
  std::string eui64_str(eui64_cstr);

  std::map<std::string, zigbee_node_t>::iterator node_iterator;
  node_iterator = node_map.find(eui64_str);

  if (node_iterator != node_map.end()) {
    node_map.erase(node_iterator);
  }

  return status;
}

bool nodemap_check_if_managed(const zigbee_eui64_t eui64)
{
  bool is_managed = false;

  char eui64_cstr[ZIGBEE_EUI64_HEX_STR_LENGTH];
  zigbee_eui64_to_str(eui64, eui64_cstr, ZIGBEE_EUI64_HEX_STR_LENGTH);

  std::string eui64_str(eui64_cstr);
  std::map<std::string, zigbee_node_t>::iterator node_iterator;
  node_iterator = node_map.find(eui64_str);

  is_managed = (node_iterator != node_map.end());

  return is_managed;
}

zigbee_endpoint_id_t
  nodemap_fetch_endpoint_containing_cluster(const zigbee_eui64_t eui64,
                                            const zcl_cluster_id_t cluster_id)
{
  sl_status_t status                  = SL_STATUS_OK;
  zigbee_endpoint_id_t found_endpoint = 0;
  zigbee_node_t node;
  status = nodemap_fetch_node(eui64, &node);

  if (SL_STATUS_OK == status) {
    bool found = false;
    for (unsigned int i = 0; (i < node.endpoint_count) && (false == found);
         i++) {
      bool contains_cluster
        = zigpc_nodemgmt_endpoint_contains_cluster(node.endpoint_list[i],
                                                   cluster_id);

      if (contains_cluster) {
        found_endpoint = node.endpoint_list[i].endpoint_id;
        found          = true;
      }
    }
  }

  return found_endpoint;
}

std::vector<std::string> zigpc_node_map_get_all_eui64()
{
  std::vector<std::string> eui64_v;
  for (auto const &node: node_map) {
    eui64_v.push_back(node.first);
  }
  return eui64_v;
};
