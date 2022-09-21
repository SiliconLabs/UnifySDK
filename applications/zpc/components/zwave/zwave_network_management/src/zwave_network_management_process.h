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

/**
 * @file zwave_network_management_process.h
 * @brief Interface to zwave_network_management_process
 *
 * @{
 */

#ifndef ZWAVE_NETWORK_MANAGEMENT_PROCESS_H
#define ZWAVE_NETWORK_MANAGEMENT_PROCESS_H

#include "process.h"

typedef enum {

  NM_EV_ADD_LEARN_READY,
  NM_EV_ADD_NODE_FOUND,
  NM_EV_ADD_CONTROLLER,
  NM_EV_ADD_PROTOCOL_DONE,
  NM_EV_ADD_END_NODE,
  NM_EV_ADD_FAILED,
  NM_EV_ADD_NOT_PRIMARY,
  NM_EV_ADD_NODE_STATUS_DONE,
  NM_EV_NODE_ADD,
  NM_EV_TIMEOUT,
  /** See on_inclusion_complete */
  NM_EV_SECURITY_DONE,
  NM_EV_S0_STARTED,
  /** NM_EV_ADD_SECURITY_REQ_KEYS event is triggered (from on_keys_request)
   * when kex get (key request) is received. So that the user can be notified
   * to grant keys prompt
   */
  NM_EV_ADD_SECURITY_REQ_KEYS,
  /* NM_EV_ADD_SECURITY_KEY_CHALLENGE is called (fro on_dsk_challenge)when
   * public key report is received from other side. NM FSM calls
   * zwave_controller_on_dsk_report() to send the request user
   */
  NM_EV_ADD_SECURITY_KEY_CHALLENGE,
  NM_EV_NODE_ADD_S2,

  /** NM_EV_ADD_SECURITY_KEYS_SET event is triggered when the user grants
   * keys with zwave_network_management_keys_set()
   * NM FSM calls zwave_s2_key_grant() to notify S2 about the granted keys
   */
  NM_EV_ADD_SECURITY_KEYS_SET,
  /** NM_EV_ADD_SECURITY_DSK_SET is triggered (from zwave_network_management_dsk_set)
   * to Indicate to the S2 bootstrapping controller whether the DSK is accepted
   * and report the user input when needed
   */
  NM_EV_ADD_SECURITY_DSK_SET,
  NM_EV_REPLACE_FAILED_START,
  NM_EV_REPLACE_FAILED_STOP,
  NM_EV_REPLACE_FAILED_DONE,
  NM_EV_REPLACE_FAILED_FAIL,
  NM_EV_REPLACE_FAILED_START_S2,
  NM_EV_LEARN_SET,
  NM_EV_LEARN_STARTED,
  NM_EV_LEARN_DONE,
  NM_EV_LEARN_STOP,
  NM_EV_LEARN_FAILED,
  NM_EV_REQUEST_NODE_LIST,
  NM_EV_REQUEST_FAILED_NODE_LIST,
  NM_EV_PROXY_COMPLETE,
  NM_EV_START_PROXY_INCLUSION,
  NM_EV_START_PROXY_REPLACE,
  NM_EV_NODE_INFO,
  NM_EV_FRAME_RECEIVED,
  NM_EV_NODE_ADD_SMART_START,
  NM_EV_REMOVE_FAILED,
  NM_EV_REMOVE_FAILED_OK,
  NM_EV_REMOVE_FAILED_FAIL,
  NM_EV_SMART_START_ENABLE,
  NM_EV_SET_DEFAULT,
  NM_EV_SET_DEFAULT_COMPLETE,
  NM_EV_NODE_REMOVE,
  NM_EV_NODE_REMOVE_FAILED,
  NM_EV_NODE_REMOVE_COMPLETE,
  NM_EV_ABORT,
  NM_EV_NOP_FAIL,
  NM_EV_NOP_SUCCESS,
  NM_EV_REQUEST_NODE_NEIGHBOR_REQUEST,
  NM_EV_REQUEST_NODE_NEIGHBOR_REQUEST_COMPLETE,
  NM_EV_ASSIGN_RETURN_ROUTE_START,
  NM_EV_ASSIGN_RETURN_ROUTE_COMPLETED,
  NM_EV_MAX,  //This MUST always to be last entry in this enum
} nm_event_t;

#ifdef __cplusplus
extern "C" {
#endif

PROCESS_NAME(zwave_network_management_process);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_NETWORK_MANAGEMENT_PROCESS_H
/** @} end zwave_network_management_process */
