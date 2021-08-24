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
// Generic includes
#include <stdbool.h>
#include <string.h>

// Includes from other components
#include "sl_log.h"
#include "unity.h"

// CMocks
#include "zwapi_protocol_transport_mock.h"
#include "zwapi_protocol_mem_mock.h"
#include "zwapi_protocol_basis_mock.h"
#include "zwapi_protocol_controller_mock.h"
#include "zwave_controller_callbacks_mock.h"
#include "zwave_network_management_mock.h"
#include "zwave_controller_internal_mock.h"
#include "zwave_tx_callbacks.h"

// Includes from the Z-Wave TX
#include "zwave_tx_process.h"

// Contiki test resources
#include "contiki_test_helper.h"
#include "sys/process.h"

// Our test constants:
#include "zwave_tx_test.h"

static void (*send_data_callback_save)(uint8_t status,
                                       zwapi_tx_report_t *tx_status);
/*static void (*send_nop_callback_save)(uint8_t status,
                                      zwapi_tx_report_t *tx_status);*/
static void (*zwapi_send_test_frame_callback_save)(uint8_t status);
static void (*send_data_multi_callback_save)(uint8_t status);
static void (*zwave_transport_send_data_save)(
  uint8_t status, const zwapi_tx_report_t *tx_status, void *user);

// send_data_callback counters
static int send_done_count;
static uint8_t send_done_status;
static zwapi_tx_report_t send_done_tx_status;
static uint16_t my_user_pointer;
static void *received_user_pointer;
static zwave_tx_session_id_t test_tx_session_id;
static zwave_tx_options_t test_tx_options = {};
static sl_status_t stub_status;
static zwapi_tx_report_t test_tx_report;
static uint8_t test_frame_data_request_nif[] = {0x01, 0x02};
//static uint8_t test_frame_data_nop[]         = {0x00};

static void send_data_callback(uint8_t status,
                               const zwapi_tx_report_t *tx_status,
                               void *user)
{
  send_done_count++;
  send_done_status      = status;
  send_done_tx_status   = *tx_status;
  received_user_pointer = user;
}

static sl_status_t zwapi_send_data_stub(
  zwave_node_id_t destination_node_id,
  const uint8_t *data,
  uint8_t data_length,
  uint8_t tx_options,
  cmock_zwapi_protocol_transport_func_ptr1 callback_function,
  int cmock_num_calls)
{
  send_data_callback_save = callback_function;
  return SL_STATUS_OK;
}

static sl_status_t zwapi_send_test_frame_stub(
  zwave_node_id_t destination_node_id,
  rf_power_level_t power_level,
  cmock_zwapi_protocol_basis_func_ptr2 callback_function,
  int cmock_num_calls)
{
  zwapi_send_test_frame_callback_save = callback_function;
  return SL_STATUS_OK;
}

static sl_status_t zwapi_send_data_multi_stub(
  zwave_nodemask_t destination_node_mask,
  const uint8_t *data,
  uint8_t data_length,
  uint8_t tx_options,
  cmock_zwapi_protocol_transport_func_ptr3 callback_function,
  int cmock_num_calls)
{
  send_data_multi_callback_save = callback_function;
  return SL_STATUS_OK;
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
  test_tx_report.bACKChannelNo = 3;
  test_tx_report.bRepeaters    = 9;
  test_tx_report.bRouteTries   = 0x67;
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp()
{
  send_done_count         = 0;
  send_data_callback_save = NULL;
  stub_status             = SL_STATUS_OK;
  tx_flush_reset_step     = NULL;

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
  process_start(&zwave_tx_process, NULL);
  // Get the clock to start at something else than 0.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
}

void test_zwave_tx_send_data_happy_case()
{
  zwave_controller_transport_send_data_IgnoreAndReturn(SL_STATUS_NOT_SUPPORTED);
  zwave_controller_on_frame_transmission_success_Expect(
    test_connection_1.remote.node_id);
  /// test that we can send a singlecast element
  /// we expect zwapi_send_data to be called.
  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(test_connection_1.remote.node_id,
                                  test_expected_frame_data_1,
                                  sizeof(test_expected_frame_data_1),
                                  expected_tx_options_singlecast_fasttrack,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       (void *)&my_user_pointer,
                                       &test_tx_session_id));

  //Run the contiki events
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Just for to be sure we have a stored callback function, ie our zwapi_send_data_stub was called.
  TEST_ASSERT_NOT_NULL(send_data_callback_save);
  //Fire the callback to emulate a send ok from the zwave api
  send_data_callback_save(TRANSMIT_COMPLETE_OK, &test_tx_report);

  //Run the contiki events
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Verify that all adds up regarding callbacks and transmissions
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL_PTR(&my_user_pointer, received_user_pointer);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
  TEST_ASSERT_EQUAL_MEMORY(&test_tx_report,
                           &send_done_tx_status,
                           sizeof(zwapi_tx_report_t));
}

void test_zwave_tx_send_data_happy_stall_prevention_for_simple_transports()
{
  zwave_controller_transport_send_data_IgnoreAndReturn(SL_STATUS_FAIL);
  zwave_controller_on_frame_transmission_failed_Expect(
    test_connection_1.remote.node_id);

  // The Z-Wave Tx won't call anything else after the transport returns
  // SL_STATUS_FAIL

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       (void *)&my_user_pointer,
                                       &test_tx_session_id));

  //Run the contiki events
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Check that the frame was aborted
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL_PTR(&my_user_pointer, received_user_pointer);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_FAIL, send_done_status);
}

void test_zwave_tx_send_data_emergency_timer()
{
  zwave_controller_transport_send_data_IgnoreAndReturn(SL_STATUS_NOT_SUPPORTED);
  zwave_controller_on_frame_transmission_failed_Expect(
    test_connection_1.remote.node_id);
  /// test that we can send a singlecast element
  /// we expect zwapi_send_data to be called.
  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(test_connection_1.remote.node_id,
                                  test_expected_frame_data_1,
                                  sizeof(test_expected_frame_data_1),
                                  expected_tx_options_singlecast,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_3,
                                       send_data_callback,
                                       (void *)&my_user_pointer,
                                       &test_tx_session_id));

  //Run the contiki events
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Just for to be sure we have a stored callback function, ie our zwapi_send_data_stub was called.
  TEST_ASSERT_NOT_NULL(send_data_callback_save);

  // Now Z-Wave API does not call us back

  //Run the contiki events
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
  TEST_ASSERT_EQUAL(0, send_done_count);

  contiki_test_helper_run(EMERGENCY_TIMEOUT);

  // Verify that all adds up regarding callbacks and transmissions
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL_PTR(&my_user_pointer, received_user_pointer);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_FAIL, send_done_status);
}

void test_zwave_tx_send_data_2_frames_with_backoff()
{
  zwave_controller_transport_send_data_IgnoreAndReturn(SL_STATUS_NOT_SUPPORTED);
  zwave_controller_on_frame_transmission_success_Ignore();

  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(test_connection_2.remote.node_id,
                                  test_expected_frame_data_2,
                                  sizeof(test_expected_frame_data_2),
                                  expected_tx_options_singlecast,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  zwapi_send_data_ExpectAndReturn(test_connection_3.remote.node_id,
                                  test_expected_frame_data_3,
                                  sizeof(test_expected_frame_data_3),
                                  expected_tx_options_singlecast,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  // Frame 3, queued first with lower priority
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_3,
                                       sizeof(test_expected_frame_data_3),
                                       test_expected_frame_data_3,
                                       &test_tx_options_3,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  // Frame 2
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  //Run the contiki events. Transmission is now ongoing.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Just for to be sure we have a stored callback function, ie our zwapi_send_data_stub was called.
  TEST_ASSERT_NOT_NULL(send_data_callback_save);
  send_data_callback_save(TRANSMIT_COMPLETE_OK, &test_tx_report);

  //Run the contiki events. Frame 1 transmission is now completed
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Here the TX is in backoff, it has to wait and should not have triggered any tranmission yet.
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_NOT_NULL(send_data_callback_save);
  send_data_callback_save(TRANSMIT_COMPLETE_OK, &test_tx_report);
  TEST_ASSERT_EQUAL(1, send_done_count);

  //Run the contiki events with >backoff time. Frame 2 transmission is now ongoing
  contiki_test_helper_run(TX_BACKOFF_CONTIKI_CLOCK_JUMP);
  TEST_ASSERT_EQUAL(1, send_done_count);

  // Just for to be sure we have a stored callback function, ie our zwapi_send_data_stub was called.
  TEST_ASSERT_NOT_NULL(send_data_callback_save);
  send_data_callback_save(TRANSMIT_COMPLETE_OK, &test_tx_report);

  // Run the contiki events. Frame 2 transmission is now completed
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Verify that all adds up regarding callbacks and transmissions
  TEST_ASSERT_EQUAL(2, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
  TEST_ASSERT_EQUAL_MEMORY(&test_tx_report,
                           &send_done_tx_status,
                           sizeof(zwapi_tx_report_t));
}

void test_zwave_tx_send_data_answer_during_backoff()
{
  zwave_controller_transport_send_data_IgnoreAndReturn(SL_STATUS_NOT_SUPPORTED);

  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(test_connection_2.remote.node_id,
                                  test_expected_frame_data_2,
                                  sizeof(test_expected_frame_data_2),
                                  expected_tx_options_singlecast,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  zwapi_send_data_ExpectAndReturn(test_connection_3.remote.node_id,
                                  test_expected_frame_data_3,
                                  sizeof(test_expected_frame_data_3),
                                  expected_tx_options_singlecast,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  // Frame 3, queued first with lower priority
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_3,
                                       sizeof(test_expected_frame_data_3),
                                       test_expected_frame_data_3,
                                       &test_tx_options_3,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  // Frame 2
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  //Run the contiki events. Transmission is now ongoing.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Just for to be sure we have a stored callback function, ie our zwapi_send_data_stub was called.
  TEST_ASSERT_NOT_NULL(send_data_callback_save);
  send_data_callback_save(TRANSMIT_COMPLETE_OK, &test_tx_report);

  //Run the contiki events. Frame 1 transmission is now completed
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // The TX queue now initiated a back-off. Simulate an answer coming in, not matching yet.
  zwave_tx_on_frame_received(&test_connection_2, NULL, NULL, 0);

  contiki_test_helper_run(0);

  // Here we should still be in backoff, so no more than 1 sending done.
  TEST_ASSERT_EQUAL(1, send_done_count);

  // Simulate the expected answer coming in during back-off
  zwave_tx_on_frame_received(&test_connection_1, NULL, NULL, 0);

  // Now the next frame is being sent out.
  contiki_test_helper_run(0);

  // Just for to be sure we have a stored callback function, ie our zwapi_send_data_stub was called.
  TEST_ASSERT_NOT_NULL(send_data_callback_save);
  send_data_callback_save(TRANSMIT_COMPLETE_OK, &test_tx_report);

  // Run the contiki events. Frame 2 transmission is now completed
  contiki_test_helper_run(0);

  // Verify that all adds up regarding callbacks and transmissions
  TEST_ASSERT_EQUAL(2, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
  TEST_ASSERT_EQUAL_MEMORY(&test_tx_report,
                           &send_done_tx_status,
                           sizeof(zwapi_tx_report_t));
}

void test_zwave_tx_send_data_fails_immediately()
{
  zwave_controller_transport_send_data_IgnoreAndReturn(SL_STATUS_NOT_SUPPORTED);
  zwave_controller_on_frame_transmission_failed_Expect(
    test_connection_1.remote.node_id);

  zwapi_send_data_AddCallback(NULL);
  zwapi_send_data_ExpectAndReturn(test_connection_1.remote.node_id,
                                  test_expected_frame_data_1,
                                  sizeof(test_expected_frame_data_1),
                                  expected_tx_options_singlecast_fasttrack,
                                  NULL,
                                  SL_STATUS_FAIL);
  zwapi_send_data_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  // Run the contiki events. We expect send_data to fail and get thrown away immediately
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Verify that all adds up regarding callbacks and transmissions
  TEST_ASSERT_EQUAL(1, send_done_count);
}

void test_zwave_tx_frame_too_large()
{
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_tx_send_data(&test_connection_1,
                                       ZWAVE_MAX_FRAME_SIZE + 1,
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  // Run the contiki events. We expect send_data to fail and get thrown away immediately
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Verify that all adds up regarding callbacks and transmissions
  TEST_ASSERT_EQUAL(0, send_done_count);
}

void test_zwave_tx_unknown_parent_id()
{
  zwave_tx_options_t test_invalid_options
    = {.number_of_responses     = 2,
       .discard_timeout_ms      = 0,
       .valid_parent_session_id = true,
       .parent_session_id       = (void *)0xFFF};
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_tx_send_data(&test_connection_1,
                                       ZWAVE_MAX_FRAME_SIZE + 1,
                                       test_expected_frame_data_1,
                                       &test_invalid_options,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  // Run the contiki events. We expect send_data to fail and get thrown away immediately
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Verify that all adds up regarding callbacks and transmissions
  TEST_ASSERT_EQUAL(0, send_done_count);
}

void test_zwave_tx_send_data_fasttrack_requeue()
{
  zwave_controller_transport_send_data_IgnoreAndReturn(SL_STATUS_NOT_SUPPORTED);
  zwave_controller_on_frame_transmission_failed_Expect(
    test_connection_1.remote.node_id);

  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(test_connection_1.remote.node_id,
                                  test_expected_frame_data_1,
                                  sizeof(test_expected_frame_data_1),
                                  expected_tx_options_singlecast_fasttrack,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  zwapi_send_data_ExpectAndReturn(test_connection_1.remote.node_id,
                                  test_expected_frame_data_1,
                                  sizeof(test_expected_frame_data_1),
                                  expected_tx_options_singlecast,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  // Fasttrack transmit fails and gets requeued once.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
  TEST_ASSERT_NOT_NULL(send_data_callback_save);
  send_data_callback_save(TRANSMIT_COMPLETE_FAIL, NULL);
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Fasttrack transmit fails again and gets dropped.
  send_data_callback_save(TRANSMIT_COMPLETE_FAIL, NULL);
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Verify that all adds up regarding callbacks and transmissions
  TEST_ASSERT_EQUAL(
    1,
    send_done_count);  //Only 1 user callback, the first failed attempt must not trigger anything.
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_FAIL, send_done_status);
}

void test_zwave_tx_send_data_discard_timeout_expired()
{
  // Move the clock a little forward, so frames are not queued at time = 0.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
  zwave_controller_transport_send_data_IgnoreAndReturn(SL_STATUS_NOT_SUPPORTED);
  zwave_controller_on_frame_transmission_failed_Ignore();

  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(test_connection_1.remote.node_id,
                                  test_expected_frame_data_1,
                                  sizeof(test_expected_frame_data_1),
                                  expected_tx_options_singlecast_fasttrack,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  zwapi_send_data_ExpectAndReturn(test_connection_3.remote.node_id,
                                  test_expected_frame_data_3,
                                  sizeof(test_expected_frame_data_3),
                                  expected_tx_options_singlecast,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  // Queue up 3 frames (1,2 an 3), number 2 has a discard timeout.
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_3,
                                       sizeof(test_expected_frame_data_3),
                                       test_expected_frame_data_3,
                                       &test_tx_options_3,
                                       send_data_callback,
                                       NULL,
                                       NULL));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       NULL));

  // Frame 1 gets tranmitted:
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
  TEST_ASSERT_NOT_NULL(send_data_callback_save);
  send_data_callback_save(TRANSMIT_COMPLETE_OK, &test_tx_report);
  // Frame 1 transmission takes longer than test_tx_options_2.discard_timeout_ms
  contiki_test_helper_run(test_tx_options_2.discard_timeout_ms + 1);

  // Move on to the next frame, frame too must be discarded and frame 3 transmitted
  send_data_callback_save(TRANSMIT_COMPLETE_OK, &test_tx_report);
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Verify that all adds up regarding callbacks and transmissions
  TEST_ASSERT_EQUAL(3, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
  TEST_ASSERT_EQUAL_MEMORY(&test_tx_report,
                           &send_done_tx_status,
                           sizeof(zwapi_tx_report_t));
  TEST_ASSERT_EQUAL_PTR(NULL, received_user_pointer);
}

void test_zwave_tx_send_data_abort_unsent_frame()
{
  zwave_controller_transport_send_data_IgnoreAndReturn(SL_STATUS_NOT_SUPPORTED);

  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(test_connection_1.remote.node_id,
                                  test_expected_frame_data_1,
                                  sizeof(test_expected_frame_data_1),
                                  expected_tx_options_singlecast_fasttrack,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  zwapi_send_data_ExpectAndReturn(test_connection_2.remote.node_id,
                                  test_expected_frame_data_2,
                                  sizeof(test_expected_frame_data_2),
                                  expected_tx_options_singlecast,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  // Queue up 3 frames (1,2 an 3),
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       NULL));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_3,
                                       sizeof(test_expected_frame_data_3),
                                       test_expected_frame_data_3,
                                       &test_tx_options_3,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       NULL,
                                       NULL));

  // Frame 1 gets tranmitted:
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
  TEST_ASSERT_NOT_NULL(send_data_callback_save);
  send_data_callback_save(TRANSMIT_COMPLETE_OK, &test_tx_report);
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Now we want to abort frame 3, which is still in the queue:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_abort_transmission(test_tx_session_id));

  // Continue sending frames, frame 2
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
  TEST_ASSERT_NOT_NULL(send_data_callback_save);
  send_data_callback_save(TRANSMIT_COMPLETE_OK, &test_tx_report);

  // Frame 3 must not trigger any send_data call.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP
                          + TX_BACKOFF_CONTIKI_CLOCK_JUMP);  // pass the backoff

  // Verify that it looks looks as expected, 2 frames have been sent.
  TEST_ASSERT_EQUAL(2, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
  send_done_tx_status.wTransmitTicks = 0;
  TEST_ASSERT_EQUAL_MEMORY(&test_tx_report,
                           &send_done_tx_status,
                           sizeof(zwapi_tx_report_t));
  TEST_ASSERT_EQUAL_PTR(NULL, received_user_pointer);
}

void test_zwave_tx_send_data_abort_frame_transmission_ongoing()
{
  zwave_controller_transport_send_data_IgnoreAndReturn(SL_STATUS_NOT_SUPPORTED);

  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(test_connection_1.remote.node_id,
                                  test_expected_frame_data_1,
                                  sizeof(test_expected_frame_data_1),
                                  expected_tx_options_singlecast_fasttrack,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  zwapi_send_data_ExpectAndReturn(test_connection_2.remote.node_id,
                                  test_expected_frame_data_2,
                                  sizeof(test_expected_frame_data_2),
                                  expected_tx_options_singlecast,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  //After send_data for frame 2, there should be a send_data_abort.
  zwapi_abort_send_data_ExpectAndReturn(SL_STATUS_OK);

  zwapi_send_data_ExpectAndReturn(test_connection_3.remote.node_id,
                                  test_expected_frame_data_3,
                                  sizeof(test_expected_frame_data_3),
                                  expected_tx_options_singlecast,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  // Queue up 3 frames (1,2 an 3),
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_3,
                                       sizeof(test_expected_frame_data_3),
                                       test_expected_frame_data_3,
                                       &test_tx_options_3,
                                       send_data_callback,
                                       NULL,
                                       NULL));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       NULL,
                                       NULL));

  // Frame 1 gets tranmitted:
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
  TEST_ASSERT_NOT_NULL(send_data_callback_save);
  send_data_callback_save(TRANSMIT_COMPLETE_OK, &test_tx_report);
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Now we want to abort frame 2, which has been passed to the Z-Wave API
  TEST_ASSERT_EQUAL(SL_STATUS_IN_PROGRESS,
                    zwave_tx_abort_transmission(test_tx_session_id));

  // Now simulate the callback from the Z-Wave API, despite the abort call
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
  send_data_callback_save(TRANSMIT_COMPLETE_NO_ACK,
                          NULL);  //let's see when we use NO_ACK.

  // Frame 3 will be sent without using the frame 2 backoff.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
  send_data_callback_save(TRANSMIT_COMPLETE_OK, &test_tx_report);
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Verify that it looks looks as expected, 3 frame callbacks should be returned.
  TEST_ASSERT_EQUAL(3, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
  TEST_ASSERT_EQUAL_MEMORY(&test_tx_report,
                           &send_done_tx_status,
                           sizeof(zwapi_tx_report_t));
  TEST_ASSERT_EQUAL_PTR(NULL, received_user_pointer);
}

void test_zwave_tx_send_data_multicast_frame_happy_case()
{
  static const zwave_controller_connection_info_t test_connection_multicast = {
    .local  = {.node_id = 5, .endpoint_id = 0, .is_multicast = false},
    .remote = {.multicast_group = 0x34, .endpoint_id = 0, .is_multicast = true},
    .encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
  };

  zwave_controller_transport_send_data_IgnoreAndReturn(SL_STATUS_NOT_SUPPORTED);

  zwave_nodemask_t expected_node_list = {};
  memset(expected_node_list, 0, sizeof(zwave_nodemask_t));
  zwave_node_id_t test_node_1 = 3;
  zwave_node_id_t test_node_2 = 45;
  ZW_ADD_NODE_TO_MASK(test_node_1, expected_node_list);
  ZW_ADD_NODE_TO_MASK(test_node_2, expected_node_list);

  zwave_tx_get_nodes_ExpectAndReturn(
    NULL,
    test_connection_multicast.remote.multicast_group,
    SL_STATUS_OK);
  zwave_tx_get_nodes_IgnoreArg_nodes();
  zwave_tx_get_nodes_ReturnMemThruPtr_nodes(expected_node_list,
                                            sizeof(zwave_nodemask_t));

  zwapi_send_data_multi_AddCallback(zwapi_send_data_multi_stub);
  zwapi_send_data_multi_ExpectAndReturn(expected_node_list,
                                        test_expected_frame_data_1,
                                        sizeof(test_expected_frame_data_1),
                                        expected_tx_options_multicast,
                                        NULL,
                                        SL_STATUS_OK);
  zwapi_send_data_multi_IgnoreArg_callback_function();

  // Queue up our multicast frame
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_multicast,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_with_follow_up,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  // Get the tranmission ongoing:
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
  TEST_ASSERT_NOT_NULL(send_data_multi_callback_save);
  send_data_multi_callback_save(TRANSMIT_COMPLETE_OK);

  // Now it will prepare follow-up frames
  zwave_tx_get_nodes_ExpectAndReturn(
    NULL,
    test_connection_multicast.remote.multicast_group,
    SL_STATUS_OK);
  zwave_tx_get_nodes_IgnoreArg_nodes();
  zwave_tx_get_nodes_IgnoreArg_nodes();
  zwave_tx_get_nodes_ReturnMemThruPtr_nodes(expected_node_list,
                                            sizeof(zwave_nodemask_t));

  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(test_node_1,
                                  test_expected_frame_data_1,
                                  sizeof(test_expected_frame_data_1),
                                  expected_tx_options_singlecast_fasttrack,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
  TEST_ASSERT_NOT_NULL(send_data_callback_save);
  send_data_callback_save(TRANSMIT_COMPLETE_OK, &test_tx_report);

  zwapi_send_data_ExpectAndReturn(test_node_2,
                                  test_expected_frame_data_1,
                                  sizeof(test_expected_frame_data_1),
                                  expected_tx_options_singlecast_fasttrack,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
  TEST_ASSERT_NOT_NULL(send_data_callback_save);
  send_data_callback_save(TRANSMIT_COMPLETE_OK, &test_tx_report);

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Verify that it looks looks as expected
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
  TEST_ASSERT_EQUAL_PTR(NULL, received_user_pointer);
}

void test_zwave_tx_transport_encapsulated_frame()
{
  static const zwave_controller_connection_info_t test_connection_encrypted = {
    .local         = {.node_id = 1, .endpoint_id = 0, .is_multicast = false},
    .remote        = {.node_id = 1, .endpoint_id = 2, .is_multicast = false},
    .encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS,
  };

  stub_status = SL_STATUS_OK;
  zwave_controller_transport_send_data_AddCallback(
    (CMOCK_zwave_controller_transport_send_data_CALLBACK)
      zwave_transport_send_data_stub);
  zwave_controller_transport_send_data_ExpectAndReturn(
    &test_connection_encrypted,
    sizeof(test_expected_frame_data_1),
    test_expected_frame_data_1,
    &test_tx_options_2,
    NULL,
    NULL,
    NULL,
    SL_STATUS_OK);

  zwave_controller_transport_send_data_IgnoreArg_session();
  zwave_controller_transport_send_data_IgnoreArg_on_send_complete();
  zwave_controller_transport_send_data_IgnoreArg_user();

  // Queue up our frame that should be encrypted
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_encrypted,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  // Process the first frame. (only 1 event) It should trigger transport.
  contiki_test_helper_run_once(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Before zwave_transport makes a callback, it needs to queue up a parent frame, unencrypted.
  // Queue up our frame that should be encrypted
  test_tx_options.number_of_responses      = 0;
  test_tx_options.parent_session_id        = test_tx_session_id;
  test_tx_options.valid_parent_session_id  = true;
  test_tx_options.discard_timeout_ms       = 0;
  test_tx_options.qos_priority             = 1;
  test_tx_options.use_parent_frame_options = true;

  stub_status = SL_STATUS_NOT_SUPPORTED;
  zwave_controller_transport_send_data_IgnoreAndReturn(SL_STATUS_NOT_SUPPORTED);

  // Child frame is sent using ZWAPI send_data
  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(test_connection_2.remote.node_id,
                                  test_expected_frame_data_2,
                                  sizeof(test_expected_frame_data_2),
                                  expected_tx_options_singlecast,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  // Here it is transport that normally calls send_data, it's invisible to the user
  // so no callback
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_2,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options,
                                       send_data_callback,
                                       NULL,
                                       NULL));

  //TEST_ASSERT_EQUAL(1, send_done_count);
  zwave_tx_log_queue(true);
  // Here the parent frame for transport will trigger a callback
  contiki_test_helper_run_clock_increases_for_each_event(
    DEFAULT_CONTIKI_CLOCK_JUMP);

  // Just for to be sure we have a stored callback function, ie our zwapi_send_data_stub was called.
  TEST_ASSERT_NOT_NULL(send_data_callback_save);
  //Fire the callback to emulate a send ok from the zwave api
  send_data_callback_save(TRANSMIT_COMPLETE_OK, &test_tx_report);

  // Now the child frame is fully done.
  contiki_test_helper_run_clock_increases_for_each_event(
    DEFAULT_CONTIKI_CLOCK_JUMP);

  // 1 frame has been sent (via ZWAPI), the parent frame is still pending
  TEST_ASSERT_EQUAL(1, send_done_count);

  // Pass the back-off for this frame
  contiki_test_helper_run(TX_BACKOFF_CONTIKI_CLOCK_JUMP);

  // Now parent frame is still in the queue and needs to be removed
  // only after the transports gives us the callback.

  // If we force the process to look at the queue, the TX queue should
  // be stalled until the callback
  process_post(&zwave_tx_process, ZWAVE_TX_SEND_NEXT_MESSAGE, 0);
  contiki_test_helper_run_clock_increases_for_each_event(
    DEFAULT_CONTIKI_CLOCK_JUMP);

  // Z-Wave transport is done and makes its callback:
  TEST_ASSERT_NOT_NULL(zwave_transport_send_data_save);
  zwave_transport_send_data_save(TRANSMIT_COMPLETE_OK,
                                 &test_tx_report,
                                 test_tx_session_id);

  // Run contiki again to process the frame callback
  contiki_test_helper_run(TX_BACKOFF_CONTIKI_CLOCK_JUMP);

  // Verify that it looks looks as expected
  TEST_ASSERT_EQUAL(2, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
  // We expect that the transmission time is copied from TX
  // Contiki made a clock jump of DEFAULT_CONTIKI_CLOCK_JUMP for each event
  send_done_tx_status.wTransmitTicks = 0;
  TEST_ASSERT_EQUAL_MEMORY(&test_tx_report,
                           &send_done_tx_status,
                           sizeof(zwapi_tx_report_t));
  TEST_ASSERT_EQUAL_PTR(NULL, received_user_pointer);

  // Bonus test. Try to call callbacks again, for non-existing frames
  TEST_ASSERT_NOT_NULL(send_data_callback_save);
  send_data_callback_save(TRANSMIT_COMPLETE_OK, &test_tx_report);

  // Nothing should have changed
  TEST_ASSERT_EQUAL(2, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
  TEST_ASSERT_EQUAL_MEMORY(&test_tx_report,
                           &send_done_tx_status,
                           sizeof(zwapi_tx_report_t));
  TEST_ASSERT_EQUAL_PTR(NULL, received_user_pointer);

  // Same thing with the transport callback:
  TEST_ASSERT_NOT_NULL(zwave_transport_send_data_save);
  zwave_transport_send_data_save(TRANSMIT_COMPLETE_OK,
                                 &test_tx_report,
                                 test_tx_session_id);

  // No change expected
  TEST_ASSERT_EQUAL(2, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
  TEST_ASSERT_EQUAL_MEMORY(&test_tx_report,
                           &send_done_tx_status,
                           sizeof(zwapi_tx_report_t));
  TEST_ASSERT_EQUAL_PTR(NULL, received_user_pointer);
}

void test_zwave_tx_try_to_send_empty_queue()
{
  process_post(&zwave_tx_process, ZWAVE_TX_SEND_NEXT_MESSAGE, 0);
  contiki_test_helper_run_once(DEFAULT_CONTIKI_CLOCK_JUMP);
  process_post(&zwave_tx_process, ZWAVE_TX_SEND_NEXT_MESSAGE, 0);
  contiki_test_helper_run_once(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Nothing special should happen here.
}

void test_zwave_tx_shutdown()
{
  zwapi_abort_send_data_ExpectAndReturn(SL_STATUS_OK);

  // Nothing special will happen here.
  zwave_tx_shutdown();

  // Trigger another shutdown with the process:
  zwapi_abort_send_data_ExpectAndReturn(SL_STATUS_FAIL);
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

void test_zwave_tx_intercept_node_info()
{
  zwapi_request_node_info_ExpectAndReturn(test_connection_1.remote.node_id,
                                          SL_STATUS_OK);
  zwave_controller_on_node_information_Ignore();
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_frame_data_request_nif),
                                       test_frame_data_request_nif,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       (void *)&my_user_pointer,
                                       0));
  //Run the contiki events
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
  zwave_controller_callbacks->on_node_information(
    test_connection_1.remote.node_id,
    NULL);
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Verify that all adds up regarding callbacks and transmissions
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL_PTR(&my_user_pointer, received_user_pointer);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
}

void test_zwave_tx_intercept_protocol_cc_too_short()
{
  zwave_controller_transport_send_data_IgnoreAndReturn(SL_STATUS_NOT_SUPPORTED);
  /// test that we can send a singlecast element
  /// we expect zwapi_send_data to be called.
  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(test_connection_1.remote.node_id,
                                  test_frame_data_request_nif,
                                  1,
                                  expected_tx_options_singlecast_fasttrack,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       1,
                                       test_frame_data_request_nif,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       (void *)&my_user_pointer,
                                       &test_tx_session_id));

  //Run the contiki events
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Just for to be sure we have a stored callback function, ie our zwapi_send_data_stub was called.
  TEST_ASSERT_NOT_NULL(send_data_callback_save);
  //Fire the callback to emulate a send ok from the zwave api
  send_data_callback_save(TRANSMIT_COMPLETE_OK, &test_tx_report);

  //Run the contiki events
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Verify that all adds up regarding callbacks and transmissions
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL_PTR(&my_user_pointer, received_user_pointer);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
  TEST_ASSERT_EQUAL_MEMORY(&test_tx_report,
                           &send_done_tx_status,
                           sizeof(zwapi_tx_report_t));
}

void test_zwave_tx_intercept_node_info_failed()
{
  zwapi_request_node_info_ExpectAndReturn(test_connection_1.remote.node_id,
                                          SL_STATUS_OK);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_frame_data_request_nif),
                                       test_frame_data_request_nif,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       (void *)&my_user_pointer,
                                       0));
  //Run the contiki events
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
  zwave_controller_callbacks->on_node_info_req_failed(
    test_connection_1.remote.node_id);
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Verify that all adds up regarding callbacks and transmissions
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL_PTR(&my_user_pointer, received_user_pointer);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_FAIL, send_done_status);
}

void test_zwave_tx_intercept_node_info_timeout()
{
  // Move the clock a little forward, so frames are not queued at time = 0.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
  zwapi_request_node_info_ExpectAndReturn(test_connection_1.remote.node_id,
                                          SL_STATUS_OK);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_frame_data_request_nif),
                                       test_frame_data_request_nif,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       (void *)&my_user_pointer,
                                       0));
//Run the contiki events
#ifndef SEND_DATA_EMERGENCY_TIMEOUT
#define SEND_DATA_EMERGENCY_TIMEOUT 65
#endif
  // First contiki "tick" will cause the enqueued tx frame to be "sent"
  // Second contiki "tick" will trigger the timeout.
  // if the first contiki "tick" is omitted, the timeout will occur before the
  // internal state in zwave_tx has changed to
  // ZWAVE_TX_STATE_TRANSMISSION_ONGOING, and the send_data_callback will
  // never be called. In real world scenarios this is always guaranteed to
  // happen as the timeout is 65 seconds, and the Contiki tick time is 1000 ms
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
  contiki_test_helper_run((SEND_DATA_EMERGENCY_TIMEOUT - 1) * CLOCK_SECOND);
  // after (SEND_DATA_EMERGENCY_TIMEOUT - 1)  seconds we expect the timeout not
  // to have ocurred yet.
  TEST_ASSERT_EQUAL(0, send_done_count);
  // But after one more second it should :)
  contiki_test_helper_run((1) * CLOCK_SECOND);
  // Verify that all adds up regarding callbacks and transmissions
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL_PTR(&my_user_pointer, received_user_pointer);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_FAIL, send_done_status);
}
/*
void test_zwave_tx_intercept_nop()
{
  zwave_network_management_get_node_id_IgnoreAndReturn(our_node_id);
  // Move the clock a little forward, so frames are not queued at time = 0.
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  zwapi_send_nop_AddCallback(zwapi_send_nop_stub);
  zwapi_send_nop_ExpectAndReturn(
    test_connection_1.remote.node_id,
    TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_AUTO_ROUTE | TRANSMIT_OPTION_EXPLORE,
    NULL,
    SL_STATUS_OK);
  zwapi_send_nop_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_frame_data_nop),
                                       test_frame_data_nop,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       (void *)&my_user_pointer,
                                       0));

  //Run the contiki events
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Just for to be sure we have a stored callback function, ie our zwapi_send_nop_stub was called.
  TEST_ASSERT_NOT_NULL(send_nop_callback_save);
  //Fire the callback to emulate a send ok from the zwave api
  send_nop_callback_save(TRANSMIT_COMPLETE_OK, &test_tx_report);

  //Run the contiki events
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Verify that all adds up regarding callbacks and transmissions
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(&my_user_pointer, received_user_pointer);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
  TEST_ASSERT_EQUAL_MEMORY(&test_tx_report,
                           &send_done_tx_status,
                           sizeof(zwapi_tx_report_t));
}
*/
void test_zwave_tx_send_data_test_frame_happy_case()
{
  zwave_controller_transport_send_data_IgnoreAndReturn(SL_STATUS_NOT_SUPPORTED);
  zwave_network_management_get_node_id_IgnoreAndReturn(our_node_id);
  /// test that we can send a singlecast element
  /// we expect zwapi_send_data to be called.
  zwapi_send_test_frame_AddCallback(zwapi_send_test_frame_stub);
  zwapi_send_test_frame_ExpectAndReturn(test_connection_1.remote.node_id,
                                        test_tx_options_test_frame.rf_power,
                                        NULL,
                                        SL_STATUS_OK);
  zwapi_send_test_frame_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_tx_send_test_frame(test_connection_1.remote.node_id,
                             test_tx_options_test_frame.rf_power,
                             send_data_callback,
                             (void *)&my_user_pointer,
                             &test_tx_session_id));

  //Run the contiki events
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Just for to be sure we have a stored callback function, ie our zwapi_send_data_stub was called.
  TEST_ASSERT_NOT_NULL(zwapi_send_test_frame_callback_save);
  //Fire the callback to emulate a send ok from the zwave api
  zwapi_send_test_frame_callback_save(TRANSMIT_COMPLETE_OK);

  //Run the contiki events
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Verify that all adds up regarding callbacks and transmissions
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL_PTR(&my_user_pointer, received_user_pointer);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
}

void test_zwave_tx_reject_sending_to_ourselves()
{
  zwave_controller_transport_send_data_IgnoreAndReturn(SL_STATUS_NOT_SUPPORTED);
  zwave_controller_on_frame_transmission_success_Ignore();

  zwave_controller_connection_info_t test_connection_to_ourselves = {};
  test_connection_to_ourselves.remote.is_multicast                = false;
  test_connection_to_ourselves.remote.node_id                     = our_node_id;

  // Queue up a message to ourselves
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_tx_send_data(&test_connection_to_ourselves,
                                       sizeof(test_expected_frame_data_2),
                                       test_expected_frame_data_2,
                                       &test_tx_options_2,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  // Verify that all adds up regarding callbacks and transmissions
  TEST_ASSERT_EQUAL(0, send_done_count);
}

void test_zwave_tx_send_data_multicast_as_broadcast()
{
  static const zwave_controller_connection_info_t test_connection_multicast = {
    .local  = {.node_id = 5, .endpoint_id = 0, .is_multicast = false},
    .remote = {.multicast_group = 0x34, .endpoint_id = 0, .is_multicast = true},
    .encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
  };

  zwave_controller_transport_send_data_IgnoreAndReturn(SL_STATUS_NOT_SUPPORTED);

  zwave_nodemask_t expected_node_list = {};
  memset(expected_node_list, 0, sizeof(zwave_nodemask_t));
  zwave_node_id_t test_node_1 = 3;
  zwave_node_id_t test_node_2 = 45;
  ZW_ADD_NODE_TO_MASK(test_node_1, expected_node_list);
  ZW_ADD_NODE_TO_MASK(test_node_2, expected_node_list);

  zwave_tx_get_nodes_ExpectAndReturn(
    NULL,
    test_connection_multicast.remote.multicast_group,
    SL_STATUS_OK);
  zwave_tx_get_nodes_IgnoreArg_nodes();
  zwave_tx_get_nodes_ReturnMemThruPtr_nodes(expected_node_list,
                                            sizeof(zwave_nodemask_t));

  zwapi_send_data_multi_AddCallback(zwapi_send_data_multi_stub);
  zwapi_send_data_multi_ExpectAndReturn(
    expected_node_list,
    test_expected_frame_data_1,
    sizeof(test_expected_frame_data_1),
    expected_tx_options_multicast_as_broadcast,
    NULL,
    SL_STATUS_OK);
  zwapi_send_data_multi_IgnoreArg_callback_function();

  // Queue up our multicast frame
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_multicast,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_group_id,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  // Get the tranmission ongoing:
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
  TEST_ASSERT_NOT_NULL(send_data_multi_callback_save);
  send_data_multi_callback_save(TRANSMIT_COMPLETE_OK);
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Verify that it looks looks as expected
  TEST_ASSERT_EQUAL(1, send_done_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, send_done_status);
  TEST_ASSERT_EQUAL_PTR(NULL, received_user_pointer);
}

void test_zwave_tx_abort_send_data_multicast_with_a_bunch_of_child_sessions()
{
  static const zwave_controller_connection_info_t test_connection_multicast = {
    .local  = {.node_id = 5, .endpoint_id = 0, .is_multicast = false},
    .remote = {.multicast_group = 0x34, .endpoint_id = 0, .is_multicast = true},
    .encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
  };

  zwave_controller_transport_send_data_IgnoreAndReturn(SL_STATUS_NOT_SUPPORTED);

  zwave_nodemask_t expected_node_list = {};
  memset(expected_node_list, 0, sizeof(zwave_nodemask_t));
  ZW_ADD_NODE_TO_MASK(3, expected_node_list);
  ZW_ADD_NODE_TO_MASK(4, expected_node_list);
  ZW_ADD_NODE_TO_MASK(5, expected_node_list);
  ZW_ADD_NODE_TO_MASK(6, expected_node_list);
  ZW_ADD_NODE_TO_MASK(7, expected_node_list);

  zwave_tx_get_nodes_ExpectAndReturn(
    NULL,
    test_connection_multicast.remote.multicast_group,
    SL_STATUS_OK);
  zwave_tx_get_nodes_IgnoreArg_nodes();
  zwave_tx_get_nodes_ReturnMemThruPtr_nodes(expected_node_list,
                                            sizeof(zwave_nodemask_t));

  zwapi_send_data_multi_AddCallback(zwapi_send_data_multi_stub);
  zwapi_send_data_multi_ExpectAndReturn(expected_node_list,
                                        test_expected_frame_data_1,
                                        sizeof(test_expected_frame_data_1),
                                        expected_tx_options_multicast,
                                        NULL,
                                        SL_STATUS_OK);
  zwapi_send_data_multi_IgnoreArg_callback_function();

  // Queue up our multicast frame
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_multicast,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_with_follow_up,
                                       send_data_callback,
                                       NULL,
                                       &test_tx_session_id));

  // Get the tranmission ongoing:
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);
  TEST_ASSERT_NOT_NULL(send_data_multi_callback_save);
  send_data_multi_callback_save(TRANSMIT_COMPLETE_OK);

  // Now it will prepare follow-up frames
  zwave_tx_get_nodes_ExpectAndReturn(
    NULL,
    test_connection_multicast.remote.multicast_group,
    SL_STATUS_OK);
  zwave_tx_get_nodes_IgnoreArg_nodes();
  zwave_tx_get_nodes_IgnoreArg_nodes();
  zwave_tx_get_nodes_ReturnMemThruPtr_nodes(expected_node_list,
                                            sizeof(zwave_nodemask_t));

  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(3,
                                  test_expected_frame_data_1,
                                  sizeof(test_expected_frame_data_1),
                                  expected_tx_options_singlecast_fasttrack,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Now we are pending node 3 follow up transmission callback..
  // try to abort the multicast
  zwapi_abort_send_data_ExpectAndReturn(SL_STATUS_OK);
  TEST_ASSERT_EQUAL(SL_STATUS_IN_PROGRESS,
                    zwave_tx_abort_transmission(test_tx_session_id));
  TEST_ASSERT_NOT_NULL(send_data_callback_save);
  send_data_callback_save(TRANSMIT_COMPLETE_OK, &test_tx_report);

  // No more calls to send data should be made now
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Verify that it looks looks as expected
  TEST_ASSERT_EQUAL(0, send_done_count);
}

void test_zwave_tx_reset_step()
{
  // Queue up a frame.
  zwave_controller_transport_send_data_IgnoreAndReturn(SL_STATUS_NOT_SUPPORTED);
  zwave_controller_on_frame_transmission_success_Ignore();
  /// we expect zwapi_send_data to be called.
  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(test_connection_1.remote.node_id,
                                  test_expected_frame_data_1,
                                  sizeof(test_expected_frame_data_1),
                                  expected_tx_options_singlecast_fasttrack,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       (void *)&my_user_pointer,
                                       &test_tx_session_id));

  // If we reset here, we should just get the queue empty and return immediately
  TEST_ASSERT_NOT_NULL(tx_flush_reset_step);
  TEST_ASSERT_EQUAL(SL_STATUS_EMPTY, tx_flush_reset_step());

  // Now queue a frame and initiate transmission.
  zwapi_send_data_ExpectAndReturn(test_connection_1.remote.node_id,
                                  test_expected_frame_data_1,
                                  sizeof(test_expected_frame_data_1),
                                  expected_tx_options_singlecast_fasttrack,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_send_data(&test_connection_1,
                                       sizeof(test_expected_frame_data_1),
                                       test_expected_frame_data_1,
                                       &test_tx_options_1,
                                       send_data_callback,
                                       (void *)&my_user_pointer,
                                       &test_tx_session_id));

  //Run the contiki events
  contiki_test_helper_run(DEFAULT_CONTIKI_CLOCK_JUMP);

  // Now the transmission is ongoing, try to reset:
  zwapi_abort_send_data_ExpectAndReturn(SL_STATUS_OK);
  TEST_ASSERT_NOT_NULL(tx_flush_reset_step);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, tx_flush_reset_step());

  // when the Z-Wave API callback comes around, the queue gets flushed the
  // controller gets notified
  TEST_ASSERT_NOT_NULL(send_data_callback_save);
  //Fire the callback to emulate a send ok from the zwave api
  send_data_callback_save(TRANSMIT_COMPLETE_OK, &test_tx_report);

  zwave_controller_on_reset_step_complete_Expect(4);

  //Run the contiki events
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
