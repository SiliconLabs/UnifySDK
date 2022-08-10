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
 * @file network_monitor_fixt.h
 * @brief Fixtures for network monitor
 *
 * @{
 */

#ifndef NETWORK_MONITOR_FIXT_H
#define NETWORK_MONITOR_FIXT_H

#include <stdbool.h>
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Setup fixture for the network monitor.
 *
 * Starts the network monitor Contiki process.
 *
 * @return SL_STATUS_OK, always.
 */
sl_status_t network_monitor_setup_fixt(void);

#ifdef __cplusplus
}
#endif

#endif  //NETWORK_MONITOR_FIXT_H
/** @} end network_monitor_fixt */

