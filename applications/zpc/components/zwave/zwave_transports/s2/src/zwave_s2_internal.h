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
 * @defgroup zwave_s2_internal Z-Wave S2 internals
 * @ingroup zwave_s2_transport
 * @brief Internal functions for the S2 component
 *
 * @{
 */

#ifndef ZWAVE_S2_INTERNAL_H
#define ZWAVE_S2_INTERNAL_H

#include "S2.h"
#include "zwave_rx.h"
#include "zwave_controller_transport_internal.h"
#ifdef __cplusplus
extern "C" {
#endif

extern struct S2 *s2_ctx;

#define COMMAND_CLASS_SECURITY_2         0x9F
#define COMMAND_CLASS_SECURITY_2_VERSION 1

/**
 * @brief Sending an S2 encapsulated frame.
 *
 * This function will encrypt a data payload and encapsupate it into an
 * Security 2 message. This function wraps S2_send_data. This function can
 * only handle one transmit session at a time.
 *
 * @param connection        Contains information about target node.
 * @param data_length       Length of un-encrypted data.
 * @param cmd_data          un-encrypted data to send
 * @param tx_options        Transmit options. The number_of_responses field
 * determines if S2 verify delivery is going to be used
 * @param on_send_complete  Callback for the transmission complete event.
 * @param user              User pointer
 * @param parent_session_id Parent Session handle that gets used to duplicate tx_options.
 * @return sl_status_t
 *   - SL_STATUS_OK             on success
 *   - SL_STATUS_NOT_SUPPORTED  if unknown encapuslation scheme is applied.
 *   - SL_STATUS_BUSY           if a tranmission is ongoing.
 *   - SL_STATUS_WOULD_OVERFLOW if we cannot handle the frame and it should just be dropped.
 */
sl_status_t
  zwave_s2_send_data(const zwave_controller_connection_info_t *connection,
                     uint16_t data_length,
                     const uint8_t *cmd_data,
                     const zwave_tx_options_t *tx_options,
                     const on_zwave_tx_send_data_complete_t on_send_complete,
                     void *user,
                     zwave_tx_session_id_t parent_session_id);

/**
 * @brief Entry point of S2 frames received from the PHY/radio stack.
 *
 * This function is used to feed S2 frames into the S2 engine. See
 * S2_application_command_handler
 *
 * @param connection_info Information about sender.
 * @param rx_options      Receive options
 * @param frame_data      S2 control frame data
 * @param frame_length    S2 control frame length
 * @return true           If frame is accepted
 * @return false          If frame is not an S2 frame.
 */
sl_status_t zwave_s2_on_frame_received(
  const zwave_controller_connection_info_t *connection_info,
  const zwave_rx_receive_options_t *rx_options,
  const uint8_t *frame_data,
  uint16_t frame_length);

/**
 * @brief Asks LibS2 to erase an MPAN entry
 *
 * @param group_id        The Z-Wave Group ID that is to be re-shuffled
 */
void zwave_s2_on_on_multicast_group_deleted(zwave_multicast_group_id_t group_id);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_S2_INTERNAL_H
/** @} end zwave_s2_internal */
