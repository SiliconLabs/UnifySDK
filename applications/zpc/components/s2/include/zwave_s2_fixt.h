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
 * @file zwave_s2_fixt.h
 * @brief Fixture for Z-Wave S2
 *
 * Fixtures for Z-Wave S2 for startup
 *
 * @{
 */

#ifndef ZWAVE_S2_FIXT_H
#define ZWAVE_S2_FIXT_H
#include <stdbool.h>
#include "sl_status.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Setup fixture for the Z-Wave S2.
 *
 * Responsible for starting the zwave_s2_process
 *
 * @return SL_STATUS_OK Setup success
 * @return SL_STATUS_FAIL Setup failed
 */
sl_status_t zwave_s2_fixt_setup(void);
#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_S2_FIXT_H
/** @} end zwave_s2_fixt */

