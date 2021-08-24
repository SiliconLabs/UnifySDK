/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup zwave_network_management Z-Wave Network Management
 * @ingroup components
 * @brief Z-Wave network management component
 *
 * This component handles all logic related to changing the Z-Wave network,
 * adding and removing nodes. It also handles destroying the network and
 * joining the controller to an existing network. All Z-Wave secure inclusion
 * steps are also handled from within this module. Secure inclusion initiated
 * by an inclusion controller(proxy inclusion) is also passes through this
 * component.
 *
 * @{
 */

#ifndef ZWAVE_NETWORK_MANAGEMENT_H
#define ZWAVE_NETWORK_MANAGEMENT_H

#include <stdint.h>
#include <stdbool.h>
#include "sl_status.h"
#include "zwave_controller_types.h"
#include "zwave_network_management_state.h"
#include "zwave_controller_keyset.h"

#ifdef __cplusplus
extern "C" {
#endif

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

/**
 * @brief Get the current state of the network management module.
 *
 * @return zwave_network_management_state_t
 */
zwave_network_management_state_t zwave_network_management_get_state();

/** * @brief Abort current state and return to idle if its possible.
 *
 * Abort the current network management operation and return to idle. Note that
 * is not alway possible to abort the current operation.
 *
 * @return sl_status_t FAILURE if it is not possible to go to idle now.
 */
sl_status_t zwave_network_management_abort();

/**
 * @brief Put the Z-Wave controller in add node mode.
 *
 * When this function is activated Z-Wave controller will be open for adding nodes
 * nodes who sends a node information frame or a NWI inclusion request.
 *
 * Note that the controller will go back to idle state if no inclusion request
 * has been received within a a RECOMMENDED timeout value of 60 seconds.
 * Add Mode will timeout automatically after @ref ADD_REMOVE_TIMEOUT seconds
 * if no node gets included.
 *
 * The following callbacks may be triggered asynchronous by this operation
 * - on_state
 * - on_error
 * - on_add_node_started
 * - on_node_added
 *
 * @return sl_status_t FAILURE is state is not IDLE. No callbacks will be
 * triggered
 */
sl_status_t zwave_network_management_add_node();

/**
 * @brief Put the Z-Wave controller in smart start add node mode.
 *
 * This function should be called after receiving a SmartStart inclusion request
 * to which we have a matching provision list entry.
 *
 * The following callbacks may be triggered asynchronous by this operation:
 *
 * - on_state
 * - on_error
 * - on_add_node_started
 * - on_node_added
 *
 * @param dsk                   The full DSK of the node to add
 * @param granted_keys          The S2 keys to grant to the new node.
 * @param preferred_inclusion   The preferred protocol to use for inclusion.
 * @return sl_status_t FAILURE is state is not IDLE
 */
sl_status_t zwave_network_management_add_node_with_dsk(
  const zwave_dsk_t dsk,
  zwave_keyset_t granted_keys,
  zwave_protocol_t preferred_inclusion);

/**
 * @brief Set the controller in smart start mode.
 *
 * When smart start is enabled the controller will open the Z-Wave controller for
 * smart start inclusions. If an other network management operation is activated
 * the smart start inclusion mode will be temporary suspended until the
 * operation is done.
 *
 * When a smart start inclusion request is received the smart start mode is
 * automatically disabled. Smart start mode must be re-enabled manually if a smart
 * start inclusion is not stated as a response to a smart start inclusion
 * request.
 *
 * @param enable
 * - TRUE the network will be kept open for smart start inclusions.
 * - FALSE smart start inclusions are disabled.
 *
 */
void zwave_network_management_smart_start_enable(bool enable);

/**
 * @brief Inform an S2 bootstrapping controller which
 * keys must be granted to the node being bootstrapped.
 *
 * @param accept This field is used to indicate if the S2 bootstrapping process
 * is accepted by the user and must continue
 *
 * @param csa This field is used to indicate if the S2 bootstrapping controller
 * MUST allow CSA for Authentication.
 *
 * @param granted_keys
 * This field is used to indicate which network keys were granted during
 * bootstrapping.
 *
 * @return sl_status_t false if the we not expecting wating for this message
 */
sl_status_t zwave_network_management_keys_set(bool accept,
                                              bool csa,
                                              zwave_keyset_t granted_keys);

/**
 * @brief Indicate to the S2 bootstrapping controller whether
 * the DSK is accepted and report the user input when needed.
 *
 * @param dsk The full DSK of device to include.
 * @return sl_status_t false if the we not expecting waiting for this message
 */
sl_status_t zwave_network_management_dsk_set(zwave_dsk_t dsk);

/**
 * @brief Put the Z-Wave controller in remove node mode.
 *
 * When this function is activated Z-Wave controller will be open for removing nodes
 * nodes who sends a node information frame or a NWI exclusion request.
 *
 * The following callbacks may be triggered asynchronous by this operation
 * - on_state
 * - on_error
 * - on_node_removed
 *
 * @return sl_status_t FAILURE is state is not IDLE
 */
sl_status_t zwave_network_management_remove_node();

/**
 * @brief Put the Z-Wave controller in learn mode.
 *
 * When this function is activated Z-Wave controller will be open for joining
 * a new network to be removed from its current network to form a new blank
 * network. If the controller enters a network where this is not already a
 * SIS it assigns itself the SIS role.
 *
 * @param intent indicate the Learn Mode Intent (i.e., inclusion/exclusion,
 *               NWI, NWE and stop learn mode).
 * The Learn Mode intent value can be:
 * - 0x00: Stop learn mode (ZW_SET_LEARN_MODE_DISABLE)
 * - 0x01: Start direct range inclusion/exclusion (ZW_SET_LEARN_MODE_DIRECT_RANGE)
 * - 0x02: Start learn mode expecting NWI (ZW_SET_LEARN_MODE_NWI)
 * - 0x03: Start learn mode expecting NWE (ZW_SET_LEARN_MODE_NWE)
 *
 * @return sl_status_t BUSY if the state is not IDLE
 */
sl_status_t zwave_network_management_learn_mode(uint8_t intent);

/**
 * @brief Reset the controller.
 *
 * The controller will reset its home id to a random value and assign itself
 * node id 1. The controller will always assign itself the SIS role.
 *
 * The following callbacks may be triggered asynchronous by this operation
 *
 * - on_state
 * - on_new_network_entered
 *
 * @return sl_status_t FAILURE is state is not IDLE
 */
sl_status_t zwave_network_management_set_default(void);

///TODO:
///
sl_status_t zwave_network_management_replace_failed(zwave_node_id_t nodeid);
sl_status_t zwave_network_management_remove_failed(zwave_node_id_t nodeid);
sl_status_t zwave_network_management_proxy_inclusion(zwave_node_id_t nodeid);

/**
 * @brief Get the cached home_id.
 *
 * @return home_id
 */
zwave_home_id_t zwave_network_management_get_home_id();

/**
 * @brief Get the cached node_id.
 *
 * @return node_id
 */
zwave_node_id_t zwave_network_management_get_node_id();

/**
 * @brief Return the currently granted keys.
 *
 * @return zwave_keyset_t indicating the granted keys for our Z-Wave module
 */
zwave_keyset_t zwave_network_management_get_granted_keys();

/**
 * @returns true if ZPC is the SIS in the current network.
 */
bool zwave_network_management_is_zpc_sis();

/**
 * @brief Request Node Neighbor Discovery
 * @param node_id the NodeID of the node for which the neighbor discovery is requested
 *
 * @return sl_status_t BUSY if the state is not IDLE
 */
sl_status_t zwave_network_management_request_node_neighbor_discovery(
  zwave_node_id_t node_id);

/**
 * @brief Request the NM to perform proxy inclusion
 * @param node_id the NodeID of the node
 *
 * @return sl_status_t BUSY if the state is not IDLE
 */
sl_status_t zwave_network_management_start_proxy_inclusion(
  zwave_node_id_t node_id, zwave_node_info_t nif, uint8_t inclusion_step);

/**
 * @brief Assign static return routes within a Routing End Node.
 *
 * @param node_id is the Routing End Node ID
 * @param destination_node_id is the Route destination Node ID
 *
 * @returns returns  SL_STATUS_OK if assign return route was initiated
 */
sl_status_t zwave_network_management_assign_return_route(
  zwave_node_id_t node_id, zwave_node_id_t destination_node_id);

/* An application MUST time out waiting for the
 * ADD_NODE_STATUS_NODE_FOUND status if it does not receive the indication
 * after calling AddNodeToNetwork(ADD_NODE_ANY).
 * Value: 60 seconds (60000 ms) Refer: Z-wave Appl. Programmers Guide */
#define ADD_REMOVE_TIMEOUT                60000
#define NOP_FOR_REMOVE_FAILED_TIMEOUT     65000
#define SMART_START_SELF_DESTRUCT_TIMEOUT 3000  //3 seconds
#define LEARN_MODE_TIMEOUT_NETWORK_WIDE   6000  //6 seconds
// defining Direct range learn mode operation timeout to 20 seconds
#define LEARN_MODE_TIMEOUT_DIRECT_RANGE 20000

/* An application MUST time out waiting for the ADD_NODE_STATUS_ADDING_END_NODE
 * status if it does not receive the indication within a time period after
 * receiving the ADD_NODE_STATUS_NODE_FOUND status.
 * The RECOMMENDED timeout interval is 60 sec. */
#define ADD_NODE_STATUS_ADDING_END_NODE_TIMEOUT 60000

/*
 * INS13954 indicates that the timeout should be as follows:
 * 1. for including an End Node:
 * timeout = 76000 ms + listening nodes * 217 ms + FLiRS Nodes * 3517 ms
 * 2. for including a Controller Node:
 * timeout = 76000 ms + listening nodes * 217 ms + FLiRS Nodes * 3517 ms + network nodes * 732 ms
 *
 * This is the worst case scenario time out, we do not bother calculating the right timeout
 */
#define ADD_NODE_PROTOCOL_NEIGHBOR_DISCOVERY_TIMEOUT \
  (76000 + ZW_MAX_NODES * (3517 + 732))

#ifdef __cplusplus
}
#endif

/** @} end zwave_network_management */

#endif  //ZWAVE_NETWORK_MANAGEMENT_H
