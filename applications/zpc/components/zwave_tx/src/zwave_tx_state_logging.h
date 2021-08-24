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
 * @file zwave_tx_state_logging.h
 * @brief Logging helpers for \ref zwave_tx states and events.
 * 
 * @{
 */

#ifndef ZWAVE_TX_STATE_LOGGING_H
#define ZWAVE_TX_STATE_LOGGING_H

#include "zwave_tx_process.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Return the name of a Z-Wave TX state
 * 
 * @param state The state value to convert to a string
 * @return const char* 
 */
const char *zwave_tx_state_name(zwave_tx_state_t state);

/**
 * @brief Return the name of a Z-Wave TX event.
 * 
 * @param event The event to convert to a string
 * @return const char* 
 */
const char *zwave_tx_event_name(process_event_t event);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_TX_STATE_LOGGING_H
/** @} end zwave_tx_state_logging */
