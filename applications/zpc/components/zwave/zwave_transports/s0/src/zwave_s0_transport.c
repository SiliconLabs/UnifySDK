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
// Generic includes
#include <stdint.h>
#include <string.h>
#include <ctimer.h>
#include <assert.h>

#include "zwave_tx.h"
#include "sl_log.h"
#include "ZW_classcmd.h"
#include "zwave_controller_internal.h"
#include "zwave_controller_transport_internal.h"
#include "zwave_controller_command_class_indices.h"
#include "zwave_s0_internal.h"
#include "zwave_s0_network.h"
#include "S2.h"
#include "zwave_s0_transport.h"
#include "zwave_controller_transport.h"
#include "zwave_s2_keystore.h"
#include "s2_keystore.h"
#include "zwave_s0_sm.h"
#include "aes.h"
#include "zwave_controller_storage.h"

#define LOG_TAG "zwave_s0_transport"

#define NONCE_TABLE_SIZE       5 * 3
#define NUM_TX_SESSIONS        2
#define MAX_ENCRYPTED_MSG_SIZE 128
#define MAX_NONCES             10
#define MAX_RXSESSIONS         2

#define NONCE_TIMEOUT 10

/*
 * Nonce request timer SHOULD be started by a node sending a Nonce Get Command.
 * If the Nonce request timer is started, the Nonce Report MUST be received
 * before the timer runs out. The duration of this timer will depend on the
 * application it is trying to protect. (10 seconds here)
 */
#define NONCE_REQUEST_TIMER 10000
/* A timer Nonce timer  MUST be started by a node after sending a Nonce Report
 * Command. The S0 Encapsulated Message MUST be received within the specified
 * timeout in order to be accepted.
 * The Nonce timer MUST implement a timeout in the range 3..20 seconds.
 *
 * Note that the Nonce timer and the Nonce request timer MUST be started when
 * the command has been sent and not when the transmission has been
 * acknowledged, since an attacker could delay the acknowledgement frame.
*/
#define NONCE_TIMER 20000

#define NONCE_BLOCK_LIST_SIZE 10
#define RECEIVERS_NONCE_SIZE \
  8 /* The size of the nonce field in a Nonce
                                     Report */

static zwave_tx_session_id_t zwave_tx_parent_session_id = NULL;
static bool zwave_tx_valid_parent_session_id            = false;
#define UNUSED(x) x = x;
#define STR_CASE(x) \
  case x:           \
    return #x;

typedef enum {
  NONCE_GET,
  NONCE_GET_SENT,
  ENC_MSG,
  ENC_MSG_SENT,
  ENC_MSG2,
  ENC_MSG2_SENT,
  TX_DONE,
  TX_FAIL
} tx_state_t;

const char *s0_cmd_name(uint8_t cmd)
{
  static char message[25];
  switch (cmd) {
    STR_CASE(SECURITY_NONCE_GET)
    STR_CASE(SECURITY_NONCE_REPORT)
    STR_CASE(SECURITY_MESSAGE_ENCAPSULATION_NONCE_GET)
    STR_CASE(SECURITY_MESSAGE_ENCAPSULATION)
    default:
      snprintf(message, sizeof(message), "%d", cmd);
      return message;
  }
}
const char *s0_state_name(tx_state_t state)
{
  static char message[25];
  switch (state) {
    STR_CASE(NONCE_GET)
    STR_CASE(NONCE_GET_SENT)
    STR_CASE(ENC_MSG)
    STR_CASE(ENC_MSG_SENT)
    STR_CASE(ENC_MSG2)
    STR_CASE(ENC_MSG2_SENT)
    STR_CASE(TX_DONE)
    STR_CASE(TX_FAIL)
    default:
      snprintf(message, sizeof(message), "%d", state);
      return message;
  }
}

typedef struct sec_tx_session {
  zwave_controller_connection_info_t conn_info;
  uint8_t buf[256];     // data to be encrypted, max len is uint8_t so 256 size
  const uint8_t *data;  // Pointer the data to be encrypted
  uint8_t data_len;     // Length of the data to be encrypted
  tx_state_t state;
  uint8_t tx_code;
  clock_time_t transition_time;
  struct ctimer timer;  // Session timer
  uint8_t crypted_msg[46];
  uint8_t seq;       // Sequence number used in multisegment messages
  const void *user;  // User supplied pointer which is returned in the
                     // callback
  on_zwave_tx_send_data_complete_t
    callback;  // Callback function, called when the
               // session has been terminated
  const zwapi_tx_report_t
    *tx_ext_status;  // Extended TX status, most notably IMA
                     // information
  zwave_tx_session_id_t session_id;
} sec_tx_session_t;

typedef struct _authdata_ {
  uint8_t sh;             /* Security Header for authentication */
  uint8_t senderNodeID;   /* Sender ID for authentication */
  uint8_t receiverNodeID; /* Receiver ID for authentication */
  uint8_t payloadLength;  /* Length of authenticated payload */
} auth_data_t;

static sec_tx_session_t tx_sessions[NUM_TX_SESSIONS];

/* Nonce block type*/
typedef struct {
  uint8_t nonce[RECEIVERS_NONCE_SIZE];
  uint8_t src;
  uint8_t dst;
  unsigned int in_use;
} nonce_block_t;
nonce_block_t nonce_block[NONCE_BLOCK_LIST_SIZE];

static unsigned int block_next_elem; /* Cyclic counter for the next
                                            element in nonce block */

/************************ AES Helper functions ********************************/
static uint8_t aes_key[16];
static uint8_t aes_iv[16];

static void aes_encrypt(uint8_t *in, uint8_t *out)
{
  AES128_ECB_encrypt(in, aes_key, out);
}

static void aes_set_key_tpt(const uint8_t *key, const uint8_t *iv)
{
  memcpy(aes_key, key, 16);
  memcpy(aes_iv, iv, 16);
}

static void aes_ofb(uint8_t *data, uint8_t len)
{
  for (uint8_t i = 0; i < len; i++) {
    if ((i & 0xF) == 0x0) {
      aes_encrypt(aes_iv, aes_iv);
    }
    data[i] ^= aes_iv[i & 0xF];
  }
}

/*
 * Calculate the authtag for the message,
 */
static void aes_cbc_mac(const uint8_t *data, uint8_t len, uint8_t *mac)
{
  aes_encrypt(aes_iv, mac);
  for (uint8_t i = 0; i < len; i++) {
    mac[i & 0xF] ^= data[i];
    if ((i & 0xF) == 0xF) {
      aes_encrypt(mac, mac);
    }
  }

  /* if len is not divisible by 16 do the final pass, this is the padding
   * described in the spec */
  if (len & 0xF) {
    aes_encrypt(mac, mac);
  }
}

/***************** Nonce Blocked list **************************************/
/**
 * Test if an S0 nonce with particular source and destination is blocked.
 *
 * \return 1 if nonce is in block, 0 otherwise
 */
static unsigned int s0_is_nonce_blocked(const uint8_t src,
                                        const uint8_t dst,
                                        const uint8_t *nonce)
{
  for (unsigned int i = 0; i < NONCE_BLOCK_LIST_SIZE; i++) {
    if (nonce_block[i].in_use && (0 == memcmp(nonce_block[i].nonce, nonce, 8))
        && (nonce_block[i].src == src) && (nonce_block[i].dst == dst)) {
      return 1;
    }
  }
  return 0;
}

/**
 *  We block the last NONCE_BLOCK_LIST_SIZE external nonces we have used.
 *  This function adds nonce to the block.
 */
static void
  s0_block_nonce(const uint8_t src, const uint8_t dst, const uint8_t *nonce)
{
  nonce_block[block_next_elem].in_use = 1;
  memcpy(nonce_block[block_next_elem].nonce, nonce, RECEIVERS_NONCE_SIZE);
  nonce_block[block_next_elem].src = src;
  nonce_block[block_next_elem].dst = dst;
  block_next_elem = (block_next_elem + 1) % NONCE_BLOCK_LIST_SIZE;
}

/********************** Nonce Management **************************************/

typedef struct nonce {
  zwave_node_id_t src;
  zwave_node_id_t dst;
  u8_t timeout;
  u8_t reply_nonce;  //indicate if this nonce from a enc message sent by me
  u8_t nonce[RECEIVERS_NONCE_SIZE];
} nonce_t;

static nonce_t nonce_table[NONCE_TABLE_SIZE];  //Nonces received or sent
struct ctimer nonce_timer;

/**
 * Register a new nonce from sent from src to dst
 */
static u8_t
  register_nonce(u8_t src, u8_t dst, u8_t reply_nonce, const u8_t nonce[8])
{
  if (reply_nonce) {
    /*Only one reply nonce is allowed*/
    for (uint8_t i = 0; i < NONCE_TABLE_SIZE; i++) {
      if (nonce_table[i].reply_nonce && nonce_table[i].timeout > 0
          && nonce_table[i].src == src && nonce_table[i].dst == dst) {
        sl_log_debug(LOG_TAG,
                     "Reply nonce overwritten %d->%d slot:%d\n",
                     src,
                     dst,
                     i);
        memcpy(nonce_table[i].nonce, nonce, 8);
        nonce_table[i].timeout = NONCE_TIMEOUT;
        return 1;
      }
    }
  }

  for (uint8_t i = 0; i < NONCE_TABLE_SIZE; i++) {
    if (nonce_table[i].timeout == 0) {
      nonce_table[i].src         = src;
      nonce_table[i].dst         = dst;
      nonce_table[i].reply_nonce = reply_nonce;
      memcpy(nonce_table[i].nonce, nonce, 8);
      nonce_table[i].timeout = NONCE_TIMEOUT;
      sl_log_debug(LOG_TAG, "Nonce registered %d->%d slot:%d\n", src, dst, i);
      return 1;
    }
  }

  sl_log_error(LOG_TAG, "Nonce table is full\n");
  return 0;
}

/**
 * Receive nonce sent from src to dst, if th ri. If a nonce
 * is found, then remove all entries from that src->dst combination
 * from the table.
 *
 * If any_nonce is set then ri is ignored
 */
static u8_t get_s0_nonce(
  u8_t src, u8_t dst, u8_t ri, u8_t nonce[RECEIVERS_NONCE_SIZE], u8_t any_nonce)
{
  for (uint8_t i = 0; i < NONCE_TABLE_SIZE; i++) {
    if ((nonce_table[i].timeout > 0) && (nonce_table[i].src == src)
        && (nonce_table[i].dst == dst)
        && (any_nonce || (nonce_table[i].nonce[0] == ri))) {
      memcpy(nonce, nonce_table[i].nonce, RECEIVERS_NONCE_SIZE);
      return 1;
    }
  }
  return 0;
}

/**
 * Remove all nonces from nonce table sent from src to dst
 * @param src
 * @param dst
 */
static void nonce_clear(u8_t src, u8_t dst)
{
  /*Remove entries from table from that source dest combination */
  for (uint8_t i = 0; i < NONCE_TABLE_SIZE; i++) {
    if ((nonce_table[i].timeout) && (nonce_table[i].src == src)
        && (nonce_table[i].dst == dst)) {
      nonce_table[i].timeout = 0;
      sl_log_debug(LOG_TAG,
                   "%d -> %d nonce cleared slot: %d\n",
                   nonce_table[i].src,
                   nonce_table[i].dst,
                   i);
    }
  }
}

static void nonce_timer_timeout(void *data)
{
  (void)data;
  for (uint8_t i = 0; i < NONCE_TABLE_SIZE; i++) {
    if (nonce_table[i].timeout > 0) {
      nonce_table[i].timeout--;
      if (nonce_table[i].timeout == 0) {
        sl_log_debug(LOG_TAG,
                     "%d -> %d nonce timed out slot: %d\n",
                     nonce_table[i].src,
                     nonce_table[i].dst,
                     i);
      }
    }
  }
  ctimer_set(&nonce_timer, 1000, nonce_timer_timeout, 0);
}

/**************************Security TX Code ***********************************/
static void tx_session_state_set(sec_tx_session_t *s, tx_state_t state);
static uint8_t enckey[16];
static uint8_t authkey[16];
static uint8_t enckeyz[16];
static uint8_t authkeyz[16];

/**
 * Set the network key
 */
void s0_set_key(const uint8_t *netkey)
{
  uint8_t p[16];
  uint8_t temp[16] = {0};

  if (memcmp(netkey, temp, 16) == 0) {
    sl_log_debug(LOG_TAG, "Setting S0 key with 0x00 16 times.\n");
  } else {
    sl_log_debug(LOG_TAG, "Setting S0 key to the network key.\n");
  }

  memcpy(aes_key, netkey, 16);
  memset(p, 0x55, 16);
  aes_encrypt(p, authkey);
  memset(p, 0xAA, 16);
  aes_encrypt(p, enckey);

  memcpy(aes_key, temp, 16);
  memset(p, 0x55, 16);
  aes_encrypt(p, authkeyz);
  memset(p, 0xAA, 16);
  aes_encrypt(p, enckeyz);
}

static void s0_timeout(void *user)
{
  sec_tx_session_t *s = (sec_tx_session_t *)user;
  sl_log_warning(LOG_TAG,
                 "Security0 transmit timeout state: %s\n",
                 s0_state_name(s->state));
  s->tx_code = TRANSMIT_COMPLETE_FAIL;
  tx_session_state_set(s, TX_FAIL);
}

/**
 * Lookup a tx session by nodeid
 */
static sec_tx_session_t *get_tx_session_by_node(uint8_t snode, uint8_t dnode)
{
  for (uint8_t i = 0; i < NUM_TX_SESSIONS; i++) {
    if (tx_sessions[i].conn_info.remote.node_id == dnode
        && tx_sessions[i].conn_info.local.node_id == snode) {
      return &tx_sessions[i];
    }
  }
  return 0;
}

/**
 * Get the maximum frame size supported by a node.
 */
static uint8_t get_node_max_frame_size(zwave_node_id_t node_id)
{
  (void)node_id;
  return ZWAVE_MAX_MAC_PAYLOAD_SIZE;
}

/**
 * Encrypt a message and write the encrypted data into s->crypted_msg
 */
static uint8_t encrypt_msg(sec_tx_session_t *s, uint8_t pass2)
{
  uint8_t iv[16] = {0};
  uint8_t tmp[8]; /* Initialization vector for enc, dec,& auth */
  uint8_t mac[16];
  uint8_t len;  // Length of the encrypted part
  uint8_t more_to_send;
  uint8_t *enc_data;
  auth_data_t *auth;
  uint8_t maxlen = get_node_max_frame_size(s->conn_info.remote.node_id);

  len          = s->data_len;
  more_to_send = 0;

  //sl_log_debug(LOG_TAG, "s->data[0]: 0x%02x, s->data[1]: 0x%02x\n",s->data[0],s->data[1]);
  /*Check if we should break this message in two */
  if (len + 20 > maxlen) {
    len          = maxlen - 20;
    more_to_send = 1;
  }

  assert(len + 20 <= sizeof(s->crypted_msg));
  /* Make the IV */

  do {
    AES_CTR_DRBG_Generate(&s2_ctr_drbg, iv);
  } while (get_s0_nonce(s->conn_info.remote.node_id,
                        s->conn_info.local.node_id,
                        iv[0],
                        tmp,
                        false));

  /*Choose a nonce from sender */
  /* Find the nonce (remote->local) which the remote node must have sent and use it in encryption for sending from (local->remote) */
  if (!get_s0_nonce(s->conn_info.remote.node_id,
                    s->conn_info.local.node_id,
                    0,
                    iv + 8,
                    true)) {
    sl_log_warning(LOG_TAG,
                   "error: Nonce for NodeIDs %u -> %u is not found\n",
                   s->conn_info.remote.node_id,
                   s->conn_info.local.node_id);
    return 0;
  } else {
    /* Clear the nonce (remote->local) in nonce table as its one time use and as its been copied from nonce table to local structure */
    nonce_clear(s->conn_info.remote.node_id, s->conn_info.local.node_id);
  }

  /*Register my nonce */
  register_nonce(s->conn_info.local.node_id,
                 s->conn_info.remote.node_id,
                 true,
                 iv);

  /* Setup pointers */
  enc_data = s->crypted_msg + 10;
  auth     = (auth_data_t *)(s->crypted_msg + 6);

  /* Copy data into a second buffer Insert security flags */

  if (pass2) {
    *enc_data
      = SECURITY_MESSAGE_ENCAPSULATION_PROPERTIES1_SEQUENCED_BIT_MASK
        | SECURITY_MESSAGE_ENCAPSULATION_PROPERTIES1_SECOND_FRAME_BIT_MASK
        | (s->seq & 0xF);
  } else if (more_to_send) {
    *enc_data = SECURITY_MESSAGE_ENCAPSULATION_PROPERTIES1_SEQUENCED_BIT_MASK
                | (s->seq & 0xF);
  } else {
    *enc_data = 0;
  }

  memcpy(enc_data + 1, s->data, len);

  if ((s->data[0] == COMMAND_CLASS_SECURITY)
      && (s->data[1] == NETWORK_KEY_SET)) {
    sl_log_debug(LOG_TAG, "COMMAND_CLASS_SECURITY, NETWORK_KEY_SET\n");
    /*Encrypt */
    aes_set_key_tpt(enckeyz, iv);
  } else {
    /*Encrypt */
    aes_set_key_tpt(enckey, iv);
  }

  aes_ofb(enc_data, len + 1);

  /*Fill in the auth structure*/
  auth->sh             = more_to_send ? SECURITY_MESSAGE_ENCAPSULATION_NONCE_GET
                                      : SECURITY_MESSAGE_ENCAPSULATION;
  auth->senderNodeID   = s->conn_info.local.node_id;
  auth->receiverNodeID = s->conn_info.remote.node_id;
  auth->payloadLength  = len + 1;

  if ((s->data[0] == COMMAND_CLASS_SECURITY)
      && (s->data[1] == NETWORK_KEY_SET)) {
    sl_log_debug(LOG_TAG, "COMMAND_CLASS_SECURITY, NETWORK_KEY_SET\n");
    /* Authtag */
    aes_set_key_tpt(authkeyz, iv);
  } else {
    /* Authtag */
    aes_set_key_tpt(authkey, iv);
  }
  aes_cbc_mac((uint8_t *)auth, 4 + len + 1, mac);
  s->crypted_msg[0] = COMMAND_CLASS_SECURITY;
  s->crypted_msg[1] = auth->sh;
  memcpy(s->crypted_msg + 2, iv, 8);

  s->crypted_msg[2 + 8 + len + 1] = iv[8];
  memcpy(s->crypted_msg + 2 + 8 + len + 2, mac, 8);

  s->data += len;
  s->data_len -= len;
  return len + 20;
}

static void
  callback(uint8_t status, const zwapi_tx_report_t *tx_info, void *user)
{
  sec_tx_session_t *s = (sec_tx_session_t *)user;

  sl_log_debug(LOG_TAG, "S0 Send data callback: status %d\n", status);
  s->tx_code = status;
  // TX ext status return from serial API only if transmit succeed
  if (tx_info) {
    s->tx_ext_status = tx_info;
  }
  if (s->state == ENC_MSG) {
    tx_session_state_set(s,
                         status == TRANSMIT_COMPLETE_OK ? ENC_MSG_SENT
                                                        : TX_FAIL);
  } else if (s->state == ENC_MSG2) {
    tx_session_state_set(s,
                         status == TRANSMIT_COMPLETE_OK ? ENC_MSG2_SENT
                                                        : TX_FAIL);
  } else if (s->state == NONCE_GET) {
    tx_session_state_set(s,
                         status == TRANSMIT_COMPLETE_OK ? NONCE_GET_SENT
                                                        : TX_FAIL);
  }
}

static void tx_session_state_set(sec_tx_session_t *s, tx_state_t state)
{
  uint8_t len;
  zwave_tx_options_t options  = {};
  options.number_of_responses = 1;
  options.discard_timeout_ms  = NONCE_REQUEST_TIMER;
  options.qos_priority        = ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY;

  static const uint8_t nonce_get[]
    = {COMMAND_CLASS_SECURITY, SECURITY_NONCE_GET};

  zwave_controller_connection_info_t c2 = {};
  c2.local.node_id                      = s->conn_info.local.node_id;
  c2.remote.node_id                     = s->conn_info.remote.node_id;
  c2.encapsulation                      = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;

  // If the call initiated from outside S0, the parent frame options will take precedence.
  options.valid_parent_session_id = zwave_tx_valid_parent_session_id;
  options.parent_session_id       = zwave_tx_parent_session_id;

  s->state           = state;
  s->transition_time = clock_time();
  sl_log_debug(LOG_TAG, "S0 state %s\n", s0_state_name(state));

  switch (s->state) {
    case NONCE_GET:
      //sl_log_debug(LOG_TAG, "s: %p\n", s);
      options.number_of_responses = 1;
      if (zwave_tx_send_data(&c2,
                             sizeof(nonce_get),
                             nonce_get,
                             &options,
                             callback,
                             s,
                             0)
          != SL_STATUS_OK) {
        s->tx_code = TRANSMIT_COMPLETE_FAIL;
        tx_session_state_set(s, TX_FAIL);
      } else {
        /* The Nonce request timer MUST be started when the command has been
         * sent and not when the transmission has been acknowledged, since an
         * attacker could delay the acknowledgement frame
         */
        ctimer_set(&s->timer,
                   NONCE_REQUEST_TIMER + clock_time() - s->transition_time,
                   s0_timeout,
                   s);
      }
      break;
    case NONCE_GET_SENT:
      break;
    case ENC_MSG:
      ctimer_stop(&s->timer);
      len = encrypt_msg(s, 0);

      //If there is more data to send.
      if (s->data_len > 0) {
        options.number_of_responses = 1;
      }
      if ((len == 0)
          || (zwave_tx_send_data(&c2,
                                 len,
                                 s->crypted_msg,
                                 &options,
                                 callback,
                                 s,
                                 0)
              != SL_STATUS_OK)) {
        s->tx_code = TRANSMIT_COMPLETE_FAIL;
        tx_session_state_set(s, TX_FAIL);
      } else {
        /* The Nonce timer MUST be started when the command has been sent
         * and not when the transmission has been acknowledged, since an
         * attacker could delay the acknowledgement frame
         */
        ctimer_set(&s->timer,
                   NONCE_REQUEST_TIMER + clock_time() - s->transition_time,
                   s0_timeout,
                   s);
      }
      break;
    case ENC_MSG_SENT:
      /*If there is no more data to send*/
      if (s->data_len == 0) {
        tx_session_state_set(s, TX_DONE);
      } else {
      }
      break;
    case ENC_MSG2:
      ctimer_stop(&s->timer);
      //sl_log_debug(LOG_TAG, "ENC_MSG2: s->data[0]: 0x%02x, s->data[1]: 0x%02x\n",s->data[0],s->data[1]);
      len = encrypt_msg(s, 1);
      if ((len == 0)
          || (zwave_tx_send_data(&s->conn_info,
                                 len,
                                 s->crypted_msg,
                                 &options,
                                 callback,
                                 s,
                                 0))
               != SL_STATUS_OK) {
        s->tx_code = TRANSMIT_COMPLETE_FAIL;
        tx_session_state_set(s, TX_FAIL);
      } else {
        /* The Nonce timer MUST be started when the command has been sent
       * and not when the transmission has been acknowledged, since an
       * attacker could delay the acknowledgement frame
       */
        ctimer_set(&s->timer,
                   NONCE_REQUEST_TIMER + clock_time() - s->transition_time,
                   s0_timeout,
                   s);
      }
      break;
    case ENC_MSG2_SENT:
      tx_session_state_set(s, TX_DONE);
      break;
    case TX_DONE:
      s->conn_info.remote.node_id = 0;
      ctimer_stop(&s->timer);
      if (s->callback)
        s->callback(TRANSMIT_COMPLETE_OK,
                    s->tx_ext_status,
                    zwave_tx_parent_session_id);
      s->tx_ext_status = 0;
      break;
    case TX_FAIL:
      s->conn_info.remote.node_id = 0;
      ctimer_stop(&s->timer);
      if (s->callback)
        s->callback(s->tx_code, s->tx_ext_status, zwave_tx_parent_session_id);

      s->tx_ext_status = 0;
      break;
    default:
      sl_log_error(LOG_TAG, "Unknown state %d\n", state);
      break;
  }
}

/**
 * Get the next sequence number no node may receive the same sequence number
 * in two concurrent transmissions
 */
static uint8_t get_seq()
{
  static uint8_t s = 0;
  s++;
  return s;
}

sl_status_t
  zwave_s0_send_data(const zwave_controller_connection_info_t *conn_info,
                     uint16_t data_length_2_byte,
                     const uint8_t *cmd_data,
                     const zwave_tx_options_t *tx_options,
                     const on_zwave_tx_send_data_complete_t on_send_complete,
                     void *user,
                     zwave_tx_session_id_t parent_session_id)
{
  (void)tx_options;
  sec_tx_session_t *s;
  if (conn_info->encapsulation != ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // S0 multicast will be a silent frame. Just pretend we did it by
  // invoking the callback immediately.
  // Tx Queue will then ask us for individual follow ups.
  if (conn_info->remote.is_multicast == true) {
    if (on_send_complete) {
      on_send_complete(TRANSMIT_COMPLETE_OK, NULL, user);
    }
    return SL_STATUS_OK;
  }

  /*FIXME frame length was uint8_t in GW S0 but the
   * zwave_s0_on_frame_received has to have 16bit length field because of
   *  zwave_controller_transport_t
   */
  uint8_t data_length = data_length_2_byte & 0xff;

  // Call for sending data comes from outside S2, so we use the parent frame functionality
  zwave_tx_parent_session_id       = parent_session_id;
  zwave_tx_valid_parent_session_id = true;

  s = get_tx_session_by_node(conn_info->local.node_id,
                             conn_info->remote.node_id);
  if (s) {
    sl_log_error(LOG_TAG,
                 "Already have one tx session from NodeID %d to %d\n",
                 conn_info->local.node_id,
                 conn_info->remote.node_id);
    return SL_STATUS_BUSY;
  }

  for (uint8_t i = 0; i < NUM_TX_SESSIONS; i++) {
    if (tx_sessions[i].conn_info.remote.node_id == 0) {
      s                         = &tx_sessions[i];
      tx_sessions[i].session_id = zwave_tx_parent_session_id;
      break;
    }
  }

  if (!s) {
    sl_log_error(LOG_TAG, "No more s0 TX sessions available\n");
    return SL_STATUS_BUSY;
  }

  s->conn_info = *conn_info;
  memcpy(s->buf, cmd_data, data_length);
  s->data     = &s->buf[0];
  s->data_len = data_length;
  s->callback = on_send_complete;
  s->user     = user;
  s->seq = get_seq();  // Fix this function to make seq according to sessions

  tx_session_state_set(s, NONCE_GET);
  return SL_STATUS_OK;
}

/**
 * Register a nonce by source and destination
 */
static void s0_register_nonce(uint8_t src, uint8_t dst, const uint8_t *nonce)
{
  sec_tx_session_t *s;
  if (s0_is_nonce_blocked(src, dst, nonce)) {
    sl_log_warning(LOG_TAG,
                   "Ignoring duplicate nonce for NodeIDs %d -> %d\n",
                   src,
                   dst);
    return;
  }
  s = get_tx_session_by_node(dst, src);
  if (s) {
    register_nonce(src, dst, false, nonce);
    s0_block_nonce(src, dst, nonce);
    if (s->state == NONCE_GET_SENT || s->state == NONCE_GET) {
      tx_session_state_set(s, ENC_MSG);
    } else if ((s->state == ENC_MSG_SENT)
               || (s->state == ENC_MSG && s->data_len > 0)) {
      tx_session_state_set(s, ENC_MSG2);
    }
  } else {
    sl_log_warning(LOG_TAG,
                   "Nonce Report not matching any known session. Received for "
                   "NodeIDs %d -> %d\n",
                   src,
                   dst);
  }
}

/************************* Security RX code ***********************************/

typedef enum { RX_INIT, RX_ENC1, RX_ENC2, RX_SESSION_DONE } rx_session_state_t;

typedef struct {
  uint8_t snode;
  uint8_t dnode;
  rx_session_state_t state;
  uint8_t seq_nr;
  uint8_t msg[MAX_ENCRYPTED_MSG_SIZE];
  uint8_t msg_len;
  clock_time_t timeout;
} rx_session_t;

uint8_t is_free(const rx_session_t *e)
{
  return (e->state == RX_SESSION_DONE) || (e->timeout < clock_time());
}

rx_session_t rxsessions[MAX_RXSESSIONS];

/**
 * Get a new free RX session.
 */
rx_session_t *new_rx_session(uint8_t snode, uint8_t dnode)
{
  for (uint8_t i = 0; i < MAX_RXSESSIONS; i++) {
    if (is_free(&rxsessions[i])) {
      rxsessions[i].snode = snode;
      rxsessions[i].dnode = dnode;
      rxsessions[i].timeout
        = clock_time() + CLOCK_SECOND * 10;  //Timeout in 10s
      return &rxsessions[i];
    }
  }
  return 0;
}

void free_rx_session(rx_session_t *s)
{
  s->state = RX_SESSION_DONE;
}

void s0_abort_all_tx_sessions()
{
  for (uint8_t i = 0; i < NUM_TX_SESSIONS; i++) {
    if (tx_sessions[i].conn_info.remote.node_id) {
      tx_sessions[i].tx_code = TRANSMIT_COMPLETE_FAIL;
      tx_session_state_set(&tx_sessions[i], TX_FAIL);
    }
  }
}

void free_all_rx_session()
{
  for (uint8_t i = 0; i < MAX_RXSESSIONS; i++) {
    free_rx_session(&rxsessions[i]);
  }
}

/**
 * Get a specific nonce from the nonce table. The session must not be expired
 */
rx_session_t *get_rx_session_by_nodes(uint8_t snode, uint8_t dnode)
{
  rx_session_t *e;
  for (uint8_t i = 0; i < MAX_RXSESSIONS; i++) {
    e = &rxsessions[i];
    if (!is_free(e) && e->dnode == dnode && e->snode == snode) {
      return e;
    }
  }
  return 0;
}
/**
 * Send a nonce from given source to given destination. The nonce is registered
 * internally
 */
void s0_send_nonce(const zwave_controller_connection_info_t *conn_info)
{
  uint8_t nonce[8]       = {0};
  uint8_t temp_nonce[16] = {0};
  uint8_t tmp[8]         = {0};
  uint8_t nonce_res[sizeof(ZW_SECURITY_NONCE_REPORT_FRAME)];
  zwave_controller_connection_info_t c2 = {};

  do {
    AES_CTR_DRBG_Generate(&s2_ctr_drbg, temp_nonce);
    memcpy(nonce, temp_nonce, sizeof(nonce));
  } while (get_s0_nonce(conn_info->local.node_id,
                        conn_info->remote.node_id,
                        nonce[0],
                        tmp,
                        false));

  nonce_res[0] = COMMAND_CLASS_SECURITY;
  nonce_res[1] = SECURITY_NONCE_REPORT;
  memcpy(&nonce_res[2], nonce, 8);

  zwave_tx_options_t options  = {};
  options.number_of_responses = 1;
  options.discard_timeout_ms  = NONCE_TIMER;
  options.qos_priority        = ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY;

  memcpy(&c2, conn_info, sizeof(zwave_controller_connection_info_t));
  c2.encapsulation = 0;
  if (zwave_tx_send_data(&c2,
                         sizeof(nonce_res),
                         (uint8_t *)nonce_res,
                         &options,
                         0,
                         0,
                         0)
      == SL_STATUS_OK) {
    register_nonce(conn_info->local.node_id,
                   conn_info->remote.node_id,
                   false,
                   nonce);
  } else {
    sl_log_error(LOG_TAG,
                 "Failure in sending nonce report from %d -> %d\n",
                 conn_info->local.node_id,
                 conn_info->remote.node_id);
  }
}

//Returns len of decrypted buffer, 0 on error
uint8_t
  s0_decrypt_message(const zwave_controller_connection_info_t *connection_info,
                     const zwave_rx_receive_options_t *rx_options,
                     const uint8_t *frame_data1,
                     uint16_t frame_length_2_byte,
                     uint8_t *output_buffer)
{
  (void)rx_options;
  uint8_t iv[16]; /* Initialization vector for enc, dec,& auth */
  uint8_t mac[16];
  uint8_t *dec_message = output_buffer;
  rx_session_t *s;
  uint8_t *enc_payload;
  uint8_t ri;
  auth_data_t *auth;
  uint8_t flags;
  uint8_t frame_data[0xffff];
  uint8_t frame_length;

  frame_length = frame_length_2_byte & 0xff;  //FIXME as GW S0 code can handle
                                              // only uint8_t len
  memcpy(frame_data, frame_data1, frame_length);
  if (frame_length < 19) {
    sl_log_error(LOG_TAG, "Encrypted message is too short\n");
    return 0;
  }

  ri = frame_data[frame_length - 9];

  /*Build the IV*/
  memcpy(iv, frame_data + 2, 8);

  /*Find the nonce (local->remote) in the nonce table as this is decrypt*/
  if (!get_s0_nonce(connection_info->local.node_id,
                    connection_info->remote.node_id,
                    ri,
                    &iv[8],
                    false)) {
    sl_log_debug(
      LOG_TAG,
      "error: Nonce for NodeIDs %d -> %d not found. Incoming frame will "
      "not be decrypted",
      (int)connection_info->local.node_id,
      (int)connection_info->remote.node_id);
    return 0;
  } else {
    /*Clear the (local->remote) Nonce from nonce table  as its one time use
       * and its copied in local structure */
    nonce_clear(connection_info->local.node_id,
                connection_info->remote.node_id);
  }

  /*TODO don't create sessions for single fragment frames*/
  s = get_rx_session_by_nodes(connection_info->remote.node_id,
                              connection_info->local.node_id);
  if (!s) {
    s = new_rx_session(connection_info->remote.node_id,
                       connection_info->local.node_id);
    if (s) {
      s->state = RX_INIT;
    } else {
      sl_log_warning(LOG_TAG, "No more RX sessions available\n");
      return false;
    }
  }

  enc_payload = frame_data + 2 + 8;

  /*Temporarily use dec_message for auth verification*/
  auth = (auth_data_t *)dec_message;
  /*Fill in the auth structure*/
  auth->sh             = frame_data[1];
  auth->senderNodeID   = connection_info->remote.node_id;
  auth->receiverNodeID = connection_info->local.node_id;
  auth->payloadLength  = frame_length - 19;
  memcpy((uint8_t *)auth + 4, enc_payload, auth->payloadLength);

  /* Authtag */
  aes_set_key_tpt(authkey, iv);
  aes_cbc_mac((uint8_t *)auth, 4 + auth->payloadLength, mac);

  if (memcmp(mac, frame_data + frame_length - 8, 8) != 0) {
    sl_log_error(LOG_TAG, "Unable to verify auth tag\n");
    return 0;
  }
  /*Decrypt */
  aes_set_key_tpt(enckey, iv);
  aes_ofb(enc_payload, auth->payloadLength);

  flags = *enc_payload;

  if (flags & SECURITY_MESSAGE_ENCAPSULATION_PROPERTIES1_SEQUENCED_BIT_MASK) {
    if ((flags
         & SECURITY_MESSAGE_ENCAPSULATION_PROPERTIES1_SECOND_FRAME_BIT_MASK)
        == 0) {
      //First frame
      s->seq_nr = flags & 0xF;
      s->msg_len = frame_length - 20;
      s->state   = RX_ENC1;
      memcpy(s->msg, enc_payload + 1, s->msg_len);
      return 0;
    } else {
      //Second frame
      if ((s->state != RX_ENC1) || (flags & 0xF) != s->seq_nr) {
        sl_log_error(
          LOG_TAG,
          "State is %d, received sequence number is %u while we expected %u\n",
          (int)s->state,
          flags & 0xF,
          s->seq_nr);
        goto state_error;
      } else {
        s->state = RX_ENC2;
      }
      memcpy(dec_message, s->msg, s->msg_len);
      memcpy(dec_message + s->msg_len, enc_payload + 1, frame_length - 20);

      free_rx_session(s);
      return (s->msg_len + frame_length - 20);
    }
  } else {
    /* Single frame message */
    memcpy(dec_message, enc_payload + 1, frame_length - 20);
    free_rx_session(s);
    return (frame_length - 20);
  }
  return false;
state_error:
  sl_log_error(LOG_TAG, "Security RX session is not in the right state\n");
  return false;
}

sl_status_t
  s0_application_command_handler(const zwave_controller_connection_info_t *c,
                                 const zwave_rx_receive_options_t *rx_options,
                                 const uint8_t *frame_data,
                                 uint16_t frame_length)
{
  static uint8_t encrypted_frame_data[ZWAVE_MAX_FRAME_SIZE];
  if (frame_length > ZWAVE_MAX_FRAME_SIZE) {
    sl_log_debug(LOG_TAG,
                 "error: Frame length is more than "
                 "ZWAVE_MAX_FRAME_SIZE: %d. Incoming frame will be dropped.\n",
                 ZWAVE_MAX_FRAME_SIZE);
    return SL_STATUS_WOULD_OVERFLOW;
  }

  if (frame_data[COMMAND_CLASS_INDEX] != COMMAND_CLASS_SECURITY) {
    return SL_STATUS_NOT_FOUND;
  }

  //S0 code decrypts the data in place. So const frame_data is copied here
  memcpy(encrypted_frame_data, frame_data, frame_length);

  uint8_t decrypted_frame[64];
  uint8_t len;

  sl_log_debug(LOG_TAG,
               "s0_application_command_handler NodeID %d -> %d. Command: %s "
               "Encapsulation: %d",
               c->remote.node_id,
               c->local.node_id,
               s0_cmd_name(frame_data[COMMAND_INDEX]),
               c->encapsulation);
  switch (frame_data[COMMAND_INDEX]) {
    case SECURITY_SCHEME_REPORT:
      if (frame_length >= sizeof(ZW_SECURITY_SCHEME_REPORT_FRAME)) {
        s0_scheme_report_received(
          frame_data[offsetof(ZW_SECURITY_SCHEME_REPORT_FRAME,
                              supportedSecuritySchemes)],
          c->remote.node_id);
      }
      break;
    case NETWORK_KEY_VERIFY:
      s0_network_key_verify_received(c->remote.node_id);
      break;

    case SECURITY_NONCE_GET:
      if (c->encapsulation == ZWAVE_CONTROLLER_ENCAPSULATION_NONE) {
        s0_send_nonce(c);
      }
      break;
    case SECURITY_NONCE_REPORT:
      if ((c->encapsulation == ZWAVE_CONTROLLER_ENCAPSULATION_NONE)
          && (frame_length >= sizeof(ZW_SECURITY_NONCE_REPORT_FRAME))) {
        s0_register_nonce(
          c->remote.node_id,
          c->local.node_id,
          &frame_data[offsetof(ZW_SECURITY_NONCE_REPORT_FRAME, nonceByte1)]);
      }
      break;
    case SECURITY_COMMANDS_SUPPORTED_GET:
      if (c && c->local.is_multicast) {
        return SL_STATUS_OK;
      }

      if (c->encapsulation == ZWAVE_CONTROLLER_ENCAPSULATION_NONE) {
        return SL_STATUS_NOT_SUPPORTED;
      }
      zwave_keyset_t supporting_node_keys;
      zwave_controller_storage_get_node_granted_keys(
        zwave_network_management_get_node_id(),
        &supporting_node_keys);
      zwave_controller_encapsulation_scheme_t supporting_node_scheme
        = zwave_controller_get_highest_encapsulation(supporting_node_keys);
      sl_log_debug(LOG_TAG,
                   "supporting_node_scheme = %d\n",
                   supporting_node_scheme);
      if (supporting_node_scheme == ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0) {
        sl_log_error(LOG_TAG,
                     "Not implemented: ZPC's highest security scheme is "
                     "ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0\n");
        return SL_STATUS_NOT_SUPPORTED;
        // ZPC does not support to be included right now. So highest key will
        // never be S0
        //TODO:  Implement if ZPC has S0 as highest key.
        //https://jira.silabs.com/browse/UIC-794
        // Sending S0 supported command class get
      } else {  // Always respond with empty S0 command supported report for ZPC
        zwave_controller_connection_info_t c2;
        memcpy(&c2, c, sizeof(zwave_controller_connection_info_t));
        c2.encapsulation           = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0;
        zwave_tx_options_t options = {};
        const uint8_t empty_s0_command_supported_report[3]
          = {COMMAND_CLASS_SECURITY, SECURITY_COMMANDS_SUPPORTED_REPORT, 0x00};
        zwave_tx_send_data(&c2,
                           sizeof(empty_s0_command_supported_report),
                           empty_s0_command_supported_report,
                           &options,
                           0,
                           0,
                           0);
      }
      break;
    case SECURITY_MESSAGE_ENCAPSULATION_NONCE_GET:
    case SECURITY_MESSAGE_ENCAPSULATION:

      len = s0_decrypt_message(c,
                               rx_options,
                               encrypted_frame_data,
                               frame_length,
                               decrypted_frame);
      zwave_controller_connection_info_t c2;
      memcpy(&c2, c, sizeof(zwave_controller_connection_info_t));
      c2.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0;
      if (len) {
        zwave_controller_on_frame_received(&c2,
                                           rx_options,
                                           decrypted_frame,
                                           len);
      } else {
        return SL_STATUS_FAIL;
      }
      break;
    default:
      sl_log_debug(LOG_TAG,
                   "S0 Command: 0x%02X not supported\n",
                   frame_data[COMMAND_INDEX]);
      return (SL_STATUS_NOT_FOUND);
  }
  return SL_STATUS_OK;
}

sl_status_t
  zwave_s0_on_frame_received(const zwave_controller_connection_info_t *c,
                             const zwave_rx_receive_options_t *rx_options,
                             const uint8_t *frame_data1,
                             uint16_t frame_length_2_byte)
{
  return s0_application_command_handler(c,
                                        rx_options,
                                        frame_data1,
                                        frame_length_2_byte);
}

void free_all_tx_sessions()
{
  for (uint8_t i = 0; i < NUM_TX_SESSIONS; i++) {
    ctimer_stop(&tx_sessions[i].timer);
    memset(&tx_sessions[i], 0, sizeof(sec_tx_session_t));
  }
}

void free_nonce_block_list()
{
  for (uint8_t i = 0; i < NONCE_BLOCK_LIST_SIZE; i++) {
    memset(&nonce_block[i], 0, sizeof(nonce_block_t));
  }
}
void reset_block_next_elem()
{
  block_next_elem = 0;
}

void reset_s0_timers()
{
  ctimer_stop(&nonce_timer);
  ctimer_set(&nonce_timer, 1000, nonce_timer_timeout, 0);
}

sl_status_t zwave_s0_on_abort_send_data(zwave_tx_session_id_t session_id)
{
  if (zwave_tx_parent_session_id == session_id) {
    sl_log_debug(LOG_TAG,
                 "Aborting S0 send session for frame id=%p",
                 session_id);
    zwave_tx_parent_session_id       = NULL;
    zwave_tx_valid_parent_session_id = false;
    for (uint8_t i = 0; i < NUM_TX_SESSIONS; i++) {
      if (tx_sessions[i].session_id == session_id) {
        callback(TRANSMIT_COMPLETE_FAIL,
                 tx_sessions[i].tx_ext_status,
                 &tx_sessions[i]);
        break;
      }
    }
    return SL_STATUS_OK;
  }
  return SL_STATUS_NOT_FOUND;
}

sl_status_t zwave_s0_transport_init()
{
  static zwave_controller_transport_t transport = {
    .priority          = 4,  // see zwave_controller_transport->priority
    .command_class     = COMMAND_CLASS_SECURITY,
    .version           = COMMAND_CLASS_SECURITY_VERSION,
    .send_data         = zwave_s0_send_data,
    .abort_send_data   = zwave_s0_on_abort_send_data,
    .on_frame_received = zwave_s0_on_frame_received,
  };

  if (zwave_s0_network_init() != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }
  return zwave_controller_transport_register(&transport);
}
