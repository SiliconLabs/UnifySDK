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
// Includes from this component
#include "zwave_tx_state_logging.h"

// Generic includes
#include <stdio.h>

// Unify Includes
#include "sl_log.h"
#define LOG_TAG "zwave_tx_state_logging"

#define STR_CASE(x) \
  case x:           \
    return #x;

const char *zwave_tx_state_name(zwave_tx_state_t state)
{
  switch (state) {
    STR_CASE(ZWAVE_TX_STATE_IDLE)
    STR_CASE(ZWAVE_TX_STATE_TRANSMISSION_ONGOING)
    STR_CASE(ZWAVE_TX_STATE_BACKOFF)
    default:
      sl_log_warning(LOG_TAG, "Unknown Tx state : %d", state);
      return "Unknown";
  }
}

const char *zwave_tx_event_name(process_event_t event)
{
  switch (event) {
    STR_CASE(ZWAVE_TX_SEND_NEXT_MESSAGE)
    STR_CASE(ZWAVE_TX_SEND_OPERATION_COMPLETE)
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
      sl_log_warning(LOG_TAG, "Unknown Tx event : %d", event);
      return "Unknown";
  }
}

const char *zwave_back_off_reason_name(zwave_tx_backoff_reason_t reason)
{
  switch (reason) {
    STR_CASE(BACKOFF_CURRENT_SESSION_ID)
    STR_CASE(BACKOFF_EXPECTED_ADDITIONAL_FRAMES)
    default:
      sl_log_warning(LOG_TAG, "Unknown Tx back-off reason : %d", reason);
      return "Unknown";
  }
}
