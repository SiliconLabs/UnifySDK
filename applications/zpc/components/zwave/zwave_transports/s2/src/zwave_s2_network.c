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
#include "zwave_s2_network.h"

#include "etimer.h"
#include "s2_classcmd.h"
#include "s2_inclusion.h"
#include "s2_keystore.h"
#include "sl_log.h"
#include "zpc_endian.h"
#include "zwapi_protocol_transport.h"
#include "zwave_s2_internal.h"
#include "zwave_s2_keystore_int.h"

#define LOG_TAG "zwave_s2_network"

#define STR_CASE(x) \
  case x:           \
    return #x;

static zwave_s2_network_callbacks_t callbacks;
static zwave_node_id_t my_node_id;
const char *s2_inclusion_event_name(zwave_event_codes_t ev)
{
  static char message[25];
  switch (ev) {
    STR_CASE(S2_NODE_INCLUSION_INITIATED_EVENT)
    STR_CASE(S2_NODE_INCLUSION_KEX_REPORT_EVENT)
    STR_CASE(S2_NODE_INCLUSION_PUBLIC_KEY_CHALLENGE_EVENT)
    STR_CASE(S2_NODE_INCLUSION_COMPLETE_EVENT)
    STR_CASE(S2_NODE_JOINING_COMPLETE_EVENT)
    STR_CASE(S2_NODE_INCLUSION_FAILED_EVENT)
    STR_CASE(S2_JOINING_COMPLETE_NEVER_STARTED_EVENT)
    default:
      snprintf(message, sizeof(message), "%d", ev);
      return message;
  }
}

static void event_handler(zwave_event_t *event)
{
  sl_log_debug(LOG_TAG,
               "S2 inclusion event: %s\n",
               s2_inclusion_event_name(event->event_type));
  switch (event->event_type) {
    case S2_NODE_INCLUSION_INITIATED_EVENT:
      if (callbacks.on_inclusion_started) {
        callbacks.on_inclusion_started();
      }
      break;
    case S2_NODE_INCLUSION_PUBLIC_KEY_CHALLENGE_EVENT:
      if (callbacks.on_dsk_challenge) {
        assert(event->evt.s2_event.s2_data.challenge_req.length
               >= sizeof(zwave_dsk_t));
        callbacks.on_dsk_challenge(
          event->evt.s2_event.s2_data.challenge_req.granted_keys,
          event->evt.s2_event.s2_data.challenge_req.dsk_length,
          event->evt.s2_event.s2_data.challenge_req.public_key);
      }
      break;
    case S2_NODE_INCLUSION_KEX_REPORT_EVENT:
      if (callbacks.on_keys_request) {
        callbacks.on_keys_request(
          event->evt.s2_event.s2_data.kex_report.security_keys,
          event->evt.s2_event.s2_data.kex_report.csa);
      }
      break;
    case S2_NODE_INCLUSION_COMPLETE_EVENT:
    case S2_NODE_JOINING_COMPLETE_EVENT:
      // Security 2 event, inclusion of the node  to the network has been
      // completed.
      if (callbacks.on_inclusion_complete) {
        callbacks.on_inclusion_complete(
          event->evt.s2_event.s2_data.inclusion_complete.exchanged_keys,
          ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE);
      }
      zwave_s2_create_new_dynamic_ecdh_key();
      break;
    case S2_NODE_INCLUSION_FAILED_EVENT:
      // Security 2 event, inclusion of the node to the network has failed.
      if (callbacks.on_inclusion_complete) {
        zwave_kex_fail_type_t kex_fail_type
          = event->evt.s2_event.s2_data.inclusion_fail.kex_fail_type;
        if (ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE == kex_fail_type) {
          // In some cases libs2 sends S2_NODE_INCLUSION_FAILED_EVENT with
          // kex_fail type ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE,
          // in this case set kex_fail_type to UNKNOWN.
          kex_fail_type = ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_UNKNOWN;
        }
        callbacks.on_inclusion_complete(0, kex_fail_type);
      }
      zwave_s2_create_new_dynamic_ecdh_key();
      break;
  }
}

void zwave_s2_key_grant(bool accept, uint8_t keys, bool csa)
{
  s2_inclusion_key_grant(s2_ctx, accept, keys, csa);
}

void zwave_s2_dsk_accept(bool accept, uint8_t *dsk, uint8_t len)
{
  s2_inclusion_challenge_response(s2_ctx, accept, dsk, len);
}

void zwave_s2_network_init()
{
  zwave_home_id_t home_id;

  sl_log_debug(LOG_TAG, "Initializing Security 2 engine\n");

  home_id    = zwave_network_management_get_home_id();
  my_node_id = zwave_network_management_get_node_id();

  if (s2_ctx)
    S2_destroy(s2_ctx);

  s2_inclusion_init(
    SECURITY_2_SCHEME_1_SUPPORT,
    KEX_REPORT_CURVE_25519,
    SECURITY_2_SECURITY_2_CLASS_0 | SECURITY_2_SECURITY_2_CLASS_1
      | SECURITY_2_SECURITY_2_CLASS_2 | SECURITY_2_SECURITY_0_NETWORK_KEY);

  s2_ctx = S2_init_ctx(home_id);

  s2_inclusion_set_event_handler(&event_handler);
  zwave_s2_create_new_dynamic_ecdh_key();

#ifdef NDEBUG // If not debug build do not print the S2 keys 
  sl_log_info(LOG_TAG,
               "Use command line (zwave_log_security_keys) to log keys\n");
#else
  // Print out our keys on the console
  zwave_s2_log_security_keys(SL_LOG_INFO);
#endif
}

void zwave_s2_start_learn_mode(zwave_node_id_t node_id)
{
  s2_connection_t s2_connection = {0};

  /*We update the context here because the homeID has changed */
  zwave_s2_network_init();

  s2_connection.l_node        = my_node_id;
  s2_connection.r_node        = node_id;
  s2_connection.zw_tx_options = 0;

  // CC:009F.01.00.11.09C
  // - Authenticated Learn Mode ECDH key pair: This key pair is used for joining
  //   a network and when the S2 Bootstrapping requires Authentication. This key
  //   pair MUST be staticThe first one is static and is used for being S2
  //   bootstrapped when joining a new network (Learn Mode ECDH key pair).
  // - Unauthenticated Learn Mode ECDH key pair: This key pair is used for
  //   joining a network and when the S2 Bootstrapping does not require
  //   Authentication. This key pair MAY be dynamic.

  // Assume that we need to provide the static key
  // TODO: this part will change when we update the libs2 branch
  zwave_s2_keystore_set_ecdh_key_mode(ZWAVE_S2_KEYSTORE_STATIC_ECDH_KEY);
  s2_inclusion_joining_start(s2_ctx, &s2_connection, 0);
}

void zwave_s2_neighbor_discovery_complete()
{
  s2_inclusion_neighbor_discovery_complete(s2_ctx);
}

void zwave_s2_start_add_node(zwave_node_id_t node_id)
{
  s2_connection_t s2_connection = {0};

  s2_connection.l_node        = my_node_id;
  s2_connection.r_node        = node_id;
  s2_connection.zw_tx_options = 0;

  // CC:009F.01.00.11.09C
  // This keypair is used for adding nodes into a network. (controller side). //// This key pair MUST be dynamic
  zwave_s2_keystore_set_ecdh_key_mode(ZWAVE_S2_KEYSTORE_DYNAMIC_ECDH_KEY);
  s2_inclusion_including_start(s2_ctx, &s2_connection);
}

void zwave_s2_set_network_callbacks(const zwave_s2_network_callbacks_t *cb)
{
  callbacks = *cb;
}
