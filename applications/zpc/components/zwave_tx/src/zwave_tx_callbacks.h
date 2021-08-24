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

#ifndef ZWAVE_TX_CALLBACKS_H
#define ZWAVE_TX_CALLBACKS_H

// Includes from other components
#include "zwave_controller_connection_info.h"
#include "zwapi_protocol_transport.h"
#include "zwave_rx.h"

/**
 * @defgroup zwave_tx_callbacks Z-Wave TX callbacks
 * @ingroup zwave_tx
 * @brief Z-Wave TX callbacks registered to other components.
 *
 * These functions are callbacks that are registered to other components.
 * When called, they post an event in the \ref zwave_tx_process
 *
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Callback for Z-Wave TX, for incoming Z-Wave frames.
 *
 * Refer to \ref zwave_controller_callbacks_t on_frame_received()
 * for the functions parameter descriptions.
 * This functions copies the last received frame connection_info and passes
 * it on to the \ref zwave_tx_process to process.
 */
void zwave_tx_on_frame_received(
  const zwave_controller_connection_info_t *connection_info,
  const zwave_rx_receive_options_t *rx_options,
  const uint8_t *frame_data,
  uint16_t frame_length);

/**
 * @brief Callback for Z-Wave TX, for transport encapsulation.
 */
void on_zwave_transport_send_data_complete(uint8_t status,
                                           const zwapi_tx_report_t *tx_info,
                                           void *user);

/**
 * @brief Callback function passed to the \ref ZWAPI for \ref zwapi_send_data().
 */
void zwave_tx_send_data_callback(uint8_t status, zwapi_tx_report_t *tx_status);

/**
 * @brief Callback function passed to the \ref ZWAPI for \ref zwapi_send_data_multi().
 */
void zwave_tx_send_data_multi_callback(uint8_t status);

/**
 * @brief Callback function passed to the \ref ZWAPI for \ref zwapi_send_test_frame().
 */
void zwave_tx_send_test_frame_callback(uint8_t status);

// zwave_controller callback functions
/**
 * @brief Callback function to when a Node Info Request fails.
 *
 * See \ref zwave_controller_callbacks_t for details on the callback.
 */
void zwave_tx_on_node_info_req_failed(zwave_node_id_t node_id);

/**
 * @brief Callback function to whena  Node Info Request is received.
 *
 * See \ref zwave_controller_callbacks_t for details on the callback.
 */
void zwave_tx_on_node_information(zwave_node_id_t node_id,
                                  const zwave_node_info_t *node_info);

#ifdef __cplusplus
}
#endif
/** @} end of zwave_tx_callbacks */

#endif  // ZWAVE_TX_CALLBACKS_H
