/* © 2014 Silicon Laboratories Inc. */
/*
 * s2_inclusion.c
 *
 *  Created on: Aug 28, 2015
 *      Author: trasmussen
 */
#include "s2_inclusion.h"
#include <string.h>
#include <stdint.h>
#include "s2_protocol.h"
#include "s2_keystore.h"

#include <platform.h>
#include "ZW_classcmd.h"
#include "s2_classcmd.h"
#include "kderiv.h"
#include "curve25519.h"
#include "s2_inclusion_internal.h"
#ifdef ZWAVE_PSA_SECURE_VAULT
#include "s2_psa.h"
#endif

//#define DEBUG       // To enable debug_print_hex()
//#define DEBUGPRINT

#ifdef DEBUGPRINT
#include "../../../Components/DebugPrint/DebugPrint.h"
#else
#define DPRINT(PSTRING)
#define DPRINTF(PFORMAT, ...)
#endif

#define CHECK_AND_FAIL(CHECK, FAILURE)   if ((CHECK)){ return (FAILURE); }


/** Forward declarations of transition actions to be able to define the state machine flow.
 *  Look in bottom of file for actual implementation of actions.
 */
#ifdef ZW_CONTROLLER // Those functions used during transistions are only supported by a controller.
void execute_action_controller(uint8_t action);
#endif // ZW_CONTROLLER
static void s2_joining_start(void);
static void s2_send_kex_report(void);
static void s2_send_pub_key_b(void);
static void s2_pub_key_a_recv(void);
static void s2_do_ecdh_calc_b(void);
static void s2_send_echo_kex_set(void);
static void s2_send_net_key_get(void);
static void s2_send_net_key_verify(void);
static void s2_joining_complete(zwave_event_codes_t);
static void s2_send_final_transfer_end(void);
void inclusion_failed_frame_send(uint8_t error, uint8_t secure);
static void remote_inclusion_failed_evt_push(void);
void inclusion_failed_evt_push(uint8_t fail_code);
void s2_inclusion_send_frame(void);
void s2_inclusion_send_data(void);

static void s2_public_key_read(uint8_t *buf);
#ifndef ZWAVE_PSA_SECURE_VAULT
static void s2_private_key_read(uint8_t *buf);
#else
uint8_t zwave_shared_secret[ZWAVE_ECDH_SECRET_LENGTH];
#endif

/* Forward declaration */
/** @brief This function handles events received during secure inclusion of a node.
 *
 *  @param[in] context Pointer to the context to which the event is related.
 */
static void s2_inclusion_post_event_internal(struct S2 *p_context);

/** Default event handler which silently will discard events in case no handler is configured.
 */
static void s2_default_evt_handler(zwave_event_t * evt);

/** TODO: We should ensure the events in this enum matches the COMMANDS in COMMAND_CLASS_S2 to
 *        make it easier to understand the flow and avoid unneccesary translation of codes.
 *  CAUTION: Depending on compile target and/or optimization level, an enum can be 1 byte, 2 bytes,
 *           or maybe 4 bytes (word), e.g. on cortex-M. In z-wave it is 1 byte or 2 bytes, and this
 *           must be taken into consideration when casting.
 */

#ifdef DEBUG_S2_INCL_FSM
const char * s2_incl_state_name(s2_inclusion_state_t st)
{
  static char str[25];
  switch (st)
  {
  case S2_INC_IDLE                   :       return  "S2_INC_IDLE";
  case S2_AWAITING_KEX_GET           :       return  "S2_AWAITING_KEX_GET";
  case S2_AWAITING_KEX_REPORT        :       return  "S2_AWAITING_KEX_REPORT";
  case S2_AWAITING_KEY_USER_ACCEPT   :       return  "S2_AWAITING_KEY_USER_ACCEPT";
  case S2_AWAITING_KEX_SET           :       return  "S2_AWAITING_KEX_SET";
  case S2_AWAITING_PUB_KEY_A         :       return  "S2_AWAITING_PUB_KEY_A";
  case S2_AWAITING_PUB_KEY_B         :       return  "S2_AWAITING_PUB_KEY_B";
  case S2_AWAITING_USER_ACCEPT       :       return  "S2_AWAITING_USER_ACCEPT";
  case S2_AWAITING_USER_A_ACCEPT     :       return  "S2_AWAITING_USER_A_ACCEPT";
  case S2_PENDING_ECHO_KEX_REPORT    :       return  "S2_PENDING_ECHO_KEX_REPORT";
  case S2_ECHO_KEX_SET_SENDING       :       return  "S2_ECHO_KEX_SET_SENDING";
  case S2_AWAITING_ECHO_KEX_SET      :       return  "S2_AWAITING_ECHO_KEX_SET";
  case S2_AWAITING_ECHO_KEX_REPORT   :       return  "S2_AWAITING_ECHO_KEX_REPORT";
  case S2_AWAITING_NET_KEY_GET       :       return  "S2_AWAITING_NET_KEY_GET";
  case S2_AWAITING_NET_KEY_REPORT    :       return  "S2_AWAITING_NET_KEY_REPORT";
  case S2_AWAITING_NET_KEY_VERIFY    :       return  "S2_AWAITING_NET_KEY_VERIFY";
  case S2_AWAITING_TRANSFER_END      :       return  "S2_AWAITING_TRANSFER_END";
  case S2_KEY_EXCHANGED              :       return  "S2_KEY_EXCHANGED";
  case S2_SENDING_FINAL_TRANSFER_END :       return  "S2_SENDING_FINAL_TRANSFER_END";
  case S2_ERROR_SENT                 :       return  "S2_ERROR_SENT";
  case S2_INC_STATE_ANY              :       return  "S2_INC_STATE_ANY";

  default:
    snprintf(str, sizeof str, "%d", st);
    return str;
  }
}

const char * s2_incl_event_name(s2_inclusion_event_t ev)
{
  static char str[25];
  switch (ev)
  {
  case S2_KEX_REPORT_RECV           :       return  "S2_KEX_REPORT_RECV";
  case S2_PUB_KEY_RECV_B            :       return  "S2_PUB_KEY_RECV_B";
  case S2_PUB_KEY_RECV_A            :       return  "S2_PUB_KEY_RECV_A";
  case S2_KEX_GET_RECV              :       return  "S2_KEX_GET_RECV";
  case S2_KEX_SET_RECV              :       return  "S2_KEX_SET_RECV";
  case S2_ECHO_KEX_REPORT_RECV      :       return  "S2_ECHO_KEX_REPORT_RECV";
  case S2_NET_KEY_REPORT_RECV       :       return  "S2_NET_KEY_REPORT_RECV";
  case S2_ECHO_KEX_SET_RECV         :       return  "S2_ECHO_KEX_SET_RECV";
  case S2_NET_KEY_GET_RECV          :       return  "S2_NET_KEY_GET_RECV";
  case S2_NET_KEY_VERIFY_RECV       :       return  "S2_NET_KEY_VERIFY_RECV";
  case S2_TRANSFER_END_RECV         :       return  "S2_TRANSFER_END_RECV";
  case S2_INCLUSION_ERROR_RECV      :       return  "S2_INCLUSION_ERROR_RECV";
  case S2_INCLUDING_START           :       return  "S2_INCLUDING_START";
  case S2_JOINING_START             :       return  "S2_JOINING_START";
  case S2_INCLUDING_ACCEPT          :       return  "S2_INCLUDING_ACCEPT";
  case S2_INCLUDING_REJECT          :       return  "S2_INCLUDING_REJECT";
  case S2_INCLUDING_DECRYPT_FAILED  :       return  "S2_INCLUDING_DECRYPT_FAILED";
  case S2_SEND_FINAL_TRANSFER_END   :       return  "S2_SEND_FINAL_TRANSFER_END";
  case S2_INCLUSION_TX_QUEUED       :       return  "S2_INCLUSION_TX_QUEUED";
  case S2_NET_KEY_VERIFY_FINAL_RECV :       return  "S2_NET_KEY_VERIFY_FINAL_RECV";
  case S2_INCLUSION_RETRY           :       return  "S2_INCLUSION_RETRY";
  case S2_INCLUSION_TIMEOUT         :       return  "S2_INCLUSION_TIMEOUT";
  case S2_INCLUSION_ERROR           :       return  "S2_INCLUSION_ERROR";
  case S2_INCLUSION_ERROR_SENT      :       return  "S2_INCLUSION_ERROR_SENT";
  case S2_INCLUSION_SEND_DONE       :       return  "S2_INCLUSION_SEND_DONE";
  case S2_INCLUSION_SEND_FAILED     :       return  "S2_INCLUSION_SEND_FAILED";
  case S2_DISCOVERY_COMPLETE        :       return  "S2_DISCOVERY_COMPLETE";
  case S2_NO_KEYS_GRANTED           :       return  "S2_NO_KEYS_GRANTED";
  case S2_EVT_ANY                   :       return  "S2_EVT_ANY";

  default:
    snprintf(str, sizeof str, "%d", ev);
    return str;
  }
}

#endif /* DEBUG_S2_INCL_FSM */

#ifdef ZW_CONTROLLER
extern const s2_transition_t s2_transition_table_controller[];
extern const size_t s2_transition_table_controller_length;
#endif

static const s2_transition_t s2_transition_table[] = {
  {S2_INC_IDLE,                   S2_JOINING_START,           S2_JOINING_START_ACTION,            S2_AWAITING_KEX_GET},
  {S2_AWAITING_KEX_GET,           S2_DISCOVERY_COMPLETE,      S2_TIMEOUT_TB1_SET_ACTION,          S2_AWAITING_KEX_GET},
  {S2_AWAITING_KEX_GET,           S2_KEX_GET_RECV,            S2_SEND_KEX_REPORT_ACTION,          S2_AWAITING_KEX_SET},
  {S2_AWAITING_KEX_GET,           S2_INCLUDING_REJECT,        S2_NO_ACTION,                       S2_INC_IDLE},
  {S2_AWAITING_KEX_GET,           S2_INCLUSION_TIMEOUT,       S2_JOINING_NEVER_STARTED_ACTION,      S2_INC_IDLE},
  {S2_AWAITING_KEX_SET,           S2_KEX_SET_RECV,            S2_SEND_PUB_KEY_B_ACTION,           S2_AWAITING_PUB_KEY_A},
  {S2_AWAITING_KEX_SET,           S2_INCLUSION_RETRY,         S2_RESEND_FRAME_ACTION,             S2_INC_STATE_ANY},
  {S2_AWAITING_KEX_SET,           S2_KEX_GET_RECV,            S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_PUB_KEY_A,         S2_PUB_KEY_RECV_A,          S2_PUB_KEY_A_RECV_ACTION,           S2_AWAITING_USER_A_ACCEPT},
  {S2_AWAITING_PUB_KEY_A,         S2_INCLUSION_RETRY,         S2_RESEND_FRAME_ACTION,             S2_INC_STATE_ANY},
  {S2_AWAITING_PUB_KEY_A,         S2_KEX_GET_RECV,            S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_PUB_KEY_A,         S2_KEX_SET_RECV,            S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_USER_A_ACCEPT,     S2_INCLUDING_ACCEPT,        S2_DO_ECDH_CALC_B_ACTION,           S2_ECHO_KEX_SET_SENDING},
  {S2_AWAITING_USER_A_ACCEPT,     S2_INCLUDING_REJECT,        S2_ABORT_ACTION,                    S2_INC_IDLE},
  {S2_AWAITING_USER_A_ACCEPT,     S2_KEX_GET_RECV,            S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_USER_A_ACCEPT,     S2_KEX_SET_RECV,            S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_USER_A_ACCEPT,     S2_PUB_KEY_RECV_A,          S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_ECHO_KEX_SET_SENDING,       S2_INCLUSION_SEND_FAILED,   S2_NO_ACTION,                       S2_ECHO_KEX_SET_SENDING},
  {S2_ECHO_KEX_SET_SENDING,       S2_INCLUSION_SEND_DONE,     S2_NO_ACTION,                       S2_ECHO_KEX_SET_SENDING},
  {S2_ECHO_KEX_SET_SENDING,       S2_INCLUSION_RETRY,         S2_SEND_ECHO_KEX_SET_ACTION,        S2_ECHO_KEX_SET_SENDING},
  {S2_ECHO_KEX_SET_SENDING,       S2_ECHO_KEX_REPORT_RECV,    S2_SEND_NET_KEY_GET_ACTION,         S2_AWAITING_NET_KEY_REPORT},
  {S2_ECHO_KEX_SET_SENDING,       S2_KEX_GET_RECV,            S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_ECHO_KEX_SET_SENDING,       S2_KEX_SET_RECV,            S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_ECHO_KEX_SET_SENDING,       S2_PUB_KEY_RECV_A,          S2_NO_ACTION,                       S2_INC_STATE_ANY},

  {S2_AWAITING_NET_KEY_REPORT,    S2_NET_KEY_REPORT_RECV,     S2_SEND_NET_KEY_VERIFY_ACTION,      S2_KEY_EXCHANGED},
  {S2_AWAITING_NET_KEY_REPORT,    S2_INCLUSION_RETRY,         S2_RESEND_DATA_ACTION,              S2_INC_STATE_ANY},
  {S2_AWAITING_NET_KEY_REPORT,    S2_KEX_GET_RECV,            S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_NET_KEY_REPORT,    S2_KEX_SET_RECV,            S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_NET_KEY_REPORT,    S2_PUB_KEY_RECV_A,          S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_NET_KEY_REPORT,    S2_ECHO_KEX_REPORT_RECV,    S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_NET_KEY_REPORT,    S2_NO_KEYS_GRANTED,         S2_SEND_FINAL_TRANSFER_END_ACTION,  S2_SENDING_FINAL_TRANSFER_END},

  {S2_KEY_EXCHANGED,              S2_TRANSFER_END_RECV,       S2_SEND_NET_KEY_GET_ACTION,         S2_AWAITING_NET_KEY_REPORT},
  {S2_KEY_EXCHANGED,              S2_INCLUSION_RETRY,         S2_RESEND_DATA_ACTION,              S2_INC_STATE_ANY},
  {S2_KEY_EXCHANGED,              S2_KEX_GET_RECV,            S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_KEY_EXCHANGED,              S2_KEX_SET_RECV,            S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_KEY_EXCHANGED,              S2_PUB_KEY_RECV_A,          S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_KEY_EXCHANGED,              S2_ECHO_KEX_REPORT_RECV,    S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_KEY_EXCHANGED,              S2_NET_KEY_REPORT_RECV,     S2_NO_ACTION,                       S2_INC_STATE_ANY},

  {S2_KEY_EXCHANGED,              S2_SEND_FINAL_TRANSFER_END, S2_SEND_FINAL_TRANSFER_END_ACTION,  S2_SENDING_FINAL_TRANSFER_END},
  {S2_SENDING_FINAL_TRANSFER_END, S2_INCLUSION_RETRY,         S2_RESEND_DATA_ACTION,              S2_INC_STATE_ANY},
  {S2_SENDING_FINAL_TRANSFER_END, S2_INCLUSION_SEND_DONE,     S2_JOINING_COMPLETE_ACTION,         S2_INC_IDLE},
  {S2_SENDING_FINAL_TRANSFER_END, S2_KEX_GET_RECV,            S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_SENDING_FINAL_TRANSFER_END, S2_KEX_SET_RECV,            S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_SENDING_FINAL_TRANSFER_END, S2_PUB_KEY_RECV_A,          S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_SENDING_FINAL_TRANSFER_END, S2_ECHO_KEX_REPORT_RECV,    S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_SENDING_FINAL_TRANSFER_END, S2_NET_KEY_REPORT_RECV,     S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_SENDING_FINAL_TRANSFER_END, S2_SEND_FINAL_TRANSFER_END, S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_SENDING_FINAL_TRANSFER_END, S2_TRANSFER_END_RECV,       S2_NO_ACTION,                       S2_INC_STATE_ANY},

  {S2_ERROR_SENT,              S2_INCLUSION_SEND_DONE,      S2_ABORT_ACTION,        S2_INC_IDLE},
  {S2_INC_IDLE,                S2_INCLUSION_TIMEOUT,        S2_NO_ACTION,           S2_INC_IDLE},
  {S2_INC_IDLE,                S2_EVT_ANY,                  S2_NO_ACTION,           S2_INC_IDLE},
  {S2_INC_STATE_ANY,           S2_INCLUSION_TIMEOUT,        S2_ABORT_ACTION,        S2_INC_IDLE},
  {S2_INC_STATE_ANY,           S2_INCLUSION_ERROR_SENT,     S2_NO_ACTION,           S2_ERROR_SENT},
  {S2_INC_STATE_ANY,           S2_INCLUSION_ERROR_RECV,     S2_REMOTE_ERROR_ACTION, S2_INC_IDLE},
  {S2_INC_STATE_ANY,           S2_INCLUSION_ERROR,          S2_ABORT_ACTION,        S2_INC_IDLE},
  {S2_INC_STATE_ANY,           S2_INCLUDING_DECRYPT_FAILED, S2_NO_ACTION,           S2_INC_STATE_ANY},
  {S2_INC_STATE_ANY,           S2_INCLUSION_SEND_DONE,      S2_NO_ACTION,           S2_INC_STATE_ANY},
  {S2_INC_STATE_ANY,           S2_INCLUSION_SEND_FAILED,    S2_ABORT_ACTION,        S2_INC_IDLE},
  {S2_INC_STATE_ANY,           S2_DISCOVERY_COMPLETE,       S2_NO_ACTION,           S2_INC_STATE_ANY},
  {S2_INC_STATE_ANY,           S2_EVT_ANY,                  S2_ABORT_ACTION,        S2_INC_IDLE},
};

/** Double array containing rules for frame processing.
 *  Note, as frame indexing starts from KEX GET command class number, the KEX GET value must be
 *  subtracted from command class value to obtain the rule index.
 *  - First rule is minimal length of frame.
 *  - Second rule is frame encryption, non-encrypted, temp key encrypted, network key encrypted.
 */
static const uint8_t m_frame_rules[][4] = {
  {SECURITY_2_KEX_GET_LENGTH       , SINGLE_RULE_SUPPORT, NON_SECURE,         NON_SECURE}, // Kex get frame rules.
  {SECURITY_2_KEX_REPORT_LENGTH    , ECHO_SUPPORT,        NON_SECURE,         TEMP_KEY_SECURE}, // Kex report frame rules.
  {SECURITY_2_KEX_SET_LENGTH       , ECHO_SUPPORT,        NON_SECURE,         TEMP_KEY_SECURE}, // Kex set frame rules.
  {SECURITY_2_KEX_FAIL_LENGTH      , DUAL_RULE_SUPPORT,   NON_SECURE,         TEMP_KEY_SECURE},  // KEX_GET frame rules.
  {SECURITY_2_PUB_KEY_LENGTH       , SINGLE_RULE_SUPPORT, NON_SECURE,         NON_SECURE}, // public key frame rules.
  {SECURITY_2_NET_KEY_GET_LENGTH   , SINGLE_RULE_SUPPORT, TEMP_KEY_SECURE,    TEMP_KEY_SECURE}, // Network key get frame rules.
  {SECURITY_2_NET_KEY_REPORT_LENGTH, SINGLE_RULE_SUPPORT, TEMP_KEY_SECURE,    TEMP_KEY_SECURE}, // Network key report frame rules.
  {SECURITY_2_NET_KEY_VERIFY_LENGTH, SINGLE_RULE_SUPPORT, NETWORK_KEY_SECURE, NETWORK_KEY_SECURE}, // Network key verify frame rules.
  {SECURITY_2_TRANSFER_END_LENGTH  , SINGLE_RULE_SUPPORT, TEMP_KEY_SECURE,    TEMP_KEY_SECURE}, // Transfer end frame rules.
};

static const uint8_t m_key_slot_pair[][2] = {
  {KEY_CLASS_S2_UNAUTHENTICATED , UNAUTHENTICATED_KEY_SLOT},
  {KEY_CLASS_S2_AUTHENTICATED   , AUTHENTICATED_KEY_SLOT},
  {KEY_CLASS_S2_ACCESS          , ACCESS_KEY_SLOT},
#ifdef ZW_CONTROLLER
  {KEY_CLASS_S2_AUTHENTICATED_LR, LR_AUTHENTICATED_KEY_SLOT},
  {KEY_CLASS_S2_ACCESS_LR       , LR_ACCESS_KEY_SLOT},
#endif
};

/**
 * Mapping between key slots reported by S2.c when decrypting a frame and the key class bitmask used in the libs2 API and
 * here. The order must be the same as that in \ref m_key_slot_pair, with the nonstandard keys like TEMP_KEY_SECURE and S0 key at the end.
 * Used only for validating the keys used to decrypt the Network Key Report on the including side.
 */
static const uint8_t m_key_slot_key_class_pair[] = {
  KEY_CLASS_S2_UNAUTHENTICATED,  // UNAUTHENTICATED_KEY_SLOT is class id 0 = index 0 in array.
  KEY_CLASS_S2_AUTHENTICATED,    // AUTHENTICATED_KEY_SLOT   is class id 1 = index 1 in array.
  KEY_CLASS_S2_ACCESS,           // ACCESS_KEY_SLOT          is class id 2 = index 2 in array.
  // Class id 3 is actually the LR Auth keyslot, but it identifies as "normal" Auth
  KEY_CLASS_S2_AUTHENTICATED,    // LR_AUTHENTICATED_KEY_SLOT   is class id 3 = index 3 in array.
  // Class id 4 is actually the LR Auth keyslot, but it identifies as "normal" Auth
  KEY_CLASS_S2_ACCESS,           // LR_ACCESS_KEY_SLOT          is class id 4 = index 4 in array.
  0x00,                          // TEMP_KEY_SECURE          is class id 5 = index 5 in array, which is not a valid value for network key exchange, hence 0x00.
  KEY_CLASS_S0                   // S0 key                   is class id 6 = index 6 in array. Index only used temporary to exchange S0 key
};

s2_event_handler_t m_evt_handler = s2_default_evt_handler;
uint32_t           m_event_buffer[S2_EVT_BUFFER_SIZE];
static uint8_t m_schemes;
static uint8_t m_curves;
static uint8_t m_keys;             //< Bitmask of keys this node should request when joining.
uint8_t shared_secret[32];
uint8_t shared_key_mem[64]; //< Shared memory array for handling of keys/auth tag during inclusion.

struct S2 *mp_context;

uint8_t m_retry_counter;

static void execute_action(uint8_t action)
{
  switch (action)
  {
    case S2_REMOTE_ERROR_ACTION:
      remote_inclusion_failed_evt_push();
    break;

    case S2_ABORT_ACTION:
      inclusion_failed_evt_push(mp_context->kex_fail_code);
    break;

    case S2_JOINING_START_ACTION:
      s2_joining_start();
    break;

    case S2_TIMEOUT_TB1_SET_ACTION:
      s2_inclusion_stop_timeout();
      s2_inclusion_set_timeout(mp_context, TB1_TIMEOUT);
    break;

    case S2_SEND_KEX_REPORT_ACTION:
      s2_send_kex_report();
    break;

    case S2_SEND_PUB_KEY_B_ACTION:
      s2_send_pub_key_b();
    break;

    case S2_PUB_KEY_A_RECV_ACTION:
      s2_pub_key_a_recv();
    break;

    case S2_DO_ECDH_CALC_B_ACTION:
      s2_do_ecdh_calc_b();
    break;

    case S2_SEND_ECHO_KEX_SET_ACTION:
      s2_send_echo_kex_set();
    break;

    case S2_SEND_NET_KEY_GET_ACTION:
      s2_send_net_key_get();
    break;

    case S2_SEND_NET_KEY_VERIFY_ACTION:
      s2_send_net_key_verify();
    break;

    case S2_SEND_FINAL_TRANSFER_END_ACTION:
      s2_send_final_transfer_end();
    break;

    case S2_JOINING_COMPLETE_ACTION:
      s2_joining_complete(S2_NODE_JOINING_COMPLETE_EVENT);
    break;

    case S2_JOINING_NEVER_STARTED_ACTION:
      s2_joining_complete(S2_JOINING_COMPLETE_NEVER_STARTED_EVENT);
    break;

    case S2_RESEND_FRAME_ACTION:
      s2_inclusion_send_frame();
    break;

    case S2_RESEND_DATA_ACTION:
      s2_inclusion_send_data();
    break;

    case S2_NO_ACTION:
    break;

    default:
#ifdef ZW_CONTROLLER
      execute_action_controller(action);
#endif // ZW_CONTROLLER
    break;
  }
}

static bool process_s2_inclusion_event(s2_inclusion_event_t event, const s2_transition_t * table, size_t table_length)
{
  for (uint8_t i = 0; i < table_length; i++)
  {
    if (mp_context->inclusion_state == table[i].state || S2_INC_STATE_ANY == table[i].state)
    {
      DPRINTF("STATE TRANSITION: ? %u -> %u \n", mp_context->inclusion_state, table[i].state);

      if ((event == table[i].event) || (S2_EVT_ANY == table[i].event))
      {
        DPRINTF("new_state: %u, action: %u \n", table[i].new_state, table[i].action);

        // Found a match. Execute action and update state if new state is different from S2_INC_STATE_ANY.
        if ((S2_INC_STATE_ANY != table[i].new_state)
            && (mp_context->inclusion_state != table[i].new_state))
        {
          mp_context->inclusion_state = table[i].new_state;
        }

        execute_action(table[i].action);
        return true;
      }
    }
  }

  return false;
}

static void s2_default_evt_handler(__attribute__((unused)) zwave_event_t *evt)
{
}
/** Member function for internal event handling.
 *  This function is intended to be called when the event type is known, that is when a frame is
 *  decoded or an event is triggered by a function call.
 */
void process_event(uint16_t evt)
{
  bool event_handled = false;
  s2_inclusion_event_t event = (s2_inclusion_event_t)evt;

#if defined(DEBUG_S2_INCL_FSM)
  /* Disabled in C51 build to save code space on state/event _name() functions*/
    DPRINTF( "s2incl process_event event: %s state: %s\n", s2_incl_event_name(event), s2_incl_state_name(mp_context->inclusion_state));
#endif

#ifdef ZW_CONTROLLER
  event_handled = process_s2_inclusion_event(event, s2_transition_table_controller, s2_transition_table_controller_length);
#endif
  if (!event_handled)
  {
    process_s2_inclusion_event(event, s2_transition_table, ELEM_COUNT(s2_transition_table));
  }

//  ZW_DEBs2_joining_complete,        UG_SEND_BYTE(')');
}

/** Restore all keys.
 * Each key has a corresponding bit in the bitmask specified for the Kex Report/Set frames
 * in the S2 spec SDS11274.
 * This function puts S2_UNAUTHENTICATED in index 0, S2_AUTHENTICATED in index 1,
 * and S2_ACCESS in index 2, LR_AUTH in 3 and LR_ACCESS in 4.
 * This defines the API between the S2 module and the glue layer around it.
 * */
void s2_restore_keys(struct S2 *p_context, __attribute__((unused)) bool make_keys_persist_se)
{
  uint8_t i;
  bool    ret_val;
  MP_CTX_DEF

  DPRINTF("Restore Keys\n");

  mp_context->loaded_keys = 0;

  for (i = 0; i < ELEM_COUNT(m_key_slot_pair); i++)
  {
    ret_val = keystore_network_key_read(m_key_slot_pair[i][0], shared_key_mem);
    if (true == ret_val)
    {
#if defined(ZWAVE_PSA_SECURE_VAULT) && defined(ZWAVE_PSA_AES)
      S2_network_key_update(mp_context, convert_key_class_to_psa_key_id(m_key_slot_pair[i][0]),
                            m_key_slot_pair[i][1], shared_key_mem, 0, make_keys_persist_se);
     DPRINTF("^ Loading: %d, loadedKeys:%d, presist: %d\n", m_key_slot_pair[i][1], mp_context->loaded_keys, make_keys_persist_se);
#else
      S2_network_key_update(mp_context, ZWAVE_KEY_ID_NONE, m_key_slot_pair[i][1], shared_key_mem, 0, false);
#endif
    }
  }
  // Clearing of memory to ensure key is not exposed accidentially elsewhere.
  memset(shared_key_mem, 0, sizeof(NETWORK_KEY_SIZE));
}

/**@brief Function for notifying application that inclusion failed.
 *
 * @param[in] mp_context Pointer to current context holding information on inclusion.
 */
void inclusion_failed_evt_push(uint8_t fail_type)
{
  zwave_event_t * p_s2_event = (zwave_event_t *)m_event_buffer;

  s2_inclusion_stop_timeout();
  s2_restore_keys(mp_context, false);
  // Post event upwards to inform that inclusion of node A has failed.
  p_s2_event->event_type                                        = S2_NODE_INCLUSION_FAILED_EVENT;
  p_s2_event->evt.s2_event.peer                                 = mp_context->inclusion_peer;
  p_s2_event->evt.s2_event.s2_data.inclusion_fail.kex_fail_type = fail_type;
  m_evt_handler(p_s2_event);
}

/**@brief Function for notifying application that inclusion failed.
 *
 * @param[in] mp_context Pointer to current context holding information on inclusion.
 */
static void remote_inclusion_failed_evt_push(void)
{
  inclusion_failed_evt_push(mp_context->buf[SECURITY_2_KEX_FAIL_FAIL_TYPE_POS]);
}

/**@brief Function for sending back an KEX Fail frame that inclusion failed.
 *
 * @param[in] mp_context Pointer to current context holding information on inclusion.
 * @param[in] error     Error code to be transmitted.
 * @param[in] secure    Parameter denoting if the error frame should be send secure 0 = non-secure, 1 = secure.
 */
void inclusion_failed_frame_send(uint8_t error, uint8_t secure)
{
  mp_context->kex_fail_code = error;
  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_CLASS_POS]      = COMMAND_CLASS_SECURITY_2;
  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_POS]            = KEX_FAIL;
  mp_context->u.inclusion_buf[SECURITY_2_KEX_FAIL_FAIL_TYPE_POS] = error;
  mp_context->inclusion_buf_length                               = SECURITY_2_KEX_FAIL_LENGTH;

  m_retry_counter = MAX_RETRY_COUNT;
  if (TEMP_KEY_SECURE == secure)
  {
    mp_context->inclusion_peer.class_id = TEMP_KEY_SECURE;
    mp_context->inclusion_peer.tx_options = 0;
    s2_inclusion_send_data();
  }
  else
  {
    s2_inclusion_send_frame();
  }
}

/** Section defining validation functions for joining node - Begin.
 */

static uint8_t validate_echo_kex_report(void)
{
  if ((m_schemes == mp_context->buf[SECURITY_2_KEX_REP_SCHEME_POS]) &&
      (m_curves == mp_context->buf[SECURITY_2_KEX_REP_CURVE_POS]) &&
      (m_keys == mp_context->buf[SECURITY_2_KEX_REP_KEYS_POS]) &&
      ((mp_context->csa_support | SECURITY_2_ECHO_ON) == mp_context->buf[SECURITY_2_KEX_REP_ECHO_POS]) )
  {
    return 0;
  }

  inclusion_failed_frame_send(KEX_FAIL_AUTH, NON_SECURE);

  return KEX_FAIL;
}
/** Section defining validation functions for joining node - End.
 */

// Slave side - done
void s2_inclusion_set_event_handler(s2_event_handler_t evt_handler)
{
  m_evt_handler = evt_handler;
}

void s2_inclusion_challenge_response(struct S2 *p_context, uint8_t include, const uint8_t* p_response, uint8_t responseLength)
{
  MP_CTX_DEF

  if (ACCEPT_INCLUSION == include)
  {
    // The caller is internal protocol code, thus it is trusted and should keep within the buffer
    // limit of public key size.
    while(responseLength--)
    {
      mp_context->public_key[responseLength] = p_response[responseLength];
    }
    process_event(S2_INCLUDING_ACCEPT);
  }
  else
  {
    process_event(S2_INCLUDING_REJECT);
  }
}


void s2_inclusion_joining_start(struct S2 *p_context, s2_connection_t *p_connection, uint8_t csa)
{
  MP_CTX_DEF

  mp_context->inclusion_peer = *p_connection;
  mp_context->inclusion_mode = csa ? INCLUSION_MODE_CSA : INCLUSION_MODE_SSA;
  mp_context->kex_fail_code = 0;

  /* In case of LR inclusion, strip keys that are invalid in LR */
  if (IS_LR_NODE(mp_context->inclusion_peer.l_node)) {
    m_keys &= ~(KEY_CLASS_S2_UNAUTHENTICATED | KEY_CLASS_S0);
  }

  process_event(S2_JOINING_START);
}

void s2_inclusion_neighbor_discovery_complete(struct S2 *p_context)
{
  MP_CTX_DEF

  process_event(S2_DISCOVERY_COMPLETE);
}

void s2_inclusion_abort(struct S2 *p_context)
{
  MP_CTX_DEF

  process_event(S2_INCLUDING_REJECT);
}


void s2_inclusion_post_event(struct S2 *p_context,s2_connection_t* src) {
  MP_CTX_DEF

  if((src->rx_options & S2_RXOPTION_MULTICAST) == S2_RXOPTION_MULTICAST) {
    return;
  }

  if(mp_context->inclusion_state == S2_AWAITING_KEX_GET) {
    mp_context->inclusion_peer=*src;
  }

  if(mp_context->inclusion_peer.r_node == src->r_node) {
    mp_context->inclusion_peer.class_id = src->class_id;
    s2_inclusion_post_event_internal(mp_context);
  }
}

static void s2_inclusion_post_event_internal(struct S2 *p_context)
{
  // Command byte in S2 frame is used to identify the event.
  uint8_t  crypt_rule;
  uint16_t event;
  uint8_t  event_rule_index;

  MP_CTX_DEF
  event            = mp_context->buf[SECURITY_2_COMMAND_POS];
  event_rule_index = mp_context->buf[SECURITY_2_COMMAND_POS] - S2_INCLUSION_COMMAND_OFFSET;

  if ((ELEM_COUNT(m_frame_rules) < event_rule_index) ||
      (mp_context->inclusion_state == S2_INC_IDLE))
  {
    // Not an inclusion event. Just return.
    return;
  }

  if(m_frame_rules[event_rule_index][LENGTH_RULE_INDEX] > mp_context->length)
  {
    process_event(S2_INCLUSION_ERROR);
    return;
  }

  if ((ECHO_SUPPORT == m_frame_rules[event_rule_index][ENCRYPTION_SUPPORT_INDEX])
      && (mp_context->buf[SECURITY_2_KEX_SET_ECHO_POS] & SECURITY_2_ECHO_ON))
  {
    crypt_rule = m_frame_rules[event_rule_index][ENCRYPTION_RULE_INDEX_B];
  }
  else
  {
    crypt_rule = m_frame_rules[event_rule_index][ENCRYPTION_RULE_INDEX_A];
  }

  if (NETWORK_KEY_SECURE == crypt_rule)
  {
    if ((mp_context->inclusion_peer.class_id >= sizeof(m_key_slot_key_class_pair)) ||
        ((mp_context->key_requested != m_key_slot_key_class_pair[mp_context->inclusion_peer.class_id]) &&    // If this happens, it could either mean we have an invalid request or an late ariving repeated frame.
        ((mp_context->key_exchange & m_key_slot_key_class_pair[mp_context->inclusion_peer.class_id]) == 0))) // Hence, we check if the class id matches and already received key.
    {
      inclusion_failed_frame_send(KEX_FAIL_AUTH, NON_SECURE);
      process_event(S2_INCLUSION_ERROR_SENT);
      return;
    }
  }
  else if (crypt_rule != mp_context->inclusion_peer.class_id)
  {
    if ((DUAL_RULE_SUPPORT != m_frame_rules[event_rule_index][ENCRYPTION_SUPPORT_INDEX])
        || (mp_context->inclusion_peer.class_id != m_frame_rules[event_rule_index][ENCRYPTION_RULE_INDEX_B]))
    {
      inclusion_failed_frame_send(KEX_FAIL_AUTH, NON_SECURE);
      process_event(S2_INCLUSION_ERROR_SENT);
      return;
    }
  }

  switch (event)
  {
    case KEX_SET:
    case KEX_REPORT:
      // KEX_SET and KEX_REPORT frames can be with/without echo bit.
      // To differentiate in state machine the echo bit is shifted to first byte in event to either
      // denote an S2_KEX_SET_RECV or S2_ECHO_KEX_SET_RECV event respectively.
      event |= ((mp_context->buf[SECURITY_2_KEX_SET_ECHO_POS] & SECURITY_2_ECHO_ON) << KEX_ECHO_BIT_POS);
      break;

    case PUBLIC_KEY_REPORT:
      event |= ((mp_context->buf[SECURITY_2_PUB_KEY_INC_FLAG_POS] & SECURITY_2_INCLUDING_NODE) << PUBLIC_KEY_INCLUDING_NODE_BIT_POS);
      break;

    case SECURITY_2_NETWORK_KEY_VERIFY:
      // Key exchange on including side is bits representing all keys exchanged so far.
      // Taking the negative value, results in all upper bits set and thus it can be evaluated if more keys should be exchanged.
      // Example: 0x04 -> 0xFC, which indicates that keys denoted by bit 0 and 1 are already exchanged.
      // This event is only triggered in the including side.
      event = (mp_context->key_exchange == mp_context->key_granted) ? S2_NET_KEY_VERIFY_FINAL_RECV : S2_NET_KEY_VERIFY_RECV;
      break;

    case SECURITY_2_TRANSFER_END:
      // Key exchange is a single bit value denoting the key being exchanged or bit shifted one to the left when a key is exchanged.
      // Taking the negative value, results in all upper bits set and thus it can be evaluated if more keys should be exchanged.
      // Example: 0x04 -> 0xFC, which indicates that keys denoted by bit 0 and 1 are already exchanged.
      // The minus trick only works when a single bit is set, and hence only on the joining side.
      // This event is only posted on the joining side AND on the including side for final transfer end.
      event = ((-mp_context->key_exchange) & mp_context->key_granted) ? S2_TRANSFER_END_RECV : S2_SEND_FINAL_TRANSFER_END;
      break;

    default:
      // Do nothing.
      break;
  }

  process_event(event);
}

#ifdef __C51__
void ZCB_s2_inclusion_notify_timeout(struct S2 *p_context);
code const void (code * ZCB_s2_inclusion_notify_timeout_p)(void) = &ZCB_s2_inclusion_notify_timeout;
void ZCB_s2_inclusion_notify_timeout(struct S2 *p_context)
#else
void s2_inclusion_notify_timeout(struct S2 *p_context)
#endif
{
  MP_CTX_DEF

  if (S2_ECHO_KEX_SET_SENDING == mp_context->inclusion_state) // Hack by checking current state too early as we are debugging ring issue.
    process_event((NO_RETRIES == m_retry_counter) || (QUEUE_FULL == m_retry_counter) ? S2_INCLUSION_TIMEOUT : S2_INCLUSION_RETRY);
  else
    process_event(S2_INCLUSION_TIMEOUT);
}

void s2_inclusion_send_done(struct S2 *p_context, uint8_t status)
{
  MP_CTX_DEF

  if (QUEUE_FULL == m_retry_counter)
  {
    m_retry_counter = MAX_RETRY_COUNT;
    process_event(S2_INCLUSION_RETRY);
  }
  else if ((m_retry_counter > NO_RETRIES) && (S2_ECHO_KEX_SET_SENDING != mp_context->inclusion_state)) // Hack by checking current state too early as we are debugging ring issue.
  {
    process_event(status ? S2_INCLUSION_SEND_DONE : S2_INCLUSION_RETRY);
  }
  else
  {
    process_event(status ? S2_INCLUSION_SEND_DONE : S2_INCLUSION_SEND_FAILED);
  }
}

void s2_inclusion_decryption_failure(struct S2 *p_context,s2_connection_t* src)
{
  MP_CTX_DEF

  if(src->r_node == mp_context->inclusion_peer.r_node) {
    process_event(S2_INCLUDING_DECRYPT_FAILED);
  }
}


uint8_t s2_inclusion_init(uint8_t schemes, uint8_t curves, uint8_t keys_to_request)
{
  CHECK_AND_FAIL((SECURITY_2_SCHEME_SUPPORT_MASK != schemes), KEX_FAIL_KEX_SCHEME);
  CHECK_AND_FAIL((KEX_REPORT_CURVE_25519 != curves), KEX_FAIL_KEX_CURVES);
  CHECK_AND_FAIL(((~SECURITY_2_KEY_MASK & keys_to_request) ||
                 ((SECURITY_2_KEY_MASK & keys_to_request) == 0)), KEX_FAIL_KEX_KEY);

  m_schemes = schemes;
  m_curves  = curves;
  m_keys    = keys_to_request;  // m_keys should include all the keys to request when being bootstrapped
                                // LR keys are so far excluded
  return 0;
}

/** Section defining joining node functions for state transistion actions - Start.
 */
static void s2_send_kex_report(void)
{
  zwave_event_t * p_s2_event;

  s2_inclusion_stop_timeout();

  /* We have received a Kex Get and therefore old keys are no longer valid and should be cleared.
   * We postpone this until after Kex Get has been received, so we don't delete our keys
   * prematurely in case a controller replication was performed instead.
   */
  keystore_network_key_clear(0xFF);

  p_s2_event = (zwave_event_t *)m_event_buffer;
  p_s2_event->event_type        = S2_NODE_INCLUSION_INITIATED_EVENT;
  p_s2_event->evt.s2_event.peer = mp_context->inclusion_peer;
  m_evt_handler(p_s2_event);

  mp_context->csa_support = (mp_context->inclusion_mode == INCLUSION_MODE_CSA ? SECURITY_2_CSA_ON : 0);
  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_CLASS_POS]  = COMMAND_CLASS_SECURITY_2;
  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_POS]        = KEX_REPORT;
  mp_context->u.inclusion_buf[SECURITY_2_KEX_REP_ECHO_POS]   = mp_context->csa_support | SECURITY_2_ECHO_OFF;
  mp_context->u.inclusion_buf[SECURITY_2_KEX_REP_SCHEME_POS] = m_schemes;
  mp_context->u.inclusion_buf[SECURITY_2_KEX_REP_CURVE_POS]  = m_curves;
  mp_context->u.inclusion_buf[SECURITY_2_KEX_REP_KEYS_POS]   = m_keys;
  mp_context->inclusion_buf_length                           = SECURITY_2_KEX_REPORT_LENGTH;
  mp_context->is_keys_restored = false;
  m_retry_counter = MAX_RETRY_COUNT;
  s2_inclusion_send_frame();
  // While waiting for KEX SET set s2 inclucion timeout TB2_TIMEOUT
  s2_inclusion_set_timeout(mp_context, TB2_TIMEOUT);
}

static void s2_send_pub_key_b(void)
{
  uint8_t support;  // Helper variable, being reused purely for code optimization reasons on C51 targets

  s2_inclusion_stop_timeout();

  support = mp_context->buf[SECURITY_2_KEX_SET_SCHEME_POS];
  // Check only single scheme is selected and that the selected bit matches a supported scheme.
  if ((0 == (support & (support - 1))) && (support & m_schemes))
  {
    mp_context->scheme_support = support;
  }
  else
  {
    support = KEX_FAIL_KEX_SCHEME;
    goto error_handling;
  }

  support = mp_context->buf[SECURITY_2_KEX_SET_CURVE_POS];
  // Check only single scheme is selected and that the selected bit matches a supported scheme.
  if ((0 == (support & (support - 1))) && (support & m_curves))
  {
    mp_context->curve_support = support;
  }
  else
  {
    support = KEX_FAIL_KEX_CURVES;
    goto error_handling;
  }

  support = mp_context->buf[SECURITY_2_KEX_SET_KEYS_POS];
  //  no keys granted is not ok when node is LR
  if ((0 == support) && IS_LR_NODE(mp_context->inclusion_peer.l_node)) {
    support = KEX_FAIL_KEX_KEY;
    goto error_handling;
  } else if (0 == (~m_keys & support)) {
  // Check if granted key/s are supported - no keys granted are also ok when nmode is classic
    mp_context->key_granted = support;
  } else {
    // We have been granted keys we don't support, this is an error
    support = KEX_FAIL_KEX_KEY;
    goto error_handling;
  }

  if(mp_context->buf[SECURITY_2_KEX_SET_CSA_POS] & SECURITY_2_CSA_ON)
  {
    if(mp_context->inclusion_mode!= INCLUSION_MODE_CSA) //We did not ask for this
    {
      support = KEX_FAIL_KEX_KEY;
      goto error_handling;
    }
  }
  else
  {
    /**
     * Should we be able to reject here if we are not granted CSA
     */
    mp_context->inclusion_mode = INCLUSION_MODE_SSA;
  }

  mp_context->key_granted  = mp_context->buf[SECURITY_2_KEX_SET_KEYS_POS];
  mp_context->key_exchange = 0x01;
  mp_context->kex_set_byte2 = mp_context->buf[SECURITY_2_KEX_SET_CSA_POS];

  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_CLASS_POS]    = COMMAND_CLASS_SECURITY_2;
  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_POS]          = PUBLIC_KEY_REPORT;
  mp_context->u.inclusion_buf[SECURITY_2_PUB_KEY_INC_FLAG_POS] = SECURITY_2_JOINING_NODE;

  s2_public_key_read(&mp_context->u.inclusion_buf[SECURITY_2_PUB_KEY_KEY_POS]);
  // If any bit besides the KEY_CLASS_S0 or KEY_CLASS_S2_UNAUTHENTICATED is set,
  // then the first two bytes of the public key must be cleared.

  if ((mp_context->inclusion_mode == INCLUSION_MODE_SSA) &&
      (mp_context->key_granted & ~(KEY_CLASS_S0 | KEY_CLASS_S2_UNAUTHENTICATED)))
  {
    mp_context->u.inclusion_buf[SECURITY_2_PUB_KEY_KEY_POS]     = 0x00;
    mp_context->u.inclusion_buf[SECURITY_2_PUB_KEY_KEY_POS + 1] = 0x00;
  }
  mp_context->inclusion_buf_length = SECURITY_2_PUB_KEY_LENGTH;

  m_retry_counter = MAX_RETRY_COUNT;
  s2_inclusion_send_frame();
  s2_inclusion_set_timeout(mp_context, TB3_TIMEOUT);
  return;

error_handling:
  inclusion_failed_frame_send(support, NON_SECURE);
  process_event(S2_INCLUSION_ERROR_SENT);
}

static void s2_joining_start(void)
{
  mp_context->inclusion_peer.tx_options = 0;
}

static void s2_send_echo_kex_set(void)
{
  s2_inclusion_stop_timeout();

  m_retry_counter--;

  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_CLASS_POS]  = COMMAND_CLASS_SECURITY_2;
  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_POS]        = KEX_SET;
  mp_context->u.inclusion_buf[SECURITY_2_KEX_SET_ECHO_POS]   = mp_context->kex_set_byte2 | SECURITY_2_ECHO_ON;
  mp_context->u.inclusion_buf[SECURITY_2_KEX_SET_SCHEME_POS] = mp_context->scheme_support;
  mp_context->u.inclusion_buf[SECURITY_2_KEX_SET_CURVE_POS]  = mp_context->curve_support;
  mp_context->u.inclusion_buf[SECURITY_2_KEX_SET_KEYS_POS]   = mp_context->key_granted;
  mp_context->inclusion_buf_length                           = SECURITY_2_KEX_SET_LENGTH;

  mp_context->inclusion_peer.class_id = TEMP_KEY_SECURE;
    mp_context->inclusion_peer.tx_options = S2_TXOPTION_VERIFY_DELIVERY;

  // Return value is ignored as the retransmission of ECHO frames is based on callback from timer
  // due to unknown user repsonse time and lenghty ECDH calc on remote end.
    S2_send_data(mp_context, &mp_context->inclusion_peer, mp_context->u.inclusion_buf, mp_context->inclusion_buf_length);

  s2_inclusion_set_timeout(mp_context, TB5_TIMEOUT);
}

static void s2_pub_key_a_recv(void)
{
  // Post event upwards to inform that a public key is received and the user should confirm the
  // inclusion of the node.
  zwave_event_t * s2_event;

  s2_inclusion_stop_timeout();

  s2_event = (zwave_event_t *)m_event_buffer;

//  s2_glue_print_str("send_echo_kex_set length: ");
//  s2_glue_print_num(mp_context->length);
//  s2_glue_print_str("send_echo_kex_set length: ");
//  s2_glue_print_num(mp_context->length);

  s2_inclusion_set_timeout(mp_context, TBI1_TIMEOUT);
  // Public key receive.
  memcpy(mp_context->public_key,
         &mp_context->buf[SECURITY_2_PUB_KEY_KEY_POS],sizeof(public_key_t));

  s2_event->event_type = S2_NODE_INCLUSION_PUBLIC_KEY_CHALLENGE_EVENT;
  s2_event->evt.s2_event.peer = mp_context->inclusion_peer;

  // For now, do a raw copy. Design should be cleaned up to ensure proper use of buffers.
  // Remember to strip away the frame header.
  memcpy(s2_event->evt.s2_event.s2_data.challenge_req.public_key,
         &mp_context->buf[SECURITY_2_PUB_KEY_KEY_POS], sizeof(public_key_t));
  s2_event->evt.s2_event.s2_data.challenge_req.length       = sizeof(public_key_t);
  s2_event->evt.s2_event.s2_data.challenge_req.granted_keys = mp_context->key_granted;
  // We do not support Client Side Auth for LR. There are no non-S2 devices to support on LR.
  if ((mp_context->key_granted & (SECURITY_2_SECURITY_2_CLASS_1 | SECURITY_2_SECURITY_2_CLASS_2)) &&
      (mp_context->inclusion_mode == INCLUSION_MODE_CSA))
  {
    s2_event->evt.s2_event.s2_data.challenge_req.dsk_length = DSK_CSA_CHALLENGE_LENGTH;
  }
  else
  {
    s2_event->evt.s2_event.s2_data.challenge_req.dsk_length = 0;
  }
  m_evt_handler(s2_event);

}

static void s2_send_net_key_get(void)
{
  s2_inclusion_stop_timeout();

  if ((KEX_REPORT == mp_context->buf[SECURITY_2_COMMAND_POS]) &&
      (validate_echo_kex_report() != 0))
  {
    process_event(S2_INCLUSION_ERROR_SENT);
    return;
  }
  // If a transfer end is received then it must be ensured that the KEY was verified
  // and that including side has set KeyRequestComplete to '0'.
  else if ((SECURITY_2_TRANSFER_END == mp_context->buf[SECURITY_2_COMMAND_POS]) &&
      (SECURITY_2_KEY_VERIFIED != (mp_context->buf[SECURITY_2_TRANSFER_END_FLAGS_POS] &
          (SECURITY_2_KEY_REQ_COMPLETE | SECURITY_2_KEY_VERIFIED))))
  {
    inclusion_failed_frame_send(KEX_FAIL_KEY_VERIFY, NON_SECURE);
    process_event(S2_INCLUSION_ERROR);
    return;
  }

  if (0 == mp_context->key_granted)
  {
    // No keys granted
    process_event(S2_NO_KEYS_GRANTED);
    return;
  }

  while (0 == (mp_context->key_exchange & mp_context->key_granted))
  {
    mp_context->key_exchange <<= 1;
  }

  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_CLASS_POS]       = COMMAND_CLASS_SECURITY_2;
  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_POS]             = SECURITY_2_NETWORK_KEY_GET;
  mp_context->u.inclusion_buf[SECURITY_2_NET_KEY_GET_REQ_KEY_POS] = mp_context->key_exchange;
  mp_context->inclusion_buf_length                                = SECURITY_2_NET_KEY_GET_LENGTH;

  mp_context->inclusion_peer.class_id = TEMP_KEY_SECURE;
    mp_context->inclusion_peer.tx_options = S2_TXOPTION_VERIFY_DELIVERY;

  m_retry_counter = MAX_RETRY_COUNT;
  s2_inclusion_send_data();
  s2_inclusion_set_timeout(mp_context, TB5_TIMEOUT);
}

static void s2_send_net_key_verify(void)
{
  uint8_t received_key;
#if defined(ZWAVE_PSA_SECURE_VAULT) && defined(ZWAVE_PSA_AES)
  uint8_t *network_key;
#endif

  s2_inclusion_stop_timeout();

  received_key = mp_context->buf[SECURITY_2_NET_KEY_REP_GRANT_KEY_POS];

  if (received_key != mp_context->key_exchange)
  {
        goto error_handling;
  }

  keystore_network_key_write(received_key, &mp_context->buf[SECURITY_2_NET_KEY_REP_KEY_POS]);

  mp_context->key_exchange <<= 1;
  // Update context with new key.
#if defined(ZWAVE_PSA_SECURE_VAULT) && defined(ZWAVE_PSA_AES)
  uint32_t net_key_id;
  net_key_id = convert_key_class_to_psa_key_id(received_key);
  network_key = (uint8_t *)&mp_context->buf[SECURITY_2_NET_KEY_REP_KEY_POS];
    S2_network_key_update(mp_context, net_key_id, NETWORK_KEY_SECURE,
                        network_key, 0, false);
#if !defined(ZW_CONTROLLER)
  /* Do not let key material linger around, clear them from memory */
  memset(network_key, 0, 16);
  network_key = NULL;
#endif
#else
  S2_network_key_update(mp_context, ZWAVE_KEY_ID_NONE, NETWORK_KEY_SECURE,
                       &mp_context->buf[SECURITY_2_NET_KEY_REP_KEY_POS], 0, false);
#endif

  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_CLASS_POS] = COMMAND_CLASS_SECURITY_2;
  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_POS]       = SECURITY_2_NETWORK_KEY_VERIFY;
  mp_context->inclusion_buf_length                          = SECURITY_2_NET_KEY_VERIFY_LENGTH;

  mp_context->inclusion_peer.class_id = NETWORK_KEY_SECURE;
  mp_context->inclusion_peer.tx_options = 0;

  m_retry_counter = MAX_RETRY_COUNT;
  s2_inclusion_send_data();
  s2_inclusion_set_timeout(mp_context, TB7_TIMEOUT);
  return;

error_handling:
  inclusion_failed_frame_send(KEX_FAIL_KEY_REPORT, NON_SECURE);
  process_event(S2_INCLUSION_ERROR_SENT);
}

static void s2_send_final_transfer_end(void)
{
  s2_inclusion_stop_timeout();
  // If a transfer end is received then it must be ensured that the KEY was verified
  // and that including side has set KeyRequestComplete to '0'.
  if ((SECURITY_2_TRANSFER_END == mp_context->buf[SECURITY_2_COMMAND_POS]) &&
      (SECURITY_2_KEY_VERIFIED != (mp_context->buf[SECURITY_2_TRANSFER_END_FLAGS_POS] &
          (SECURITY_2_KEY_REQ_COMPLETE | SECURITY_2_KEY_VERIFIED))))
  {
    inclusion_failed_frame_send(KEX_FAIL_KEY_VERIFY, NON_SECURE);
    process_event(S2_INCLUSION_ERROR);
    return;
  }

  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_CLASS_POS]      = COMMAND_CLASS_SECURITY_2;
  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_POS]            = SECURITY_2_TRANSFER_END;
  mp_context->u.inclusion_buf[SECURITY_2_TRANSFER_END_FLAGS_POS] = SECURITY_2_KEY_REQ_COMPLETE;
  mp_context->inclusion_buf_length                               = SECURITY_2_TRANSFER_END_LENGTH;


  DPRINT("send_final_transfer_end\r\n");
  mp_context->inclusion_peer.class_id = TEMP_KEY_SECURE;
  mp_context->inclusion_peer.tx_options = 0;

  m_retry_counter = MAX_RETRY_COUNT;
  s2_inclusion_send_data();
}

static void s2_joining_complete(zwave_event_codes_t complete_type)
{
    /* Restore the real network key (note: Perhaps postpone until next senddata?) */
  zwave_event_t * s2_event = (zwave_event_t *)m_event_buffer;

  s2_restore_keys(mp_context, true);
  s2_event->event_type        = complete_type;
  s2_event->evt.s2_event.peer = mp_context->inclusion_peer;

  s2_event->evt.s2_event.s2_data.inclusion_complete.exchanged_keys = mp_context->key_granted;

  m_evt_handler(s2_event);
}

#ifdef ZWAVE_PSA_SECURE_VAULT
static void s2_keypair_keyid_read(uint32_t *keyid)
{
  if ((mp_context->inclusion_mode == INCLUSION_MODE_SSA) &&
      (mp_context->key_granted & ~(KEY_CLASS_S2_NOT_VALID | KEY_CLASS_S0 | KEY_CLASS_S2_UNAUTHENTICATED)))
  {
    keystore_keyid_read(keyid);
  }
  else
  {
    keystore_dynamic_keyid_read(keyid);
  }
}

#else // #ifdef ZWAVE_PSA_SECURE_VAULT

/**
 * Read the correct private key depending on granted keys.
 *
 * To avoid a security problem, we only use the primary keypair if any of the authenticated/access keys
 * are among the requested. Otherwise we use the dynamic pair. This ensures that the primary keypair
 * will only ever be sent over the radio with two or more bytes zeroed out.
 * In CSA mode we also use the dynamic keypair because we (the joining side) always send our full
 * public key without padding.
 */
static void s2_private_key_read(uint8_t *buf)
{
  if ((mp_context->inclusion_mode == INCLUSION_MODE_SSA) &&
      (mp_context->key_granted & ~(KEY_CLASS_S2_NOT_VALID | KEY_CLASS_S0 | KEY_CLASS_S2_UNAUTHENTICATED)))
  {
    keystore_private_key_read(buf);
  }
  else
  {
    // keystore will return persisted private key if secure bootstrapping was initiated through SmartStart
    keystore_dynamic_private_key_read(buf);
  }
}
#endif // #ifdef ZWAVE_PSA_SECURE_VAULT

/**
 * Read the correct public key depending on granted keys.
 *
 * We only use the persisted primary keypair if any of the authenticated/access keys
 * are among the requested. Otherwise we use the dynamic keypair. This ensures that the persisted
 * primary keypair will only ever be sent over the radio with two or more bytes zeroed out.
 * In CSA mode we also use the dynamic keypair because we (the joining side) then will send our full
 * public key without padding.
 */
static void s2_public_key_read(uint8_t *buf)
{
  if ((mp_context->inclusion_mode == INCLUSION_MODE_SSA) &&
      (mp_context->key_granted & ~(KEY_CLASS_S2_NOT_VALID | KEY_CLASS_S0 | KEY_CLASS_S2_UNAUTHENTICATED)))
  {
    keystore_public_key_read(buf);
  }
  else
  {
    // keystore will return persisted public key if secure bootstrapping was initiated through SmartStart
    keystore_dynamic_public_key_read(buf);
  }
}

static void s2_do_ecdh_calc_b(void)
{
#ifdef ZWAVE_PSA_SECURE_VAULT
  uint32_t keyid;
  s2_keypair_keyid_read(&keyid);
  zw_status_t status = zw_compute_ecdh_shared_secret(mp_context->public_key, keyid, zwave_shared_secret);
  if (status != ZW_PSA_SUCCESS) {
    zw_security_error(status);
  }
  memcpy(shared_secret, zwave_shared_secret, ZWAVE_ECDH_SECRET_LENGTH);
#else
  s2_private_key_read(&shared_key_mem[LOCAL_PRIVATE_KEY_INDEX]);
  crypto_scalarmult_curve25519(shared_secret, &shared_key_mem[LOCAL_PRIVATE_KEY_INDEX], mp_context->public_key);
#endif
  memcpy(&shared_key_mem[PUBLIC_KEY_A_INDEX], mp_context->public_key, 32);
  s2_public_key_read(&shared_key_mem[PUBLIC_KEY_B_INDEX]);

  tempkey_extract(shared_secret, shared_key_mem, mp_context->public_key);
  S2_network_key_update(mp_context, ZWAVE_KEY_ID_NONE, TEMP_KEY_SECURE, mp_context->public_key, 1, false);

  m_retry_counter = TBI1_TIMEOUT / TB5_TIMEOUT;
  s2_send_echo_kex_set();
}

void s2_inclusion_send_frame(void)
{
  if (0 == S2_send_frame(mp_context,
                         &mp_context->inclusion_peer,
                         mp_context->u.inclusion_buf,
                         mp_context->inclusion_buf_length))
  {
    m_retry_counter = QUEUE_FULL;
  }
  else
  {
    m_retry_counter--;
  }
}

void s2_inclusion_send_data(void)
{
  if (0 == S2_send_data(mp_context,
                        &mp_context->inclusion_peer,
                        mp_context->u.inclusion_buf,
                        mp_context->inclusion_buf_length))
  {
#if defined(DEBUG_S2_INCL_FSM)
    S2_dbg_printf("s2_inclusion_send_data: S2 module was busy. Will retry.\n");
#endif
    m_retry_counter = QUEUE_FULL;
  }
  else
  {
    m_retry_counter--;
  }
}

uint8_t s2_get_key_count(void) {
    return ELEM_COUNT(m_key_slot_pair) + 1;
}

/** Section defining including node functions for state transistion actions - End.
  */
