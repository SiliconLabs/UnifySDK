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

//#define DEBUGPRINT

#ifdef DEBUGPRINT
#include "../../../Components/DebugPrint/DebugPrint.h"
#else
#define DPRINT(PSTRING)
#define DPRINTF(PFORMAT, ...)
#endif

#define CHECK_AND_FAIL(CHECK, FAILURE)   if ((CHECK)){ return (FAILURE); }

/** Following defines are specified in SDS11274 for node inclusion. */
#define SECONDS_10   1000          //< Definition of 10 Seconds in 10ms resolution.
#define SECONDS_30   3000          //< Definition of 30 Seconds in 10ms resolution.
#define SECONDS_60   6000          //< Definition of 60 Seconds in 10ms resolution.
#define SECONDS_240  24000         //< Definition of 240 Seconds in 10ms resolution.
#define TA1_TIMEOUT  SECONDS_10    //< Timeout TA1 is used after sending KEX Get and is the time until KEX Report must be received from the joining node.
#define TA2_TIMEOUT  SECONDS_10    //< Timeout TA2 is used after sending KEX Set and is the time until Public Key B must be received from the joining node.
#define TA3_TIMEOUT  SECONDS_30    //< Timeout TA3 is used after sending Public Key A and is the time until Echo(KEX Get) must be received from the joining node.
#define TA4_TIMEOUT  SECONDS_10    //< Timeout TA4 is used after sending Echo(KEX Report) and is the time until Network Key Get must be received from the joining node.
#define TA5_TIMEOUT  SECONDS_10    //< Timeout TA5 is used after sending Network Key Set and is the time until Network Key Verify must be received from the joining node.
#define TA6_TIMEOUT  SECONDS_10    //< Timeout TA6 is used after sending Transfer End and is the time until Transfer End or second key exchange round is initiated by a Network Key Get has been received from the joining node.
#define TAI1_TIMEOUT SECONDS_240   //< Timeout TI1 is used after receiving public key and this the time in wich the user must accept/reject secure inclusion of the node.
#define TAI2_TIMEOUT SECONDS_240   //< Timeout TA6 is used after receiving public key B and this is the time in wich the user must provide/reject the missing part of the DSK.
#define TB0_TIMEOUT  SECONDS_60    //< Timeout TB0 is used after non-secure inclusion has completed and the time until neighbor discovery has been completed..
#define TB1_TIMEOUT  SECONDS_30    //< Timeout TB1 is used after neighbor discovery has completed and the time until KEX Get must be received from the including node.
#define TB2_TIMEOUT  SECONDS_240    //< Timeout TB2 is used after sending KEX Report and is the time until KEX Set must be received from the including node.
#define TB3_TIMEOUT  SECONDS_10    //< Timeout TB3 is used after sending Public Key B and is the time until Public Key A must be received from the including node.
#define TB5_TIMEOUT  SECONDS_10    //< Timeout TB5 is used after sending Echo(KEX Set) and is the time until Echo(KEX Report) must be received from the including node.
#define TB6_TIMEOUT  SECONDS_10    //< Timeout TB6 is used after sending Network Key Get and is the time until Network key report must be received from the including node.
#define TB7_TIMEOUT  SECONDS_10    //< Timeout TB7 is used after sending Network Key Verify  and is the time until Transfer End must be received from the including node.
#define TBI1_TIMEOUT SECONDS_240   //< Timeout TI1 is after receiving public key and tis the time in wich the user must accept/reject secure inclusion of the node.

#define MAX_PUBLIC_KEY_LENGTH 32   //< Maximum length of supported public key. Currently only curve 25519 for ECDH is supported, thus key length is fixed to 32 bytes.
#define KEX_ECHO_BIT_POS      8    //< Bit position in event enumeration when identifying an echo frame event.
#define PUBLIC_KEY_INCLUDING_NODE_BIT_POS   8    //< Bit position in event enumeration when identifying including node in public key frame event.

/** TODO: Investigate if there is/should be a common header for util functions as this.
 */
#define ELEM_COUNT(ARRAY)          (sizeof(ARRAY)/(sizeof(ARRAY[0])))
#define S2_EVT_BUFFER_SIZE         (128 / 4) //< Max event buffer in s2 inclusion model. Buffer is uint32_t to ensure proper alignment therefor size is divided by 4.

#define LOCAL_PUSH                  0xFE    //< Value identifying that the frame should be sent non-secure.
#define NON_SECURE                  0xFF    //< Value identifying that the frame should be sent non-secure.
#define SINGLE_RULE_SUPPORT         0       //< Only a single encryption rule applies to this frametype.
#define DUAL_RULE_SUPPORT           1       //< Two encryption rules applies to this frametype.
#define ECHO_SUPPORT                2       //< This frame type support echo frames, where the encryption rule depends on whether the frame is an echo.
#define LOCAL_PRIVATE_KEY_INDEX     0       //< Index in auth tag array for local private key.
#define PUBLIC_KEY_A_INDEX          0       //< Index in auth tag array for public key A.
#define PUBLIC_KEY_B_INDEX          32      //< Index in auth tag array for public key B.
#define ECDH_PUBLIC_KEY_SIZE        32      //< Size of an ECDH public key.
#define NETWORK_KEY_SIZE            16      //< Size of a Z-Wave network key.
#define ACCEPT_INCLUSION            1       //< Value defining inclusion acceptance.
#define S2_INCLUSION_COMMAND_OFFSET KEX_GET //< Offset from zero where the numbering of inclusion related commands begin. Kex get is the first valid command and hence defines the offset.
#define LENGTH_RULE_INDEX           0       //< Index in frame rules array where minimum length of the received frame type.
#define ENCRYPTION_SUPPORT_INDEX    1       //< Index in frame rules array where rules on how a received packet should be encrypted in order to be accepted.
#define ENCRYPTION_RULE_INDEX_A     2       //< Encryption rule index for first rule. Index rule A is also used for non-echo frames.
#define ENCRYPTION_RULE_INDEX_B     3       //< Encryption rule index for second rule. Index rule B is also used for echo frames.
#define NO_RETRIES                  0x00    //< Indication that state has changed and retries are disabled. States supporting retries must set the retry counter accordingly.
#define QUEUE_FULL                  0xFF    //< Indication that a frame could not be transmitted and that a send done/failed should result in retry event.
#define MAX_RETRY_COUNT             0x03

/** Forward declarations of transition actions to be able to define the state machine flow.
 *  Look in bottom of file for actual implementation of actions.
 */
#ifdef ZW_CONTROLLER // Those functions used during transistions are only supported by a controller.
static void s2_send_kex_get(void);
static void s2_kex_rep_recv(void);
static void s2_send_kex_set(void);
static void s2_send_pub_key_a(void);
static void s2_send_echo_kex_report(void);
static void s2_send_net_key_report(void);
static void s2_send_transfer_end(void);
static void s2_inclusion_complete(void);
static void s2_do_ecdh_calc_a(void);
static void s2_check_no_keys();
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
static void inclusion_failed_frame_send(uint8_t error, uint8_t secure);
static void remote_inclusion_failed_evt_push(void);
static void inclusion_failed_evt_push(uint8_t fail_code);
static void s2_inclusion_send_frame(void);
static void s2_inclusion_send_data(void);

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
typedef enum{
  S2_KEX_REPORT_RECV      = KEX_REPORT,
  S2_PUB_KEY_RECV_B       = PUBLIC_KEY_REPORT | (SECURITY_2_JOINING_NODE << PUBLIC_KEY_INCLUDING_NODE_BIT_POS),
  S2_PUB_KEY_RECV_A       = PUBLIC_KEY_REPORT | (SECURITY_2_INCLUDING_NODE << PUBLIC_KEY_INCLUDING_NODE_BIT_POS),
  // Slave side
  S2_KEX_GET_RECV         = KEX_GET,
  S2_KEX_SET_RECV         = KEX_SET,
  S2_ECHO_KEX_REPORT_RECV = (SECURITY_2_ECHO_ON << KEX_ECHO_BIT_POS) | KEX_REPORT,
  S2_NET_KEY_REPORT_RECV  = SECURITY_2_NETWORK_KEY_REPORT,
  // Slave side - done

  S2_ECHO_KEX_SET_RECV    = (SECURITY_2_ECHO_ON << KEX_ECHO_BIT_POS) | KEX_SET, //< KEX_SET frames can be with/without echo bit. To differentiate in state machine the echo bit is shifted to first byte in event.
  S2_NET_KEY_GET_RECV     = SECURITY_2_NETWORK_KEY_GET,
  S2_NET_KEY_VERIFY_RECV  = SECURITY_2_NETWORK_KEY_VERIFY,
  S2_TRANSFER_END_RECV    = SECURITY_2_TRANSFER_END,
  S2_INCLUSION_ERROR_RECV = KEX_FAIL,
  S2_INCLUDING_START      = 0x0200,
  S2_JOINING_START,
  S2_INCLUDING_ACCEPT,
  S2_INCLUDING_REJECT,
  S2_INCLUDING_DECRYPT_FAILED,
  // Slave side
  S2_SEND_FINAL_TRANSFER_END,
  S2_INCLUSION_TX_QUEUED,      // Outgoing frame has been queued for transmission
  // Slave side - done
  S2_NET_KEY_VERIFY_FINAL_RECV,
  S2_INCLUSION_RETRY,
  S2_INCLUSION_TIMEOUT,
  S2_INCLUSION_ERROR,
  S2_INCLUSION_ERROR_SENT,
  S2_INCLUSION_SEND_DONE,
  S2_INCLUSION_SEND_FAILED,
  S2_DISCOVERY_COMPLETE,
  S2_NO_KEYS_GRANTED,
  S2_EVT_ANY,
}s2_inclusion_event_t;

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

typedef enum{
  S2_NO_ACTION,
  S2_ABORT_ACTION,
  S2_TIMEOUT_ACTION,
  S2_RESEND_FRAME_ACTION,
  S2_RESEND_DATA_ACTION,
#ifdef ZW_CONTROLLER
  S2_SEND_KEX_GET_ACTION,
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
#endif // ZW_CONTROLLER
  S2_JOINING_START_ACTION,
  S2_TIMEOUT_TB1_SET_ACTION,
  S2_SEND_KEX_REPORT_ACTION,
  S2_SEND_PUB_KEY_B_ACTION,
  S2_PUB_KEY_A_RECV_ACTION,
  S2_DO_ECDH_CALC_B_ACTION,
  S2_SEND_ECHO_KEX_SET_ACTION,
  S2_SEND_NET_KEY_GET_ACTION,
  S2_SEND_NET_KEY_VERIFY_ACTION,
  S2_SEND_FINAL_TRANSFER_END_ACTION,
  S2_JOINING_COMPLETE_ACTION,
  S2_REMOTE_ERROR_ACTION,
  S2_JOINING_NEVER_STARTED_ACTION
}s2_action_t;

typedef struct{
  s2_inclusion_state_t  state;
  s2_inclusion_event_t  event;
  s2_action_t           action;
  s2_inclusion_state_t  new_state;
}s2_transition_t;


static const s2_transition_t s2_transition_table[] = {
#ifdef ZW_CONTROLLER // Those states and transistions are only supported by a controller.
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
#endif // ZW_CONTROLLER
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

static s2_event_handler_t m_evt_handler = s2_default_evt_handler;
static uint32_t           m_event_buffer[S2_EVT_BUFFER_SIZE];
static uint8_t m_schemes;
static uint8_t m_curves;
static uint8_t m_keys;             //< Bitmask of keys this node should request when joining.
static uint8_t shared_secret[32];
static uint8_t shared_key_mem[64]; //< Shared memory array for handling of keys/auth tag during inclusion.

static struct S2 *mp_context;
#define MP_CTX_DEF mp_context = p_context;

static uint8_t m_retry_counter;

void s2_default_evt_handler(zwave_event_t *evt)
{
  UNUSED(evt);
}
/** Member function for internal event handling.
 *  This function is intended to be called when the event type is known, that is when a frame is
 *  decoded or an event is triggered by a function call.
 */
static void process_event(uint16_t evt)
{
  uint8_t              i;
  s2_inclusion_event_t event = (s2_inclusion_event_t)evt;


#if defined(DEBUG_S2_INCL_FSM)
  /* Disabled in C51 build to save code space on state/event _name() functions*/
    DPRINTF( "s2incl process_event event: %s state: %s\n", s2_incl_event_name(event), s2_incl_state_name(mp_context->inclusion_state));
#endif
  for (i = 0; i < ELEM_COUNT(s2_transition_table); i++)
  {
    if (mp_context->inclusion_state == s2_transition_table[i].state || S2_INC_STATE_ANY == s2_transition_table[i].state)
    {
      DPRINTF("? %u %u ", mp_context->inclusion_state, s2_transition_table[i].state);

      if ((event == s2_transition_table[i].event) || (S2_EVT_ANY == s2_transition_table[i].event))
      {
        DPRINTF("! %u %u ", s2_transition_table[i].new_state, s2_transition_table[i].action);

        // Found a match. Execute action and update state if new state is different from S2_INC_STATE_ANY.
        if ((S2_INC_STATE_ANY != s2_transition_table[i].new_state)
            && (mp_context->inclusion_state != s2_transition_table[i].new_state))
        {
          mp_context->inclusion_state = s2_transition_table[i].new_state;
        }

        switch (s2_transition_table[i].action)
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
#ifdef ZW_CONTROLLER // Those states and transistions are only supported by a controller.
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
#endif // ZW_CONTROLLER
          case S2_NO_ACTION:
          default:
          break;
        }
        break;
      }
    }
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
void s2_restore_keys(struct S2 *p_context)
{
  uint8_t i;
  bool    ret_val;

  MP_CTX_DEF

  mp_context->loaded_keys = 0;

  for (i = 0; i < ELEM_COUNT(m_key_slot_pair); i++)
  {
    ret_val = keystore_network_key_read(m_key_slot_pair[i][0], shared_key_mem);
    if (true == ret_val)
    {
      S2_network_key_update(mp_context, m_key_slot_pair[i][1], shared_key_mem, 0);
    }
  }
  // Clearing of memory to ensure key is not exposed accidentially elsewhere.
  memset(shared_key_mem, 0, sizeof(NETWORK_KEY_SIZE));
}

/**@brief Function for notifying application that inclusion failed.
 *
 * @param[in] mp_context Pointer to current context holding information on inclusion.
 */
static void inclusion_failed_evt_push(uint8_t fail_type)
{
  zwave_event_t * p_s2_event = (zwave_event_t *)m_event_buffer;

  s2_inclusion_stop_timeout();
  s2_restore_keys(mp_context);
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
static void inclusion_failed_frame_send(uint8_t error, uint8_t secure)
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


#ifdef ZW_CONTROLLER
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


void s2_inclusion_including_start(struct S2 *p_context, s2_connection_t* conn)
{
  MP_CTX_DEF

  if(mp_context->inclusion_state == S2_INC_IDLE)
  {
    mp_context->inclusion_peer = *conn;
    mp_context->inclusion_peer.tx_options = 0;
    mp_context->inclusion_mode = INCLUSION_MODE_SSA;
    mp_context->kex_fail_code = 0;
    process_event(S2_INCLUDING_START);
  }
}
#endif


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
  // Check if granted key/s are supported - no keys granted are also ok
  if (0 == (~m_keys & support))
  {
    mp_context->key_granted = support;
  }
  else
  {
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

  keystore_public_key_read(&mp_context->u.inclusion_buf[SECURITY_2_PUB_KEY_KEY_POS]);
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

  s2_inclusion_set_timeout(mp_context, TB0_TIMEOUT);
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

  s2_inclusion_stop_timeout();

  received_key = mp_context->buf[SECURITY_2_NET_KEY_REP_GRANT_KEY_POS];

  if (received_key != mp_context->key_exchange)
  {
    goto error_handling;
  }

  keystore_network_key_write(received_key, &mp_context->buf[SECURITY_2_NET_KEY_REP_KEY_POS]);

  mp_context->key_exchange <<= 1;
  // Update context with new key.

  S2_network_key_update(mp_context,  NETWORK_KEY_SECURE, &mp_context->buf[SECURITY_2_NET_KEY_REP_KEY_POS], 0);

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

  s2_restore_keys(mp_context);
  s2_event->event_type        = complete_type;
  s2_event->evt.s2_event.peer = mp_context->inclusion_peer;

  s2_event->evt.s2_event.s2_data.inclusion_complete.exchanged_keys = mp_context->key_granted;

  m_evt_handler(s2_event);
}

static void s2_do_ecdh_calc_b(void)
{
  keystore_private_key_read(&shared_key_mem[LOCAL_PRIVATE_KEY_INDEX]);
  crypto_scalarmult_curve25519(shared_secret, &shared_key_mem[LOCAL_PRIVATE_KEY_INDEX], mp_context->public_key);

  memcpy(&shared_key_mem[PUBLIC_KEY_A_INDEX], mp_context->public_key, 32);
  keystore_public_key_read(&shared_key_mem[PUBLIC_KEY_B_INDEX]);

  tempkey_extract(shared_secret, shared_key_mem, mp_context->public_key);
  S2_network_key_update(mp_context, TEMP_KEY_SECURE, mp_context->public_key, 1);

  m_retry_counter = TBI1_TIMEOUT / TB5_TIMEOUT;
  s2_send_echo_kex_set();
}

static void s2_inclusion_send_frame(void)
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

static void s2_inclusion_send_data(void)
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

#ifdef ZW_CONTROLLER // Those functions used during validation/transistions are only supported by a controller.

/** Section defining validation functions for including node - Begin.
 */
static void s2_do_ecdh_calc_a(void)
{
  keystore_private_key_read(&shared_key_mem[LOCAL_PRIVATE_KEY_INDEX]);
  crypto_scalarmult_curve25519(shared_secret, &shared_key_mem[LOCAL_PRIVATE_KEY_INDEX], mp_context->public_key);

  memcpy(&shared_key_mem[PUBLIC_KEY_B_INDEX], mp_context->public_key, 32);
  keystore_public_key_read(&shared_key_mem[PUBLIC_KEY_A_INDEX]);

  tempkey_extract(shared_secret, shared_key_mem, mp_context->public_key);
  S2_network_key_update(mp_context, TEMP_KEY_SECURE, mp_context->public_key, 1);
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
  S2_network_key_update(mp_context, TEMP_KEY_SECURE, mp_context->public_key, 1);

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
      S2_network_key_update(mp_context, NETWORK_KEY_SECURE, &mp_context->u.inclusion_buf[3], 0);
  }
}

static void s2_inclusion_complete(void)
{
  // Post event upwards to inform that inclusion of noda A has successfully completed.
  zwave_event_t * s2_event;

  s2_inclusion_stop_timeout();
  s2_restore_keys(mp_context);

  s2_event = (zwave_event_t *)m_event_buffer;
  s2_event->event_type                                             = S2_NODE_INCLUSION_COMPLETE_EVENT;
  s2_event->evt.s2_event.peer                                      = mp_context->inclusion_peer;
  s2_event->evt.s2_event.s2_data.inclusion_complete.exchanged_keys = mp_context->key_exchange;

  m_evt_handler(s2_event);
}

static void s2_check_no_keys() {
  if(mp_context->key_granted) {
    inclusion_failed_evt_push(mp_context->kex_fail_code);
  } else {
    s2_inclusion_complete();
  }
}

#endif // ZW_CONTROLLER
/** Section defining including node functions for state transistion actions - End.
  */


