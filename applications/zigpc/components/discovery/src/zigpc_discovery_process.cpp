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

#include <memory>
#include <queue>

// Shared Unify includes
#include <sl_log.h>
#include <sl_status.h>

// ZigPC includes
#include <zigpc_datastore.h>
#include <zigpc_common_zigbee.h>

// Component includes
#include "zigpc_discovery.h"
#include "zigpc_discovery.hpp"
#include "zigpc_discovery_callbacks.hpp"
#include "zigpc_discovery_events.hpp"
#include "zigpc_discovery_process.hpp"
#include "zigpc_discovery_timers.hpp"

PROCESS(zigpc_discovery_process, "ZigPC Discovery process");

static constexpr char LOG_TAG[] = "zigpc_discovery";

static constexpr process_event_t QUEUE_EVENT = 0x5;

namespace zigpc_discovery::process
{
using queue_t = std::queue<event_t>;

/**
 * @brief Get the queue object
 *
 * @return queue_t&
 */
static queue_t &get_queue(void)
{
  static queue_t q;
  return q;
}

void clear_queue(void)
{
  queue_t &q = get_queue();
  while (!q.empty()) {
    q.pop();
  }
}

/**
 * @brief Remove and return an event from the process queue.
 *
 * @return event_t
 */
static event_t dequeue_event(void)
{
  queue_t &q = get_queue();

  event_t event = std::move(q.front());

  q.pop();

  return event;
}

size_t queue_size(void)
{
  return get_queue().size();
}

sl_status_t enqueue_event(event_t &&event)
{
  get_queue().push(std::move(event));

  process_post(&zigpc_discovery_process, QUEUE_EVENT, nullptr);

  return SL_STATUS_OK;
}
const event_t *peek_event(void)
{
  const queue_t &q = get_queue();
  if (q.empty()) {
    return nullptr;
  } else {
    return &get_queue().front();
  }
}

}  // namespace zigpc_discovery::process

// //////////////
// Public API
// //////////////

sl_status_t zigpc_discovery_fixt_setup(void)
{
  zigpc_discovery::callbacks::clear();
  zigpc_discovery::process::clear_queue();

  sl_status_t status = zigpc_discovery_register_gateway_callbacks();

  if (status != SL_STATUS_OK) {
    return status;
  }

  process_start(&zigpc_discovery_process, nullptr);

  return status;
}

int zigpc_discovery_fixt_shutdown(void)
{
  return 0;
}

// //////////////
// Contiki Process
// //////////////

PROCESS_THREAD(zigpc_discovery_process, ev, data)
{
  PROCESS_BEGIN()
  while (true) {
    if ((ev == QUEUE_EVENT) && (zigpc_discovery::process::queue_size() > 0)) {
      zigpc_discovery::process::event_t event
        = zigpc_discovery::process::dequeue_event();

      sl_status_t status = event->handle();
      sl_log_debug(LOG_TAG, "%s: Handler status: 0x%X", event->label, status);
    }

    PROCESS_WAIT_EVENT();
  }

  PROCESS_END()
}
