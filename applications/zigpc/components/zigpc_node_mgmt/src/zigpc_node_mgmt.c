/*******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <stdlib.h>
#include <string.h>

#include "zigpc_node_map.h"
#include "zigpc_node_mgmt.h"
#include "zigpc_node_mgmt_int.h"
#include "zigpc_node_mgmt_process.h"

sl_status_t zigpc_nodemgmt_manage_node_eui64(const zigbee_eui64_t eui64)
{
  sl_status_t status = SL_STATUS_OK;

  if (eui64 != NULL) {
    node_mgmt_manage_eui64_data_t data;
    memcpy(data.eui64, eui64, ZIGBEE_EUI64_SIZE);

    zigpc_node_process_send_event(NODE_MGMT_MANAGE_EUI64_EVT,
                                  (void *)&data,
                                  sizeof(node_mgmt_manage_eui64_data_t));

    status = SL_STATUS_OK;
  } else {
    status = SL_STATUS_FAIL;
  }

  return status;
}

sl_status_t zigpc_nodemgmt_manage_node(const zigbee_node_t node)
{
  sl_status_t status = SL_STATUS_OK;

  if (is_valid_zigbee_node(node)) {
    node_mgmt_manage_node_data_t data;
    memcpy(&data.node, &node, sizeof(zigbee_node_t));

    zigpc_node_process_send_event(NODE_MGMT_MANAGE_EVT,
                                  (void *)&data,
                                  sizeof(node_mgmt_manage_node_data_t));

    status = SL_STATUS_OK;
  } else {
    status = SL_STATUS_FAIL;
  }

  return status;
}

sl_status_t zigpc_nodemgmt_add_node_endpoint(const zigbee_eui64_t eui64,
                                             const zigbee_endpoint_t endpoint)
{
  sl_status_t status = SL_STATUS_OK;

  if ((eui64 != NULL) && (is_valid_zigbee_endpoint(endpoint))) {
    node_mgmt_add_endpoint_data_t endpoint_data;
    memcpy(endpoint_data.eui64, eui64, ZIGBEE_EUI64_SIZE);
    endpoint_data.endpoint = endpoint;

    zigpc_node_process_send_event(NODE_MGMT_ADD_END_EVT,
                                  (void *)&endpoint_data,
                                  sizeof(node_mgmt_add_endpoint_data_t));

    status = SL_STATUS_OK;
  } else {
    status = SL_STATUS_FAIL;
  }

  return status;
}

sl_status_t zigpc_nodemgmt_fetch_node(const zigbee_eui64_t eui64,
                                      zigbee_node_t *node)
{
  sl_status_t status;
  status = nodemap_fetch_node(eui64, node);

  return status;
}

sl_status_t
  zigpc_nodemgmt_fetch_node_ep_cluster(const zigbee_eui64_t eui64,
                                       const zigbee_endpoint_id_t endpoint_id,
                                       const zcl_cluster_id_t cluster_id,
                                       zcl_cluster_type_t *cluster)
{
  sl_status_t status;
  status
    = nodemap_fetch_node_ep_cluster(eui64, endpoint_id, cluster_id, cluster);

  return status;
}

zigbee_endpoint_id_t zigpc_nodemgmt_fetch_endpoint_containing_cluster(
  const zigbee_eui64_t eui64, const zcl_cluster_id_t cluster_id)
{
  return nodemap_fetch_endpoint_containing_cluster(eui64, cluster_id);
}

sl_status_t zigpc_nodemgmt_update_node(const zigbee_node_t node)
{
  sl_status_t status = SL_STATUS_OK;

  if (is_valid_zigbee_node(node)) {
    node_mgmt_manage_node_data_t data;
    memcpy(&data.node, &node, sizeof(zigbee_node_t));

    zigpc_node_process_send_event(NODE_MGMT_UPDATE_EVT,
                                  (void *)&data,
                                  sizeof(node_mgmt_manage_node_data_t));
  } else {
    status = SL_STATUS_FAIL;
  }

  return status;
}

sl_status_t zigpc_nodemgmt_remove_node(const zigbee_eui64_t eui64)
{
  sl_status_t status = SL_STATUS_OK;
  if (eui64 != NULL) {
    node_mgmt_remove_node_data_t data;
    memcpy(&data.eui64, eui64, ZIGBEE_EUI64_SIZE);

    zigpc_node_process_send_event(NODE_MGMT_REMOVE_EVT,
                                  (void *)&data,
                                  sizeof(node_mgmt_remove_node_data_t));
    status = SL_STATUS_OK;
  } else {
    status = SL_STATUS_FAIL;
  }

  return status;
}

bool zigpc_nodemgmt_endpoint_contains_cluster(const zigbee_endpoint_t endpoint,
                                              const zcl_cluster_id_t cluster_id)
{
  bool has_cluster = false;
  unsigned int i;

  for (i = 0; (i < endpoint.cluster_count) && (!has_cluster); i++) {
    zcl_cluster_id_t to_check = endpoint.cluster_list[i].cluster_id;

    has_cluster = (to_check == cluster_id);
  }

  return has_cluster;
}

bool zigpc_nodemgmt_managed_endpoint_contains_cluster(
  const zigbee_eui64_t eui64,
  const zigbee_endpoint_id_t endpoint_id,
  const zcl_cluster_id_t cluster_id)
{
  sl_status_t status;
  zcl_cluster_type_t cluster;

  bool has_cluster = false;

  status
    = nodemap_fetch_node_ep_cluster(eui64, endpoint_id, cluster_id, &cluster);

  has_cluster = (status == SL_STATUS_OK);

  return has_cluster;
}

bool zigpc_nodemgmt_managed_node_contains_cluster(
  const zigbee_eui64_t eui64, const zcl_cluster_id_t cluster_id)
{
  bool has_cluster = false;
  zigbee_node_t node;
  sl_status_t status = nodemap_fetch_node(eui64, &node);

  if (status != SL_STATUS_OK) {
    has_cluster = false;
  } else {
    unsigned int i;
    for (i = 0; (i < node.endpoint_count) && (!has_cluster); i++) {
      has_cluster
        = zigpc_nodemgmt_endpoint_contains_cluster(node.endpoint_list[i],
                                                   cluster_id);
    }
  }

  return has_cluster;
}

bool zigpc_node_check_sleepy(const zigbee_eui64_t eui64)
{
  //TODO
  //replace hardcoded value after poll control cluster info added
  //to zcl_profiles/zcl_util
  bool has_cluster
    = zigpc_nodemgmt_managed_node_contains_cluster(eui64, 0x0020);

  return has_cluster;
}

bool zigpc_nodemgmt_check_if_managed(const zigbee_eui64_t eui64)
{
  return nodemap_check_if_managed(eui64);
}
