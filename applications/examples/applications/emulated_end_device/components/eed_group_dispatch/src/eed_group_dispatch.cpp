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
#include "eed_group_dispatch.h"

// Unify Includes
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "dotdot_mqtt.h"
#include "sl_log.h"
#include "unify_dotdot_attribute_store.h"
#include "unify_dotdot_attribute_store_group_cluster.h"

// EED Includes
#include "eed_attribute_store_helper.h"
#include "eed_attribute_store_attribute_types.h"

// Generic includes
#include <string>

constexpr char LOG_TAG[] = "eed_mqtt_group_dispatch";

///////////////////////////////////////////////////////////////////////////////
// Helper functions
///////////////////////////////////////////////////////////////////////////////
static void
  mqtt_dispatch_to_endpoints_under_node(uint16_t group_id,
                                        attribute_store_node_t unid_node,
                                        dotdot_unid_t unid,
                                        const char *cluster_name,
                                        const char *command,
                                        const char *message,
                                        size_t message_length,
                                        mqtt_message_callback_t callback)
{
  // Now iterate through endpoints
  uint32_t endpoint_id_child_index = 0;
  attribute_store_node_t endpoint_id_node
    = attribute_store_get_node_child_by_type(unid_node,
                                             ATTRIBUTE_ENDPOINT_ID,
                                             endpoint_id_child_index);

  while (endpoint_id_node != ATTRIBUTE_STORE_INVALID_NODE) {
    dotdot_endpoint_id_t endpoint_id = 0;
    if (SL_STATUS_OK
        != attribute_store_get_reported(endpoint_id_node,
                                        &endpoint_id,
                                        sizeof(dotdot_endpoint_id_t))) {
      sl_log_debug(LOG_TAG,
                   "Error retrieving Endpoint ID for Attribute Store node %d.",
                   endpoint_id_node);
      // Move to the next Endpoint
      endpoint_id_child_index += 1;
      endpoint_id_node
        = attribute_store_get_node_child_by_type(unid_node,
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
      = attribute_store_get_node_child_by_type(unid_node,
                                               ATTRIBUTE_ENDPOINT_ID,
                                               endpoint_id_child_index);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
void eed_group_dispatch(uint16_t group_id,
                        const char *cluster_name,
                        const char *command,
                        const char *message,
                        size_t message_length,
                        mqtt_message_callback_t callback)
{
  // Go around in our network, find who's part of the group id
  // and dispatch the received command:
  attribute_store_node_t root_node = attribute_store_get_root();
  uint32_t unid_child_index        = 0;
  attribute_store_node_t unid_node
    = attribute_store_get_node_child_by_type(root_node,
                                             ATTRIBUTE_UNID,
                                             unid_child_index);
  // UNID iteration
  while (unid_node != ATTRIBUTE_STORE_INVALID_NODE) {
    // Get the UNID already, it's the same for all endpoints:
    char unid[MAXIMUM_UNID_SIZE];
    if (SL_STATUS_OK
        != eed_attribute_store_get_unid_from_node(unid_node, unid)) {
      // Move to the next UNID
      unid_child_index += 1;
      unid_node = attribute_store_get_node_child_by_type(root_node,
                                                         ATTRIBUTE_UNID,
                                                         unid_child_index);
      sl_log_warning(LOG_TAG,
                     "Error retrieving UNID value for Attribute Store node %d.",
                     unid_node);
      continue;
    }

    // Now go through the endpoints:
    mqtt_dispatch_to_endpoints_under_node(group_id,
                                          unid_node,
                                          unid,
                                          cluster_name,
                                          command,
                                          message,
                                          message_length,
                                          callback);

    // Move to the next UNID.
    unid_child_index += 1;
    unid_node = attribute_store_get_node_child_by_type(root_node,
                                                       ATTRIBUTE_UNID,
                                                       unid_child_index);
  }
}

sl_status_t eed_group_dispatch_init()
{
  uic_mqtt_dotdot_set_group_dispatch_callback(&eed_group_dispatch);
  return SL_STATUS_OK;
}