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
#include <list>


// Unify shared components
#include <sl_status.h>
#include <sl_log.h>
#include <attribute_store.h>

// ZigPC components
#include <zigpc_common_zigbee.h>
#include <zigpc_common_util.hpp>

#include "zigpc_datastore.h"
#include "zigpc_datastore.hpp"
#include "zigpc_datastore_id.h"
#include "zigpc_datastore_accessor.hpp"
#include "zigpc_datastore_util.hpp"

/**********************************
 **********************************
 *
 * Network Entity APIs
 *
 **********************************
 **********************************/

sl_status_t zigpc_datastore_create_network()
{
  NetworkAccessor network;
  return network.create(attribute_store_get_root(), PRIMARY_NETWORK_ENTRY);
}

sl_status_t zigpc_datastore_read_network(zigpc_network_data_t *const data)
{
  NetworkAccessor network;
  return network.read(attribute_store_get_root(), PRIMARY_NETWORK_ENTRY, data);
}
sl_status_t zigpc_datastore_write_network(const zigpc_network_data_t *data)
{
  NetworkAccessor network;
  return network.write(attribute_store_get_root(), PRIMARY_NETWORK_ENTRY, data);
}

sl_status_t zigpc_datastore_remove_network(void)
{
  NetworkAccessor network;
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
  NetworkAccessor network;
  return network.child_count(attribute_store_get_root(),
                             PRIMARY_NETWORK_ENTRY,
                             ZIGPC_DS_TYPE_DEVICE);
}

sl_status_t zigpc_datastore_find_device_by_index(size_t index,
                                                 zigbee_eui64_t *eui64)
{
  zigbee_eui64_uint_t eui64_i;
  attribute_store_node_t parent = zigpc_datastore::util::get_device_parent();
  DeviceAccessor device;

  sl_status_t status = device.find_by_index(parent, index, &eui64_i);
  if (status == SL_STATUS_OK) {
    status = zigbee_uint_to_eui64(eui64_i, *eui64);
  }

  return status;
}

sl_status_t zigpc_datastore_create_device(const zigbee_eui64_t eui64)
{
  zigbee_eui64_uint_t eui64_i   = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent = zigpc_datastore::util::get_device_parent();
  DeviceAccessor device;

  return device.create(parent, eui64_i);
}

sl_status_t zigpc_datastore_read_device(const zigbee_eui64_t eui64,
                                        zigpc_device_data_t *const data)
{
  zigbee_eui64_uint_t eui64_i   = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent = zigpc_datastore::util::get_device_parent();
  DeviceAccessor device;

  return device.read(parent, eui64_i, data);
}

sl_status_t zigpc_datastore_write_device(const zigbee_eui64_t eui64,
                                         const zigpc_device_data_t *data)
{
  zigbee_eui64_uint_t eui64_i   = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent = zigpc_datastore::util::get_device_parent();
  DeviceAccessor device;

  return device.write(parent, eui64_i, data);
}

sl_status_t zigpc_datastore_remove_device(const zigbee_eui64_t eui64)
{
  zigbee_eui64_uint_t eui64_i   = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent = zigpc_datastore::util::get_device_parent();
  DeviceAccessor device;

  return device.remove(parent, eui64_i);
}

sl_status_t zigpc_datastore_remove_device_children(const zigbee_eui64_t eui64)
{
  zigbee_eui64_uint_t eui64_i   = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent = zigpc_datastore::util::get_device_parent();
  DeviceAccessor device;

  return device.remove_children(parent, eui64_i);
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
  attribute_store_node_t parent = zigpc_datastore::util::get_device_parent();
  DeviceAccessor device;

  return device.child_count(parent, eui64_i, ZIGPC_DS_TYPE_ENDPOINT);
}

sl_status_t zigpc_datastore_find_endpoint_by_index(
  const zigbee_eui64_t eui64, size_t index, zigbee_endpoint_id_t *endpoint_id)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore::util::get_endpoint_parent(eui64_i);
  EndpointAccessor endpoint;

  return endpoint.find_by_index(parent, index, endpoint_id);
}

sl_status_t zigpc_datastore_create_endpoint(const zigbee_eui64_t eui64,
                                            zigbee_endpoint_id_t endpoint_id)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore::util::get_endpoint_parent(eui64_i);
  EndpointAccessor endpoint;

  return endpoint.create(parent, endpoint_id);
}
sl_status_t zigpc_datastore_read_endpoint(const zigbee_eui64_t eui64,
                                          zigbee_endpoint_id_t endpoint_id,
                                          zigpc_endpoint_data_t *const data)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore::util::get_endpoint_parent(eui64_i);
  EndpointAccessor endpoint;

  return endpoint.read(parent, endpoint_id, data);
}
sl_status_t zigpc_datastore_write_endpoint(const zigbee_eui64_t eui64,
                                           zigbee_endpoint_id_t endpoint_id,
                                           const zigpc_endpoint_data_t *data)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore::util::get_endpoint_parent(eui64_i);
  EndpointAccessor endpoint;

  return endpoint.write(parent, endpoint_id, data);
}

sl_status_t zigpc_datastore_remove_endpoint(const zigbee_eui64_t eui64,
                                            zigbee_endpoint_id_t endpoint_id)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore::util::get_endpoint_parent(eui64_i);
  EndpointAccessor endpoint;

  return endpoint.remove(parent, endpoint_id);
}
/*********/


size_t zigpc_datastore_get_group_count(const zigbee_eui64_t eui64,
                                       zigbee_endpoint_id_t endpoint_id,
                                       bool is_reported)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore::util::get_endpoint_parent(eui64_i);
  EndpointAccessor endpoint;

  return endpoint.child_count(parent,
                              endpoint_id,
                              GroupAccessor::get_entity_type(is_reported));
}

sl_status_t
  zigpc_datastore_find_group_by_index(const zigbee_eui64_t eui64,
                                      zigbee_endpoint_id_t endpoint_id,
                                      bool is_reported,
                                      size_t index,
                                      zigbee_group_id_t *group_id)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore::util::get_cluster_parent(eui64_i, endpoint_id);
  GroupAccessor group(is_reported);

  return group.find_by_index(parent, index, group_id);
}

bool zigpc_datastore_contains_group(const zigbee_eui64_t eui64,
                                    zigbee_endpoint_id_t endpoint_id,
                                    bool is_reported,
                                    zigbee_group_id_t group_id)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore::util::get_cluster_parent(eui64_i, endpoint_id);
  GroupAccessor group(is_reported);

  attribute_store_node_t group_node = group.find(parent, group_id);

  bool is_found = !(group_node == ATTRIBUTE_STORE_INVALID_NODE);

  return is_found;
}

sl_status_t zigpc_datastore_create_group(const zigbee_eui64_t eui64,
                                         zigbee_endpoint_id_t endpoint_id,
                                         bool is_reported,
                                         zigbee_group_id_t group_id)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore::util::get_cluster_parent(eui64_i, endpoint_id);
  GroupAccessor group(is_reported);

  return group.create(parent, group_id);
}

sl_status_t zigpc_datastore_read_group(const zigbee_eui64_t eui64,
                                       zigbee_endpoint_id_t endpoint_id,
                                       bool is_reported,
                                       zigbee_group_id_t group_id,
                                       zigpc_group_data_t *const data)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore::util::get_cluster_parent(eui64_i, endpoint_id);
  GroupAccessor group(is_reported);

  return group.read(parent, group_id, data);
}

sl_status_t zigpc_datastore_write_group(const zigbee_eui64_t eui64,
                                        zigbee_endpoint_id_t endpoint_id,
                                        bool is_reported,
                                        zigbee_group_id_t group_id,
                                        const zigpc_group_data_t *data)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore::util::get_cluster_parent(eui64_i, endpoint_id);
  GroupAccessor group(is_reported);

  return group.write(parent, group_id, data);
}

sl_status_t zigpc_datastore_remove_group(const zigbee_eui64_t eui64,
                                         zigbee_endpoint_id_t endpoint_id,
                                         bool is_reported,
                                         zigbee_group_id_t group_id)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore::util::get_cluster_parent(eui64_i, endpoint_id);
  GroupAccessor group(is_reported);

  return group.remove(parent, group_id);
}

/**********************************
 *
 **********************************
 *
 * Cluster Entity APIs
 *
 **********************************
 **********************************/

size_t zigpc_datastore_get_cluster_count(const zigbee_eui64_t eui64,
                                         zigbee_endpoint_id_t endpoint_id,
                                         zcl_cluster_side_t cluster_side)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore::util::get_endpoint_parent(eui64_i);
  EndpointAccessor endpoint;

  return endpoint.child_count(parent,
                              endpoint_id,
                              ClusterAccessor::get_entity_type(cluster_side));
}

sl_status_t
  zigpc_datastore_find_cluster_by_index(const zigbee_eui64_t eui64,
                                        zigbee_endpoint_id_t endpoint_id,
                                        zcl_cluster_side_t cluster_side,
                                        size_t index,
                                        zcl_cluster_id_t *cluster_id)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore::util::get_cluster_parent(eui64_i, endpoint_id);
  ClusterAccessor cluster(cluster_side);

  return cluster.find_by_index(parent, index, cluster_id);
}

bool zigpc_datastore_contains_cluster(const zigbee_eui64_t eui64,
                                      zigbee_endpoint_id_t endpoint_id,
                                      zcl_cluster_side_t cluster_side,
                                      zcl_cluster_id_t cluster_id)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore::util::get_cluster_parent(eui64_i, endpoint_id);
  ClusterAccessor cluster(cluster_side);

  attribute_store_node_t cluster_node = cluster.find(parent, cluster_id);

  bool is_found = !(cluster_node == ATTRIBUTE_STORE_INVALID_NODE);

  return is_found;
}

sl_status_t zigpc_datastore_create_cluster(const zigbee_eui64_t eui64,
                                           zigbee_endpoint_id_t endpoint_id,
                                           zcl_cluster_side_t cluster_side,
                                           zcl_cluster_id_t cluster_id)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore::util::get_cluster_parent(eui64_i, endpoint_id);
  ClusterAccessor cluster(cluster_side);

  return cluster.create(parent, cluster_id);
}
sl_status_t zigpc_datastore_read_cluster(const zigbee_eui64_t eui64,
                                         zigbee_endpoint_id_t endpoint_id,
                                         zcl_cluster_side_t cluster_side,
                                         zcl_cluster_id_t cluster_id,
                                         zigpc_cluster_data_t *const data)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore::util::get_cluster_parent(eui64_i, endpoint_id);
  ClusterAccessor cluster(cluster_side);

  return cluster.read(parent, cluster_id, data);
}
sl_status_t zigpc_datastore_write_cluster(const zigbee_eui64_t eui64,
                                          zigbee_endpoint_id_t endpoint_id,
                                          zcl_cluster_side_t cluster_side,
                                          zcl_cluster_id_t cluster_id,
                                          const zigpc_cluster_data_t *data)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore::util::get_cluster_parent(eui64_i, endpoint_id);
  ClusterAccessor cluster(cluster_side);

  return cluster.write(parent, cluster_id, data);
}

sl_status_t zigpc_datastore_remove_cluster(const zigbee_eui64_t eui64,
                                           zigbee_endpoint_id_t endpoint_id,
                                           zcl_cluster_side_t cluster_side,
                                           zcl_cluster_id_t cluster_id)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore::util::get_cluster_parent(eui64_i, endpoint_id);
  ClusterAccessor cluster(cluster_side);

  return cluster.remove(parent, cluster_id);
}

/**********************************
 **********************************
 *
 * Cluster Attribute List APIs
 *
 **********************************
 **********************************/

size_t zigpc_datastore_get_attribute_count(const zigbee_eui64_t eui64,
                                           zigbee_endpoint_id_t endpoint_id,
                                           zcl_cluster_side_t cluster_side,
                                           zcl_cluster_id_t cluster_id)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore::util::get_cluster_parent(eui64_i, endpoint_id);
  ClusterAccessor cluster(cluster_side);

  return cluster.child_count(parent,
                             cluster_id,
                             ZIGPC_DS_TYPE_CLUSTER_ATTRIBUTE);
}

sl_status_t
  zigpc_datastore_read_attribute_list(const zigbee_eui64_t eui64,
                                      zigbee_endpoint_id_t endpoint_id,
                                      zcl_cluster_side_t cluster_side,
                                      zcl_cluster_id_t cluster_id,
                                      zcl_attribute_id_t *const attribute_list,
                                      size_t *attribute_count)
{
  sl_status_t status  = SL_STATUS_OK;
  auto cluster_parent = ATTRIBUTE_STORE_INVALID_NODE;
  size_t count        = 0U;
  ClusterAccessor cluster(cluster_side);

  if (attribute_list == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
    cluster_parent
      = zigpc_datastore::util::get_cluster_parent(eui64_i, endpoint_id);

    count = cluster.child_count(cluster_parent,
                                cluster_id,
                                ZIGPC_DS_TYPE_CLUSTER_ATTRIBUTE);
  }

  if (count > *attribute_count) {
    status = SL_STATUS_INVALID_COUNT;
  } else if (count > 0) {
    attribute_store_node_t parent = cluster.find(cluster_parent, cluster_id);
    ClusterAttributeListAccessor attributes;

    status = attributes.read_list(parent, attribute_list, *attribute_count);
  }
  *attribute_count = count;

  return status;
}

sl_status_t
  zigpc_datastore_write_attribute_list(const zigbee_eui64_t eui64,
                                       zigbee_endpoint_id_t endpoint_id,
                                       zcl_cluster_side_t cluster_side,
                                       zcl_cluster_id_t cluster_id,
                                       const zcl_attribute_id_t *attribute_list,
                                       size_t attribute_count)
{
  sl_status_t status  = SL_STATUS_OK;
  auto cluster_parent = ATTRIBUTE_STORE_INVALID_NODE;
  ClusterAccessor cluster(cluster_side);

  if (attribute_list == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
    cluster_parent
      = zigpc_datastore::util::get_cluster_parent(eui64_i, endpoint_id);
  }

  if (status == SL_STATUS_OK) {
    attribute_store_node_t attributes_parent
      = cluster.find(cluster_parent, cluster_id);
    ClusterAttributeListAccessor attributes;

    status = attributes.remove_list(attributes_parent);
    if (status == SL_STATUS_OK) {
      status = attributes.create_list(attributes_parent,
                                      attribute_list,
                                      attribute_count);
    }
  }

  return status;
}

/**********************************
 **********************************
 *
 * Cluster Command List APIs
 *
 **********************************
 **********************************/

size_t zigpc_datastore_get_command_count(const zigbee_eui64_t eui64,
                                         zigbee_endpoint_id_t endpoint_id,
                                         zcl_cluster_side_t cluster_side,
                                         zcl_cluster_id_t cluster_id,
                                         zcl_command_type_t command_type)
{
  zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent
    = zigpc_datastore::util::get_cluster_parent(eui64_i, endpoint_id);
  ClusterAccessor cluster(cluster_side);

  return cluster.child_count(
    parent,
    cluster_id,
    ClusterCommandListAccessor::get_entity_type(command_type));
}

sl_status_t
  zigpc_datastore_read_command_list(const zigbee_eui64_t eui64,
                                    zigbee_endpoint_id_t endpoint_id,
                                    zcl_cluster_side_t cluster_side,
                                    zcl_cluster_id_t cluster_id,
                                    zcl_command_type_t command_type,
                                    zcl_command_id_t *const command_list,
                                    size_t *command_count)
{
  sl_status_t status  = SL_STATUS_OK;
  auto cluster_parent = ATTRIBUTE_STORE_INVALID_NODE;
  size_t count        = 0U;
  ClusterAccessor cluster(cluster_side);

  if (command_list == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
    cluster_parent
      = zigpc_datastore::util::get_cluster_parent(eui64_i, endpoint_id);

    count = cluster.child_count(
      cluster_parent,
      cluster_id,
      ClusterCommandListAccessor::get_entity_type(command_type));
  }

  if (count > *command_count) {
    status = SL_STATUS_INVALID_COUNT;
  } else if (count > 0) {
    attribute_store_node_t command_parent
      = cluster.find(cluster_parent, cluster_id);
    ClusterCommandListAccessor commands(command_type);

    status = commands.read_list(command_parent, command_list, *command_count);
  }
  *command_count = count;

  return status;
}

sl_status_t
  zigpc_datastore_write_command_list(const zigbee_eui64_t eui64,
                                     zigbee_endpoint_id_t endpoint_id,
                                     zcl_cluster_side_t cluster_side,
                                     zcl_cluster_id_t cluster_id,
                                     zcl_command_type_t command_type,
                                     const zcl_command_id_t *command_list,
                                     size_t command_count)
{
  sl_status_t status  = SL_STATUS_OK;
  auto cluster_parent = ATTRIBUTE_STORE_INVALID_NODE;
  ClusterAccessor cluster(cluster_side);

  if (command_list == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    zigbee_eui64_uint_t eui64_i = zigbee_eui64_to_uint(eui64);
    cluster_parent
      = zigpc_datastore::util::get_cluster_parent(eui64_i, endpoint_id);
  }

  if (status == SL_STATUS_OK) {
    attribute_store_node_t commands_parent
      = cluster.find(cluster_parent, cluster_id);
    ClusterCommandListAccessor commands(command_type);

    status = commands.remove_list(commands_parent);
    if (status == SL_STATUS_OK) {
      status
        = commands.create_list(commands_parent, command_list, command_count);
    }
  }

  return status;
}

/**********************************
 **********************************
 *
 * Utility Functions
 *
 **********************************
 **********************************/

sl_status_t zigpc_datastore_find_endpoint_containing_cluster(
  const zigbee_eui64_t eui64,
  zcl_cluster_side_t cluster_side,
  zcl_cluster_id_t cluster_id,
  zigbee_endpoint_id_t *const endpoint_id)
{
  sl_status_t status = SL_STATUS_NOT_FOUND;

  std::vector<zigbee_endpoint_id_t> endpoint_list
    = zigpc_datastore::endpoint::get_id_list(eui64);

  for (zigbee_endpoint_id_t ep_to_check: endpoint_list) {
    bool is_found = zigpc_datastore_contains_cluster(eui64,
                                                     ep_to_check,
                                                     cluster_side,
                                                     cluster_id);
    if (is_found == true) {
      *endpoint_id = ep_to_check;
      status       = SL_STATUS_OK;
      break;
    }
  }

  if (status == SL_STATUS_NOT_FOUND) {
    *endpoint_id = ZIGBEE_INVALID_APP_ENDPOINT_ID;
  }

  return status;
}

void zigpc_datastore_log_device_disc_state(const zigbee_eui64_t eui64)
{
  if (eui64 != nullptr) {
    zigbee_eui64_uint_t device_id = zigbee_eui64_to_uint(eui64);
    attribute_store_node_t dev_parent
      = zigpc_datastore::util::get_device_parent();
    DeviceAccessor device;

    std::string dev_log(device.to_str(dev_parent, device_id));
    if (!dev_log.empty()) {
      sl_log_info(LOG_TAG, "Device: %s", dev_log.c_str());

      zigpc_datastore::util::log_endpoints_under_dev(dev_parent,
                                                     device,
                                                     device_id);
    } else {
      sl_log_info(LOG_TAG, "Device NOT FOUND");
    }
  }
}

sl_status_t zigpc_datastore_log_network(const char *log_tag,
                                        sl_log_level_t log_level,
                                        const char *log_prefix)
{
  sl_status_t status = SL_STATUS_OK;
  NetworkAccessor network;
  std::string nwk_info(
    network.to_str(attribute_store_get_root(), PRIMARY_NETWORK_ENTRY));

  if ((log_tag == nullptr) || (log_prefix == nullptr)) {
    status = SL_STATUS_NULL_POINTER;
  } else if (nwk_info.empty()) {
    status = SL_STATUS_NOT_FOUND;
  } else {
    sl_log(log_tag, log_level, "%s: %s", log_prefix, nwk_info.c_str());
  }

  return status;
}

sl_status_t zigpc_datastore_log_device(const char *log_tag,
                                       sl_log_level_t log_level,
                                       const char *log_prefix,
                                       const zigbee_eui64_t eui64)
{
  sl_status_t status            = SL_STATUS_OK;
  zigbee_eui64_uint_t eui64_i   = zigbee_eui64_to_uint(eui64);
  attribute_store_node_t parent = zigpc_datastore::util::get_device_parent();
  DeviceAccessor device;

  std::string dev_info(device.to_str(parent, eui64_i));

  if ((log_tag == nullptr) || (log_prefix == nullptr)) {
    status = SL_STATUS_NULL_POINTER;
  } else if (dev_info.empty()) {
    status = SL_STATUS_NOT_FOUND;
  } else {
    sl_log(log_tag, log_level, "%s: %s", log_prefix, dev_info.c_str());
  }

  return status;
}
