/* # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 */

#ifndef UIC_STDIN_PROCESS_H
#define UIC_STDIN_PROCESS_H

/**
 * @ingroup uic_stdin
 * @brief UIC interface to Contiki STDIN feature.
 *
 * Contains setup logic for UIC and some code that would normally be part of
 * the Contiki main loop.
 *
 * UIC STDIN makes it possible to integrate CLI commands to an existing
 * CLI system running in the UIC part. When integrating UIC in an application
 * the UIC will start a CLI by default.
 *
 * The Contiki code has been modified to remove the usual STDIN setup logic.
 *
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_status.h"

/**
 * @brief Start and setup CLI process in UIC other.
 *
 * Starts the CLI process and enables other applications
 * to interface the same CLI.
 *
 */
sl_status_t uic_stdin_setup();

/**
 * @brief Tear down the CLI.
 *
 * Tear down CLI and restore the normal terminal.
 *
 */
int uic_stdin_teardown();

#ifdef __cplusplus
}
#endif

#endif  //UIC_STDIN_PROCESS_H
        /** @} end uic_stdin_process */
