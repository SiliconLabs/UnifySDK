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
 * @file zwapi_utils.h
 * @brief Internal utility helpers for Z-Wave API
 */

#ifndef ZWAPI_UTILS_H
#define ZWAPI_UTILS_H

#include "zwave_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Read the node ID from buffer based on node ID basetype. Note that
 *        this helper also advances the index.
 * @returns zwave_node_id_t parsed node ID
 */
zwave_node_id_t zwapi_read_node_id(uint8_t *pData, uint8_t *index);

/**
 * @brief Write the node ID in buffer based on node ID basetype. Note that
 *        this helper also advances the index.
 */
void zwapi_write_node_id(uint8_t *pData, uint8_t *index, zwave_node_id_t node_id);

#ifdef __cplusplus
}
#endif

#endif  // ZWAPI_UTILS_H
