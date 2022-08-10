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

#include "cmock.h"
#include "contiki_test_helper.h"
#include "process.h"

#include "S2_mock.h"
#include "s2_inclusion_mock.h"
#include "zwave_s2_network.h"
#include "zwave_s2_network_callbacks_mock.h"
#include "sl_log.h"
#include "zwapi_protocol_mem_mock.h"

// Includes from LibS2
#include "s2_protocol.h"

#include "zwave_network_management_mock.h"

PROCESS_NAME(zwave_s2_process);

CTR_DRBG_CTX s2_ctr_drbg;

// Create an S2 Context here:
static struct S2 test_ctx;
extern struct S2 *s2_ctx;

// Const test variables
static const zwave_home_id_t test_home_id = 0xcafebabe;
static const zwave_node_id_t test_node_id = 0x1;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp()
{
  // Configure our HomeID and NodeID for the test:
  zwave_network_management_get_home_id_IgnoreAndReturn(test_home_id);
  zwave_network_management_get_node_id_IgnoreAndReturn(test_node_id);
  s2_ctx = &test_ctx;
  memset(s2_ctx, 0, sizeof(struct S2));
  contiki_test_helper_init();
}

void tearDown()
{
  process_exit(&zwave_s2_process);
}

static s2_event_handler_t my_event_handler;

static void
  my_s2_inclusion_set_event_handler_CALLBACK(s2_event_handler_t evt_handler,
                                             int cmock_num_calls)
{
  my_event_handler = evt_handler;
}

void test_s2_network_init()
{
  S2_destroy_Expect(s2_ctx);
  s2_inclusion_init_IgnoreAndReturn(true);
  s2_inclusion_set_event_handler_StubWithCallback(
    my_s2_inclusion_set_event_handler_CALLBACK);
  S2_init_ctx_IgnoreAndReturn(0);

  zwapi_memory_get_buffer_IgnoreAndReturn(SL_STATUS_OK);

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
  zwave_node_id_t node = 0x42;
  s2_connection_t peer = {0};

  peer.r_node        = node;
  peer.l_node        = test_node_id;
  peer.zw_tx_options = 0;

  zwapi_memory_get_buffer_IgnoreAndReturn(SL_STATUS_OK);

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
