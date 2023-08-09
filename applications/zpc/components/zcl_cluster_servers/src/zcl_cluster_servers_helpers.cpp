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
//Include from this component
#include "zcl_cluster_servers.h"
#include "zcl_cluster_servers_helpers.hpp"

//Includes from other components
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"

// Interfaces
#include "zwave_controller_types.h"
#include "zap-types.h"

//Generic includes
#include <string>

///////////////////////////////////////////////////////////////////////////////
// Functions shared with the component
//////////////////////////////////////////////////////////////////////////////
sl_status_t
  zcl_cluster_servers_get_unid_endpoint(attribute_store_node_t endpoint_id_node,
                                        zwave_endpoint_id_t &endpoint_id,
                                        std::string &unid)
{
  sl_status_t status = SL_STATUS_OK;
  status |= attribute_store_get_reported(endpoint_id_node,
                                         &endpoint_id,
                                         sizeof(zwave_endpoint_id_t));

  unid_t received_unid = "";
  status |= attribute_store_network_helper_get_unid_from_node(endpoint_id_node,
                                                              received_unid);

  unid = std::string(received_unid);
  return status;
}

/**
 * @brief Reads the UCL network status of a node, starting from any attribute
 * under the NodeID in the attribute store.
 *
 * @param node     Attribute store node under the NodeID for which we want
 * to read the network status.
 */
NodeStateNetworkStatus get_network_status(attribute_store_node_t node)
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
