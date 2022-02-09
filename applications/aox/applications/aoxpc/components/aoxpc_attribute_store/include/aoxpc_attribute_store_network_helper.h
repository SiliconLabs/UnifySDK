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

#ifndef AOXPC_ATTRIBUTE_STORE_NETWORK_HELPER_H
#define AOXPC_ATTRIBUTE_STORE_NETWORK_HELPER_H

#include "attribute_store.h"

// Unify definitions
#include "sl_status.h"
#include "uic_typedefs.h"

// AoX Gecko SDK
#include "aoa_types.h"

/**
 * @defgroup aoxpc_attribute_store_network_helpers AoXPC Attribute Store Helpers
 * @ingroup aoxpc_attribute_store
 * @brief Helper functions to read find out UNID / Endpoints
 * nodes in the @ref attribute_store
 *
 * These helper functions create or get the attribute store nodes for a given UNID.
 *
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Take a UNID and return its attribute store node representing the HomeID.
 *
 * Note: If the HomeID of that UNID does not exist in the attribute store,
 * this function will create it.
 *
 * @param node_unid The unid_t value to parse and search in the tree.
 *
 * @returns attribute_store_node_t representing the HomeID for the UNID
 *          in the attribute store
 */
attribute_store_node_t aoxpc_attribute_store_network_helper_create_unid_node(
  const aoa_id_t node_unid);

/**
 * @brief Take a UNID/endpoint combination and returns its attribute store node.
 *
 * Note: If the UNID/endpoint does not exist in the attribute store, this function
 * will create it.

 * @param node_unid   The @ref unid_t value to parse and search in the tree.
 * @param endpoint_id The @ref zwave_endpoint_id_t to search under the UNID
 *                    in the @ref attribute_store tree.
 *
 * @returns attribute_store_node_t representing the UNID/endpoint
 *          in the attribute store
 */
attribute_store_node_t
  aoxpc_attribute_store_network_helper_create_endpoint_node(
    const aoa_id_t node_unid, dotdot_endpoint_id_t endpoint_id);

/**
 * @brief Take a UNID and return its attribute store node representing its UNID.
 *
 * @param node_unid The aoa_id_t value to parse and search in the tree.
 *
 * @returns attribute_store_node_t representing the UNID
 *          in the attribute store.
 * @returns ATTRIBUTE_STORE_INVALID_NODE if no matching UNID node exists
 */
attribute_store_node_t
  aoxpc_attribute_store_network_helper_get_unid_node(const aoa_id_t node_unid);

/**
 * @brief Take a UNID/endpoint combination and returns its attribute store node.
 *
 * @param node_unid   The @ref unid_t value to parse and search in the tree.
 * @param endpoint_id The @ref zwave_endpoint_id_t to search under the UNID
 *                    in the @ref attribute_store tree.
 *
 * @returns attribute_store_node_t representing the UNID/endpoint
 *          in the attribute store
 * @returns ATTRIBUTE_STORE_INVALID_NODE if no matching UNID/endpoint node exists
 */
attribute_store_node_t aoxpc_attribute_store_network_helper_get_endpoint_node(
  const aoa_id_t node_unid, dotdot_endpoint_id_t endpoint_id);

#ifdef __cplusplus
}
#endif

/** @} end aoxpc_attribute_store_network_helpers */

#endif  //AOXPC_ATTRIBUTE_STORE_NETWORK_HELPER_H
