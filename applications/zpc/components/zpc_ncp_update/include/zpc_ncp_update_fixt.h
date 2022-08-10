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
 * @defgroup zpc_ncp_update_fixt NCP Update fixture
 * @ingroup zpc_components
 *
 * @brief Componet that handles NCP updates
 *
 * This component check if a NCP firmware update has been requested from
 * the commandline and then applies the image and exit.
 *
 * @{
 */

#ifndef ZPC_NCP_UPDATE_FIXT_H
#define ZPC_NCP_UPDATE_FIXT_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief This function check if the NCP firmware should be updated
 *
 * If an update is requested the ZPC boot process is stopped, by this
 * function returning SL_STATUS_BUSY
 *
 * @return sl_status_t
 */
sl_status_t zpc_ncp_update_fixt_setup();

#ifdef __cplusplus
}
#endif

#endif  //ZPC_NCP_UPDATE_FIXT_H
/** @} end zpc_ncp_update_fixt */
