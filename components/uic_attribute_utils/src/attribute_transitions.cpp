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

// Includes from other Unify components
#include "attribute_store_helper.h"
#include "sl_log.h"

// Contiki
#include "sys/etimer.h"
#include "sys/ctimer.h"

// Generic includes
#include <float.h>
#include <map>
#include <vector>
#include <algorithm>
#include <memory>

constexpr char LOG_TAG[] = "attribute_transitions";

// UIC-806 make this timer a configurable value
constexpr clock_time_t transition_refresh_rate = 1000;  // in ms

///////////////////////////////////////////////////////////////////////////////
// Local definitions
///////////////////////////////////////////////////////////////////////////////
using transition_t = struct transition {
  float start_value;
  float finish_value;
  clock_time_t start_time;
  clock_time_t finish_time;
};

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

struct fixed_transition {
  attribute_store_node_t node;
  attribute_store_node_value_state_t value_type;
  float step;
  float target_value;
  struct ctimer timer;

  // Dont copy this as it has a reference
  fixed_transition(fixed_transition const &) = delete;

  fixed_transition() : timer({}) {}

  ~fixed_transition()
  {
    ctimer_stop(&timer);
  }
};

std::map<attribute_store_node_t, std::unique_ptr<fixed_transition>>
  fixed_transition_pool;

///////////////////////////////////////////////////////////////////////////////
// Private functions
///////////////////////////////////////////////////////////////////////////////
static void on_attribute_node_deleted(attribute_store_node_t deleted_node)
{
  nodes_under_transition.erase(deleted_node);
  //attribute_stop_transition(deleted_node);
}

static void fixed_transition_step(void *data)
{
  fixed_transition *transition = static_cast<fixed_transition *>(data);

  if (false
      == attribute_store_is_value_defined(transition->node,
                                          transition->value_type)) {
    // Value got undefined during a transition, just abort the transition.
    attribute_stop_transition(transition->node);
    return;
  }

  float current_value
    = attribute_store_get_number(transition->node, transition->value_type);
  float next_value = current_value + transition->step;
  //Are we there yet ?
  if (((transition->step > 0.0) && (next_value < transition->target_value))
      || ((transition->step < 0.0)
          && (next_value > transition->target_value))) {
    attribute_store_set_number(transition->node,
                               next_value,
                               transition->value_type);
    ctimer_restart(&transition->timer);
  } else {
    attribute_store_set_number(transition->node,
                               transition->target_value,
                               transition->value_type);

    attribute_stop_transition(transition->node);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions
///////////////////////////////////////////////////////////////////////////////
sl_status_t attribute_transitions_init()
{
  nodes_under_transition.clear();
  attribute_store_register_delete_callback(&on_attribute_node_deleted);
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
    sl_log_debug(LOG_TAG,
                 "Re-starting a new transition on Attribute ID %d",
                 node);
    attribute_stop_transition(node);
  }

  transition_t transition = {};
  transition.start_value  = attribute_store_get_reported_number(node);
  transition.finish_value = attribute_store_get_desired_number(node);
  if ((transition.start_value == FLT_MIN)
      || (transition.finish_value == FLT_MIN)) {
    return SL_STATUS_FAIL;
  }

  transition.start_time  = clock_time();
  transition.finish_time = clock_time() + duration;

  sl_log_debug(
    LOG_TAG,
    "Starting transition on Attribute ID %d, value goes %.0f -> %.0f in %d ms",
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
  sl_log_debug(LOG_TAG, "Stopping transition on Attribute ID %d", node);

  fixed_transition_pool.erase(node);
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
  if (fixed_transition_pool.count(node) > 0) {
    return true;
  }

  if (nodes_under_transition.count(node) > 0) {
    return true;
  }
  return false;
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
    attribute_store_set_reported_number(it->first, value);
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

sl_status_t attribute_start_fixed_transition(
  attribute_store_node_t node,
  attribute_store_node_value_state_t value_type,
  float target_value,
  float step,
  clock_time_t step_interval)
{
  attribute_stop_transition(node);

  auto result = fixed_transition_pool.insert(
    std::make_pair(node, std::make_unique<fixed_transition>()));
  auto &e = result.first->second;

  e->node         = node;
  e->value_type   = value_type;
  e->step         = step;
  e->target_value = target_value;

  ctimer_set(&e->timer, step_interval, fixed_transition_step, &(*e));
  return SL_STATUS_OK;
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
      fixed_transition_pool.clear();
    } else if (ev == START_REFRESH_TIMER) {
      etimer_set(&refresh_timer, transition_refresh_rate);
    } else if ((ev == PROCESS_EVENT_TIMER) && (data == &refresh_timer)) {
      refresh_all_transitions();
    }

    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}
