/**
 * @file zwave_s0_sm.c
 * @brief S0 Bootstrapping state machine
 *
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 */

#include <ctimer.h>
#include <stddef.h>
#include <string.h>
#include "zwave_s0_sm.h"
#include "sl_log.h"
#include "ZW_classcmd.h"
#include "zwave_s0_internal.h"
#include "zwave_s0_transport.h"
#include "zwave_s0_network.h"
#include "s2_keystore.h"
#include "s2_classcmd.h"

#define LOG_TAG            "zwave_s0_sm"
#define S0_INCLUSION_TIMER 10 * CLOCK_SECOND
#define ELEM_COUNT(ARRAY)  (sizeof(ARRAY) / (sizeof(ARRAY[0])))
#define S0_KEY_LENGTH      16
#define UNUSED(x)          x = x;

#define STR_CASE(x) \
  case x:           \
    return #x;

static s0_on_bootstrapping_complete_cb s0_on_bootstrapping_complete_callback
  = NULL;

typedef enum {
  EV_S0_BOOTSTRAPPING_START,
  EV_S0_SCHEME_REPORT_RECEIVED,
  EV_S0_SEND_KEY_SET,
  EV_S0_NET_KEY_VERIFY_RECEIVED,
  EV_S0_SEND_SCHEME_INHERIT,
  EV_S0_WAS_NOT_CONTROLLER,
  EV_S0_WRONG_SCHEME_REPORT,
  EV_S0_BOOTSTRAP_ABORT,
  EV_S0_TIMEOUT
} s0_bootstrap_event_t;

typedef enum {
  S0_SEND_SCHEME_GET_ACTION,
  S0_SCHEME_REPORT_RECV_ACTION,
  S0_SEND_KEY_SET_ACTION,
  S0_NET_KEY_VERIFY_RECEIVED_ACTION,
  S0_SEND_SCHEME_INHERIT_ACTION,
  S0_2SCHEME_REPORT_RECEIVED_ACTION,
  S0_BOOTSTRAP_ABORT_ACTION,
  S0_NO_ACTION
} s0_action_t;

const char *s0_event_name(s0_bootstrap_event_t ev)
{
  switch (ev) {
    STR_CASE(EV_S0_BOOTSTRAPPING_START)
    STR_CASE(EV_S0_SCHEME_REPORT_RECEIVED)
    STR_CASE(EV_S0_SEND_KEY_SET)
    STR_CASE(EV_S0_NET_KEY_VERIFY_RECEIVED)
    STR_CASE(EV_S0_SEND_SCHEME_INHERIT)
    STR_CASE(EV_S0_WAS_NOT_CONTROLLER)
    STR_CASE(EV_S0_WRONG_SCHEME_REPORT)
    STR_CASE(EV_S0_BOOTSTRAP_ABORT)
    STR_CASE(EV_S0_TIMEOUT)
    default:
      sl_log_warning(LOG_TAG, "Unknown S0 event: %d", ev);
      return "Unknown";
  }
}

const char *s0_incl_state_name(s0_bootstrap_state_t state)
{
  switch (state) {
    STR_CASE(S0_INC_IDLE)
    STR_CASE(S0_AWAITING_SCHEME_REPORT)
    STR_CASE(S0_HANDLE_SCHEME_REPORT)
    STR_CASE(S0_AWAITING_KEY_VERIFY)
    STR_CASE(S0_HANDLE_NET_KEY_VERIFY)
    STR_CASE(S0_AWAITING_2SCHEME_REPORT)
    default:
      sl_log_warning(LOG_TAG, "Unknown S0 state: %d", state);
      return "Unknown";
  }
}

const char *s0_action_name(s0_action_t action)
{
  switch (action) {
    STR_CASE(S0_SEND_SCHEME_GET_ACTION)
    STR_CASE(S0_SCHEME_REPORT_RECV_ACTION)
    STR_CASE(S0_SEND_KEY_SET_ACTION)
    STR_CASE(S0_NET_KEY_VERIFY_RECEIVED_ACTION)
    STR_CASE(S0_SEND_SCHEME_INHERIT_ACTION)
    STR_CASE(S0_2SCHEME_REPORT_RECEIVED_ACTION)
    STR_CASE(S0_BOOTSTRAP_ABORT_ACTION)
    STR_CASE(S0_NO_ACTION)
    default:
      sl_log_warning(LOG_TAG, "Unknown S0 action: %d", action);
      return "Unknown";
  }
}

typedef struct {
  s0_bootstrap_state_t state;
  s0_bootstrap_event_t event;
  s0_action_t action;
  s0_bootstrap_state_t new_state;
} s0_transition_t;

typedef struct {
  s0_bootstrap_state_t bootstrap_state;
  zwave_controller_connection_info_t bootstrap_conn;
  struct ctimer bootstrap_timer;
  uint8_t scheme_report;
  uint8_t is_controller;
} s0_sm_t;

s0_sm_t s0_sm;

void s0_scheme_report_received(const uint8_t scheme, zwave_node_id_t from_node);
void s0_handle_net_key_verify();
static void s0_sm_process_event(uint16_t evt);

static void reset_s0_sm()
{
  sl_log_debug(LOG_TAG, "Resetting s0 state machine\n");
  ctimer_stop(&s0_sm.bootstrap_timer);
  memset(&s0_sm, 0, sizeof(s0_sm));
}

static void s0_abort_bootstrap()
{
  sl_log_debug(LOG_TAG, "Aborting S0 bootstrap.\n");
  if (s0_on_bootstrapping_complete_callback) {
    s0_on_bootstrapping_complete_callback(
      0,
      ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_KEX_SCHEME);
  }
  ctimer_stop(&s0_sm.bootstrap_timer);
  memset(&s0_sm, 0, sizeof(s0_sm));
}

static void s0_timeout(void *data)
{
  (void)data;
  sl_log_debug(LOG_TAG,
               "Timeout in response from NodeID: %d in state: %s\n",
               s0_sm.bootstrap_conn.remote.node_id,
               s0_incl_state_name(s0_sm.bootstrap_state));
  s0_sm_process_event(EV_S0_TIMEOUT);
}

static void s0_send_data_callback(uint8_t status,
                                  const zwapi_tx_report_t *tx_info,
                                  void *user)
{
  (void)tx_info;
  (void)user;
  sl_log_debug(LOG_TAG,
               "S0 Send data callback invoked with status = %d\n",
               status);
  if (status == TRANSMIT_COMPLETE_OK) {
    ctimer_set(&s0_sm.bootstrap_timer, S0_INCLUSION_TIMER, s0_timeout, 0);
  } else if (status == TRANSMIT_COMPLETE_FAIL) {
    sl_log_warning(LOG_TAG,
                   "Error in transmission to NodeID %d "
                   "while S0 bootstrapping is in %s state. "
                   "Aborting S0 bootstrapping.",
                   s0_sm.bootstrap_conn.remote.node_id,
                   s0_incl_state_name(s0_sm.bootstrap_state));
    s0_sm_process_event(EV_S0_BOOTSTRAP_ABORT);
  }
}

static sl_status_t
  s0_bootstrap_frames_generic_send(zwave_controller_encapsulation_scheme_t en,
                                   const uint8_t *frame,
                                   uint8_t frame_len)
{
  zwave_tx_options_t options         = {0};
  s0_sm.bootstrap_conn.encapsulation = en;

  options.qos_priority = ZWAVE_TX_QOS_RECOMMENDED_TIMING_CRITICAL_PRIORITY;
  options.number_of_responses = 1;
  options.discard_timeout_ms  = S0_INCLUSION_TIMER;

  sl_status_t ret = zwave_tx_send_data(&s0_sm.bootstrap_conn,
                                       frame_len,
                                       frame,
                                       &options,
                                       s0_send_data_callback,
                                       0,
                                       0);
  return ret;
}

static sl_status_t s0_send_scheme_inherit()
{
  ZW_SECURITY_SCHEME_INHERIT_FRAME inherit_frame
    = {.cmdClass                 = COMMAND_CLASS_SECURITY,
       .cmd                      = SECURITY_SCHEME_INHERIT,
       .supportedSecuritySchemes = 0};

  return s0_bootstrap_frames_generic_send(
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0,
    (uint8_t *)&inherit_frame,
    sizeof(inherit_frame));
}

static sl_status_t s0_send_key_set()
{
  uint8_t temp_key[S0_KEY_LENGTH] = {0};
  uint8_t key[S0_KEY_LENGTH]      = {0};

  s0_set_key(temp_key);
  if (keystore_network_key_read(KEY_CLASS_S0, key) == false) {
    sl_log_error(LOG_TAG,
                 "Failed to read the S0 key from the keystore. "
                 "Cannot set the network key for S0 bootstrapping.");
    return SL_STATUS_FAIL;
  }

  uint8_t buf[sizeof(ZW_NETWORK_KEY_SET_1BYTE_FRAME) + S0_KEY_LENGTH - 1] = {0};
  ZW_NETWORK_KEY_SET_1BYTE_FRAME *net_key_set_frame
    = (ZW_NETWORK_KEY_SET_1BYTE_FRAME *)buf;

  net_key_set_frame->cmdClass = COMMAND_CLASS_SECURITY;
  net_key_set_frame->cmd      = NETWORK_KEY_SET;
  memcpy(&buf[2], key, S0_KEY_LENGTH);

  return s0_bootstrap_frames_generic_send(
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0,
    buf,
    sizeof(buf));
}

static sl_status_t s0_send_scheme_get()
{
  ZW_SECURITY_SCHEME_GET_FRAME scheme_get_frame
    = {.cmdClass                 = COMMAND_CLASS_SECURITY,
       .cmd                      = SECURITY_SCHEME_GET,
       .supportedSecuritySchemes = 0};

  return s0_bootstrap_frames_generic_send(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                                          (uint8_t *)&scheme_get_frame,
                                          sizeof(scheme_get_frame));
}

static const s0_transition_t s0_transition_table[] = {
  {S0_INC_IDLE,
   EV_S0_BOOTSTRAPPING_START,
   S0_SEND_SCHEME_GET_ACTION,
   S0_AWAITING_SCHEME_REPORT},

  /* If the Bootstrapping needs to be aborted while scheme get is being sent?
   * for e.g. if scheme get sending failed
   */
  {S0_AWAITING_SCHEME_REPORT,
   EV_S0_BOOTSTRAP_ABORT,
   S0_BOOTSTRAP_ABORT_ACTION,
   S0_INC_IDLE},

  /* If the Bootstrapping needs to be aborted while timing out in getting scheme
   * report as response to scheme get
   */
  {S0_AWAITING_SCHEME_REPORT,
   EV_S0_TIMEOUT,
   S0_BOOTSTRAP_ABORT_ACTION,
   S0_INC_IDLE},

  {S0_AWAITING_SCHEME_REPORT,
   EV_S0_SCHEME_REPORT_RECEIVED,
   S0_SCHEME_REPORT_RECV_ACTION,
   S0_HANDLE_SCHEME_REPORT},

  {S0_HANDLE_SCHEME_REPORT,
   EV_S0_SEND_KEY_SET,
   S0_SEND_KEY_SET_ACTION,
   S0_AWAITING_KEY_VERIFY},

  /* If the Bootstrapping needs to be aborted while timing out in getting net
   * key verify as response to net key set
   */
  {S0_AWAITING_KEY_VERIFY,
   EV_S0_TIMEOUT,
   S0_BOOTSTRAP_ABORT_ACTION,
   S0_INC_IDLE},

  {S0_AWAITING_KEY_VERIFY,
   EV_S0_NET_KEY_VERIFY_RECEIVED,
   S0_NET_KEY_VERIFY_RECEIVED_ACTION,
   S0_HANDLE_NET_KEY_VERIFY},

  {S0_AWAITING_KEY_VERIFY,
   EV_S0_BOOTSTRAP_ABORT,
   S0_BOOTSTRAP_ABORT_ACTION,
   S0_INC_IDLE},

  /* If the Bootstrapping needs to be finished because the node being
   * bootstrapped is not a controller so no need of sending scheme inherit
   */
  {S0_HANDLE_NET_KEY_VERIFY,
   EV_S0_WAS_NOT_CONTROLLER,
   S0_NO_ACTION,
   S0_INC_IDLE},

  {S0_HANDLE_NET_KEY_VERIFY,
   EV_S0_SEND_SCHEME_INHERIT,
   S0_SEND_SCHEME_INHERIT_ACTION,
   S0_AWAITING_2SCHEME_REPORT},

  {S0_AWAITING_2SCHEME_REPORT,
   EV_S0_SCHEME_REPORT_RECEIVED,
   S0_2SCHEME_REPORT_RECEIVED_ACTION,
   S0_INC_IDLE},

  /* If the Bootstrapping needs to be aborted while timing out in getting scheme
   * report as response to scheme inherit (only if node being boostrapped is
   * controller)
   */
  {S0_AWAITING_2SCHEME_REPORT,
   EV_S0_TIMEOUT,
   S0_BOOTSTRAP_ABORT_ACTION,
   S0_INC_IDLE},

  {S0_AWAITING_2SCHEME_REPORT,
   EV_S0_BOOTSTRAP_ABORT,
   S0_BOOTSTRAP_ABORT_ACTION,
   S0_INC_IDLE},
};

static void s0_sm_process_event(uint16_t evt)
{
  s0_bootstrap_event_t event = (s0_bootstrap_event_t)evt;
  uint8_t key[S0_KEY_LENGTH];

  sl_log_debug(LOG_TAG,
               "s0_sm_process_event state: %s: event: %s\n",
               s0_incl_state_name(s0_sm.bootstrap_state),
               s0_event_name(event));
  for (int i = 0; i < ELEM_COUNT(s0_transition_table); i++) {
    if ((s0_sm.bootstrap_state == s0_transition_table[i].state)
        && (event == s0_transition_table[i].event)) {
      if (s0_sm.bootstrap_state != s0_transition_table[i].new_state) {
        s0_sm.bootstrap_state = s0_transition_table[i].new_state;
      }

      sl_log_debug(LOG_TAG,
                   "action: %s new state: %s\n",
                   s0_action_name(s0_transition_table[i].action),
                   s0_incl_state_name(s0_sm.bootstrap_state));
      switch (s0_transition_table[i].action) {
        case S0_SEND_SCHEME_GET_ACTION:
          if (SL_STATUS_OK != s0_send_scheme_get()) {
            sl_log_error(LOG_TAG,
                         "Failure in sending SCHEME GET. "
                         "Aborting S0 bootstrapping.\n");
            s0_sm_process_event(EV_S0_BOOTSTRAP_ABORT);
          }

          break;
        case S0_SCHEME_REPORT_RECV_ACTION:
          s0_sm_process_event(EV_S0_SEND_KEY_SET);
          break;
        case S0_SEND_KEY_SET_ACTION:
          if (SL_STATUS_OK != s0_send_key_set()) {
            sl_log_error(LOG_TAG,
                         "Failure in sending NETWORK KEY SET. "
                         "Aborting S0 bootstrapping.\n");
            s0_sm_process_event(EV_S0_BOOTSTRAP_ABORT);
          }
          if (keystore_network_key_read(KEY_CLASS_S0, key) == true) {
            s0_set_key(key);
          } else {
            sl_log_error(LOG_TAG,
                         "Failure in reading KEY_CLASS_S0 from keystore. "
                         "Aborting S0 bootstrapping.\n");
            s0_sm_process_event(EV_S0_BOOTSTRAP_ABORT);
          }
          break;
        case S0_NET_KEY_VERIFY_RECEIVED_ACTION:
          s0_handle_net_key_verify();
          break;
        case S0_SEND_SCHEME_INHERIT_ACTION:
          if (SL_STATUS_OK != s0_send_scheme_inherit()) {
            sl_log_error(LOG_TAG,
                         "Failure in sending SCHEME INHERIT. "
                         "Aborting S0 bootstrapping.\n");
            s0_sm_process_event(EV_S0_BOOTSTRAP_ABORT);
          }
          break;
        case S0_2SCHEME_REPORT_RECEIVED_ACTION:
          if (s0_on_bootstrapping_complete_callback && s0_sm.is_controller) {
            s0_on_bootstrapping_complete_callback(
              KEY_CLASS_S0,
              ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE);
          } else {
            sl_log_warning(
              LOG_TAG,
              "No s0_on_bootstrapping_complete_callback set to be notified?\n");
          }
          break;
        case S0_BOOTSTRAP_ABORT_ACTION:
          s0_abort_bootstrap();
          break;
        case S0_NO_ACTION:
          reset_s0_sm();
          break;
        default:
          break;
      }
    }
  }
}

void s0_bootstrapping_start(const zwave_controller_connection_info_t *conn,
                            uint8_t is_controller)
{
  if (s0_sm.bootstrap_state == S0_INC_IDLE) {
    reset_s0_sm();
    s0_sm.bootstrap_conn.local.node_id  = conn->local.node_id;
    s0_sm.bootstrap_conn.remote.node_id = conn->remote.node_id;
    s0_sm_process_event(EV_S0_BOOTSTRAPPING_START);
    s0_sm.is_controller = is_controller;
  } else {
    sl_log_debug(LOG_TAG,
                 "S0 bootstrap state machine is not idle and "
                 "received bootstrapping request. Dropping the "
                 "request\n");
  }
}

void s0_scheme_report_received(const uint8_t scheme, zwave_node_id_t from_node)
{
  sl_log_debug(LOG_TAG, "s0_scheme_report_received\n");
  /* Scheme report can be received from other node than one which is being
    * bootstrapped as scheme report is not s0 encapsulated and transport layer
    * does not have to decrypt it*/
  if ((from_node == s0_sm.bootstrap_conn.remote.node_id)
      && ((s0_sm.bootstrap_state == S0_AWAITING_SCHEME_REPORT)
          || (s0_sm.bootstrap_state == S0_AWAITING_2SCHEME_REPORT))) {
    ctimer_stop(&s0_sm.bootstrap_timer);
    s0_sm.scheme_report = scheme;
    s0_sm_process_event(EV_S0_SCHEME_REPORT_RECEIVED);
  } else {
    sl_log_debug(LOG_TAG,
                 "S0 bootstrap state machine is not expecting "
                 "scheme report. Dropping it.\n");
  }
}

void s0_network_key_verify_received(zwave_node_id_t from_node)
{
  if (from_node == s0_sm.bootstrap_conn.remote.node_id) {
    ctimer_stop(&s0_sm.bootstrap_timer);
    s0_sm_process_event(EV_S0_NET_KEY_VERIFY_RECEIVED);
  }
}

void s0_handle_net_key_verify()
{
  if (!s0_sm.is_controller) {
    if (s0_on_bootstrapping_complete_callback) {
      s0_on_bootstrapping_complete_callback(
        KEY_CLASS_S0,
        ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE);
    }
    s0_sm_process_event(EV_S0_WAS_NOT_CONTROLLER);
  } else {
    s0_sm_process_event(EV_S0_SEND_SCHEME_INHERIT);
  }
}

void s0_set_callback(s0_on_bootstrapping_complete_cb cb)
{
  s0_on_bootstrapping_complete_callback = cb;
}

s0_bootstrap_state_t get_s0_sm_state()
{
  return s0_sm.bootstrap_state;
}

void s0_bootstrapping_stop() {
  reset_s0_sm();
}