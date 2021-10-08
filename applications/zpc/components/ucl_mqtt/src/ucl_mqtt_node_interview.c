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
// Includes from this component
#include "ucl_mqtt_node_interview.h"

// Includes from other components
#include "attribute_store.h"
#include "zpc_attribute_store.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"

#include "sl_log.h"
#define LOG_TAG "ucl_mqtt_node_interview"

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t ucl_mqtt_initiate_node_interview(const unid_t node_unid)
{
  if (is_zpc_unid(node_unid)) {
    sl_log_debug(LOG_TAG,
                 "Node interview requested for the ZPC's UNID. Ignoring.");
    return SL_STATUS_OK;
  }

  attribute_store_node_t node
    = attribute_store_network_helper_get_node_id_node(node_unid);

  if (node == ATTRIBUTE_STORE_INVALID_NODE) {
    return SL_STATUS_NOT_FOUND;
  }

  // Start off by deleting all the endpoints under the node:
  attribute_store_node_t current_endpoint
    = attribute_store_get_node_child_by_type(node, ATTRIBUTE_ENDPOINT_ID, 0);
  while (current_endpoint != ATTRIBUTE_STORE_INVALID_NODE) {
    // No harm in calling delete on an invalid node.
    if (SL_STATUS_OK != attribute_store_delete_node(current_endpoint)) {
      return SL_STATUS_FAIL;
    }
    // Keep the child index to 0 here, since we delete them
    current_endpoint
      = attribute_store_get_node_child_by_type(node, ATTRIBUTE_ENDPOINT_ID, 0);
  }

  // Now that we wiped the endpoints, create endpoint 0 again with an empty NIF.
  current_endpoint
    = attribute_store_network_helper_create_endpoint_node(node_unid, 0);
  attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, current_endpoint);

  return SL_STATUS_OK;
}

sl_status_t
  ucl_mqtt_initiate_endpoint_interview(const unid_t node_unid,
                                       zwave_endpoint_id_t endpoint_id)
{
  if (is_zpc_unid(node_unid)) {
    sl_log_debug(LOG_TAG,
                 "Endpoint interview requested for the ZPC's UNID. Ignoring.");
    return SL_STATUS_OK;
  }

  attribute_store_node_t node
    = attribute_store_network_helper_get_node_id_node(node_unid);

  if (node == ATTRIBUTE_STORE_INVALID_NODE) {
    return SL_STATUS_NOT_FOUND;
  }

  // Start off by deleting all the endpoints under the node:
  attribute_store_node_t endpoint
    = attribute_store_get_node_child_by_value(node,
                                              ATTRIBUTE_ENDPOINT_ID,
                                              REPORTED_ATTRIBUTE,
                                              &endpoint_id,
                                              sizeof(zwave_endpoint_id_t),
                                              0);
  if (endpoint == ATTRIBUTE_STORE_INVALID_NODE) {
    return SL_STATUS_NOT_FOUND;
  }

  if (SL_STATUS_OK != attribute_store_delete_node(endpoint)) {
    return SL_STATUS_FAIL;
  }

  // Now that we wiped the endpoint, create it again with an empty NIF.
  endpoint = attribute_store_network_helper_create_endpoint_node(node_unid,
                                                                 endpoint_id);
  attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint);

  return SL_STATUS_OK;
}
