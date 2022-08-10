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
 * @defgroup zwave_controller_transport_internal Z-Wave Controller Transport internals.
 * @ingroup zwave_controller_transport
 * @brief Internal Z-Wave Controller transport related functions
 *
 * @{
 */

#ifndef ZWAVE_CONTROLLER_TRANSPORT_INTERNAL_H
#define ZWAVE_CONTROLLER_TRANSPORT_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "zwave_controller.h"
#include "zwave_tx.h"

/**
 * @brief Entry point of of frame to the transport plugin system .
 *
 * @param connection_info Information about the source and destination node if
 *   the frame, as well as information about how the frame was encapsulated
 * @param rx_options      Receive specific information
 * @param frame_data      Pointer to de-encapsulated data
 * @param frame_length    Length of data
 * @return SL_STATUS_NOT_FOUND if no handler was found. Otherwise the status if
 * the executed hander list returned
 */
sl_status_t zwave_controller_transport_on_frame_received(
    const zwave_controller_connection_info_t *connection_info,
    const zwave_rx_receive_options_t *rx_options,
    const uint8_t *frame_data,
    uint16_t frame_length);


#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_CONTROLLER_TRANSPORT_INTERNAL_H
/** @} end zwave_controller_transport_internal */
