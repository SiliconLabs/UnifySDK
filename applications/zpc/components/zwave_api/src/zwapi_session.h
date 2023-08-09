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
 * @file zwapi_session.h
 * @brief Session level handling such as retries and request/response for
 * commands using the zwapi_connection.
 */

#ifndef ZWAPI_SESSION_H
#define ZWAPI_SESSION_H

#include <stdbool.h>
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/// @name Z-Wave API frames indexes used for parsing
///@{
#define IDX_LEN  0
#define IDX_TYPE 1
#define IDX_CMD  2
#define IDX_DATA 3
///@}

/// @name Z-Wave API frame types
///@{
#define FRAME_TYPE_REQUEST  0x00
#define FRAME_TYPE_RESPONSE 0x01
///@}

/**
 * @brief Initialize a zwapi_session with the Z-Wave module.
 *
 * @param serial_port String representing the serial device path
 * @returns integer file descriptor for the serial device. the value will be
 * greater than 0 if successful.
 */
int zwapi_session_init(const char *serial_port);

/**
 * @brief Close the zwapi_session with the Z-Wave module.
 */
void zwapi_session_shutdown();

/**
 * @brief Closes and re-initializes the zwapi_session to the Z-Wave module.
 *
 * @returns integer file descriptor for the serial device.
 * the value will be greater than 0 if successful.
 *
 * If no access to hard reset is available, it is recommended to issue a
 * FUNC_ID_SERIAL_API_SOFT_RESET command to the Z-Wave module after restarting
 * a session, and subsequently wait 1500ms before resuming operations.
 */
int zwapi_session_restart();

// Previously named SendFrame in the legacy SerialAPI module
/**
 * @brief Send data frame to Z-Wave chip via serial port and wait for ACK.
 *
 * @param command       Z-Wave Serial API Command (zwapi_func_ids.h)
 * @param payload_buffer Byte array with serial API command parameters
 * @param payload_buffer_length Length in bytes of parameter array
 * It may take up to RX_ACK_TIMEOUT_DEFAULT ms to receive a ACK
 * from the Z-Wave module
 * @returns SL_STATUS_OK if the frame sent and Ack'ed by the module.
 * SL_STATUS_FAIL otherwise
 */
sl_status_t zwapi_session_send_frame(uint8_t command,
                                     const uint8_t *payload_buffer,
                                     uint8_t payload_buffer_length);

/**
 * @brief Send data frame to Z-Wave chip via serial API, wait for ACK and a RES
 * frame.
 *
 * @param command Z-Wave Serial API Command (zwapi_func_ids.h)
 * @param payload_buffer Byte array with serial API command parameters
 * @param payload_buffer_length Length in bytes of parameter array
 * @param response_buf  output Buffer to hold response (buffer MUST be large
 * enough to hold SERIAL_BUFFER_SIZE bytes).
 * @param response_len  ouput Length of response copied to response_buf.
 * It may take up to RX_ACK_TIMEOUT_DEFAULT ms to receive a ACK
 * It may take up to RX_BYTE_TIMEOUT_DEFAULT ms to receive a RES frame
 * @returns SL_STATUS_OK if the frame sent and Ack'ed by the module and the
 * corresponding Response frame was received. SL_STATUS_FAIL otherwise
 *
 * aka SendFrameWithResponse
 */
sl_status_t
  zwapi_session_send_frame_with_response(uint8_t command,
                                         const uint8_t *payload_buffer,
                                         uint8_t payload_buffer_length,
                                         uint8_t *response_buf,
                                         uint8_t *response_len);

/**
 * @brief Send data frame to Z-Wave chip via serial API without waiting for ACK
 *
 * @param command               Z-Wave Serial API Command (zwapi_func_ids.h)
 * @param payload_buffer        Byte array with serial API command parameters
 * @param payload_buffer_length Length in bytes of parameter array
 * @returns SL_STATUS_OK if the frame has been sent.
 *
 * Does not wait for any ACK. Can be used e.g. if the Z-Wave module is
 * expected to restart
 */
sl_status_t zwapi_session_send_frame_no_ack(uint8_t command,
                                            const uint8_t *payload_buffer,
                                            uint8_t payload_buffer_length);

/**
 * @brief Loads the received frames from the serial device into the
 * zwapi_session receive queue
 *
 * aka DrainRX
 */
void zwapi_session_enqueue_rx_frames(void);

/**
 * @brief Get the next frame from the zwapi_session receive queue
 *
 * @param frame_ptr Pointer to frame data. This is set to NULL if no frame was
 * available Must be deallocated with free() after use.
 * @param frame_len length of frame. This is set to 0 if no frame was available
 * @returns true the receive queue still contains more frames
 * @returns false the receive queue is now empty.
 *
 * aka DequeueFrame
 */
bool zwapi_session_dequeue_frame(uint8_t **frame_ptr, uint8_t *frame_len);

/**
 * @brief Flush the zwapi_session receive queue
 *
 * aka SerialFlushQueue
 */
void zwapi_session_flush_queue(void);

#ifdef __cplusplus
}
#endif

#endif  // ZWAPI_SESSION_H
