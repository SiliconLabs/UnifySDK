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
 * @defgroup zwave_tx_route_cache Z-Wave Tx Route cache
 * @ingroup zwave_tx
 * @brief Caches route data with NodeID destinations.
 *
 * The Z-Wave Tx route cache allows to cache routing information, for example
 * how many hops were last used with a destination.
 *
 * @{
 */

#ifndef ZWAVE_TX_ROUTE_CACHE_H
#define ZWAVE_TX_ROUTE_CACHE_H

#include "zwave_tx.h"
#include "zwave_node_id_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes and clear the route cache
 */
void zwave_tx_route_cache_init();

/**
 * @brief Construct a new zwave tx route cache set number of repeaters object
 *
 * @param destination_node_id   The NodeID we are communicating with
 * @param number_of_repeaters   The number of repeaters that were used in the
 *                              last communication
 */
void zwave_tx_route_cache_set_number_of_repeaters(
  zwave_node_id_t destination_node_id, uint8_t number_of_repeaters);

/**
 * @brief Returns the cached number of repeaters for a destination NodeID
 *
 * @param destination_node_id   The NodeID we are communicating with
 * @returns uint8_t value for the number of repeaters. Returns 0 if unknown.
 */
uint8_t zwave_tx_route_cache_get_number_of_repeaters(
  zwave_node_id_t destination_node_id);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_TX_ROUTE_CACHE_H
/** @} end zwave_tx_route_cache */
