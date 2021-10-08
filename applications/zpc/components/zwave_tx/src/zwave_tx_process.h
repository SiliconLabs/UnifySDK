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

#ifndef ZWAVE_TX_PROCESS_H
#define ZWAVE_TX_PROCESS_H

#include "zwave_tx.h"
#include "process.h"

/**
 * @defgroup zwave_tx_process Z-Wave TX Process
 * @ingroup zwave_tx
 * @brief Z-Wave TX process taking care of the TX Queue and interfaces with the \ref ZWAPI
 *
 * The Z-Wave TX Process is responsible for looking into the
 * \ref zwave_tx_queue and perform transmission requests, monitor callbacks
 * with the \ref ZWAPI. This process requires the \ref ZWAPI to be initialized
 * before it will work properly. The TX Process is a small state machine
 * consisting of 3 states:
 * \image html zwave_tx_process.png "Z-Wave TX Process state machine"
 *
 * The \ref zwave_tx_process must be started after the \ref zwave_rx_process
 * @{
 */

/**
 * @brief Name the of \ref contiki process for the Z-Wave TX.
 *
 * This is used to register the name of the Z-Wave TX Process.
 */
PROCESS_NAME(zwave_tx_process);

/**
 * @brief Event definitions for the Z-Wave TX Process.
 */
typedef enum {
  /// Send the next message in the TX Queue.
  ZWAVE_TX_SEND_NEXT_MESSAGE,
  /// The ongoing transmission is now completed.
  ZWAVE_TX_SEND_OPERATION_COMPLETE,
  /// A frame has been received.
  ZWAVE_TX_FRAME_RECEIVED,
} zwave_tx_events_t;

/**
 * @brief The Z-Wave TX Process states.
 */
typedef enum zwave_tx_state {
  /// Z-Wave TX is idle and new transmissions to Z-Wave nodes
  /// can be initiated.
  ZWAVE_TX_STATE_IDLE,
  /// Z-Wave TX has passed on the message to the Z-Wave module
  /// and waits for a callback.
  ZWAVE_TX_STATE_TRANSMISSION_ONGOING,
  /// Z-Wave TX waits after a transmission that requires a response
  /// to minimize the risk of radio transmit collisions.
  ZWAVE_TX_STATE_BACKOFF,
} zwave_tx_state_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Aborts a transmission that has been queued but not delivered yet.
 *
 * This function is used to attempt to abort a queued or ongoing transmission.
 *
 * @param session_id the session_id of the element to abort.
 * @returns
 * - SL_STATUS_IN_PROGRESS if the element was sent to the \ref ZWAPI and
 *                         cancellation was requested but pending
 *                         \ref ZWAPI to return a callback.
 * - SL_STATUS_OK          if the element was removed from the queue.
 * - SL_STATUS_NOT_FOUND   if the element identified by session_id does not
 *                         exist in the queue.
 */
sl_status_t
  zwave_tx_process_abort_transmission(zwave_tx_session_id_t session_id);

/**
 * @brief Triggers the processing of the next frame in the queue if we are idle
 *
 * Use this function instead of posting directly a ZWAVE_TX_SEND_NEXT_MESSAGE
 * event to the Z-Wave TX Process
 */
void zwave_tx_process_check_queue();

/**
 * @brief Logs the state of the Z-Wave TX Process using sl_log
 */
void zwave_tx_process_log_state();

/**
 * @brief Initiates a flush of the Tx Queue for a reset operation
 */
sl_status_t zwave_tx_flush_queue_reset_step();

#ifdef __cplusplus
}
#endif

/** @} end of zwave_tx_process */

#endif  // ZWAVE_TX_PROCESS_H
