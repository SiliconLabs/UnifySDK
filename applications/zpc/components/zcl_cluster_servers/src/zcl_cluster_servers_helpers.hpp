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

/**
 * @defgroup zcl_cluster_servers_helpers ZCL Clusters helpers
 * @ingroup zcl_cluster_servers
 * @brief Helper functions that all ZCL Cluster servers can use
 *
 * @{
 */

#ifndef ZCL_CLUSTER_SERVERS_HELPERS_HPP
#define ZCL_CLUSTER_SERVERS_HELPERS_HPP

#include "sl_status.h"
#include "attribute_store.h"
#include "ucl_definitions.h"
#include "zwave_controller_types.h"

#include <string>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Helper function that finds out the UNID/Endpoint ID values an endpoint
 * attribute store node
 *
 * @param [in] endpoint_id_node   Attribute Store node of the endpoint
 * @param [out] endpoint_id       Reference where the Endpoint ID value will be written
 * @param [out] unid              Reference where the UNID String value will be written
 *
 * @returns SL_STATUS_OK if the unid/endpoint_id were written, any other code
 *          otherwise
 */
sl_status_t
  zcl_cluster_servers_get_unid_endpoint(attribute_store_node_t endpoint_id_node,
                                        zwave_endpoint_id_t &endpoint_id,
                                        std::string &unid);

/**
 * @brief Reads the UCL network status of a node, starting from any attribute
 * under the NodeID in the attribute store.
 *
 * @param node     Attribute store node under the NodeID for which we want
 * to read the network status.
 */
node_state_topic_state_t get_network_status(attribute_store_node_t node);

#ifdef __cplusplus
}
#endif

#endif  //ZCL_CLUSTER_SERVERS_HELPERS_HPP
/** @} end zcl_cluster_servers_helpers */
