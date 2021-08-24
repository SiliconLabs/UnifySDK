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

#ifndef ZIGPC_NET_MGMT_NOTIFY_INT_H
#define ZIGPC_NET_MGMT_NOTIFY_INT_H

#include "zigpc_common_observable.h"
#include "zigpc_net_mgmt_fsm.h"

/**
 * @brief Network Management observable instance
 *
 */
struct zigpc_observable zigpc_net_mgmt_observable;

/**
 * @brief Clear and reinitialize ZigPC Net Mgmt observable instance.
 *
 * @return sl_status_t SL_STATUS_OK on success,
 * if not there is an error in initializing observable
 */
sl_status_t zigpc_net_mgmt_reset_observable(void);

/**
 * @brief Notify network init to observers
 *
 * @param pc_eui64 EUI64 of PC Gateway
 */
void zigpc_net_mgmt_notify_network_init(const zigbee_eui64_t pc_eui64);

/**
 * @brief Clear the list of RequestedStateParameters that will be needed from
 * Unify as a response to Networkmanagement/Write action. See
 *
 */
void zigpc_net_mgmt_notify_clear_requested_parameter_list(void);

/**
  * @brief Add a parameter to the list of RequestedStateParameters that will be
  * sent from ZigPC as a response to Networkmanagement/Write action.
  *
  * @param parameter_str Requesting State Parameter as a character array
  * @return sl_status_t SL_STATUS_OK on successfully adding to the list,
  * SL_STATUS_NULL_POINTER on invalid parameter_str passed, SL_STATUS_EMPTY on
  * empty parameter string passed, or SL_STATUS_FULL if there is not enough room
  * to fit the passed parameter in the requested state parameters list.
  */
sl_status_t
  zigpc_net_mgmt_notify_add_requested_parameter(const char *parameter_str);

/**
 * @brief Notify FSM state updates to observers.
 *
 * @param new_state The new state transitioned to
 * @param next_supported_states List of next stated that can be transitioned
 * into
 * @return sl_status_t SL_STATUS_OK on success, SL_STATUS_NULL_POINTER on
 * invalid next_supported_states passed in.
 */
sl_status_t zigpc_net_mgmt_notify_state_update(
  const enum zigpc_net_mgmt_fsm_state new_state,
  const enum zigpc_net_mgmt_fsm_state *const next_supported_states);

/**
 * @brief Notify node added events to observer
 *
 * @param eui64 Added EUI64
 */
void zigpc_net_mgmt_notify_node_added(const zigbee_eui64_t eui64);

/**
 * @brief Notify node interview state (success/fail) to observers
 *
 * @param eui64 Node EUI64
 * @param success Status if the interview process has succeeded
 */
void zigpc_net_mgmt_notify_node_interview_status(const zigbee_eui64_t eui64,
                                                 bool success);

/**
 * @brief Notify node removed events to observer
 *
 * @param eui64 Removed device identifier
 */
void zigpc_net_mgmt_notify_node_removed(const zigbee_eui64_t eui64);

#endif /* ZIGPC_NET_MGMT_NOTIFY_INT_H */
