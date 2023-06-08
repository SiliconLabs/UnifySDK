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
 * @defgroup zwave_rx_callbacks Z-Wave RX callbacks
 * @ingroup zwave_rx
 * @brief Z-Wave RX Event callbacks
 *
 * Small module proxying Z-Wave API events to other components.
 *
 * @{
 */

#ifndef ZWAVE_RX_CALLBACKS_H
#define ZWAVE_RX_CALLBACKS_H

#include "sl_status.h"

/**
 * @brief A callback type to get notified of Z-Wave API restarts.
 */
typedef void (*zwave_api_started_callback_t)(void);

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief Registers a function that must be invoked whenever the Z-Wave API
 * sends a API started frame.
 */
sl_status_t zwave_rx_register_zwave_api_started_callback(
  zwave_api_started_callback_t callback);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_RX_CALLBACKS_H
/** @} end zwave_rx_callbacks */
