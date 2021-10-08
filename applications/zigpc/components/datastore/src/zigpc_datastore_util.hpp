/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA);; available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

/**
 * @defgroup zigpc_datastore_util ZigPC datastore utility functions.
 * @ingroup zigpc_datastore
 * @brief Datastore related utility functions.
 *
 * @{
 */

#ifndef ZIGPC_DATASTORE_UTIL_HPP
#define ZIGPC_DATASTORE_UTIL_HPP

#include "zigpc_datastore_id.h"

/**
 * @brief Default network index to use.
 *
 */
static const uint8_t PRIMARY_NETWORK_ENTRY = PRIMARY_NETWORK_NUM;

/**********************************
 **********************************
 *
 * Utility Entity Find API
 *
 **********************************
 **********************************/

namespace zigpc_datastore
{
namespace util
{
/**
 * @brief Retrieve the attribute store node ID of the device parent,
 * the network entity.
 *
 * @return attribute_store_node_t Parent network attribute store node ID, or
 * ATTRIBUTE_STORE_INVALID_NODE if not found.
 */
inline attribute_store_node_t get_device_parent(void)
{
  NetworkAccessor network;
  return network.find(attribute_store_get_root(), PRIMARY_NETWORK_ENTRY);
}

/**
 * @brief Retrieve the attribute store node ID of the endpoint parent (the
 * device entity)
 *
 * @param eui64                   Device identifier.
 * @return attribute_store_node_t Parent device attribute store node ID, or
 * ATTRIBUTE_STORE_INVALID_NODE if not found.
 */
inline attribute_store_node_t get_endpoint_parent(zigbee_eui64_uint_t eui64)
{
  attribute_store_node_t nwk_node;
  NetworkAccessor network;
  DeviceAccessor device;

  nwk_node = network.find(attribute_store_get_root(), PRIMARY_NETWORK_ENTRY);
  return device.find(nwk_node, eui64);
}

/**
 * @brief Retrieve the attribute store node ID of the cluster parent (the
 * endpoint entity)
 *
 * @param eui64                   Device identifier.
 * @param endpoint_id             Endpoint identifier.
 * @return attribute_store_node_t Parent endpoint attribute store node ID, or
 * ATTRIBUTE_STORE_INVALID_NODE if not found.
 */
inline attribute_store_node_t
  get_cluster_parent(zigbee_eui64_uint_t eui64,
                     zigbee_endpoint_id_t endpoint_id)
{
  attribute_store_node_t nwk_node;
  attribute_store_node_t dev_node;
  NetworkAccessor network;
  DeviceAccessor device;
  EndpointAccessor endpoint;

  nwk_node = network.find(attribute_store_get_root(), PRIMARY_NETWORK_ENTRY);
  dev_node = device.find(nwk_node, eui64);
  return endpoint.find(dev_node, endpoint_id);
}

/**
 * @brief Helper to log attributes under a cluster
 *
 * @param cluster_parent  Parent entity node ID (Endpoint).
 * @param cluster         Cluster Accessor.
 * @param cluster_side    Cluster side (server/client).
 * @param cluster_id      Cluster Identifier.
 */
void log_attributes_under_cluster(attribute_store_node_t cluster_parent,
                                  ClusterAccessor &cluster,
                                  zcl_cluster_side_t cluster_side,
                                  zcl_cluster_id_t cluster_id);

/**
 * @brief Helper to log clusters under an enedpoint
 *
 * @param ep_parent     Parent entity node ID (Device).
 * @param endpoint      Endpoint Accessor.
 * @param endpoint_id   Endpoint identifier.
 * @param clsuter_side  Cluster side (server/client).
 */
void log_clusters_under_ep(attribute_store_node_t ep_parent,
                           EndpointAccessor &endpoint,
                           zigbee_endpoint_id_t endpoint_id,
                           zcl_cluster_side_t cluster_side);

/**
 * @brief Helper to log clusters under an enedpoint
 *
 * @param dev_parent  Parent entity node ID (Network).
 * @param device      Device Accessor.
 * @param device_id   Device identifier.
 */
void log_endpoints_under_dev(attribute_store_node_t dev_parent,
                             DeviceAccessor &device,
                             zigbee_eui64_uint_t device_id);

}  // namespace util
}  // namespace zigpc_datastore

/** @} end zigpc_datastore_util */

#endif  //ZIGPC_DATASTORE_UTIL_HPP
