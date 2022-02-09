/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "attribute_timeouts.h"

// Includes from other UIC components
#include "attribute_store_helper.h"
#include "sl_log.h"

// Contiki
#include "ctimer.h"

// Generic includes
#include <map>
#include <vector>

constexpr char LOG_TAG[] = "attribute_timeouts";

// Private functions prototypes
static void attribute_timeout_restart_watch_timer();
static void attribute_timeout_invoke_timeout_functions(void *user);
static void on_attribute_node_deleted(attribute_changed_event *event);

///////////////////////////////////////////////////////////////////////////////
// Local definitions
///////////////////////////////////////////////////////////////////////////////
typedef struct attribute_timeout {
  // Timestamp from when we can call the callback
  clock_time_t timestamp;
  // Callback function to invoke
  attribute_timeout_callback_t callback_function;
} attribute_timeout_t;

///////////////////////////////////////////////////////////////////////////////
// Private variables
///////////////////////////////////////////////////////////////////////////////
// List of registered timeouts for attributes.
static std::multimap<attribute_store_node_t, attribute_timeout_t>
  attribute_timeouts;

// Private timer for timeouts
static struct ctimer watch_timer;

///////////////////////////////////////////////////////////////////////////////
// Private helper functions
///////////////////////////////////////////////////////////////////////////////
static void attribute_timeout_restart_watch_timer()
{
  // Find out among all timeouts, who is next to "expire"
  clock_time_t next_timeout                  = 0;
  clock_time_t now                           = clock_time();
  attribute_store_node_t next_node_to_expire = ATTRIBUTE_STORE_INVALID_NODE;

  // Go through all the nodes pending a Get Command response
  for (auto item: attribute_timeouts) {
    clock_time_t timeout = item.second.timestamp;

    if ((timeout > now) && ((next_timeout == 0) || (timeout < next_timeout))) {
      next_timeout        = timeout;
      next_node_to_expire = item.first;
    }
  }

  // Do we have a next timeout?
  if (next_timeout != 0) {
    clock_time_t time_until_new_timeout = next_timeout - now;
    ctimer_set(&watch_timer,
               time_until_new_timeout,
               &attribute_timeout_invoke_timeout_functions,
               nullptr);

    sl_log_debug(
      LOG_TAG,
      "(Re-)Started attribute watch timer for %u ms. Next node to expire: %d",
      time_until_new_timeout,
      next_node_to_expire);
  }
}

static void attribute_timeout_invoke_timeout_functions(void *user)
{
  clock_time_t now             = clock_time();
  bool check_for_more_timeouts = false;

  for (auto it = attribute_timeouts.begin(); it != attribute_timeouts.end();
       ++it) {
    if (it->second.timestamp <= now) {
      // Save the callback data
      attribute_store_node_t node           = it->first;
      attribute_timeout_callback_t callback = it->second.callback_function;
      sl_log_debug(LOG_TAG,
                   "Timeout for Attribute ID %d. Invoking callback",
                   node);

      // First erase from the container.
      // in case the callback re-register another timeout
      attribute_timeouts.erase(it);

      // Then invoke the callback
      callback(node);
      // Then exit, we invalidated our container iteration by modifying it.
      check_for_more_timeouts = true;
      break;
    }
  }

  // Is there more that expired?
  if (true == check_for_more_timeouts) {
    attribute_timeout_invoke_timeout_functions(nullptr);
  }

  // finally restart our timer.
  attribute_timeout_restart_watch_timer();
}

static void on_attribute_node_deleted(attribute_changed_event *event)
{
  if (event->change != ATTRIBUTE_DELETED) {
    return;
  }
  // Cancel all the callbacks for that node, if we had any.
  if (attribute_timeouts.count(event->updated_node)) {
    attribute_timeouts.erase(event->updated_node);
    // Check if that affects our timer:
    attribute_timeout_restart_watch_timer();
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t attribute_timeouts_init()
{
  attribute_store_register_callback(&on_attribute_node_deleted);
  attribute_timeouts.clear();

  return SL_STATUS_OK;
}

int attribute_timeouts_teardown()
{
  attribute_timeouts.clear();
  ctimer_stop(&watch_timer);
  return 0;
}

sl_status_t
  attribute_timeout_set_callback(attribute_store_node_t node,
                                 clock_time_t duration,
                                 attribute_timeout_callback_t callback_function)
{
  // Don't crash by accepting any function.
  if (callback_function == nullptr) {
    sl_log_warning(LOG_TAG,
                   "Attribute timeout callback rejected for Attribute ID %d",
                   node);
    return SL_STATUS_FAIL;
  }

  if (duration == 0) {
    // Don't bother starting the timer and call the callback immediately
    callback_function(node);
    return SL_STATUS_OK;
  }

  // First cancel if the callback was already there:
  attribute_timeout_cancel_callback(node, callback_function);

  // Now we can safely add a new one, since we know the node/callback combination
  // is not in our list
  attribute_timeout new_timeout = {};
  new_timeout.callback_function = callback_function;
  new_timeout.timestamp         = clock_time() + duration;

  attribute_timeouts.insert(std::make_pair(node, new_timeout));
  sl_log_debug(LOG_TAG,
               "Starting timeout for Attribute ID %d with duration: %lu ms",
               node,
               duration);

  // Make sure our timer runs against the nearest timeout:
  if (!attribute_timeouts.empty()) {
    attribute_timeout_restart_watch_timer();
  }

  return SL_STATUS_OK;
}

sl_status_t attribute_timeout_cancel_callback(
  attribute_store_node_t node, attribute_timeout_callback_t callback_function)
{
  auto range = attribute_timeouts.equal_range(node);
  for (auto it = range.first; it != range.second; it++) {
    if (it->second.callback_function == callback_function) {
      attribute_timeouts.erase(it);
      return SL_STATUS_OK;
    }
  }

  return SL_STATUS_NOT_FOUND;
}
