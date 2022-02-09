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
  TEST_ASSERT_EQUAL_PTR(NULL, received_user_pointer);

  // Bonus test. Try to call callbacks again, for non-existing frames
  zwave_transport_send_data_save(TRANSMIT_COMPLETE_OK,
                                 &test_tx_report,
                                 test_tx_session_id);

  // Nothing should have changed
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
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
  on_zwave_transport_send_data_complete(TRANSMIT_COMPLETE_OK,
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
  zwave_tx_log_queue(true);

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
  tx_options.valid_parent_session_id = true;
  tx_options.parent_session_id       = test_tx_session_id;
  tx_options.number_of_responses     = 2;
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
  tx_options.valid_parent_session_id = true;
  tx_options.parent_session_id       = test_tx_session_id;
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
  tx_options.valid_parent_session_id = true;
  tx_options.parent_session_id       = test_tx_session_id;
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
