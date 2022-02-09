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

#include <algorithm>
#include <cinttypes>
#include <memory>
#include <unordered_map>

// Unify includes
#include <sl_log.h>
#include <sys/ctimer.h>

// ZigPC includes
#include <zigpc_common_zigbee.h>

// Component includes
#include "zigpc_discovery.h"
#include "zigpc_discovery_timers.hpp"

static constexpr char LOG_TAG[] = "zigpc_discovery";

static constexpr char LOG_FMT_FOR_DEVICE[]
  = "CallbackTimer: %s for Eui64:%016" PRIX64 ": 0x%X";
static constexpr char LOG_FMT_TIMEOUT_FOR_DEVICE[]
  = "CallbackTimer: Timeout reached after %ums for Eui64:%016" PRIX64 ": 0x%X";

namespace zigpc_discovery::timers
{
/**
   * @brief Timer entry that stored information regarding timer and notify
   * status on expiry.
   *
   */
class CallbackTimer
{
  private:
  zigbee_eui64_uint_t eui64; /**< Device identifier */
  ctimer timer = {};         /**< Timer instance to be stored in Contiki */
  clock_time_t duration_ms;  /**< Timer duration */
  zigpc_discovery_status_t timeout_status; /**< Expired timer notify status */
  callback_t callback;                     /**< Timer expiry callback */

  /**
   * @brief Callback invoked by ctimer to notify a Contiki timer has expired.
   *
   * @param data Parameter passed as data in ctimer_set()
   */
  static void on_ctimer_expired(void *data)
  {
    if (data == nullptr) {
      return;
    }

    // Get information regarding timeout. This should point to a the eui64
    // member of the CallbackTimer instance (Set in contructor)
    CallbackTimer &timer = *static_cast<CallbackTimer *>(data);

    timer.on_timeout();
  }

  /**
   * @brief Timer expired callback handler
   *
   */
  void on_timeout()
  {
    sl_log_debug(LOG_TAG, LOG_FMT_TIMEOUT_FOR_DEVICE, duration_ms, eui64);
    callback(eui64, timeout_status);
  }

  public:
  explicit CallbackTimer(zigbee_eui64_uint_t eui64,
                         clock_time_t duration_ms,
                         zigpc_discovery_status_t timeout_status,
                         const callback_t &callback) :
    eui64(eui64),
    duration_ms(duration_ms),
    timeout_status(timeout_status),
    callback(callback)
  {
    timer.next = nullptr;
    ctimer_set(&timer, duration_ms, on_ctimer_expired, this);
  }

  ~CallbackTimer()
  {
    ctimer_stop(&timer);
  }
};

using timer_map_t
  = std::unordered_map<zigbee_eui64_uint_t, std::unique_ptr<CallbackTimer>>;

static timer_map_t &get_map(void)
{
  static timer_map_t stored_timers;
  return stored_timers;
}

void clear(void)
{
  get_map().clear();
}

sl_status_t create(zigbee_eui64_uint_t eui64,
                   clock_time_t duration_ms,
                   zigpc_discovery_status_t timeout_status,
                   const callback_t &callback)
{
  if (callback == nullptr) {
    sl_log_error(LOG_TAG,
                 LOG_FMT_FOR_DEVICE,
                 "Callback pointer not set",
                 eui64,
                 SL_STATUS_NULL_POINTER);
    return SL_STATUS_NULL_POINTER;
  }

  timer_map_t &map = get_map();

  // Remove existing timer
  auto found_device = map.find(eui64);
  if (found_device != map.end()) {
    map.erase(eui64);
  }

  // NOTE: References to elements in unordered_map are valid in all cases, even after a rehash
  map.emplace(eui64,
              std::make_unique<CallbackTimer>(eui64,
                                              duration_ms,
                                              timeout_status,
                                              callback));

  return SL_STATUS_OK;
}

sl_status_t remove(zigbee_eui64_uint_t eui64)
{
  timer_map_t &map = get_map();

  auto found_entry = map.find(eui64);
  if (found_entry == map.end()) {
    sl_log_warning(LOG_TAG,
                   LOG_FMT_FOR_DEVICE,
                   "Failed to find timer to remove",
                   eui64,
                   SL_STATUS_NOT_FOUND);
    // NOTE: Ignore requests that haven't been stored
    return SL_STATUS_OK;
  }

  map.erase(eui64);

  return SL_STATUS_OK;
}

}  // namespace zigpc_discovery::timers
