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

// Unify components
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_resolver_rule.h"

// ZPC components
#include "zpc_attribute_store.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"

#include "sl_log.h"
#define LOG_TAG "ucl_mqtt_node_interview"

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
void undefine_attribute_with_get_rule(attribute_store_node_t node)
{
  if (attribute_resolver_has_get_rule(attribute_store_get_node_type(node))) {
   // If node is already under resolution, we should just retry
   if(SL_STATUS_NOT_FOUND == attribute_resolver_restart_get_resolution(node)) {
        attribute_store_undefine_reported(node);
   }
  }
}

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

  uint32_t child_index = 0;
  attribute_store_node_t current_endpoint
    = attribute_store_get_node_child_by_type(node,
                                             ATTRIBUTE_ENDPOINT_ID,
                                             child_index);
  while (current_endpoint != ATTRIBUTE_STORE_INVALID_NODE) {
    // Undefine all attributes that have a Get rule attached under the endpoint.
    attribute_store_walk_tree(current_endpoint,
                              &undefine_attribute_with_get_rule);
    child_index += 1;
    current_endpoint
      = attribute_store_get_node_child_by_type(node,
                                               ATTRIBUTE_ENDPOINT_ID,
                                               child_index);
  }

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

  // Get the endpoint by value:
  attribute_store_node_t endpoint
    = attribute_store_get_node_child_by_value(node,
                                              ATTRIBUTE_ENDPOINT_ID,
                                              REPORTED_ATTRIBUTE,
                                              &endpoint_id,
                                              sizeof(zwave_endpoint_id_t),
                                              0);

  // Undefine everything under it
  attribute_store_walk_tree(endpoint, &undefine_attribute_with_get_rule);

  return SL_STATUS_OK;
}
