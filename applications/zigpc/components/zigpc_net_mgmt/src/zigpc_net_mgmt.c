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

#include <string.h>
#include <stdio.h>

// Shared UIC includes
#include <sl_status.h>
#include <sl_log.h>

// ZigPC includes
#include <zigpc_common_zigbee.h>
#include <zigpc_gateway.h>

// Component includes
#include "zigpc_net_mgmt.h"
#include "zigpc_net_mgmt_fsm.h"
#include "zigpc_net_mgmt_process_send.h"

sl_status_t zigpc_netmgmt_network_permit_joins(bool enable)
{
  sl_status_t status = SL_STATUS_OK;

  if (!enable && zigpc_netmgmt_fsm_state_change_in_progress()) {
    status = SL_STATUS_BUSY;
  } else {
    status = zigpc_gateway_network_permit_joins(enable);
  }

  return status;
}

sl_status_t zigpc_net_mgmt_add_node(const zigbee_eui64_t eui64,
                                    const zigbee_install_code_t install_code,
                                    const uint8_t install_code_length)
{
  sl_status_t status = SL_STATUS_OK;
  if (eui64 == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    zigpc_net_mgmt_process_data_fsm_t process_data_fsm;
    zigpc_net_mgmt_fsm_node_add_t *add_node_data
      = &process_data_fsm.fsm_data.node_add_request;
    memcpy(add_node_data->eui64, eui64, sizeof(zigbee_eui64_t));
    memcpy(add_node_data->install_code,
           install_code,
           sizeof(zigbee_install_code_t));
    add_node_data->install_code_length = install_code_length;

    process_data_fsm.fsm_event = ZIGPC_NET_MGMT_FSM_EVENT_NODE_ADD_REQUEST;

    status = zigpc_net_mgmt_process_send_event(
      ZIGPC_NET_MGMT_EVENT_FSM,
      (void *)&process_data_fsm,
      sizeof(zigpc_net_mgmt_process_data_fsm_t));
  }

  return status;
}

sl_status_t zigpc_net_mgmt_interview_node(const zigbee_eui64_t eui64)
{
  sl_status_t status = SL_STATUS_OK;
  zigpc_net_mgmt_process_data_fsm_t process_data_fsm;
  zigpc_net_mgmt_fsm_node_interview_t *interview_node_data
    = &process_data_fsm.fsm_data.node_interview_request;

  memcpy(interview_node_data->eui64, eui64, sizeof(zigbee_eui64_t));

  process_data_fsm.fsm_event = ZIGPC_NET_MGMT_FSM_EVENT_NODE_INTERVIEW_REQUEST;

  zigpc_net_mgmt_process_send_event(ZIGPC_NET_MGMT_EVENT_FSM,
                                    (void *)&process_data_fsm,
                                    sizeof(zigpc_net_mgmt_process_data_fsm_t));

  return status;
}

sl_status_t zigpc_net_mgmt_remove_node(const zigbee_eui64_t eui64)
{
  sl_status_t status = SL_STATUS_OK;
  zigpc_net_mgmt_process_data_fsm_t process_data_fsm;
  zigpc_net_mgmt_fsm_node_remove_t *remove_node_data
    = &process_data_fsm.fsm_data.node_remove_request;
  if (zigpc_net_mgmt_fsm_get_state() != ZIGPC_NET_MGMT_FSM_STATE_IDLE) {
    status = SL_STATUS_BUSY;
  } else {
    memcpy(remove_node_data->eui64, eui64, sizeof(zigbee_eui64_t));

    process_data_fsm.fsm_event = ZIGPC_NET_MGMT_FSM_EVENT_NODE_REMOVE_REQUEST;

    status = zigpc_net_mgmt_process_send_event(
      ZIGPC_NET_MGMT_EVENT_FSM,
      (void *)&process_data_fsm,
      sizeof(zigpc_net_mgmt_process_data_fsm_t));
  }

  return status;
}

sl_status_t zigpc_net_mgmt_state_change_request(
  const zigpc_net_mgmt_state_change_request_t *action)
{
  sl_status_t status = SL_STATUS_OK;
  zigpc_net_mgmt_process_data_fsm_t process_data_fsm;
  zigpc_net_mgmt_state_change_request_t *request_data
    = &process_data_fsm.fsm_data.state_change_request;

  if (action == NULL) {
    status = SL_STATUS_NULL_POINTER;
  }
  if ((status == SL_STATUS_OK)
      && ((action->requested_state == ZIGPC_NET_MGMT_FSM_STATE_MIN_VAL)
          || (action->requested_state >= ZIGPC_NET_MGMT_FSM_STATE_MAX_VAL))) {
    status = SL_STATUS_INVALID_TYPE;
  }

  if (status == SL_STATUS_OK) {
    request_data->requested_state = action->requested_state;

    if (action->param_eui64_filled == true) {
      memcpy(request_data->param_eui64,
             action->param_eui64,
             sizeof(zigbee_eui64_t));
      request_data->param_eui64_filled = true;
    } else {
      request_data->param_eui64_filled = false;
    }

    process_data_fsm.fsm_event = ZIGPC_NET_MGMT_FSM_EVENT_STATE_CHANGE_REQUEST;

    status = zigpc_net_mgmt_process_send_event(
      ZIGPC_NET_MGMT_EVENT_FSM,
      (void *)&process_data_fsm,
      sizeof(zigpc_net_mgmt_process_data_fsm_t));
  }

  return status;
}
