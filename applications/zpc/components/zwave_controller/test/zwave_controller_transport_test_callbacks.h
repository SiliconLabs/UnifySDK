
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
 * @file zwave_controller_transport_test_callbacks.h
 * @brief used in zwave_controller_transport_test
 *
 * @{
 */
#include "zwave_controller_transport.h"

#ifndef ZWAVE_CONTROLLER_TRANSPORT_TEST_CALLBACKS_H
#define ZWAVE_CONTROLLER_TRANSPORT_TEST_CALLBACKS_H

#ifdef __cplusplus
extern "C" {
#endif

sl_status_t send_data1(const zwave_controller_connection_info_t *connection,
                       uint16_t data_length,
                       const uint8_t *data,
                       const zwave_tx_options_t *tx_options,
                       const on_zwave_tx_send_data_complete_t on_send_complete,
                       void *user,
                       zwave_tx_session_id_t session);

sl_status_t
  on_frame_received1(const zwave_controller_connection_info_t *connection_info,
                     const zwave_rx_receive_options_t *rx_options,
                     const uint8_t *frame_data,
                     uint16_t frame_length);

sl_status_t send_data2(const zwave_controller_connection_info_t *connection,
                       uint16_t data_length,
                       const uint8_t *data,
                       const zwave_tx_options_t *tx_options,
                       const on_zwave_tx_send_data_complete_t on_send_complete,
                       void *user,
                       zwave_tx_session_id_t session);

sl_status_t
  on_frame_received2(const zwave_controller_connection_info_t *connection_info,
                     const zwave_rx_receive_options_t *rx_options,
                     const uint8_t *frame_data,
                     uint16_t frame_length);

void
  on_frame_received3(const zwave_controller_connection_info_t *connection_info,
                     const zwave_rx_receive_options_t *rx_options,
                     const uint8_t *frame_data,
                     uint16_t frame_length);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_CONTROLLER_TRANSPORT_TEST_CALLBACKS_H
/** @} end zwave_controller_transport_test_callbacks */
