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
#include "zwave_tx_route_cache.h"

// Generic includes
#include <array>

// Unify shared components
#include "sl_log.h"
constexpr char LOG_TAG[] = "zwave_tx_group_cache";

using route_cache_data_t = struct {
  // Z-Wave NodeID with who we are communicating (singlecast)
  zwave_node_id_t node_id;
  // Number of repeaters used to communicate with a NodeID.
  uint8_t number_of_repeaters;
};

namespace
{
// Array of routing cache.
std::array<route_cache_data_t, ZWAVE_TX_ROUTE_CACHE_BUFFER_SIZE> route_cache;
}  // namespace

void zwave_tx_route_cache_init()
{
  // Zero out the route cache at init.
  route_cache.fill({0, 0});
}

void zwave_tx_route_cache_set_number_of_repeaters(
  zwave_node_id_t destination_node_id, uint8_t number_of_repeaters)
{
  for (auto &cache: route_cache) {
    if (cache.node_id == destination_node_id) {
      cache.number_of_repeaters = number_of_repeaters;
      if (0 == number_of_repeaters) {
        cache.node_id = 0;
      }
      return;
    }
  }

  // No NodeID found in the existing cache, find the first available slot if the
  // number of repeaters is > 0.
  if (number_of_repeaters == 0) {
    return;
  }
  for (auto &cache: route_cache) {
    if (cache.node_id == 0) {
      cache.node_id             = destination_node_id;
      cache.number_of_repeaters = number_of_repeaters;
      return;
    }
  }

  sl_log_debug(LOG_TAG,
               "No more queue space to save routed destinations. "
               "Ignoring");
}

uint8_t zwave_tx_route_cache_get_number_of_repeaters(
  zwave_node_id_t destination_node_id)
{
  for (const auto &cache: route_cache) {
    if (cache.node_id == destination_node_id) {
      return cache.number_of_repeaters;
    }
  }
  return 0;
}
