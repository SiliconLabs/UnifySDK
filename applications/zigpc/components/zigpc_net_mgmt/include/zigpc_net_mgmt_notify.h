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
 * @file zigpc_net_mgmt_notify.h
 * @defgroup  zigpc_net_mgmt_notify ZigPC Observer API
 * @ingroup zigpc_net_mgmt
 * @brief Observer API for getting events from zigpc_gateway
 *
 * In order to listen to events from Net Management, ZigPC Net Mgmt allows components
 * to register observer functions that receive an event type and associated data
 *
 * @{
 */

#ifndef ZIGPC_NET_MGMT_NOTIFY_H
#define ZIGPC_NET_MGMT_NOTIFY_H

/* Contiki includes */
#include "sl_status.h"
#include "zigpc_net_mgmt.h"
#include "zigpc_common_observable.h"
#include "zigpc_common_zigbee.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ZIGPC_NET_MGMT_REQUESTED_STATE_PARAMS_MAX 10

/**
 * @brief Notify event types
 *
 */
enum zigpc_net_mgmt_notify_event {
  ZIGPC_NET_MGMT_NOTIFY_NETWORK_INIT = 0,
  ZIGPC_NET_MGMT_NOTIFY_STATE_UPDATE,
  ZIGPC_NET_MGMT_NOTIFY_NODE_ADDED,
  ZIGPC_NET_MGMT_NOTIFY_NODE_INTERVIEW_STATUS,
  ZIGPC_NET_MGMT_NOTIFY_NODE_REMOVED,
  ZIGPC_NET_MGMT_NOTIFY_MAX,
};

/**
 * @brief Event data on network state update
 *
 */
typedef struct zigpc_net_mgmt_on_network_init {
  zigbee_eui64_t pc_eui64;
} zigpc_net_mgmt_on_network_init_t;

/**
 * @brief Event data on network state update. These fields
 * will be used to publish the new state as well as any missing
 * paramters needed to advance the network management FSM.
 *
 */
typedef struct zigpc_net_mgmt_on_network_state_update {
  /**
   * @brief The new network management state
   *
   */
  enum zigpc_net_mgmt_fsm_state new_state;

  /**
   * @brief The next possible states in network management based on the new
   * state that is supplied.
   *
   */
  enum zigpc_net_mgmt_fsm_state
    next_supported_states_list[ZIGPC_NET_MGMT_FSM_STATE_MAX_VAL];
  uint8_t next_supported_states_count;

  /**
   * @brief List of requested state parameters needed to perform network
   * management action (add node, remove node, etc.). These parameters are
   * supplied as a list of character arrays.
   *
   */
  const char
    *requested_state_parameter_list[ZIGPC_NET_MGMT_REQUESTED_STATE_PARAMS_MAX];
  uint8_t requested_state_parameter_count;
} zigpc_net_mgmt_on_network_state_update_t;

/**
 * @brief Event data on node added to the network
 *
 */
typedef struct zigpc_net_mgmt_on_node_added {
  zigbee_eui64_t eui64;
} zigpc_net_mgmt_on_node_added_t;

typedef struct {
  bool success;
  zigbee_eui64_t eui64;
} zigpc_net_mgmt_on_node_interview_status_t;

/**
 * @brief Event data on node removed from the network
 *
 */
typedef struct {
  zigbee_eui64_t eui64;
} zigpc_net_mgmt_on_node_removed_t;

/**
 * @brief Register observer to be informed of Net Mgmt updates
 * If the observer is already registered, nothing will be done.
 *
 * @param event           ZigPC Net Mgmt notify event type
 * @param callback        Callback to register with event
 * @return sl_status_t    SL_STATUS_OK on success
 */
sl_status_t
  zigpc_net_mgmt_register_observer(enum zigpc_net_mgmt_notify_event event,
                                   zigpc_observer_callback_t callback);

/**
 * @brief Unregister observer from Net Mgmt updates
 * If the observer is not found, nothing will be done.
 *
 * @param event           ZigPC Net Mgmt notify event type
 * @param callback        Callback to un-register with event
 * @return sl_status_t    SL_STATUS_OK on removal
 */
sl_status_t
  zigpc_net_mgmt_unregister_observer(enum zigpc_net_mgmt_notify_event event,
                                     zigpc_observer_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif  //ZIGPC_NET_MGMT_NOTIFY_H

/** @} end zigpc_net_mgmt_notify */
