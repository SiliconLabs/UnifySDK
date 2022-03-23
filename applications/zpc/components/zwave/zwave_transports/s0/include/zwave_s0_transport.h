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
 * @defgroup zwave_s0_transport Security 0 Transport
 * @ingroup zwave_transports
 * @brief Transport for Security 0
 *
 * @{
 */

#ifndef ZWAVE_S0_TRANSPORT_H
#define ZWAVE_S0_TRANSPORT_H

#include "sl_status.h"
#include "zwave_controller_transport_internal.h"
#include "zwave_rx.h"

#ifdef __cplusplus
extern "C" {
#endif
#define COMMAND_CLASS_SECURITY         0x98
#define COMMAND_CLASS_SECURITY_VERSION 1
/**
 * @brief Sending an S0 encapsulated frame.
 *
 * This function will encrypt a data payload and encapsupate it into an
 * Security 0 message.
 *
 * @param connection        Contains information about target node.
 * @param data_length       Length of un-encrypted data.
 * @param cmd_data          un-encrypted data to send
 * @param tx_options        Transmit options. The number_of_responses field
 *                        determines if S0 verify delivery is going to be used
 * @param on_send_complete  Callback for the transmission complete event.
 * @param user              User pointer
 * @param parent_session_id Parent session id of this session
 * @return sl_status_t
 *   - SL_STATUS_OK             on success
 *   - SL_STATUS_NOT_SUPPORTED  if unknown encapuslation scheme is applied.
 *   - SL_STATUS_BUSY           if a tranmission is ongoing.
 *   - SL_STATUS_WOULD_OVERFLOW if we cannot handle the frame and it should just be dropped.
 */
sl_status_t
  zwave_s0_send_data(const zwave_controller_connection_info_t *connection,
                     uint16_t data_length,
                     const uint8_t *cmd_data,
                     const zwave_tx_options_t *tx_options,
                     const on_zwave_tx_send_data_complete_t on_send_complete,
                     void *user,
                     zwave_tx_session_id_t parent_session_id);

/**
 * @brief Entry point of S0 frames received from the PHY/radio stack.
 *
 * This function is used to feed S0 frames into the S0 engine. See
 * S0_application_command_handler
 *
 * @param connection_info Information about sender.
 * @param rx_options      Receive options
 * @param frame_data      S0 control frame data
 * @param frame_length    S0 control frame length
 * @return sl_status_t
 *   - SL_STATUS_WOULD_OVERFLOW Frame length is more than ZWAVE_MAX_FRAME_SIZE
 *   - SL_STATUS_NOT_FOUND if Command class is not Security 0 or command inside
 *                         the command class is unknown
 *   - SL_STATUS_NOT_SUPPORTED If frame is not supported because of some reason
 *   - SL_STATUS_FAIL Decryption of frame failed
 *   - SL_STATUS_OK if OK
 */
sl_status_t zwave_s0_on_frame_received(
  const zwave_controller_connection_info_t *connection_info,
  const zwave_rx_receive_options_t *rx_options,
  const uint8_t *frame_data,
  uint16_t frame_length);

/**
 * @brief Initialize the S0 Transport
 * Also calls zwave_s0_network_init() which aborts all tx sessions and
 * frees them and also frees rx sessions and reads S0 key from keystore and
 * initializes the S0 key
 *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error occurred in zwave_s0_network_init()
 */
sl_status_t zwave_s0_transport_init(void);

/**
 * @brief Set the S0 network key.
 * @param network_key S0 Key (Class 80) read from S2 keystore
 */
void s0_set_key(const uint8_t *network_key);

/**
 * @brief Z-Wave controller transport callback function which
 *        will be triggered when S0 frame tranmission is aborted.
 * @param session_id The session id of the frame which has been aborted.
 *
 * @return SL_STATUS_OK to indicate that ongoing session are aborted.
 * @return SL_STATUS_NOT_FOUND to indicate that no transmission was ongoing.
 */
sl_status_t zwave_s0_on_abort_send_data(zwave_tx_session_id_t session_id);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_S0_TRANSPORT_H
        /** @} end zwave_s0_transport */