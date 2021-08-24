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

// UIC shared components
#include "sl_status.h"
#include "sl_log.h"
#include "attribute_store.h"

// ZigPC components
#include "zigpc_common_zigbee.h"

#include "zigpc_datastore.h"
#include "zigpc_datastore_id.h"
#include "zigpc_datastore_helper.hpp"

/**
 * @brief Default network index to use.
 *
 */
static const uint8_t PRIMARY_NETWORK_ENTRY = PRIMARY_NETWORK_NUM;

// Parent node find helpers

/**
 * @brief Retrieve the attribute store node ID of the device parent,
 * the network entity.
 *
 * @return attribute_store_node_t Parent network attribute store node ID, or
 * ATTRIBUTE_STORE_INVALID_NODE if not found.
 */
static inline attribute_store_node_t
  zigpc_datastore_helper_get_device_parent(void)
{
  NetworkStoreHelper network;
  return network.find(attribute_store_get_root(), PRIMARY_NETWORK_ENTRY);
}

/**
 * @brief Retrieve the attribute store node ID of the endpoint parent (the
 * device entity)
 *
 * @param eui64
 * @return attribute_store_node_t Parent device attribute store node ID, or
 * ATTRIBUTE_STORE_INVALID_NODE if not found.
 */
static inline attribute_store_node_t
  zigpc_datastore_helper_get_endpoint_parent(zigbee_eui64_uint_t eui64)
{
  attribute_store_node_t nwk_node;
  NetworkStoreHelper network;
  DeviceStoreHelper device;

  nwk_node = network.find(attribute_store_get_root(), PRIMARY_NETWORK_ENTRY);
  return device.find(nwk_node, eui64);
}

/**
 * @brief Retrieve the attribute store node ID of the cluster parent (the
 * endpoint entity)
 *
 * @param eui64
 * @param endpoint_id
 * @return attribute_store_node_t Parent endpoint attribute store node ID, or
 * ATTRIBUTE_STORE_INVALID_NODE if not found.
 */
static inline attribute_store_node_t
  zigpc_datastore_helper_get_cluster_parent(zigbee_eui64_uint_t eui64,
                                            zigbee_endpoint_id_t endpoint_id)
{
  attribute_store_node_t nwk_node;
  attribute_store_node_t dev_node;
  NetworkStoreHelper network;
  DeviceStoreHelper device;
  EndpointStoreHelper endpoint;

  nwk_node = network.find(attribute_store_get_root(), PRIMARY_NETWORK_ENTRY);
  dev_node = device.find(nwk_node, eui64);
  return endpoint.find(dev_node, endpoint_id);
}

/**********************************
 **********************************
 *
 * Network Entity APIs
 *
 **********************************
 **********************************/

sl_status_t zigpc_datastore_create_network()
{
  NetworkStoreHelper network;
  return network.create(attribute_store_get_root(), PRIMARY_NETWORK_ENTRY);
}

sl_status_t zigpc_datastore_read_network(zigpc_network_data_t *const data)
{
  NetworkStoreHelper network;
  return network.read(attribute_store_get_root(), PRIMARY_NETWORK_ENTRY, data);
}
sl_status_t zigpc_datastore_write_network(const zigpc_network_data_t *data)
{
  NetworkStoreHelper network;
  return network.write(attribute_store_get_root(), PRIMARY_NETWORK_ENTRY, data);
}

sl_status_t zigpc_datastore_remove_network(void)
{
  NetworkStoreHelper network;
  return network.remove(attribute_store_get_root(), PRIMARY_NETWORK_ENTRY);
}

/**********************************
 **********************************
 *
 * Device Entity APIs
 *
 **********************************
 **********************************/

size_t zigpc_datastore_get_device_count(void)
{
  NetworkStoreHelper network;
  return network.child_count(attribute_store_get_root(), PRIMARY_NETWORK_ENTRY);
}

sl_status_t zigpc_datastore_find_device_by_index(size_t index,
                                                 zigbee_eui64_t *eui64)
{
  zigbee_eui64_uint_t eui64_i;
  attribute_store_node_t parent = zigpc_datastore_helper_get_device_parent();
  DeviceStoreHelper device;

  sl_status_t status = device.find_by_index(parent, index, &eui64_i);
  if (status == SL_STATUS_OK) {
    status = zigbee_uint_to_eui64(eui64_i, *eui64);
  }

  return status;
}

sl_status_t zigpc_datastore_create_device(const zigbee_eui64_t eui64)
{
  zigbee_eui64_uint_t eui64_i   = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent = zigpc_datastore_helper_get_device_parent();
  DeviceStoreHelper device;

  return device.create(parent, eui64_i);
}

sl_status_t zigpc_datastore_read_device(const zigbee_eui64_t eui64,
                                        zigpc_device_data_t *const data)
{
  zigbee_eui64_uint_t eui64_i   = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent = zigpc_datastore_helper_get_device_parent();
  DeviceStoreHelper device;

  return device.read(parent, eui64_i, data);
}

sl_status_t zigpc_datastore_write_device(const zigbee_eui64_t eui64,
                                         const zigpc_device_data_t *data)
{
  zigbee_eui64_uint_t eui64_i   = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent = zigpc_datastore_helper_get_device_parent();
  DeviceStoreHelper device;

  return device.write(parent, eui64_i, data);
}

sl_status_t zigpc_datastore_remove_device(const zigbee_eui64_t eui64)
{
  zigbee_eui64_uint_t eui64_i   = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent = zigpc_datastore_helper_get_device_parent();
  DeviceStoreHelper device;

  return device.remove(parent, eui64_i);
}

/**********************************
 **********************************
 *
 * Endpoint Entity APIs
 *
 **********************************
 **********************************/

size_t zigpc_datastore_get_endpoint_count(const zigbee_eui64_t eui64)
{
  zigbee_eui64_uint_t eui64_i   = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent = zigpc_datastore_helper_get_device_parent();
  DeviceStoreHelper device;

  return device.child_count(parent, eui64_i);
}

sl_status_t zigpc_datastore_find_endpoint_by_index(
  const zigbee_eui64_t eui64, size_t index, zigbee_endpoint_id_t *endpoint_id)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore_helper_get_endpoint_parent(eui64_i);
  EndpointStoreHelper endpoint;

  return endpoint.find_by_index(parent, index, endpoint_id);
}

sl_status_t zigpc_datastore_create_endpoint(const zigbee_eui64_t eui64,
                                            zigbee_endpoint_id_t endpoint_id)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore_helper_get_endpoint_parent(eui64_i);
  EndpointStoreHelper endpoint;

  return endpoint.create(parent, endpoint_id);
}
sl_status_t zigpc_datastore_read_endpoint(const zigbee_eui64_t eui64,
                                          zigbee_endpoint_id_t endpoint_id,
                                          zigpc_endpoint_data_t *const data)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore_helper_get_endpoint_parent(eui64_i);
  EndpointStoreHelper endpoint;

  return endpoint.read(parent, endpoint_id, data);
}
sl_status_t zigpc_datastore_write_endpoint(const zigbee_eui64_t eui64,
                                           zigbee_endpoint_id_t endpoint_id,
                                           const zigpc_endpoint_data_t *data)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore_helper_get_endpoint_parent(eui64_i);
  EndpointStoreHelper endpoint;

  return endpoint.write(parent, endpoint_id, data);
}

sl_status_t zigpc_datastore_remove_endpoint(const zigbee_eui64_t eui64,
                                            zigbee_endpoint_id_t endpoint_id)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore_helper_get_endpoint_parent(eui64_i);
  EndpointStoreHelper endpoint;

  return endpoint.remove(parent, endpoint_id);
}

/**********************************
 **********************************
 *
 * Cluster Entity APIs
 *
 **********************************
 **********************************/

size_t zigpc_datastore_get_cluster_count(const zigbee_eui64_t eui64,
                                         zigbee_endpoint_id_t endpoint_id)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore_helper_get_endpoint_parent(eui64_i);
  EndpointStoreHelper endpoint;

  return endpoint.child_count(parent, endpoint_id);
}

sl_status_t
  zigpc_datastore_find_cluster_by_index(const zigbee_eui64_t eui64,
                                        zigbee_endpoint_id_t endpoint_id,
                                        size_t index,
                                        zcl_cluster_id_t *cluster_id)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore_helper_get_cluster_parent(eui64_i, endpoint_id);
  ClusterStoreHelper cluster;

  return cluster.find_by_index(parent, index, cluster_id);
}

sl_status_t zigpc_datastore_create_cluster(const zigbee_eui64_t eui64,
                                           zigbee_endpoint_id_t endpoint_id,
                                           zcl_cluster_id_t cluster_id)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore_helper_get_cluster_parent(eui64_i, endpoint_id);
  ClusterStoreHelper cluster;

  return cluster.create(parent, cluster_id);
}
sl_status_t zigpc_datastore_read_cluster(const zigbee_eui64_t eui64,
                                         zigbee_endpoint_id_t endpoint_id,
                                         zcl_cluster_id_t cluster_id,
                                         zigpc_cluster_data_t *const data)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore_helper_get_cluster_parent(eui64_i, endpoint_id);
  ClusterStoreHelper cluster;

  return cluster.read(parent, cluster_id, data);
}
sl_status_t zigpc_datastore_write_cluster(const zigbee_eui64_t eui64,
                                          zigbee_endpoint_id_t endpoint_id,
                                          zcl_cluster_id_t cluster_id,
                                          const zigpc_cluster_data_t *data)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore_helper_get_cluster_parent(eui64_i, endpoint_id);
  ClusterStoreHelper cluster;

  return cluster.write(parent, cluster_id, data);
}

sl_status_t zigpc_datastore_remove_cluster(const zigbee_eui64_t eui64,
                                           zigbee_endpoint_id_t endpoint_id,
                                           zcl_cluster_id_t cluster_id)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore_helper_get_cluster_parent(eui64_i, endpoint_id);
  ClusterStoreHelper cluster;

  return cluster.remove(parent, cluster_id);
}
