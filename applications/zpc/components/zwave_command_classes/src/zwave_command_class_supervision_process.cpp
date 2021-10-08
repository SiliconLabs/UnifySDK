
/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
// Includes from this component
#include "zwave_command_class_supervision.h"
#include "zwave_command_class_supervision_internals.h"
#include "zwave_command_class_supervision_process.h"

// Includes from other components
#include "ZW_classcmd.h"
#include "sl_log.h"

// ZPC includes
#include "zwave_tx_groups.h"

// Generic includes
#include <map>
#include <cstdlib>
#include <random>
#include <string.h>

constexpr char LOG_TAG[] = "supervision_process";

////////////////////////////////////////////////////////////////////////////////
// Private variables
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Supervision Timer used to periodically check for expired supervision
 * sessions
 */
static struct etimer supervision_timer;

// List of Supervision sessions that are monitored.
std::map<supervision_id_t, supervised_session_t> sessions;
typedef std::map<supervision_id_t, supervised_session_t>::iterator
  session_iterator_t;

// Variable for the next session-id to assign for a transmission
static supervision_id_t next_supervision_id = 1;
static uint8_t next_session_id              = 0;

static void increment_unique_supervision_id()
{
  next_supervision_id += 1;
  if (next_supervision_id == INVALID_SUPERVISION_ID) {
    next_supervision_id += 1;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Private event functions
////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Restarts or stop the supervision timer towards the next supervision
 * session to expire.
 */
static void zwave_command_class_supervision_restart_timer()
{
  etimer_stop(&supervision_timer);
  if (sessions.size() == 0) {
    return;
  }

  // Else find the next supervision session to expire
  clock_time_t next_expiry = 0;
  for (auto it = sessions.begin(); it != sessions.end(); ++it) {
    if (it->second.expiry_time == 0) {
      continue;
    }
    if (next_expiry == 0) {
      next_expiry = it->second.expiry_time;
    } else if (it->second.expiry_time < next_expiry) {
      next_expiry = it->second.expiry_time;
    }
  }

  // Set the timer and leave extra room for the Supervision Report to come to us.
  if (next_expiry != 0) {
    if (next_expiry < clock_time()) {
      // Already expired! Restart the timer in 1ms and get rid of
      // the already expired sessions
      etimer_set(&supervision_timer, 1);
    } else {
      etimer_set(&supervision_timer, next_expiry - clock_time());
    }
  }
}

/**
 * @brief Goes around and terminate the sessions that expired.
 */
static void supervision_process_on_timer_expired_event(etimer *timer)
{
  clock_time_t now = clock_time();
  std::vector<supervision_id_t> expired_sessions;
  for (session_iterator_t it = sessions.begin(); it != sessions.end(); ++it) {
    if (it->second.expiry_time <= now) {
      sl_log_debug(LOG_TAG,
                   "Timed out waiting for a Supervision Report for NodeID "
                   "%d:%d Session ID %d. "
                   "Considering Supervision Session failed",
                   it->second.session.node_id,
                   it->second.session.endpoint_id,
                   it->second.session.session_id);
      expired_sessions.push_back(it->first);
      if (it->second.callback != nullptr) {
        it->second.callback(SUPERVISION_REPORT_FAIL,
                            it->second.tx_info_valid ? &it->second.tx_info
                                                     : nullptr,
                            it->second.user);
        // Break in case the callbacks triggered some supervision session closure.
        break;
      }
    }
  }
  // Remove them from the sessions list.
  for (auto &session: expired_sessions) {
    sessions.erase(session);
  }

  //Restart the timer for the next sessions.
  zwave_command_class_supervision_restart_timer();
}

////////////////////////////////////////////////////////////////////////////////
// Process functions (shared among the component)
////////////////////////////////////////////////////////////////////////////////
supervised_session_t *zwave_command_class_supervision_find_session_by_unique_id(
  supervision_id_t supervision_id)
{
  if (sessions.count(supervision_id)) {
    return (supervised_session_t *)(&sessions[supervision_id]);
  } else {
    return nullptr;
  }
}

supervision_id_t zwave_command_class_supervision_find_session(
  uint8_t session_id, zwave_node_id_t node_id, zwave_endpoint_id_t endpoint_id)
{
  for (auto it = sessions.begin(); it != sessions.end(); ++it) {
    if (it->second.session.node_id == node_id
        && it->second.session.endpoint_id == endpoint_id
        && it->second.session.session_id == session_id) {
      return it->first;
    }
  }
  return INVALID_SUPERVISION_ID;
}

static supervision_id_t
  zwave_command_class_supervision_create_singlecast_session(
    zwave_node_id_t node_id,
    zwave_endpoint_id_t endpoint_id,
    uint32_t discard_timeout_ms,
    on_zwave_tx_send_data_complete_t callback,
    void *user)
{
  supervised_session_t new_session = {};

  new_session.session.session_id
    = next_session_id & SUPERVISION_GET_PROPERTIES1_SESSION_ID_MASK;
  new_session.session.node_id     = node_id;
  new_session.session.endpoint_id = endpoint_id;
  new_session.session.group_id    = ZWAVE_TX_INVALID_GROUP;
  new_session.tx_info_valid       = false;
  new_session.callback            = callback;
  new_session.user                = user;
  new_session.tx_session_valid    = false;
  new_session.status              = SUPERVISION_REPORT_NO_SUPPORT;
  new_session.expiry_time
    = clock_time() + discard_timeout_ms + SUPERVISION_SEND_DATA_EMERGENCY_TIMER;

  sessions.insert(
    std::pair<supervision_id_t, supervised_session_t>(next_supervision_id,
                                                      new_session));
  supervision_id_t return_value = next_supervision_id;

  // Increment the Session IDs for the next call
  increment_unique_supervision_id();
  next_session_id += 1;
  return return_value;
}

static supervision_id_t
  zwave_command_class_supervision_update_singlecast_follow_up_session(
    zwave_node_id_t node_id,
    zwave_multicast_group_id_t group_id,
    zwave_endpoint_id_t endpoint_id,
    uint32_t discard_timeout_ms,
    on_zwave_tx_send_data_complete_t callback,
    void *user)
{
  for (auto it = sessions.begin(); it != sessions.end(); ++it) {
    if (it->second.session.node_id == node_id
        && it->second.session.endpoint_id == endpoint_id
        && it->second.session.group_id == group_id) {
      it->second.callback    = callback;
      it->second.user        = user;
      it->second.expiry_time = clock_time() + discard_timeout_ms
                               + SUPERVISION_SEND_DATA_EMERGENCY_TIMER;
      return it->first;
    }
  }
  sl_log_debug(LOG_TAG,
               "Could not update Singlecast follow-up Supervision Session for "
               "NodeID %d, Endpoint ID %d, Group ID %d. Ignoring.",
               node_id,
               endpoint_id,
               group_id);
  return 0;
}

static supervision_id_t zwave_command_class_supervision_create_group_session(
  zwave_multicast_group_id_t group_id,
  zwave_endpoint_id_t endpoint_id,
  uint32_t discard_timeout_ms,
  on_zwave_tx_send_data_complete_t callback,
  void *user)
{
  supervision_id_t return_value    = INVALID_SUPERVISION_ID;
  supervised_session_t new_session = {};
  new_session.session.session_id
    = next_session_id & SUPERVISION_GET_PROPERTIES1_SESSION_ID_MASK;
  new_session.session.endpoint_id = endpoint_id;
  new_session.session.group_id    = group_id;
  new_session.callback            = callback;
  new_session.user                = user;
  new_session.tx_info_valid       = false;
  new_session.tx_session_valid    = false;
  new_session.status              = SUPERVISION_REPORT_NO_SUPPORT;
  new_session.expiry_time
    = clock_time() + discard_timeout_ms + SUPERVISION_SEND_DATA_EMERGENCY_TIMER;

  zwave_nodemask_t node_list = {};
  zwave_tx_get_nodes(node_list, group_id);
  for (zwave_node_id_t n = ZW_MIN_NODE_ID; n <= ZW_LR_MAX_NODE_ID; n++) {
    if (ZW_IS_NODE_IN_MASK(n, node_list)) {
      new_session.session.node_id = n;
      sessions.insert(
        std::pair<supervision_id_t, supervised_session_t>(next_supervision_id,
                                                          new_session));
      return_value = next_supervision_id;
      increment_unique_supervision_id();
    }
  }

  // Increment the Session IDs for the next call, all nodes in this group have
  // the same Session ID
  next_session_id += 1;
  return return_value;
}

supervision_id_t zwave_command_class_supervision_create_session(
  const zwave_controller_connection_info_t *connection,
  const zwave_tx_options_t *tx_options,
  on_zwave_tx_send_data_complete_t callback,
  void *user)
{
  // What kind of message do we have here ?
  // Singlecast, Multicast, or singlecast follow-up ?
  if (connection->remote.is_multicast == true) {
    return zwave_command_class_supervision_create_group_session(
      connection->remote.multicast_group,
      connection->remote.endpoint_id,
      tx_options->discard_timeout_ms,
      callback,
      user);
  } else if (tx_options->group_id != ZWAVE_TX_INVALID_GROUP) {
    return zwave_command_class_supervision_update_singlecast_follow_up_session(
      connection->remote.node_id,
      tx_options->group_id,
      connection->remote.endpoint_id,
      tx_options->discard_timeout_ms,
      callback,
      user);
  } else {
    return zwave_command_class_supervision_create_singlecast_session(
      connection->remote.node_id,
      connection->remote.endpoint_id,
      tx_options->discard_timeout_ms,
      callback,
      user);
  }
}

sl_status_t zwave_command_class_supervision_assign_session_tx_id(
  supervision_id_t supervision_id, zwave_tx_session_id_t tx_session_id)
{
  supervised_session_t *session
    = zwave_command_class_supervision_find_session_by_unique_id(supervision_id);
  if (session == nullptr) {
    return SL_STATUS_NOT_FOUND;
  }

  sl_log_debug(LOG_TAG,
               "Assigning Z-Wave Tx Session %p to Supervision Session %d",
               tx_session_id,
               supervision_id);

  session->tx_session_valid = true;
  session->tx_session_id    = tx_session_id;
  return SL_STATUS_OK;
}

sl_status_t
  zwave_command_class_supervision_close_session(supervision_id_t supervision_id)
{
  if (supervision_id == INVALID_SUPERVISION_ID) {
    return SL_STATUS_NOT_FOUND;
  }
  // Find the session
  session_iterator_t it = sessions.find(supervision_id);
  if (it == sessions.end()) {
    return SL_STATUS_NOT_FOUND;
  }
  sessions.erase(it);
  zwave_command_class_supervision_restart_timer();

  return SL_STATUS_OK;
}

sl_status_t zwave_command_class_supervision_close_session_by_tx_session(
  zwave_tx_session_id_t tx_session_id)
{
  // Find the session that has the desired tx_session_id
  bool session_found              = false;
  supervision_id_t supervision_id = INVALID_SUPERVISION_ID;

  for (auto it = sessions.begin(); it != sessions.end(); ++it) {
    if (it->second.tx_session_valid == true
        && it->second.tx_session_id == tx_session_id) {
      session_found  = true;
      supervision_id = it->first;
      break;
    }
  }
  if (session_found == false) {
    sl_log_debug(LOG_TAG,
                 "Could not find Tx Session %p among supervised sessions",
                 tx_session_id);
    return SL_STATUS_NOT_FOUND;
  }

  return zwave_command_class_supervision_close_session(supervision_id);
}

////////////////////////////////////////////////////////////////////////////////
// Contiki Process
////////////////////////////////////////////////////////////////////////////////
PROCESS(zwave_command_class_supervision_process,
        "zwave_command_class_supervision_process");

PROCESS_THREAD(zwave_command_class_supervision_process, ev, data)
{
  PROCESS_BEGIN();

  while (1) {
    if (ev == PROCESS_EVENT_INIT) {
      // Start up the first Session ID with a random number.
      std::random_device rd;
      std::mt19937 gen(rd());

      next_session_id = gen() % UINT8_MAX;
    } else if (ev == PROCESS_EVENT_EXIT) {
      // Clear all the sessions when we exit.
      sessions.clear();

    } else if (ev == PROCESS_EVENT_EXITED) {
      // Do not do anything with this event, just wait to go down.

    } else if (ev == ZWAVE_COMMAND_CLASS_SUPERVISION_START_TIMER) {
      // Start (or restart) the supervision timer.
      zwave_command_class_supervision_restart_timer();

    } else if (ev == PROCESS_EVENT_TIMER) {
      // We timed out waiting for some Supervision Report.

      // At that point, we stop hoping for it and
      // just consider the supervision session a FAIL.
      supervision_process_on_timer_expired_event((etimer *)data);
    }

    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}
