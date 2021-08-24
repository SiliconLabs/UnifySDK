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

#ifndef ZIGPC_NET_MGMT_FSM_H
#define ZIGPC_NET_MGMT_FSM_H

/* Contiki includes */
#include "sys/clock.h"
#include "sys/etimer.h"
#include "sl_status.h"
#include "zigpc_net_mgmt.h"
#include "zigpc_common_zigbee.h"
#include "zigpc_gateway_notify.h"

/**
 * @brief Duration set for Node Add Process to complete
 *
 */
#define ZIGPC_NET_MGMT_ADD_TIMEOUT (CLOCK_SECOND * 300)

/**
 * @brief Duration set for Node Remove Process to complete
 *
 */
#define ZIGPC_NET_MGMT_REMOVE_TIMEOUT (CLOCK_SECOND * 5)

/**
 * @brief Duration set for Node Interview Process to complete
 *
 */
#define ZIGPC_NODE_INTERVIEW_TIMEOUT_MS 5000

/**
 * @brief Network Management FSM Events
 *
 */
enum zigpc_net_mgmt_fsm_event {
  ZIGPC_NET_MGMT_FSM_EVENT_MIN_VAL = 0,

  /**
   * @brief This event is received when the gateway setup and initialization
   * has succeeded
   *
   */
  ZIGPC_NET_MGMT_FSM_EVENT_INIT_COMPLETE,

  /**
   * @brief This event is received as a generic timeout event for any state,
   * marking the end of time sensitive state
   *
   */
  ZIGPC_NET_MGMT_FSM_EVENT_TIMEOUT,

  /**
   * @brief This event is received when
   *
   */
  ZIGPC_NET_MGMT_FSM_EVENT_STATE_CHANGE_REQUEST,

  /**
   * @brief This event is received when calling to add a node to the network.
   * This request can only be processed if the FSM is in an IDLE state. This
   * will cause the FSM To enter the "node interview" state.
   *
   */
  ZIGPC_NET_MGMT_FSM_EVENT_NODE_ADD_REQUEST,

  /**
   * @brief This event is received when the node has started joining the
   * network. This event will initialize a timer to trigger a timeout
   * event if the node has not completed joining the network successfully
   * (see ZIGPC_NET_MGMT_ADD_TIMEOUT).
   *
   */
  ZIGPC_NET_MGMT_FSM_EVENT_NODE_ADD_START,

  /**
   * @brief This event is received when the node has successfully joined the
   * network. If this event is not received in time (see
   * ZIGPC_NET_MGMT_ADD_TIMEOUT), a timeout event will be received.
   *
   */
  ZIGPC_NET_MGMT_FSM_EVENT_NODE_ADD_COMPLETE,

  /**
   * @brief This event is received when calling to interview a node already on
   * the network. This request can only be processed if the FSM is in an IDLE
   * state. This will cause the FSM To enter the "node interview" state.
   *
   */
  ZIGPC_NET_MGMT_FSM_EVENT_NODE_INTERVIEW_REQUEST,

  /**
   * @brief This event is received when the gateway has successfully interviewed
   * the general node information, including how many endpoint interview events
   * to listen for later (see
   * ZIGPC_NET_MGMT_FSM_EVENT_NODE_ENDPOINT_INTERVIEWED).If this event is not
   * received in time (see ZIGPC_NODE_INTERVIEW_TIMEOUT_MS), a timeout event
   * will be received.
   *
   */
  ZIGPC_NET_MGMT_FSM_EVENT_NODE_INTERVIEWED,

  /**
   * @brief This event is received when the gateway has successfully interviewed
   * endpoint information for the node (see
   * ZIGPC_NET_MGMT_FSM_EVENT_NODE_INTERVIEWED). The endpoint information is
   * compared against total number of endpoint interview events to expect. Once
   * the expected number of endpoint interview information is received, the FSM
   * will transition into the IDLE state.If this event is not received in time
   * (see ZIGPC_NODE_INTERVIEW_TIMEOUT_MS), a timeout event will be received.
   *
   */
  ZIGPC_NET_MGMT_FSM_EVENT_NODE_ENDPOINT_INTERVIEWED,
  ZIGPC_NET_MGMT_FSM_EVENT_NODE_REMOVE_REQUEST,
  ZIGPC_NET_MGMT_FSM_EVENT_NODE_REMOVE_COMPLETE,

  ZIGPC_NET_MGMT_FSM_EVENT_MAX_VAL,
};

typedef struct {
  zigbee_eui64_t zigpc_eui64;
  zigbee_panid_t zigpc_panid;
  zigbee_ext_panid_t zigpc_ext_panid;
  zigbee_radio_channel_t zigpc_radio_channel;
} zigpc_net_mgmt_fsm_init_complete_t;

typedef struct {
  zigbee_eui64_t eui64;
  zigbee_install_code_t install_code;
  uint8_t install_code_length;
} zigpc_net_mgmt_fsm_node_add_t;

typedef struct {
  zigbee_eui64_t eui64;
} zigpc_net_mgmt_fsm_node_interview_t;

typedef struct {
  zigbee_eui64_t eui64;
} zigpc_net_mgmt_fsm_node_remove_t;

typedef struct {
  zigbee_eui64_t eui64;
  uint8_t endpoint_total_count;
  uint8_t endpoint_discovered_count;
} zigpc_net_mgmt_fsm_interview_t;

/**
 * @brief Finite State Machine variables
 */
struct zigpc_net_mgmt_fsm {
  enum zigpc_net_mgmt_fsm_state state;
  zigbee_eui64_t joining_eui64;
  zigpc_net_mgmt_fsm_interview_t interview;
  zigbee_eui64_t eui64_to_remove;
  zigpc_net_mgmt_state_change_request_t state_change_request;
  struct etimer timer;
};

/**
 * @brief Finite State Machine event container
 */
typedef union {
  /**
   * @brief Event data for Actions requested from FSM.
   */

  zigpc_net_mgmt_fsm_node_add_t node_add_request;
  zigpc_net_mgmt_fsm_node_interview_t node_interview_request;
  zigpc_net_mgmt_fsm_node_remove_t node_remove_request;
  zigpc_net_mgmt_state_change_request_t state_change_request;

  /**
   * @brief Event data for stimulus received from ZigPC Gateway.
   */

  zigpc_net_mgmt_fsm_init_complete_t on_net_init_complete;
  struct zigpc_gateway_on_node_add on_node_add_start;
  struct zigpc_gateway_on_node_add on_node_add_complete;
  zigpc_gateway_on_node_discovered_t on_node_discovered;
  zigpc_gateway_on_node_endpoint_discovered_t on_node_endpoint_discovered;
  zigpc_gateway_on_node_removed_t on_node_remove_complete;
} zigpc_net_mgmt_fsm_data_t;

/**
 * @brief Network Management FSM Process Data
 */
typedef struct {
  enum zigpc_net_mgmt_fsm_event fsm_event;
  zigpc_net_mgmt_fsm_data_t fsm_data;
} zigpc_net_mgmt_process_data_fsm_t;

/**
 * @brief Initialize Network Management State Machine
 *
 */
void zigpc_net_mgmt_fsm_reset(void);

/**
 * @brief Send event to Network Management State Machine
 *
 * @param event       FSM event type.
 * @param event_data  FSM event data.
 */
sl_status_t zigpc_net_mgmt_fsm_post_event(
  enum zigpc_net_mgmt_fsm_event event,
  const zigpc_net_mgmt_fsm_data_t *const event_data);

/**
 * @brief Get the current Network Management FSM state
 *
 * @return enum zigpc_net_mgmt_fsm_state
 */
enum zigpc_net_mgmt_fsm_state zigpc_net_mgmt_fsm_get_state(void);

/**
 * @brief Get reference for Network Management FSM timer
 *
 * @return enum struct etimer *
 */
struct etimer *zigpc_net_mgmt_fsm_get_timer(void);

/**
 * @brief Return Network Management State Name
 *
 * @param state
 * @return const char*
 */
const char *zigpc_net_mgmt_fsm_state_name(enum zigpc_net_mgmt_fsm_state state);

/**
 * @brief Return Network Management Event Name
 *
 * @param ev
 * @return const char*
 */
const char *zigpc_net_mgmt_fsm_event_name(enum zigpc_net_mgmt_fsm_event ev);

#endif /* ZIGPC_NET_MGMT_FSM_H */
