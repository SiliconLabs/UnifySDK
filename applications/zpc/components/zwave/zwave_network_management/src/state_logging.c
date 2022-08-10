/**
 * @file state_logging.c
 * @brief Pretty printing of state and event names of zwave network management
 * state machine
 *
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 */

#include <stdio.h>
#include "nm_state_machine.h"
#include "zwave_network_management_state.h"

#include "sl_log.h"
#define LOG_TAG "network_management_state_logging"

#define STR_CASE(x) \
  case x:           \
    return #x;

const char *nm_state_name(zwave_network_management_state_t state)
{
  switch (state) {
    STR_CASE(NM_IDLE)
    STR_CASE(NM_WAITING_FOR_ADD)
    STR_CASE(NM_NODE_FOUND)
    STR_CASE(NM_WAIT_FOR_PROTOCOL)
    STR_CASE(NM_SET_DEFAULT)
    STR_CASE(NM_LEARN_MODE)
    STR_CASE(NM_LEARN_MODE_STARTED)
    STR_CASE(NM_WAIT_FOR_SECURE_ADD)
    STR_CASE(NM_WAITING_FOR_NODE_REMOVAL)
    STR_CASE(NM_WAITING_FOR_FAILED_NODE_REMOVAL)
    STR_CASE(NM_WAIT_FOR_SECURE_LEARN)
    STR_CASE(NM_REPLACE_FAILED_REQ)
    STR_CASE(NM_PREPARE_SUC_INCLISION)
    STR_CASE(NM_WAIT_FOR_SUC_INCLUSION)
    STR_CASE(NM_PROXY_INCLUSION_WAIT_NIF)
    STR_CASE(NM_WAIT_FOR_TX_TO_SELF_DESTRUCT)
    STR_CASE(NM_WAIT_FOR_SELF_DESTRUCT_REMOVAL)
    STR_CASE(NM_SEND_NOP)
    STR_CASE(NM_ASSIGNING_RETURN_ROUTE)
    STR_CASE(NM_FAILED_NODE_REMOVE)
    default:
      sl_log_warning(LOG_TAG, "Unknown state: %d", state);
      return "Unknown";
  }
}
const char *nm_event_name(nm_event_t event)
{
  switch (event) {
    STR_CASE(NM_EV_ADD_LEARN_READY)
    STR_CASE(NM_EV_ADD_NODE_FOUND)
    STR_CASE(NM_EV_ADD_CONTROLLER)
    STR_CASE(NM_EV_ADD_PROTOCOL_DONE)
    STR_CASE(NM_EV_ADD_END_NODE)
    STR_CASE(NM_EV_ADD_FAILED)
    STR_CASE(NM_EV_ADD_NOT_PRIMARY)
    STR_CASE(NM_EV_ADD_NODE_STATUS_DONE)
    STR_CASE(NM_EV_NODE_ADD)
    STR_CASE(NM_EV_TIMEOUT)
    STR_CASE(NM_EV_SECURITY_DONE)
    STR_CASE(NM_EV_S0_STARTED)
    STR_CASE(NM_EV_ADD_SECURITY_REQ_KEYS)
    STR_CASE(NM_EV_ADD_SECURITY_KEY_CHALLENGE)
    STR_CASE(NM_EV_NODE_ADD_S2)
    STR_CASE(NM_EV_ADD_SECURITY_KEYS_SET) /* 109 */
    STR_CASE(NM_EV_ADD_SECURITY_DSK_SET)
    STR_CASE(NM_EV_REPLACE_FAILED_START)
    STR_CASE(NM_EV_REPLACE_FAILED_STOP) /*112*/
    STR_CASE(NM_EV_REPLACE_FAILED_DONE)
    STR_CASE(NM_EV_REPLACE_FAILED_FAIL)
    STR_CASE(NM_EV_REPLACE_FAILED_START_S2)
    STR_CASE(NM_EV_LEARN_SET)
    STR_CASE(NM_EV_LEARN_FAILED)
    STR_CASE(NM_EV_REQUEST_NODE_LIST)
    STR_CASE(NM_EV_REQUEST_FAILED_NODE_LIST)
    STR_CASE(NM_EV_PROXY_COMPLETE)
    STR_CASE(NM_EV_START_PROXY_INCLUSION)
    STR_CASE(NM_EV_START_PROXY_REPLACE)
    STR_CASE(NM_EV_NODE_INFO)
    STR_CASE(NM_EV_FRAME_RECEIVED)
    STR_CASE(NM_EV_NODE_ADD_SMART_START) /* 125 */
    STR_CASE(NM_EV_REMOVE_FAILED_OK)
    STR_CASE(NM_EV_REMOVE_FAILED_FAIL)
    STR_CASE(NM_EV_SMART_START_ENABLE)
    STR_CASE(NM_EV_REMOVE_FAILED)
    STR_CASE(NM_EV_SET_DEFAULT)
    STR_CASE(NM_EV_SET_DEFAULT_COMPLETE)
    STR_CASE(NM_EV_NODE_REMOVE)
    STR_CASE(NM_EV_NODE_REMOVE_FAILED)
    STR_CASE(NM_EV_NODE_REMOVE_COMPLETE)
    STR_CASE(NM_EV_ABORT)
    STR_CASE(NM_EV_NOP_FAIL)
    STR_CASE(NM_EV_NOP_SUCCESS)
    STR_CASE(NM_EV_REQUEST_NODE_NEIGHBOR_REQUEST)
    STR_CASE(NM_EV_LEARN_STARTED)
    STR_CASE(NM_EV_LEARN_DONE)
    STR_CASE(NM_EV_ASSIGN_RETURN_ROUTE_START)
    STR_CASE(NM_EV_ASSIGN_RETURN_ROUTE_COMPLETED)
    STR_CASE(NM_EV_MAX)

    default:
      sl_log_warning(LOG_TAG, "Unknown event: %d", event);
      return "Unknown";
  }
}
