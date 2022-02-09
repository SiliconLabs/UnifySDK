/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "aoxpc_attribute_store_network_helper.h"
#include "aoxpc_attribute_store_defined_attribute_types.h"
#include "aoxpc_attribute_store.h"

// Generic includes
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Includes from other components
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "sl_log.h"
#include "sl_status.h"

#define LOG_TAG "aoxpc_attribute_store"

///////////////////////////////////////////////////////////////////////////////
// UNID/Endpoint creation functions
///////////////////////////////////////////////////////////////////////////////
attribute_store_node_t aoxpc_attribute_store_network_helper_create_unid_node(
  const aoa_id_t node_unid)
{
  attribute_store_node_t root_node = attribute_store_get_root();
  if (root_node == ATTRIBUTE_STORE_INVALID_NODE) {
    // Attribute store not initialized
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  uint32_t unid_node_index = 0;
  bool node_found          = false;
  attribute_store_node_t unid_node
    = attribute_store_get_node_child_by_type(root_node,
                                             ATTRIBUTE_BLE_UNID,
                                             unid_node_index);

  while ((ATTRIBUTE_STORE_INVALID_NODE != unid_node) && (node_found == false)) {
    aoa_id_t node_unid_value = {};
    if (SL_STATUS_OK
        == attribute_store_get_reported_string(unid_node,
                                               (char *)&node_unid_value,
                                               sizeof(aoa_id_t))) {
      // Compare the aoa_id_t values
      if (strcmp(node_unid_value, node_unid) == 0) {
        node_found = true;
      } else {
        unid_node_index += 1;
      }
    } else {
      unid_node_index += 1;
    }
    // Try the next child
    unid_node = attribute_store_get_node_child_by_type(root_node,
                                                       ATTRIBUTE_BLE_UNID,
                                                       unid_node_index);
  }

  // If the node exists, return it
  if (node_found == true) {
    return unid_node;
  }

  // If it does not exist, create it.
  sl_log_debug(LOG_TAG,
               "Creating BLE UNID %s in the attribute store",
               node_unid);
  unid_node = attribute_store_add_node(ATTRIBUTE_BLE_UNID, root_node);
  attribute_store_set_reported_string(unid_node, node_unid);
  return unid_node;
}

attribute_store_node_t
  aoxpc_attribute_store_network_helper_create_endpoint_node(
    const aoa_id_t node_unid, dotdot_endpoint_id_t endpoint_id)
{
  // Get (and create if does not exist) the UNID node
  attribute_store_node_t unid_node
    = aoxpc_attribute_store_network_helper_create_unid_node(node_unid);

  if (unid_node == ATTRIBUTE_STORE_INVALID_NODE) {
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  // Look for an endpoint under the node_id node:
  attribute_store_node_t endpoint_node
    = attribute_store_get_node_child_by_value(unid_node,
                                              ATTRIBUTE_ENDPOINT_ID,
                                              REPORTED_ATTRIBUTE,
                                              &endpoint_id,
                                              sizeof(endpoint_id),
                                              0);

  if (endpoint_node != ATTRIBUTE_STORE_INVALID_NODE) {
    return endpoint_node;
  }

  // If we get here, there is no endpoint
  // under the UNID that matches our need. We create one.
  sl_log_debug(LOG_TAG,
               "Creating BLE UNID %s Endpoint %i in the attribute store",
               node_unid,
               endpoint_id);
  endpoint_node = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, unid_node);
  attribute_store_set_reported(endpoint_node,
                               &endpoint_id,
                               sizeof(endpoint_id));
  return endpoint_node;
}

///////////////////////////////////////////////////////////////////////////////
// UNID/Endpoint getter functions
///////////////////////////////////////////////////////////////////////////////
attribute_store_node_t
  aoxpc_attribute_store_network_helper_get_unid_node(const aoa_id_t node_unid)
{
  attribute_store_node_t root_node = attribute_store_get_root();
  if (root_node == ATTRIBUTE_STORE_INVALID_NODE) {
    // Attribute store not initialized
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  uint32_t unid_node_index = 0;
  bool node_found          = false;
  attribute_store_node_t unid_node
    = attribute_store_get_node_child_by_type(root_node,
                                             ATTRIBUTE_BLE_UNID,
                                             unid_node_index);

  while ((ATTRIBUTE_STORE_INVALID_NODE != unid_node) && (node_found == false)) {
    aoa_id_t node_unid_value = {};
    if (SL_STATUS_OK
        == attribute_store_get_reported_string(unid_node,
                                               (char *)&node_unid_value,
                                               sizeof(aoa_id_t))) {
      // Compare the aoa_id_t values
      if (strcmp(node_unid_value, node_unid) == 0) {
        node_found = true;
      } else {
        unid_node_index += 1;
      }
    } else {
      unid_node_index += 1;
    }
    // Try the next child
    unid_node = attribute_store_get_node_child_by_type(root_node,
                                                       ATTRIBUTE_BLE_UNID,
                                                       unid_node_index);
  }

  // return what we found.
  return unid_node;
}

attribute_store_node_t aoxpc_attribute_store_network_helper_get_endpoint_node(
  const aoa_id_t node_unid, dotdot_endpoint_id_t endpoint_id)
{
  attribute_store_node_t unid_node
    = aoxpc_attribute_store_network_helper_get_unid_node(node_unid);

  // Look for an endpoint under the unid_node
  return attribute_store_get_node_child_by_value(unid_node,
                                                 ATTRIBUTE_ENDPOINT_ID,
                                                 REPORTED_ATTRIBUTE,
                                                 &endpoint_id,
                                                 sizeof(endpoint_id),
                                                 0);
}
