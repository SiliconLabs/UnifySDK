/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file
 * @brief NCP update module for AoXPC
 */

#ifndef AOXPC_NCP_UPDATE_FIXT_H
#define AOXPC_NCP_UPDATE_FIXT_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief NCP update entry point
 * 
 * Check if NCP firmware update has been requested from command line, apply the
 * image and exit.
 * 
 * @retval SL_STATUS_ABORT Update was requested, stop boot process.
 */
sl_status_t aoxpc_ncp_update_fixt_setup();

#ifdef __cplusplus
}
#endif

#endif  //AOXPC_NCP_UPDATE_FIXT_H
