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
 * @defgroup zwave_s0_inclusion_state_machine Security 0 Inclusion state machine
 * @ingroup zwave_s0_transport
 * @brief Interface to Z-Wave S0 Inclusion State Machine.
 *
 * @startuml
   top to bottom direction
   scale 2200 width

   rectangle  S0_INC_IDLE #aliceblue;line:blue;line.bold;text:blue
   together {
   rectangle  S0_AWAITING_SCHEME_REPORT #aliceblue;line:blue;line.bold;text:blue
   rectangle  S0_HANDLE_SCHEME_REPORT #aliceblue;line:blue;line.bold;text:blue
   rectangle  S0_AWAITING_KEY_VERIFY #aliceblue;line:blue;line.bold;text:blue
   rectangle  S0_HANDLE_NET_KEY_VERIFY #aliceblue;line:blue;line.bold;text:blue
   rectangle  S0_AWAITING_2SCHEME_REPORT #aliceblue;line:blue;line.bold;text:blue
   }

     S0_INC_IDLE -down-> S0_AWAITING_SCHEME_REPORT
   note on link
   EV_S0_BOOTSTRAPPING_START
   S0_SEND_SCHEME_GET_ACTION
   s0_send_scheme_get()
   end note

     S0_AWAITING_SCHEME_REPORT --> S0_INC_IDLE
   note on link
   EV_S0_BOOTSTRAP_ABORT
   S0_BOOTSTRAP_ABORT_ACTION
   s0_abort_bootstrap()
   end note

     S0_AWAITING_SCHEME_REPORT --> S0_INC_IDLE
   note on link
   EV_S0_TIMEOUT
   S0_BOOTSTRAP_ABORT_ACTION
   s0_abort_bootstrap()
   end note

     S0_AWAITING_SCHEME_REPORT --> S0_HANDLE_SCHEME_REPORT
   note on link
   EV_S0_SCHEME_REPORT_RECEIVED
   S0_SCHEME_REPORT_RECV_ACTION
   s0_handle_net_key_verify()
   end note

     S0_HANDLE_SCHEME_REPORT --> S0_INC_IDLE
   note on link
   EV_S0_WRONG_SCHEME_REPORT
   S0_BOOTSTRAP_ABORT_ACTION
   s0_abort_bootstrap()
   end note

     S0_HANDLE_SCHEME_REPORT --> S0_AWAITING_KEY_VERIFY
   note on link
   EV_S0_SEND_KEY_SET
   S0_SEND_KEY_SET_ACTION
   s0_send_key_set()
   end note

     S0_AWAITING_KEY_VERIFY --> S0_INC_IDLE
   note on link
   EV_S0_TIMEOUT
   S0_BOOTSTRAP_ABORT_ACTION
   s0_abort_bootstrap()
   end note

     S0_AWAITING_KEY_VERIFY --> S0_HANDLE_NET_KEY_VERIFY
   note on link
   S0_HANDLE_NET_KEY_VERIFY
   S0_NET_KEY_VERIFY_RECEIVED_ACTION
   s0_handle_net_key_verify()
   end note

     S0_AWAITING_KEY_VERIFY --> S0_INC_IDLE
   note on link
   EV_S0_BOOTSTRAP_ABORT
   S0_BOOTSTRAP_ABORT_ACTION
   s0_abort_bootstrap()
   end note

     S0_HANDLE_NET_KEY_VERIFY --> S0_INC_IDLE
   note on link
   EV_S0_WAS_NOT_CONTROLLER
   S0_NO_ACTION
   s0_abort_bootstrap()
   end note

     S0_HANDLE_NET_KEY_VERIFY --> S0_AWAITING_2SCHEME_REPORT
   note on link
   EV_S0_SEND_SCHEME_INHERIT
   s0_send_scheme_inherit()
   end note

     S0_AWAITING_2SCHEME_REPORT --> S0_INC_IDLE
   note on link
   EV_S0_SCHEME_REPORT_RECEIVED
   S0_SEND_SCHEME_INHERIT_ACTION
   callback to NM
   end note

     S0_AWAITING_2SCHEME_REPORT --> S0_INC_IDLE
   note on link
   EV_S0_TIMEOUT
   S0_BOOTSTRAP_ABORT_ACTION
   s0_abort_bootstrap()
   end note


     S0_AWAITING_2SCHEME_REPORT --> S0_INC_IDLE
   note on link
   EV_S0_BOOTSTRAP_ABORT
   S0_BOOTSTRAP_ABORT_ACTION
   s0_abort_bootstrap()
   end note
  @enduml
 *
 * @{
 */

#ifndef ZWAVE_S0_SM_H
#define ZWAVE_S0_SM_H

#include "zwave_s0_network.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef enum {
  S0_INC_IDLE,
  S0_AWAITING_SCHEME_REPORT,
  S0_HANDLE_SCHEME_REPORT,
  S0_AWAITING_KEY_VERIFY,
  S0_HANDLE_NET_KEY_VERIFY,
  S0_AWAITING_2SCHEME_REPORT
}s0_bootstrap_state_t;

/**
 * @brief Start the S0 add node process.
 *
 * Calling this function will make the S0 FSM start the bootstrap process of a
 * newly added node. This must be called right after ADD_NODE_DONE event for
 * nodes supporting COMMAND_CLASS_SECURITY
 *
 * @param conn zwave_controller_connection_info_t structure
 * @param is_controller If the node being included controller. To do extra S0
 *                      boot strapping steps for controllers
 */

void s0_bootstrapping_start(const zwave_controller_connection_info_t *conn,
                            uint8_t is_controller);

/**
 * @brief Stop the S0 bootstrapping timers
 * 
 */
void s0_bootstrapping_stop(void);

/**
 * @brief This function is used to notify the S0 Inclusion state machine that
 * S0 Scheme report frame was received.
 */
void s0_scheme_report_received(uint8_t scheme, zwave_node_id_t from_node);
/**
 * @brief This function is used to notify the S0 Inclusion state machine that
 * S0 network key verify frame was received.
 */
void s0_network_key_verify_received(zwave_node_id_t from_node);

/**
 * @brief Set callback function for the S0 bootstrap. This callback function
 * is copied internally into S0 and called when S0 bootstrap is complete
 */
void s0_set_callback(s0_on_bootstrapping_complete_cb cb);
/**
 * @brief Get the state of S0 Bootstrap state machine
 */
s0_bootstrap_state_t get_s0_sm_state();
#endif
/** @} end zwave_s0_sm*/
