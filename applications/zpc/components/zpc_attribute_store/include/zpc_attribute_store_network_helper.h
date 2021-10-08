/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef ATTRIBUTE_STORE_NETWORK_HELPER_H
#define ATTRIBUTE_STORE_NETWORK_HELPER_H

// Includes from this component
#include "attribute_store.h"

// Includes from other components
#include "sl_status.h"
#include "zwave_unid.h"
#include "zwave_controller_types.h"  // for zwave_endpoint_id_t

/**
 * @defgroup attribute_store_network_helpers Attribute Store Z-Wave Network Helpers
 * @ingroup attribute_store
 * @brief Helper functions to read find out HomeID / NodeID / Endpoints nodes in the @ref attribute_store
 *
 * These helper functions return the attribute store nodes for a given UNID.
 * Note that if requested node does not exist in the @ref attribute_store, they will
 * be created and their newly created node will be returned.
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
attribute_store_node_t
  attribute_store_network_helper_create_home_id_node(const unid_t node_unid);

/**
 * @brief Take a UNID and return its attribute store node representing the NodeID.
 *
 * Note: If the NodeID of that UNID does not exist in the attribute store,
 * this function will create it.
 *
 * @param node_unid The unid_t value to parse and search in the tree.
 *
 * @returns attribute_store_node_t representing the NodeID for the UNID
 *          in the attribute store
 */
attribute_store_node_t
  attribute_store_network_helper_create_node_id_node(const unid_t node_unid);

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
attribute_store_node_t attribute_store_network_helper_create_endpoint_node(
  const unid_t node_unid, zwave_endpoint_id_t endpoint_id);

/**
 * @brief Take a UNID and return its attribute store node representing the HomeID.
 *
 *
 * @param node_unid The unid_t value to parse and search in the tree.
 *
 * @returns attribute_store_node_t representing the HomeID for the UNID
 *          in the attribute store.
 * @returns ATTRIBUTE_STORE_INVALID_NODE if no matching UNID node exists
 */
attribute_store_node_t
  attribute_store_network_helper_get_home_id_node(const unid_t node_unid);

/**
 * @brief Take a UNID and return its attribute store node representing the NodeID.
 *
 * @param node_unid The unid_t value to parse and search in the tree.
 *
 * @returns attribute_store_node_t representing the NodeID for the UNID
 *          in the attribute store.
 * @returns ATTRIBUTE_STORE_INVALID_NODE if no matching UNID node exists
 */
attribute_store_node_t
  attribute_store_network_helper_get_node_id_node(const unid_t node_unid);

/**
 * @brief Take a Z-Wave NodeID and return its Attribute Store node representing the NodeID.
 *
 * This function will convert the NodeID to UNID, ensuring that the
 * returned Node representing the NodeID belongs to the same HomeiD
 * as the ZPC
 *
 * @param zwave_node_id The Z-Wave NodeID value to parse and search in the tree.
 *
 * @returns attribute_store_node_t representing the NodeID for the Z-Wave
 *          NodeID in the attribute store.
 * @returns ATTRIBUTE_STORE_INVALID_NODE if no matching Z-Wave NodeID node exists
 */
attribute_store_node_t
  attribute_store_network_helper_get_zwave_node_id_node(zwave_node_id_t zwave_node_id);

/**
 * @brief Take a UNID/endpoint combination and returns its attribute store node.
 *
 * @param node_unid   The @ref unid_t value to parse and search in the tree.
 * @param endpoint_id The @ref zwave_endpoint_id_t to search under the UNID
 *                    in the @ref attribute_store tree.
 *
 * @returns attribute_store_node_t representing the UNID/endpoint
 *          in the attribute store
 * @returns ATTRIBUTE_STORE_INVALID_NODE if no matching UNID node exists
 */
attribute_store_node_t attribute_store_network_helper_get_endpoint_node(
  const unid_t node_unid, zwave_endpoint_id_t endpoint_id);

/**
 * @brief Traverse the tree up from a node and find under which UNID/endpoints it is.
 *
 * @param node        The Attribute store node for which the parent UNID/endpoint
 *                    will be search for
 * @param unid        A pointer where to write the found UNID
 * @param endpoint_id A pointer where to write the found endpoint id
 *
 * @returns SL_STATUS_OK        If parents nodes (HomeID/NodeID/endpoints) were
 *                              found and the values were copied in the UNID
 *                              and endpoint_id variables
 *                              has been copied to the read_value pointer
 * @returns SL_STATUS_NOT_FOUND If the function went all the way up to the tree root
 *                              and did not find UNID/endpoints
 * @returns SL_STATUS_FAIL      If some error occurred reading values in the tree.
 */
sl_status_t attribute_store_network_helper_get_unid_endpoint_from_node(
  attribute_store_node_t node, unid_t unid, zwave_endpoint_id_t *endpoint_id);

/**
 * @brief Traverse up the tree from a node and finds under which UNID it is.
 *
 * @param node        The Attribute store node for which the parent UNID
 *                    will be search for
 * @param unid        A pointer where to write the found UNID
 *
 * @returns SL_STATUS_OK        If parents nodes (HomeID/NodeID) were
 *                              found and the value was copied in the UNID
 *                              variable
 *                              has been copied to the read_value pointer
 * @returns SL_STATUS_NOT_FOUND If the function went all the way up to the tree root
 *                              and did not find UNID
 * @returns SL_STATUS_FAIL      If some error occurred reading values in the tree.
 */
sl_status_t
  attribute_store_network_helper_get_unid_from_node(attribute_store_node_t node,
                                                    unid_t unid);

/**
 * @brief Traverse up the tree from a node and finds under which Z-Wave NodeID it is.
 *
 * @param node          The Attribute store node for which the parent NodeID
 *                      will be search for
 * @param zwave_node_id A pointer where to write the found Z-Wave NodeID
 *
 * @returns SL_STATUS_OK        If a NodeID type of parents node was
 *                              found and its reported value was copied in the
 *                              zwave_node_id variable
 * @returns SL_STATUS_NOT_FOUND If the function went all the way up to the tree root
 *                              and did not find NodeID type of node
 * @returns SL_STATUS_FAIL      If some error occurred reading values in the tree.
 */
sl_status_t attribute_store_network_helper_get_node_id_from_node(
  attribute_store_node_t node, zwave_node_id_t *zwave_node_id);

/**
 * @brief Traverse up the tree from a node and finds under which Z-Wave endpoint id it is located.
 *
 * @param node              The Attribute store node for which the parent endpoint
 *                          will be searched for
 * @param zwave_endpoint_id A pointer where to write the found Z-Wave endpoint id
 *
 * @returns SL_STATUS_OK        If an endpoint type of parents node was
 *                              found and its value was copied in the
 *                              zwave_endpoint_id variable
 * @returns SL_STATUS_NOT_FOUND If the function went all the way up to the tree root
 *                              and did not find any endpoint type of attribute
 * @returns SL_STATUS_FAIL      If some error occurred reading values in the tree.
 */
sl_status_t attribute_store_network_helper_get_endpoint_id_from_node(
  attribute_store_node_t node, zwave_endpoint_id_t *zwave_endpoint_id);

/**
 * @brief Traverse up the tree from a node and finds under which Z-Wave NodeID/endpoint id it is located.
 *
 * @param node              The Attribute store node for which the parent Z-Wave NodeID/endpoint
 *                          will be searched for
 * @param zwave_node_id     A pointer where to write the found Z-Wave NodeID (reported value)
 * @param zwave_endpoint_id A pointer where to write the found Z-Wave endpoint id (reported value)
 *
 * @returns SL_STATUS_OK        If both NodeID/endpoint parents nodes were
 *                              found and their value were copied in the
 *                              zwave_node_id and zwave_endpoint_id variables
 * @returns SL_STATUS_NOT_FOUND If the function went all the way up to the tree root
 *                              and did not find consecutive NodeID/endpoint attribute types
 * @returns SL_STATUS_FAIL      If some error occurred reading values in the tree.
 */
sl_status_t attribute_store_network_helper_get_zwave_ids_from_node(
  attribute_store_node_t node,
  zwave_node_id_t *zwave_node_id,
  zwave_endpoint_id_t *zwave_endpoint_id);

#ifdef __cplusplus
}
#endif

/** @} end attribute_store_network_helpers */

#endif  //ATTRIBUTE_STORE_NETWORK_HELPER_H
