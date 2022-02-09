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

#include <process.h>
#include <sl_log.h>
#include <sl_status.h>

#include <zigpc_discovery.h>
#include "zigpc_net_mgmt_fsm.h"
#include "zigpc_net_mgmt_gateway.h"
#include "zigpc_net_mgmt_process.h"
#include "zigpc_net_mgmt_process_send.h"

#include "zigpc_common_observable.h"

#include "zigpc_gateway_notify.h"

static const char LOG_TAG[] = "zigpc_net_mgmt_gateway";

void zigpc_net_mgmt_callback_on_network_initialized(void *event_data)
{
  zigpc_net_mgmt_process_data_fsm_t process_data_fsm;
  zigpc_gateway_on_network_init_t *init_data
    = &process_data_fsm.fsm_data.on_net_init_complete;

  const zigpc_gateway_on_network_init_t *net_init
    = (zigpc_gateway_on_network_init_t *)event_data;

  if (zigpc_net_mgmt_fsm_get_state() == ZIGPC_NET_MGMT_FSM_STATE_INIT) {
    memcpy(init_data->zigpc_eui64,
           net_init->zigpc_eui64,
           sizeof(zigbee_eui64_t));
    init_data->zigpc_endpoint_id = net_init->zigpc_endpoint_id;
    memcpy(init_data->zigpc_ext_panid,
           net_init->zigpc_ext_panid,
           sizeof(zigbee_eui64_t));
    init_data->zigpc_panid         = net_init->zigpc_panid;
    init_data->zigpc_radio_channel = net_init->zigpc_radio_channel;

    process_data_fsm.fsm_event = ZIGPC_NET_MGMT_FSM_EVENT_INIT_COMPLETE;

    zigpc_net_mgmt_process_send_event(
      ZIGPC_NET_MGMT_EVENT_FSM,
      (void *)&process_data_fsm,
      sizeof(zigpc_net_mgmt_process_data_fsm_t));
  }
}
void zigpc_net_mgmt_callback_on_node_add_start(void *event_data)
{
  const zigpc_gateway_on_node_add_t *node_add
    = (zigpc_gateway_on_node_add_t *)event_data;

  if (node_add != NULL) {
    zigpc_net_mgmt_process_data_fsm_t process_data_fsm;
    zigpc_gateway_on_node_add_t *node_add_start
      = &process_data_fsm.fsm_data.on_node_add_start;

    memcpy(node_add_start->eui64, node_add->eui64, sizeof(zigbee_eui64_t));

    process_data_fsm.fsm_event = ZIGPC_NET_MGMT_FSM_EVENT_NODE_ADD_START;

    zigpc_net_mgmt_process_send_event(
      ZIGPC_NET_MGMT_EVENT_FSM,
      (void *)&process_data_fsm,
      sizeof(zigpc_net_mgmt_process_data_fsm_t));
  }
}

void zigpc_net_mgmt_callback_on_node_add_complete(void *event_data)
{
  zigpc_net_mgmt_process_data_fsm_t process_data_fsm;
  struct zigpc_gateway_on_node_add *node_add_complete
    = &process_data_fsm.fsm_data.on_node_add_complete;

  const struct zigpc_gateway_on_node_add *node_add
    = (struct zigpc_gateway_on_node_add *)event_data;

  if (zigpc_net_mgmt_fsm_get_state() == ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD) {
    memcpy(node_add_complete->eui64, node_add->eui64, sizeof(zigbee_eui64_t));

    process_data_fsm.fsm_event = ZIGPC_NET_MGMT_FSM_EVENT_NODE_ADD_COMPLETE;

    zigpc_net_mgmt_process_send_event(
      ZIGPC_NET_MGMT_EVENT_FSM,
      (void *)&process_data_fsm,
      sizeof(zigpc_net_mgmt_process_data_fsm_t));
  }
}

void zigpc_net_mgmt_on_discovery_status(
  zigbee_eui64_uint_t eui64, zigpc_discovery_status_t discovery_status)
{
  zigpc_net_mgmt_process_data_fsm_t process_data_fsm = {};

  zigpc_net_mgmt_fsm_interview_status_t *on_node_interview_status
    = &process_data_fsm.fsm_data.on_node_interview_status;

  zigbee_uint_to_eui64(eui64, on_node_interview_status->eui64);
  on_node_interview_status->discovery_status = discovery_status;

  process_data_fsm.fsm_event = ZIGPC_NET_MGMT_FSM_EVENT_NODE_INTERVIEW_STATUS;

  zigpc_net_mgmt_process_send_event(ZIGPC_NET_MGMT_EVENT_FSM,
                                    (void *)&process_data_fsm,
                                    sizeof(zigpc_net_mgmt_process_data_fsm_t));
}

void zigpc_net_mgmt_callback_on_node_removed(void *event_data)
{
  zigpc_net_mgmt_process_data_fsm_t process_data_fsm;
  zigpc_gateway_on_node_removed_t *node_removed_data
    = &process_data_fsm.fsm_data.on_node_remove_complete;

  const zigpc_gateway_on_node_removed_t *node_removed
    = (zigpc_gateway_on_node_removed_t *)event_data;

  if (zigpc_net_mgmt_fsm_get_state() == ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE) {
    memcpy(node_removed_data->eui64,
           node_removed->eui64,
           sizeof(zigbee_eui64_t));

    process_data_fsm.fsm_event = ZIGPC_NET_MGMT_FSM_EVENT_NODE_REMOVE_COMPLETE;

    zigpc_net_mgmt_process_send_event(
      ZIGPC_NET_MGMT_EVENT_FSM,
      (void *)&process_data_fsm,
      sizeof(zigpc_net_mgmt_process_data_fsm_t));
  }
}

sl_status_t zigpc_net_mgmt_gateway_init_observer(void)
{
  sl_status_t result;

  result = zigpc_gateway_register_observer(
    ZIGPC_GATEWAY_NOTIFY_NETWORK_INIT,
    zigpc_net_mgmt_callback_on_network_initialized);

  if (result == SL_STATUS_OK) {
    result = zigpc_gateway_register_observer(
      ZIGPC_GATEWAY_NOTIFY_NODE_ADD_START,
      zigpc_net_mgmt_callback_on_node_add_start);
  }

  if (result == SL_STATUS_OK) {
    result = zigpc_gateway_register_observer(
      ZIGPC_GATEWAY_NOTIFY_NODE_ADD_COMPLETE,
      zigpc_net_mgmt_callback_on_node_add_complete);
  }

  if (result == SL_STATUS_OK) {
    result = zigpc_discovery_add_listener(zigpc_net_mgmt_on_discovery_status);
  }

  if (result == SL_STATUS_OK) {
    result = zigpc_gateway_register_observer(
      ZIGPC_GATEWAY_NOTIFY_NODE_REMOVED,
      zigpc_net_mgmt_callback_on_node_removed);
  }

  if (result != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Gateway observer registrations failed: 0x%X",
                 result);
  }

  return result;
}
