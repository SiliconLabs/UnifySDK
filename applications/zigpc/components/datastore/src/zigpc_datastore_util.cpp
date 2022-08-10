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

// Unify shared components
#include <sl_status.h>
#include <sl_log.h>
#include <attribute_store.h>

// ZigPC components
#include <zigpc_common_zigbee.h>

#include "zigpc_datastore.h"
#include "zigpc_datastore.hpp"
#include "zigpc_datastore_id.h"
#include "zigpc_datastore_accessor.hpp"
#include "zigpc_datastore_util.hpp"

/**********************************
 **********************************
 *
 * Utility Logging API
 *
 **********************************
 **********************************/

namespace zigpc_datastore
{
namespace util
{
void log_attributes_under_cluster(attribute_store_node_t cluster_parent,
                                  ClusterAccessor &cluster,
                                  zcl_cluster_id_t cluster_id)
{
  attribute_store_node_t attr_parent = cluster.find(cluster_parent, cluster_id);
  if (attr_parent != ATTRIBUTE_STORE_INVALID_NODE) {
    ClusterAttributeListAccessor attributes;

    size_t attr_count = cluster.child_count(cluster_parent,
                                            cluster_id,
                                            ZIGPC_DS_TYPE_CLUSTER_ATTRIBUTE);

    std::string attrs_log(attributes.to_str(attr_parent, attr_count));

    sl_log_info(LOG_TAG, "    |- %s", attrs_log.c_str());
  }
}

void log_commands_under_cluster(attribute_store_node_t cluster_parent,
                                ClusterAccessor &cluster,
                                zcl_cluster_id_t cluster_id,
                                zcl_command_type_t command_type)
{
  attribute_store_node_t cmd_parent = cluster.find(cluster_parent, cluster_id);
  if (cmd_parent != ATTRIBUTE_STORE_INVALID_NODE) {
    ClusterCommandListAccessor commands(command_type);

    size_t cmd_count = cluster.child_count(
      cluster_parent,
      cluster_id,
      ClusterCommandListAccessor::get_entity_type(command_type));

    std::string cmds_log(commands.to_str(cmd_parent, cmd_count));
    sl_log_info(LOG_TAG, "    |- %s", cmds_log.c_str());
  }
}

void log_clusters_under_ep(attribute_store_node_t ep_parent,
                           EndpointAccessor &endpoint,
                           zigbee_endpoint_id_t endpoint_id,
                           zcl_cluster_side_t cluster_side)
{
  attribute_store_node_t cluster_parent = endpoint.find(ep_parent, endpoint_id);

  if (cluster_parent != ATTRIBUTE_STORE_INVALID_NODE) {
    ClusterAccessor cluster(cluster_side);
    size_t cluster_count
      = endpoint.child_count(ep_parent,
                             endpoint_id,
                             ClusterAccessor::get_entity_type(cluster_side));

    for (size_t cl_i = 0; cl_i < cluster_count; cl_i++) {
      zcl_cluster_id_t cluster_id;
      sl_status_t status
        = cluster.find_by_index(cluster_parent, cl_i, &cluster_id);
      if (status == SL_STATUS_OK) {
        std::string cl_log(cluster.to_str(cluster_parent, cluster_id));
        sl_log_info(LOG_TAG, "  |- %s", cl_log.c_str());

        log_attributes_under_cluster(cluster_parent, cluster, cluster_id);
        log_commands_under_cluster(cluster_parent,
                                   cluster,
                                   cluster_id,
                                   ZCL_CLUSTER_RECEIVED_COMMAND);

        log_commands_under_cluster(cluster_parent,
                                   cluster,
                                   cluster_id,
                                   ZCL_CLUSTER_GENERATED_COMMAND);
      }
    }
  }
}

void log_endpoints_under_dev(attribute_store_node_t dev_parent,
                             DeviceAccessor &device,
                             zigbee_eui64_uint_t device_id)
{
  attribute_store_node_t ep_parent = device.find(dev_parent, device_id);

  if (ep_parent != ATTRIBUTE_STORE_INVALID_NODE) {
    EndpointAccessor endpoint;
    size_t ep_count
      = device.child_count(dev_parent, device_id, ZIGPC_DS_TYPE_ENDPOINT);
    for (size_t ep_i = 0; ep_i < ep_count; ep_i++) {
      zigbee_endpoint_id_t endpoint_id;
      sl_status_t status
        = endpoint.find_by_index(ep_parent, ep_i, &endpoint_id);
      if (status == SL_STATUS_OK) {
        std::string ep_log(endpoint.to_str(ep_parent, endpoint_id));
        sl_log_info(LOG_TAG, "|- Endpoint: %s", ep_log.c_str());

        log_clusters_under_ep(ep_parent,
                              endpoint,
                              endpoint_id,
                              ZCL_CLUSTER_SERVER_SIDE);
        log_clusters_under_ep(ep_parent,
                              endpoint,
                              endpoint_id,
                              ZCL_CLUSTER_CLIENT_SIDE);
      }
    }
  }
}

}  // namespace util

}  // namespace zigpc_datastore
