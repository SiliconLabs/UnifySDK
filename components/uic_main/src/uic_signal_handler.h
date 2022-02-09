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
 * @defgroup uic_main_signal Signal handler for Unify Main
 *
 * This handles CTRL-C signal and broadcast PROCESS_EVENT_EXIT to all Contiki
 * processes to ensure a clean shutdown.
 * @ingroup uic_main
 * @{
 */

#ifndef UIC_SIGNAL_HANDLER_H
#define UIC_SIGNAL_HANDLER_H

#include <stdbool.h>
#include "process.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Setup fixture for Unify signal handler
 *
 * @return true for success
 * @return false for failure
 */
sl_status_t uic_signal_handler_setup(void);

// Signal handler process
PROCESS_NAME(uic_signal_handler_process);

#ifdef __cplusplus
}
#endif

#endif  //UIC_SIGNAL_HANDLER_H
/** @} end uic_signal_handler */
