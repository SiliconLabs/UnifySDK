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

/**
 * @file zwave_unid.h
 * @ingroup zpc_utils
 * @brief UNID mapping functions
 *
 * This module provides utility function for constructing and parsing
 * Unique Node IDentifiers(UNID)
 *
 * @addtogroup zpc_utils
 * @{
 */

#ifndef ZWAVE_UNID_H
#define ZWAVE_UNID_H

#ifdef __cplusplus
extern "C" {
#endif

#include "zwave_utils.h"
#include "sl_status.h"

typedef char unid_t[17];

/**
 * @brief Form a UNID.
 *
 * This function creates a UNID from the provided node id and the current home
 * ID.
 *
 * @param node_id Node ID to form the UNID from
 * @param unid The UNID output
 */
void zwave_unid_from_node_id(zwave_node_id_t node_id, unid_t unid);

/**
 * @brief Convert a UNID to a node ID.
 *
 * Provided a UNID, this function returns the node ID.
 *
 * @param unid UNID to parse
 * @param node_id Node ID output, which must be a valid pointer.
 * @return sl_status_t
 *  - SL_STATUS_OK                if node id could be mapped
 *  - SL_STATUS_INVALID_PARAMETER format of the UNID could not be passed.
 *  - SL_STATUS_INVALID_KEY       home id does not match the current home id
 *  - SL_STATUS_INVALID_RANGE     Node id is not in a valid range.
 */
sl_status_t zwave_unid_to_node_id(const unid_t unid,zwave_node_id_t* node_id);

/**
 * @brief Convert a UNID to a Home ID.
 *
 * Provided a UNID, this function return the detected Home ID.
 *
 * @param unid    UNID to parse
 * @param home_id HomeID output, must be a valid pointer.
 * @return sl_status_t
 *  - SL_STATUS_OK                if node id could be mapped
 *  - SL_STATUS_INVALID_PARAMETER format of the UNID could not be parsed correctly.
 */
sl_status_t zwave_unid_to_home_id(const unid_t unid, zwave_home_id_t *home_id);

/**
 * @brief Set the Z-Wave home ID to create UNIDs from.
 *
 * @deprecated This function may be deprecated when we start using the attribute
 * system.
 *
 * @param home_id home id
 */
void zwave_unid_set_home_id(zwave_home_id_t home_id);

#ifdef __cplusplus
}
#endif
/** @} end zwave_unid */

#endif  //ZWAVE_UNID_H
