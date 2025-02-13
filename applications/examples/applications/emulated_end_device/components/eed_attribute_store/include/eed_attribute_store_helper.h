/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef EED_ATTRIBUTE_STORE_HELPER_H
#define EED_ATTRIBUTE_STORE_HELPER_H

#include "attribute_store.h"
#include "sl_status.h"
#include "uic_typedefs.h"

/**
 * @defgroup eed_attribute_store_helper EED Attribute Store Helper
 * @ingroup eed_attribute_store_helper
 * @brief Helper functions for the Attribute Store
 *
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Take a UNID and return its Attribute Store node.
 *
 * @param unid  UNID to find in the Attribute Store tree
 *
 * @returns attribute_store_node_t representing the UNID in the attribute store.
 * @returns ATTRIBUTE_STORE_INVALID_NODE if no matching UNID node exists
 */
attribute_store_node_t eed_attribute_store_get_unid_node(dotdot_unid_t unid);

/**
 * @brief Take a UNID/Endpoint and return its Attribute Store node.
 *
 * @param unid  UNID to find in the Attribute Store tree
 * @param endpoint_id  Endpoint ID to find in the Attribute Store tree
 *
 * @returns attribute_store_node_t representing the UNID/Endpoint in the attribute store.
 * @returns ATTRIBUTE_STORE_INVALID_NODE if no matching UNID node exists
 */
attribute_store_node_t
  eed_attribute_store_get_endpoint_node(dotdot_unid_t unid,
                                        dotdot_endpoint_id_t endpoint_id);

/**
 * @brief Take an Attribute Store node and returns its UNID
 *
 * @param node          Attribute Store node to find the UNID from
 * @param unid          Pointer where the UNID will be written
 *
 * @returns SL_STATUS_OK if UNID was fetched, any other value otherwise
 */
sl_status_t eed_attribute_store_get_unid_from_node(attribute_store_node_t node,
                                                   char *unid);

/**
 * @brief Take an Attribute Store node and returns its UNID/Endpoint
 *
 * @param node          Attribute Store node to find the UNID from
 * @param unid          Pointer where the UNID will be written
 * @param endpoint_id   Pointer where the Endpoint ID will be written
 *
 * @returns SL_STATUS_OK if UNID/endpoint values were fetched, any other value otherwise
 */
sl_status_t eed_attribute_store_get_endpoint_from_node(
  attribute_store_node_t node, char *unid, dotdot_endpoint_id_t *endpoint_id);

#ifdef __cplusplus
}
#endif

/** @} end eed_attribute_store_helper */

#endif  //EED_ATTRIBUTE_STORE_HELPER_H
