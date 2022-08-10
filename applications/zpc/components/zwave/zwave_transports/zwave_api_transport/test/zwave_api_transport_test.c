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
#include "zwave_api_transport.h"

// Unity test
#include "unity.h"

// Generic includes
#include <string.h>

// Test helpers
#include "contiki_test_helper.h"
#include "process.h"

// Mock includes
#include "zwave_controller_callbacks_mock.h"
#include "zwave_controller_transport_mock.h"
#include "zwave_controller_internal_mock.h"
#include "zwapi_protocol_transport_mock.h"
#include "zwapi_protocol_basis_mock.h"
#include "zwapi_protocol_controller_mock.h"
#include "zwave_tx_groups_mock.h"

// Static test variables
static zwave_controller_callbacks_t zwave_controller_callbacks;
static zwave_controller_transport_t zwave_api_transport;

static cmock_zwapi_protocol_transport_func_ptr1 zwave_api_send_data_callback;
static cmock_zwapi_protocol_transport_func_ptr3
  zwave_api_send_data_multi_callback;
static cmock_zwapi_protocol_basis_func_ptr2 zwave_api_send_test_frame_callback;

static zwave_controller_connection_info_t info;
static zwave_tx_options_t tx_options;
static zwave_tx_session_id_t parent_session_id;
static void *user;

static uint8_t received_status;
static zwapi_tx_report_t received_tx_report;
static void *received_user;

// Sub functions
static sl_status_t zwave_controller_register_callbacks_stub(
  const zwave_controller_callbacks_t *callbacks, int cmock_num_calls)
{
  TEST_ASSERT_NOT_NULL(callbacks);
  zwave_controller_callbacks = *callbacks;
  return SL_STATUS_OK;
}

static sl_status_t zwave_controller_transport_register_stub(
  const zwave_controller_transport_t *transport, int cmock_num_calls)
{
  TEST_ASSERT_NOT_NULL(transport);
  zwave_api_transport = *transport;

  TEST_ASSERT_NOT_NULL(zwave_api_transport.abort_send_data);
  TEST_ASSERT_NOT_NULL(zwave_api_transport.send_data);
  TEST_ASSERT_NULL(zwave_api_transport.on_frame_received);
  TEST_ASSERT_EQUAL(0x00, zwave_api_transport.command_class);
  TEST_ASSERT_EQUAL(0, zwave_api_transport.version);
  TEST_ASSERT_EQUAL(0, zwave_api_transport.priority);
  return SL_STATUS_OK;
}

static sl_status_t zwapi_send_data_stub(
  zwave_node_id_t destination_node_id,
  const uint8_t *data,
  uint8_t data_length,
  uint8_t tx_options,
  cmock_zwapi_protocol_transport_func_ptr1 callback_function,
  int cmock_num_calls)
{
  zwave_api_send_data_callback = callback_function;
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
  zwave_api_send_data_multi_callback = callback_function;
  return SL_STATUS_OK;
}

static sl_status_t zwapi_send_test_frame_stub(
  zwave_node_id_t destination_node_id,
  rf_power_level_t power_level,
  cmock_zwapi_protocol_basis_func_ptr2 callback_function,
  int cmock_num_calls)
{
  zwave_api_send_test_frame_callback = callback_function;
  return SL_STATUS_OK;
}

// test callbacks
static void on_send_data_complete(uint8_t status,
                                  const zwapi_tx_report_t *tx_report,
                                  void *user)
{
  received_status = status;
  if (tx_report != NULL) {
    received_tx_report = *tx_report;
  }
  received_user = user;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  contiki_test_helper_init();

  memset(&zwave_controller_callbacks, 0, sizeof(zwave_controller_callbacks));
  memset(&zwave_api_transport, 0, sizeof(zwave_api_transport));
  memset(&info, 0, sizeof(info));
  memset(&tx_options, 0, sizeof(tx_options));
  user                               = NULL;
  parent_session_id                  = NULL;
  zwave_api_send_data_callback       = NULL;
  zwave_api_send_data_multi_callback = NULL;
  zwave_api_send_test_frame_callback = NULL;

  received_status = 0xFF;
  memset(&received_tx_report, 0, sizeof(received_tx_report));
  received_user = NULL;

  zwave_controller_register_callbacks_Stub(
    &zwave_controller_register_callbacks_stub);

  zwave_controller_transport_register_Stub(
    zwave_controller_transport_register_stub);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_api_transport_init());

  // Make sure the process is initialized.
  contiki_test_helper_run(1);
}

void test_zwave_api_transport_send_data_happy_case()
{
  TEST_ASSERT_NOT_NULL(zwave_api_transport.send_data);

  // Prepare data:
  info.remote.node_id   = 23;
  tx_options.fasttrack  = false;
  const uint8_t frame[] = {0x56, 0x32, 0x9F, 0x22, 0x01, 0xFF};
  user                  = (void *)34;
  parent_session_id     = (void *)53;

  uint8_t expected_zwapi_tx_options = TRANSMIT_OPTION_ACK
                                      | TRANSMIT_OPTION_AUTO_ROUTE
                                      | TRANSMIT_OPTION_EXPLORE;

  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(info.remote.node_id,
                                  frame,
                                  sizeof(frame),
                                  expected_zwapi_tx_options,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_api_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  //Run the contiki events
  contiki_test_helper_run(0);

  //Additional test: Node info event are to be ignored when sending data.
  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks.on_node_information);
  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks.on_node_info_req_failed);
  zwave_controller_callbacks.on_node_information(info.remote.node_id, NULL);
  zwave_controller_callbacks.on_node_info_req_failed(info.remote.node_id);

  contiki_test_helper_run(0);

  // Expect to notify of a frame transmission success
  zwapi_tx_report_t report = {};
  zwave_controller_on_frame_transmission_Expect(true,
                                                &report,
                                                info.remote.node_id);

  // callback from the Z-Wave API
  TEST_ASSERT_NOT_NULL(zwave_api_send_data_callback);
  zwave_api_send_data_callback(TRANSMIT_COMPLETE_OK, &report);

  TEST_ASSERT_EQUAL_PTR(user, received_user);
  TEST_ASSERT_EQUAL_MEMORY(&report,
                           &received_tx_report,
                           sizeof(zwapi_tx_report_t));
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, received_status);
}

void test_zwave_api_transport_send_data_null_callback()
{
  TEST_ASSERT_NOT_NULL(zwave_api_transport.send_data);

  // Prepare data:
  info.remote.node_id   = 23;
  tx_options.fasttrack  = false;
  const uint8_t frame[] = {0x56, 0x32, 0x9F, 0x22, 0x01, 0xFF};
  user                  = (void *)6565;
  parent_session_id     = (void *)53;

  uint8_t expected_zwapi_tx_options = TRANSMIT_OPTION_ACK
                                      | TRANSMIT_OPTION_AUTO_ROUTE
                                      | TRANSMIT_OPTION_EXPLORE;

  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(info.remote.node_id,
                                  frame,
                                  sizeof(frame),
                                  expected_zwapi_tx_options,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_api_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  NULL,
                                                  user,
                                                  parent_session_id));

  //Run the contiki events
  contiki_test_helper_run(0);

  //Additional test: Node info event are to be ignored when sending data.
  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks.on_node_information);
  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks.on_node_info_req_failed);
  zwave_controller_callbacks.on_node_information(info.remote.node_id, NULL);
  zwave_controller_callbacks.on_node_info_req_failed(info.remote.node_id);

  contiki_test_helper_run(0);

  // Expect to notify of a frame transmission success
  zwave_controller_on_frame_transmission_Expect(true,
                                                NULL,
                                                info.remote.node_id);

  // callback from the Z-Wave API
  TEST_ASSERT_NOT_NULL(zwave_api_send_data_callback);
  zwave_api_send_data_callback(TRANSMIT_COMPLETE_OK, NULL);

  // callback again, it shoudl be ignored.
  zwave_api_send_data_callback(TRANSMIT_COMPLETE_OK, NULL);
}

void test_zwave_api_transport_send_data_frame_rejected()
{
  TEST_ASSERT_NOT_NULL(zwave_api_transport.send_data);

  // Prepare data:
  info.remote.node_id   = 23;
  tx_options.fasttrack  = false;
  const uint8_t frame[] = {0x56, 0x32, 0x9F, 0x22, 0x01, 0xFF};
  user                  = (void *)34;
  parent_session_id     = (void *)53;

  uint8_t expected_zwapi_tx_options = TRANSMIT_OPTION_ACK
                                      | TRANSMIT_OPTION_AUTO_ROUTE
                                      | TRANSMIT_OPTION_EXPLORE;

  zwapi_send_data_AddCallback(NULL);
  zwapi_send_data_ExpectAndReturn(info.remote.node_id,
                                  frame,
                                  sizeof(frame),
                                  expected_zwapi_tx_options,
                                  NULL,
                                  SL_STATUS_FAIL);
  zwapi_send_data_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_api_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  //Run the contiki events
  contiki_test_helper_run(0);
}

void test_zwave_api_transport_send_data_transmit_fail()
{
  TEST_ASSERT_NOT_NULL(zwave_api_transport.send_data);

  // Prepare data:
  info.remote.node_id   = 18;
  tx_options.fasttrack  = false;
  const uint8_t frame[] = {0x56, 0x32, 0x9F, 0x22, 0x01, 0xFF};
  user                  = (void *)34;
  parent_session_id     = (void *)53;

  uint8_t expected_zwapi_tx_options = TRANSMIT_OPTION_ACK
                                      | TRANSMIT_OPTION_AUTO_ROUTE
                                      | TRANSMIT_OPTION_EXPLORE;

  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(info.remote.node_id,
                                  frame,
                                  sizeof(frame),
                                  expected_zwapi_tx_options,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_api_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  //Run the contiki events
  contiki_test_helper_run(0);

  // Notify of a transmit failure.
  zwave_controller_on_frame_transmission_Expect(false,
                                                NULL,
                                                info.remote.node_id);

  // callback from the Z-Wave API
  TEST_ASSERT_NOT_NULL(zwave_api_send_data_callback);
  zwave_api_send_data_callback(TRANSMIT_COMPLETE_FAIL, NULL);

  TEST_ASSERT_EQUAL_PTR(user, received_user);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_FAIL, received_status);
}

void test_zwave_api_transport_send_data_transmit_fail_fast_track()
{
  TEST_ASSERT_NOT_NULL(zwave_api_transport.send_data);

  // Prepare data:
  info.remote.node_id   = 23;
  tx_options.fasttrack  = true;
  const uint8_t frame[] = {0x56, 0x32, 0x9F, 0x22, 0x01, 0xFF};
  user                  = (void *)34;
  parent_session_id     = (void *)53;

  uint8_t expected_zwapi_tx_options = TRANSMIT_OPTION_ACK;

  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(info.remote.node_id,
                                  frame,
                                  sizeof(frame),
                                  expected_zwapi_tx_options,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_api_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  //Run the contiki events
  contiki_test_helper_run(0);

  // No fail notification for fast track attempts

  // callback from the Z-Wave API
  TEST_ASSERT_NOT_NULL(zwave_api_send_data_callback);
  zwave_api_send_data_callback(TRANSMIT_COMPLETE_FAIL, NULL);

  TEST_ASSERT_EQUAL_PTR(user, received_user);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_FAIL, received_status);
}

void test_zwave_api_transport_send_data_transmit_timeout()
{
  TEST_ASSERT_NOT_NULL(zwave_api_transport.send_data);

  // Prepare data:
  info.remote.node_id   = 23;
  tx_options.fasttrack  = false;
  const uint8_t frame[] = {0x56, 0x32, 0x9F, 0x22, 0x01, 0xFF};
  user                  = (void *)34;
  parent_session_id     = (void *)53;

  uint8_t expected_zwapi_tx_options = TRANSMIT_OPTION_ACK
                                      | TRANSMIT_OPTION_AUTO_ROUTE
                                      | TRANSMIT_OPTION_EXPLORE;

  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(info.remote.node_id,
                                  frame,
                                  sizeof(frame),
                                  expected_zwapi_tx_options,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_api_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  //Run the contiki events
  contiki_test_helper_run(0);
  contiki_test_helper_run(SEND_DATA_EMERGENCY_TIMEOUT * 1000 - 1);
  contiki_test_helper_run(0);

  // Pass the timeout
  zwapi_abort_send_data_ExpectAndReturn(SL_STATUS_OK);
  contiki_test_helper_run(3);

  // Still wait for the callback
  zwave_controller_on_frame_transmission_Expect(false,
                                                NULL,
                                                info.remote.node_id);

  // callback from the Z-Wave API
  TEST_ASSERT_NOT_NULL(zwave_api_send_data_callback);
  zwave_api_send_data_callback(0x49, NULL);

  TEST_ASSERT_EQUAL_PTR(user, received_user);
  TEST_ASSERT_EQUAL(0x49, received_status);
}

void test_zwave_api_transport_send_data_transmit_timeout_for_request_nif()
{
  TEST_ASSERT_NOT_NULL(zwave_api_transport.send_data);

  // Prepare data:
  info.remote.node_id   = 101;
  tx_options.fasttrack  = false;
  const uint8_t frame[] = {0x01, 0x02};
  user                  = (void *)0707;
  parent_session_id     = (void *)102;

  zwapi_request_node_info_ExpectAndReturn(info.remote.node_id, SL_STATUS_OK);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_api_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  //Run the contiki events
  contiki_test_helper_run(0);
  contiki_test_helper_run(SEND_DATA_EMERGENCY_TIMEOUT * 1000 - 1);
  contiki_test_helper_run(0);

  // Pass the timeout
  zwave_controller_on_frame_transmission_Expect(false,
                                                NULL,
                                                info.remote.node_id);
  contiki_test_helper_run(3);

  TEST_ASSERT_EQUAL_PTR(user, received_user);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_FAIL, received_status);
}

void test_zwave_api_transport_send_data_multi_happy_case()
{
  TEST_ASSERT_NOT_NULL(zwave_api_transport.send_data);

  // Prepare data:
  info.remote.node_id           = 23;
  info.remote.is_multicast      = true;
  info.remote.multicast_group   = 22;
  tx_options.transport.group_id = 2;
  const uint8_t frame[]         = {0x56, 0x32, 0x9F, 0x22, 0x01, 0xFF};
  user                          = (void *)34;
  parent_session_id             = (void *)53;

  uint8_t expected_zwapi_tx_options = TRANSMIT_OPTION_MULTICAST_AS_BROADCAST;

  zwave_tx_get_nodes_ExpectAndReturn(NULL,
                                     info.remote.multicast_group,
                                     SL_STATUS_OK);
  zwave_tx_get_nodes_IgnoreArg_nodes();

  zwapi_send_data_multi_AddCallback(zwapi_send_data_multi_stub);
  zwapi_send_data_multi_ExpectAndReturn(NULL,
                                        frame,
                                        sizeof(frame),
                                        expected_zwapi_tx_options,
                                        NULL,
                                        SL_STATUS_OK);
  zwapi_send_data_multi_IgnoreArg_destination_node_mask();
  zwapi_send_data_multi_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_api_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  //Run the contiki events
  contiki_test_helper_run(0);

  // callback from the Z-Wave API
  TEST_ASSERT_NOT_NULL(zwave_api_send_data_multi_callback);
  zwave_api_send_data_multi_callback(TRANSMIT_COMPLETE_NO_ACK);

  TEST_ASSERT_EQUAL_PTR(user, received_user);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_NO_ACK, received_status);
}

void test_zwave_api_transport_send_data_multi_frame_rejected()
{
  TEST_ASSERT_NOT_NULL(zwave_api_transport.send_data);

  // Prepare data:
  info.remote.node_id           = 23;
  info.remote.is_multicast      = true;
  info.remote.multicast_group   = 22;
  tx_options.transport.group_id = 2;
  const uint8_t frame[]         = {0x56, 0x32, 0x9F, 0x22, 0x01, 0xFF};
  user                          = (void *)34;
  parent_session_id             = (void *)53;

  uint8_t expected_zwapi_tx_options = TRANSMIT_OPTION_MULTICAST_AS_BROADCAST;

  zwave_tx_get_nodes_ExpectAndReturn(NULL,
                                     info.remote.multicast_group,
                                     SL_STATUS_OK);
  zwave_tx_get_nodes_IgnoreArg_nodes();

  zwapi_send_data_multi_AddCallback(NULL);
  zwapi_send_data_multi_ExpectAndReturn(NULL,
                                        frame,
                                        sizeof(frame),
                                        expected_zwapi_tx_options,
                                        NULL,
                                        SL_STATUS_FAIL);
  zwapi_send_data_multi_IgnoreArg_destination_node_mask();
  zwapi_send_data_multi_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_api_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  //Run the contiki events
  contiki_test_helper_run(0);
}

void test_zwave_api_transport_send_data_multi_no_node_mask()
{
  TEST_ASSERT_NOT_NULL(zwave_api_transport.send_data);

  // Prepare data:
  info.remote.node_id           = 23;
  info.remote.is_multicast      = true;
  info.remote.multicast_group   = 22;
  tx_options.transport.group_id = 2;
  const uint8_t frame[]         = {0x56, 0x32, 0x9F, 0x22, 0x01, 0xFF};
  parent_session_id             = (void *)200;

  zwave_tx_get_nodes_ExpectAndReturn(NULL,
                                     info.remote.multicast_group,
                                     SL_STATUS_FAIL);
  zwave_tx_get_nodes_IgnoreArg_nodes();

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_api_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  //Run the contiki events
  contiki_test_helper_run(0);
}

void test_zwave_api_transport_send_test_frame_happy_case()
{
  TEST_ASSERT_NOT_NULL(zwave_api_transport.send_data);

  // Prepare data:
  info.remote.node_id                = 3;
  tx_options.transport.is_test_frame = true;
  tx_options.transport.rf_power      = MINUS_7_DBM;
  user                               = (void *)199;

  zwapi_send_test_frame_AddCallback(zwapi_send_test_frame_stub);
  zwapi_send_test_frame_ExpectAndReturn(info.remote.node_id,
                                        tx_options.transport.rf_power,
                                        NULL,
                                        SL_STATUS_OK);
  zwapi_send_test_frame_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_api_transport.send_data(&info,
                                                  0,
                                                  NULL,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  //Run the contiki events
  contiki_test_helper_run(0);

  // callback from the Z-Wave API
  TEST_ASSERT_NOT_NULL(zwave_api_send_test_frame_callback);
  zwave_api_send_test_frame_callback(TRANSMIT_COMPLETE_VERIFIED);

  TEST_ASSERT_EQUAL_PTR(user, received_user);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_VERIFIED, received_status);
}

void test_zwave_api_transport_send_test_frame_rejected()
{
  TEST_ASSERT_NOT_NULL(zwave_api_transport.send_data);

  // Prepare data:
  info.remote.node_id                = 3;
  tx_options.transport.is_test_frame = true;
  tx_options.transport.rf_power      = MINUS_1_DBM;

  zwapi_send_test_frame_AddCallback(NULL);
  zwapi_send_test_frame_ExpectAndReturn(info.remote.node_id,
                                        tx_options.transport.rf_power,
                                        NULL,
                                        SL_STATUS_FAIL);
  zwapi_send_test_frame_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_api_transport.send_data(&info,
                                                  0,
                                                  NULL,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));
}

void test_zwave_api_transport_abort_happy_case()
{
  TEST_ASSERT_NOT_NULL(zwave_api_transport.abort_send_data);
  TEST_ASSERT_NOT_NULL(zwave_api_transport.send_data);

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    zwave_api_transport.abort_send_data(0));

  // Prepare data:
  info.remote.node_id   = 23;
  tx_options.fasttrack  = false;
  const uint8_t frame[] = {0x56};
  user                  = (void *)34;
  parent_session_id     = (void *)53;

  uint8_t expected_zwapi_tx_options = TRANSMIT_OPTION_ACK
                                      | TRANSMIT_OPTION_AUTO_ROUTE
                                      | TRANSMIT_OPTION_EXPLORE;

  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(info.remote.node_id,
                                  frame,
                                  sizeof(frame),
                                  expected_zwapi_tx_options,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_api_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  //Run the contiki events
  contiki_test_helper_run(0);

  // Try with unknown session, nothing will happen.
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    zwave_api_transport.abort_send_data(parent_session_id + 1));

  contiki_test_helper_run(0);

  // Abort successfully.
  zwapi_abort_send_data_ExpectAndReturn(SL_STATUS_FULL);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_api_transport.abort_send_data(parent_session_id));

  // We still wait for callback.
  contiki_test_helper_run(0);

  // Expect to notify of a frame transmission success
  zwapi_tx_report_t report = {};
  zwave_controller_on_frame_transmission_Expect(true,
                                                &report,
                                                info.remote.node_id);

  // callback from the Z-Wave API
  TEST_ASSERT_NOT_NULL(zwave_api_send_data_callback);
  zwave_api_send_data_callback(TRANSMIT_COMPLETE_VERIFIED, &report);

  TEST_ASSERT_EQUAL_PTR(user, received_user);
  TEST_ASSERT_EQUAL_MEMORY(&report,
                           &received_tx_report,
                           sizeof(zwapi_tx_report_t));
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_VERIFIED, received_status);

  // Cannot abort again
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    zwave_api_transport.abort_send_data(parent_session_id));
}

void test_zwave_api_transport_send_data_intercept_nop()
{
  TEST_ASSERT_NOT_NULL(zwave_api_transport.send_data);

  // Prepare data:
  info.remote.node_id   = 3;
  const uint8_t frame[] = {0x00};

  uint8_t expected_zwapi_tx_options = TRANSMIT_OPTION_ACK
                                      | TRANSMIT_OPTION_AUTO_ROUTE
                                      | TRANSMIT_OPTION_EXPLORE;

  zwapi_send_nop_ExpectAndReturn(info.remote.node_id,
                                 expected_zwapi_tx_options,
                                 NULL,
                                 SL_STATUS_OK);
  zwapi_send_nop_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_api_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  //Run the contiki events
  contiki_test_helper_run(0);
}

void test_zwave_api_transport_send_data_intercept_nif_request_not_supported()
{
  TEST_ASSERT_NOT_NULL(zwave_api_transport.send_data);

  // Prepare data:
  info.remote.node_id   = 3;
  const uint8_t frame[] = {0x01, 0x02};

  zwapi_request_node_info_ExpectAndReturn(info.remote.node_id, SL_STATUS_FAIL);

  uint8_t expected_zwapi_tx_options = TRANSMIT_OPTION_ACK
                                      | TRANSMIT_OPTION_AUTO_ROUTE
                                      | TRANSMIT_OPTION_EXPLORE;

  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(info.remote.node_id,
                                  frame,
                                  sizeof(frame),
                                  expected_zwapi_tx_options,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_api_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  //Run the contiki events
  contiki_test_helper_run(0);

  // Expect to notify of a frame transmission success
  zwapi_tx_report_t report = {};
  zwave_controller_on_frame_transmission_Expect(true,
                                                &report,
                                                info.remote.node_id);

  // callback from the Z-Wave API
  TEST_ASSERT_NOT_NULL(zwave_api_send_data_callback);
  zwave_api_send_data_callback(TRANSMIT_COMPLETE_OK, &report);

  TEST_ASSERT_EQUAL_PTR(user, received_user);
  TEST_ASSERT_EQUAL_MEMORY(&report,
                           &received_tx_report,
                           sizeof(zwapi_tx_report_t));
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, received_status);
}

void test_zwave_api_transport_send_data_intercept_nif_request_happy_case()
{
  TEST_ASSERT_NOT_NULL(zwave_api_transport.send_data);

  // Prepare data:
  info.remote.node_id   = 3;
  const uint8_t frame[] = {0x01, 0x02};
  user                  = (void *)0xFF;

  zwapi_request_node_info_ExpectAndReturn(info.remote.node_id, SL_STATUS_OK);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_api_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  //Run the contiki events
  contiki_test_helper_run(0);

  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks.on_node_information);

  // Wrong NIF comes in
  zwave_controller_callbacks.on_node_information(info.remote.node_id + 1, NULL);
  TEST_ASSERT_NOT_EQUAL(user, received_user);

  // Expected NIF comes in
  zwave_controller_on_frame_transmission_Expect(true,
                                                NULL,
                                                info.remote.node_id);
  zwave_controller_callbacks.on_node_information(info.remote.node_id, NULL);
  TEST_ASSERT_EQUAL_PTR(user, received_user);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, received_status);
}

void test_zwave_api_transport_send_data_intercept_nif_request_transmit_fail()
{
  TEST_ASSERT_NOT_NULL(zwave_api_transport.send_data);

  // Prepare data:
  info.remote.node_id   = 10;
  tx_options.fasttrack  = false;
  const uint8_t frame[] = {0x01, 0x02};
  user                  = (void *)0xFF;

  zwapi_request_node_info_ExpectAndReturn(info.remote.node_id, SL_STATUS_OK);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_api_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  //Run the contiki events
  contiki_test_helper_run(0);

  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks.on_node_information);
  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks.on_node_info_req_failed);

  // Wrong NIF comes in
  zwave_controller_callbacks.on_node_information(info.remote.node_id + 1, NULL);
  TEST_ASSERT_NOT_EQUAL(user, received_user);

  // NIF failure callback comes in
  zwave_controller_on_frame_transmission_Expect(false,
                                                NULL,
                                                info.remote.node_id);
  zwave_controller_callbacks.on_node_info_req_failed(info.remote.node_id);
  TEST_ASSERT_EQUAL_PTR(user, received_user);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_FAIL, received_status);
}

void test_zwave_api_transport_send_data_reject_frame_too_large()
{
  TEST_ASSERT_NOT_NULL(zwave_api_transport.send_data);

  // Prepare data:
  info.remote.node_id = 11;

  TEST_ASSERT_EQUAL(
    SL_STATUS_WOULD_OVERFLOW,
    zwave_api_transport.send_data(&info,
                                  ZWAVE_API_MAXIMUM_FRAME_LENGTH + 1,
                                  NULL,
                                  &tx_options,
                                  &on_send_data_complete,
                                  user,
                                  parent_session_id));

  //Run the contiki events
  contiki_test_helper_run(0);
}

void test_zwave_api_transport_send_data_rejects_double_frame()
{
  TEST_ASSERT_NOT_NULL(zwave_api_transport.send_data);

  // Prepare data:
  info.remote.node_id   = 23;
  tx_options.fasttrack  = false;
  const uint8_t frame[] = {0x56, 0x32, 0x9F, 0x22, 0x01, 0xFF};
  user                  = (void *)34;
  parent_session_id     = (void *)53;

  uint8_t expected_zwapi_tx_options = TRANSMIT_OPTION_ACK
                                      | TRANSMIT_OPTION_AUTO_ROUTE
                                      | TRANSMIT_OPTION_EXPLORE;

  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(info.remote.node_id,
                                  frame,
                                  sizeof(frame),
                                  expected_zwapi_tx_options,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_api_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  TEST_ASSERT_EQUAL(SL_STATUS_BUSY,
                    zwave_api_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  //Run the contiki events
  contiki_test_helper_run(0);

  // Expect to notify of a frame transmission success
  zwapi_tx_report_t report = {};
  zwave_controller_on_frame_transmission_Expect(true,
                                                &report,
                                                info.remote.node_id);

  // callback from the Z-Wave API
  TEST_ASSERT_NOT_NULL(zwave_api_send_data_callback);
  zwave_api_send_data_callback(TRANSMIT_COMPLETE_OK, &report);

  TEST_ASSERT_EQUAL_PTR(user, received_user);
  TEST_ASSERT_EQUAL_MEMORY(&report,
                           &received_tx_report,
                           sizeof(zwapi_tx_report_t));
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, received_status);
}

void test_zwave_api_transport_reset_state()
{
  // Nothing should happen outside of transmissions.
  zwave_api_transport_reset();
  zwave_api_transport_reset();

  TEST_ASSERT_NOT_NULL(zwave_api_transport.send_data);

  // Prepare data:
  info.remote.node_id   = 53;
  tx_options.fasttrack  = false;
  const uint8_t frame[] = {0x56, 0x32, 0x9F, 0x22, 0x01, 0x6F};
  user                  = (void *)34;
  parent_session_id     = (void *)53;

  uint8_t expected_zwapi_tx_options = TRANSMIT_OPTION_ACK
                                      | TRANSMIT_OPTION_AUTO_ROUTE
                                      | TRANSMIT_OPTION_EXPLORE;

  zwapi_send_data_AddCallback(zwapi_send_data_stub);
  zwapi_send_data_ExpectAndReturn(info.remote.node_id,
                                  frame,
                                  sizeof(frame),
                                  expected_zwapi_tx_options,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_api_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));

  //Run the contiki events
  contiki_test_helper_run(0);

  // Expect to notify of a frame transmission failure
  zwave_controller_on_frame_transmission_Expect(false,
                                                NULL,
                                                info.remote.node_id);

  // Now ask to reset the state:
  zwave_api_transport_reset();

  // Check if higher level callback has been invoked
  TEST_ASSERT_EQUAL_PTR(user, received_user);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_FAIL, received_status);

  // Queueing should be possible again:
  zwapi_send_data_ExpectAndReturn(info.remote.node_id,
                                  frame,
                                  sizeof(frame),
                                  expected_zwapi_tx_options,
                                  NULL,
                                  SL_STATUS_OK);
  zwapi_send_data_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_api_transport.send_data(&info,
                                                  sizeof(frame),
                                                  frame,
                                                  &tx_options,
                                                  &on_send_data_complete,
                                                  user,
                                                  parent_session_id));
}