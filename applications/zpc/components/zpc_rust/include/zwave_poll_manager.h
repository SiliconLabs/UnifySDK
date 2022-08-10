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
 * @defgroup zwave_poll_manager Z-Wave Polling Manager
 * @ingroup zpc_components
 * @brief Z-Wave attribute poll manager which reads the attribute
 *        list from yaml configuration file and register the attribute
 *        poll interval to poll engine.
 *
 * @{
 */
#include "sl_status.h"

#ifndef ZWAVE_POLL_MANAGER_H
#define ZWAVE_POLL_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the attribute poll manager
 *
 * @return sl_status_t SL_STATUS_OK on success
 */
sl_status_t zwave_poll_manager_init();

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_POLL_MANAGER_H
/** @} end zwave_poll_manager */
