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
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"
#include "unify_dotdot_defined_attribute_types.h"
#include "unify_dotdot_attribute_store_node_state.h"
#include "zpc_attribute_store.h"

// Generic includes
#include <stdlib.h>
#include <stdbool.h>

// Includes from other components
#include "attribute_store_helper.h"
#include "sl_log.h"
#include "sl_status.h"
#include "zwave_unid.h"
#include "zwave_api.h"

#define LOG_TAG "zpc_attribute_store"

///////////////////////////////////////////////////////////////////////////////
// HomeID/NodeID/Endpoint creation functions
///////////////////////////////////////////////////////////////////////////////
attribute_store_node_t
  attribute_store_network_helper_create_home_id_node(const unid_t node_unid)
{
  zwave_home_id_t home_id;
  if (SL_STATUS_OK != zwave_unid_to_home_id(node_unid, &home_id)) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  attribute_store_node_t root_node = attribute_store_get_root();
  if (root_node == ATTRIBUTE_STORE_INVALID_NODE) {
    // Attribute store not initialized
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  attribute_store_node_t home_id_node
    = attribute_store_get_node_child_by_value(root_node,
                                              ATTRIBUTE_HOME_ID,
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&home_id,
                                              sizeof(home_id),
                                              0);

  if (home_id_node != ATTRIBUTE_STORE_INVALID_NODE) {
    return home_id_node;
  }

  // If we get here, there is no HomeID
  // under the root that matches our need. We create one.
  home_id_node = attribute_store_add_node(ATTRIBUTE_HOME_ID, root_node);

  attribute_store_set_node_attribute_value(home_id_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&home_id,
                                           sizeof(home_id));
  return home_id_node;
}

attribute_store_node_t
  attribute_store_network_helper_create_node_id_node(const unid_t node_unid)
{
  zwave_node_id_t node_id;
  if (SL_STATUS_OK != zwave_unid_to_node_id(node_unid, &node_id)) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  attribute_store_node_t home_id_node;
  home_id_node = attribute_store_network_helper_create_home_id_node(node_unid);
  if (ATTRIBUTE_STORE_INVALID_NODE == home_id_node) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  attribute_store_node_t node_id_node
    = attribute_store_get_node_child_by_value(home_id_node,
                                              ATTRIBUTE_NODE_ID,
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&node_id,
                                              sizeof(node_id),
                                              0);

  if (node_id_node != ATTRIBUTE_STORE_INVALID_NODE) {
    return node_id_node;
  }

  // If we get here, there is no NodeID
  // under the HomeID that matches our need. We create one.
  node_id_node = attribute_store_add_node(ATTRIBUTE_NODE_ID, home_id_node);
  attribute_store_set_node_attribute_value(node_id_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&node_id,
                                           sizeof(node_id));
  return node_id_node;
}

attribute_store_node_t attribute_store_network_helper_create_endpoint_node(
  const unid_t node_unid, zwave_endpoint_id_t endpoint_id)
{
  attribute_store_node_t node_id_identifier;
  node_id_identifier
    = attribute_store_network_helper_create_node_id_node(node_unid);
  if (node_id_identifier == ATTRIBUTE_STORE_INVALID_NODE) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  // Look for an endpoint under the node_id node:
  attribute_store_node_t endpoint_node
    = attribute_store_get_node_child_by_value(node_id_identifier,
                                              ATTRIBUTE_ENDPOINT_ID,
                                              REPORTED_ATTRIBUTE,
                                              &endpoint_id,
                                              sizeof(endpoint_id),
                                              0);

  if (endpoint_node != ATTRIBUTE_STORE_INVALID_NODE) {
    return endpoint_node;
  }

  // If we get here, there is no endpoint
  // under the NodeID that matches our need. We create one.
  endpoint_node
    = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, node_id_identifier);
  attribute_store_set_node_attribute_value(endpoint_node,
                                           REPORTED_ATTRIBUTE,
                                           &endpoint_id,
                                           sizeof(endpoint_id));
  return endpoint_node;
}

///////////////////////////////////////////////////////////////////////////////
// HomeID/NodeID/Endpoint getter functions
///////////////////////////////////////////////////////////////////////////////
attribute_store_node_t
  attribute_store_network_helper_get_home_id_node(const unid_t node_unid)
{
  zwave_home_id_t home_id;
  if (SL_STATUS_OK != zwave_unid_to_home_id(node_unid, &home_id)) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  attribute_store_node_t root_node = attribute_store_get_root();
  if (root_node == ATTRIBUTE_STORE_INVALID_NODE) {
    // Attribute store not initialized
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  return attribute_store_get_node_child_by_value(root_node,
                                                 ATTRIBUTE_HOME_ID,
                                                 REPORTED_ATTRIBUTE,
                                                 (uint8_t *)&home_id,
                                                 sizeof(home_id),
                                                 0);
}

attribute_store_node_t
  attribute_store_network_helper_get_node_id_node(const unid_t node_unid)
{
  zwave_node_id_t node_id;
  if (SL_STATUS_OK != zwave_unid_to_node_id(node_unid, &node_id)) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  attribute_store_node_t home_id_node;
  home_id_node = attribute_store_network_helper_get_home_id_node(node_unid);
  if (ATTRIBUTE_STORE_INVALID_NODE == home_id_node) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  return attribute_store_get_node_child_by_value(home_id_node,
                                                 ATTRIBUTE_NODE_ID,
                                                 REPORTED_ATTRIBUTE,
                                                 (uint8_t *)&node_id,
                                                 sizeof(node_id),
                                                 0);
}

attribute_store_node_t attribute_store_network_helper_get_zwave_node_id_node(
  zwave_node_id_t zwave_node_id)
{
  unid_t unid;
  zwave_unid_from_node_id(zwave_node_id, unid);

  return attribute_store_network_helper_get_node_id_node(unid);
}

attribute_store_node_t attribute_store_network_helper_get_endpoint_node(
  const unid_t node_unid, zwave_endpoint_id_t endpoint_id)
{
  attribute_store_node_t node_id_identifier
    = attribute_store_network_helper_get_node_id_node(node_unid);
  if (node_id_identifier == ATTRIBUTE_STORE_INVALID_NODE) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  // Look for an endpoint under the node_id node:
  return attribute_store_get_node_child_by_value(node_id_identifier,
                                                 ATTRIBUTE_ENDPOINT_ID,
                                                 REPORTED_ATTRIBUTE,
                                                 &endpoint_id,
                                                 sizeof(endpoint_id),
                                                 0);
}

attribute_store_node_t
  attribute_store_get_endpoint_0_node(attribute_store_node_t node_id_node)
{
  const zwave_endpoint_id_t endpoint_id = 0;
  return attribute_store_get_node_child_by_value(node_id_node,
                                                 ATTRIBUTE_ENDPOINT_ID,
                                                 REPORTED_ATTRIBUTE,
                                                 &endpoint_id,
                                                 sizeof(endpoint_id),
                                                 0);
}

sl_status_t attribute_store_network_helper_get_unid_endpoint_from_node(
  attribute_store_node_t node, unid_t unid, zwave_endpoint_id_t *endpoint_id)
{
  if (attribute_store_get_node_type(node) != ATTRIBUTE_ENDPOINT_ID) {
    node
      = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);
  }
  if (node == ATTRIBUTE_STORE_INVALID_NODE) {
    return SL_STATUS_NOT_FOUND;
  }

  zwave_endpoint_id_t endpoint;
  if (SL_STATUS_OK
      != attribute_store_get_reported(node, &endpoint, sizeof(endpoint))) {
    // Abort if no endpoint data was retrieved.
    return SL_STATUS_FAIL;
  }

  // Continue climbing up until we find a NodeID type of node.
  // We allow spaces between the endpoint and NodeID. Should we not?
  node = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_NODE_ID);
  if (node == ATTRIBUTE_STORE_INVALID_NODE) {
    return SL_STATUS_NOT_FOUND;
  }

  zwave_node_id_t node_id;
  if (SL_STATUS_OK
      != attribute_store_get_reported(node,
                                      &node_id,
                                      sizeof(zwave_node_id_t))) {
    // Abort if no NodeID data was retrieved.
    return SL_STATUS_FAIL;
  }

  // Continue climbing up until we find the HomeID type of node, check that it
  // it our HomeID. Do not reject the request if it is not our HomeID.
  node = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_HOME_ID);
  if (node != get_zpc_network_node()) {
    sl_log_debug(
      LOG_TAG,
      "Note: Retrieving UNID/endpoint values from a foreign HomeID.");
  }

  // At that point, we are happy and can copy the variables
  // back to the caller's pointers
  zwave_unid_from_node_id(node_id, unid);
  *endpoint_id = endpoint;
  return SL_STATUS_OK;
}

sl_status_t
  attribute_store_network_helper_get_unid_from_node(attribute_store_node_t node,
                                                    unid_t unid)
{
  attribute_store_type_t current_node_type
    = attribute_store_get_node_type(node);
  // Start climbing up until we find an NodeID type of node:
  while (node != ATTRIBUTE_STORE_INVALID_NODE
         && current_node_type != ATTRIBUTE_NODE_ID) {
    node              = attribute_store_get_node_parent(node);
    current_node_type = attribute_store_get_node_type(node);
  }

  if (node == ATTRIBUTE_STORE_INVALID_NODE) {
    return SL_STATUS_NOT_FOUND;
  }

  zwave_node_id_t node_id = 0;
  if (SL_STATUS_OK
      != attribute_store_get_reported(node,
                                      &node_id,
                                      sizeof(zwave_node_id_t))) {
    // Abort if no NodeID data was retrieved.
    return SL_STATUS_FAIL;
  }

  // Continue climbing up until we find the HomeID type of node, check that it
  // it our HomeID. Do not reject the request if it is not our HomeID.
  node = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_HOME_ID);
  if (node != get_zpc_network_node()) {
    sl_log_debug(LOG_TAG, "Note: Retrieving UNID value from a foreign HomeID.");
  }

  // At that point, we are happy and can copy the variables
  // back to the caller's pointers
  zwave_unid_from_node_id(node_id, unid);
  return SL_STATUS_OK;
}

sl_status_t attribute_store_network_helper_get_node_id_from_node(
  attribute_store_node_t node, zwave_node_id_t *zwave_node_id)
{
  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_NODE_ID);

  if (node_id_node == ATTRIBUTE_STORE_INVALID_NODE) {
    return SL_STATUS_NOT_FOUND;
  }

  if (SL_STATUS_OK
      != attribute_store_get_reported(node_id_node,
                                      zwave_node_id,
                                      sizeof(zwave_node_id_t))) {
    // Abort if no NodeID data was retrieved.
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

sl_status_t attribute_store_network_helper_get_endpoint_id_from_node(
  attribute_store_node_t node, zwave_endpoint_id_t *zwave_endpoint_id)
{
  attribute_store_node_t endpoint_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_ENDPOINT_ID);

  if (endpoint_id_node == ATTRIBUTE_STORE_INVALID_NODE) {
    return SL_STATUS_NOT_FOUND;
  }

  if (SL_STATUS_OK
      != attribute_store_get_reported(endpoint_id_node,
                                      zwave_endpoint_id,
                                      sizeof(zwave_endpoint_id_t))) {
    // Abort if no Endpoint ID data was retrieved.
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

sl_status_t attribute_store_network_helper_get_zwave_ids_from_node(
  attribute_store_node_t node,
  zwave_node_id_t *zwave_node_id,
  zwave_endpoint_id_t *zwave_endpoint_id)
{
  attribute_store_type_t node_type   = attribute_store_get_node_type(node);
  attribute_store_node_t parent_node = attribute_store_get_node_parent(node);
  attribute_store_type_t parent_node_type
    = attribute_store_get_node_type(parent_node);

  // Start climbing up until we find a consecutive EndpoinID -> NodeID
  // types of node:
  while (parent_node != ATTRIBUTE_STORE_INVALID_NODE
         && node_type != ATTRIBUTE_ENDPOINT_ID
         && parent_node_type != ATTRIBUTE_NODE_ID) {
    node             = attribute_store_get_node_parent(node);
    parent_node      = attribute_store_get_node_parent(node);
    node_type        = attribute_store_get_node_type(node);
    parent_node_type = attribute_store_get_node_type(parent_node);
  }

  if (parent_node == ATTRIBUTE_STORE_INVALID_NODE) {
    return SL_STATUS_NOT_FOUND;
  }

  // If the parent node is not invalid, when we found the NodeID/endpoint combo
  // Copy the data and return
  if (SL_STATUS_OK
      != attribute_store_get_reported(parent_node,
                                      zwave_node_id,
                                      sizeof(zwave_node_id_t))) {
    // Abort if no NodeID data was retrieved.
    return SL_STATUS_FAIL;
  }
  if (SL_STATUS_OK
      != attribute_store_get_reported(node,
                                      zwave_endpoint_id,
                                      sizeof(zwave_endpoint_id_t))) {
    // Abort if no Endpoint ID data was retrieved.
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

attribute_store_node_t
  zwave_command_class_get_endpoint_id_node(zwave_node_id_t node_id,
                                           zwave_endpoint_id_t endpoint_id)
{
  return attribute_store_get_node_child_by_value(
    attribute_store_get_node_child_by_value(get_zpc_network_node(),
                                            ATTRIBUTE_NODE_ID,
                                            REPORTED_ATTRIBUTE,
                                            (uint8_t *)&node_id,
                                            sizeof(node_id),
                                            0),
    ATTRIBUTE_ENDPOINT_ID,
    REPORTED_ATTRIBUTE,
    &endpoint_id,
    sizeof(endpoint_id),
    0);
}

NodeStateNetworkStatus
  attribute_store_network_helper_get_network_status(attribute_store_node_t node)
{
  // Default to UNAVAILABLE if the value is undefined in the attribute store
  NodeStateNetworkStatus network_status = ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE;

  attribute_store_node_t node_id_node
    = attribute_store_get_first_parent_with_type(node, ATTRIBUTE_NODE_ID);

  attribute_store_node_t network_status_node
    = attribute_store_get_first_child_by_type(node_id_node,
                                              DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS);
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  return network_status;
}
