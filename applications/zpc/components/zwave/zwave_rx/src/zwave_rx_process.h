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
 * @defgroup zwave_rx_process Z-Wave RX process
 * @ingroup zwave_rx
 * @brief Z-Wave RX process
 *
 * The Z-Wave RX Process is responsible for continously monitoring
 * incoming messages from the Z-Wave Module. When received, the messages
 * will be parsed by the @ref zwave_api component and dispatched to the callbacks
 * registered by the @ref zwave_rx component, and once more parsed and
 * forwarded to the relevant components
 * (e.g. @ref zwave_controller_component, @ref zwave_network_management).
 *
 * @{
 */

#ifndef ZWAVE_RX_PROCESS_H
#define ZWAVE_RX_PROCESS_H

#include "process.h"

/**
 * @brief Name the of \ref contiki process for the Z-Wave RX.
 *
 * This is used to register the name of the Z-Wave RX Process.
 */
PROCESS_NAME(zwave_rx_process);

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  // ZWAVE_RX_PROCESS_H
        /** @} end of zwave_rx_process */
