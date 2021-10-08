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
 * @file zigpc_ucl_fixt.h
 * @defgroup zigpc_ucl_fixt ZigPC UCL Utility Fixtures
 * @ingroup zigpc_ucl
 * @brief Setup and shutdown fixtures for ZigPC UCL utilities.
 *
 * @{
 */

#ifndef ZIGPC_UCL_FIXT_H
#define ZIGPC_UCL_FIXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_status.h"

/** @brief Setup fixture for ZigPC UCL component
 *
 * @return SL_SATUS_OK if initialization is successful, error otherwise.
 */
sl_status_t zigpc_ucl_fixt_setup(void);

/** @brief Shutdown fixture for ZigPC UCL component
 *
 * @return Return Always return 0.
 */
int zigpc_ucl_fixt_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* ZIGPC_UCL_FIXT_H */

/** @} end zigpc_ucl */
