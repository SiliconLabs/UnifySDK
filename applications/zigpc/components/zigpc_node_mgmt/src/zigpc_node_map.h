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
 * @file zigpc_node_map.h
 * @defgroup zigpc_node_map ZigPC Node Storage Map
 * @ingroup zigpc_node_mgmt
 * @brief API for the node map, which is used to store node information for the
 * node management component
 *
 * @{
 *
**/

#ifndef ZIGPC_NODE_MAP_H
#define ZIGPC_NODE_MAP_H

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
sl_status_t nodemap_manage_node_eui64(const zigbee_eui64_t eui64);

/**
 * @brief Start tracking a new node based on the given node object
 *
 * @param eui64         Zigbee node object.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_FAIL if not.
 */
sl_status_t nodemap_manage_node(const zigbee_node_t node);

/**
 * @brief Start tracking a new node based on the EUI64/endpoint combination.
 *
 * @param eui64         Zigbee device identifier.
 * @param endpoint      Zigbee device endpoint information.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_FAIL if not.
 */
sl_status_t nodemap_add_node_endpoint(const zigbee_eui64_t eui64,
                                      const zigbee_endpoint_t endpoint);

/**
 * @brief Retrieve node information.
 *
 * @param eui64         Zigbee device identifier.
 * @param node          Pointer to store node information.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_FAIL if not.
 */
sl_status_t nodemap_fetch_node(const zigbee_eui64_t eui64, zigbee_node_t *node);

/**
 * @brief Retrieve node endpoint information.
 *
 * @param eui64         Zigbee device identifier.
 * @param endpoint_id   Zigbee device endpoint identifier.
 * @param node          Pointer to store endpoint information.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_FAIL if not.
 */
sl_status_t nodemap_fetch_node_ep(const zigbee_eui64_t eui64,
                                  const zigbee_endpoint_id_t endpoint_id,
                                  zigbee_endpoint_t *endpoint);

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
  nodemap_fetch_node_ep_cluster(const zigbee_eui64_t eui64,
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
zigbee_endpoint_id_t
  nodemap_fetch_endpoint_containing_cluster(const zigbee_eui64_t eui64,
                                            const zcl_cluster_id_t cluster_id);
/**
 * @brief Update the state of one node that is tracked.
 *
 * @param node          Updated node information.
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_FAIL if not.
 */
sl_status_t nodemap_update_node(const zigbee_node_t node);

/**
 * @brief Remove a node that is tracked based on EUI64.
 *
 * @param eui64         Zigbee device identifier
 * @return sl_status_t  SL_STATUS_OK on success, SL_STATUS_FAIL if not.
 */
sl_status_t nodemap_remove_node(const zigbee_eui64_t eui64);

bool nodemap_check_if_managed(const zigbee_eui64_t eui64);
#ifdef __cplusplus
}
#endif

#endif  // ZIGPC_NODE_MAP_H
/** @} end zigpc_node_map **/
