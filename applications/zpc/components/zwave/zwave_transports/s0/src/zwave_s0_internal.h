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
 * @defgroup zwave_s0_internal Security 0 Internal functions
 * @ingroup zwave_s0_transport
 * @brief Internal S0 transport component functions
 *
 * @{
 */

#ifndef ZWAVE_S0_INTERNAL_H
#define ZWAVE_S0_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Abort all S0 network TX sessions.
 *
 * This function aborts all NUM_TX_SESSIONS of S0 transmit session and calls
 * the callback with TRANSMIT_COMPLETE_FAIL to notify upper layer for failure
 *
 */
void s0_abort_all_tx_sessions();

/**
 * @brief Free all S0 RX sessions
 *
 * Sets the state of all MAX_RXSESSIONS S0 receive sessions to RX_SESSION_DONE
 */
void free_all_rx_session();
/**
 * @brief Free all S0 Tx sessions
 *
 * Stops the Nonce Request timer and frees all tx sessions
 */
void free_all_tx_sessions();
/**
 * @brief Free S0 nonce block list
 *
 * Frees all NONCE_BLOCK_LIST_SIZE number of nonce blocks
 */
void free_nonce_block_list();
/**
 * @brief Reset (block_next_elem)s0 block next elem variable
 */
void reset_block_next_elem();

/**
 * @brief Reset nonce timer
 */
void reset_s0_timers();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_S0_INTERNAL_H
/** @} end zwave_s0_internal */
