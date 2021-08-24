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

#include <string.h>
#include <malloc.h>

/* Contiki includes */
#include "sl_log.h"
#include "sl_status.h"

#include "zigpc_net_mgmt_notify.h"

#include "zigpc_node_state_int.h"

static const char LOG_TAG[] = "zigpc_node_state_observers";

/**
 * @brief Event handler when the PAN has been created
 *
 * @param event_data Event data (see struct zigpc_net_mgmt_on_network_init)
 */
void zigpc_node_state_on_network_init(void *event_data)
{
  static zigpc_node_state_status_change_t state_change;

  const struct zigpc_net_mgmt_on_network_init *net_init
    = (struct zigpc_net_mgmt_on_network_init *)event_data;

  memcpy(state_change.eui64, net_init->pc_eui64, sizeof(zigbee_eui64_t));
  state_change.status = ZIGBEE_NODE_STATUS_INCLUDED;

  zigpc_node_state_process_event_node_status_change(&state_change);
}

/**
 * @brief Event handler when a node has been added to the PAN
 *
 * @param event_data Event data (see struct zigpc_net_mgmt_on_node_added)
 */
void zigpc_node_state_on_node_added(void *event_data)
{
  static zigpc_node_state_status_change_t state_change;

  const struct zigpc_net_mgmt_on_node_added *node_added
    = (struct zigpc_net_mgmt_on_node_added *)event_data;

  memcpy(state_change.eui64, node_added->eui64, sizeof(zigbee_eui64_t));
  state_change.status = ZIGBEE_NODE_STATUS_NODEID_ASSIGNED;

  zigpc_node_state_process_event_node_status_change(&state_change);
}

/**
 * @brief Event handler when a node either succeeded or failed interview on the
 * PAN
 *
 * @param event_data Event data (see zigpc_net_mgmt_on_node_interview_status_t)
 */
void zigpc_node_state_on_node_interview_status(void *event_data)
{
  static zigpc_node_state_status_change_t state_change;

  const zigpc_net_mgmt_on_node_interview_status_t *node_interview_status
    = (zigpc_net_mgmt_on_node_interview_status_t *)event_data;

  memcpy(state_change.eui64,
         node_interview_status->eui64,
         sizeof(zigbee_eui64_t));
  state_change.status = (node_interview_status->success)
                          ? ZIGBEE_NODE_STATUS_INCLUDED
                          : ZIGBEE_NODE_STATUS_INTERVIEW_FAIL;

  zigpc_node_state_process_event_node_status_change(&state_change);
}

sl_status_t zigpc_node_state_init_observers(void)
{
  sl_status_t status;

  status = zigpc_net_mgmt_register_observer(ZIGPC_NET_MGMT_NOTIFY_NETWORK_INIT,
                                            zigpc_node_state_on_network_init);

  if (status == SL_STATUS_OK) {
    status = zigpc_net_mgmt_register_observer(ZIGPC_NET_MGMT_NOTIFY_NODE_ADDED,
                                              zigpc_node_state_on_node_added);
  }

  if (status == SL_STATUS_OK) {
    status = zigpc_net_mgmt_register_observer(
      ZIGPC_NET_MGMT_NOTIFY_NODE_INTERVIEW_STATUS,
      zigpc_node_state_on_node_interview_status);
  }

  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Network management observer registrations failed: 0x%X",
                 status);
  }

  return status;
}