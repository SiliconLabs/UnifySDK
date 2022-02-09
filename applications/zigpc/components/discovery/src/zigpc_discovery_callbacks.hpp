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
 * @file zigpc_discovery_callbacks.hpp
 * @defgroup zigpc_discovery_callbacks ZigPC Discovery Internal Callback Storage
 * @ingroup zigpc_discovery
 * @brief This module provides the ability to add/remove/invoke discovery-based
 * callbacks. Both device-specific and \ref WILDCARD_EUI64 based callbacks can
 * be added and removed.
 *
 * @{
 */

#ifndef ZIGPC_DISCOVERY_CALLBACKS_HPP
#define ZIGPC_DISCOVERY_CALLBACKS_HPP

// Shared Unify includes
#include <sl_status.h>

// ZigPC includes
#include <zigpc_common_zigbee.h>

// Component includes
#include "zigpc_discovery.h"

namespace zigpc_discovery::callbacks
{
/**
 * @brief Wildcard identifier to be used when adding/removing callbacks.
 *
 * Using this EUI64 will allow listeners to be notified of every device
 * discovery status update.
 *
 */
constexpr zigbee_eui64_uint_t WILDCARD_EUI64 = ZIGBEE_NULL_EUI64_UINT;

/**
 * @brief Clear all registered callbacks.
 *
 * NOTE: This should be called at initialization.
 *
 */
void clear(void);

/**
 * @brief Invoke registered callbacks the discovery status for the device.
 *
 * This function also invokes callbacks registered using /ref WILDCARD_EUI64.
 *
 * @param eui64   Device Identifier.
 * @param status  Corresponding discovery status.
 */
void notify(zigbee_eui64_uint_t eui64, zigpc_discovery_status status);

/**
 * @brief Add a callback to be invoked for a specific EUI64 (or
 * \ref WILDCARD_EUI64 for all devices).
 *
 * @param eui64     Device identifier.
 * @param callback  Callback to register.
 * @return sl_status_t SL_STATUS_OK on success, SL_STATUS_NULL_POINTER on
 * invalid callbacks passed, or error otherwise.
 */
sl_status_t add(zigbee_eui64_uint_t eui64,
                const zigpc_discovery_status_callback_t callback);

/**
 * @brief Remove all registered callbacks for a specific EUI64.
 *
 * NOTE: Cannot remove callbacks registered to \ref WILDCARD_EUI64
 *
 * @param eui64        Device identifier.
 * @return sl_status_t SL_STATUS_OK on success, SL_STATUS_INVALID_PARAMETER on
 * \ref WILDCARD_EUI64 passed in, or error otherwise.
 */
sl_status_t remove_for_device(zigbee_eui64_uint_t eui64);

/**
 * @brief Remove a registered callback for a specific EUI64 (or
 * \ref WILDCARD_EUI64 for all devices).
 *
 * @param eui64        Device identifier.
 * @param callback     Callback to remove.
 * @return sl_status_t SL_STATUS_OK on success, SL_STATUS_NULL_POINTER on
 * invalid callbacks passed, or error otherwise.
 */
sl_status_t remove(zigbee_eui64_uint_t eui64,
                   const zigpc_discovery_status_callback_t callback);

}  // namespace zigpc_discovery::callbacks

#endif /* ZIGPC_DISCOVERY_CALLBACKS_HPP */

/** @} end zigpc_discovery_callbacks */
