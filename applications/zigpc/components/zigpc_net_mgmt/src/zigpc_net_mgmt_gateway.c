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
  zigpc_net_mgmt_fsm_init_complete_t *init_data
    = &process_data_fsm.fsm_data.on_net_init_complete;

  const struct zigpc_gateway_on_network_init *net_init
    = (struct zigpc_gateway_on_network_init *)event_data;

  if (zigpc_net_mgmt_fsm_get_state() == ZIGPC_NET_MGMT_FSM_STATE_INIT) {
    memcpy(init_data->zigpc_eui64,
           net_init->zigpc_eui64,
           sizeof(zigbee_eui64_t));
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
  zigpc_net_mgmt_process_data_fsm_t process_data_fsm;
  struct zigpc_gateway_on_node_add *node_add_start
    = &process_data_fsm.fsm_data.on_node_add_start;

  const struct zigpc_gateway_on_node_add *node_add
    = (struct zigpc_gateway_on_node_add *)event_data;

  if (zigpc_net_mgmt_fsm_get_state() == ZIGPC_NET_MGMT_FSM_STATE_IDLE) {
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

void zigpc_net_mgmt_callback_on_node_discovered(void *event_data)
{
  zigpc_net_mgmt_process_data_fsm_t process_data_fsm;
  zigpc_gateway_on_node_discovered_t *node_discovered_data
    = &process_data_fsm.fsm_data.on_node_discovered;

  const zigpc_gateway_on_node_discovered_t *ep_disc
    = (zigpc_gateway_on_node_discovered_t *)event_data;

  memcpy(node_discovered_data->eui64, ep_disc->eui64, sizeof(zigbee_eui64_t));
  node_discovered_data->endpoint_count = ep_disc->endpoint_count;

  process_data_fsm.fsm_event = ZIGPC_NET_MGMT_FSM_EVENT_NODE_INTERVIEWED;

  zigpc_net_mgmt_process_send_event(ZIGPC_NET_MGMT_EVENT_FSM,
                                    (void *)&process_data_fsm,
                                    sizeof(zigpc_net_mgmt_process_data_fsm_t));
}

void zigpc_net_mgmt_callback_on_node_endpoint_discovered(void *event_data)
{
  zigpc_net_mgmt_process_data_fsm_t process_data_fsm;
  zigpc_gateway_on_node_endpoint_discovered_t *ep_discovered_data
    = &process_data_fsm.fsm_data.on_node_endpoint_discovered;

  const zigpc_gateway_on_node_endpoint_discovered_t *ep_disc
    = (zigpc_gateway_on_node_endpoint_discovered_t *)event_data;

  memcpy(ep_discovered_data->eui64, ep_disc->eui64, sizeof(zigbee_eui64_t));
  ep_discovered_data->endpoint.endpoint_id   = ep_disc->endpoint.endpoint_id;
  ep_discovered_data->endpoint.cluster_count = ep_disc->endpoint.cluster_count;

  if (ep_discovered_data->endpoint.cluster_count > 0) {
    memcpy(ep_discovered_data->endpoint.cluster_list,
           ep_disc->endpoint.cluster_list,
           sizeof(zcl_cluster_type_t)
             * ep_discovered_data->endpoint.cluster_count);
  }

  process_data_fsm.fsm_event
    = ZIGPC_NET_MGMT_FSM_EVENT_NODE_ENDPOINT_INTERVIEWED;

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
    result = zigpc_gateway_register_observer(
      ZIGPC_GATEWAY_NOTIFY_NODE_DISCOVERED,
      zigpc_net_mgmt_callback_on_node_discovered);
  }

  if (result == SL_STATUS_OK) {
    result = zigpc_gateway_register_observer(
      ZIGPC_GATEWAY_NOTIFY_NODE_ENDPOINT_DISCOVERED,
      zigpc_net_mgmt_callback_on_node_endpoint_discovered);
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
