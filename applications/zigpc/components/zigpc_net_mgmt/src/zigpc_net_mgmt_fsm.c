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

#include <stdlib.h>
#include <string.h>

// Unify shared includes
#include <sl_log.h>
#include <sl_status.h>
#include <sys/etimer.h>

// ZigPC includes
#include <zigpc_datastore.h>
#include <zigpc_discovery.h>
#include <zigpc_gateway.h>

#include "zigpc_net_mgmt_notify_int.h"
#include "zigpc_net_mgmt_fsm.h"

static const char LOG_TAG[] = "zigpc_net_mgmt_fsm";

#define ZIGPC_NET_MGMT_FSM_NUM_STATES \
  (ZIGPC_NET_MGMT_FSM_STATE_MAX_VAL - ZIGPC_NET_MGMT_FSM_STATE_MIN_VAL)
#define ZIGPC_NET_MGMT_FSM_NUM_EVENTS \
  (ZIGPC_NET_MGMT_FSM_EVENT_MAX_VAL - ZIGPC_NET_MGMT_FSM_EVENT_MIN_VAL)

#define ZIGPC_NET_MGMT_REMOVE_NODE_PARAMETERS_COUNT 1

typedef sl_status_t (*zigpc_net_mgmt_hdl)(
  const zigpc_net_mgmt_fsm_data_t *const event_data);

struct zigpc_net_mgmt_fsm fsm;

static const char *zigpc_net_mgmt_remove_node_parameters
  [ZIGPC_NET_MGMT_REMOVE_NODE_PARAMETERS_COUNT]
  = {
    "Unid",
};

static sl_status_t zigpc_net_mgmt_hdl_init_complete(
  const zigpc_net_mgmt_fsm_data_t *const event_data)
{
  sl_status_t status = SL_STATUS_OK;

  const zigpc_gateway_on_network_init_t *data
    = &event_data->on_net_init_complete;
  etimer_stop(&fsm.timer);

  if (data == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    zigpc_network_data_t nwk;

    memcpy(nwk.gateway_eui64, data->zigpc_eui64, sizeof(zigbee_eui64_t));
    nwk.panid = data->zigpc_panid;
    memcpy(nwk.ext_panid, data->zigpc_ext_panid, sizeof(zigbee_ext_panid_t));
    nwk.radio_channel = data->zigpc_radio_channel;

    zigpc_datastore_create_network();
    status = zigpc_datastore_write_network(&nwk);
    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG, "Unable to store network data: 0x%X", status);
    }
  }

  // Create a device entity for the gateway radio
  if (status == SL_STATUS_OK) {
    // Remove any persisted stale device data, if exists
    zigpc_datastore_remove_device(data->zigpc_eui64);
    status = zigpc_datastore_create_device(data->zigpc_eui64);
    if (status == SL_STATUS_OK) {
      // Since there is no discovery needed for knowing endpoints supported
      // on the gateway, set the total and discovered fields to 1
      const uint8_t zigpc_endpoint_count = 1;

      zigpc_device_data_t dev = {
        .network_status            = ZIGBEE_NODE_STATUS_INCLUDED,
        .max_cmd_delay             = 0,
        .endpoint_total_count      = zigpc_endpoint_count,
        .endpoint_discovered_count = zigpc_endpoint_count,
      };
      status = zigpc_datastore_write_device(data->zigpc_eui64, &dev);
    }
    if (status == SL_STATUS_OK) {
      status = zigpc_datastore_create_endpoint(data->zigpc_eui64,
                                               data->zigpc_endpoint_id);
    }
  }

  if (status == SL_STATUS_OK) {
    zigpc_datastore_log_network(LOG_TAG, SL_LOG_INFO, "Network info");

    // TODO: Remove the following logs since log_network already does this
    sl_log_debug(LOG_TAG, "PC PANID: 0x%04X", data->zigpc_panid);
    sl_log_debug(LOG_TAG, "PC Channel: 0x%02X", data->zigpc_radio_channel);

    zigpc_net_mgmt_notify_network_init(data->zigpc_eui64);

    fsm.state = ZIGPC_NET_MGMT_FSM_STATE_IDLE;
  }

  return status;
}

static sl_status_t zigpc_net_mgmt_hdl_idle_add_request(
  const zigpc_net_mgmt_fsm_data_t *const event_data)
{
  const zigpc_net_mgmt_fsm_node_add_t *data = &event_data->node_add_request;

  if (data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  zigpc_gateway_add_node_install_code(data->eui64,
                                      data->install_code,
                                      data->install_code_length);

  /* No state change as this is a command sent to zigpc_gateway */

  return SL_STATUS_OK;
}

static sl_status_t zigpc_net_mgmt_hdl_idle_add_start(
  const zigpc_net_mgmt_fsm_data_t *const event_data)
{
  sl_status_t status = SL_STATUS_OK;

  if (event_data == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    const zigpc_gateway_on_node_add_t *data = &event_data->on_node_add_start;
    etimer_stop(&fsm.timer);
    etimer_set(&fsm.timer, ZIGPC_NET_MGMT_ADD_TIMEOUT);

    memcpy(fsm.joining_eui64, data->eui64, sizeof(zigbee_eui64_t));
    fsm.state = ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD;
  }

  return status;
}

static sl_status_t zigpc_net_mgmt_hdl_node_add_complete(
  const zigpc_net_mgmt_fsm_data_t *const event_data)
{
  sl_status_t status                      = SL_STATUS_OK;
  const zigpc_gateway_on_node_add_t *data = &event_data->on_node_add_complete;

  if (data == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else if (memcmp(fsm.joining_eui64, data->eui64, sizeof(zigbee_eui64_t))) {
    sl_log_warning(LOG_TAG, "Received unexpected node joined event");
    status = SL_STATUS_IN_PROGRESS;
  }

  if (status == SL_STATUS_OK) {
    etimer_stop(&fsm.timer);

    // Remove any persisted stale device data, if exists
    zigpc_datastore_remove_device(data->eui64);
    status = zigpc_datastore_create_device(data->eui64);
  }

  if (status == SL_STATUS_OK) {
    zigpc_device_data_t dev = {
      .network_status            = ZIGBEE_NODE_STATUS_NODEID_ASSIGNED,
      .max_cmd_delay             = 1,
      .endpoint_total_count      = 0,
      .endpoint_discovered_count = 0,
    };
    status = zigpc_datastore_write_device(data->eui64, &dev);
  }

  if (status == SL_STATUS_OK) {
    zigpc_datastore_log_device(LOG_TAG,
                               SL_LOG_DEBUG,
                               "Device Added (Pre-Interview)",
                               data->eui64);

    zigpc_net_mgmt_notify_node_added(data->eui64);

    if (fsm.state_change_in_progress == true) {
      fsm.state_change_in_progress = false;

      status = zigpc_gateway_network_permit_joins(false);
      if (status != SL_STATUS_OK) {
        sl_log_warning(
          LOG_TAG,
          "Failed to request deny joins after node add complete: 0x%X",
          status);
      }
    }

    fsm.state = ZIGPC_NET_MGMT_FSM_STATE_IDLE;
  }

  return status;
}

static sl_status_t zigpc_net_mgmt_hdl_node_add_timeout(
  const zigpc_net_mgmt_fsm_data_t *const event_data)
{
  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_IDLE;

  sl_status_t status = zigpc_gateway_network_permit_joins(false);
  if (status != SL_STATUS_OK) {
    sl_log_warning(LOG_TAG,
                   "Failed to request deny joins after node add timeout: 0x%X",
                   status);
  }
  /* unused */
  (void)(event_data);

  return SL_STATUS_OK;
}

static sl_status_t zigpc_net_mgmt_hdl_node_interview_status(
  const zigpc_net_mgmt_fsm_data_t *const event_data)
{
  if (event_data == NULL) {
    return SL_STATUS_NULL_POINTER;
  }

  const zigpc_net_mgmt_fsm_interview_status_t *interview_status
    = &event_data->on_node_interview_status;

  etimer_stop(&fsm.timer);

  if (interview_status->discovery_status == DISCOVERY_START) {
    fsm.state = ZIGPC_NET_MGMT_FSM_STATE_NODE_INTERVIEW;

  } else if (interview_status->discovery_status == DISCOVERY_SUCCESS) {
    fsm.state = ZIGPC_NET_MGMT_FSM_STATE_IDLE;
    zigpc_net_mgmt_notify_node_interview_status(interview_status->eui64, true);

  } else if (interview_status->discovery_status == DEVICE_DISCOVERY_FAIL) {
    fsm.state = ZIGPC_NET_MGMT_FSM_STATE_IDLE;
    zigpc_net_mgmt_notify_node_interview_status(interview_status->eui64, false);

  } else if (interview_status->discovery_status == ENDPOINT_DISCOVERY_FAIL) {
    fsm.state = ZIGPC_NET_MGMT_FSM_STATE_IDLE;
    zigpc_net_mgmt_notify_node_interview_status(interview_status->eui64, false);

  } else {
    sl_log_warning(LOG_TAG,
                   "Unknown discovery status received %d",
                   interview_status->discovery_status);
  }

  return SL_STATUS_OK;
}

static sl_status_t zigpc_net_mgmt_hdl_idle_remove_request(
  const zigpc_net_mgmt_fsm_data_t *const event_data)
{
  sl_status_t status = SL_STATUS_OK;
  const zigpc_net_mgmt_fsm_node_remove_t *data
    = &event_data->node_remove_request;

  if (data == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    zigpc_device_data_t dev;

    status = zigpc_datastore_read_device(data->eui64, &dev);
    if (status == SL_STATUS_OK) {
      dev.network_status = ZIGBEE_NODE_STATUS_UNAVAILABLE;
      status             = zigpc_datastore_write_device(data->eui64, &dev);
    }
  }

  if (status == SL_STATUS_OK) {
    etimer_stop(&fsm.timer);
    etimer_set(&fsm.timer, ZIGPC_NET_MGMT_REMOVE_TIMEOUT);

    zigpc_gateway_remove_node(data->eui64);

    memcpy(fsm.eui64_to_remove, data->eui64, sizeof(zigbee_eui64_t));
    fsm.state = ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE;
  }

  return status;
}

static sl_status_t zigpc_net_mgmt_hdl_node_remove_complete(
  const zigpc_net_mgmt_fsm_data_t *const event_data)
{
  sl_status_t status = SL_STATUS_OK;
  const zigpc_gateway_on_node_removed_t *data
    = &event_data->on_node_remove_complete;

  if (data == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else if (memcmp(fsm.eui64_to_remove, data->eui64, sizeof(zigbee_eui64_t))) {
    sl_log_warning(LOG_TAG, "Received unexpected node removed event");
    status = SL_STATUS_IN_PROGRESS;
  } else {
    zigpc_datastore_log_device(LOG_TAG,
                               SL_LOG_INFO,
                               "Removed Device",
                               data->eui64);

    status = zigpc_datastore_remove_device(data->eui64);
  }

  if (status == SL_STATUS_OK) {
    etimer_stop(&fsm.timer);

    zigpc_net_mgmt_notify_node_removed(data->eui64);

    fsm.state = ZIGPC_NET_MGMT_FSM_STATE_IDLE;
  }

  return status;
}

static sl_status_t zigpc_net_mgmt_hdl_node_remove_timeout(
  const zigpc_net_mgmt_fsm_data_t *const event_data)
{
  // TODO: Add action(s) to perform
  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_IDLE;

  /* unused */
  (void)(event_data);

  return SL_STATUS_OK;
}

static sl_status_t zigpc_net_mgmt_hdl_idle_state_change_request(
  const zigpc_net_mgmt_fsm_data_t *const event_data)
{
  sl_status_t status                      = SL_STATUS_OK;
  unsigned int requested_parameters_count = 0U;
  const zigpc_net_mgmt_state_change_request_t *data
    = &event_data->state_change_request;

  if (data == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    fsm.state_change_in_progress = true;
    switch (data->requested_state) {
      case ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE:
        fsm.state_change_request.requested_state = data->requested_state;
        while (requested_parameters_count
               < ZIGPC_NET_MGMT_REMOVE_NODE_PARAMETERS_COUNT) {
          zigpc_net_mgmt_notify_add_requested_parameter(
            zigpc_net_mgmt_remove_node_parameters[requested_parameters_count]);
          requested_parameters_count++;
        }
        fsm.state = ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE;
        break;
      case ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD:
        status = zigpc_gateway_network_permit_joins(true);
        if (status != SL_STATUS_OK) {
          sl_log_warning(LOG_TAG,
                         "Failed to request permit joins on state change: 0x%X",
                         status);
        } else {
          etimer_stop(&fsm.timer);
          etimer_set(&fsm.timer, ZIGPC_NET_MGMT_ADD_TIMEOUT);
          fsm.state = ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD;
        }
        break;
      case ZIGPC_NET_MGMT_FSM_STATE_IDLE:
      default:
        status = SL_STATUS_INVALID_TYPE;
    }
  }

  return status;
}

static sl_status_t zigpc_net_mgmt_hdl_add_state_change_request(
  const zigpc_net_mgmt_fsm_data_t *const event_data)
{
  sl_status_t status = SL_STATUS_OK;
  const zigpc_net_mgmt_state_change_request_t *data
    = &event_data->state_change_request;

  if (data == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    switch (data->requested_state) {
      case ZIGPC_NET_MGMT_FSM_STATE_IDLE:
        status = zigpc_gateway_network_permit_joins(false);
        if (status != SL_STATUS_OK) {
          sl_log_warning(LOG_TAG,
                         "Failed to request deny joins on state change: 0x%X",
                         status);
        }
        fsm.state = ZIGPC_NET_MGMT_FSM_STATE_IDLE;
        break;
      default:
        status = SL_STATUS_NOT_SUPPORTED;
    }
  }

  return status;
}

static sl_status_t zigpc_net_mgmt_hdl_remove_state_change_request(
  const zigpc_net_mgmt_fsm_data_t *const event_data)
{
  sl_status_t status = SL_STATUS_OK;
  const zigpc_net_mgmt_state_change_request_t *data
    = &event_data->state_change_request;

  if (data == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    switch (data->requested_state) {
      case ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE:
        if (data->param_eui64_filled == true) {
          etimer_stop(&fsm.timer);
          etimer_set(&fsm.timer, ZIGPC_NET_MGMT_REMOVE_TIMEOUT);
          zigpc_gateway_remove_node(data->param_eui64);

          memcpy(fsm.eui64_to_remove,
                 data->param_eui64,
                 sizeof(zigbee_eui64_t));
        } else {
          zigpc_net_mgmt_notify_add_requested_parameter(
            zigpc_net_mgmt_remove_node_parameters[0]);
        }
        break;
      case ZIGPC_NET_MGMT_FSM_STATE_IDLE:
        fsm.state = ZIGPC_NET_MGMT_FSM_STATE_IDLE;
        break;
      default:
        status = SL_STATUS_NOT_SUPPORTED;
    }
  }

  return status;
}

/**
 * Network Management FSM table
 */
static const zigpc_net_mgmt_hdl zigpc_net_mgmt_fsm_map[ZIGPC_NET_MGMT_FSM_NUM_STATES][ZIGPC_NET_MGMT_FSM_NUM_EVENTS] = {
  [ZIGPC_NET_MGMT_FSM_STATE_INIT] = {
    [ZIGPC_NET_MGMT_FSM_EVENT_INIT_COMPLETE] = zigpc_net_mgmt_hdl_init_complete,
  },
  [ZIGPC_NET_MGMT_FSM_STATE_IDLE] = {
    [ZIGPC_NET_MGMT_FSM_EVENT_NODE_ADD_REQUEST] = zigpc_net_mgmt_hdl_idle_add_request,
    [ZIGPC_NET_MGMT_FSM_EVENT_NODE_INTERVIEW_STATUS] = zigpc_net_mgmt_hdl_node_interview_status,
    [ZIGPC_NET_MGMT_FSM_EVENT_NODE_ADD_START] = zigpc_net_mgmt_hdl_idle_add_start,
    [ZIGPC_NET_MGMT_FSM_EVENT_NODE_REMOVE_REQUEST] = zigpc_net_mgmt_hdl_idle_remove_request,
    [ZIGPC_NET_MGMT_FSM_EVENT_STATE_CHANGE_REQUEST] = zigpc_net_mgmt_hdl_idle_state_change_request,
  },
  [ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD] = {
    [ZIGPC_NET_MGMT_FSM_EVENT_NODE_ADD_REQUEST] = zigpc_net_mgmt_hdl_idle_add_request,
    [ZIGPC_NET_MGMT_FSM_EVENT_NODE_ADD_START] = zigpc_net_mgmt_hdl_idle_add_start,
    [ZIGPC_NET_MGMT_FSM_EVENT_NODE_ADD_COMPLETE] = zigpc_net_mgmt_hdl_node_add_complete,
    [ZIGPC_NET_MGMT_FSM_EVENT_TIMEOUT] = zigpc_net_mgmt_hdl_node_add_timeout,
    [ZIGPC_NET_MGMT_FSM_EVENT_STATE_CHANGE_REQUEST] = zigpc_net_mgmt_hdl_add_state_change_request,
  },
  [ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE] = {
    [ZIGPC_NET_MGMT_FSM_EVENT_NODE_REMOVE_COMPLETE] = zigpc_net_mgmt_hdl_node_remove_complete,
    [ZIGPC_NET_MGMT_FSM_EVENT_TIMEOUT] = zigpc_net_mgmt_hdl_node_remove_timeout,
    [ZIGPC_NET_MGMT_FSM_EVENT_STATE_CHANGE_REQUEST] = zigpc_net_mgmt_hdl_remove_state_change_request,
  },
  [ZIGPC_NET_MGMT_FSM_STATE_NODE_INTERVIEW] = {
    [ZIGPC_NET_MGMT_FSM_EVENT_NODE_INTERVIEW_STATUS] = zigpc_net_mgmt_hdl_node_interview_status,
  },
};

/**
 * @brief Stored mapping of current state to supported next states
 *
 */
static enum zigpc_net_mgmt_fsm_state zigpc_net_mgmt_fsm_next_states
  [ZIGPC_NET_MGMT_FSM_STATE_MAX_VAL][ZIGPC_NET_MGMT_FSM_STATE_MAX_VAL] = {
  [ZIGPC_NET_MGMT_FSM_STATE_INIT] = {0},
  [ZIGPC_NET_MGMT_FSM_STATE_IDLE] = {
    ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD,
    ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE,
    0,
  },
  [ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD] = {
    ZIGPC_NET_MGMT_FSM_STATE_IDLE,
    0,
  },
  [ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE] = {
    ZIGPC_NET_MGMT_FSM_STATE_IDLE,
    0,
  },
  [ZIGPC_NET_MGMT_FSM_STATE_NODE_INTERVIEW] = {
    ZIGPC_NET_MGMT_FSM_STATE_IDLE,
    0,
  },
};

void zigpc_net_mgmt_fsm_reset(void)
{
  etimer_stop(&fsm.timer);
  fsm.state = ZIGPC_NET_MGMT_FSM_STATE_INIT;
}

enum zigpc_net_mgmt_fsm_state zigpc_net_mgmt_fsm_get_state(void)
{
  return fsm.state;
}

struct etimer *zigpc_net_mgmt_fsm_get_timer(void)
{
  return &fsm.timer;
}

bool zigpc_netmgmt_fsm_state_change_in_progress(void)
{
  return fsm.state_change_in_progress;
}

void zigpc_net_mgmt_fsm_clear_state_change_request(void)
{
  fsm.state_change_request.requested_state = ZIGPC_NET_MGMT_FSM_STATE_MIN_VAL;
  fsm.state_change_request.param_eui64_filled = false;
  fsm.state_change_in_progress                = false;
}

sl_status_t zigpc_net_mgmt_fsm_post_event(
  enum zigpc_net_mgmt_fsm_event event,
  const zigpc_net_mgmt_fsm_data_t *const event_data)
{
  sl_status_t status = SL_STATUS_OK;
  zigpc_net_mgmt_hdl event_handler;
  enum zigpc_net_mgmt_fsm_state prev_state = fsm.state;

  if ((event <= ZIGPC_NET_MGMT_FSM_EVENT_MIN_VAL)
      || (event >= ZIGPC_NET_MGMT_FSM_EVENT_MAX_VAL)) {
    sl_log_error(LOG_TAG, "Invalid event(%d) received", event);
    status = SL_STATUS_INVALID_PARAMETER;
  }

  if (status == SL_STATUS_OK) {
    event_handler = zigpc_net_mgmt_fsm_map[prev_state][event];
    if (!event_handler) {
      sl_log_warning(LOG_TAG,
                     "State(%s): Invalid Event(%s) received",
                     zigpc_net_mgmt_fsm_state_name(prev_state),
                     zigpc_net_mgmt_fsm_event_name(event));
      status = SL_STATUS_INVALID_TYPE;
    }
  }

  if (status == SL_STATUS_OK) {
    zigpc_net_mgmt_notify_clear_requested_parameter_list();
    sl_log_info(LOG_TAG,
                "Old state: %s",
                zigpc_net_mgmt_fsm_state_name(prev_state));
    sl_log_info(LOG_TAG,
                "Event received: %s",
                zigpc_net_mgmt_fsm_event_name(event));
    status = (event_handler)(event_data);
    sl_log_info(LOG_TAG,
                "New state: %s",
                zigpc_net_mgmt_fsm_state_name(fsm.state));
    if (status == SL_STATUS_NOT_SUPPORTED) {
      sl_log_warning(LOG_TAG, "State transition not supported");
    } else if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG, "State transition failed: 0x%X", status);
    }
  }

  if (status == SL_STATUS_OK && prev_state != fsm.state) {
    (void)zigpc_net_mgmt_notify_state_update(
      fsm.state,
      zigpc_net_mgmt_fsm_next_states[fsm.state]);
  }

  if (fsm.state == ZIGPC_NET_MGMT_FSM_STATE_IDLE) {
    zigpc_net_mgmt_fsm_clear_state_change_request();
  }

  return status;
}
