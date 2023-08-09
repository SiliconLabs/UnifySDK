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
 * @defgroup zwave_network_management_types Z-Wave Network Management Types
 * @ingroup zwave_definitions
 * @brief Z-Wave Network Management shared type definitions
 **
 * @{
 */

#ifndef ZWAVE_NETWORK_MANAGEMENT_TYPES_H
#define ZWAVE_NETWORK_MANAGEMENT_TYPES_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  /// The process failed, no new node was added in the network.
  ///
  ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_ADD_FAIL,
  /// Node has been included but the security bootstrapping failed.
  ///
  ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_ADD_SECURITY_FAIL,
  /// The remove process failed (no node was removed)
  ///
  ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_REMOVE_FAIL,
  /// The learn process failed in some general way
  ///
  ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_LEARN_MODE_FAIL,
  /// The learn process is complete and the controller was included in a network but
  /// security bootstrapping failed. The node is not operating securely.
  ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_LEARN_MODE_SECURITY_FAIL,
  /// The node responded to a NOP; thus the node is no longer failing.
  ///
  ZWAVE_NETWORK_MANAGEMENT_ERROR_FAILED_NODE_REMOVE_FAIL,
  /// The node responded to a NOP; thus the node is no longer failing.
  ///
  ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_REPLACE_FAIL,
  /// We were in SmartStart Learn mode but did not get a successful
  /// security bootstrapping. We will reset oureslves now.
  ZWAVE_NETWORK_MANAGEMENT_ERROR_NODE_SMART_START_INCLUSION_SECURITY_FAIL,
} zwave_network_management_error_t;

/**
 * @brief Error codes used in S2 inclusion
 *
 */
typedef enum {
  /// Bootstrapping was successful
  ///
  ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE,

  /// Key failure indicating that no match exists between requested/granted
  /// keys in the network.
  ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_KEX_KEY,

  ///Scheme failure indicating that no scheme is supported by
  ///controller or joining node specified an invalid scheme.
  ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_KEX_SCHEME,

  //Curve failure indicating that no curve is supported by controller
  //or joining node specified an invalid curve.
  ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_KEX_CURVES,

  /// Node failed to decrypt received frame.
  ///
  ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_DECRYPT,

  /// User has canceled the S2 bootstrapping.
  ///
  ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_CANCEL,

  /// The Echo KEX Set/Report frame did not match the earlier
  /// exchanged frame.
  ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_AUTH,

  /// The joining node has requested a key, which was not granted by
  /// the including node at an earlier stage.
  ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_KEY_GET,

  /// Including node failed to decrypt and hence verify the received
  /// frame encrypted with exchanged key.
  ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_KEY_VERIFY,

  /// The including node has transmitted a frame containing a
  /// different key than what is currently being exchanged.
  ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_KEY_REPORT,

  /// Unknown KEX fail type.
  ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_UNKNOWN,
} zwave_kex_fail_type_t;

typedef enum {
  /// No Learn mode intent.
  ZWAVE_NETWORK_MANAGEMENT_LEARN_NONE,
  /// Network Wide Inclusion learn mode
  ZWAVE_NETWORK_MANAGEMENT_LEARN_NWI,
  /// Enable Network Wide Exclusion
  ZWAVE_NETWORK_MANAGEMENT_LEARN_NWE,
  /// SmartStart Learn mode
  ZWAVE_NETWORK_MANAGEMENT_LEARN_SMART_START,
} zwave_learn_mode_t;

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
   * Waiting for the Z-Wave Protocol to assign NodeID and HomeID to the node
   * being included
   */
  NM_WAIT_FOR_PROTOCOL,

  /** Reset state. Also used as part of Network Exclusion
   *
   * The Network Management engine will now wait for the Z-Wave Controller to
   * reset, leave its network and start a new network.
   */
  NM_SET_DEFAULT,
  /** Learn mode state.
   *
   * The Z-Wave Network Management state machine is in Learn Mode.
   * (awaiting inclusion, exclusion, or controller replication)
   * GW is processing LEARN_SET, waiting for protocol.
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
   *
   * The Protocol has now included a node in the network, we are waiting for the
   * S0 or S2 Bootstrapping to complete or time out.
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
   *     ZWAVE_NETWORK_MANAGEMENT_ERROR_FAILED_NODE_REMOVE_FAIL)
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
   * In case of inclusion, Network Management is waiting for S2/S0 bootstrapping
   * to complete.
   *
   * In case of NM_EV_ADD_SECURITY_KEY_CHALLENGE, accept.
   * In case of NM_EV_LEARN_SET/DISABLE, abort S2.
   *
   * In case of exclusion, controller replication, and controller
   * shift, NM_EV_SECURITY_DONE is triggered synchronously in NMS and
   * state is changed synchronously when LEARN_MODE_DONE is
   * received to NM_SET_DEFAULT
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
   * transfer end to SUC. Then go to NM_WIAT_FOR_SUC_INCLUSION
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
   * waits for protocol callback nop_send_done() to trigger
   * NM_EV_TX_DONE_SELF_DESTRUCT.  Then we start a timer, call
   * ZW_RemoveFailedNode() and go to \ref
   * NM_WAIT_FOR_SELF_DESTRUCT_REMOVAL.
   */
  NM_WAIT_FOR_TX_TO_SELF_DESTRUCT,

  /** Add mode state.
   * SmartStart.
   *
   * Waiting for the Z-Wave API module to check that the node has left
   * the network after a failed S2 bootstrapping attempt following a SmartStart
   * inclusion
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

#endif  //ZWAVE_NETWORK_MANAGEMENT_TYPES_H
        /** @} end zwave_network_management_types */