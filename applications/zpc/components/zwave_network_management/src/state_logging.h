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
 * @file state_logging.h
 * @brief Logging helpers for internal nm states.
 * 
 * @{
 */
 
#ifndef STATE_LOGGING_H
#define STATE_LOGGING_H

#include "nm_state_machine.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Return the name string of a network management state
 * 
 * @param state 
 * @return const char* 
 */
const char* nm_state_name(zwave_network_management_state_t state);

/**
 * @brief Return the name string of a network management event.
 * 
 * @param ev 
 * @return const char* 
 */
const char * nm_event_name(nm_event_t ev);



#ifdef __cplusplus
}
#endif

#endif  //STATE_LOGGING_H
/** @} end state_logging */
