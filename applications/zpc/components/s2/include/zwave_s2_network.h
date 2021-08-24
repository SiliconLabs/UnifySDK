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

/**
 * @file zwave_s2_network.h
 * @brief Wrap LIBS2 inclusion functionality
 *
 * The main purpose of this module is to wrap the LIBS2 inclusion functions.
 * Here, the S2 inclusion is split into a set of callbacks, which are used by the
 * zwave_network_managemnt component. All S2 functionality, which is related to
 * properties of the Z-Wave network is handled here.
 *
 * @{
 */

#ifndef ZWAVE_S2_NETWORK_H
#define ZWAVE_S2_NETWORK_H

#ifdef __cplusplus
extern "C" {
#endif
#include "zwave_network_management.h"

typedef struct zwave_s2_network_callbacks {
  /// This event is triggered when the S2 inclusion process has started
  /// This can be use to stop e.g.,the S0 inclusion FSM.
  ///
  void (*on_inclusion_started)();
  /// Inclusion S2 complete event.
  /// @param granted_kyes Set of keys which has been granted.
  /// @param kex_fail_code Failure code of the inclusion a
  ///                      successful inclusion will have KEX_FAIL_NONE
  void (*on_inclusion_complete)(zwave_keyset_t granted_keys,
                                zwave_kex_fail_type_t kex_fail_code);
  /// This event is trigged after a zwave_s2_start_add_node when the node
  /// request its keys. A zwave_s2_key_grant must be called in response to this.
  /// S2_NODE_INCLUSION_KEX_REPORT_EVENT
  /// @param requested_keys Keys which the nodes has requested.
  /// @param csa            True if the node has requested client side authentication.
  void (*on_keys_request)(zwave_keyset_t requested_keys, bool csa);
  /// This event is trigged when a node has sent its public key for verification
  /// by the controller. A zwave_s2_dsk_accept must be called in response to this.
  /// See zwave_s2_dsk_accept  S2_NODE_INCLUSION_PUBLIC_KEY_CHALLENGE_EVENT
  /// @param granted_keys    Security keys granted earlier in inclusion process.
  /// @param num_blank_bytes Number of bytes which has been blanked out in the DSK
  /// @param dsk             DSK of the joining/including node with the first
  ///                        num_blank_bytes bytes set to zero.
  void (*on_dsk_challenge)(zwave_keyset_t granted_keys,
                           int num_blank_bytes,
                           zwave_dsk_t dsk);
} zwave_s2_network_callbacks_t;


/**
 * @brief Initialize the S2 network layer.
 *
 * This function must be called when ever the home ID or node ID of the
 * controller changes.
 *
 */
void zwave_s2_network_init();

/**
 * @brief Set callback function for the S2 inclusion. This structure is copied
 * internally.
 *
 * @param cb
 */
void zwave_s2_set_network_callbacks(const zwave_s2_network_callbacks_t *cb);

/**
 * @brief Start the S2 learn mode.
 *
 * This function will open up the S2 FSM for being included into a new network.
 *
 * @param node_id The node ID of the controller which is allowed to grant keys.
 */
void zwave_s2_start_learn_mode(zwave_node_id_t node_id);

/**
 * Tell LIBS2 that the gateway has been included (i.e., to start the TB1 timer).
 * This must be called on the "protocol done" inclusion event. See \ref
 * zwapi_add_node_to_network.
 */
void zwave_s2_neighbor_discovery_complete();

/**
 * @brief Start the S2 add node process.
 *
 * Calling this function will make the S2 FSM start the inclusion process of a
 * newly added node. This must be called right after ADD_NODE_DONE event for
 * nodes supporting COMMAND_CLASS_SECURITY2
 *
 * @param node_id The node we wish to include
 */
void zwave_s2_start_add_node(zwave_node_id_t node_id);


/**
 * @brief Must be called as a response to the on_keys_request callback.
 *
 * @param accept True if we should accept keys request
 * @param keys   Set of keys to grant.
 * @param csa    True if we allow for client side inclusion
 */
void zwave_s2_key_grant(bool accept, uint8_t keys, bool csa);

/**
 * @brief Must be called as a response to the on_dsk_challenge callbacks.
 *
 * @param accept True if we should accept keys request
 * @param dsk    Missing part of the dsk which the user has provided.
 * @param len    The number of bytes the user has provided.
 */
void zwave_s2_dsk_accept(bool accept, uint8_t *dsk, uint8_t len);

/**
 * Abort the current secure add /learn, the on_inclusion_complete callback will
 * be called when the inclusion has stopped.
 */
void zwave_s2_abort_join();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_S2_NETWORK_H
/** @} end zwave_s2_network */
