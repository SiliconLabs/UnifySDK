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

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "zwapi_session.h"
#include "zwapi_connection.h"
#include "zwapi_timestamp.h"
#include "sl_log.h"

#define LOG_TAG "zwapi_session"

#define TIMEOUT_TIME 1600
// This number should be higher than the MAX_TRANSMISSION_RETRIES
#define MAX_RX_QUEUE_LENGTH 30
// CLEANUP: MAX_TRANSMISSION_RETRIES used to be 20 in send_frame()
// Find out what our strategy should be, INS12350 says it should be 3
#define MAX_TRANSMISSION_RETRIES           20
#define MAX_RX_FRAMES_WAITING_FOR_RESPONSE 3
#define MAX_TX_TIMEOUTS                    3

typedef struct list_elem {
  struct list_elem *next;
  uint8_t *data;
  uint8_t len;
} zwapi_session_list_elem_t;

static zwapi_session_list_elem_t *zwapi_session_rx_queue;

// Previously named rxQueue_Len in the legacy SerialAPI module
/**
 Give the number of outstanding packets in zwapi_session_rx_queue
*****************************************************************************/
static int zwapi_session_get_rx_queue_length(void)
{
  zwapi_session_list_elem_t *q = zwapi_session_rx_queue;
  int len                      = 0;

  while (q) {
    q = q->next;
    len++;
  }
  return len;
}

// Previously named QueueFrame in the legacy SerialAPI module
/**
* Enqueue a new element in the zwapi_session_rx_queue from the zwapi_connection
* buffer. Only REQUEST type frames will be enqueued, others will be ignored
* TODO: Get rid of malloc
*****************************************************************************/
static void zwapi_session_enqueue_frame(void)
{
  zwapi_session_list_elem_t *new_elem;
  if (zwapi_session_get_rx_queue_length() >= MAX_RX_QUEUE_LENGTH) {
    sl_log_error(LOG_TAG, "Rx queue is full! Aborting\n");
    return;
  }
  new_elem
    = (zwapi_session_list_elem_t *)malloc(sizeof(zwapi_session_list_elem_t));
  new_elem->data = malloc(FRAME_LENGTH_MAX * sizeof(uint8_t));
  new_elem->next = NULL;
  new_elem->len
    = zwapi_connection_get_last_rx_frame(new_elem->data, FRAME_LENGTH_MAX);

  // Verify that this is a REQUEST frame (RESPONSES are not queued here)
  if (new_elem->len >= IDX_TYPE + 1) {
    if (new_elem->data[IDX_TYPE] == FRAME_TYPE_REQUEST) {
      // Add frame to end of queue
      if (zwapi_session_rx_queue != NULL) {
        zwapi_session_list_elem_t *e;
        for (e = zwapi_session_rx_queue; e->next; e = e->next) {
          // Nothing here. Just iterating to the end of the queue.
        }
        e->next = new_elem;
      } else {
        zwapi_session_rx_queue = new_elem;
      }
      // Make an additional poll request to ensure
      // that are no pending frames unread.
      if (callbacks->poll_request) {
        callbacks->poll_request();
      }
    } else {  // it is a RESPONSE (or other?) frame type, do not enqueue
      free(new_elem->data);
      free(new_elem);
    }
  } else {  // Frame is too short, this should never happen
    free(new_elem->data);
    free(new_elem);
  }
}

// Previously named WaitResponse in the legacy SerialAPI module
/**
Wait for a acknowledgement (ACK, NAK, CAN) or possibly another SOF (REQ/RES)
frame from the Z-Wave serial device.
*****************************************************************************/
static zwapi_connection_status_t zwapi_session_wait_for_response(void)
{
  zwapi_connection_status_t connection_status;
  static zwapi_timestamp_t session_timer;
  zwapi_timestamp_get(&session_timer, TIMEOUT_TIME);
  while (1) {
    connection_status = zwapi_connection_refresh();
    if (connection_status != ZWAPI_CONNECTION_STATUS_IDLE) {
      break;
    }

    if (zwapi_is_timestamp_elapsed(&session_timer)) {
      sl_log_warning(LOG_TAG, "Timed out waiting for a response\n");
      connection_status = ZWAPI_CONNECTION_STATUS_RX_TIMEOUT;
      break;
    }
  }
  return connection_status;
}

int zwapi_session_init(const char *serial_port)
{
  return zwapi_connection_init(serial_port);
}

void zwapi_session_shutdown()
{
  zwapi_session_flush_queue();
  zwapi_connection_shutdown();
}

int zwapi_session_restart()
{
  return zwapi_connection_restart();
}

bool zwapi_session_dequeue_frame(uint8_t **frame_ptr, uint8_t *frame_len)
{
  if (zwapi_session_rx_queue) {
    zwapi_session_list_elem_t *elem = zwapi_session_rx_queue;
    zwapi_session_rx_queue          = zwapi_session_rx_queue->next;

    *frame_ptr = elem->data;
    *frame_len = elem->len;
    free(elem);

  } else {
    *frame_ptr = NULL;
    *frame_len = 0;
  }

  // Are there more elements in our session queue?
  return (zwapi_session_rx_queue) ? true : false;
}

void zwapi_session_enqueue_rx_frames()
{
  while (zwapi_connection_refresh() == ZWAPI_CONNECTION_STATUS_FRAME_RECEIVED) {
    // Enqueue available REQ frames.
    zwapi_session_enqueue_frame();
  }
}

sl_status_t zwapi_session_send_frame(uint8_t command,
                                     const uint8_t *payload_buffer,
                                     uint8_t payload_buffer_length)
{
  // First check for incoming frames
  zwapi_session_enqueue_rx_frames();

  // Then attempt to transmit our frame
  uint8_t consecutive_tx_timeout_count = 0;
  for (int i = 0; i < MAX_TRANSMISSION_RETRIES; i++) {
    zwapi_connection_tx(command,
                        FRAME_TYPE_REQUEST,
                        payload_buffer,
                        payload_buffer_length,
                        true);
    zwapi_connection_status_t connection_status
      = zwapi_session_wait_for_response();
    switch (connection_status) {
      case ZWAPI_CONNECTION_STATUS_FRAME_SENT:
        return SL_STATUS_OK;

      case ZWAPI_CONNECTION_STATUS_FRAME_RECEIVED:
        consecutive_tx_timeout_count = 0;
        zwapi_session_enqueue_frame();
        sl_log_debug(LOG_TAG, "Received a frame while trying to send\n");
        /* If we received a frame here then we were both sending. The embedded target will have
          * queued a CAN at this point, since we have been sending a frame to the uart buffer.
          * before ACK'ing the received frame.
          */
        break;

      case ZWAPI_CONNECTION_STATUS_TX_CAN:
        sl_log_debug(LOG_TAG, "Frame collision detected.\n");
        consecutive_tx_timeout_count = 0;
        break;

      case ZWAPI_CONNECTION_STATUS_TX_TIMEOUT:
        // Follow up on how many consecutive TX timeout we had.
        consecutive_tx_timeout_count++;
        sl_log_warning(LOG_TAG, "Timed out waiting for ACK frame\n");
        if (consecutive_tx_timeout_count >= MAX_TX_TIMEOUTS) {
          // We should restart the serial port
          sl_log_warning(LOG_TAG, "Reopening serial port\n");
          zwapi_connection_restart();
          return SL_STATUS_FAIL;
        }
        break;

      case ZWAPI_CONNECTION_STATUS_RX_TIMEOUT:
        // Nothing special to do here, we are missing an Ack.
        // Try again without back-off
        continue;

      case ZWAPI_CONNECTION_STATUS_TX_NAK:
        // The other end is unhappy about our frame.
        // Parsing went off the rails for them
        return SL_STATUS_FAIL;

      default:
        sl_log_error(LOG_TAG,
                     "Unknown Z-Wave API connection state: %d. Ignoring\n",
                     connection_status);
        break;
    }

    sl_log_debug(LOG_TAG,
                 "Retransmission %d/%d of 0x%02x\n",
                 i + 1,
                 MAX_TRANSMISSION_RETRIES,
                 command);

    /*TODO consider to use an exponential backoff, and do not backoff until our own framehandler is idle. Also
      * the magnitude of the backoff seem very large... this is to be analyzed. */
    // CLEANUP: INS12350 says it should be 100 + i * 1000. But this is probably not doable here.
    static zwapi_timestamp_t session_timer;
    zwapi_timestamp_get(&session_timer, 20);
    while (!zwapi_is_timestamp_elapsed(&session_timer)) {
      zwapi_session_enqueue_rx_frames();
    }
  }

  sl_log_error(LOG_TAG, "All attempts to transmit a frame have failed\n");
  return SL_STATUS_FAIL;
}

sl_status_t
  zwapi_session_send_frame_with_response(uint8_t command,
                                         const uint8_t *payload_buffer,
                                         uint8_t payload_buffer_length,
                                         uint8_t *response_buf,
                                         uint8_t *response_len)
{
  // zwapi_session_send_frame() takes care of retries and receiving the ACK
  sl_status_t send_frame_status
    = zwapi_session_send_frame(command, payload_buffer, payload_buffer_length);

  // Now start waiting for the RES frame
  if (send_frame_status == SL_STATUS_OK) {
    for (int i = 0; i < MAX_RX_FRAMES_WAITING_FOR_RESPONSE; i++) {
      zwapi_connection_status_t connection_status
        = zwapi_session_wait_for_response();
      if (connection_status == ZWAPI_CONNECTION_STATUS_FRAME_RECEIVED) {
        // Here we need to retrieve the frame to parse it
        // before we can decide what to do.
        uint8_t tmp_buffer[FRAME_LENGTH_MAX];
        int tmp_buffer_length = 0;
        tmp_buffer_length
          = zwapi_connection_get_last_rx_frame(tmp_buffer, FRAME_LENGTH_MAX);

        if (tmp_buffer_length >= FRAME_LENGTH_MIN) {
          if (tmp_buffer[IDX_TYPE] == FRAME_TYPE_RESPONSE) {
            if (tmp_buffer[IDX_CMD] == command) {
              if (response_buf) {
                memcpy(response_buf, tmp_buffer, tmp_buffer_length);
              }
              if (response_len) {
                *response_len = tmp_buffer_length;
              }
              return SL_STATUS_OK;
            } else {
              /* This if for the case where we get a callback from another function instead of a response */
              sl_log_error(LOG_TAG,
                           "Got new RES frame for Cmd 0x%x (not 0x%x) \
                          while sending %d\n",
                           tmp_buffer[IDX_CMD],
                           command,
                           i);
            }
          } else {  // Did we receive a REQ frame instead ? Then just enqueue it
            zwapi_session_enqueue_frame();
          }
        } else {  //Corrupt frame data ? Just ignore it.
          sl_log_error(LOG_TAG, "Received too short frame from \
                      zwapi_connection_get_last_rx_frame()! \n");
        }
      } else {
        sl_log_warning(LOG_TAG,
                       "Unexpected receive state! %s\n",
                       zwapi_connection_status_to_string(connection_status));
      }
    }
  }

  sl_log_warning(LOG_TAG, "The frame was not ACK'ed\n");
  return SL_STATUS_FAIL;
}

sl_status_t zwapi_session_send_frame_no_ack(uint8_t command,
                                            const uint8_t *payload_buffer,
                                            uint8_t payload_buffer_length)
{
  // First check for incoming frames
  zwapi_session_enqueue_rx_frames();

  // Send our command
  zwapi_connection_tx(command,
                      FRAME_TYPE_REQUEST,
                      payload_buffer,
                      payload_buffer_length,
                      false);
  return SL_STATUS_OK;
}

void zwapi_session_flush_queue(void)
{
  zwapi_session_list_elem_t *e;
  while (zwapi_session_rx_queue) {
    e                      = zwapi_session_rx_queue;
    zwapi_session_rx_queue = zwapi_session_rx_queue->next;
    free(e->data);
    free(e);
  }
}
