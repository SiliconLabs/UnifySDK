
/******************************************************************************
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

/**
 * @file zwave_network_management_state.h
 * @addtogroup zwave_network_management
 * @ingroup zwave_controller
 * @{
 */

#ifndef ZWAVE_NETWORK_MANAGEMENT_STATE_H
#define ZWAVE_NETWORK_MANAGEMENT_STATE_H

/**
 * @brief States of the Network Management module.
 *
 */
typedef enum {
  /** Idle state. */
  NM_IDLE,

  /** Add mode state.
   * Direct range/NWI, S2, or SmartStart.
   *
   * Waiting for protocol.  On \ref NM_EV_ADD_NODE_FOUND, go to \ref NM_NODE_FOUND.
   */
  NM_WAITING_FOR_ADD,
  /** Add mode state.
   * Direct range/NWI, S2, SmartStart, Proxy Inclusion, or Proxy Replace.
   *
   * Waiting for protocol to receive node info from a node.  On \ref
   * NM_EV_ADD_CONTROLLER or \ref NM_EV_ADD_END_NODE, cache the node
   * id and node info and go to \ref NM_WAIT_FOR_PROTOCOL.
   */
  NM_NODE_FOUND,
  /** Add mode state.
   * Direct range/NWI, S2, SmartStart, Proxy Inclusion, or Proxy Replace.
   *
   * Waiting for protocol to assign node ID and home ID to the new node.
   *
   * On \ref NM_EV_ADD_PROTOCOL_DONE, send an ADD_NODE_STOP to
   * protocol.  On \ref NM_EV_ADD_NODE_STATUS_DONE, register the new
   * node in \ref node_db and go to \ref NM_WAIT_FOR_SECURE_ADD.  If
   * GW is handling inclusion, start security processing the new node
   * (S2 or S0 inclusion).  Otherwise, start a 2 s timer and go to \ref
   * NM_PREPARE_SUC_INCLISION.
   */
  NM_WAIT_FOR_PROTOCOL,

  /** Learn mode state (exclusion).  Also used if GW is processing DEFAULT_SET.
   *
   * Waiting for NetworkManagement_mdns_exited() to be called from
   * \ref ZIP_Router.  Post \ref ZIP_EVENT_RESET to \ref ZIP_Router
   * and call SendReplyWhenNetworkIsUpdated() to go to \ref
   * NM_WAITING_FOR_PROBE.  SendReplyWhenNetworkIsUpdated() sets up a
   * trigger to bring NMS back to \ref NM_IDLE when the gateway reset
   * is completed.
   *
   * \note When changing to this state, we MUST call rd_exit() to advance
   * the state machine.
   */
  NM_SET_DEFAULT,
  /** Learn mode state.
   *
   * GW is processing LEARN_SET, waiting for protocol.
   *
   * On callback (LearnModeStatus()) with LEARN_MODE_STARTED, go to \ref
   * NM_LEARN_MODE_STARTED, Lock RD probe machine, invalidate
   * MyNodeID, and start S2 learn-mode state machine.
   *
   * Can be canceled or time out.  This will trigger a partial
   * re-initialization of the gateway (ApplicationInitNIF()), unlock RD
   * probe.  ResetState() from nm_send_reply() with the
   * LEARN_MODE_FAILED will bring NMS back to \ref NM_IDLE.
   */
  NM_LEARN_MODE,

  /** Learn mode state.
   *
   * The gateway is processing LEARN_SET and has received
   * LEARN_MODE_STARTED from protocol.  Now the protocol is committed
   * and the gateway waits for protocol completion.
   *
   * On protocol callback (LearnModeStatus()) with LEARN_MODE_DONE,
   * the gateway has been assigned a node ID and home ID.  Go to \ref
   * NM_WAIT_FOR_SECURE_LEARN, detect new environment and reset
   * gateway accordingly.
   *
   * Cannot be interrupted because the gateway properties have changed
   * and we are waiting on the protocol.
   */
  NM_LEARN_MODE_STARTED,

  /** Add mode state.
   * Direct range/NWI, S2, SmartStart, Proxy Inclusion, Proxy Replace.
   *
   * Waiting for the S2 inclusion state machine to complete.  On \ref
   * NM_EV_SECURITY_DONE, go to \ref NM_WAIT_FOR_PROBE_AFTER_ADD and
   * start probe.
   *
   * If SmartStart security fails, start a
   * #SMART_START_SELF_DESTRUCT_TIMEOUT sec. timer and go to
   * #NM_WAIT_FOR_SELF_DESTRUCT.
   */
  NM_WAIT_FOR_SECURE_ADD,

  /** Remove node state.
   *
   * SL_STATUS OK return value of zwapi_remove_node_from_network() bring NMS
   * to this state. Here we set time of ADD_REMOVE_TIMEOUT (60 seconds) to wait
   * for callback on_remove_node_status_update()
   *
   * On timeout or callback status
   * REMOVE_NODE_STATUS_FAILED (NM_EV_NODE_REMOVE_FAILED)
   * NMS is brought back to NMS_IDLE state
   *
   */
  NM_WAITING_FOR_NODE_REMOVAL,
  /** Remove failed node state.
   *
   * Waiting for protocol after call to zwapi_remove_failed_node()
   * In callback on_remove_failed_status_update(),
   * - ZW_NODE_OK / ZW_FAILED_NODE_NOT_REMOVED /
   *   NM_WAITING_FOR_FAILED_NODE_REMOVAL : (NM_EV_REMOVE_FAILED_FAIL)
   *   reports error in node deletion with
   *   zwave_controller_on_error(
   *     ZWAVE_NETWORK_MANAGEMENT_ERROR_FAILED_NODE_REMOVE_FAIL);
   *
   * - ZW_FAILED_NODE_REMOVED (NM_EV_REMOVE_FAILED_OK) or on timeout
   *   Reports node is delete with zwave_controller_on_node_deleted()
   *
   * next state of NMS after this is NM_IDLE.
   */
  NM_WAITING_FOR_FAILED_NODE_REMOVAL,

  /** Learn mode state.
   *
   * The gateway has received #LEARN_MODE_DONE from protocol.
   *
   * In case of inclusion, Network Management is waiting for S2/S0
   * learn mode to complete.  On \ref NM_EV_SECURITY_DONE, go to \ref
   * NM_WAIT_FOR_MDNS and call rd_exit() to reset the gateway for a
   * new network.
   *
   * In case of #NM_EV_ADD_SECURITY_KEY_CHALLENGE, accept.
   * In case of NM_EV_LEARN_SET/DISABLE, abort S2.
   *
   * In case of exclusion, controller replication, and controller
   * shift, #NM_EV_SECURITY_DONE is triggered synchronously in NMS and
   * state is changed synchronously when #LEARN_MODE_DONE is
   * received to either #NM_SET_DEFAULT or #NM_WAIT_FOR_MDNS.
   */
  NM_WAIT_FOR_SECURE_LEARN,

  /** Replace failed state.
   *
   * Replace and replace S2.
   *
   * On \ref NM_EV_REPLACE_FAILED_DONE, if GW is handling inclusion, go to
   * \ref NM_WAIT_FOR_SECURE_ADD.  Otherwise go to
   * \ref NM_PREPARE_SUC_INCLISION.
   */
  NM_REPLACE_FAILED_REQ,

  /** Add mode state.
   *
   * Direct range/NWI, S2, SmartStart, Replace Failed, Proxy Inclusion, or
   * Proxy Replace.
   *
   * Wait for timeout to let protocol finish up its add by sending a
   * transfer end to SUC. Then go to \ref NM_WIAT_FOR_SUC_INCLUSION
   * and request handover.
   */
  NM_PREPARE_SUC_INCLISION,

  /** Add mode state.
   *
   * Direct range/NWI, S2, SmartStart, Replace Failed, Proxy Inclusion, or
   * Proxy Replace.
   *
   * Waiting for the SUC to complete the secure part of inclusion.
   *
   * On \ref NM_EV_PROXY_COMPLETE, go to \ref NM_WAIT_FOR_SECURE_ADD
   * and post \ref NM_EV_SECURITY_DONE.
   */
  NM_WAIT_FOR_SUC_INCLUSION,

  /** Add mode state.
   * Proxy inclusion and proxy replace.
   *
   * Safe-guarded by a timer, on timeout, go to \ref NM_IDLE.
   *
   * On \ref NM_EV_NODE_INFO, inclusion goes to \ref NM_NODE_FOUND and fakes
   * \ref NM_EV_ADD_CONTROLLER and \ref NM_EV_ADD_NODE_STATUS_DONE.  Replace
   * goes to \ref NM_REPLACE_FAILED_REQ and fakes
   * \ref NM_EV_REPLACE_FAILED_DONE.
   */
  NM_PROXY_INCLUSION_WAIT_NIF,

  /** Add mode state.
   * SmartStart.
   *
   * The gateway must send NOPs to a failing node and wait for the
   * result, before doing ZW_RemoveFailed().
   *
   * The NOP is sent before entering this state.  Now the gateway
   * waits for protocol callback nop_send_done() to trigger \ref
   * NM_EV_TX_DONE_SELF_DESTRUCT.  Then we start a timer, call
   * ZW_RemoveFailedNode() and go to \ref
   * NM_WAIT_FOR_SELF_DESTRUCT_REMOVAL.
   */
  NM_WAIT_FOR_TX_TO_SELF_DESTRUCT,

  /** Add mode state.
   * SmartStart.
   *
   * Waiting for protocol to complete the ZW_RemoveFailedNode() (or
   * for timeout).  Protocol callback RemoveSelfDestructStatus()
   * triggers sending status to unsolicited destinations with
   * #ResetState() callback.  Timeout calls #ResetState() directly.
   */
  NM_WAIT_FOR_SELF_DESTRUCT_REMOVAL,
  /** Failed node remove state.
   *
   *  NMS sends NOP to failing node before coming to this state and waits for
   *  callback nop_send_done() and calls zwapi_remove_failed_node(). If
   *  zwapi_remove_failed_node() does not return ZW_FAILED_NODE_REMOVE_STARTED
   *  its callback (on_remove_failed_status_update) is called with
   *  ZW_FAILED_NODE_NOT_REMOVED()
   */
  NM_FAILED_NODE_REMOVE,
  /** Failed node remove state.
   *  Add node state (SmartStart self destruct)
   *
   *  NMS Sends NOP in this state and next state is set according to if entry
   *  was due to timeout event or NM_EV_REMOVE_FAILED event from NM_IDLE.
   */
  NM_SEND_NOP,
  /**
   * Assign return route state
   * when the NM get the zwave_api assign return route callback
   * the NM state shall be shift to IDLE
   *
   */
  NM_ASSIGNING_RETURN_ROUTE,
  /**
   * Performing a neighbor discovery/update
   * The protocol has requested a node to perform a neighbor discovery and
   * waits for the result.
   */
  NM_NEIGHBOR_DISCOVERY_ONGOING,
} zwave_network_management_state_t;
#endif

/** @} end of zwave_network_management */
