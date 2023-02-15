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
// Includes from the component being tested
#include "zwave_tx_callbacks.h"
#include "zwave_tx_fixt.h"
#include "zwave_tx_state_logging.h"
#include "zwave_tx_route_cache.h"

// Generic includes
#include <stdbool.h>
#include <string.h>

// Includes from other components
#include "sl_log.h"
#include "unity.h"

// CMocks
#include "zwave_controller_callbacks_mock.h"
#include "zwave_network_management_mock.h"
#include "zwave_controller_internal_mock.h"
#include "zwave_controller_transport_mock.h"
#include "zwave_tx_groups_mock.h"

// Includes from the Z-Wave TX
#include "zwave_tx_process.h"

// Contiki test resources
#include "contiki_test_helper.h"
#include "sys/process.h"

// Our test constants:
#include "zwave_tx_test.h"

static void (*zwave_transport_send_data_save)(
  uint8_t status, const zwapi_tx_report_t *tx_status, void *user);

// send_data_callback counters
static int send_done_count;
static uint8_t send_done_status;
static zwapi_tx_report_t send_done_tx_status;
static uint16_t my_user_pointer;
static void *received_user_pointer;
static zwave_tx_session_id_t test_tx_session_id;
static zwave_tx_session_id_t test_tx_session_id_2;
static zwave_tx_session_id_t test_tx_session_id_3;
static sl_status_t stub_status;
static zwapi_tx_report_t test_tx_report;
static zwave_nodemask_t test_nodemask;

static void send_data_callback(uint8_t status,
                               const zwapi_tx_report_t *tx_status,
                               void *user)
{
  send_done_count++;
  send_done_status      = status;
  send_done_tx_status   = *tx_status;
  received_user_pointer = user;
}

static sl_status_t zwave_transport_send_data_stub(
  uint8_t destination_node_id,
  const uint8_t *data,
  uint8_t data_length,
  uint8_t tx_options,
  on_zwave_tx_send_data_complete_t callback_function,
  int cmock_num_calls)
{
  zwave_transport_send_data_save = callback_function;
  return stub_status;
}

// Our callbacks
static const zwave_controller_callbacks_t *zwave_controller_callbacks;
static zwave_controller_reset_step_t tx_flush_reset_step;

static sl_status_t
  zwave_controller_callback_save(const zwave_controller_callbacks_t *cb, int n)
{
  zwave_controller_callbacks = cb;
  return SL_STATUS_OK;
}

static sl_status_t zwave_controller_reset_step_save(
  const zwave_controller_reset_step_t step_function,
  int32_t priority,
  int cmock_num_calls)
{
  TEST_ASSERT_NOT_NULL(step_function);
  tx_flush_reset_step = step_function;
  return SL_STATUS_OK;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  test_tx_report.ack_channel_number  = 3;
  test_tx_report.number_of_repeaters = 9;
  test_tx_report.routing_attempts    = 0x67;
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp()
{
  send_done_count                = 0;
  zwave_transport_send_data_save = NULL;
  stub_status                    = SL_STATUS_OK;
  tx_flush_reset_step            = NULL;
  memset(test_nodemask, 0, sizeof(zwave_nodemask_t));

  // By default, all tests are performed when Network Management is IDLE.
  zwave_network_management_get_state_IgnoreAndReturn(NM_IDLE);

  zwave_controller_transport_send_data_StopIgnore();
  zwave_controller_transport_send_data_AddCallback(
    (CMOCK_zwave_controller_transport_send_data_CALLBACK)
      zwave_transport_send_data_stub);

  // Always return the same NodeID for ourselves.
  zwave_network_management_get_node_id_IgnoreAndReturn(our_node_id);

  // Initialize the Command Class handler component.
  zwave_controller_register_callbacks_AddCallback(
    zwave_controller_callback_save);
  zwave_controller_register_callbacks_ExpectAndReturn(NULL, SL_STATUS_OK);
  zwave_controller_register_callbacks_IgnoreArg_callbacks();

  zwave_controller_register_reset_step_AddCallback(
    zwave_controller_reset_step_save);
  zwave_controller_register_reset_step_ExpectAndReturn(NULL, 4, SL_STATUS_OK);
  zwave_controller_register_reset_step_IgnoreArg_step_function();

  contiki_test_helper_init();
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_tx_fixt_setup());

  // Get the clock to start at something else than 0.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
}

void test_zwave_tx_reject_empty_frame()
{
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_tx_send_data(&test_connection_1,
                                       0,
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       (void *)&my_user_pointer,
                                       &test_tx_session_id));
}

void test_zwave_tx_try_to_send_empty_queue()
{
  // Nothing should happen here.
  process_post(&zwave_tx_process, ZWAVE_TX_SEND_NEXT_MESSAGE, 0);
  contiki_test_helper_run_once(DEFAULT_CONTIKI_CLOCK_JUMP);
  process_post(&zwave_tx_process, ZWAVE_TX_SEND_NEXT_MESSAGE, 0);
  contiki_test_helper_run_once(DEFAULT_CONTIKI_CLOCK_JUMP);
}

void test_zwave_tx_shutdown()
{
  // Nothing special will happen here.
  process_post(&zwave_tx_process, PROCESS_EVENT_EXIT, 0);
  contiki_test_helper_run_once(DEFAULT_CONTIKI_CLOCK_JUMP);

  process_post(&zwave_tx_process, PROCESS_EVENT_EXITED, 0);
  contiki_test_helper_run_once(DEFAULT_CONTIKI_CLOCK_JUMP);
}

void test_zwave_process_unhandled_event()
{
  // Simulate a send complete event when no transmission is ongoing.
  // It should get ignored
  process_post(&zwave_tx_process, ZWAVE_TX_SEND_OPERATION_COMPLETE, 0);
  contiki_test_helper_run_once(DEFAULT_CONTIKI_CLOCK_JUMP);
}

void test_zwave_tx_send_data_happy_case()
{
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_2,
    sizeof(test_tx_options_2),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);

  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // Queue up our frame that should be encrypted
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run_once(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Transport informs it is done.
  TEST_ASSERT_NOT_NULL(zwave_transport_send_data_save);
  zwave_transport_send_data_save(TRANSMIT_COMPLETE_VERIFIED,
                                 &test_tx_report,
                                 test_tx_session_id);

  // Now the child frame is fully done.
  contiki_test_helper_run_clock_increases_for_each_event(
    TX_BACKOFF_CONTIKI_CLOCK_JUMP);

  // Verify that it looks looks as expected
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_VERIFIED, send_done_status);
  // We expect that the transmission time is copied from TX
  // Contiki made a clock jump of DEFAULT_CONTIKI_CLOCK_JUMP for each event
  send_done_tx_status.transmit_ticks = 0;
  TEST_ASSERT_EQUAL_MEMORY(&test_tx_report,
                           &send_done_tx_status,
                           sizeof(zwapi_tx_report_t));
  TEST_ASSERT_EQUAL_PTR(NULL, received_user_pointer);

  // Bonus test. Try to call callbacks again, for non-existing frames
  zwave_transport_send_data_save(TRANSMIT_COMPLETE_OK,
                                 &test_tx_report,
                                 test_tx_session_id);

  // Nothing should have changed
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_VERIFIED, send_done_status);
  TEST_ASSERT_EQUAL_MEMORY(&test_tx_report,
                           &send_done_tx_status,
                           sizeof(zwapi_tx_report_t));
  TEST_ASSERT_EQUAL_PTR(NULL, received_user_pointer);
}

void test_zwave_tx_send_data_fast_track()
{
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);

  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // Queue up our frame that should be encrypted
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run_once(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Transport informs it is done, it failed
  TEST_ASSERT_NOT_NULL(zwave_transport_send_data_save);
  zwave_transport_send_data_save(TRANSMIT_COMPLETE_FAIL,
                                 &test_tx_report,
                                 test_tx_session_id);

  // Tx will try a requeue
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  zwave_controller_transport_send_data_IgnoreArg_tx_options();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  TEST_ASSERT_EQUAL(0, send_done_count);

  // Second attempt also fails
  TEST_ASSERT_NOT_NULL(zwave_transport_send_data_save);
  zwave_transport_send_data_save(TRANSMIT_COMPLETE_FAIL,
                                 &test_tx_report,
                                 test_tx_session_id);

  contiki_test_helper_run_clock_increases_for_each_event(
    TX_BACKOFF_CONTIKI_CLOCK_JUMP);

  // Verify that it looks looks as expected
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_FAIL, send_done_status);
  // We expect that the transmission time is copied from TX
  // Contiki made a clock jump of DEFAULT_CONTIKI_CLOCK_JUMP for each event
  send_done_tx_status.transmit_ticks = 0;
  TEST_ASSERT_EQUAL_MEMORY(&test_tx_report,
                           &send_done_tx_status,
                           sizeof(zwapi_tx_report_t));
  TEST_ASSERT_EQUAL_PTR(NULL, received_user_pointer);

  // Bonus test. Try to call callbacks again, for non-existing frames
  zwave_transport_send_data_save(TRANSMIT_COMPLETE_OK,
                                 &test_tx_report,
                                 test_tx_session_id);

  // Nothing should have changed
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_FAIL, send_done_status);
  TEST_ASSERT_EQUAL_MEMORY(&test_tx_report,
                           &send_done_tx_status,
                           sizeof(zwapi_tx_report_t));
  TEST_ASSERT_EQUAL_PTR(NULL, received_user_pointer);
}

void test_zwave_tx_send_data_null_callback()
{
  zwave_controller_transport_send_data_IgnoreAndReturn(SL_STATUS_OK);

  // Queue up our frame that should be encrypted
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_2,
                                       NULL,
                                       NULL,
                                       &test_tx_session_id));

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run_once(DEFAULT_CONTIKI_CLOCK_JUMP);

  // call the callback from the transport:
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_FAIL, NULL, NULL);

  // Now the child frame is fully done.
  contiki_test_helper_run_clock_increases_for_each_event(
    TX_BACKOFF_CONTIKI_CLOCK_JUMP);

  // Just checking that it did not crash here due to null pointer.
}

void test_zwave_tx_priority_and_back_off()
{
  // Frame 3, queued first with lower priority
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_3,
                                       sizeof(test_expected_frame_data_3),
                                       test_expected_frame_data_3,
                                       &test_tx_options_3,
                                       send_data_callback,
                                       &test_tx_session_id_3,
                                       &test_tx_session_id_3));

  // Frame 2
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  // Frame 1
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       &test_tx_session_id,
                                       &test_tx_session_id));

  // Tx Queue should try to send frame with test_tx_options_1 first
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Tx Queue should try to send frame with test_tx_options_2 second
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_2,
    sizeof(test_connection_2),
    sizeof(test_expected_frame_data_2),
    test_expected_frame_data_2,
    sizeof(test_expected_frame_data_2),
    &test_tx_options_2,
    sizeof(test_tx_options_2),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // Transmit ok
  // call the callback from the transport for element 1:
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id);

  // Get the clock incremented between transmit and callback
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // call the callback from the transport:
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_VERIFIED,
                                        NULL,
                                        test_tx_session_id_2);

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // We should be in back-off, nothing be sent until the back-off is passed.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Tx Queue should try to send frame with test_tx_options_3 last
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_3,
    sizeof(test_connection_3),
    sizeof(test_expected_frame_data_3),
    test_expected_frame_data_3,
    sizeof(test_expected_frame_data_3),
    &test_tx_options_3,
    sizeof(test_tx_options_3),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // Next element called when back-off expired
  contiki_test_helper_run(TX_BACKOFF_CONTIKI_CLOCK_JUMP);

  // If the transport callback before contiki executes, it should also work.
  on_zwave_transport_send_data_complete(25,
                                        &test_tx_report,
                                        test_tx_session_id_3);

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Test we received the correct data
  TEST_ASSERT_EQUAL(3, send_done_count);
  TEST_ASSERT_EQUAL(25, send_done_status);
  send_done_tx_status.transmit_ticks = 0;
  TEST_ASSERT_EQUAL_MEMORY(&test_tx_report,
                           &send_done_tx_status,
                           sizeof(zwapi_tx_report_t));
  TEST_ASSERT_EQUAL_PTR(&test_tx_session_id_3, received_user_pointer);
}

void test_zwave_tx_receive_transport_callback_in_different_order()
{
  // Frame 3, queued first with lower priority
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_3,
                                       sizeof(test_expected_frame_data_3),
                                       test_expected_frame_data_3,
                                       &test_tx_options_3,
                                       send_data_callback,
                                       &test_tx_session_id_3,
                                       &test_tx_session_id_3));

  // Frame 2
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       &test_tx_session_id_2,
                                       &test_tx_session_id_2));

  // Frame 1
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       &test_tx_session_id,
                                       &test_tx_session_id));

  // Tx Queue should try to send frame with test_tx_options_1 first
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Receive all 3 callbacks, should not happen but who knows
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id_2);
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id_3);
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id);

  // Transmit ok
  // Get the clock incremented between transmit and callback
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Test we received the correct data, we should be in back-off for session 2.
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
  TEST_ASSERT_EQUAL_PTR(&test_tx_session_id_2, received_user_pointer);

  // Get the clock incremented to get the other 2 callbacks
  contiki_test_helper_run(TX_BACKOFF_CONTIKI_CLOCK_JUMP);

  zwave_tx_process_log_state();
  // Test we received the correct data, we should be in back-off for session 2.
  TEST_ASSERT_EQUAL(3, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
  TEST_ASSERT_EQUAL_PTR(&test_tx_session_id_3, received_user_pointer);
}

void test_zwave_tx_receive_transport_cannot_handle_frame()
{
  // Frame 1
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       &send_data_callback,
                                       &test_tx_session_id,
                                       &test_tx_session_id));

  // Tx Queue should try to send frame, transport will say no!
  stub_status = SL_STATUS_FAIL;
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_FAIL);
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // We should have dropped the frame and called back.
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_FAIL, send_done_status);
  TEST_ASSERT_EQUAL_PTR(&test_tx_session_id, received_user_pointer);
}

void test_zwave_tx_discard_timeout()
{
  // Frame 2, discard timeout 400ms.
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       &test_tx_session_id_2,
                                       &test_tx_session_id_2));

  // Frame 1
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       &test_tx_session_id,
                                       &test_tx_session_id));

  // Tx Queue should try to send frame with test_tx_options_1 first
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // We are now waiting for more frames, or a transport callback.
  // Wait more than tx_options_2 discard timeout.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP + 400);

  // Transmit ok
  // call the callback from the transport for element 1:
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id);

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Test we received the correct data last, i.e. frame 2 discarded.
  TEST_ASSERT_EQUAL(2, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_FAIL, send_done_status);
  TEST_ASSERT_EQUAL_PTR(&test_tx_session_id_2, received_user_pointer);
}

void test_zwave_tx_queue_flush_while_empty()
{
  TEST_ASSERT_NOT_NULL(tx_flush_reset_step);
  TEST_ASSERT_EQUAL(SL_STATUS_ALREADY_INITIALIZED, tx_flush_reset_step());

  // Tx Queue is shut down now
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_tx_send_data(&test_connection_3,
                                       sizeof(test_expected_frame_data_3),
                                       test_expected_frame_data_3,
                                       &test_tx_options_3,
                                       send_data_callback,
                                       &test_tx_session_id_3,
                                       &test_tx_session_id_3));

  // Tx queue opens again after entering a new netowrk
  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks->on_new_network_entered);
  zwave_controller_callbacks->on_new_network_entered(1, 2, 3, 4);

  // Tx Queue is happy again
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_3,
                                       sizeof(test_expected_frame_data_3),
                                       test_expected_frame_data_3,
                                       &test_tx_options_3,
                                       send_data_callback,
                                       &test_tx_session_id_3,
                                       &test_tx_session_id_3));
}

void test_zwave_tx_queue_flush_while_idle()
{
  // Queue 3 frames:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_3,
                                       sizeof(test_expected_frame_data_3),
                                       test_expected_frame_data_3,
                                       &test_tx_options_3,
                                       send_data_callback,
                                       &test_tx_session_id_3,
                                       &test_tx_session_id_3));

  // Frame 2
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  // Queue up our frame that should be encrypted
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       &test_tx_session_id,
                                       &test_tx_session_id));

  TEST_ASSERT_NOT_NULL(tx_flush_reset_step);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, tx_flush_reset_step());

  // Tx Queue should try to send frame with test_tx_options_1 first
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run(0);

  // call the callback from the transport for element 1:
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id);

  // Tx Queue should try to send frame with test_tx_options_2 second
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_2,
    sizeof(test_connection_2),
    sizeof(test_expected_frame_data_2),
    test_expected_frame_data_2,
    sizeof(test_expected_frame_data_2),
    &test_tx_options_2,
    sizeof(test_tx_options_2),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  contiki_test_helper_run(0);

  // call the callback from the transport:
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id_2);

  // No back-off here, we are flushing the queue!

  // Tx Queue should try to send frame with test_tx_options_3 last
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_3,
    sizeof(test_connection_3),
    sizeof(test_expected_frame_data_3),
    test_expected_frame_data_3,
    sizeof(test_expected_frame_data_3),
    &test_tx_options_3,
    sizeof(test_tx_options_3),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  contiki_test_helper_run(0);

  // If the transport callback before contiki executes, it should also work.
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_FAIL,
                                        &test_tx_report,
                                        test_tx_session_id_3);

  // Now we expect to be notified that Tx Queue flush is completed
  zwave_controller_on_reset_step_complete_Expect(
    ZWAVE_CONTROLLER_TX_FLUSH_RESET_STEP_PRIORITY);

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
}

void test_zwave_tx_queue_flush_while_in_back_off()
{
  // Queue 3 frames:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_3,
                                       sizeof(test_expected_frame_data_3),
                                       test_expected_frame_data_3,
                                       &test_tx_options_3,
                                       send_data_callback,
                                       &test_tx_session_id_3,
                                       &test_tx_session_id_3));

  // Frame 2
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  // Queue up our frame that should be encrypted
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       &test_tx_session_id,
                                       &test_tx_session_id));

  // Tx Queue should try to send frame with test_tx_options_1 first
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run(0);

  // call the callback from the transport for element 1:
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id);

  // Tx Queue should try to send frame with test_tx_options_2 second
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_2,
    sizeof(test_connection_2),
    sizeof(test_expected_frame_data_2),
    test_expected_frame_data_2,
    sizeof(test_expected_frame_data_2),
    &test_tx_options_2,
    sizeof(test_tx_options_2),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  contiki_test_helper_run(0);

  // call the callback from the transport:
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id_2);

  // We are in back-off, flush the queue
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Tx Queue should try to send frame with test_tx_options_3 last
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_3,
    sizeof(test_connection_3),
    sizeof(test_expected_frame_data_3),
    test_expected_frame_data_3,
    sizeof(test_expected_frame_data_3),
    &test_tx_options_3,
    sizeof(test_tx_options_3),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  TEST_ASSERT_NOT_NULL(tx_flush_reset_step);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, tx_flush_reset_step());
  contiki_test_helper_run(0);

  // If the transport callback before contiki executes, it should also work.
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_FAIL,
                                        &test_tx_report,
                                        test_tx_session_id_3);

  // Now we expect to be notified that Tx Queue flush is completed
  zwave_controller_on_reset_step_complete_Expect(
    ZWAVE_CONTROLLER_TX_FLUSH_RESET_STEP_PRIORITY);

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
}

void test_zwave_tx_queue_flush_while_in_back_off_for_last_frame()
{
  // Queue just 1 frame with back-off
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  // Tx Queue tries to send our frame
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_2,
    sizeof(test_connection_2),
    sizeof(test_expected_frame_data_2),
    test_expected_frame_data_2,
    sizeof(test_expected_frame_data_2),
    &test_tx_options_2,
    sizeof(test_tx_options_2),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  contiki_test_helper_run(0);

  // call the callback from the transport:
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id_2);

  // We are in back-off, waiting for an answer, flush the queue
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Now we expect to be notified that Tx Queue flush is completed immediately,
  // because we were in back-off for the last element
  zwave_controller_on_reset_step_complete_Expect(
    ZWAVE_CONTROLLER_TX_FLUSH_RESET_STEP_PRIORITY);

  TEST_ASSERT_NOT_NULL(tx_flush_reset_step);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, tx_flush_reset_step());
}

void test_zwave_tx_queue_flush_while_transmission_ongoing_transport_aborts()
{
  // Frame 2
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  // Queue up our frame that should be encrypted
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       &test_tx_session_id,
                                       &test_tx_session_id));

  // Tx Queue should try to send frame with test_tx_options_1 first
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run(1);

  // Abort while ongoing.
  zwave_controller_transport_abort_send_data_ExpectAndReturn(test_tx_session_id,
                                                             SL_STATUS_OK);
  TEST_ASSERT_NOT_NULL(tx_flush_reset_step);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, tx_flush_reset_step());

  // call the callback from the transport for element 1:
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id);

  // Tx Queue should try to send frame with test_tx_options_2 second
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_2,
    sizeof(test_connection_2),
    sizeof(test_expected_frame_data_2),
    test_expected_frame_data_2,
    sizeof(test_expected_frame_data_2),
    &test_tx_options_2,
    sizeof(test_tx_options_2),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  contiki_test_helper_run(0);

  // call the callback from the transport:
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id_2);

  // Now we expect to be notified that Tx Queue flush is completed
  zwave_controller_on_reset_step_complete_Expect(
    ZWAVE_CONTROLLER_TX_FLUSH_RESET_STEP_PRIORITY);

  // No back-off on queue flush.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
}

void test_zwave_tx_queue_flush_while_transmission_ongoing_transport_no_abort()
{
  // Frame 2
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  // Queue up our frame that should be encrypted
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       &test_tx_session_id,
                                       &test_tx_session_id));

  // Tx Queue should try to send frame with test_tx_options_1 first
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run(1);

  // Abort while ongoing.
  zwave_controller_transport_abort_send_data_ExpectAndReturn(test_tx_session_id,
                                                             SL_STATUS_FAIL);
  TEST_ASSERT_NOT_NULL(tx_flush_reset_step);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, tx_flush_reset_step());

  // Tx Queue should try to send frame with test_tx_options_2 second
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_2,
    sizeof(test_connection_2),
    sizeof(test_expected_frame_data_2),
    test_expected_frame_data_2,
    sizeof(test_expected_frame_data_2),
    &test_tx_options_2,
    sizeof(test_tx_options_2),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  contiki_test_helper_run(0);

  // call the callback from the transport:
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id_2);

  // Now we expect to be notified that Tx Queue flush is completed
  zwave_controller_on_reset_step_complete_Expect(
    ZWAVE_CONTROLLER_TX_FLUSH_RESET_STEP_PRIORITY);

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
}

void test_zwave_tx_queue_flush_with_auto_follow_up_frame()
{
  // Prepare a multicast frame with automatic follow-ups
  const zwave_tx_options_t test_tx_options_with_follow_up
    = {.number_of_responses = 1, .qos_priority = 123, .send_follow_ups = true};

  // test connection_info structures
  const zwave_controller_connection_info_t test_connection
    = {.local  = {.node_id = 1, .endpoint_id = 0, .is_multicast = false},
       .remote = {.multicast_group = 1, .endpoint_id = 2, .is_multicast = true},
       .encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE};

  // Frame 2
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_with_follow_up,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  // Tx Queue sends our frame
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection,
    sizeof(test_connection),
    sizeof(test_expected_frame_data_2),
    test_expected_frame_data_2,
    sizeof(test_expected_frame_data_2),
    &test_tx_options_with_follow_up,
    sizeof(test_tx_options_with_follow_up),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();
  zwave_controller_transport_send_data_IgnoreArg_tx_options();

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run_once(1);

  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id_2);

  zwave_controller_transport_abort_send_data_ExpectAndReturn(
    test_tx_session_id_2,
    SL_STATUS_OK);
  TEST_ASSERT_NOT_NULL(tx_flush_reset_step);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, tx_flush_reset_step());

  // Reach an empty Tx Queue immmediately
  zwave_controller_on_reset_step_complete_Expect(
    ZWAVE_CONTROLLER_TX_FLUSH_RESET_STEP_PRIORITY);

  // Now the Tx queue will not enqueue a bunch of follow-up frames
  contiki_test_helper_run_once(1);
  zwave_tx_process_log_state();
}

void test_zwave_tx_send_test_frame_happy_case()
{
  zwave_controller_transport_send_data_ExpectAndReturn(NULL,
                                                       0,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       SL_STATUS_OK);

  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_tx_options();
  zwave_controller_transport_send_data_IgnoreArg_data();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // Queue up our frame that should be encrypted
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_test_frame(test_connection_1.remote.node_id,
                                             MINUS_4_DBM,
                                             send_data_callback,
                                             &test_tx_session_id,
                                             &test_tx_session_id));

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run_once(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Transport informs it is done.
  TEST_ASSERT_NOT_NULL(zwave_transport_send_data_save);
  zwapi_tx_report_t test_tx_report = {};
  zwave_transport_send_data_save(TRANSMIT_COMPLETE_OK,
                                 &test_tx_report,
                                 test_tx_session_id);

  // Now the child frame is fully done.
  contiki_test_helper_run_clock_increases_for_each_event(
    TX_BACKOFF_CONTIKI_CLOCK_JUMP);

  // Verify that it looks looks as expected
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
  // We expect that the transmission time is copied from TX
  // Contiki made a clock jump of DEFAULT_CONTIKI_CLOCK_JUMP for each event
  send_done_tx_status.transmit_ticks = 0;
  TEST_ASSERT_EQUAL_MEMORY(&test_tx_report,
                           &send_done_tx_status,
                           sizeof(zwapi_tx_report_t));
  TEST_ASSERT_EQUAL_PTR(&test_tx_session_id, received_user_pointer);
}

void test_zwave_tx_send_data_abort()
{
  // Queue up our frame
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       &test_tx_session_id,
                                       &test_tx_session_id));

  // Abort some unknown frame
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    zwave_tx_abort_transmission(test_tx_session_id + 1));

  // abort our frame already
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_abort_transmission(test_tx_session_id));

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // We should get notified
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_FAIL, send_done_status);
  TEST_ASSERT_EQUAL_PTR(&test_tx_session_id, received_user_pointer);
}

void test_zwave_tx_send_data_abort_ongoing()
{
  // Queue up our frame
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       &test_tx_session_id,
                                       &test_tx_session_id));

  // Send the frame
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);

  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // We are now waiting for more frames, or a transport callback.
  // abort our frame
  zwave_controller_transport_abort_send_data_ExpectAndReturn(test_tx_session_id,
                                                             SL_STATUS_OK);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_abort_transmission(test_tx_session_id));

  // Still waiting for transport callback, it said ok to abort that frame
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
  TEST_ASSERT_EQUAL(0, send_done_count);

  // call the callback from the transport:
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id);
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // We should get notified
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
  TEST_ASSERT_EQUAL_PTR(&test_tx_session_id, received_user_pointer);
}

void test_zwave_tx_send_data_abort_ongoing_transport_no_support()
{
  // Queue up our frame
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       &test_tx_session_id,
                                       &test_tx_session_id));

  // Send the frame
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);

  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // We are now waiting for more frames, or a transport callback.
  // abort our frame
  zwave_controller_transport_abort_send_data_ExpectAndReturn(test_tx_session_id,
                                                             SL_STATUS_FAIL);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_abort_transmission(test_tx_session_id));

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Don't wait for transport callback, it said not ok to abort that frame
  // We should get notified
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_FAIL, send_done_status);
  TEST_ASSERT_EQUAL_PTR(&test_tx_session_id, received_user_pointer);
}

void test_zwave_tx_send_data_abort_during_back_off()
{
  // Queue up our frame
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       &test_tx_session_id,
                                       &test_tx_session_id));

  // Send the frame
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_2,
    sizeof(test_tx_options_2),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);

  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Transport informs it is done.
  TEST_ASSERT_NOT_NULL(zwave_transport_send_data_save);
  zwave_transport_send_data_save(TRANSMIT_COMPLETE_OK,
                                 &test_tx_report,
                                 test_tx_session_id);

  // Now the child frame is fully done.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
  send_done_tx_status.transmit_ticks = 0;
  TEST_ASSERT_EQUAL_MEMORY(&test_tx_report,
                           &send_done_tx_status,
                           sizeof(zwapi_tx_report_t));
  TEST_ASSERT_EQUAL_PTR(&test_tx_session_id, received_user_pointer);

  // Abort during back-off
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_abort_transmission(test_tx_session_id));
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    zwave_tx_abort_transmission(test_tx_session_id));

  contiki_test_helper_run(0);
}

void test_zwave_tx_get_number_of_responses()
{
  zwave_tx_options_t tx_options  = {};
  tx_options.number_of_responses = 3;
  // Queue up a frame with 3 responses
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &tx_options,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  TEST_ASSERT_EQUAL(3, zwave_tx_get_number_of_responses(test_tx_session_id));

  // Queue a child with 2 responses
  tx_options.transport.valid_parent_session_id = true;
  tx_options.transport.parent_session_id       = test_tx_session_id;
  tx_options.number_of_responses               = 2;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &tx_options,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  // Total generated by this frame chain is 5.
  TEST_ASSERT_EQUAL(5, zwave_tx_get_number_of_responses(test_tx_session_id_2));
}

void test_zwave_tx_with_child_frames()
{
  zwave_tx_options_t tx_options = {};
  // Queue up a frame with 3 responses
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &tx_options,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &tx_options,
    sizeof(tx_options),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);

  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Queue a child while frame is ongoing
  tx_options.transport.valid_parent_session_id = true;
  tx_options.transport.parent_session_id       = test_tx_session_id;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &tx_options,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_2),
    test_expected_frame_data_2,
    sizeof(test_expected_frame_data_2),
    &tx_options,
    sizeof(tx_options),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);

  zwave_controller_transport_send_data_IgnoreArg_tx_options();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Child 1 callback
  TEST_ASSERT_NOT_NULL(zwave_transport_send_data_save);
  zwave_transport_send_data_save(TRANSMIT_COMPLETE_OK,
                                 NULL,
                                 test_tx_session_id_2);

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);

  TEST_ASSERT_NOT_NULL(zwave_transport_send_data_save);
  zwave_transport_send_data_save(TRANSMIT_COMPLETE_OK,
                                 NULL,
                                 test_tx_session_id);

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  TEST_ASSERT_EQUAL(2, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
}

void test_zwave_tx_with_child_frames_weird_order()
{
  zwave_tx_options_t tx_options = {};
  // Queue up a frame with 3 responses
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &tx_options,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &tx_options,
    sizeof(tx_options),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);

  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Queue a child while frame is ongoing
  tx_options.transport.valid_parent_session_id = true;
  tx_options.transport.parent_session_id       = test_tx_session_id;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &tx_options,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_2),
    test_expected_frame_data_2,
    sizeof(test_expected_frame_data_2),
    &tx_options,
    sizeof(tx_options),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);

  zwave_controller_transport_send_data_IgnoreArg_tx_options();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Parent frame callback before child
  TEST_ASSERT_NOT_NULL(zwave_transport_send_data_save);
  zwave_transport_send_data_save(TRANSMIT_COMPLETE_OK,
                                 NULL,
                                 test_tx_session_id);

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);

  TEST_ASSERT_NOT_NULL(zwave_transport_send_data_save);
  zwave_transport_send_data_save(TRANSMIT_COMPLETE_OK,
                                 NULL,
                                 test_tx_session_id_2);

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  TEST_ASSERT_EQUAL(2, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
}

void test_zwave_tx_resume_from_back_off()
{
  // Queue 2 different frames, the first one (frame 2) needs 1 answer.
  // Frame 3
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_3,
                                       sizeof(test_expected_frame_data_3),
                                       test_expected_frame_data_3,
                                       &test_tx_options_3,
                                       send_data_callback,
                                       &test_tx_session_id_3,
                                       &test_tx_session_id_3));

  // Frame 2
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  // We are not transmitting yet, so if an answer comes now from the destination
  // we ignore
  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks->on_rx_frame_received);
  zwave_controller_callbacks->on_rx_frame_received(
    test_connection_2.remote.node_id);

  // Tx Queue sends frame 2 first
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_2,
    sizeof(test_connection_2),
    sizeof(test_expected_frame_data_2),
    test_expected_frame_data_2,
    sizeof(test_expected_frame_data_2),
    &test_tx_options_2,
    sizeof(test_tx_options_2),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Transmit ok
  // call the callback from the transport for element 1:
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id_2);

  // We should be in back-off.
  contiki_test_helper_run(10);

  // Receive a frame from another NodeID in the meantime:
  zwave_controller_callbacks->on_rx_frame_received(
    test_connection_2.remote.node_id + 1);

  // We should still be in back-off.
  contiki_test_helper_run(10);

  // Receive a frame from our NodeID
  zwave_controller_callbacks->on_rx_frame_received(
    test_connection_2.remote.node_id);

  // Tx Queue should try to send frame 3 now, back-off is over due to answer
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_3,
    sizeof(test_connection_3),
    sizeof(test_expected_frame_data_3),
    test_expected_frame_data_3,
    sizeof(test_expected_frame_data_3),
    &test_tx_options_3,
    sizeof(test_tx_options_3),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  contiki_test_helper_run(0);

  // There should be no effect if we receive a frame from a node while
  // transmitting, when we don't expect any answer
  zwave_controller_callbacks->on_rx_frame_received(
    test_connection_3.remote.node_id);

  // Get the clock incremented between transmit and callback
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // call the callback from the transport:
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id_3);

  // Just run through the back-off here.
  contiki_test_helper_run(TX_BACKOFF_CONTIKI_CLOCK_JUMP);

  // Test we received the correct data in the callback
  TEST_ASSERT_EQUAL(2, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
}

void test_zwave_tx_receive_only_one_response_during_backoff()
{
  // Queue 2 different frames, the first one (frame 2) needs 1 answer.
  // Frame 3
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_3,
                                       sizeof(test_expected_frame_data_3),
                                       test_expected_frame_data_3,
                                       &test_tx_options_3,
                                       send_data_callback,
                                       &test_tx_session_id_3,
                                       &test_tx_session_id_3));

  // Frame 2
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2_multi_responses,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  // We are not transmitting yet, so if an answer comes now from the destination
  // we ignore
  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks->on_rx_frame_received);
  zwave_controller_callbacks->on_rx_frame_received(
    test_connection_2.remote.node_id);

  // Tx Queue sends frame 2 first
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_2,
    sizeof(test_connection_2),
    sizeof(test_expected_frame_data_2),
    test_expected_frame_data_2,
    sizeof(test_expected_frame_data_2),
    &test_tx_options_2_multi_responses,
    sizeof(test_tx_options_2_multi_responses),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Transmit ok
  // call the callback from the transport for element 1:
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id_2);

  // We should be in back-off.
  contiki_test_helper_run(10);

  // Receive a frame from another NodeID in the meantime:
  zwave_controller_callbacks->on_rx_frame_received(
    test_connection_2.remote.node_id + 1);

  // We should still be in back-off.
  contiki_test_helper_run(10);

  // Receive a frame from our NodeID
  zwave_controller_callbacks->on_rx_frame_received(
    test_connection_2.remote.node_id);

  // We should still be in back-off, we need 3 responses
  contiki_test_helper_run(20);

  // Tx Queue should try to send frame 3 now, back-off is over due to answer
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_3,
    sizeof(test_connection_3),
    sizeof(test_expected_frame_data_3),
    test_expected_frame_data_3,
    sizeof(test_expected_frame_data_3),
    &test_tx_options_3,
    sizeof(test_tx_options_3),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // Just run through the back-off here.
  contiki_test_helper_run(TX_BACKOFF_CONTIKI_CLOCK_JUMP);

  // There should be no effect if we receive a frame from a node while
  // transmitting, when we don't expect any answer
  zwave_controller_callbacks->on_rx_frame_received(
    test_connection_3.remote.node_id);

  // Get the clock incremented between transmit and callback
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // call the callback from the transport:
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id_3);

  contiki_test_helper_run(0);

  // Test we received the correct data in the callback
  TEST_ASSERT_EQUAL(2, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
}

void test_reject_too_long_frames()
{
  // Try to send a frame of 65000 bytes. Should be rejected
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_tx_send_data(&test_connection_2,
                                       65000,
                                       test_expected_frame_data_2,
                                       &test_tx_options_2_multi_responses,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));
}

void test_frame_with_invalid_parent()
{
  // Try to send a frame whose parent is valid but not in the queue
  // It will just queue it as a standalone frame.
  const zwave_tx_options_t test_tx_options
    = {.number_of_responses = 1,
       .discard_timeout_ms  = 400,
       .qos_priority        = 20,
       .fasttrack           = false,
       .send_follow_ups     = false,
       .transport           = {
                   .parent_session_id       = (zwave_tx_session_id_t)20,
                   .valid_parent_session_id = true,
                   .is_first_follow_up      = false,
                   .is_test_frame           = false,
                   .rf_power                = NORMAL_POWER,
                   .group_id                = ZWAVE_TX_INVALID_GROUP,
       }};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));
}

void test_reject_frame_with_our_node_id_as_destination()
{
  // Make a connection with our NodeID as destination
  const zwave_controller_connection_info_t test_connection = {
    .local  = {.node_id = 1, .endpoint_id = 0, .is_multicast = false},
    .remote = {.node_id = our_node_id, .endpoint_id = 2, .is_multicast = false},
    .encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
  };

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_tx_send_data(&test_connection,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));
}

void test_tx_logging_functions()
{
  // clang-format off
  // This is just for shiny coverage numbers.

  // Log the name of the tx state
  TEST_ASSERT_EQUAL_STRING("ZWAVE_TX_STATE_IDLE", zwave_tx_state_name(ZWAVE_TX_STATE_IDLE));
  TEST_ASSERT_EQUAL_STRING("ZWAVE_TX_STATE_TRANSMISSION_ONGOING", zwave_tx_state_name(ZWAVE_TX_STATE_TRANSMISSION_ONGOING));
  TEST_ASSERT_EQUAL_STRING("ZWAVE_TX_STATE_BACKOFF", zwave_tx_state_name(ZWAVE_TX_STATE_BACKOFF));
  TEST_ASSERT_EQUAL_STRING("Unknown", zwave_tx_state_name(23));

  // Log the name of the contiki events
  TEST_ASSERT_EQUAL_STRING("ZWAVE_TX_SEND_NEXT_MESSAGE", zwave_tx_event_name(ZWAVE_TX_SEND_NEXT_MESSAGE));
  TEST_ASSERT_EQUAL_STRING("ZWAVE_TX_SEND_OPERATION_COMPLETE", zwave_tx_event_name(ZWAVE_TX_SEND_OPERATION_COMPLETE));
  TEST_ASSERT_EQUAL_STRING("PROCESS_EVENT_NONE", zwave_tx_event_name(PROCESS_EVENT_NONE));
  TEST_ASSERT_EQUAL_STRING("PROCESS_EVENT_INIT", zwave_tx_event_name(PROCESS_EVENT_INIT));
  TEST_ASSERT_EQUAL_STRING("PROCESS_EVENT_POLL", zwave_tx_event_name(PROCESS_EVENT_POLL));
  TEST_ASSERT_EQUAL_STRING("PROCESS_EVENT_EXIT", zwave_tx_event_name(PROCESS_EVENT_EXIT));
  TEST_ASSERT_EQUAL_STRING("PROCESS_EVENT_SERVICE_REMOVED", zwave_tx_event_name(PROCESS_EVENT_SERVICE_REMOVED));
  TEST_ASSERT_EQUAL_STRING("PROCESS_EVENT_CONTINUE", zwave_tx_event_name(PROCESS_EVENT_CONTINUE));
  TEST_ASSERT_EQUAL_STRING("PROCESS_EVENT_MSG", zwave_tx_event_name(PROCESS_EVENT_MSG));
  TEST_ASSERT_EQUAL_STRING("PROCESS_EVENT_EXITED", zwave_tx_event_name(PROCESS_EVENT_EXITED));
  TEST_ASSERT_EQUAL_STRING("PROCESS_EVENT_TIMER", zwave_tx_event_name(PROCESS_EVENT_TIMER));
  TEST_ASSERT_EQUAL_STRING("PROCESS_EVENT_COM", zwave_tx_event_name(PROCESS_EVENT_COM));
  TEST_ASSERT_EQUAL_STRING("Unknown", zwave_tx_event_name(20));

  // Log the name of the tx back-off reasons
  TEST_ASSERT_EQUAL_STRING("BACKOFF_CURRENT_SESSION_ID", zwave_back_off_reason_name(BACKOFF_CURRENT_SESSION_ID));
  TEST_ASSERT_EQUAL_STRING("BACKOFF_EXPECTED_ADDITIONAL_FRAMES", zwave_back_off_reason_name(BACKOFF_EXPECTED_ADDITIONAL_FRAMES));
  TEST_ASSERT_EQUAL_STRING("BACKOFF_PROTOCOL_SENDING_FRAMES", zwave_back_off_reason_name(BACKOFF_PROTOCOL_SENDING_FRAMES));
  TEST_ASSERT_EQUAL_STRING("BACKOFF_INCOMING_UNSOLICITED_ROUTED_FRAME", zwave_back_off_reason_name(BACKOFF_INCOMING_UNSOLICITED_ROUTED_FRAME));
  TEST_ASSERT_EQUAL_STRING("Unknown", zwave_back_off_reason_name(BACKOFF_INCOMING_UNSOLICITED_ROUTED_FRAME+1));
  // clang-format on
}

void test_full_zwave_tx_queue()
{
  // We do not care about interactions with the transports for this test.
  zwave_controller_transport_send_data_IgnoreAndReturn(SL_STATUS_OK);

  // Queue a first element:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       (void *)&my_user_pointer,
                                       &test_tx_session_id_2));
  contiki_test_helper_run(0);

  for (int i = 1; i < ZWAVE_TX_QUEUE_BUFFER_SIZE; ++i) {
    TEST_ASSERT_EQUAL(SL_STATUS_OK,
                      zwave_tx_send_data(&test_connection_1,
                                         sizeof(test_expected_frame_data_1),
                                         test_expected_frame_data_1,
                                         &test_tx_options_1,
                                         send_data_callback,
                                         (void *)&my_user_pointer,
                                         &test_tx_session_id));
  }

  TEST_ASSERT_EQUAL(ZWAVE_TX_QUEUE_BUFFER_SIZE, zwave_tx_get_queue_size());

  // Now there is no more queue space:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       (void *)&my_user_pointer,
                                       &test_tx_session_id));

  // Get one free slot
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id_2);

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);

  // Now queueing should work again:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       (void *)&my_user_pointer,
                                       &test_tx_session_id));

  // And now we are full again:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       (void *)&my_user_pointer,
                                       &test_tx_session_id));
}

void test_additional_back_off()
{
  zwave_node_id_t chatty_node_id = 23;
  // Frame 3, queued first with lower priority
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_3,
                                       sizeof(test_expected_frame_data_3),
                                       test_expected_frame_data_3,
                                       &test_tx_options_3,
                                       send_data_callback,
                                       &test_tx_session_id_3,
                                       &test_tx_session_id_3));

  // Frame 2
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  TEST_ASSERT_EQUAL(2, zwave_tx_get_queue_size());

  // Tx Queue should try to send frame with test_tx_options_2 first, it has 1 expected reply
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_2,
    sizeof(test_connection_2),
    sizeof(test_expected_frame_data_2),
    test_expected_frame_data_2,
    sizeof(test_expected_frame_data_2),
    &test_tx_options_2,
    sizeof(test_tx_options_2),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Tell the Tx Queue that we will get a couple of surprise frames from NodeID 23.
  zwave_tx_set_expected_frames(chatty_node_id, 2);

  // Transmit ok
  // call the callback from the transport for element 2.
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id_2);

  // We should be in back-off, nothing be sent until the back-off is passed.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Receive our reply... and we should STILL be in back-off due to more incoming
  // frames
  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks->on_rx_frame_received);
  zwave_controller_callbacks->on_rx_frame_received(
    test_connection_2.remote.node_id);
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // receive one frame from NodeID 23... Still not enough, we want 2 frames from NodeID 23
  zwave_controller_callbacks->on_rx_frame_received(chatty_node_id);
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Now we are happy!
  zwave_controller_callbacks->on_rx_frame_received(chatty_node_id);

  // Green light, the Tx Queue opens fire again.
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_3,
    sizeof(test_connection_3),
    sizeof(test_expected_frame_data_3),
    test_expected_frame_data_3,
    sizeof(test_expected_frame_data_3),
    &test_tx_options_3,
    sizeof(test_tx_options_3),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // If the transport callback before contiki executes, it should also work.
  on_zwave_transport_send_data_complete(200,
                                        &test_tx_report,
                                        test_tx_session_id_3);

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Test we received the correct data
  TEST_ASSERT_EQUAL(2, send_done_count);
  TEST_ASSERT_EQUAL(200, send_done_status);
  send_done_tx_status.transmit_ticks = 0;
  TEST_ASSERT_EQUAL_MEMORY(&test_tx_report,
                           &send_done_tx_status,
                           sizeof(zwapi_tx_report_t));
  TEST_ASSERT_EQUAL_PTR(&test_tx_session_id_3, received_user_pointer);
}

void test_additional_back_off_modify_expected_frames_along_the_way()
{
  zwave_node_id_t chatty_node_id_1 = 23;
  zwave_node_id_t chatty_node_id_2 = 1256;
  // Frame 3, queued first with lower priority
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_3,
                                       sizeof(test_expected_frame_data_3),
                                       test_expected_frame_data_3,
                                       &test_tx_options_3,
                                       send_data_callback,
                                       &test_tx_session_id_3,
                                       &test_tx_session_id_3));

  // Frame 2
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  // Tx Queue should try to send frame with test_tx_options_2 first, it has 1 expected reply
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_2,
    sizeof(test_connection_2),
    sizeof(test_expected_frame_data_2),
    test_expected_frame_data_2,
    sizeof(test_expected_frame_data_2),
    &test_tx_options_2,
    sizeof(test_tx_options_2),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Tell the Tx Queue that we will get a some surprise frames from 2 chatty nodes.
  zwave_tx_set_expected_frames(chatty_node_id_1, 2);
  zwave_tx_set_expected_frames(chatty_node_id_2, 50);

  // Transmit ok
  // call the callback from the transport for element 2.
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id_2);

  // We should be in back-off, nothing be sent until the back-off is passed.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Receive our reply... and we should STILL be in back-off due to more incoming
  // frames
  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks->on_rx_frame_received);
  zwave_controller_callbacks->on_rx_frame_received(
    test_connection_2.remote.node_id);
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // receive one frame from NodeID 23... Still not enough, we want 2 frames from NodeID 23
  zwave_controller_callbacks->on_rx_frame_received(chatty_node_id_1);
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  zwave_controller_callbacks->on_rx_frame_received(chatty_node_id_2);
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Now we are happy with chatty_node_id_1!... but still not with chatty_node_id_2
  zwave_controller_callbacks->on_rx_frame_received(chatty_node_id_1);
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  zwave_controller_callbacks->on_rx_frame_received(chatty_node_id_2);
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  zwave_controller_callbacks->on_rx_frame_received(chatty_node_id_2);
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Now we get to know that we will get only 1 more frame:
  zwave_tx_set_expected_frames(chatty_node_id_2, 1);
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  zwave_controller_callbacks->on_rx_frame_received(chatty_node_id_2);

  // Green light, the Tx Queue opens fire again.
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_3,
    sizeof(test_connection_3),
    sizeof(test_expected_frame_data_3),
    test_expected_frame_data_3,
    sizeof(test_expected_frame_data_3),
    &test_tx_options_3,
    sizeof(test_tx_options_3),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // If the transport callback before contiki executes, it should also work.
  on_zwave_transport_send_data_complete(200,
                                        &test_tx_report,
                                        test_tx_session_id_3);

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Test we received the correct data
  TEST_ASSERT_EQUAL(2, send_done_count);
  TEST_ASSERT_EQUAL(200, send_done_status);
  send_done_tx_status.transmit_ticks = 0;
  TEST_ASSERT_EQUAL_MEMORY(&test_tx_report,
                           &send_done_tx_status,
                           sizeof(zwapi_tx_report_t));
  TEST_ASSERT_EQUAL_PTR(&test_tx_session_id_3, received_user_pointer);
}

void test_time_out_while_waiting_for_additional_frames()
{
  zwave_node_id_t chatty_node_id_1 = 200;
  zwave_node_id_t chatty_node_id_2 = 201;

  // Tell the Tx Queue that we have incoming frames:
  zwave_tx_set_expected_frames(chatty_node_id_1, 1);
  zwave_tx_set_expected_frames(chatty_node_id_2, 1);

  // Queue something, that will put us in back-off for additional frames, we will wait a few seconds.
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // receive one frame from chatty_node_id_1... Not enough
  zwave_controller_callbacks->on_rx_frame_received(chatty_node_id_1);

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // The frame promised by chatty_node_id_2 never comes after the back-off passes:
  // Tx Queue should try to send frame the next frame
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_2,
    sizeof(test_connection_2),
    sizeof(test_expected_frame_data_2),
    test_expected_frame_data_2,
    sizeof(test_expected_frame_data_2),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();
  contiki_test_helper_run(2800);
}

void test_additional_back_off_modify_expected_frames_just_before_we_transmit()
{
  zwave_node_id_t chatty_node_id = 299;
  // Frame 3, queued first with lower priority
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_3,
                                       sizeof(test_expected_frame_data_3),
                                       test_expected_frame_data_3,
                                       &test_tx_options_3,
                                       send_data_callback,
                                       &test_tx_session_id_3,
                                       &test_tx_session_id_3));

  // Frame 2
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  // Tx Queue should try to send frame with test_tx_options_2 first, it has 1 expected reply
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_2,
    sizeof(test_connection_2),
    sizeof(test_expected_frame_data_2),
    test_expected_frame_data_2,
    sizeof(test_expected_frame_data_2),
    &test_tx_options_2,
    sizeof(test_tx_options_2),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Transmit ok
  // call the callback from the transport for element 2.
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id_2);

  // Get into response expecting back-off
  contiki_test_helper_run_once(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Receive our reply... it's a green ligth to move forward
  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks->on_rx_frame_received);
  zwave_controller_callbacks->on_rx_frame_received(
    test_connection_2.remote.node_id);

  // We just posted a "Send next message" event while we are idle... But now,
  // somebody says: wait for a frame!
  zwave_tx_set_expected_frames(chatty_node_id, 1);

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Only when we receive the expect frame will we be sending the next one.
  // Now we are happy with chatty_node_id_1!... but still not with chatty_node_id_2
  zwave_controller_callbacks->on_rx_frame_received(chatty_node_id);

  // Green light, the Tx Queue opens fire again.
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_3,
    sizeof(test_connection_3),
    sizeof(test_expected_frame_data_3),
    test_expected_frame_data_3,
    sizeof(test_expected_frame_data_3),
    &test_tx_options_3,
    sizeof(test_tx_options_3),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // If the transport callback before contiki executes, it should also work.
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id_3);

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  zwave_tx_process_log_state();

  // Test we received the correct data
  TEST_ASSERT_EQUAL(2, send_done_count);
}

void test_log_functions()
{
  // Nothing to test here, just invoking the log functions in different
  // scenarios
  zwave_tx_log_queue(true);
  zwave_tx_log_queue(false);

  zwave_tx_log_element((zwave_tx_session_id_t)1, true);
  zwave_tx_log_element((zwave_tx_session_id_t)0, false);

  // Queue 1 frame:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_3,
                                       sizeof(test_expected_frame_data_3),
                                       test_expected_frame_data_3,
                                       &test_tx_options_3,
                                       send_data_callback,
                                       &test_tx_session_id_3,
                                       &test_tx_session_id_3));

  zwave_tx_log_queue(true);
  zwave_tx_log_queue(false);

  zwave_tx_log_element(test_tx_session_id_3, true);
  zwave_tx_log_element(test_tx_session_id_3, false);

  // Queue another frame
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  // Queue a third frame
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       (void *)&my_user_pointer,
                                       &test_tx_session_id));

  zwave_tx_log_queue(true);
  zwave_tx_log_queue(false);

  zwave_tx_log_element(test_tx_session_id_2, true);
  zwave_tx_log_element(test_tx_session_id_2, false);
}

void test_get_frame_pointers_and_lengths()
{
  TEST_ASSERT_NULL(zwave_tx_get_frame((zwave_tx_session_id_t)0));
  TEST_ASSERT_EQUAL(0, zwave_tx_get_frame_length((zwave_tx_session_id_t)0));

  TEST_ASSERT_NULL(zwave_tx_get_frame((zwave_tx_session_id_t)1));
  TEST_ASSERT_EQUAL(0, zwave_tx_get_frame_length((zwave_tx_session_id_t)1));

  // Queue up a frame
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_3,
                                       sizeof(test_expected_frame_data_3),
                                       test_expected_frame_data_3,
                                       &test_tx_options_3,
                                       send_data_callback,
                                       &test_tx_session_id_3,
                                       &test_tx_session_id_3));

  const uint8_t *frame_pointer = zwave_tx_get_frame(test_tx_session_id_3);
  TEST_ASSERT_EQUAL(sizeof(test_expected_frame_data_3),
                    zwave_tx_get_frame_length(test_tx_session_id_3));
  TEST_ASSERT_NOT_NULL(frame_pointer);

  TEST_ASSERT_EQUAL_UINT8_ARRAY(test_expected_frame_data_3,
                                frame_pointer,
                                sizeof(test_expected_frame_data_3));

  // Queue up another frame
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  frame_pointer = zwave_tx_get_frame(test_tx_session_id_2);
  TEST_ASSERT_EQUAL(sizeof(test_expected_frame_data_2),
                    zwave_tx_get_frame_length(test_tx_session_id_2));
  TEST_ASSERT_NOT_NULL(frame_pointer);

  TEST_ASSERT_EQUAL_UINT8_ARRAY(test_expected_frame_data_2,
                                frame_pointer,
                                sizeof(test_tx_session_id_2));

  // Get the frame 2 sent out. (modifies the container):
  zwave_controller_transport_send_data_IgnoreAndReturn(SL_STATUS_OK);
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Transmit ok
  // call the callback from the transport for element 2:
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id_2);
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Frame is available until the end of the back-off
  TEST_ASSERT_EQUAL(sizeof(test_expected_frame_data_2),
                    zwave_tx_get_frame_length(test_tx_session_id_2));

  contiki_test_helper_run(TX_BACKOFF_CONTIKI_CLOCK_JUMP);

  // Frame 2 should not be available anymore, but frame 3 is:
  TEST_ASSERT_EQUAL(0, zwave_tx_get_frame_length(test_tx_session_id_2));
  TEST_ASSERT_NULL(zwave_tx_get_frame(test_tx_session_id_2));

  frame_pointer = zwave_tx_get_frame(test_tx_session_id_3);
  TEST_ASSERT_EQUAL(sizeof(test_expected_frame_data_3),
                    zwave_tx_get_frame_length(test_tx_session_id_3));
  TEST_ASSERT_NOT_NULL(frame_pointer);

  TEST_ASSERT_EQUAL_UINT8_ARRAY(test_expected_frame_data_3,
                                frame_pointer,
                                sizeof(test_expected_frame_data_3));
}

void test_zwave_tx_decrement_expected_responses_for_parent_frames()
{
  // Queue a frame that requires a response, the first one (frame 2) needs 1 answer.
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  // Tx Queue sends frame 2 first
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_2,
    sizeof(test_connection_2),
    sizeof(test_expected_frame_data_2),
    test_expected_frame_data_2,
    sizeof(test_expected_frame_data_2),
    &test_tx_options_2,
    sizeof(test_tx_options_2),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Now we queue a child frame with no response:
  zwave_tx_options_t options                = {};
  options.transport.valid_parent_session_id = true;
  options.transport.parent_session_id       = test_tx_session_id_2;

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &options,
                                       NULL,
                                       NULL,
                                       &test_tx_session_id_3));

  // Queue an unrelated frame, it will have to wait for the first frame session
  // to be over, regardless of priority
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       (void *)&my_user_pointer,
                                       &test_tx_session_id));

  // Tx Queue sends the child frame next
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_2,
    sizeof(test_connection_2),
    sizeof(test_expected_frame_data_2),
    test_expected_frame_data_2,
    sizeof(test_expected_frame_data_2),
    &options,
    sizeof(options),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_tx_options();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Now while transmitting, we get an answer. It will decrement
  // the number of expected answers for the parent frame
  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks->on_rx_frame_received);
  zwave_controller_callbacks->on_rx_frame_received(
    test_connection_2.remote.node_id);

  // Transmit ok
  // call the callback from the transport for the child frame:
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id_3);

  // It goes back to the parent frame, where the transport which also makes a callback
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id_2);

  // User callback will be called as soon as we run contiki events
  TEST_ASSERT_EQUAL(0, send_done_count);

  // Tx Queue should try to send frame 3 now, no back-off
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // We should not be in backoff, as the answer to the parent frame was received.
  contiki_test_helper_run(10);

  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
}

void test_zwave_tx_backoff_for_protocol()
{
  zwave_network_management_get_state_StopIgnore();

  // Queue 1 frame. 1 expected reply.
  // Network management is assigning return routes.
  zwave_network_management_get_state_ExpectAndReturn(NM_ASSIGNING_RETURN_ROUTE);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2_multi_responses,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  // We should now initiate a back-off
  contiki_test_helper_run(1);

  // Nothing will happen.
  contiki_test_helper_run(10);

  // We can accept new frames
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       (void *)&my_user_pointer,
                                       &test_tx_session_id));

  // Nothing will happen still.
  contiki_test_helper_run(10);

  // If we pass the back-off, we ask again network management if we can go now.
  zwave_network_management_get_state_ExpectAndReturn(
    NM_NEIGHBOR_DISCOVERY_ONGOING);
  contiki_test_helper_run(1000);

  // Pass the back-off once more, with a state that allows us to transmit
  zwave_network_management_get_state_ExpectAndReturn(NM_WAIT_FOR_SECURE_ADD);
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();
  contiki_test_helper_run(1000);
}

void test_zwave_tx_has_frames_for_destination_with_backoff()
{
  TEST_ASSERT_FALSE(zwave_tx_has_frames_for_node(0));
  TEST_ASSERT_FALSE(zwave_tx_has_frames_for_node(1));
  TEST_ASSERT_FALSE(zwave_tx_has_frames_for_node(5000));

  // Frame to NodeID 3 is expecting responses, so there will be a back-off.
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2_multi_responses,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  TEST_ASSERT_TRUE(
    zwave_tx_has_frames_for_node(test_connection_2.remote.node_id));
  TEST_ASSERT_FALSE(
    zwave_tx_has_frames_for_node(test_connection_2.remote.node_id + 1));

  // Get the frame sent
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_2,
    sizeof(test_connection_2),
    sizeof(test_expected_frame_data_2),
    test_expected_frame_data_2,
    sizeof(test_expected_frame_data_2),
    &test_tx_options_2_multi_responses,
    sizeof(test_tx_options_2_multi_responses),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();
  contiki_test_helper_run(10);

  // We still have a frame in the queue for NodeID 3, as the transmission is
  // ongoing
  TEST_ASSERT_TRUE(
    zwave_tx_has_frames_for_node(test_connection_2.remote.node_id));
  TEST_ASSERT_FALSE(
    zwave_tx_has_frames_for_node(test_connection_2.remote.node_id + 1));

  // get a Send data complete, Transmit ok, we still have the frame in the queue
  // until the end of the back-off
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
                                        NULL,
                                        test_tx_session_id_2);

  TEST_ASSERT_TRUE(
    zwave_tx_has_frames_for_node(test_connection_2.remote.node_id));
  TEST_ASSERT_FALSE(
    zwave_tx_has_frames_for_node(test_connection_2.remote.node_id + 1));

  contiki_test_helper_run(10);

  TEST_ASSERT_TRUE(
    zwave_tx_has_frames_for_node(test_connection_2.remote.node_id));
  TEST_ASSERT_FALSE(
    zwave_tx_has_frames_for_node(test_connection_2.remote.node_id + 1));

  // Tell there is no more frame coming
  contiki_test_helper_run(TX_BACKOFF_CONTIKI_CLOCK_JUMP * 3);

  TEST_ASSERT_FALSE(
    zwave_tx_has_frames_for_node(test_connection_2.remote.node_id));
  TEST_ASSERT_FALSE(
    zwave_tx_has_frames_for_node(test_connection_2.remote.node_id + 1));
}

void test_zwave_tx_has_frames_for_destination_many_elements()
{
  TEST_ASSERT_FALSE(zwave_tx_has_frames_for_node(0));
  TEST_ASSERT_FALSE(zwave_tx_has_frames_for_node(1));
  TEST_ASSERT_FALSE(zwave_tx_has_frames_for_node(5000));

  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = 4;
  info.remote.is_multicast                = false;

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&info,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2_multi_responses,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  info.remote.node_id      = 5;
  info.remote.is_multicast = false;

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&info,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2_multi_responses,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  info.remote.node_id      = 1450;
  info.remote.is_multicast = false;

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&info,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2_multi_responses,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  TEST_ASSERT_FALSE(zwave_tx_has_frames_for_node(1));
  TEST_ASSERT_TRUE(zwave_tx_has_frames_for_node(4));
  TEST_ASSERT_TRUE(zwave_tx_has_frames_for_node(5));
  TEST_ASSERT_FALSE(zwave_tx_has_frames_for_node(6));
  TEST_ASSERT_TRUE(zwave_tx_has_frames_for_node(1450));

  // Try with a multicast group
  info.remote.node_id      = 20;
  info.remote.is_multicast = true;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&info,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2_multi_responses,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  zwave_tx_get_nodes_ExpectAndReturn(NULL, info.remote.node_id, SL_STATUS_OK);
  zwave_tx_get_nodes_IgnoreArg_nodes();
  zwave_tx_get_nodes_ReturnMemThruPtr_nodes(test_nodemask,
                                            sizeof(test_nodemask));
  TEST_ASSERT_FALSE(zwave_tx_has_frames_for_node(20));

  zwave_tx_get_nodes_ExpectAndReturn(NULL, info.remote.node_id, SL_STATUS_OK);
  zwave_tx_get_nodes_IgnoreArg_nodes();
  zwave_tx_get_nodes_ReturnMemThruPtr_nodes(test_nodemask,
                                            sizeof(test_nodemask));

  TEST_ASSERT_FALSE(zwave_tx_has_frames_for_node(1));
  TEST_ASSERT_TRUE(zwave_tx_has_frames_for_node(4));
  TEST_ASSERT_TRUE(zwave_tx_has_frames_for_node(5));

  zwave_tx_get_nodes_ExpectAndReturn(NULL, info.remote.node_id, SL_STATUS_OK);
  zwave_tx_get_nodes_IgnoreArg_nodes();
  zwave_tx_get_nodes_ReturnMemThruPtr_nodes(test_nodemask,
                                            sizeof(test_nodemask));
  TEST_ASSERT_FALSE(zwave_tx_has_frames_for_node(6));
  TEST_ASSERT_TRUE(zwave_tx_has_frames_for_node(1450));

  // Now add node 31 in group 20.
  ZW_ADD_NODE_TO_MASK(31, test_nodemask);
  zwave_tx_get_nodes_ExpectAndReturn(NULL, info.remote.node_id, SL_STATUS_OK);
  zwave_tx_get_nodes_IgnoreArg_nodes();
  zwave_tx_get_nodes_ReturnMemThruPtr_nodes(test_nodemask,
                                            sizeof(test_nodemask));
  TEST_ASSERT_TRUE(zwave_tx_has_frames_for_node(31));
}

void test_zwave_tx_apply_unsolicited_back_off_when_we_received_routed_frame()
{
  // Set something in the queue and send it
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);

  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();
  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run_once(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Get a callback, saying that we used repeaters to talk to the node.
  TEST_ASSERT_EQUAL(0, send_done_count);
  test_tx_report.number_of_repeaters = 3;
  TEST_ASSERT_NOT_NULL(zwave_transport_send_data_save);
  zwave_transport_send_data_save(TRANSMIT_COMPLETE_VERIFIED,
                                 &test_tx_report,
                                 test_tx_session_id);

  contiki_test_helper_run_once(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Verify that it looks looks as expected
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_VERIFIED, send_done_status);

  // Now the tx route cache knows we have 3 repeaters for NodeID 3
  // Put something more in the queue, but don't run the events.
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  // Receive an unsolicited routed frame before we look at the queue.
  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks->on_rx_frame_received);
  zwave_controller_callbacks->on_rx_frame_received(
    test_connection_1.remote.node_id);

  // Now we should be applying a 10 + 2x(3+1)*10ms = 90ms back-off.
  contiki_test_helper_run(0);
  contiki_test_helper_run(89);

  // When it expires we send the new frame to the transport
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);

  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();
  contiki_test_helper_run(10);
}

void test_zwave_tx_prolong_unsolicited_back_off_when_we_received_routed_frame()
{
  // Set something in the queue and send it
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);

  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();
  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run_once(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Get a callback, saying that we used repeaters to talk to the node.
  TEST_ASSERT_EQUAL(0, send_done_count);
  test_tx_report.number_of_repeaters = 3;
  TEST_ASSERT_NOT_NULL(zwave_transport_send_data_save);
  zwave_transport_send_data_save(TRANSMIT_COMPLETE_VERIFIED,
                                 &test_tx_report,
                                 test_tx_session_id);

  contiki_test_helper_run_once(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Verify that it looks looks as expected
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_VERIFIED, send_done_status);

  // Now the tx route cache knows we have 3 repeaters for NodeID 3
  // Put something more in the queue, but don't run the events.
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  // Receive an unsolicited routed frame before we look at the queue.
  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks->on_rx_frame_received);
  zwave_controller_callbacks->on_rx_frame_received(
    test_connection_1.remote.node_id);

  // Now we should be applying a 10 + 2x(3+1)*10ms = 90ms back-off.
  contiki_test_helper_run(0);
  contiki_test_helper_run(89);

  // Now if we received a new unsolicited routed frame, we back-off an extra 90ms
  zwave_controller_callbacks->on_rx_frame_received(
    test_connection_1.remote.node_id);

  contiki_test_helper_run(0);
  contiki_test_helper_run(89);

  // When it expires we send the new frame to the transport
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);

  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();
  contiki_test_helper_run(10);
}

void test_zwave_tx_do_not_cache_repeaters_for_multicast_destinations()
{
  // Get a multicast destination
  const zwave_controller_connection_info_t test_connection
    = {.local  = {.node_id = 1, .endpoint_id = 0, .is_multicast = false},
       .remote = {.multicast_group = 1, .endpoint_id = 2, .is_multicast = true},
       .encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE};

  // Set something in the queue and send it
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection,
    sizeof(test_connection),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);

  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run_once(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Get a callback, saying that we used repeaters to talk to the group.
  TEST_ASSERT_EQUAL(0, send_done_count);
  test_tx_report.number_of_repeaters = 3;
  TEST_ASSERT_NOT_NULL(zwave_transport_send_data_save);
  zwave_transport_send_data_save(TRANSMIT_COMPLETE_VERIFIED,
                                 &test_tx_report,
                                 test_tx_session_id);

  contiki_test_helper_run_once(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Directly check the cache, it should not have saved anything:
  TEST_ASSERT_EQUAL(0,
                    zwave_tx_route_cache_get_number_of_repeaters(
                      test_connection.remote.node_id));
}

void test_zwave_tx_do_not_cache_repeaters_failed_transmissions()
{
  // Set something in the queue and send it
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);

  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // We are now waiting for more frames, or a transport callback.
  contiki_test_helper_run_once(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Get a callback, saying that we used repeaters to talk to the group.
  // however, transmission was not successful
  TEST_ASSERT_EQUAL(0, send_done_count);
  test_tx_report.number_of_repeaters = 2;
  TEST_ASSERT_NOT_NULL(zwave_transport_send_data_save);
  zwave_transport_send_data_save(TRANSMIT_COMPLETE_NO_ACK,
                                 &test_tx_report,
                                 test_tx_session_id);

  contiki_test_helper_run_once(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Directly check the cache, it should not have saved anything:
  TEST_ASSERT_EQUAL(0,
                    zwave_tx_route_cache_get_number_of_repeaters(
                      test_connection_1.remote.node_id));
}

void test_zwave_tx_send_data_skip_back_off()
{
  // Expect a frame from NodeID 200.
  zwave_tx_set_expected_frames(200, 1);

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // We should be in back-off, now queue a frame that should by-pass the back-off
  zwave_tx_options_t test_tx_options                        = test_tx_options_1;
  test_tx_options.transport.ignore_incoming_frames_back_off = true;

  // Queue a frame that is allowed to skip back-off
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  // It should be sent immediately; the Tx Queue opens fire again.
  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options,
    sizeof(test_tx_options),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);
  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // If the transport callback before contiki executes, it should also work.
  TEST_ASSERT_NOT_NULL(zwave_transport_send_data_save);
  zwave_transport_send_data_save(TRANSMIT_COMPLETE_OK,
                                 &test_tx_report,
                                 test_tx_session_id);

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
  // Verify that it looks looks as expected
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);

  // Now we should be back in back-off, try with a frame that cannot skip back-off
  // Queue a frame that is allowed to skip back-off
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id_2));

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Now get our expected frames reduced to 0, so that we send the next frame.
  zwave_tx_set_expected_frames(200, 0);

  zwave_controller_transport_send_data_ExpectWithArrayAndReturn(
    &test_connection_1,
    sizeof(test_connection_1),
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    &test_tx_options_1,
    sizeof(test_tx_options_1),
    NULL,
    NULL,
    sizeof(void *),
    NULL,
    SL_STATUS_OK);

  // Connection is modified by the Tx Queue, ignore this here
  zwave_controller_transport_send_data_IgnoreArg_connection();
  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  TEST_ASSERT_NOT_NULL(zwave_transport_send_data_save);
  zwave_transport_send_data_save(TRANSMIT_COMPLETE_OK,
                                 &test_tx_report,
                                 test_tx_session_id_2);

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Verify that it looks looks as expected
  TEST_ASSERT_EQUAL(2, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
}
