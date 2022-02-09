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
 * @file zigpc_discovery_timers.hpp
 * @defgroup zigpc_discovery_timers ZigPC Discovery Internal Timers
 * @ingroup zigpc_discovery
 * @brief This module manages a list of callback timers used by zigpc_discovery.
 * callbacks with timeouts can be added removed.
 *
 * @{
 */

#ifndef ZIGPC_DISCOVERY_TIMERS_HPP
#define ZIGPC_DISCOVERY_TIMERS_HPP

#include <functional>
#include <unordered_map>

// Unify includes
#include <sys/ctimer.h>

// ZigPC includes
#include <zigpc_common_zigbee.h>

// Component includes
#include "zigpc_discovery.h"

namespace zigpc_discovery::timers
{
/**
 * @brief Callback type used to notify timeout events.
 *
 */
using callback_t
  = std::function<void(zigbee_eui64_uint_t, zigpc_discovery_status_t)>;

/**
 * @brief Clear all timers.
 *
 */
void clear(void);

/**
 * @brief Create a callback timer.
 *
 * @param eui64           Device identifier.
 * @param duration_ms     Duration to set timer.
 * @param timeout_status  Status to send on timer expiry.
 * @param timeout_cb      Callback to invoke on timer expiry.
 * @return sl_status_t    SL_STATUS_OK on successful creation,
 * SL_STATUS_NULL_POINTER on invalid callback passed in, or error otherwise.
 */
sl_status_t create(zigbee_eui64_uint_t eui64,
                   clock_time_t duration_ms,
                   zigpc_discovery_status_t timeout_status,
                   const callback_t &timeout_cb);

/**
 * @brief Remove a created callback timer.
 *
 * @param eui64           Device identifier.
 * @return sl_status_t    SL_STATUS_OK on successful removal, or error
 * otherwise.
 */
sl_status_t remove(zigbee_eui64_uint_t eui64);

}  // namespace zigpc_discovery::timers

#endif /* ZIGPC_DISCOVERY_TIMERS_HPP */

/** @} end zigpc_discovery_timers */
