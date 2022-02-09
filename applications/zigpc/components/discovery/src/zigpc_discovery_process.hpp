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
 * @file zigpc_discovery_process.hpp
 * @defgroup zigpc_discovery_process ZigPC Discovery Internal Process Queue
 * @ingroup zigpc_discovery
 * @brief This module provides the interface to add events to be added as
 * Contiki events to be processed within the zigpc_discovery context.
 *
 * @{
 */

#ifndef ZIGPC_DISCOVERY_PROCESS_HPP
#define ZIGPC_DISCOVERY_PROCESS_HPP

#include <memory>
#include <queue>

// Shared Unify includes
#include <sl_status.h>

// ZigPC includes
#include <zigpc_common_zigbee.h>

// Component includes
#include "zigpc_discovery_events.hpp"

namespace zigpc_discovery::process
{
using event_t = std::unique_ptr<events::DiscoverEvent>;

/**
 * @brief Retrieve the current number of events in the process queue.
 *
 * @return size_t process queue size.
 */
size_t queue_size(void);

/**
 * @brief Add an event to be processed in the zigpc_discovery process context.
 *
 * @return event_t      Pointer to event, or nullptr if the queue is empty.
 */
const event_t *peek_event(void);

/**
 * @brief Add an event to be processed in the zigpc_discovery process context.
 *
 * @param event         Reference to discovery event.
 * @return sl_status_t  SL_STATUS_OK on success, error otherwise.
 */
sl_status_t enqueue_event(event_t &&event);

/**
 * @brief Remove all events from process queue.
 *
 */
void clear_queue(void);

}  // namespace zigpc_discovery::process

#endif /* ZIGPC_DISCOVERY_PROCESS_HPP */

/** @} end zigpc_discovery_process */
