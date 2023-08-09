/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "zpc_dotdot_mqtt_group_dispatch.h"

// Unify Includes
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "dotdot_mqtt.h"
#include "sl_log.h"
#include "unify_dotdot_attribute_store_group_cluster.h"

// ZPC Includes
#include "zpc_attribute_store.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"

// Generic includes
#include <string>

constexpr char LOG_TAG[] = "zpc_dotdot_mqtt_group_dispatch";

///////////////////////////////////////////////////////////////////////////////
// Helper functions
///////////////////////////////////////////////////////////////////////////////
static void zpc_dotdot_mqtt_dispatch_to_endpoints_under_node(
  uint16_t group_id,
  attribute_store_node_t node_id_node,
  unid_t unid,
  const char *cluster_name,
  const char *command,
  const char *message,
  size_t message_length,
  mqtt_message_callback_t callback)
{
  // Now iterate through endpoints
  uint32_t endpoint_id_child_index = 0;
  attribute_store_node_t endpoint_id_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_ENDPOINT_ID,
                                             endpoint_id_child_index);

  while (endpoint_id_node != ATTRIBUTE_STORE_INVALID_NODE) {
    zwave_endpoint_id_t endpoint_id = 0;
    if (SL_STATUS_OK
        != attribute_store_read_value(endpoint_id_node,
                                      REPORTED_ATTRIBUTE,
                                      &endpoint_id,
                                      sizeof(zwave_endpoint_id_t))) {
      sl_log_debug(LOG_TAG,
                   "Error retrieving Endpoint ID for Attribute Store node %d.",
                   endpoint_id_node);
      // Move to the next Endpoint
      endpoint_id_child_index += 1;
      endpoint_id_node
        = attribute_store_get_node_child_by_type(node_id_node,
                                                 ATTRIBUTE_ENDPOINT_ID,
                                                 endpoint_id_child_index);
      continue;
    }

    if (is_unid_endpoint_in_group(unid, endpoint_id, group_id) == true) {
      sl_log_debug(LOG_TAG,
                   "Dispatching Group ID %d Command to UNID %s Endpoint %d",
                   group_id,
                   unid,
                   endpoint_id);
      // Call the command handler on the individual node:
      std::string new_topic = "ucl/by-unid/" + std::string(unid) + "/ep"
                              + std::to_string(endpoint_id) + "/"
                              + std::string(cluster_name) + "/Commands/"
                              + std::string(command);
      callback(new_topic.c_str(), message, message_length);
    }

    // Move to the next endpoint.
    endpoint_id_child_index += 1;
    endpoint_id_node
      = attribute_store_get_node_child_by_type(node_id_node,
                                               ATTRIBUTE_ENDPOINT_ID,
                                               endpoint_id_child_index);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
void zpc_dotdot_mqtt_group_dispatch(uint16_t group_id,
                                    const char *cluster_name,
                                    const char *command,
                                    const char *message,
                                    size_t message_length,
                                    mqtt_message_callback_t callback)
{
  // Go around in our network, find who's part of the group id
  // and dispatch the received command:
  attribute_store_node_t network_node = get_zpc_network_node();
  uint32_t node_id_child_index        = 0;
  attribute_store_node_t node_id_node
    = attribute_store_get_node_child_by_type(network_node,
                                             ATTRIBUTE_NODE_ID,
                                             node_id_child_index);
  // NodeID iteration
  while (node_id_node != ATTRIBUTE_STORE_INVALID_NODE) {
    // Get the UNID already, it's the same for all endpoints:
    unid_t unid;
    if (SL_STATUS_OK
        != attribute_store_network_helper_get_unid_from_node(node_id_node,
                                                             unid)) {
      // Move to the next NodeID
      node_id_child_index += 1;
      node_id_node
        = attribute_store_get_node_child_by_type(network_node,
                                                 ATTRIBUTE_NODE_ID,
                                                 node_id_child_index);
      sl_log_warning(LOG_TAG,
                     "Error retrieving UNID value for Attribute Store node %d.",
                     node_id_node);
      continue;
    }

    // Now go through the endpoints:
    zpc_dotdot_mqtt_dispatch_to_endpoints_under_node(group_id,
                                                     node_id_node,
                                                     unid,
                                                     cluster_name,
                                                     command,
                                                     message,
                                                     message_length,
                                                     callback);

    // Move to the next NodeID.
    node_id_child_index += 1;
    node_id_node = attribute_store_get_node_child_by_type(network_node,
                                                          ATTRIBUTE_NODE_ID,
                                                          node_id_child_index);
  }
}

sl_status_t zpc_dotdot_mqtt_group_dispatch_init()
{
  uic_mqtt_dotdot_set_group_dispatch_callback(&zpc_dotdot_mqtt_group_dispatch);
  return SL_STATUS_OK;
}