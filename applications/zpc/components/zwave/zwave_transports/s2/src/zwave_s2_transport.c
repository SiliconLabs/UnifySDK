
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
#include "zwave_s2_transport.h"
#include "zwave_s2_keystore.h"
#include "zwave_s2_process.h"
#include "zwave_s2_internal.h"

// Includes from LibS2
#include "S2.h"
#include "s2_inclusion.h"
#include "s2_keystore.h"
#include "s2_protocol.h"
#include "s2_classcmd.h"

// Generic includes
#include <stdio.h>
#include <string.h>

// Includes from other components
#include "zwave_controller_connection_info.h"
#include "zwave_controller_internal.h"
#include "zwave_controller_storage.h"
#include "zwave_network_management.h"
#include "zwave_rx.h"
#include "zwave_tx.h"
#include "zwave_tx_groups.h"
#include "sys/clock.h"

#include "sl_log.h"
#define LOG_TAG "zwave_s2_transport"

/* Verify Delivery timeout in end nodes. Current 500 and 700 series nodes have a fixed timeout.
 * If that changes, this timeout must be calculated in a more sophisticated way. Unit: milliseconds. */
#define END_NODE_VERIFY_DELIVERY_TIMEOUT 500

//This is the time it takes a z-wave node to do 3*S2 frame decryption + generating a nonce report
#define NONCE_REP_TIME                       250
#define SECURITY_2_NONCE_GET                 0x01
#define SECURITY_2_COMMANDS_SUPPORTED_REPORT 0x0E
#define INVALID_KEY_ID                       0xFF

struct S2 *s2_ctx;

typedef enum { SINGLECAST, MULTICAST } zwave_s2_current_transmission_type_t;

// Z-Wave TX settings
static on_zwave_tx_send_data_complete_t s2_send_callback;
static zwave_tx_session_id_t zwave_tx_parent_session_id = NULL;
static bool zwave_tx_valid_parent_session_id            = false;
static zwave_s2_current_transmission_type_t current_transmission_type
  = SINGLECAST;
// FIXME: For now we make a local copy of the last sent frame,
// as Z-Wave TX does not guarantee that it will stay available.
// (actually in most case it is moved somewhere else when S2 tries to use it)
// Ideally we have TX to make a pointer available for us.
uint8_t last_frame_data[ZWAVE_MAX_FRAME_SIZE];
uint16_t last_frame_data_length = 0;

/* Holds the TX_STATUS_TYPE of ZW_SendDataXX() callback for the most recent S2 frame */
static zwapi_tx_report_t s2_send_tx_status;
static void *s2_send_user;
static clock_time_t transmit_start_time;
static uint8_t secure_nif[ZWAVE_MAX_FRAME_SIZE];
static uint8_t secure_nif_length;

static uint8_t
  encapsulation_to_class(zwave_controller_encapsulation_scheme_t encap)
{
  switch (encap) {
    case ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED:
      return 0;
    case ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED:
      return 1;
    case ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS:
      return 2;
    default:
      return INVALID_KEY_ID;
  }
}

static zwave_controller_encapsulation_scheme_t
  class_to_encapsulation(uint8_t key_class)
{
  switch (key_class) {
    case 0:
      return ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_UNAUTHENTICATED;
    case 1:
      return ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_AUTHENTICATED;
    case 2:
      return ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS;
    default:
      assert(0);
      return ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  }
}

/**
 * @brief Finds out which keyset to use for S2 transmissions to a NodeID.
 *
 * @param node_id       The NodeID to transmit to
 * @returns zwave_s2_keyset_t to use with S2_send_data functions.
 */
static zwave_s2_keyset_t get_s2_keyset_from_node(zwave_node_id_t node_id)
{
  zwave_s2_keyset_t keyset = UNKNOWN_KEYSET;
  zwave_protocol_t node_protocol
    = zwave_controller_storage_inclusion_protocol(node_id);
  switch (node_protocol) {
    case PROTOCOL_ZWAVE:
      keyset = ZWAVE_KEYSET;
      break;
    case PROTOCOL_ZWAVE_LONG_RANGE:
      keyset = ZWAVE_LONG_RANGE_KEYSET;
      break;
    default:
      sl_log_error(LOG_TAG,
                   "Unknown keyset for NodeID %d. S2 transmission will fail.",
                   node_id);
      keyset = UNKNOWN_KEYSET;
  }

  return keyset;
}

/**
 * @brief Finds out which keyset to use for S2 transmissions to a Group ID.
 *
 * @param group_id            The GroupID to transmit to
 * @returns zwave_s2_keyset_t to use with S2_send_data functions.
 */
static zwave_s2_keyset_t
  get_s2_keyset_from_group(zwave_multicast_group_id_t group_id)
{
  // We assume here that the group is uniform. Pick up the first NodeID from the
  // group and get its keyset.
  zwave_nodemask_t node_list = {};
  if (SL_STATUS_OK != zwave_tx_get_nodes(node_list, group_id)) {
    return UNKNOWN_KEYSET;
  }

  for (zwave_node_id_t node_id = ZW_MIN_NODE_ID; node_id <= ZW_LR_MAX_NODE_ID;
       node_id++) {
    if (ZW_IS_NODE_IN_MASK(node_id, node_list) == true) {
      return get_s2_keyset_from_node(node_id);
    }
  }
  // No node in the mask ?
  sl_log_warning(LOG_TAG,
                 "Unknown keyset for GroupID %d. S2 transmission will fail.",
                 group_id);
  return UNKNOWN_KEYSET;
}

static void send_frame_callback(uint8_t status,
                                const zwapi_tx_report_t *tx_info,
                                void *user)
{
  (void)user;
  if (tx_info) {
    s2_send_tx_status = *tx_info;
  }
  uint16_t tx_time = clock_time() - transmit_start_time;
  if (current_transmission_type == SINGLECAST) {
    tx_time += NONCE_REP_TIME;
  }
  S2_send_frame_done_notify(s2_ctx,
                            status == TRANSMIT_COMPLETE_OK
                              ? S2_TRANSMIT_COMPLETE_OK
                              : S2_TRANSMIT_COMPLETE_NO_ACK,
                            tx_time);
}

/*************************** S2 external functions ****************************/

void S2_get_hw_random(uint8_t *buf, uint8_t len)
{
  uint8_t rnd[32] = {0};
  int n, left, pos;

  left = len;
  pos  = 0;
  while (left > 0) {
    zwapi_get_random_word(rnd, sizeof(rnd));
    n = left > sizeof(rnd) ? sizeof(rnd) : left;
    memcpy(&buf[pos], rnd, n);
    left -= n;
    pos += n;
  }
}

// FIXME: LibS2 should probably have a public API for this.
// S2 passes us a Class ID and we must de-cipher else we believe that LR key
// classes are higher security.
#define UNAUTHENTICATED_KEY_SLOT  0
#define AUTHENTICATED_KEY_SLOT    1
#define ACCESS_KEY_SLOT           2
#define LR_AUTHENTICATED_KEY_SLOT 3
#define LR_ACCESS_KEY_SLOT        4
static uint8_t convert_lr_to_zwave_class_id(uint8_t class_id)
{
  if (class_id == LR_ACCESS_KEY_SLOT) {
    class_id = ACCESS_KEY_SLOT;
  } else if (class_id == LR_AUTHENTICATED_KEY_SLOT) {
    class_id = AUTHENTICATED_KEY_SLOT;
  }
  // Not a LR class ID does not require any convertion.
  return class_id;
}

void S2_get_commands_supported(node_t lnode,
                               uint8_t class_id,
                               const uint8_t **cmdClasses,
                               uint8_t *length)
{
  (void)lnode;
  class_id              = convert_lr_to_zwave_class_id(class_id);
  uint8_t key_mask      = 1 << class_id;
  uint8_t assigned_keys = zwave_s2_keystore_get_assigned_keys()
                          & SECURITY_2_KEY_MASK
                          & ~SECURITY_2_SECURITY_0_NETWORK_KEY;
  sl_log_info(LOG_TAG,
              "Received a S2 Supported Command Get on key class 0x%02X. "
              "Our granted S2 key class set is 0x%02X",
              key_mask,
              assigned_keys);

  if ((assigned_keys & key_mask) > (assigned_keys & ~key_mask)) {
    *cmdClasses = secure_nif;
    *length     = secure_nif_length;
  } else {
    *length = 0;
  }
}

void S2_send_done_event(struct S2 *ctxt, s2_tx_status_t status)
{
  /* The two zeros are padding to align with libs2 numeric value of S2_TRANSMIT_COMPLETE_VERIFIED */
  const uint8_t s2zw_codes[] = {TRANSMIT_COMPLETE_OK,
                                TRANSMIT_COMPLETE_NO_ACK,
                                TRANSMIT_COMPLETE_FAIL,
                                0,
                                0,
                                TRANSMIT_COMPLETE_VERIFIED};
  on_zwave_tx_send_data_complete_t cb_save;

  S2_stop_timeout(ctxt);
  cb_save          = s2_send_callback;
  s2_send_callback = NULL;
  if (cb_save) {
    cb_save(s2zw_codes[status], &s2_send_tx_status, s2_send_user);
  }

  // Forget about the parent frame if it triggered the transmission
  zwave_tx_valid_parent_session_id = false;
  memset(&s2_send_tx_status, 0, sizeof s2_send_tx_status);
}

void S2_msg_received_event(struct S2 *ctxt,
                           s2_connection_t *src,
                           uint8_t *buf,
                           uint16_t len)
{
  (void)ctxt;
  zwave_controller_connection_info_t info = {};
  zwave_rx_receive_options_t options      = {};

  info.encapsulation       = class_to_encapsulation(src->class_id);
  info.local.node_id       = src->l_node;
  info.local.endpoint_id   = 0;
  info.local.is_multicast = (src->rx_options & S2_RXOPTION_MULTICAST) != 0;

  info.remote.node_id      = src->r_node;
  info.remote.endpoint_id  = 0;
  
  options.rssi         = src->zw_rx_RSSIval;
  options.status_flags = src->zw_rx_status;

  zwave_controller_on_frame_received(&info, &options, buf, len);
}

uint8_t S2_send_frame(struct S2 *ctxt,
                      const s2_connection_t *conn,
                      uint8_t *buf,
                      uint16_t len)
{
  (void)ctxt;
  zwave_controller_connection_info_t info = {};
  zwave_tx_options_t options              = {};

  info.encapsulation  = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  info.local.node_id  = conn->l_node;
  info.remote.node_id = conn->r_node;

  // If there is no parent frame, S2 will fallback on these settings (e.g. inclusion S2 frames)
  options.qos_priority = ZWAVE_TX_QOS_RECOMMENDED_TIMING_CRITICAL_PRIORITY;
  options.number_of_responses = 1;
  options.discard_timeout_ms  = 0;

  // If the call initiated from outside S2, the parent frame options will take precedence.
  options.valid_parent_session_id = zwave_tx_valid_parent_session_id;
  options.parent_session_id       = zwave_tx_parent_session_id;
  if (zwave_tx_valid_parent_session_id == true) {
    options.number_of_responses = 0;
  }

  // One exception, Nonce Get will require a response.
  if (len >= 2 && buf[1] == SECURITY_2_NONCE_GET) {
    options.number_of_responses = 1;
  }

  transmit_start_time = clock_time();
  return SL_STATUS_OK
         == zwave_tx_send_data(&info,
                               len,
                               buf,
                               &options,
                               send_frame_callback,
                               0,
                               0);
}

uint8_t S2_send_frame_no_cb(struct S2 *ctxt,
                            const s2_connection_t *conn,
                            uint8_t *buf,
                            uint16_t len)
{
  (void)ctxt;
  zwave_controller_connection_info_t info = {};
  zwave_tx_options_t options              = {};

  info.encapsulation          = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  info.local.node_id          = conn->l_node;
  info.remote.node_id         = conn->r_node;
  options.number_of_responses = 1;
  options.discard_timeout_ms  = NONCE_REP_TIME;
  // Nonce report have to be delivered really quick. We up their OoS to be 10 levels
  // above an S2 bootstrapping. They take precedence over anything else.
  options.qos_priority = ZWAVE_TX_QOS_RECOMMENDED_TIMING_CRITICAL_PRIORITY
                         + 10 * ZWAVE_TX_RECOMMENDED_QOS_GAP;

  return SL_STATUS_OK
         == zwave_tx_send_data(&info, len, buf, &options, NULL, 0, 0);
}

uint8_t S2_send_frame_multi(struct S2 *ctxt,
                            s2_connection_t *conn,
                            uint8_t *buf,
                            uint16_t len)
{
  (void)ctxt;
  zwave_controller_connection_info_t info = {};
  zwave_tx_options_t options              = {};

  info.encapsulation       = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  info.local.node_id       = conn->l_node;
  info.remote.node_id      = conn->r_node;
  info.remote.is_multicast = true;

  // Set the Z-Wave TX Options
  options.group_id                = conn->r_node;
  options.valid_parent_session_id = zwave_tx_valid_parent_session_id;
  options.parent_session_id       = zwave_tx_parent_session_id;

  transmit_start_time = clock_time();
  return SL_STATUS_OK
         == zwave_tx_send_data(&info,
                               len,
                               buf,
                               &options,
                               send_frame_callback,
                               0,
                               0);
}
/************************* Our interface functions ****************************/

sl_status_t
  zwave_s2_send_data(const zwave_controller_connection_info_t *connection,
                     uint16_t data_length,
                     const uint8_t *cmd_data,
                     const zwave_tx_options_t *tx_options,
                     const on_zwave_tx_send_data_complete_t on_send_complete,
                     void *user,
                     zwave_tx_session_id_t parent_session_id)
{
  on_zwave_tx_send_data_complete_t cb_save;
  void *user_save;
  s2_connection_t s2_connection = {};

  s2_connection.l_node = connection->local.node_id;
  if (connection->remote.is_multicast == false) {
    uint8_t responses = zwave_tx_get_number_of_responses(parent_session_id);
    s2_connection.tx_options = responses > 0 ? S2_TXOPTION_VERIFY_DELIVERY : 0;
  }
  s2_connection.class_id = encapsulation_to_class(connection->encapsulation);
  if (s2_connection.class_id == INVALID_KEY_ID) {
    return SL_STATUS_NOT_SUPPORTED;
  }

  // Can we handle the frame size?
  if (data_length > sizeof(last_frame_data)) {
    sl_log_error(LOG_TAG,
                 "Too large frame requested for transmitting. Discarding\n");
    return SL_STATUS_WOULD_OVERFLOW;
  }

  cb_save   = s2_send_callback;
  user_save = s2_send_user;

  // Call for sending data comes from outside S2, so we use the parent frame functionality
  zwave_tx_parent_session_id       = parent_session_id;
  zwave_tx_valid_parent_session_id = true;

  // save the frame data
  last_frame_data_length = data_length;
  memcpy(last_frame_data, cmd_data, last_frame_data_length);

  s2_send_callback = on_send_complete;
  s2_send_user     = user;

  if (connection->remote.is_multicast == false) {
    // Singlecast message.
    current_transmission_type   = SINGLECAST;
    s2_connection.r_node        = connection->remote.node_id;
    s2_connection.zw_tx_options = 0;  // Z-Wave TX Queue takes the decision here

    // Is it a Singlecast follow-up ?
    if (tx_options->group_id != ZWAVE_TX_INVALID_GROUP) {
      s2_connection.tx_options |= S2_TXOPTION_SINGLECAST_FOLLOWUP;
      s2_connection.mgrp_group_id = tx_options->group_id;
      if (tx_options->is_first_follow_up == true) {
        s2_connection.tx_options |= S2_TXOPTION_FIRST_SINGLECAST_FOLLOWUP;
      }

      // Always ask for verify delivery on those follow-up frames.
      s2_connection.tx_options |= S2_TXOPTION_VERIFY_DELIVERY;

      if (S2_send_data_singlecast_follow_up_with_keyset(
            s2_ctx,
            &s2_connection,
            get_s2_keyset_from_node(connection->remote.node_id),
            last_frame_data,
            last_frame_data_length)) {
        return SL_STATUS_OK;
      } else {
        s2_send_callback = cb_save;
        s2_send_user     = user_save;
        return SL_STATUS_BUSY;
      }
    }

    // Else it is a regular singlecast frame
    if (S2_send_data_singlecast_with_keyset(
          s2_ctx,
          &s2_connection,
          get_s2_keyset_from_node(connection->remote.node_id),
          last_frame_data,
          last_frame_data_length)) {
      return SL_STATUS_OK;
    } else {
      s2_send_callback = cb_save;
      s2_send_user     = user_save;
      return SL_STATUS_BUSY;
    }
  } else {
    // Multicast message.
    s2_connection.r_node      = connection->remote.multicast_group;
    current_transmission_type = MULTICAST;

    if (S2_send_data_multicast_with_keyset(
          s2_ctx,
          &s2_connection,
          get_s2_keyset_from_group(connection->remote.multicast_group),
          last_frame_data,
          last_frame_data_length)) {
      return SL_STATUS_OK;
    } else {
      s2_send_callback = cb_save;
      s2_send_user     = user_save;
      return SL_STATUS_BUSY;
    }
  }
}

sl_status_t zwave_s2_on_frame_received(
  const zwave_controller_connection_info_t *connection_info,
  const zwave_rx_receive_options_t *rx_options,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  uint8_t frame_buffer[ZWAVE_MAX_FRAME_SIZE];
  s2_connection_t s2_connection = {};

  //Only process frames that has not yet been de-encapsulated here
  if (connection_info->encapsulation != ZWAVE_CONTROLLER_ENCAPSULATION_NONE) {
    return SL_STATUS_NOT_FOUND;
  }

  if (frame_data[0] != COMMAND_CLASS_SECURITY_2) {
    return SL_STATUS_NOT_FOUND;
  }

  s2_connection.r_node        = connection_info->remote.node_id;
  s2_connection.l_node        = connection_info->local.node_id;
  s2_connection.zw_rx_status  = rx_options->status_flags;
  s2_connection.zw_rx_RSSIval = rx_options->rssi;
  s2_connection.rx_options
    = rx_options->status_flags
          & (RECEIVE_STATUS_TYPE_BROAD | RECEIVE_STATUS_TYPE_MULTI)
        ? S2_RXOPTION_MULTICAST
        : 0;

  if (frame_length >= sizeof(frame_buffer)) {
    assert(0);
    return SL_STATUS_WOULD_OVERFLOW;
  }
  // As libs2 is doing inplace decryption of the frame to save memory, we need
  // to make a copy of the frame here.
  memcpy(frame_buffer, frame_data, frame_length);

  // Note that the S2_msg_received_event may be called directly by
  // S2_application_command_handler
  S2_application_command_handler(s2_ctx,
                                 &s2_connection,
                                 frame_buffer,
                                 frame_length);
  return SL_STATUS_OK;
}

sl_status_t zwave_s2_set_secure_nif(const uint8_t *nif, uint8_t nif_length)
{
  if (nif_length > sizeof(secure_nif)) {
    return SL_STATUS_WOULD_OVERFLOW;
  }

  memcpy(secure_nif, nif, nif_length);
  secure_nif_length = nif_length;
  return SL_STATUS_OK;
}

void zwave_s2_transport_init()
{
  s2_send_callback                 = NULL;
  zwave_tx_parent_session_id       = NULL;
  zwave_tx_valid_parent_session_id = false;
  current_transmission_type        = SINGLECAST;
  memset(last_frame_data, 0, sizeof(last_frame_data));
  last_frame_data_length = 0;
}

void S2_resynchronization_event(node_t remote_node,
                                sos_event_reason_t reason,
                                uint8_t seqno,
                                node_t local_node)
{
  (void)remote_node;
  (void)reason;
  (void)seqno;
  (void)local_node;
  // It is optional to react to this event, so we choose to do nothing.
  // See the Z/IP Gateway 7.14.2 Software Release Notes
  // (and hopefully soon the libs2 doxygen)
  // for suggestions on what could be done here.
}

void zwave_s2_on_on_multicast_group_deleted(zwave_multicast_group_id_t group_id)
{
  sl_log_debug(LOG_TAG, "Deleting MPAN Group ID %d", group_id);
  S2_free_mpan(s2_ctx, 0, group_id);
}

sl_status_t zwave_s2_abort_send_data(zwave_tx_session_id_t session_id)
{
  if (zwave_tx_parent_session_id == session_id) {
    sl_log_debug(LOG_TAG,
                 "Aborting S2 send session for frame id=%p",
                 session_id);
    zwave_tx_parent_session_id       = NULL;
    zwave_tx_valid_parent_session_id = false;
    send_frame_callback(TRANSMIT_COMPLETE_FAIL, &s2_send_tx_status, NULL);
    return SL_STATUS_OK;
  }
  return SL_STATUS_NOT_FOUND;
}