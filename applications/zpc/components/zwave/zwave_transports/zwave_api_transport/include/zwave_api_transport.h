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
 * @defgroup zwave_api_transport Z-Wave API Transport
 * @ingroup zwave_transports
 * @brief Transport for the Z-Wave API
 *
 * @{
 */

#ifndef ZWAVE_API_TRANSPORT_H
#define ZWAVE_API_TRANSPORT_H

// UIC includes
#include "sl_status.h"

// ZPC includes
#include "zwave_tx.h"

// Contiki
#include "etimer.h"

// Maximum frame length that the Z-Wave API can take
#define ZWAVE_API_MAXIMUM_FRAME_LENGTH 255

// Value used to indicate that our destination is not a singlecast
#define INVALID_NODE_ID 0

/// The maximum time in seconds that we will wait for
/// a \ref zwapi_send_data callback from the @ref zwave_api
#define SEND_DATA_EMERGENCY_TIMEOUT 65

// Session data for send data call
typedef struct send_data_state {
  // User Callback to invoken when transmission is completed
  on_zwave_tx_send_data_complete_t on_send_data_complete;
  // User pointer to use for the invoking the on_send_data_complete function
  void *user;
  // Are we idle or currently transmitting.
  bool transmission_ongoing;
  // Save the Parent Tx session ID to be able to abort
  zwave_tx_session_id_t parent_session_id;
  // Who's our destination?
  zwave_node_id_t remote_node_id;
  // Fast-track setting enabled or not
  bool fast_track;
  // Are we waiting for a NIF from the remote node?
  bool awaiting_nif;
  // Emergency timer, in case the Z-Wave API does not callback
  struct etimer emergency_timer;
} send_data_state_t;

/**
 * @brief Event definitions for the Z-Wave API transport Process.
 */
typedef enum {
  /// Send the next message in the TX Queue.
  ZWAVE_API_TRANSPORT_START_TIMER,
} zwave_api_transport_events_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Z-Wave API Transport
 * *
 * @returns SL_STATUS_OK if successful
 * @returns SL_STATUS_FAIL if an error occurred
 */
sl_status_t zwave_api_transport_init(void);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_API_TRANSPORT_H
/** @} end zwave_api_transport */
