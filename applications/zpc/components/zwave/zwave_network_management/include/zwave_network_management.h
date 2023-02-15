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
 * @defgroup zwave_network_management Z-Wave Network Management
 * @ingroup zwave_controller
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

// Generic includes
#include <stdint.h>
#include <stdbool.h>

#include "zwave_controller_types.h"
#include "zwave_generic_types.h"
#include "zwave_network_management_types.h"

#include "zwave_controller_keyset.h"
#include "zwave_network_management_types.h"
// Interfaces
#include "zwave_generic_types.h"

// Unify includes
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

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
 * @brief Put the Z-Wave controller in SmartStart add node mode.
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
 * @brief Set the controller in SmartStart Add mode.
 *
 * When SmartStart is enabled the controller will open the Z-Wave controller for
 * SmartStart inclusions. If an other network management operation is activated
 * the SmartStart inclusion mode will be temporary suspended until the
 * operation is done.
 *
 * When a SmartStart inclusion request is received the SmartStart mode is
 * automatically disabled. SmartStart mode must be re-enabled manually if a smart
 * start inclusion is not stated as a response to a SmartStart inclusion
 * request.
 *
 * @param enabled
 * - TRUE the network will be kept open for SmartStart inclusions.
 * - FALSE SmartStart inclusions are disabled.
 *
 */
void zwave_network_management_enable_smart_start_add_mode(bool enabled);

/**
 * @brief Configures if the Network Management module will enter SmartStart
 * learn mode when it goes back to IDLE, we are alone in a network SmartStart
 * Add mode is disabled.
 *
 * @param enabled
 */
void zwave_network_management_enable_smart_start_learn_mode(bool enabled);

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
 * @param mode indicate the Learn Mode Intent (i.e., inclusion/exclusion,
 *               NWI, NWE and stop learn mode).
 * @return sl_status_t BUSY if the state is not IDLE
 */
sl_status_t zwave_network_management_learn_mode(zwave_learn_mode_t mode);

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

/**
 * @brief Remove a failed node
 *
 * This function can be used for remove a failing node from Z-Wave network.
 * NOP will be sent to the node to make sure that the Z-Wave protocol marks
 * it failing on the controller and the subsequent remove failed call for that
 * node will succeed.
 *
 * The following callbacks may be triggered asynchronous by this operation
 *
 * - on_node_deleted
 * - on_error
 *
 * @param nodeid Node id of failing node which will be removed
 * @returns sl_status_t SL_STATUS_OK when remove failed is triggered, any other
 * code if it failed
 */

sl_status_t zwave_network_management_remove_failed(zwave_node_id_t nodeid);

/**
 * @brief Get the cached HomeID.
 *
 * @returns The current HomeID assigned to the Z-Wave Controller
 */
zwave_home_id_t zwave_network_management_get_home_id();

/**
 * @brief Get the cached NodeID.
 *
 * @returns The current NodeID assigned to the Z-Wave Controller
 */
zwave_node_id_t zwave_network_management_get_node_id();

/**
 * @brief Returns the number of nodes in our network.
 *
 * @return Number of nodes in our netowrk.
 */
uint16_t zwave_network_management_get_network_size();

/**
 * @brief Get the cached Node List for the current network.
 *
 * @param [out] node_list Pointer where the node list will be copied.
 */
void zwave_network_management_get_network_node_list(zwave_nodemask_t node_list);

/**
 * @brief Return the currently granted keys.
 *
 * @return zwave_keyset_t indicating the granted keys for our Z-Wave module
 */
zwave_keyset_t zwave_network_management_get_granted_keys();

/**
 * @brief Checks if we have the SIS role in the current network.
 *
 * @returns true if we have the SIS role
 * @returns false if we do not have the SIS role in this network
 */
bool zwave_network_management_is_zpc_sis();

/**
 * @brief Checks if the protocol may be sending some frames to a particular NodeID
 *
 * @param node_id       The Z-Wave NodeID that we want to check for.
 * @returns true if the protocol is currently sending frames to the node, false
 *          if there is no operation that may trigger communications with the node.
 */
bool zwave_network_management_is_protocol_sending_frames_to_node(
  zwave_node_id_t node_id);

/**
 * @brief Request Node Neighbor Discovery
 * @param node_id the NodeID of the node for which the neighbor discovery is requested
 *
 * @return sl_status_t SL_STATUS_OK if the operation was accepted, any other
 * code is case of failure
 */
sl_status_t zwave_network_management_request_node_neighbor_discovery(
  zwave_node_id_t node_id);

/**
 * @brief Request the NM to perform proxy inclusion
 * @param node_id the NodeID of the node
 * @param nif
 * @param inclusion_step
 *
 * @return sl_status_t SL_STATUS_OK if the operation was accepted, any other
 * code is case of failure
 */
sl_status_t zwave_network_management_start_proxy_inclusion(
  zwave_node_id_t node_id, zwave_node_info_t nif, uint8_t inclusion_step);

/**
 * @brief Assign static return routes to End Nodes.
 *
 * @param node_id              NodeID for which return routes will be provided
 * @param destination_node_id  Destination NodeID of the return routes
 *
 * @returns returns SL_STATUS_OK if the request was accepted.
 */
sl_status_t zwave_network_management_assign_return_route(
  zwave_node_id_t node_id, zwave_node_id_t destination_node_id);

/**
 * @brief Verify if we are carrying some network management operations
 *
 * @returns true if we are busy doing something, false if IDLE.
 */
bool zwave_network_management_is_busy();

/**
 * @brief Checks if we have return routes to assign for a given NodeID
 * @param node_id   NodeID to look for in the return route queue
 * @returns true or false
 */
bool we_have_return_routes_to_assign(zwave_node_id_t node_id);

/* An application MUST time out waiting for the
 * ADD_NODE_STATUS_NODE_FOUND status if it does not receive the indication
 * after calling AddNodeToNetwork(ADD_NODE_ANY).
 * Value: 60 seconds (60000 ms) Refer: Z-wave Appl. Programmers Guide */
#define ADD_REMOVE_TIMEOUT                60000
#define NOP_FOR_REMOVE_FAILED_TIMEOUT     65000
#define SMART_START_SELF_DESTRUCT_TIMEOUT 3000  //3 seconds

// How long do we wait for the Z-Wave API to callback after set default?
#define SET_DEFAULT_TIMEOUT 15000

// How long do we stay in Learn Mode before timing out and returning to IDLE in ms.
#define LEARN_MODE_TIMEOUT 30000

/// Time that Send data (including e.g. sending NOP) will take as a maximum.
/// Protocol that tries routing with FLiRS nodes takes very very long.
#define SEND_DATA_TIMEOUT 125000

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
