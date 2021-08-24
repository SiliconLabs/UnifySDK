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

#include "cmock.h"
#include "contiki_test_helper.h"
#include "process.h"

#include "S2_mock.h"
#include "s2_inclusion_mock.h"
#include "zwave_s2_network.h"
#include "zwave_s2_network_callbacks_mock.h"
#include "sl_log.h"

#include "zwave_network_management_mock.h"

PROCESS_NAME(zwave_s2_process);

CTR_DRBG_CTX s2_ctr_drbg;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp()
{
  contiki_test_helper_init();
}

void tearDown()
{
  process_exit(&zwave_s2_process);
}

static s2_event_handler_t my_event_handler;
static zwave_home_id_t
  zwave_network_management_get_home_id_CALLBACK(int cmock_num_calls)
{
  zwave_home_id_t homeID = 0xcafebabe;
  return homeID;
}
static zwave_node_id_t
  zwave_network_management_get_node_id_CALLBACK(int cmock_num_calls)
{
  zwave_node_id_t nodeID = 0x1;
  return nodeID;
}

static void
  my_s2_inclusion_set_event_handler_CALLBACK(s2_event_handler_t evt_handler,
                                             int cmock_num_calls)
{
  my_event_handler = evt_handler;
}

void test_s2_network_init()
{
  //Check that we a initializing libs2, S2_init and s2_inclusion init.
  zwave_network_management_get_home_id_StubWithCallback(
    zwave_network_management_get_home_id_CALLBACK);
  zwave_network_management_get_node_id_StubWithCallback(
    zwave_network_management_get_node_id_CALLBACK);

  s2_inclusion_init_IgnoreAndReturn(true);
  s2_inclusion_set_event_handler_StubWithCallback(
    my_s2_inclusion_set_event_handler_CALLBACK);
  S2_init_ctx_IgnoreAndReturn(0);

  zwave_s2_network_init();
}

/// Test case for add node
/// Test that we are calling s2_inclusion_including_start
void test_s2_add_node()
{
  zwave_node_id_t node      = 0x42;
  zwave_node_id_t this_node = 0x1;
  s2_connection_t peer      = {0};

  peer.r_node        = node;
  peer.l_node        = this_node;
  peer.zw_tx_options = 0;

  s2_inclusion_including_start_ExpectWithArray(0,
                                               0,
                                               &peer,
                                               sizeof(s2_connection_t));
  s2_inclusion_including_start_IgnoreArg_p_context();
  zwave_s2_start_add_node(node);
}

void test_s2_learn_mode()
{
  zwave_node_id_t node      = 0x42;
  zwave_node_id_t this_node = 0x1;
  s2_connection_t peer      = {0};

  peer.r_node        = node;
  peer.l_node        = this_node;
  peer.zw_tx_options = 0;

  //We expect S2 to be re-initialized because homeID have changed
  zwave_network_management_get_home_id_StubWithCallback(
    zwave_network_management_get_home_id_CALLBACK);
  zwave_network_management_get_node_id_StubWithCallback(
    zwave_network_management_get_node_id_CALLBACK);

  s2_inclusion_init_IgnoreAndReturn(true);
  s2_inclusion_set_event_handler_StubWithCallback(
    my_s2_inclusion_set_event_handler_CALLBACK);
  S2_init_ctx_IgnoreAndReturn(0);

  s2_inclusion_joining_start_ExpectWithArray(0,
                                             0,
                                             &peer,
                                             sizeof(s2_connection_t),
                                             0);
  s2_inclusion_joining_start_IgnoreArg_p_context();

  zwave_s2_start_learn_mode(node);
}

void test_s2_network_callbacks()
{
  uint8_t buff[100] = {0};
  zwave_event_t *ev = (zwave_event_t *)buff;
  zwave_dsk_t dsk   = {
    0,
    0,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
    0x1,
  };
  zwave_s2_network_callbacks_t cb
    = {.on_inclusion_started  = on_inclusion_started,
       .on_inclusion_complete = on_inclusion_complete,
       .on_keys_request       = on_keys_request,
       .on_dsk_challenge      = on_dsk_challenge};

  zwave_s2_set_network_callbacks(&cb);

  ev->event_type = S2_NODE_INCLUSION_INITIATED_EVENT;
  on_inclusion_started_Expect();
  my_event_handler(ev);

  ev->event_type                          = S2_NODE_INCLUSION_KEX_REPORT_EVENT;
  ev->evt.s2_event.s2_data.kex_report.csa = 0;
  ev->evt.s2_event.s2_data.kex_report.security_keys = 0x87;
  on_keys_request_Expect(0x87, 0);
  my_event_handler(ev);

  ev->event_type = S2_NODE_INCLUSION_PUBLIC_KEY_CHALLENGE_EVENT;
  ev->evt.s2_event.s2_data.challenge_req.dsk_length   = 2;
  ev->evt.s2_event.s2_data.challenge_req.granted_keys = 0x87;
  memcpy(&ev->evt.s2_event.s2_data.challenge_req.public_key[0],
         dsk,
         sizeof(zwave_dsk_t));
  on_dsk_challenge_ExpectWithArray(0x87, 2, dsk, sizeof(zwave_dsk_t));
  my_event_handler(ev);

  ev->event_type = S2_NODE_INCLUSION_COMPLETE_EVENT;
  ev->evt.s2_event.s2_data.inclusion_complete.exchanged_keys = 0x3;
  on_inclusion_complete_Expect(0x3, 0);
  my_event_handler(ev);

  ev->event_type = S2_NODE_JOINING_COMPLETE_EVENT;
  ev->evt.s2_event.s2_data.inclusion_complete.exchanged_keys = 0x43;
  on_inclusion_complete_Expect(0x43, 0);
  my_event_handler(ev);

  ev->event_type = S2_NODE_INCLUSION_FAILED_EVENT;
  ev->evt.s2_event.s2_data.inclusion_fail.kex_fail_type = 0x42;
  on_inclusion_complete_Expect(0, 0x42);
  my_event_handler(ev);
}
