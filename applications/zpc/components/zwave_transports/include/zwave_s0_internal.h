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
 * @file zwave_s0_internal.h
 * @brief Functions used internally in component 
 * 
 * @{
 */

#ifndef ZWAVE_S0_INTERNAL_H
#define ZWAVE_S0_INTERNAL_H

#include "zwave_utils.h"
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

/**
 * @brief Set the S0 network key. This function is called from S0 init,
 * @param netkey: S0 Key (Class 80) read from S2 keystore
 
 */ 
void s0_set_key(const uint8_t* netkey);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_S0_INTERNAL_H
/** @} end zwave_s0_internal */
