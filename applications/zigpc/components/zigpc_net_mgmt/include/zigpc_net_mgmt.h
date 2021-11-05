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
 * @file zigpc_net_mgmt.h
 * @defgroup zigpc_net_mgmt ZigPC Network Management
 * @ingroup zigpc_components
 * @brief The role of the ZigPC Network Management component is to handle
 * all logic related to changing the Zigbee network, such as: network formation
 * adding nodes to the network, discovering nodes, and removing nodes.
 *
 * @{
 */

#ifndef ZIGPC_NET_MGMT_H
#define ZIGPC_NET_MGMT_H

/* From shared components */
#include "sl_status.h"

/* From Zigbee types */
#include "zigpc_common_zigbee.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Network Management FSM States.
 */
typedef enum zigpc_net_mgmt_fsm_state {
  ZIGPC_NET_MGMT_FSM_STATE_MIN_VAL = 0,
  ZIGPC_NET_MGMT_FSM_STATE_INIT,
  ZIGPC_NET_MGMT_FSM_STATE_IDLE,
  ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD,
  ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE,
  ZIGPC_NET_MGMT_FSM_STATE_NODE_INTERVIEW,

  ZIGPC_NET_MGMT_FSM_STATE_MAX_VAL,
} zigpc_net_mgmt_fsm_state_t;

/**
 * @brief Container for state change request for the ZigPC Network Management
 * FSM. Depending on the state requested, the FSM will require additional
 * information suppled (that )
 *
 */
typedef struct {
  enum zigpc_net_mgmt_fsm_state requested_state;
  bool param_eui64_filled;
  zigbee_eui64_t param_eui64;
} zigpc_net_mgmt_state_change_request_t;

/**
 * @brief Request to enable/disable permitting Zigbee devices to join the
 * network.
 *
 * @return sl_status_t          Returns if Network Management can service the
 * request. If Network management is already going through an node addition
 * through a state change, the request to disable permit joins is denied by
 * returning SL_STATUS_BUSY.
 */
sl_status_t zigpc_netmgmt_network_permit_joins(bool enable);

/**
 * @brief Request to add a new end device to the Zigbee PAN using Z3 Install Code Method.
 *
 * @param node_eui64            To be added End Device
 * @param install_code          Install code buffer
 * @param install_code_length   Install code length
 * @return sl_status_t          Returns if Network Management can service the request
 */
sl_status_t zigpc_net_mgmt_add_node(const zigbee_eui64_t node_eui64,
                                    const zigbee_install_code_t install_code,
                                    const uint8_t install_code_length);

/**
 * @brief Request to interview a node already on the network. This request
 * can only be fullfilled if the FSM is in an idle state. The FSM, if
 * successful, will transition the FSM to the "node interview" state. If the
 * node is not on the network, or if the interview process has not completed,
 * a node interview timeout will expire which will transition back the FSM to
 * the idle state.
 *
 * @param eui64                 Identifier of the node to be interviewed
 * @return sl_status_t          SL_STATUS_OK if the request has been accepted,
 * SL_STATUS_BUSY if the network is busy to service the request
 */
sl_status_t zigpc_net_mgmt_interview_node(const zigbee_eui64_t eui64);

/**
 * @brief Request to remove a node already on the network. This request
 * can only be fullfilled if the FSM is in an idle state. The request, if
 * successful, will transition the FSM to the "remove node" state. If the
 * node is not on the network, or if the interview process has not completed,
 * a node interview timeout will expire which will transition back the FSM to
 * the idle state.
 *
 * @param eui64                 Identifier of the node to be interviewed
 * @return sl_status_t          SL_STATUS_OK if the request has been accepted,
 * SL_STATUS_BUSY if the network is busy to service the request
 */
sl_status_t zigpc_net_mgmt_remove_node(const zigbee_eui64_t eui64);

/**
 * @brief Request a state change request external to Protocol Controller. This
 * request is usually sent by the Unify MQTT facing components to manually
 * transition to a particular network managment state to perform an action such
 * as add node, remove node, etc. Passing the "idle" state to this request will
 * abort any on-going requests.
 *
 * @param request       Network Managment State Change Request
 * @return sl_status_t  SL_STATUS_OK if the requested has been accepted as
 * input to the FSM, SL_STATUS_NULL_POINTER if the input is invlid, or
 * SL_STATUS_INVALID_TYPE if the requested state type is unknown.
 */
sl_status_t zigpc_net_mgmt_state_change_request(
  const zigpc_net_mgmt_state_change_request_t *request);

#ifdef __cplusplus
}
#endif

#endif /* ZIGPC_NET_MGMT_H */

/** @} end zigpc_net_mgmt */
