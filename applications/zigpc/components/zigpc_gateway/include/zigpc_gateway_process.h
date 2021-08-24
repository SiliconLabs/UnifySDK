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
 * @file zigpc_gateway_process.h
 * @addtogroup zigpc_gateway
 * @brief Set up and tear down the ZigPC gateway process.
 *
 * TODO: Write component description for zigpc_gateway_process
 *
 * @{
 */

#ifndef ZIGPC_GATEWAY_PROCESS_H
#define ZIGPC_GATEWAY_PROCESS_H

/* Contiki includes */
#include "process.h"

#include "sl_status.h"

/**
 * @brief Set up the function for the zigpc_gateway component.
 *
 * @ingroup zigpc_gateway
 *
 * @return SL_STATUS_OK if initialization is completed, SL_STATUS_FAIL otherwise.
 */
sl_status_t zigpc_gateway_process_setup(void);

/**
 * @brief Tear-down function for the zigpc_gateway component.
 *
 * @ingroup zigpc_gateway
 *
 * @returns Always returns 0.
 */
int zigpc_gateway_process_shutdown(void);

#endif  //ZIGPC_GATEWAY_PROCESS_H

/** @} end zigpc_gateway_process */
