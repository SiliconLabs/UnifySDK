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
 * @file zwave_transport_service.h
 * @brief Functions used internally in component 
 * 
 * @{
 */

#ifndef ZWAVE_TRANSPORT_SERVICE_H
#define ZWAVE_TRANSPORT_SERVICE_H

#include "zwave_rx.h"
#include "zwave_controller_transport_internal.h"
#ifdef __cplusplus
extern "C" {
#endif

sl_status_t zwave_transport_service_transport_init();
/**
 * @brief Sending a frame with Transport service.
 * 
 * This function will split a message bigger than maximum zwave frame size 
 * into smaller frames that can fit into z-wave frame and send it with 
 * Transport service protocol
 * 
 * @param connection        Contains information about target node.
 * @param data_length       Length of un-encrypted data.
 * @param cmd_data          un-encrypted data to send
 * @param tx_options        Transmit options. The number_of_responses field
 * determines if S0 verify delivery is going to be used
 * @param on_send_complete  Callback for the transmission complete event.
 * @param user              User pointer
 * @param parent_session_id Parent session id of this session 
 * @return sl_status_t     
 *   - SL_STATUS_OK             on success
 *   - SL_STATUS_BUSY           if a tranmission is ongoing.
 *   - SL_STATUS_WOULD_OVERFLOW if we cannot handle the frame and it should 
 *                              just be dropped.
 */
sl_status_t zwave_transport_service_send_data(
  const zwave_controller_connection_info_t *connection,
  uint16_t data_length,
  const uint8_t *cmd_data,
  const zwave_tx_options_t *tx_options,
  const on_zwave_tx_send_data_complete_t on_send_complete,
  void *user,
  zwave_tx_session_id_t parent_session_id);

/**
 * @brief Entry point of Transport service frames received from the 
 * PHY/radio stack.
 * 
 * @param connection_info Information about sender.
 * @param rx_options      Receive options
 * @param frame_data      S0 control frame data
 * @param frame_length    S0 control frame length
 * @return SL_STATUS_OK   If frame is accepted
 * @return SL_STATUS_FAIL If frame can not be processed 
 * @return SL_STATUS_NOT_SUPPORTED If frame is not Tranport service 
 * @return SL_STATUS_NOT_FOUND If command of transport service frame is unknown 
 * @return SL_STATUS_WOULD_OVERFLOW If frame size if bigger than what 
 *                                  transport service can handle 
 */
sl_status_t zwave_transport_service_on_frame_received(
  const zwave_controller_connection_info_t *connection_info,
  const zwave_rx_receive_options_t *rx_options,
  const uint8_t *frame_data,
  uint16_t frame_length);
#ifdef __cplusplus
}
#endif
/** @} end zwave_transport_service*/

#endif  //ZWAVE_TRANSPORT_SERVICE_H
