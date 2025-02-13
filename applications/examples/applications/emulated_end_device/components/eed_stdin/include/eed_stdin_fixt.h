/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file eed_stdin_fixt.h
 * @brief EED STDIN fixtures interface
 *
 * @{
 */

#ifndef EED_STDIN_FIXT_H
#define EED_STDIN_FIXT_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Setup fixture for the EED STDIN.
 *
 * Starts the EED STDIN Contiki process.
 *
 * @return SL_STATUS_OK, always.
 */
sl_status_t eed_stdin_setup_fixt(void);

#ifdef __cplusplus
}
#endif

#endif  //EED_STDIN_FIXT_H
        /** @} end eed_stdin_fixt */
