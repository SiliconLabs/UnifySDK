#ifndef _S2_INCLUSION_INTERNAL_H_
#define _S2_INCLUSION_INTERNAL_H_

/**
 * These define the key slots used for the various S2 keys.
 * This information is shared between S2.c and s2_inclusion.c.
 */
#define UNAUTHENTICATED_KEY_SLOT    0       //< Slot for unauthenticated key. This value matches the ordering of security_key_t in ZW_security_api.h - do not change
#define AUTHENTICATED_KEY_SLOT      1       //< Slot for authenticated key. This value matches the ordering of security_key_t in ZW_security_api.h - do not change
#define ACCESS_KEY_SLOT             2       //< Slot for access key. This value matches the class_id returned by S2.c when decrypting with this key.
#define LR_AUTHENTICATED_KEY_SLOT   3       //< Slot for LR authenticated key. This value matches the class_id returned by S2.c when decrypting with this key.
#define LR_ACCESS_KEY_SLOT          4       //< Slot for LR access key. This value matches the ordering of security_key_t in ZW_security_api.h - do not change
#ifdef ZW_CONTROLLER
#define TEMP_KEY_SECURE             5       //< Value identifying index for the temporary key in S2 network key when transmitting secure frames.
#define NETWORK_KEY_SECURE          6       //< Value identifying index for the current network key in S2 network key when transmitting secure frames.
#else
#define TEMP_KEY_SECURE             0       //< Value identifying index for the temporary key in S2 network key when transmitting secure frames.
#define NETWORK_KEY_SECURE          1       //< Value identifying index for the current network key in S2 network key when transmitting secure frames.
#endif

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

/** TODO: Investigate if there is/should be a common header for util functions as this.
 */
#define ELEM_COUNT(ARRAY)          (sizeof(ARRAY)/(sizeof(ARRAY[0])))

#define S2_EVT_BUFFER_SIZE         (128 / 4) //< Max event buffer in s2 inclusion model. Buffer is uint32_t to ensure proper alignment therefor size is divided by 4.

#define MAX_PUBLIC_KEY_LENGTH 32   //< Maximum length of supported public key. Currently only curve 25519 for ECDH is supported, thus key length is fixed to 32 bytes.
#define KEX_ECHO_BIT_POS      8    //< Bit position in event enumeration when identifying an echo frame event.
#define PUBLIC_KEY_INCLUDING_NODE_BIT_POS   8    //< Bit position in event enumeration when identifying including node in public key frame event.

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

typedef enum{
  S2_NO_ACTION,
  S2_ABORT_ACTION,
  S2_TIMEOUT_ACTION,
  S2_RESEND_FRAME_ACTION,
  S2_RESEND_DATA_ACTION,
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
  S2_JOINING_NEVER_STARTED_ACTION,
  S2_ACTION_LAST
}s2_action_t;

typedef struct{
  s2_inclusion_state_t  state;
  s2_inclusion_event_t  event;
  uint8_t               action;     // ID of an action (s2_action_t or s2_action_controller_t).
  s2_inclusion_state_t  new_state;
}s2_transition_t;

extern struct S2 *mp_context;
#define MP_CTX_DEF mp_context = p_context;

extern s2_event_handler_t m_evt_handler;
extern uint32_t m_event_buffer[S2_EVT_BUFFER_SIZE];
extern uint8_t shared_secret[32];
extern uint8_t shared_key_mem[64];
extern uint8_t m_retry_counter;

#endif /*_S2_INCLUSION_INTERNAL_H_ */