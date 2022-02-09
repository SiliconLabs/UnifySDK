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

#include <cinttypes>
#include <list>
#include <unordered_map>

// Unify includes
#include <sl_log.h>

// ZigPC includes
#include <zigpc_common_zigbee.h>

// Component includes
#include "zigpc_discovery.h"
#include "zigpc_discovery_callbacks.hpp"

static constexpr char LOG_TAG[] = "zigpc_discovery";

using callback_map_t
  = std::unordered_map<zigbee_eui64_uint_t,
                       std::list<zigpc_discovery_status_callback_t>>;

/**
 * @brief Helper to return string representation of discovery status.
 *
 * @param status
 * @return std::string
 */
static std::string to_string(zigpc_discovery_status status)
{
  std::string str;
  switch (status) {
    case DISCOVERY_START:
      str = "DISCOVERY_START";
      break;
    case DISCOVERY_SUCCESS:
      str = "DISCOVERY_SUCCESS";
      break;
    case DEVICE_DISCOVERY_FAIL:
      str = "DEVICE_DISCOVERY_FAIL";
      break;
    case ENDPOINT_DISCOVERY_FAIL:
      str = "ENDPOINT_DISCOVERY_FAIL";
      break;
    default:
      sl_log_error(LOG_TAG,
                   "DiscoveryCallback: Unknown discovery status: %d",
                   status);
      str = "INVALID";
      break;
  }

  return str;
}

/**
 * @brief Retrieve call
 *
 * @return callback_map_t&
 */
static callback_map_t &get_callback_map(void)
{
  static callback_map_t map;
  return map;
}

void zigpc_discovery::callbacks::clear(void)
{
  get_callback_map().clear();
}

void zigpc_discovery::callbacks::notify(zigbee_eui64_uint_t eui64,
                                        zigpc_discovery_status status)
{
  callback_map_t &callback_map = get_callback_map();
  auto callbacks_found         = callback_map.find(eui64);
  if (callbacks_found != callback_map.end()) {
    for (auto cb: callbacks_found->second) {
      cb(eui64, status);
    }
  }

  std::string status_label = to_string(status);

  sl_log_debug(LOG_TAG,
               "DiscoveryCallback: Notifying listeners %s for Eui64:%016" PRIX64
               "",
               status_label.c_str(),
               eui64);

  // Notify any callbacks that is listening to all device updates
  callbacks_found = callback_map.find(WILDCARD_EUI64);
  if (callbacks_found != callback_map.end()) {
    for (auto cb: callbacks_found->second) {
      cb(eui64, status);
    }
  }
}

sl_status_t zigpc_discovery::callbacks::add(
  zigbee_eui64_uint_t eui64, const zigpc_discovery_status_callback_t callback)
{
  if (callback == nullptr) {
    return SL_STATUS_NULL_POINTER;
  }

  callback_map_t &callback_map = get_callback_map();
  callback_map[eui64].push_back(callback);

  return SL_STATUS_OK;
}

sl_status_t
  zigpc_discovery::callbacks::remove_for_device(zigbee_eui64_uint_t eui64)
{
  if (eui64 == WILDCARD_EUI64) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  get_callback_map().erase(eui64);

  return SL_STATUS_OK;
}

sl_status_t zigpc_discovery::callbacks::remove(
  zigbee_eui64_uint_t eui64, const zigpc_discovery_status_callback_t callback)
{
  if (callback == nullptr) {
    return SL_STATUS_NULL_POINTER;
  }

  callback_map_t &callback_map = get_callback_map();
  auto callbacks_found         = callback_map.find(eui64);
  if (callbacks_found != callback_map.end()) {
    callbacks_found->second.remove(callback);
  }

  return SL_STATUS_OK;
}
