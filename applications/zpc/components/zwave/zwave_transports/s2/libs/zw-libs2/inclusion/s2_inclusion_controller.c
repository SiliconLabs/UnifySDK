/* © 2022 Silicon Laboratories Inc. */
/*
 * s2_inclusion_controller.c
 *
 */
#include "kderiv.h"
#include "curve25519.h"
#include "s2_protocol.h"
#include "s2_keystore.h"
#include "s2_inclusion_internal.h"
#ifdef ZWAVE_PSA_SECURE_VAULT
#include "s2_psa.h"
#endif

#ifdef ZWAVE_PSA_SECURE_VAULT
extern uint8_t zwave_shared_secret[ZWAVE_ECDH_SECRET_LENGTH];
#endif

//#define DEBUG       // To enable debug_print_hex()
//#define DEBUGPRINT
#ifdef DEBUGPRINT
#include "../../../Components/DebugPrint/DebugPrint.h"
#else
#define DPRINT(PSTRING)
#define DPRINTF(PFORMAT, ...)
#endif

void process_event(uint16_t evt);
void inclusion_failed_frame_send(uint8_t error, uint8_t secure);
void inclusion_failed_evt_push(uint8_t fail_code);
void s2_inclusion_send_frame(void);
void s2_inclusion_send_data(void);

static void s2_send_kex_get(void);
static void s2_kex_rep_recv(void);
static void s2_send_kex_set(void);
static void s2_send_pub_key_a(void);
static void s2_send_echo_kex_report(void);
static void s2_send_net_key_report(void);
static void s2_send_transfer_end(void);
static void s2_inclusion_complete(void);
static void s2_do_ecdh_calc_a(void);
static void s2_check_no_keys(void);

typedef enum {
  S2_SEND_KEX_GET_ACTION = S2_ACTION_LAST,
  S2_KEX_REP_RECV_ACTION,
  S2_SEND_KEX_SET_ACTION,
  S2_SEND_PUB_KEY_A_ACTION,
  S2_DO_ECDH_CALC_A_ACTION,
  S2_SEND_ECHO_KEX_REPORT_ACTION,
  S2_SEND_ERROR_CANCEL_ACTION,
  S2_SEND_ERROR_DECRYPT_ACTION,
  S2_SEND_NET_KEY_REPORT_ACTION,
  S2_SEND_TRANSFER_END_ACTION,
  S2_INCLUSION_COMPLETE_ACTION,
  S2_CHECK_NO_KEYS,
} s2_action_controller_t;

const s2_transition_t s2_transition_table_controller[] = {
  {S2_INC_IDLE,                 S2_INCLUDING_START,         S2_SEND_KEX_GET_ACTION,           S2_AWAITING_KEX_REPORT},
  {S2_AWAITING_KEX_REPORT,      S2_KEX_REPORT_RECV,         S2_KEX_REP_RECV_ACTION,           S2_AWAITING_KEY_USER_ACCEPT},
  {S2_AWAITING_KEX_REPORT,      S2_INCLUSION_RETRY,         S2_RESEND_FRAME_ACTION,           S2_INC_STATE_ANY},
  {S2_AWAITING_KEY_USER_ACCEPT, S2_INCLUDING_ACCEPT,        S2_SEND_KEX_SET_ACTION,           S2_AWAITING_PUB_KEY_B},
  {S2_AWAITING_KEY_USER_ACCEPT, S2_INCLUDING_REJECT,        S2_SEND_ERROR_CANCEL_ACTION,      S2_ERROR_SENT},
  {S2_AWAITING_KEY_USER_ACCEPT, S2_KEX_REPORT_RECV,         S2_NO_ACTION,                     S2_INC_STATE_ANY},
  {S2_AWAITING_PUB_KEY_B,       S2_KEX_REPORT_RECV,         S2_NO_ACTION,                     S2_INC_STATE_ANY},
  {S2_AWAITING_PUB_KEY_B,       S2_INCLUSION_RETRY,         S2_RESEND_FRAME_ACTION,           S2_INC_STATE_ANY},
  {S2_AWAITING_PUB_KEY_B,       S2_PUB_KEY_RECV_B,          S2_SEND_PUB_KEY_A_ACTION,         S2_AWAITING_USER_ACCEPT},

  {S2_AWAITING_USER_ACCEPT,     S2_INCLUDING_ACCEPT,          S2_DO_ECDH_CALC_A_ACTION,           S2_AWAITING_ECHO_KEX_SET},
  {S2_AWAITING_USER_ACCEPT,     S2_INCLUDING_REJECT,          S2_SEND_ERROR_CANCEL_ACTION,        S2_ERROR_SENT},
  {S2_AWAITING_USER_ACCEPT,     S2_INCLUSION_RETRY,           S2_RESEND_FRAME_ACTION,             S2_INC_STATE_ANY},
  {S2_AWAITING_USER_ACCEPT,     S2_KEX_REPORT_RECV,           S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_USER_ACCEPT,     S2_PUB_KEY_RECV_B,            S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_ECHO_KEX_SET,    S2_ECHO_KEX_SET_RECV,         S2_SEND_ECHO_KEX_REPORT_ACTION,     S2_AWAITING_NET_KEY_GET},
  {S2_AWAITING_ECHO_KEX_SET,    S2_INCLUSION_RETRY,           S2_RESEND_FRAME_ACTION,              S2_INC_STATE_ANY},
  {S2_AWAITING_ECHO_KEX_SET,    S2_INCLUDING_DECRYPT_FAILED,  S2_SEND_ERROR_DECRYPT_ACTION,       S2_ERROR_SENT},
  {S2_AWAITING_ECHO_KEX_SET,    S2_KEX_REPORT_RECV,           S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_ECHO_KEX_SET,    S2_PUB_KEY_RECV_B,            S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_NET_KEY_GET,     S2_NET_KEY_GET_RECV,          S2_SEND_NET_KEY_REPORT_ACTION,      S2_AWAITING_NET_KEY_VERIFY},
  {S2_AWAITING_NET_KEY_GET,     S2_SEND_FINAL_TRANSFER_END,   S2_CHECK_NO_KEYS,                   S2_INC_IDLE},
  {S2_AWAITING_NET_KEY_GET,     S2_INCLUSION_RETRY,           S2_RESEND_DATA_ACTION,              S2_INC_STATE_ANY},
  {S2_AWAITING_NET_KEY_GET,     S2_KEX_REPORT_RECV,           S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_NET_KEY_GET,     S2_PUB_KEY_RECV_B,            S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_NET_KEY_GET,     S2_ECHO_KEX_SET_RECV,         S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_NET_KEY_GET,     S2_NET_KEY_VERIFY_RECV,       S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_NET_KEY_VERIFY,  S2_NET_KEY_VERIFY_FINAL_RECV, S2_SEND_TRANSFER_END_ACTION,        S2_AWAITING_TRANSFER_END},
  {S2_AWAITING_NET_KEY_VERIFY,  S2_NET_KEY_VERIFY_RECV,       S2_SEND_TRANSFER_END_ACTION,        S2_AWAITING_NET_KEY_GET},
  {S2_AWAITING_NET_KEY_VERIFY,  S2_INCLUSION_RETRY,           S2_RESEND_DATA_ACTION,              S2_INC_STATE_ANY},
  {S2_AWAITING_NET_KEY_VERIFY,  S2_KEX_REPORT_RECV,           S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_NET_KEY_VERIFY,  S2_PUB_KEY_RECV_B,            S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_NET_KEY_VERIFY,  S2_ECHO_KEX_SET_RECV,         S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_NET_KEY_VERIFY,  S2_NET_KEY_GET_RECV,          S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_TRANSFER_END,    S2_TRANSFER_END_RECV,         S2_INCLUSION_COMPLETE_ACTION,       S2_INC_IDLE},
  {S2_AWAITING_TRANSFER_END,    S2_INCLUSION_RETRY,           S2_RESEND_DATA_ACTION,              S2_INC_STATE_ANY},
  {S2_AWAITING_TRANSFER_END,    S2_KEX_REPORT_RECV,           S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_TRANSFER_END,    S2_PUB_KEY_RECV_B,            S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_TRANSFER_END,    S2_ECHO_KEX_SET_RECV,         S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_TRANSFER_END,    S2_NET_KEY_GET_RECV,          S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_TRANSFER_END,    S2_NET_KEY_VERIFY_RECV,       S2_NO_ACTION,                       S2_INC_STATE_ANY},
  {S2_AWAITING_TRANSFER_END,    S2_NET_KEY_VERIFY_FINAL_RECV, S2_NO_ACTION,                       S2_INC_STATE_ANY},
};

const size_t s2_transition_table_controller_length = ELEM_COUNT(s2_transition_table_controller);

void execute_action_controller(uint8_t action)
{
  const s2_action_controller_t ctrl_action = (s2_action_controller_t)action;

  switch (ctrl_action)
  {
    case S2_CHECK_NO_KEYS:
      s2_check_no_keys();
    break;

    case S2_SEND_KEX_GET_ACTION:
      s2_send_kex_get();
    break;

    case S2_KEX_REP_RECV_ACTION:
      s2_kex_rep_recv();
    break;

    case S2_SEND_KEX_SET_ACTION:
      s2_send_kex_set();
    break;

    case S2_SEND_PUB_KEY_A_ACTION:
      s2_send_pub_key_a();
    break;

    case S2_DO_ECDH_CALC_A_ACTION:
      s2_do_ecdh_calc_a();
    break;

    case S2_SEND_ECHO_KEX_REPORT_ACTION:
      s2_send_echo_kex_report();
    break;

    case S2_SEND_ERROR_DECRYPT_ACTION:
      inclusion_failed_frame_send(KEX_FAIL_DECRYPT, NON_SECURE);
    break;

    case S2_SEND_ERROR_CANCEL_ACTION:
      inclusion_failed_frame_send(KEX_FAIL_CANCEL, NON_SECURE);
    break;

    case S2_SEND_NET_KEY_REPORT_ACTION:
      s2_send_net_key_report();
    break;

    case S2_SEND_TRANSFER_END_ACTION:
      s2_send_transfer_end();
    break;

    case S2_INCLUSION_COMPLETE_ACTION:
      s2_inclusion_complete();
    break;

    default:
    break;
  }
}

/** Section defining validation functions for including node - Begin.
 */
static void s2_do_ecdh_calc_a(void)
{
#ifdef ZWAVE_PSA_SECURE_VAULT
  zw_status_t status = zw_compute_inclusion_ecdh_shared_secret(mp_context->public_key, zwave_shared_secret);
  if (status != ZW_PSA_SUCCESS) {
    zw_security_error(status);
  }
  memcpy(shared_secret, zwave_shared_secret, ZWAVE_ECDH_SECRET_LENGTH);
#else
  keystore_private_key_read(&shared_key_mem[LOCAL_PRIVATE_KEY_INDEX]);
  crypto_scalarmult_curve25519(shared_secret, &shared_key_mem[LOCAL_PRIVATE_KEY_INDEX], mp_context->public_key);
#endif
  memcpy(&shared_key_mem[PUBLIC_KEY_B_INDEX], mp_context->public_key, 32);
  keystore_public_key_read(&shared_key_mem[PUBLIC_KEY_A_INDEX]);

  tempkey_extract(shared_secret, shared_key_mem, mp_context->public_key);
  S2_network_key_update(mp_context, ZWAVE_KEY_ID_NONE, TEMP_KEY_SECURE, mp_context->public_key, 1, false);
}

/**@brief Function for validating the schemes supported in a KEX Report frame.
 *
 * @param[in,out] context Pointer to the context which contains the received buffer. Context will
 *                be updated with schemes supported by joining node.
 *
 * @retval 0                  If a valid set of schemes is requested.
 * @retval KEX_FAIL if no supported scheme is found or invalid set of schemes are requested.
 */
static uint8_t validate_reported_schemes(void)
{
  mp_context->scheme_support = (mp_context->buf[SECURITY_2_KEX_REP_SCHEME_POS] & SECURITY_2_SCHEME_SUPPORT_MASK);

  if (mp_context->scheme_support & SECURITY_2_SCHEME_1_SUPPORT)
  {
    return 0;
  }

  inclusion_failed_frame_send(KEX_FAIL_KEX_SCHEME, NON_SECURE);

  return KEX_FAIL;
}

/**@brief Function for validating the schemes supported by a joining node.
 *
 * @param[in,out] context Pointer to the context which contains the received buffer. Context will
 *                be updated with curves supported by joining node.
 *
 * @retval 0                  If the requested curve is supported.
 * @retval KEX_FAIL if no supported curve is found .
 */
static uint8_t validate_reported_curves(void)
{
  mp_context->curve_support = (mp_context->buf[SECURITY_2_KEX_REP_CURVE_POS] & KEX_REPORT_CURVE_MASK);

  if (mp_context->curve_support & KEX_REPORT_CURVE_25519)
  {
    return 0;
  }

  inclusion_failed_frame_send(KEX_FAIL_KEX_CURVES, NON_SECURE);

  return KEX_FAIL;
}

/**@brief Function for validating that requested keys matches the supported schemes.
 *
 * @param[in,out] context Pointer to the context which contains the received buffer. Context will
 *                be updated with curves supported by joining node.
 *
 * @retval 0                  If the requested curve is supported.
 * @retval KEX_FAIL if no supported curve is found .
 */
static uint8_t validate_reported_keys(void)
{
  // Store reported keys for echoing back unmodified in Kex Report Echo
  mp_context->kex_report_keys = mp_context->buf[SECURITY_2_KEX_REP_KEYS_POS];
  mp_context->key_exchange = mp_context->buf[SECURITY_2_KEX_REP_KEYS_POS];
 /* If LR inclusion, filter out invalid keys */
  if (IS_LR_NODE(mp_context->inclusion_peer.r_node)) {
    mp_context->key_exchange &= ~(KEY_CLASS_S2_UNAUTHENTICATED | KEY_CLASS_S0);
  }
  mp_context->key_granted  = mp_context->key_exchange & SECURITY_2_KEY_MASK; /* Why not mask ->key_exchange? */

  if (mp_context->key_granted != 0)
  {
    return 0;
  }

  inclusion_failed_frame_send(KEX_FAIL_KEX_KEY, NON_SECURE);

  return KEX_FAIL;
}

/**@brief Function for validating that the requested key matches one the granted keys in KEX Set
 *        frame earlier.
 *
 * @param[in,out] context Pointer to the context which contains the received buffer.
 *
 * @retval 0                  If the requested key is granted..
 * @retval KEX_FAIL if the key requested has not been granted.
 */
static uint8_t validate_requested_key(void)
{
  uint8_t req_key = mp_context->buf[SECURITY_2_NET_KEY_GET_REQ_KEY_POS];
 
  // Test that only one bit is set and that the bit corresponds to a granted key.
  if ((0 == (req_key & (req_key - 1))) &&
      (req_key & mp_context->key_granted))  // key_granted is already in LR format because it was converted in s2_inclusion_key_grant()
  {
    mp_context->key_requested = req_key;
    return 0;
  }

  mp_context->key_requested = 0;
  inclusion_failed_frame_send(KEX_FAIL_KEY_GET, NON_SECURE);

  return KEX_FAIL;

}

/**@brief Function for validating that the echo kex set frame is identical to the transmitted
 *        frame earlier.
 *
 * @param[in,out] context Pointer to the context which contains the received buffer.
 *
 * @retval 0                  If the echo frame matched the original frame.
 * @retval KEX_FAIL If the echo frame did not match the original frame.
 */
static uint8_t validate_echo_kex_set(void)
{
  // Test that only one bit is set and that the bit corresponds to a granted key.
  if ((mp_context->scheme_support == mp_context->buf[SECURITY_2_KEX_SET_SCHEME_POS]) &&
      (mp_context->curve_support  == mp_context->buf[SECURITY_2_KEX_SET_CURVE_POS]) &&
      (mp_context->key_granted    == mp_context->buf[SECURITY_2_KEX_SET_KEYS_POS]) &&
      (mp_context->inclusion_mode == ((mp_context->buf[SECURITY_2_KEX_SET_ECHO_POS] & SECURITY_2_CSA_ON) == SECURITY_2_CSA_ON) ? INCLUSION_MODE_CSA : INCLUSION_MODE_SSA)
  )
  {
    return 0;
  }

  inclusion_failed_frame_send(KEX_FAIL_AUTH, NON_SECURE);

  return KEX_FAIL;
}

/** Section defining validation functions for including node - End.
 */

/** Section defining including node functions for state transistion actions - Begin.
 */
static void s2_send_kex_get(void)
{
  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_CLASS_POS] = COMMAND_CLASS_SECURITY_2;
  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_POS]       = KEX_GET;
  mp_context->inclusion_buf_length                          = SECURITY_2_KEX_GET_LENGTH;

  m_retry_counter = MAX_RETRY_COUNT;
  s2_inclusion_send_frame();
  s2_inclusion_set_timeout(mp_context, TA1_TIMEOUT);
}

static void s2_kex_rep_recv(void)
{
  uint8_t         ret_val;
  zwave_event_t * s2_event;

  s2_inclusion_stop_timeout();

  ret_val =  validate_reported_schemes();
  if (ret_val != 0)
  {
    process_event(S2_INCLUSION_ERROR_SENT);
    return;
  }

  ret_val = validate_reported_curves();
  if (ret_val != 0)
  {
    process_event(S2_INCLUSION_ERROR_SENT);
    return;
  }

  ret_val = validate_reported_keys();
  if (ret_val != 0)
  {
    process_event(S2_INCLUSION_ERROR_SENT);
    return;
  }
  mp_context->csa_support = mp_context->buf[SECURITY_2_KEX_REP_ECHO_POS];

  s2_event = (zwave_event_t *)m_event_buffer;
  s2_event->event_type                                    = S2_NODE_INCLUSION_KEX_REPORT_EVENT;
  s2_event->evt.s2_event.peer                             = mp_context->inclusion_peer;
  s2_event->evt.s2_event.s2_data.kex_report.security_keys = mp_context->key_granted;
  s2_event->evt.s2_event.s2_data.kex_report.csa = (mp_context->buf[SECURITY_2_KEX_REP_ECHO_POS] & 2) !=0;
  m_evt_handler(s2_event);

  s2_inclusion_set_timeout(mp_context, TAI1_TIMEOUT);
}

static void s2_send_kex_set(void)
{
  s2_inclusion_stop_timeout();

  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_CLASS_POS]  = COMMAND_CLASS_SECURITY_2;
  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_POS]        = KEX_SET;
  mp_context->u.inclusion_buf[SECURITY_2_KEX_SET_ECHO_POS]   = SECURITY_2_ECHO_OFF;
  mp_context->u.inclusion_buf[SECURITY_2_KEX_SET_SCHEME_POS] = SECURITY_2_SCHEME_1_SUPPORT;
  mp_context->u.inclusion_buf[SECURITY_2_KEX_SET_CURVE_POS]  = KEX_REPORT_CURVE_25519;
  mp_context->u.inclusion_buf[SECURITY_2_KEX_SET_KEYS_POS]   = mp_context->key_granted;
  mp_context->inclusion_buf_length                           = SECURITY_2_KEX_SET_LENGTH;

  if(mp_context->inclusion_mode == INCLUSION_MODE_CSA) {
    mp_context->u.inclusion_buf[SECURITY_2_KEX_SET_ECHO_POS] |= SECURITY_2_CSA_ON;
  }
  m_retry_counter = MAX_RETRY_COUNT;
  s2_inclusion_send_frame();
  s2_inclusion_set_timeout(mp_context, TA2_TIMEOUT);
}

static void s2_send_transfer_end(void)
{
  s2_inclusion_stop_timeout();

  // A network key verify frame has been received.
  // This packet has been successfully decrypted, thus we must update the context to use temp key for sending transfer end.
  S2_network_key_update(mp_context, ZWAVE_KEY_ID_NONE, TEMP_KEY_SECURE, mp_context->public_key, 1, false);

  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_CLASS_POS]      = COMMAND_CLASS_SECURITY_2;
  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_POS]            = SECURITY_2_TRANSFER_END;
  mp_context->u.inclusion_buf[SECURITY_2_TRANSFER_END_FLAGS_POS] = SECURITY_2_KEY_VERIFIED;
  mp_context->inclusion_buf_length                               = SECURITY_2_TRANSFER_END_LENGTH;

  mp_context->inclusion_peer.class_id = TEMP_KEY_SECURE;
  mp_context->inclusion_peer.tx_options = 0;

  m_retry_counter = MAX_RETRY_COUNT;
  s2_inclusion_send_data();
  s2_inclusion_set_timeout(mp_context, TA6_TIMEOUT);
}

static void s2_send_pub_key_a(void)
{
  // Post event upwards to inform that a public key is received and the user should confirm the
  // inclusion of the node.
  zwave_event_t * s2_event;

  s2_inclusion_stop_timeout();

  s2_event = (zwave_event_t *)m_event_buffer;
  s2_event->event_type = S2_NODE_INCLUSION_PUBLIC_KEY_CHALLENGE_EVENT;
  s2_event->evt.s2_event.peer = mp_context->inclusion_peer;

  // For now, do a raw copy. Design should be cleaned up to ensure proper use of buffers.
  // Remember to strip away the frame header.
  memcpy(s2_event->evt.s2_event.s2_data.challenge_req.public_key,
         &mp_context->buf[SECURITY_2_PUB_KEY_KEY_POS], sizeof(public_key_t));
  s2_event->evt.s2_event.s2_data.challenge_req.length = sizeof(public_key_t);
  s2_event->evt.s2_event.s2_data.challenge_req.granted_keys = mp_context->key_granted;
  if ((mp_context->key_granted & (SECURITY_2_SECURITY_2_CLASS_1 | SECURITY_2_SECURITY_2_CLASS_2)) &&
      (mp_context->inclusion_mode == INCLUSION_MODE_SSA))
  {
    s2_event->evt.s2_event.s2_data.challenge_req.dsk_length = DSK_SSA_CHALLENGE_LENGTH;
  }
  else
  {
    s2_event->evt.s2_event.s2_data.challenge_req.dsk_length = 0;
  }
  memcpy(mp_context->public_key,
         &mp_context->buf[SECURITY_2_PUB_KEY_KEY_POS],
         sizeof(public_key_t));

  m_evt_handler(s2_event);

  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_CLASS_POS]    = COMMAND_CLASS_SECURITY_2;
  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_POS]          = PUBLIC_KEY_REPORT;
  mp_context->u.inclusion_buf[SECURITY_2_PUB_KEY_INC_FLAG_POS] = SECURITY_2_INCLUDING_NODE;
  mp_context->inclusion_buf_length                             = SECURITY_2_PUB_KEY_LENGTH;
  keystore_public_key_read(&mp_context->u.inclusion_buf[SECURITY_2_PUB_KEY_KEY_POS]);

  if ((mp_context->inclusion_mode == INCLUSION_MODE_CSA) &&
      (mp_context->key_granted & ~(KEY_CLASS_S0 | KEY_CLASS_S2_UNAUTHENTICATED)))
  {
    mp_context->u.inclusion_buf[SECURITY_2_PUB_KEY_KEY_POS]     = 0x00;
    mp_context->u.inclusion_buf[SECURITY_2_PUB_KEY_KEY_POS + 1] = 0x00;
    mp_context->u.inclusion_buf[SECURITY_2_PUB_KEY_KEY_POS + 2] = 0x00;
    mp_context->u.inclusion_buf[SECURITY_2_PUB_KEY_KEY_POS + 3] = 0x00;
  }

  m_retry_counter = MAX_RETRY_COUNT;
  s2_inclusion_send_frame();
  s2_inclusion_set_timeout(mp_context, TAI2_TIMEOUT);
}

static void s2_send_echo_kex_report(void)
{
  uint8_t ret_val;

  s2_inclusion_stop_timeout();

  ret_val = validate_echo_kex_set();
  if (ret_val != 0)
  {
    process_event(S2_INCLUSION_ERROR_SENT);
    return;
  }

  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_CLASS_POS]  = COMMAND_CLASS_SECURITY_2;
  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_POS]        = KEX_REPORT;
  mp_context->u.inclusion_buf[SECURITY_2_KEX_REP_ECHO_POS]   = mp_context->csa_support | SECURITY_2_ECHO_ON;
  mp_context->u.inclusion_buf[SECURITY_2_KEX_REP_SCHEME_POS] = mp_context->scheme_support;
  mp_context->u.inclusion_buf[SECURITY_2_KEX_REP_CURVE_POS]  = mp_context->curve_support;
  mp_context->u.inclusion_buf[SECURITY_2_KEX_REP_KEYS_POS]   = mp_context->kex_report_keys;
  mp_context->inclusion_buf_length                           = SECURITY_2_KEX_REPORT_LENGTH;
  mp_context->key_exchange = 0x00;

  mp_context->inclusion_peer.class_id = TEMP_KEY_SECURE;
  mp_context->inclusion_peer.tx_options = S2_TXOPTION_VERIFY_DELIVERY;

  m_retry_counter = MAX_RETRY_COUNT;
  s2_inclusion_send_data();
  s2_inclusion_set_timeout(mp_context, TA4_TIMEOUT);
}

/**
 * When bootstrapping an LR node, we actually want to transmit different S2 keys than requested
 * on the radio frames. This function does the translation.
 * In other words, the Access key requested by an LR node is different from the Access key
 * requested by a normal node. Same for the Authenticated key.
 */
static uint8_t translate_keyslot_to_read_for_LR(uint8_t requested_keyslot, node_t peer_nodeid)
{
  (void)requested_keyslot;
  (void)peer_nodeid;

  uint8_t actual_keyslot = mp_context->u.inclusion_buf[SECURITY_2_NET_KEY_REP_GRANT_KEY_POS];
  // Read other keys here in case of LR
  // Note: This logic cannot handle bootstrapping controllers requesing BOTH LR and normal keys
  // It will have to be expanded to include a check for that. Luckily, we are not required
  // to support that yet.
  if (IS_LR_NODE(mp_context->inclusion_peer.r_node)) {
    switch(actual_keyslot) {
      case KEY_CLASS_S2_AUTHENTICATED:
        actual_keyslot = KEY_CLASS_S2_AUTHENTICATED_LR;
        break;

      case KEY_CLASS_S2_ACCESS:
        actual_keyslot = KEY_CLASS_S2_ACCESS_LR;
        break;

      default:
        break;
    }
  }
  return actual_keyslot;
}

static void s2_send_net_key_report(void)
{
  uint8_t ret_val;

  s2_inclusion_stop_timeout();

  ret_val = validate_requested_key();
  if (ret_val != 0)
  {
    process_event(S2_INCLUSION_ERROR_SENT);
    return;
  }
  mp_context->key_exchange |= mp_context->buf[SECURITY_2_NET_KEY_GET_REQ_KEY_POS];

  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_CLASS_POS]         = COMMAND_CLASS_SECURITY_2;
  mp_context->u.inclusion_buf[SECURITY_2_COMMAND_POS]               = SECURITY_2_NETWORK_KEY_REPORT;
  mp_context->u.inclusion_buf[SECURITY_2_NET_KEY_REP_GRANT_KEY_POS] = mp_context->buf[SECURITY_2_NET_KEY_GET_REQ_KEY_POS];
  mp_context->inclusion_buf_length                                  = SECURITY_2_NET_KEY_REPORT_LENGTH;

  uint8_t keyslot_to_read = translate_keyslot_to_read_for_LR(
      mp_context->u.inclusion_buf[SECURITY_2_NET_KEY_REP_GRANT_KEY_POS],
      mp_context->inclusion_peer.r_node);

  keystore_network_key_read(keyslot_to_read,
                            &mp_context->u.inclusion_buf[SECURITY_2_NET_KEY_REP_KEY_POS]);

  mp_context->inclusion_peer.class_id = TEMP_KEY_SECURE;
  mp_context->inclusion_peer.tx_options = 0;

  m_retry_counter = MAX_RETRY_COUNT;
  s2_inclusion_send_data();
  s2_inclusion_set_timeout(mp_context, TA5_TIMEOUT);

  // After tranmitting the Network key, we expect the Netkey verify to be transmitted using that key.
  // Thus we must update the context.

  if(mp_context->u.inclusion_buf[SECURITY_2_NET_KEY_REP_GRANT_KEY_POS] == SECURITY_2_SECURITY_0_NETWORK_KEY) {
      // After tranmitting the Network key, we expect the Netkey verify to be transmitted using that key.
      // Thus we must update the context.
      S2_network_key_update(mp_context, ZWAVE_KEY_ID_NONE, NETWORK_KEY_SECURE, &mp_context->u.inclusion_buf[3], 0, false);
  }
}

static void s2_inclusion_complete(void)
{
  // Post event upwards to inform that inclusion of noda A has successfully completed.
  zwave_event_t * s2_event;

  //Check that the final transfer end is ok
  if ((SECURITY_2_TRANSFER_END == mp_context->buf[SECURITY_2_COMMAND_POS]) &&
      (SECURITY_2_KEY_REQ_COMPLETE == (mp_context->buf[SECURITY_2_TRANSFER_END_FLAGS_POS] &
          (SECURITY_2_KEY_REQ_COMPLETE | SECURITY_2_KEY_VERIFIED))))
  {
    s2_inclusion_stop_timeout();
    s2_restore_keys(mp_context, true);

    s2_event = (zwave_event_t *)m_event_buffer;
    s2_event->event_type                                             = S2_NODE_INCLUSION_COMPLETE_EVENT;
    s2_event->evt.s2_event.peer                                      = mp_context->inclusion_peer;
    s2_event->evt.s2_event.s2_data.inclusion_complete.exchanged_keys = mp_context->key_exchange;
    m_evt_handler(s2_event);
  } else {
    inclusion_failed_frame_send(KEX_FAIL_KEY_VERIFY,NON_SECURE);
    inclusion_failed_evt_push(KEX_FAIL_KEY_VERIFY);
  }
}

static void s2_check_no_keys(void) {
  if(mp_context->key_granted) {
    inclusion_failed_evt_push(mp_context->kex_fail_code);
  } else {
    s2_inclusion_complete();
  }
}

void s2_inclusion_key_grant(struct S2 *p_context, uint8_t include, uint8_t keys, uint8_t csa)
{
  MP_CTX_DEF

  if (ACCEPT_INCLUSION == include)
  {
    mp_context->inclusion_mode = csa ? INCLUSION_MODE_CSA : INCLUSION_MODE_SSA;
    mp_context->key_granted = keys;
    process_event(S2_INCLUDING_ACCEPT);
  }
  else
  {
    process_event(S2_INCLUDING_REJECT);
  }
}


void s2_inclusion_including_start(struct S2 *p_context, const s2_connection_t* conn)
{
  MP_CTX_DEF

  if(mp_context->inclusion_state == S2_INC_IDLE)
  {
    mp_context->inclusion_peer = *conn;
    mp_context->inclusion_peer.tx_options = 0;
    mp_context->inclusion_mode = INCLUSION_MODE_SSA;
    mp_context->kex_fail_code = 0;
    process_event(S2_INCLUDING_START);
    mp_context->is_keys_restored = false;
  }
}
