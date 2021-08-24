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

#include <string.h>
#include "zwapi_connection.h"
#include "zwapi_serial.h"
#include "zwapi_timestamp.h"
#include "sl_log.h"

#define SOF 0x01 /* Start Of Frame */
#define ACK 0x06 /* Acknowledge successfull frame reception */
#define NAK \
  0x15 /* Not Acknowledge successfull frame reception - please retransmit... */
#define CAN 0x18 /* Frame received (from host) was dropped - waiting for ACK */

/* Connection handler states */
#define STATE_SOF_HUNT    0
#define STATE_PARSE_FRAME 1

#define LOG_TAG "zwapi_connection"

// Our private frame rx buffer. Used to store the last received frame
static uint8_t rx_buffer[FRAME_LENGTH_MAX];
static uint8_t rx_buffer_length;

static uint8_t zwapi_connection_state;
static zwapi_timestamp_t timeOutACK;
static zwapi_timestamp_t timeOutRX;

// Boolean variable indicating if an acknowledgement is required
static bool ack_nak_needed = false;

static const char *zwapi_frame_to_string(const uint8_t *buffer,
                                         uint16_t buffer_length)
{
  static char message[1000] = {'\0'};
  uint16_t index            = 0;
  for (uint16_t i = 0; i < buffer_length; i++) {
    if (i == 0) {
      // Don't log the SOF byte.
      continue;
    } else if (i == 1) {
      index += snprintf(message + index, sizeof(message) - index, "Length=");
    } else if (i == 2) {
      index += snprintf(message + index, sizeof(message) - index, "Type=");
    } else if (i == 3) {
      index += snprintf(message + index, sizeof(message) - index, "Cmd=");
    }
    index
      += snprintf(message + index, sizeof(message) - index, "%02X ", buffer[i]);
  }
  return message;
}

int zwapi_connection_init(const char *serial_port)
{
  int rc;
  rx_buffer_length       = 0;
  zwapi_connection_state = STATE_SOF_HUNT;
  ack_nak_needed         = false;

  zwapi_timestamp_get(&timeOutACK, RX_ACK_TIMEOUT_DEFAULT);
  zwapi_timestamp_get(&timeOutRX, RX_BYTE_TIMEOUT_DEFAULT);
  rc = zwapi_serial_init(serial_port);
  if (rc > 0) {
    // Send ACK in case that the target expects one.
    zwapi_serial_put_byte(ACK);
    zwapi_serial_drain_buffer();
  }
  return rc;
}

void zwapi_connection_shutdown()
{
  zwapi_serial_close();
}

int zwapi_connection_restart()
{
  rx_buffer_length       = 0;
  zwapi_connection_state = STATE_SOF_HUNT;
  ack_nak_needed         = false;

  zwapi_timestamp_get(&timeOutACK, RX_ACK_TIMEOUT_DEFAULT);
  zwapi_timestamp_get(&timeOutRX, RX_BYTE_TIMEOUT_DEFAULT);

  int rc;
  rc = zwapi_serial_restart();
  if (rc) {
    // INS12350 tells to send a NAK after reopening the port
    // It can also followed by a call to FUNC_ID_SERIAL_API_SOFT_RESET
    zwapi_serial_put_byte(NAK);
    zwapi_serial_drain_buffer();
  }
  return rc;
}

void zwapi_connection_tx(
  uint8_t cmd,        /* IN Command */
  uint8_t type,       /* IN frame Type to send (Response or Request) */
  const uint8_t *Buf, /* IN pointer to uint8_t buffer containing DATA to send */
  uint8_t len,        /* IN the length of DATA to transmit */
  bool ack_needed)
{
  uint8_t tx_buffer[255];
  uint8_t *c;
  c = tx_buffer;

  *c++ = SOF;
  *c++ = len + 3;
  *c++ = type;
  *c++ = cmd;

  memcpy(c, Buf, len);
  c += len;

  uint8_t tx_checksum = 0xFF;
  for (uint8_t i = 0; i < len + 3; i++) {
    tx_checksum ^= tx_buffer[i + 1];
  }
  *c++ = tx_checksum;

  zwapi_serial_put_buffer(tx_buffer, len + 5);
  zwapi_serial_drain_buffer();

  sl_log_debug(LOG_TAG,
               "Outgoing Z-Wave API frame (hex): %s\n",
               zwapi_frame_to_string(tx_buffer, len + 5));

  ack_nak_needed = ack_needed;
  if (ack_nak_needed) {
    zwapi_timestamp_get(&timeOutACK, RX_ACK_TIMEOUT_DEFAULT);
  }
}

zwapi_connection_status_t zwapi_connection_refresh()
{
  uint8_t c                        = 0;
  bool rx_is_active                = false;
  zwapi_connection_status_t retVal = ZWAPI_CONNECTION_STATUS_IDLE;

  while (zwapi_serial_is_file_available()
         && retVal == ZWAPI_CONNECTION_STATUS_IDLE) {
    if (zwapi_serial_get_byte(&c) == 0
        && zwapi_connection_state == STATE_PARSE_FRAME) {
      // If no more data is available and we are expecting
      // to parse a frame, we mistakenly interpreted a SOF from
      // another byte.
      sl_log_warning(LOG_TAG, "Frame parsing went wrong. Aborting\n");
      break;
    }

    switch (zwapi_connection_state) {
      case STATE_SOF_HUNT:
        if (c == SOF) {
          rx_buffer_length = 0;
          rx_is_active     = true;  // now we're receiving - check for timeout
          zwapi_connection_state = STATE_PARSE_FRAME;
          zwapi_timestamp_get(&timeOutRX, RX_BYTE_TIMEOUT_DEFAULT);
        } else {
          if (ack_nak_needed) {
            if (c == ACK) {
              retVal         = ZWAPI_CONNECTION_STATUS_FRAME_SENT;
              ack_nak_needed = false;  // Done
            } else if (c == NAK) {
              retVal         = ZWAPI_CONNECTION_STATUS_TX_NAK;
              ack_nak_needed = false;
            } else if (c == CAN) {
              retVal         = ZWAPI_CONNECTION_STATUS_TX_CAN;
              ack_nak_needed = false;
            } else {
              sl_log_info(
                LOG_TAG,
                "Frame parsing received unexpected value. Ignoring\n");
            }
          }
        }
        break;

      case STATE_PARSE_FRAME:
        if ((c < FRAME_LENGTH_MIN) || (c > FRAME_LENGTH_MAX)) {
          zwapi_connection_state = STATE_SOF_HUNT;  // Restart looking for SOF
          rx_is_active           = false;           // Not really active now...
          break;
        }
        // Copy the data in our rx buffer, starting an appended length byte
        rx_buffer[0] = c;
        rx_buffer_length
          = c
            + 1;  // The initial length byte does not include the checksum field.
        int bytes_read
          = zwapi_serial_get_buffer(rx_buffer + 1, rx_buffer_length - 1);
        if (bytes_read < rx_buffer_length - 1) {
          zwapi_connection_state = STATE_SOF_HUNT;
          rx_is_active           = false;
          break;
        }

        uint8_t rx_checksum = 0xFF;
        //printf("Buffer content: ");
        for (uint8_t i = 0; i < rx_buffer_length; i++) {
          rx_checksum ^= rx_buffer[i];
          //printf("%d ",rx_buffer[i]);
        }

        if (rx_checksum == 0) {
          zwapi_serial_put_byte(ACK);
          zwapi_serial_drain_buffer();
          // Remove the checksum byte since it is validated
          rx_buffer_length -= 1;
          retVal = ZWAPI_CONNECTION_STATUS_FRAME_RECEIVED;
        } else {
          zwapi_serial_put_byte(NAK);  // Tell them something is wrong...
          zwapi_serial_drain_buffer();
          // Keep the erroneeous frame in the rx_buffer.
          retVal = ZWAPI_CONNECTION_STATUS_CHECKSUM_ERROR;
          sl_log_error(LOG_TAG,
                       "Frame received with a checksum error, and the frame is "
                       "discarded\n");
        }
        // no break

      default:
        zwapi_connection_state = STATE_SOF_HUNT;  // Restart looking for SOF
        rx_is_active           = false;           // Not really active now...
        break;
    }

    /* Check for timeouts - if no other events detected */
    if (retVal == ZWAPI_CONNECTION_STATUS_IDLE) {
      /* Are in the middle of collecting a frame and have we timed out? */
      if (rx_is_active && zwapi_is_timestamp_elapsed(&timeOutRX)) {
        /* Reset to SOF hunting */
        zwapi_connection_state = STATE_SOF_HUNT;
        rx_is_active           = false; /* Not inframe anymore */
        retVal                 = ZWAPI_CONNECTION_STATUS_RX_TIMEOUT;
      }
      /* Are we waiting for ACK and have we timed out? */
      if (ack_nak_needed && zwapi_is_timestamp_elapsed(&timeOutACK)) {
        /* Not waiting for ACK anymore */
        ack_nak_needed = false;
        /* Tell upper layer we could not get the frame through */
        retVal = ZWAPI_CONNECTION_STATUS_TX_TIMEOUT;
      }
    }
  }
  return retVal;
}

int zwapi_connection_get_last_rx_frame(uint8_t *user_buffer,
                                       int user_buffer_length)
{
  if (rx_buffer_length <= user_buffer_length) {
    memcpy(user_buffer, rx_buffer, rx_buffer_length);
    return rx_buffer_length;
  } else {
    // If the user buffer is not large enough, we clip the data
    memcpy(user_buffer, rx_buffer, user_buffer_length);
    return user_buffer_length;
  }
}

const char *zwapi_connection_status_to_string(zwapi_connection_status_t t)
{
  switch (t) {
    case ZWAPI_CONNECTION_STATUS_IDLE:
      return "Connection status: idle (nothing happened)";
    case ZWAPI_CONNECTION_STATUS_FRAME_RECEIVED:
      return "Connection status: Frame received";
    case ZWAPI_CONNECTION_STATUS_FRAME_SENT:
      return "Connection status: Frame sent";
    case ZWAPI_CONNECTION_STATUS_CHECKSUM_ERROR:
      return "Connection status: Frame checksum error";
    case ZWAPI_CONNECTION_STATUS_RX_TIMEOUT:
      return "Connection status: Rx timeout";
    case ZWAPI_CONNECTION_STATUS_TX_TIMEOUT:
      return "Connection status: Tx timeout";
    case ZWAPI_CONNECTION_STATUS_TX_NAK:
      return "Connection status: NAK received";
    case ZWAPI_CONNECTION_STATUS_TX_CAN:
      return "Connection status: CAN received";
  }
  return NULL;
}
