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
 * @defgroup aoxpc_unid AoXPC Unid
 * @brief Modules that allows to set the BLE address of the AoXPC and subsequently
 * query its Unify UNID.
 *
 * @{
 */

#ifndef AOXPC_UNID_H
#define AOXPC_UNID_H

// Generic includes
#include <stddef.h>

#include "sl_status.h"

// Includes from other components
#include "aoa_util.h"

// Endpoint ID of the AoX Locator cluster.
#define AOXPC_ENDPOINT_ID 0

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Retrieves the UNID for the AoXPC
 *
 * @param [out] aoxpc_unid Pointer to copy the AoXPC UNID.
 *
 * @return SL_STATUS_OK if the AoXPC UNID was copied to the aoxpc_unid pointer,
 * any other code in case of error.
 */
sl_status_t get_aoxpc_unid(aoa_id_t aoxpc_unid);

/**
 * @brief Set the AoXPC BLE address
 *
 * The Bluetooth address of the AoXPC MUST be set before the its UNID can be
 * retrieved
 *
 * @param address       Pointer to the BLE address.
 * @param address_type  Address type
 * @return SL_STATUS_OK if successful, other values in case of error.
 */
sl_status_t set_aoxpc_address(const uint8_t *address, uint8_t address_type);

/**
 * @brief Verifies if a UNID is the AoXPC UNID
 *
 * @param unid UNID to compare with the AoXPC UNID.
 *
 * @return true if the AoXPC UNID is identical to the unid argument
 * @return false if the Aox PC UNID is not set or is different than the unid
 *         argument
 */
bool is_aoxpc_unid(const aoa_id_t unid);

#ifdef __cplusplus
}
#endif

#endif  //AOXPC_UNID_H
/** @} end aoxpc_unid */
