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

#include <stdio.h>
#include "zwave_tx_state_logging.h"

#define STR_CASE(x) \
  case x:           \
    return #x;

const char *zwave_tx_state_name(zwave_tx_state_t state)
{
  static char message[25];
  switch (state) {
    STR_CASE(ZWAVE_TX_STATE_IDLE)
    STR_CASE(ZWAVE_TX_STATE_TRANSMISSION_ONGOING)
    STR_CASE(ZWAVE_TX_STATE_BACKOFF)
    default:
      snprintf(message, sizeof(message), "%d", state);
      return message;
  }
}

const char *zwave_tx_event_name(process_event_t event)
{
  static char message[25];
  switch (event) {
    STR_CASE(ZWAVE_TX_SEND_NEXT_MESSAGE)
    STR_CASE(ZWAVE_TX_SEND_OPERATION_COMPLETE)
    STR_CASE(ZWAVE_TX_FRAME_RECEIVED)
    // Contiki events:
    STR_CASE(PROCESS_EVENT_NONE)
    STR_CASE(PROCESS_EVENT_INIT)
    STR_CASE(PROCESS_EVENT_POLL)
    STR_CASE(PROCESS_EVENT_EXIT)
    STR_CASE(PROCESS_EVENT_SERVICE_REMOVED)
    STR_CASE(PROCESS_EVENT_CONTINUE)
    STR_CASE(PROCESS_EVENT_MSG)
    STR_CASE(PROCESS_EVENT_EXITED)
    STR_CASE(PROCESS_EVENT_TIMER)
    STR_CASE(PROCESS_EVENT_COM)
    default:
      snprintf(message, sizeof(message), "%d", event);
      return message;
  }
}
