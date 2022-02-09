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

#include <stdio.h>
#include "zigpc_net_mgmt_fsm.h"

#define STATE_CHAR_BUFFER_LENGTH 50

#define STR_CASE_STATE(x)            \
  case ZIGPC_NET_MGMT_FSM_STATE_##x: \
    return #x;

#define STR_CASE_EVENT(x)            \
  case ZIGPC_NET_MGMT_FSM_EVENT_##x: \
    return #x;

const char *zigpc_net_mgmt_fsm_state_name(enum zigpc_net_mgmt_fsm_state state)
{
  static char str[STATE_CHAR_BUFFER_LENGTH];
  switch (state) {
    STR_CASE_STATE(INIT)
    STR_CASE_STATE(IDLE)
    STR_CASE_STATE(NODE_ADD)
    STR_CASE_STATE(NODE_REMOVE)
    STR_CASE_STATE(NODE_INTERVIEW)
    default:
      snprintf(str, STATE_CHAR_BUFFER_LENGTH, "UNKNOWN STATE: %d", state);
      return str;
  }
}
const char *zigpc_net_mgmt_fsm_event_name(enum zigpc_net_mgmt_fsm_event ev)
{
  static char str[STATE_CHAR_BUFFER_LENGTH];
  switch (ev) {
    STR_CASE_EVENT(INIT_COMPLETE)
    STR_CASE_EVENT(TIMEOUT)
    STR_CASE_EVENT(STATE_CHANGE_REQUEST)
    STR_CASE_EVENT(NODE_ADD_REQUEST)
    STR_CASE_EVENT(NODE_ADD_START)
    STR_CASE_EVENT(NODE_ADD_COMPLETE)
    STR_CASE_EVENT(NODE_INTERVIEW_STATUS)
    STR_CASE_EVENT(NODE_REMOVE_REQUEST)
    STR_CASE_EVENT(NODE_REMOVE_COMPLETE)
    default:
      snprintf(str, STATE_CHAR_BUFFER_LENGTH, "UNKNOWN EVENT: %d", ev);
      return str;
  }
}
