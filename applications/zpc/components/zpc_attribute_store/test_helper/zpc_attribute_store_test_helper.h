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
 * @file zpc_attribute_store_test_helper.h
 * @brief Test helper for using the zpc attribute store
 *
 * This component contains various helpers for when writing tests using the
 * attribute store
 *
 * @{
 */

#ifndef ZPC_ATTRIBUTE_STORE_TEST_HELPER_H
#define ZPC_ATTRIBUTE_STORE_TEST_HELPER_H

#include "attribute_store.h"
#include "zwave_controller_types.h"
#include "zwave_unid.h"

#ifdef __cplusplus
extern "C" {
#endif

/** The Z-Wave Home ID */
extern const zwave_home_id_t home_id;
/** The ZPC Node ID */
extern const zwave_node_id_t zpc_node_id;
/** The ZPC's endpoint ID */
extern const zwave_node_id_t zpc_endpoint_id;
/** The Node ID of the supporting node */
extern const zwave_node_id_t node_id;
/** The supporting node endpoint ID */
extern const zwave_endpoint_id_t endpoint_id;

// Attribute Store Nodes
/** The ZPC Endpoint Attribute ID */
extern attribute_store_node_t zpc_node_id_node;
/** The ZPC Node Attribute ID*/
extern attribute_store_node_t zpc_endpoint_id_node;
/** The Home ID Attribute ID */
extern attribute_store_node_t home_id_node;
/** The Supporting Node Attribute ID */
extern attribute_store_node_t node_id_node;
/** The Supporting Node Endpoint Attribute ID */
extern attribute_store_node_t endpoint_id_node;

// UNID of the ZPC based on its NodeID / HomeID
extern unid_t zpc_unid;
// UNID of the test Supporting NodeID. based on its NodeID / HomeID
extern unid_t supporting_node_unid;

/**
 * @brief Create test network
 *
 * Create a test network consiting of
 * - root
 *   - home_id_node
 *     - zpc_node_id_node (NODE)
 *       - zpc_endpoint_id_node (ENDPOINT)
 *     - node_id_node (NODE)
 *       - endpoint_id_node (ENDPOINT)
 */
void zpc_attribute_store_test_helper_create_network();

#ifdef __cplusplus
}
#endif

#endif  //ZPC_ATTRIBUTE_STORE_TEST_HELPER_H
/** @} end zpc_attribute_store_test_helper */
