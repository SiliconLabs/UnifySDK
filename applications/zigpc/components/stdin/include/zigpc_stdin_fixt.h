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
 * @file zigpc_stdin_fixt.h
 * @defgroup zigpc_stdin ZigPC STDIN Interface
 * @ingroup zigpc_components
 * @brief ZigPC STDIN inteface provides addition commands that can be used to
 * interact with the ZigPC application.
 *
 * @{
 */

#ifndef ZIGPC_STDIN_FIXT_H
#define ZIGPC_STDIN_FIXT_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Setup fixture for the ZigPC STDIN.
 *
 * @return SL_STATUS_OK, always.
 */
sl_status_t zigpc_stdin_setup_fixt(void);

#ifdef __cplusplus
}
#endif

#endif  //ZIGPC_STDIN_FIXT_H

/** @} end zigpcc_stdin_fixt */
