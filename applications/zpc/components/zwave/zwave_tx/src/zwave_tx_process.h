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

#ifndef ZWAVE_TX_PROCESS_H
#define ZWAVE_TX_PROCESS_H

#include "zwave_tx.h"
#include "process.h"
/**
 * @defgroup zwave_tx_process Z-Wave TX Process
 * @ingroup zwave_tx
 * @brief Z-Wave TX process taking care of the TX Queue and interfaces with the @ref zwave_api
 *
 * The Z-Wave TX Process is responsible for looking into the
 * \ref zwave_tx_queue and perform transmission requests, monitor callbacks
 * with the @ref zwave_api. This process requires the @ref zwave_api to be initialized
 * before it will work properly. The TX Process is a small state machine
 * consisting of 3 states, depicted in the following diagram:
 *
@startuml
' Style for the diagram
!theme plain
skinparam ActivityBackgroundColor #DEDEDE
skinparam ActivityBorderColor #480509
skinparam ActivityBorderThickness 2
skinparam ActivityFontColor #000000
skinparam ActivityStartColor #797777

partition "Z-Wave TX Process" {
  (*) -->[init] ZWAVE_TX_STATE_IDLE
  if "Queue empty?" then
    -->[Yes] ZWAVE_TX_STATE_IDLE
  else
    -->[No] ZWAVE_TX_STATE_TRANSMISSION_ONGOING
  endif
}

partition "Z-Wave API" {
  --> zwapi_send_data()
}

partition "Z-Wave TX Process" {
  -->[Yes] ZWAVE_TX_STATE_BACKOFF
  if "Backoff timer expired/answer received" then
    if "Child frame?" then
      -->[Yes] ZWAVE_TX_STATE_TRANSMISSION_ONGOING
    else
      -->[No] ZWAVE_TX_STATE_IDLE
    endif
  else
    -->[wait] ZWAVE_TX_STATE_BACKOFF
endif
}
@enduml
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
 * @brief Checks after receiving a frame from a NodeID if we should resume from
 *        TX back-off.
 *
 * @param node_id    Z-Wave NodeID that sent us the frame
 */
void zwave_tx_process_inspect_received_frame(zwave_node_id_t node_id);

/**
 * @brief Aborts a transmission that has been queued but not delivered yet.
 *
 * This function is used to attempt to abort a queued or ongoing transmission.
 *
 * @param session_id the session_id of the element to abort.
 * @returns
 * - SL_STATUS_IN_PROGRESS if the element was sent to the @ref zwave_api and
 *                         cancellation was requested but pending
 *                         @ref zwave_api to return a callback.
 * - SL_STATUS_OK          if the element was removed from the queue.
 * - SL_STATUS_NOT_FOUND   if the element identified by session_id does not
 *                         exist in the queue.
 */
sl_status_t
  zwave_tx_process_abort_transmission(zwave_tx_session_id_t session_id);

/**
 * @brief Verifies if we are trying to flush the queue or keep it empty.
 *
 * @returns boolean value.
 */
bool zwave_tx_process_queue_flush_is_ongoing();

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
 * @brief Initiates a flush of the Tx Queue for a reset operation.
 *
 * Tx Queue will be closed for new elements until calling
 * zwave_tx_process_open_tx_queue();
 */
sl_status_t zwave_tx_process_flush_queue_reset_step();

/**
 * @brief (Re-)opens the Z-Wave Tx Queue to accept frames.
 */
void zwave_tx_process_open_tx_queue();

#ifdef __cplusplus
}
#endif

/** @} end of zwave_tx_process */

#endif  // ZWAVE_TX_PROCESS_H
