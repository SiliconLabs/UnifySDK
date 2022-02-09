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
#include "attribute_transitions.h"
#include "attribute_transitions_process.h"

// Includes from other UIC components
#include "attribute_store_helper.h"
#include "sl_log.h"

// Contiki
#include "etimer.h"

// Generic includes
#include <map>
#include <vector>

constexpr char LOG_TAG[] = "attribute_transitions";

// FIXME: UIC-806 make this timer a configurable value
constexpr clock_time_t transition_refresh_rate = 1000;  // in ms

///////////////////////////////////////////////////////////////////////////////
// Local definitions
///////////////////////////////////////////////////////////////////////////////
typedef struct transition {
  float start_value;
  float finish_value;
  clock_time_t start_time;
  clock_time_t finish_time;
} transition_t;

/**
 * @brief Event definitions for the attribute transitions.
 */
typedef enum {
  /// Start the refresh timer.
  START_REFRESH_TIMER
} attribute_transition_events_t;

// List of nodes and transition data undergoing a transition.
static std::map<attribute_store_node_t, transition_t> nodes_under_transition;
// Timer restarted at fixed interval to refresh the attribute values under transition
static struct etimer refresh_timer;

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t attribute_transitions_init()
{
  nodes_under_transition.clear();
  process_start(&attribute_transitions_process, nullptr);

  return SL_STATUS_OK;
}

int attribute_transitions_teardown()
{
  for (auto it = nodes_under_transition.begin();
       it != nodes_under_transition.end();
       ++it) {
    // We are stopping, while some are transitioning.
    // We will have to probe them again when we start
    attribute_store_undefine_reported(it->first);
  }

  nodes_under_transition.clear();
  process_exit(&attribute_transitions_process);
  return 0;
}

sl_status_t attribute_start_transition(attribute_store_node_t node,
                                       clock_time_t duration)
{
  if (is_attribute_transition_ongoing(node) == true) {
    sl_log_debug(LOG_TAG, "Re-starting a new transition on Node %d", node);
    attribute_stop_transition(node);
  }

  transition_t transition = {};
  uint32_t start_value    = 0;
  uint32_t finish_value   = 0;
  if (SL_STATUS_OK
      != attribute_store_read_value(node,
                                    REPORTED_ATTRIBUTE,
                                    (uint8_t *)&start_value,
                                    sizeof(start_value))) {
    return SL_STATUS_FAIL;
  }
  if (SL_STATUS_OK
      != attribute_store_read_value(node,
                                    DESIRED_ATTRIBUTE,
                                    (uint8_t *)&finish_value,
                                    sizeof(finish_value))) {
    return SL_STATUS_FAIL;
  }

  transition.start_value  = static_cast<float>(start_value);
  transition.finish_value = static_cast<float>(finish_value);
  transition.start_time   = clock_time();
  transition.finish_time  = clock_time() + duration;

  sl_log_debug(
    LOG_TAG,
    "Starting transition on Node %d, value goes %.0f -> %.0f in %d ms",
    node,
    transition.start_value,
    transition.finish_value,
    duration);

  if (nodes_under_transition.empty()) {
    process_post(&attribute_transitions_process, START_REFRESH_TIMER, nullptr);
  }

  nodes_under_transition.insert(
    std::pair<attribute_store_node_t, transition_t>(node, transition));

  return SL_STATUS_OK;
}

sl_status_t attribute_stop_transition(attribute_store_node_t node)
{
  sl_log_debug(LOG_TAG, "Stopping transition on node %d", node);
  nodes_under_transition.erase(node);

  if (nodes_under_transition.empty()) {
    etimer_stop(&refresh_timer);
  }

  return SL_STATUS_OK;
}

clock_time_t
  attribute_transition_get_remaining_duration(attribute_store_node_t node)
{
  clock_time_t remaining_duration = 0;
  if (nodes_under_transition.count(node)) {
    if (nodes_under_transition[node].finish_time > clock_time()) {
      remaining_duration
        = nodes_under_transition[node].finish_time - clock_time();
    }
  }
  return remaining_duration;
}

bool is_attribute_transition_ongoing(attribute_store_node_t node)
{
  return (nodes_under_transition.find(node) != nodes_under_transition.end());
}

///////////////////////////////////////////////////////////////////////////////
// Event handler functions
///////////////////////////////////////////////////////////////////////////////
static void attribute_transitions_ev_init()
{
  sl_log_debug(LOG_TAG, "Attribute transitions process started");
  nodes_under_transition.clear();
}

static void refresh_all_transitions()
{
  std::vector<attribute_store_node_t> finished_transitions;

  for (auto it = nodes_under_transition.begin();
       it != nodes_under_transition.end();
       ++it) {
    if (clock_time() >= it->second.finish_time) {
      // We are done.
      finished_transitions.push_back(it->first);
      attribute_store_set_reported_as_desired(it->first);
      continue;
    }
    // Else calculate where the node is located now,
    // between the start and the finish levels
    // We simply do a linear extrapolation
    clock_time_t time_delta = clock_time() - it->second.start_time;
    float value             = (it->second.finish_value - it->second.start_value)
                    * time_delta
                    / (it->second.finish_time - it->second.start_time)
                  + it->second.start_value;

    // Out of bounds check. Did we calculate a value located outside the
    // [start_value ; finish_value] interval ?
    if ((value < it->second.start_value) && (value < it->second.finish_value)) {
      sl_log_warning(LOG_TAG,
                     "Target value is out of bounds (smaller than both start "
                     "and finish). We probably have a calculation mistake.");
      value = it->second.finish_value;
    } else if ((value > it->second.start_value)
               && (value > it->second.finish_value)) {
      sl_log_warning(LOG_TAG,
                     "Target value is out of bounds (greater than both start "
                     "and finish). We probably have a calculation mistake.");
      value = it->second.finish_value;
    }

    if (value == it->second.finish_value) {
      finished_transitions.push_back(it->first);
    }

    // Update the reported.
    uint32_t current_value = static_cast<uint32_t>(value);
    attribute_store_set_reported(it->first,
                                 &current_value,
                                 sizeof(current_value));
  }

  // Remove all the finished transitions from the map
  for (auto it = finished_transitions.begin(); it != finished_transitions.end();
       ++it) {
    attribute_stop_transition(*it);
  }

  // Check if we restart the timer.
  if (!nodes_under_transition.empty()) {
    etimer_restart(&refresh_timer);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Contiki Process and thread
///////////////////////////////////////////////////////////////////////////////
PROCESS(attribute_transitions_process, "attribute_transitions_process");

PROCESS_THREAD(attribute_transitions_process, ev, data)
{
  PROCESS_BEGIN();
  while (1) {
    if (ev == PROCESS_EVENT_INIT) {
      attribute_transitions_ev_init();
    } else if (ev == PROCESS_EVENT_EXIT) {
      sl_log_debug(LOG_TAG, "Teardown of attribute transitions");
    } else if (ev == START_REFRESH_TIMER) {
      etimer_set(&refresh_timer, transition_refresh_rate);
    } else if ((ev == PROCESS_EVENT_TIMER) && (data == &refresh_timer)) {
      refresh_all_transitions();
    }

    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}
