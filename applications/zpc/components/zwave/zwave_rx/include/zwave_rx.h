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
 * @defgroup zwave_rx Z-Wave RX
 * @ingroup zwave_controller
 * @brief Component responsible for initializing the @ref zwave_api, listening to
 * incoming frames, and dispatching them.
 *
 * This component is composed of 2 sub-parts:
 * 1. @ref zwave_rx_fixt : This part is responsible for the @ref zwave_api
 * initialization using the @ref zpc_config for parameters.
 * 2. @ref zwave_rx_process : This component is responsible for continuously polling
 * incoming messages from the Z-Wave module using a @ref contiki process.
 *
 * In case of received Z-Wave frames, they will be passed on to the
 * @ref zwave_transports component handling de-encapsulation of incoming Z-Wave
 * frames (S0, S2, Transport Service, Multi Channel, CRC16, Multi Command)
 *
 * For some other events, such as received NIF, SmartStart inclusion or node deleted
 * from the network, the @ref zwave_controller will be notified.
 *
 * Receiving a frame is illustrated in the picture below:
 * \image html zwave_rx_receiving_a_frame.png "Receiving a Z-Wave Command"
 *
 * @{
 */

#ifndef ZWAVE_RX_H
#define ZWAVE_RX_H

#include "zwave_controller_connection_info.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief Waits for the Z-Wave API to have restarted/be ready
 * after a soft reset.
 *
 * NOTE: This function will be blocking for up to a couple of seconds.
 */
void zwave_rx_wait_for_zwave_api_to_be_ready();

#ifdef __cplusplus
}
#endif
/** @} end zwave_rx */

#endif  //ZWAVE_RX_H
