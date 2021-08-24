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

#ifndef DEV_CLI_RUN
#define DEV_CLI_RUN

#include "process.h"
/**
 * @file dev_cli_run.h
 * @defgroup dev_cli_run Developer CLI Command Runner and Manager
 * @ingroup components
 * @brief CLI command runner and management.
 *
 * Contiki process which manages the user commands
 * and exits the application when a command broadcasts COMMANDS_DONE.
 *
 *
 * @{
 */
PROCESS_NAME(dev_cli_run_process);

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { PROCESS_COMMAND_DONE } dev_cli_run_events;

#ifdef __cplusplus
}
#endif
/** @} */

#endif  //
