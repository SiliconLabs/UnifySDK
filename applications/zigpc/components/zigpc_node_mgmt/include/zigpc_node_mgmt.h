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

/**
 * @file zigpc_node_mgmt.h
 * @defgroup zigpc_node_mgmt ZigPC Node Management component
 * @ingroup zigpc_components
 * @brief API for node management. Basic operations involving nodes and node
 * endpoints and functions for storing and accessing node information.
 *
 * @{
 *
**/

#ifndef ZIGPC_NODE_MGMT_H
#define ZIGPC_NODE_MGMT_H

#include "sl_status.h"
#include "zigpc_common_zigbee.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start tracking a new node based on the EUI64
 *
 * @param eui64         Zigbee device identifier.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_FAIL if not.
 */
sl_status_t zigpc_nodemgmt_manage_node_eui64(const zigbee_eui64_t eui64);

/**
 * @brief Start tracking a new node based on the given node object
 *
 * @param eui64         Zigbee node object.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_FAIL if not.
 */
sl_status_t zigpc_nodemgmt_manage_node(const zigbee_node_t node);

/**
 * @brief Start tracking a new node based on the EUI64/endpoint combination.
 *
 * @param eui64         Zigbee device identifier.
 * @param endpoint      Zigbee device endpoint information.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_FAIL if not.
 */
sl_status_t zigpc_nodemgmt_add_node_endpoint(const zigbee_eui64_t eui64,
                                             const zigbee_endpoint_t endpoint);

/**
 * @brief Retrieve node information.
 *
 * @param eui64         Zigbee device identifier.
 * @param node          Pointer to store node information.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_FAIL if not.
 */
sl_status_t zigpc_nodemgmt_fetch_node(const zigbee_eui64_t eui64,
                                      zigbee_node_t *node);

/**
 * @brief Retrieve node endpoint cluster information.
 *
 * @param eui64         Zigbee device identifier.
 * @param endpoint_id   Zigbee device endpoint identifier.
 * @param cluster_id    Zigbee device endpoint cluster identifier.
 * @param node          Pointer to store endpoint information.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_FAIL if not.
 */
sl_status_t
  zigpc_nodemgmt_fetch_node_ep_cluster(const zigbee_eui64_t eui64,
                                       const zigbee_endpoint_id_t endpoint_id,
                                       const zcl_cluster_id_t cluster_id,
                                       zcl_cluster_type_t *cluster);

/**
 * @brief Retrieve endpoint id of the first endpoint with a given cluster.
 *
 * @param eui64         Zigbee device identifier.
 * @param cluster_id    Zigbee device endpoint cluster identifier.
 * @return the endpoint id of the endpoint with a given cluster. 0 if not found
 */
zigbee_endpoint_id_t zigpc_nodemgmt_fetch_endpoint_containing_cluster(
  const zigbee_eui64_t eui64, const zcl_cluster_id_t cluster_id);
/**
 * @brief Update the state of one node that is tracked.
 *
 * @param node          Updated node information.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_FAIL if not.
 */
sl_status_t zigpc_nodemgmt_update_node(const zigbee_node_t node);

/**
 * @brief Remove a node that is tracked based on EUI64.
 *
 * @param eui64         Zigbee device identifier
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_FAIL if not.
 */
sl_status_t zigpc_nodemgmt_remove_node(const zigbee_eui64_t eui64);

/**
 * @brief zigpc_nodemgmt_endpoint_contains_cluster.
 * Check if a specific endpoint contains a cluster
 *
 * @param endpoint - the endpoint
 * @param cluster_id - the cluster id to check
 *
 * @ return TRUE if the endpoint contains a cluster with a given ID
 */
bool zigpc_nodemgmt_endpoint_contains_cluster(
  const zigbee_endpoint_t endpoint, const zcl_cluster_id_t cluster_id);

/**
 * @brief zigpc_nodemgmt_managed_endpoint_contains_cluster
 * Check if a previously managed node+endpoint (whose info has been stored)
 * has a specific cluster ID
 *
 * @param eui64         Zigbee device identifier
 * @param endpoint_id - the endpoint ID to check on
 * @param cluster_id - the cluster ID to check
 *
 * @return TRUE if the specified endpoint has a cluster with
 * the expected cluster ID
 */
bool zigpc_nodemgmt_managed_endpoint_contains_cluster(
  const zigbee_eui64_t eui64,
  const zigbee_endpoint_id_t endpoint_id,
  const zcl_cluster_id_t cluster_id);

/**
 * @brief zigpc_nodemgmt_managed_node_contains_cluster
 * Check if a previously managed node has any endpoint
 * with a specific cluster ID
 *
 * @param eui64 - the EUI64 of the specified node
 * @param cluster_id - the cluster ID to check
 *
 * @return TRUE if any endpoint has a cluster of
 * the specified type
 */
bool zigpc_nodemgmt_managed_node_contains_cluster(
  const zigbee_eui64_t eui64, const zcl_cluster_id_t cluster_id);

/**
 * @brief zigpc_node_check_sleepy
 * Check if a managed node is a sleepy end device
 *
 * @param eui64 - the EUI64 of the managed node
 * @return TRUE if the node is a sleepy end device
 */
bool zigpc_node_check_sleepy(const zigbee_eui64_t eui64);

/**
 * @brief zigc_node_check_if_managed
 * Checks if an eui64 is managed by the zigpc
 *
 * @param eui64 - the EUI64 to check
 * @return TRUE if the node is managed
 **/
bool zigpc_nodemgmt_check_if_managed(const zigbee_eui64_t eui64);
#ifdef __cplusplus
}
#endif

#endif  //ZIGPC_NODE_MGMT_H
/**@} end zigpc_node_mgmt **/
