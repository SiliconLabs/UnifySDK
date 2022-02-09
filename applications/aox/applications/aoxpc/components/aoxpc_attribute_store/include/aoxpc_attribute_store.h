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

/**
 * @defgroup aoxpc_attribute_store AoXPC Attribute Store
 * @brief @ref attribute_store functions specifically for AoXPC
 *
 * @{
 */

#ifndef AOXPC_ATTRIBUTE_STORE_H
#define AOXPC_ATTRIBUTE_STORE_H

#include "sl_status.h"
#include "uic_typedefs.h"

#include "attribute_store.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief Returns the UNID (attribute store) locator UNID in the attribute store
 *
 * NOTE: This function requires the UNID to be set correctly in aoxpc_unid
 * to work properly
 *
 * @returns The UNID attribute store node. ATTRIBUTE_STORE_INVALID_NODE if
 *          the attribute store does not have our UNID node.
 */
attribute_store_node_t get_aoxpc_unid_node();

/**
 * @brief Returns the Endpoint ID (attribute store) node of our current
 *        UNID for a specified Endpoint ID
 *
 * NOTE: This function requires the UNID to be set correctly in aoxpc_unid
 * to work properly
 *
 * @returns The Endpoint ID attribute store node. ATTRIBUTE_STORE_INVALID_NODE if
 *          the attribute store does not have our UNID or Endpoint ID node.
 */
attribute_store_node_t get_aoxpc_endpoint_id_node(dotdot_endpoint_id_t endpoint_id);


#ifdef __cplusplus
}
#endif

#endif  //AOXPC_ATTRIBUTE_STORE_H
/** @} end aoxpc_attribute_store */
