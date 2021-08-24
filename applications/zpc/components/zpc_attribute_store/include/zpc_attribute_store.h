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
 * @file zpc_attribute_store.h
 * @brief Attribute store functions specifically for ZPC
 *
 *
 * @{
 */

#ifndef ZPC_ATTRIBUTE_STORE_H
#define ZPC_ATTRIBUTE_STORE_H

#include "sl_status.h"
#include "attribute_store.h"
#include "zwave_unid.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Verify if a UNID is the ZPC's UNID
 *
 * NOTE: This function requires network management to be initialized to work
 * properly.
 *
 * @param unid The UNID to compare with our UNID.
 * @returns true if the unid passed in argument is our unid. false otherwise.
 */
bool is_zpc_unid(const unid_t unid);

/**
 * @brief Returns the HomeID (attribute store) node of our current network
 *
 * NOTE: This function requires network management to be initialized to work
 * properly.
 * @returns The HomeID attribute store node. ATTRIBUTE_STORE_INVALID_NODE if
 *          the attribute store does not have our HomeID node.
 */
attribute_store_node_t get_zpc_network_node();

/**
 * @brief Returns the NodeID (attribute store) node of our current network / NodeID
 *
 * NOTE: This function requires network management to be initialized to work
 * properly.
 * @returns The HomeID attribute store node. ATTRIBUTE_STORE_INVALID_NODE if
 *          the attribute store does not have our HomeID node.
 * @returns The NodeID attribute store node. ATTRIBUTE_STORE_INVALID_NODE if
 *          the attribute store does not have our HomeID or NodeID node.
 */
attribute_store_node_t get_zpc_node_id_node();

/**
 * @brief Returns the Endpoint ID (attribute store) node of our current
 *        network / NodeID for a specified Endpoint ID
 *
 * NOTE: This function requires network management to be initialized to work
 * properly.
 * @returns The Endpoint ID attribute store node. ATTRIBUTE_STORE_INVALID_NODE if
 *          the attribute store does not have our HomeID, NodeID or Endpoint ID node.
 */
attribute_store_node_t
  get_zpc_endpoint_id_node(uint8_t endpoint_id);
  ///FIXME: Change uin8_t to zwave_endpoint_id_t

/**
 * @brief Initialize attribute store specifically for ZPC
 *
 * Refreshes all callbacks for HOME_ID children.
 *
 * @return SL_STATUS_OK if all callbacks was initialized.
 * @return SL_STATUS_FAIL if invokking any callback fails.
 */
sl_status_t zpc_attribute_store_init(void);

#ifdef __cplusplus
}
#endif

#endif  //ZPC_ATTRIBUTE_STORE_H
/** @} end zpc_attribute_store */
