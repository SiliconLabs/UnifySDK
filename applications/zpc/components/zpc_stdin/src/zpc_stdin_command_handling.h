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
 * @file zpc_stdin_command_handling.h
 * @brief TODO: Write brief for zpc_stdin_command_handling
 *
 * TODO: Write component description for zpc_stdin_command_handling
 *
 * @{
 */

#ifndef ZPC_STDIN_COMMAND_HANDLING_H
#define ZPC_STDIN_COMMAND_HANDLING_H

#include "sl_status.h"
#include "zwave_controller.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Adding command line interface commands to UIC process
 *
 */
void zpc_setup_cli();

// Test functionality
zwave_controller_callbacks_t get_zpc_stdin_callbacks();
#ifdef __cplusplus
}
#endif

#endif  //ZPC_STDIN_COMMAND_HANDLING_H
/** @} end zpc_stdin_command_handling */
