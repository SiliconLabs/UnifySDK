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
 * @file zpc_stdin_fixt.h
 * @brief ZPC STDIN fixtures interface
 *
 * @{
 */

#ifndef ZPC_STDIN_FIXT_H
#define ZPC_STDIN_FIXT_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Setup fixture for the ZPC STDIN.
 *
 * Starts the ZPC STDIN Contiki process.
 *
 * @return SL_STATUS_OK, always.
 */
sl_status_t zpc_stdin_setup_fixt(void);

#ifdef __cplusplus
}
#endif

#endif  //ZPC_STDIN_FIXT_H
        /** @} end zpc_stdin_fixt */
