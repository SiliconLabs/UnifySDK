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

/* Contiki includes */
#include <string.h>

#include "sl_log.h"
#include "sl_status.h"

#include "zigpc_net_mgmt.h"
#include "zigpc_net_mgmt_notify.h"

#include "zigpc_net_mgmt_notify_int.h"

static const char LOG_TAG[] = "zigpc_net_mgmt_notify";

static struct zigpc_net_mgmt_on_network_state_update
  zigpc_net_mgmt_notify_state_update_data;

/**
 * @brief Network Management observable instance
 *
 */
static struct zigpc_observable zigpc_net_mgmt_observable;

sl_status_t
  zigpc_net_mgmt_register_observer(enum zigpc_net_mgmt_notify_event event,
                                   zigpc_observer_callback_t callback)
{
  return zigpc_observable_register(&zigpc_net_mgmt_observable, event, callback);
}
sl_status_t
  zigpc_net_mgmt_unregister_observer(enum zigpc_net_mgmt_notify_event event,
                                     zigpc_observer_callback_t callback)
{
  return zigpc_observable_unregister(&zigpc_net_mgmt_observable,
                                     event,
                                     callback);
}

sl_status_t zigpc_net_mgmt_reset_observable(void)
{
  sl_status_t status = SL_STATUS_OK;

  static const zigpc_observer_event_t observer_events[ZIGPC_NET_MGMT_NOTIFY_MAX]
    = {
      ZIGPC_NET_MGMT_NOTIFY_NETWORK_INIT,
      ZIGPC_NET_MGMT_NOTIFY_STATE_UPDATE,
      ZIGPC_NET_MGMT_NOTIFY_NODE_ADDED,
      ZIGPC_NET_MGMT_NOTIFY_NODE_INTERVIEW_STATUS,
      ZIGPC_NET_MGMT_NOTIFY_NODE_REMOVED,
    };

  zigpc_observable_clear(&zigpc_net_mgmt_observable);

  status = zigpc_observable_init(&zigpc_net_mgmt_observable,
                                 observer_events,
                                 ZIGPC_NET_MGMT_NOTIFY_MAX);
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Observer init failed: 0x%X", status);
  }

  return status;
}

void zigpc_net_mgmt_notify_network_init(const zigbee_eui64_t pc_eui64)
{

  static struct zigpc_net_mgmt_on_network_init notify_data = {0};

  memcpy(notify_data.pc_eui64, pc_eui64, sizeof(zigbee_eui64_t));

  sl_status_t status
    = zigpc_observable_notify(&zigpc_net_mgmt_observable,
                              ZIGPC_NET_MGMT_NOTIFY_NETWORK_INIT,
                              (void *)&notify_data);
  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG, "NOTIFY_NET_INIT failed: 0x%X", status);
  }
}

void zigpc_net_mgmt_notify_clear_requested_parameter_list(void)
{
  for (uint8_t i = 0; i < ZIGPC_NET_MGMT_REQUESTED_STATE_PARAMS_MAX; i++) {
    zigpc_net_mgmt_notify_state_update_data.requested_state_parameter_list[i]
      = NULL;
  }
  zigpc_net_mgmt_notify_state_update_data.requested_state_parameter_count = 0U;
}

sl_status_t
  zigpc_net_mgmt_notify_add_requested_parameter(const char *parameter_str)
{
  sl_status_t status = SL_STATUS_OK;

  if (parameter_str == NULL) {
    status = SL_STATUS_NULL_POINTER;
    sl_log_error(LOG_TAG, "Requested parameter invalid: 0x%X", status);
  }

  if ((status == SL_STATUS_OK) && (strlen(parameter_str) == 0)) {
    status = SL_STATUS_EMPTY;
    sl_log_error(LOG_TAG, "Requested parameter empty: 0x%X", status);
  }

  if ((status == SL_STATUS_OK)
      && (zigpc_net_mgmt_notify_state_update_data
            .requested_state_parameter_count
          >= ZIGPC_NET_MGMT_REQUESTED_STATE_PARAMS_MAX)) {
    status = SL_STATUS_FULL;
    sl_log_error(LOG_TAG, "Requested parameter list full: 0x%X", status);
  }

  if (status == SL_STATUS_OK) {
    zigpc_net_mgmt_notify_state_update_data.requested_state_parameter_list
      [zigpc_net_mgmt_notify_state_update_data.requested_state_parameter_count]
      = parameter_str;
    zigpc_net_mgmt_notify_state_update_data.requested_state_parameter_count++;
  }

  return status;
}

sl_status_t zigpc_net_mgmt_notify_state_update(
  const enum zigpc_net_mgmt_fsm_state new_state,
  const enum zigpc_net_mgmt_fsm_state *const next_supported_states)
{
  sl_status_t status = SL_STATUS_OK;
  uint8_t next_states_count;

  if (next_supported_states == NULL) {
    status = SL_STATUS_NULL_POINTER;
  }

  if (status == SL_STATUS_OK) {
    zigpc_net_mgmt_notify_state_update_data.new_state = new_state;

    /* Get count of next states supported */
    for (next_states_count = 0;
         (next_states_count < ZIGPC_NET_MGMT_FSM_STATE_MAX_VAL)
         && (next_supported_states[next_states_count] != 0);
         next_states_count++)
      ;

    zigpc_net_mgmt_notify_state_update_data.next_supported_states_count
      = next_states_count;
    memcpy(zigpc_net_mgmt_notify_state_update_data.next_supported_states_list,
           next_supported_states,
           sizeof(enum zigpc_net_mgmt_fsm_state) * next_states_count);

    status = zigpc_observable_notify(
      &zigpc_net_mgmt_observable,
      ZIGPC_NET_MGMT_NOTIFY_STATE_UPDATE,
      (void *)&zigpc_net_mgmt_notify_state_update_data);
    if (status != SL_STATUS_OK) {
      sl_log_warning(LOG_TAG, "NOTIFY_STATE_UPDATE failed: 0x%X", status);
    }
  }
  return status;
}

void zigpc_net_mgmt_notify_node_added(const zigbee_eui64_t eui64)
{
  static struct zigpc_net_mgmt_on_node_added notify_data = {0};

  memcpy(notify_data.eui64, eui64, sizeof(zigbee_eui64_t));

  sl_status_t status = zigpc_observable_notify(&zigpc_net_mgmt_observable,
                                               ZIGPC_NET_MGMT_NOTIFY_NODE_ADDED,
                                               (void *)&notify_data);
  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG, "NOTIFY_NODE_ADDED failed: 0x%X", status);
  }
}

void zigpc_net_mgmt_notify_node_interview_status(const zigbee_eui64_t eui64,
                                                 bool success)
{
  static zigpc_net_mgmt_on_node_interview_status_t notify_data = {0};

  notify_data.success = success;
  memcpy(notify_data.eui64, eui64, sizeof(zigbee_eui64_t));

  sl_status_t status
    = zigpc_observable_notify(&zigpc_net_mgmt_observable,
                              ZIGPC_NET_MGMT_NOTIFY_NODE_INTERVIEW_STATUS,
                              (void *)&notify_data);
  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG,
                   "NOTIFY_NODE_INTERVIEW_STATUS failed: 0x%X",
                   status);
  }
}

void zigpc_net_mgmt_notify_node_removed(const zigbee_eui64_t eui64)
{
  static zigpc_net_mgmt_on_node_removed_t notify_data = {0};

  memcpy(notify_data.eui64, eui64, sizeof(zigbee_eui64_t));

  sl_status_t status
    = zigpc_observable_notify(&zigpc_net_mgmt_observable,
                              ZIGPC_NET_MGMT_NOTIFY_NODE_REMOVED,
                              (void *)&notify_data);
  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG, "NOTIFY_NODE_REMOVED failed: 0x%X", status);
  }
}
