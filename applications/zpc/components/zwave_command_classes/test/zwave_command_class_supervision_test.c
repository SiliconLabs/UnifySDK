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
#include "zwave_command_class_supervision.h"
#include "zwave_command_class_supervision_internals.h"

// Generic includes
#include <string.h>

// Test includes
#include "unity.h"
#include "contiki_test_helper.h"
#include "sys/process.h"

// Interface includes
#include "attribute_store_defined_attribute_types.h"
#include "zwave_command_class_wake_up_types.h"
#include "ZW_classcmd.h"

// Includes from other components
#include "sl_log.h"
#include "zwave_controller_connection_info.h"
#include "zwave_tx_groups.h"

// Mock includes
#include "zwave_utils_mock.h"

#include "zwave_controller_internal_mock.h"
#include "zwave_command_handler_mock.h"
#include "zwave_tx_scheme_selector_mock.h"
#include "zpc_attribute_store_network_helper_mock.h"
#include "attribute_store_mock.h"
#include "attribute_store_helper_mock.h"
#include "attribute_resolver_mock.h"
#include "zwave_tx_mock.h"
#include "zwave_unid_mock.h"

#define LOG_TAG "zwave_command_class_supervision_test"

// Static test variables
static zwave_controller_connection_info_t received_connection_info = {};
static zwave_tx_options_t received_tx_options                      = {};
static uint8_t received_data[ZWAVE_MAX_FRAME_SIZE];
static int send_data_tx_calls;
static wake_up_bitmask_t wake_up_value = 1;

static uint8_t received_supervision_status;
static zwapi_tx_report_t received_tx_info;
static void *received_user;
static void *supervision_user;
static int supervision_complete_calls;
static on_zwave_tx_send_data_complete_t supervision_on_send_complete;

// Sending and receiving data
static zwave_controller_connection_info_t test_connection_info = {};
static zwave_tx_options_t test_tx_options                      = {};
static zwapi_tx_report_t test_tx_info                          = {};
static zwave_multicast_group_id_t test_group                   = 0;
static zwave_tx_session_id_t tx_session_id                     = NULL;
static zwave_command_handler_t supervision_handler             = {};

// Callback for supervision complete
void supervision_complete_callback(uint8_t status,
                                   const zwapi_tx_report_t *tx_info,
                                   void *user)
{
  received_supervision_status = status;
  received_user               = user;
  if (tx_info != NULL) {
    received_tx_info = *tx_info;
  }
  supervision_complete_calls++;
}

void supervision_complete_callback_kills_supervision_session(
  uint8_t status, const zwapi_tx_report_t *tx_info, void *user)
{
  received_supervision_status = status;
  received_user               = user;
  if (tx_info != NULL) {
    received_tx_info = *tx_info;
  }
  supervision_complete_calls++;
  zwave_command_class_supervision_close_session_by_tx_session(tx_session_id);
}

// Stub functions
sl_status_t zwave_tx_send_data_stub(
  const zwave_controller_connection_info_t *connection,
  uint16_t data_length,
  const uint8_t *data,
  const zwave_tx_options_t *tx_options,
  const on_zwave_tx_send_data_complete_t on_send_complete,
  void *user,
  zwave_tx_session_id_t *session,
  int cmock_num_calls)
{
  supervision_on_send_complete = on_send_complete;
  supervision_user             = user;
  received_connection_info     = *connection;
  received_tx_options          = *tx_options;
  memcpy(received_data, data, data_length);

  send_data_tx_calls++;

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  supervision_handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    supervision_handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SUPERVISION,
                    supervision_handler.command_class);
  TEST_ASSERT_EQUAL(SUPERVISION_VERSION_V2, supervision_handler.version);
  TEST_ASSERT_NOT_NULL(supervision_handler.control_handler);
  TEST_ASSERT_NOT_NULL(supervision_handler.support_handler);
  TEST_ASSERT_FALSE(supervision_handler.manual_security_validation);

  return SL_STATUS_OK;
}
/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  // Create a multicast group for our tests.
  zwave_nodemask_t node_list = {};
  zwave_controller_on_multicast_group_deleted_Ignore();

  ZW_ADD_NODE_TO_MASK(1, node_list);
  ZW_ADD_NODE_TO_MASK(2, node_list);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_assign_group(node_list, &test_group));
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  send_data_tx_calls          = 0;
  supervision_complete_calls  = 0;
  supervision_user            = NULL;
  received_user               = NULL;
  received_supervision_status = 0;
  wake_up_value               = 0;
  memset(&received_tx_info, 0, sizeof(zwapi_tx_report_t));

  //Contiki starts
  contiki_test_helper_init();
  contiki_test_helper_run(0);
  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);
  zwave_command_class_supervision_init();
  contiki_test_helper_run(10);  // (get through the init event)
}

static void wake_up_on_demand_verification(bool supported)
{
  attribute_store_node_t test_node_id_node = 348;
  zwave_unid_from_node_id_Expect(test_connection_info.remote.node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  attribute_store_network_helper_get_node_id_node_ExpectAndReturn(
    NULL,
    test_node_id_node);
  attribute_store_network_helper_get_node_id_node_IgnoreArg_node_unid();
  attribute_store_node_t test_endpoint_id_node = 2;
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ENDPOINT_ID,
    REPORTED_ATTRIBUTE,
    NULL,
    sizeof(zwave_endpoint_id_t),
    0,
    test_endpoint_id_node);
  attribute_store_get_node_child_by_value_IgnoreArg_value();
  attribute_store_network_helper_get_node_id_from_node_ExpectAndReturn(
    test_endpoint_id_node,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_node_id_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_node_id_from_node_ReturnMemThruPtr_zwave_node_id(
    &test_connection_info.remote.node_id,
    sizeof(zwave_node_id_t));
  zwave_node_get_command_class_version_ExpectAndReturn(
    COMMAND_CLASS_WAKE_UP,
    test_connection_info.remote.node_id,
    0,
    3);
  zwave_node_get_command_class_version_ExpectAndReturn(
    COMMAND_CLASS_SUPERVISION,
    test_connection_info.remote.node_id,
    0,
    2);
  attribute_store_node_t test_wake_up_node_capabilities = 48;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_id_node,
    ATTRIBUTE_COMMAND_CLASS_WAKE_UP_CAPABILITIES,
    0,
    test_wake_up_node_capabilities);
  attribute_store_node_t test_wake_up_node_bitmask = 20;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_wake_up_node_capabilities,
    ATTRIBUTE_COMMAND_CLASS_WAKE_UP_CAPABILITIES_BITMASK,
    0,
    test_wake_up_node_bitmask);

  wake_up_value = 1;
  if (supported == false) {
    wake_up_value = 0;
  }
  attribute_store_get_reported_ExpectAndReturn(test_wake_up_node_bitmask,
                                               NULL,
                                               sizeof(wake_up_bitmask_t),
                                               SL_STATUS_OK);
  attribute_store_get_reported_IgnoreArg_value();
  attribute_store_get_reported_ReturnMemThruPtr_value(&wake_up_value,
                                                      sizeof(wake_up_value));
  if (supported == true) {
    is_node_or_parent_paused_ExpectAndReturn(test_node_id_node, true);
    attribute_resolver_node_or_child_needs_resolution_ExpectAndReturn(
      test_node_id_node,
      true);
  }
}

void test_zwave_command_class_supervision_init()
{
  // Nothing to test here really
  zwave_command_handler_register_handler_IgnoreAndReturn(SL_STATUS_OK);

  // Call the function
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_supervision_init());
}

void test_zwave_command_class_supervision_get_handler()
{
  // Test with wrong command
  uint8_t test_frame_data[] = {0x6C, 0x00};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    supervision_handler.support_handler(NULL,
                                        test_frame_data,
                                        sizeof(test_frame_data)));

  // Testing Supervision Encapsulated Command handler that encapsulated
  // Notification Report Command with with Home Security type and Motion detected state
  uint8_t test_frame_data_2[] = {COMMAND_CLASS_SUPERVISION,
                                 SUPERVISION_GET,
                                 0x01,
                                 0x08,
                                 0x71,
                                 0x05,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x00,
                                 0x07,
                                 0x08};
  uint8_t test_notification_cc_frame[]
    = {0x71, 0x05, 0x00, 0x00, 0x00, 0x00, 0x07, 0x08};

  // Prepare some connection info
  test_connection_info.remote.node_id     = 1;
  test_connection_info.remote.endpoint_id = 2;

  zwave_command_handler_dispatch_ExpectAndReturn(
    &test_connection_info,
    test_notification_cc_frame,
    sizeof(test_notification_cc_frame),
    SL_STATUS_OK);

  wake_up_on_demand_verification(false);

  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY,
    0,
    0,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();

  zwave_tx_send_data_AddCallback(
    (CMOCK_zwave_tx_send_data_CALLBACK)zwave_tx_send_data_stub);

  zwave_tx_send_data_ExpectAndReturn(NULL,
                                     sizeof(ZW_SUPERVISION_REPORT_FRAME),
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_tx_options();

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    supervision_handler.support_handler(&test_connection_info,
                                        test_frame_data_2,
                                        sizeof(test_frame_data_2)));

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    supervision_handler.control_handler(&test_connection_info,
                                        test_frame_data_2,
                                        sizeof(test_frame_data_2)));
}

void test_zwave_command_class_supervision_report_handler_unknown()
{
  // Testing Supervision Encapsulated Command handler that encapsulated
  // Notification Report Command with with Home Security type and Motion detected state
  uint8_t test_frame_data_2[] = {COMMAND_CLASS_SUPERVISION,
                                 SUPERVISION_REPORT,
                                 0x01,
                                 SUPERVISION_REPORT_SUCCESS,
                                 0x00};

  test_connection_info.remote.node_id     = 3;
  test_connection_info.remote.endpoint_id = 1;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    supervision_handler.control_handler(&test_connection_info,
                                        test_frame_data_2,
                                        sizeof(test_frame_data_2)));

  TEST_ASSERT_EQUAL(0, send_data_tx_calls);

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    supervision_handler.support_handler(&test_connection_info,
                                        test_frame_data_2,
                                        sizeof(test_frame_data_2)));
}

void test_zwave_command_class_supervision_report_handler_happy_case()
{
  // For the report to make sense, we need to initiate a transmission of
  // a Supervision Get.
  uint8_t test_encapsulated_frame_data[]
    = {COMMAND_CLASS_SWITCH_BINARY, SWITCH_BINARY_SET, 0xFF, 0xFF, 0x00};

  test_connection_info.remote.node_id      = 5;
  test_connection_info.remote.endpoint_id  = 4;
  test_connection_info.remote.is_multicast = false;

  test_tx_options.number_of_responses               = 0;
  test_tx_options.discard_timeout_ms                = 300;
  test_tx_options.qos_priority                      = 39485;
  test_tx_options.transport.valid_parent_session_id = false;
  test_tx_options.transport.parent_session_id       = 0;
  test_tx_options.transport.is_test_frame           = false;
  void *test_user                                   = (void *)0x9385;

  zwave_tx_send_data_AddCallback(
    (CMOCK_zwave_tx_send_data_CALLBACK)zwave_tx_send_data_stub);
  zwave_tx_send_data_ExpectAndReturn(&test_connection_info,
                                     sizeof(test_encapsulated_frame_data)
                                       + 4,  // Supervision get adds 4 bytes
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_session();

  // Generate a new session
  zwave_command_class_supervision_send_data(
    &test_connection_info,
    sizeof(test_encapsulated_frame_data),
    test_encapsulated_frame_data,
    &test_tx_options,
    supervision_complete_callback,
    test_user,
    NULL);

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(0, supervision_complete_calls);
  TEST_ASSERT_EQUAL_UINT32(test_tx_options.discard_timeout_ms,
                           received_tx_options.discard_timeout_ms);
  TEST_ASSERT_EQUAL_UINT8(1, received_tx_options.number_of_responses);
  TEST_ASSERT_EQUAL_PTR(test_tx_options.transport.parent_session_id,
                        received_tx_options.transport.parent_session_id);
  TEST_ASSERT_EQUAL_UINT32(test_tx_options.qos_priority,
                           received_tx_options.qos_priority);
  TEST_ASSERT_EQUAL_UINT8(
    test_tx_options.transport.valid_parent_session_id,
    received_tx_options.transport.valid_parent_session_id);
  TEST_ASSERT_EQUAL_UINT8(test_tx_options.transport.is_test_frame,
                          received_tx_options.transport.is_test_frame);

  // Now intercept the Session ID from the payload passed to zwave_tx_send_data()
  uint8_t used_session_id = received_data[SUPERVISION_GET_SESSION_ID_INDEX]
                            & SUPERVISION_GET_PROPERTIES1_SESSION_ID_MASK;
  uint8_t test_reply_frame_data[] = {COMMAND_CLASS_SUPERVISION,
                                     SUPERVISION_REPORT,
                                     used_session_id,
                                     SUPERVISION_REPORT_SUCCESS,
                                     0x03};
  TEST_ASSERT_EQUAL_UINT8_ARRAY(
    test_encapsulated_frame_data,
    received_data + SUPERVISION_GET_ENCAPSULATED_COMMANDS_LENGTH_INDEX + 1,
    sizeof(test_encapsulated_frame_data));

  // Simulates an incoming report, with matching Session ID
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    supervision_handler.control_handler(&test_connection_info,
                                        test_reply_frame_data,
                                        sizeof(test_reply_frame_data)));

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(1, supervision_complete_calls);
  TEST_ASSERT_EQUAL_UINT8(SUPERVISION_REPORT_SUCCESS,
                          received_supervision_status);
  TEST_ASSERT_EQUAL_PTR(test_user, received_user);
  // report arrived before tx callback, no tx_info. Don't check every field here.
  TEST_ASSERT_EQUAL_UINT8(received_tx_info.ack_channel_number, 0);
  TEST_ASSERT_EQUAL_UINT16(received_tx_info.transmit_ticks, 0);
  TEST_ASSERT_EQUAL_UINT8(received_tx_info.tx_channel_number, 0);
}

void test_zwave_command_class_supervision_report_handler_multicast_happy_case()
{
  // For the report to make sense, we need to initiate a transmission of
  // a Supervision Get.
  uint8_t test_encapsulated_frame_data[]
    = {COMMAND_CLASS_SWITCH_BINARY, SWITCH_BINARY_SET, 0xFF, 0x12, 0x00};

  test_connection_info.remote.multicast_group = test_group;
  test_connection_info.remote.endpoint_id     = 2;
  test_connection_info.remote.is_multicast    = true;

  test_tx_options.number_of_responses               = 4;
  test_tx_options.discard_timeout_ms                = 800;
  test_tx_options.qos_priority                      = 39;
  test_tx_options.transport.valid_parent_session_id = false;
  test_tx_options.transport.parent_session_id       = 0;
  test_tx_options.transport.is_test_frame           = false;
  void *test_user                                   = (void *)0x935;

  zwave_tx_send_data_AddCallback(
    (CMOCK_zwave_tx_send_data_CALLBACK)zwave_tx_send_data_stub);
  zwave_tx_send_data_ExpectAndReturn(&test_connection_info,
                                     sizeof(test_encapsulated_frame_data)
                                       + 4,  // Supervision get adds 4 bytes
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_session();

  // Generate a new session
  zwave_command_class_supervision_send_data(
    &test_connection_info,
    sizeof(test_encapsulated_frame_data),
    test_encapsulated_frame_data,
    &test_tx_options,
    supervision_complete_callback,
    test_user,
    NULL);

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(0, supervision_complete_calls);
  TEST_ASSERT_EQUAL_UINT32(test_tx_options.discard_timeout_ms,
                           received_tx_options.discard_timeout_ms);
  TEST_ASSERT_EQUAL_UINT8(test_tx_options.number_of_responses + 1,
                          received_tx_options.number_of_responses);
  TEST_ASSERT_EQUAL_PTR(test_tx_options.transport.parent_session_id,
                        received_tx_options.transport.parent_session_id);
  TEST_ASSERT_EQUAL_UINT32(test_tx_options.qos_priority,
                           received_tx_options.qos_priority);
  TEST_ASSERT_EQUAL_UINT8(
    test_tx_options.transport.valid_parent_session_id,
    received_tx_options.transport.valid_parent_session_id);
  TEST_ASSERT_EQUAL_UINT8(test_tx_options.transport.is_test_frame,
                          received_tx_options.transport.is_test_frame);

  TEST_ASSERT_EQUAL_UINT8_ARRAY(
    test_encapsulated_frame_data,
    received_data + SUPERVISION_GET_ENCAPSULATED_COMMANDS_LENGTH_INDEX + 1,
    sizeof(test_encapsulated_frame_data));

  TEST_ASSERT_EQUAL(0, supervision_complete_calls);

  // Simulate zwave_tx callback, here we should be happy:
  test_tx_info.ack_channel_number = 38;
  test_tx_info.transmit_ticks     = 0xfa9;
  test_tx_info.tx_channel_number  = 25;
  TEST_ASSERT_NOT_NULL(supervision_on_send_complete);
  supervision_on_send_complete(TRANSMIT_COMPLETE_OK,
                               &test_tx_info,
                               supervision_user);

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(0, supervision_complete_calls);

  // Now imagine that we get a success report without queuing follow-ups.
  uint8_t used_session_id = received_data[SUPERVISION_GET_SESSION_ID_INDEX]
                            & SUPERVISION_GET_PROPERTIES1_SESSION_ID_MASK;
  uint8_t test_reply_frame_data[] = {COMMAND_CLASS_SUPERVISION,
                                     SUPERVISION_REPORT,
                                     used_session_id,
                                     SUPERVISION_REPORT_SUCCESS,
                                     0x03};
  TEST_ASSERT_EQUAL_UINT8_ARRAY(
    test_encapsulated_frame_data,
    received_data + SUPERVISION_GET_ENCAPSULATED_COMMANDS_LENGTH_INDEX + 1,
    sizeof(test_encapsulated_frame_data));

  // First NodeID 1
  test_connection_info.remote.is_multicast = false;
  test_connection_info.remote.node_id      = 1;

  // Simulates an incoming report, with matching Session ID
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    supervision_handler.control_handler(&test_connection_info,
                                        test_reply_frame_data,
                                        sizeof(test_reply_frame_data)));

  TEST_ASSERT_EQUAL(1, supervision_complete_calls);

  // Second NodeID 2
  test_connection_info.remote.is_multicast = false;
  test_connection_info.remote.node_id      = 2;

  // Simulates an incoming report, with matching Session ID
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    supervision_handler.control_handler(&test_connection_info,
                                        test_reply_frame_data,
                                        sizeof(test_reply_frame_data)));

  TEST_ASSERT_EQUAL(2, supervision_complete_calls);
  TEST_ASSERT_EQUAL_UINT8(SUPERVISION_REPORT_SUCCESS,
                          received_supervision_status);
  TEST_ASSERT_EQUAL_PTR(test_user, received_user);
  // tx_info data should have been copied here due to lack of Singlecast follow-up.
}

void test_zwave_command_class_supervision_report_handler_multicast_transmit_fail()
{
  // For the report to make sense, we need to initiate a transmission of
  // a Supervision Get.
  uint8_t test_encapsulated_frame_data[]
    = {COMMAND_CLASS_SWITCH_BINARY, SWITCH_BINARY_SET, 0xFF, 0x12, 0x00};

  test_connection_info.remote.multicast_group = test_group;
  test_connection_info.remote.endpoint_id     = 2;
  test_connection_info.remote.is_multicast    = true;

  test_tx_options.number_of_responses               = 4;
  test_tx_options.discard_timeout_ms                = 800;
  test_tx_options.qos_priority                      = 39;
  test_tx_options.transport.valid_parent_session_id = false;
  test_tx_options.transport.parent_session_id       = 0;
  test_tx_options.transport.is_test_frame           = false;
  void *test_user                                   = (void *)0x935;

  zwave_tx_send_data_AddCallback(
    (CMOCK_zwave_tx_send_data_CALLBACK)zwave_tx_send_data_stub);
  zwave_tx_send_data_ExpectAndReturn(&test_connection_info,
                                     sizeof(test_encapsulated_frame_data)
                                       + 4,  // Supervision get adds 4 bytes
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_session();

  // Generate a new session
  zwave_command_class_supervision_send_data(
    &test_connection_info,
    sizeof(test_encapsulated_frame_data),
    test_encapsulated_frame_data,
    &test_tx_options,
    supervision_complete_callback,
    test_user,
    NULL);

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(0, supervision_complete_calls);
  TEST_ASSERT_EQUAL_UINT32(test_tx_options.discard_timeout_ms,
                           received_tx_options.discard_timeout_ms);
  TEST_ASSERT_EQUAL_UINT8(test_tx_options.number_of_responses + 1,
                          received_tx_options.number_of_responses);
  TEST_ASSERT_EQUAL_PTR(test_tx_options.transport.parent_session_id,
                        received_tx_options.transport.parent_session_id);
  TEST_ASSERT_EQUAL_UINT32(test_tx_options.qos_priority,
                           received_tx_options.qos_priority);
  TEST_ASSERT_EQUAL_UINT8(
    test_tx_options.transport.valid_parent_session_id,
    received_tx_options.transport.valid_parent_session_id);
  TEST_ASSERT_EQUAL_UINT8(test_tx_options.transport.is_test_frame,
                          received_tx_options.transport.is_test_frame);

  TEST_ASSERT_EQUAL_UINT8_ARRAY(
    test_encapsulated_frame_data,
    received_data + SUPERVISION_GET_ENCAPSULATED_COMMANDS_LENGTH_INDEX + 1,
    sizeof(test_encapsulated_frame_data));

  //Get the timer to run after send data call
  contiki_test_helper_run(0);

  // Simulate zwave_tx callback, here we should be happy:
  TEST_ASSERT_NOT_NULL(supervision_on_send_complete);
  supervision_on_send_complete(TRANSMIT_COMPLETE_OK,
                               &test_tx_info,
                               supervision_user);

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(0, supervision_complete_calls);

  // here we will let the timer run out. We do not check with tx was which
  // multicast session. SUPERVISION_SEND_DATA_EMERGENCY_TIMER + discard timeout
  contiki_test_helper_run(59999 + test_tx_options.discard_timeout_ms);
  TEST_ASSERT_EQUAL(0, supervision_complete_calls);
  contiki_test_helper_run(1);

  // Callbacks were called with a transmit fail.
  TEST_ASSERT_EQUAL(2, supervision_complete_calls);
  TEST_ASSERT_EQUAL_UINT8(SUPERVISION_REPORT_FAIL, received_supervision_status);
  TEST_ASSERT_EQUAL_PTR(test_user, received_user);
}

void test_zwave_command_class_supervision_report_handler_zwave_tx_fail()
{
  // For the report to make sense, we need to initiate a transmission of
  // a Supervision Get.
  uint8_t test_encapsulated_frame_data[]
    = {COMMAND_CLASS_SWITCH_BINARY, SWITCH_BINARY_SET, 0xFF, 0xFF, 0x00};

  test_connection_info.remote.node_id      = 5;
  test_connection_info.remote.endpoint_id  = 4;
  test_connection_info.remote.is_multicast = false;

  test_tx_options.number_of_responses               = 0;
  test_tx_options.discard_timeout_ms                = 300;
  test_tx_options.qos_priority                      = 39485;
  test_tx_options.transport.valid_parent_session_id = false;
  test_tx_options.transport.parent_session_id       = 0;
  test_tx_options.transport.is_test_frame           = false;
  void *test_user                                   = (void *)0x9385;

  // Ensure no callback is added, it would shadow the return value from the "ExpectAndReturn"
  zwave_tx_send_data_AddCallback(NULL);
  zwave_tx_send_data_ExpectAndReturn(
    &test_connection_info,
    sizeof(test_encapsulated_frame_data) + 4,  // Supervision get adds 4 bytes
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    SL_STATUS_FAIL);  //Session ID will be aborted

  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_session();

  // Get it to fail sending
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_command_class_supervision_send_data(
                      &test_connection_info,
                      sizeof(test_encapsulated_frame_data),
                      test_encapsulated_frame_data,
                      &test_tx_options,
                      supervision_complete_callback,
                      test_user,
                      NULL));

  // Now try a Session ID that could be matching what supervision tried to use
  uint8_t used_session_id = (received_data[SUPERVISION_GET_SESSION_ID_INDEX]
                             & SUPERVISION_GET_PROPERTIES1_SESSION_ID_MASK)
                            + 1;
  uint8_t test_reply_frame_data[] = {COMMAND_CLASS_SUPERVISION,
                                     SUPERVISION_REPORT,
                                     used_session_id,
                                     SUPERVISION_REPORT_SUCCESS,
                                     0x00};

  // Simulates an incoming report, should be unknown
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    supervision_handler.control_handler(&test_connection_info,
                                        test_reply_frame_data,
                                        sizeof(test_reply_frame_data)));

  // No callback as the session should have been aborted
  TEST_ASSERT_EQUAL(0, supervision_complete_calls);
  // no Z-Wave TX callback, no tx_info. Don't check every field here.
  TEST_ASSERT_EQUAL_UINT8(received_tx_info.ack_channel_number, 0);
  TEST_ASSERT_EQUAL_UINT16(received_tx_info.transmit_ticks, 0);
  TEST_ASSERT_EQUAL_UINT8(received_tx_info.tx_channel_number, 0);
}

void test_zwave_command_class_supervision_report_handler_working_without_update()
{
  // For the report to make sense, we need to initiate a transmission of
  // a Supervision Get.
  uint8_t test_encapsulated_frame_data[]
    = {COMMAND_CLASS_SWITCH_BINARY, SWITCH_BINARY_SET, 0xFF, 0xFF, 0x00};

  test_connection_info.remote.node_id      = 5;
  test_connection_info.remote.endpoint_id  = 4;
  test_connection_info.remote.is_multicast = false;

  test_tx_options.number_of_responses               = 0;
  test_tx_options.discard_timeout_ms                = 300;
  test_tx_options.qos_priority                      = 39485;
  test_tx_options.transport.valid_parent_session_id = false;
  test_tx_options.transport.parent_session_id       = 0;
  test_tx_options.transport.is_test_frame           = true;
  void *test_user                                   = (void *)0x9385;

  zwave_tx_send_data_AddCallback(
    (CMOCK_zwave_tx_send_data_CALLBACK)zwave_tx_send_data_stub);
  zwave_tx_send_data_ExpectAndReturn(&test_connection_info,
                                     sizeof(test_encapsulated_frame_data)
                                       + 4,  // Supervision get adds 4 bytes
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_session();

  // Generate a new session
  zwave_command_class_supervision_send_data(
    &test_connection_info,
    sizeof(test_encapsulated_frame_data),
    test_encapsulated_frame_data,
    &test_tx_options,
    supervision_complete_callback,
    test_user,
    NULL);

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(0, supervision_complete_calls);
  TEST_ASSERT_EQUAL_UINT32(test_tx_options.discard_timeout_ms,
                           received_tx_options.discard_timeout_ms);
  TEST_ASSERT_EQUAL_UINT8(1, received_tx_options.number_of_responses);
  TEST_ASSERT_EQUAL_PTR(test_tx_options.transport.parent_session_id,
                        received_tx_options.transport.parent_session_id);
  TEST_ASSERT_EQUAL_UINT32(test_tx_options.qos_priority,
                           received_tx_options.qos_priority);
  TEST_ASSERT_EQUAL_UINT8(
    test_tx_options.transport.valid_parent_session_id,
    received_tx_options.transport.valid_parent_session_id);
  TEST_ASSERT_EQUAL_UINT8(test_tx_options.transport.is_test_frame,
                          received_tx_options.transport.is_test_frame);

  // Now intercept the Session ID from the payload passed to zwave_tx_send_data()
  uint8_t used_session_id = received_data[SUPERVISION_GET_SESSION_ID_INDEX]
                            & SUPERVISION_GET_PROPERTIES1_SESSION_ID_MASK;
  uint8_t test_reply_frame_data[] = {COMMAND_CLASS_SUPERVISION,
                                     SUPERVISION_REPORT,
                                     used_session_id,  // Here no more updates
                                     SUPERVISION_REPORT_WORKING,
                                     0x20};
  TEST_ASSERT_EQUAL_UINT8_ARRAY(
    test_encapsulated_frame_data,
    received_data + SUPERVISION_GET_ENCAPSULATED_COMMANDS_LENGTH_INDEX + 1,
    sizeof(test_encapsulated_frame_data));

  // Simulates an incoming report, with matching Session ID
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    supervision_handler.control_handler(&test_connection_info,
                                        test_reply_frame_data,
                                        sizeof(test_reply_frame_data)));

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(1, supervision_complete_calls);
  TEST_ASSERT_EQUAL_UINT8(SUPERVISION_REPORT_WORKING,
                          received_supervision_status);
  TEST_ASSERT_EQUAL_PTR(test_user, received_user);
  // no Z-Wave TX callback, no tx_info. Don't check every field here.
  TEST_ASSERT_EQUAL_UINT8(received_tx_info.ack_channel_number, 0);
  TEST_ASSERT_EQUAL_UINT16(received_tx_info.transmit_ticks, 0);
  TEST_ASSERT_EQUAL_UINT8(received_tx_info.tx_channel_number, 0);

  // Simulate zwave_tx callback
  test_tx_info.ack_channel_number = 2;
  test_tx_info.transmit_ticks     = 2;
  test_tx_info.tx_channel_number  = 2;
  TEST_ASSERT_NOT_NULL(supervision_on_send_complete);
  supervision_on_send_complete(TRANSMIT_COMPLETE_OK,
                               &test_tx_info,
                               supervision_user);

  // The supervision will fail after the duration
  // time should be > SUPERVISION_REPORT_TIME + duration for the FAIL update
  contiki_test_helper_run(10);  // Get the WORKING timer started
  TEST_ASSERT_EQUAL(1, supervision_complete_calls);
  contiki_test_helper_run(32000 + 8000);
  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(2, supervision_complete_calls);
  TEST_ASSERT_EQUAL_UINT8(SUPERVISION_REPORT_FAIL, received_supervision_status);
  TEST_ASSERT_EQUAL_PTR(test_user, received_user);
  TEST_ASSERT_EQUAL_UINT8(received_tx_info.ack_channel_number, 2);
  TEST_ASSERT_EQUAL_UINT16(received_tx_info.transmit_ticks, 2);
  TEST_ASSERT_EQUAL_UINT8(received_tx_info.tx_channel_number, 2);
}

void test_zwave_command_class_supervision_report_handler_working()
{
  // For the report to make sense, we need to initiate a transmission of
  // a Supervision Get.
  uint8_t test_encapsulated_frame_data[]
    = {COMMAND_CLASS_SWITCH_BINARY, SWITCH_BINARY_SET, 0xFF, 0xFF, 0x00};

  test_connection_info.remote.node_id      = 5;
  test_connection_info.remote.endpoint_id  = 4;
  test_connection_info.remote.is_multicast = false;

  test_tx_options.number_of_responses               = 0;
  test_tx_options.discard_timeout_ms                = 300;
  test_tx_options.qos_priority                      = 39485;
  test_tx_options.transport.valid_parent_session_id = false;
  test_tx_options.transport.parent_session_id       = 0;
  test_tx_options.transport.is_test_frame           = false;
  void *test_user                                   = (void *)0x9385;

  zwave_tx_send_data_AddCallback(
    (CMOCK_zwave_tx_send_data_CALLBACK)zwave_tx_send_data_stub);
  zwave_tx_send_data_ExpectAndReturn(&test_connection_info,
                                     sizeof(test_encapsulated_frame_data)
                                       + 4,  // Supervision get adds 4 bytes
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_session();

  // Generate a new session
  zwave_command_class_supervision_send_data(
    &test_connection_info,
    sizeof(test_encapsulated_frame_data),
    test_encapsulated_frame_data,
    &test_tx_options,
    supervision_complete_callback,
    test_user,
    NULL);

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(0, supervision_complete_calls);
  TEST_ASSERT_EQUAL_UINT32(test_tx_options.discard_timeout_ms,
                           received_tx_options.discard_timeout_ms);
  TEST_ASSERT_EQUAL_UINT8(1, received_tx_options.number_of_responses);
  TEST_ASSERT_EQUAL_PTR(test_tx_options.transport.parent_session_id,
                        received_tx_options.transport.parent_session_id);
  TEST_ASSERT_EQUAL_UINT32(test_tx_options.qos_priority,
                           received_tx_options.qos_priority);
  TEST_ASSERT_EQUAL_UINT8(
    test_tx_options.transport.valid_parent_session_id,
    received_tx_options.transport.valid_parent_session_id);
  TEST_ASSERT_EQUAL_UINT8(test_tx_options.transport.is_test_frame,
                          received_tx_options.transport.is_test_frame);

  // Now intercept the Session ID from the payload passed to zwave_tx_send_data()
  uint8_t used_session_id = received_data[SUPERVISION_GET_SESSION_ID_INDEX]
                            & SUPERVISION_GET_PROPERTIES1_SESSION_ID_MASK;
  uint8_t test_reply_frame_data[]
    = {COMMAND_CLASS_SUPERVISION,
       SUPERVISION_REPORT,
       (used_session_id | SUPERVISION_GET_PROPERTIES1_STATUS_UPDATES_BIT_MASK),
       SUPERVISION_REPORT_WORKING,
       0xF2};
  TEST_ASSERT_EQUAL_UINT8_ARRAY(
    test_encapsulated_frame_data,
    received_data + SUPERVISION_GET_ENCAPSULATED_COMMANDS_LENGTH_INDEX + 1,
    sizeof(test_encapsulated_frame_data));

  // Simulates an incoming report, with matching Session ID
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    supervision_handler.control_handler(&test_connection_info,
                                        test_reply_frame_data,
                                        sizeof(test_reply_frame_data)));

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(1, supervision_complete_calls);
  TEST_ASSERT_EQUAL_UINT8(SUPERVISION_REPORT_WORKING,
                          received_supervision_status);
  TEST_ASSERT_EQUAL_PTR(test_user, received_user);

  // Simulates a second report, with unkonwn Session ID
  test_reply_frame_data[SUPERVISION_REPORT_SESSION_ID_INDEX]
    = ((used_session_id + 1)
       | SUPERVISION_GET_PROPERTIES1_STATUS_UPDATES_BIT_MASK);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    supervision_handler.control_handler(&test_connection_info,
                                        test_reply_frame_data,
                                        sizeof(test_reply_frame_data)));

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(1, supervision_complete_calls);
  TEST_ASSERT_EQUAL_UINT8(SUPERVISION_REPORT_WORKING,
                          received_supervision_status);
  TEST_ASSERT_EQUAL_PTR(test_user, received_user);

  // Simulates a final report, with matchin Session ID
  test_reply_frame_data[SUPERVISION_REPORT_SESSION_ID_INDEX]
    = ((used_session_id));  // No more update
  test_reply_frame_data[3] = SUPERVISION_REPORT_FAIL;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    supervision_handler.control_handler(&test_connection_info,
                                        test_reply_frame_data,
                                        sizeof(test_reply_frame_data)));

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(2, supervision_complete_calls);
  TEST_ASSERT_EQUAL_UINT8(SUPERVISION_REPORT_FAIL, received_supervision_status);
  TEST_ASSERT_EQUAL_PTR(test_user, received_user);
  // no Z-Wave TX callback, no tx_info. Don't check every field here.
  TEST_ASSERT_EQUAL_UINT8(received_tx_info.ack_channel_number, 0);
  TEST_ASSERT_EQUAL_UINT16(received_tx_info.transmit_ticks, 0);
  TEST_ASSERT_EQUAL_UINT8(received_tx_info.tx_channel_number, 0);
}

void test_zwave_command_class_supervision_report_send_data_complete()
{
  // Activate contiki so that the process is ready

  // Make sure the process is started here:
  zwave_command_handler_register_handler_IgnoreAndReturn(SL_STATUS_OK);
  zwave_command_class_supervision_init();
  contiki_test_helper_run(10);  // (get through the init event)

  // For the report to make sense, we need to initiate a transmission of
  // a Supervision Get.
  uint8_t test_encapsulated_frame_data[]
    = {COMMAND_CLASS_SWITCH_BINARY, SWITCH_BINARY_SET, 0xFF, 0xFF, 0x00};

  test_connection_info.remote.node_id      = 5;
  test_connection_info.remote.endpoint_id  = 1;
  test_connection_info.remote.is_multicast = false;
  void *test_user                          = (void *)0x9385;

  zwave_tx_send_data_AddCallback(
    (CMOCK_zwave_tx_send_data_CALLBACK)zwave_tx_send_data_stub);
  zwave_tx_send_data_ExpectAndReturn(&test_connection_info,
                                     sizeof(test_encapsulated_frame_data)
                                       + 4,  // Supervision get adds 4 bytes
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_session();

  // Generate a new session
  zwave_command_class_supervision_send_data(
    &test_connection_info,
    sizeof(test_encapsulated_frame_data),
    test_encapsulated_frame_data,
    &test_tx_options,
    supervision_complete_callback,
    test_user,
    NULL);

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(0, supervision_complete_calls);

  // Get a Z-Wave TX send data callback.
  TEST_ASSERT_NOT_NULL(supervision_on_send_complete);
  supervision_on_send_complete(TRANSMIT_COMPLETE_OK,
                               &test_tx_info,
                               supervision_user);

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(0, supervision_complete_calls);

  // Get through the timer activation.
  contiki_test_helper_run(10);
  // Now timer is started. Let it expire:
  // time should be > SUPERVISION_REPORT_TIME + duration
  contiki_test_helper_run(10000);

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(1, supervision_complete_calls);

  // Just increase the test coverage by stoping the process.
  process_post(PROCESS_BROADCAST, PROCESS_EVENT_EXIT, 0);
  contiki_test_helper_run(10);
  process_post(PROCESS_BROADCAST, PROCESS_EVENT_EXITED, 0);
  contiki_test_helper_run(10);
}

void test_zwave_command_class_supervision_report_multi_sessions()
{
  // For the report to make sense, we need to initiate a transmission of
  // a Supervision Get.
  uint8_t test_encapsulated_frame_data[]
    = {COMMAND_CLASS_SWITCH_BINARY, SWITCH_BINARY_SET, 0xFF, 0xFF, 0x00};

  test_connection_info.remote.node_id      = 5;
  test_connection_info.remote.endpoint_id  = 1;
  test_connection_info.remote.is_multicast = false;
  void *test_user_node_5                   = (void *)0x9385;

  zwave_tx_send_data_AddCallback(
    (CMOCK_zwave_tx_send_data_CALLBACK)zwave_tx_send_data_stub);
  zwave_tx_send_data_ExpectAndReturn(&test_connection_info,
                                     sizeof(test_encapsulated_frame_data)
                                       + 4,  // Supervision get adds 4 bytes
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_session();

  // Generate a new session
  zwave_command_class_supervision_send_data(
    &test_connection_info,
    sizeof(test_encapsulated_frame_data),
    test_encapsulated_frame_data,
    &test_tx_options,
    supervision_complete_callback,
    test_user_node_5,
    NULL);

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(0, supervision_complete_calls);

  // Get a Z-Wave TX send data callback.
  TEST_ASSERT_NOT_NULL(supervision_on_send_complete);
  supervision_on_send_complete(TRANSMIT_COMPLETE_OK,
                               &test_tx_info,
                               supervision_user);

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(0, supervision_complete_calls);

  // Get through the timer activation.
  contiki_test_helper_run(10);
  // Now timer is started.
  // Save the Session ID for node 5
  uint8_t used_session_id_node_5
    = received_data[SUPERVISION_GET_SESSION_ID_INDEX]
      & SUPERVISION_GET_PROPERTIES1_SESSION_ID_MASK;

  // Another node needs to be controlled:
  test_connection_info.remote.node_id = 231;
  void *test_user_node_231            = (void *)0x1235;

  zwave_tx_send_data_AddCallback(
    (CMOCK_zwave_tx_send_data_CALLBACK)zwave_tx_send_data_stub);
  zwave_tx_send_data_ExpectAndReturn(&test_connection_info,
                                     sizeof(test_encapsulated_frame_data)
                                       + 4,  // Supervision get adds 4 bytes
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_session();

  zwave_command_class_supervision_send_data(
    &test_connection_info,
    sizeof(test_encapsulated_frame_data),
    test_encapsulated_frame_data,
    &test_tx_options,
    supervision_complete_callback,
    test_user_node_231,
    NULL);

  // Node 231 sends us a report:
  uint8_t used_session_id_node_231
    = received_data[SUPERVISION_GET_SESSION_ID_INDEX]
      & SUPERVISION_GET_PROPERTIES1_SESSION_ID_MASK;
  uint8_t test_reply_frame_data[]
    = {COMMAND_CLASS_SUPERVISION,
       SUPERVISION_REPORT,
       used_session_id_node_231,  // Here no more updates
       SUPERVISION_REPORT_SUCCESS,
       0xFF};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    supervision_handler.control_handler(&test_connection_info,
                                        test_reply_frame_data,
                                        sizeof(test_reply_frame_data)));

  TEST_ASSERT_EQUAL(2, send_data_tx_calls);
  TEST_ASSERT_EQUAL(1, supervision_complete_calls);
  TEST_ASSERT_EQUAL_UINT8(SUPERVISION_REPORT_SUCCESS,
                          received_supervision_status);
  TEST_ASSERT_EQUAL_PTR(test_user_node_231, received_user);
  TEST_ASSERT_EQUAL_UINT8(received_tx_info.ack_channel_number, 0);
  TEST_ASSERT_EQUAL_UINT16(received_tx_info.transmit_ticks, 0);
  TEST_ASSERT_EQUAL_UINT8(received_tx_info.tx_channel_number, 0);

  // Node 5 tells us it is done now:
  test_connection_info.remote.node_id = 5;

  test_reply_frame_data[SUPERVISION_REPORT_STATUS_INDEX]
    = SUPERVISION_REPORT_SUCCESS;
  // Did not find a nice way to save the SessionID for node5 (gets overwritten)
  // but since it is incremented every time, all good.
  test_reply_frame_data[SUPERVISION_REPORT_SESSION_ID_INDEX]
    = used_session_id_node_5
      | SUPERVISION_REPORT_PROPERTIES1_MORE_STATUS_UPDATES_BIT_MASK;
  test_reply_frame_data[SUPERVISION_REPORT_DURATION_INDEX] = 2;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    supervision_handler.control_handler(&test_connection_info,
                                        test_reply_frame_data,
                                        sizeof(test_reply_frame_data)));

  TEST_ASSERT_EQUAL(2, supervision_complete_calls);
  TEST_ASSERT_EQUAL_UINT8(SUPERVISION_REPORT_SUCCESS,
                          received_supervision_status);
  TEST_ASSERT_EQUAL_PTR(test_user_node_5, received_user);
  TEST_ASSERT_EQUAL_UINT8(received_tx_info.ack_channel_number,
                          test_tx_info.ack_channel_number);
  TEST_ASSERT_EQUAL_UINT16(received_tx_info.transmit_ticks,
                           test_tx_info.transmit_ticks);
  TEST_ASSERT_EQUAL_UINT8(received_tx_info.tx_channel_number,
                          test_tx_info.tx_channel_number);

  // Now timer expires
  // time should be > SUPERVISION_REPORT_TIME + duration
  contiki_test_helper_run(10000);
  // But nothing should happen since the session has been removed.
  TEST_ASSERT_EQUAL(2, supervision_complete_calls);
  TEST_ASSERT_EQUAL_UINT8(SUPERVISION_REPORT_SUCCESS,
                          received_supervision_status);
  TEST_ASSERT_EQUAL_PTR(test_user_node_5, received_user);
  TEST_ASSERT_EQUAL_UINT8(received_tx_info.ack_channel_number,
                          test_tx_info.ack_channel_number);
  TEST_ASSERT_EQUAL_UINT16(received_tx_info.transmit_ticks,
                           test_tx_info.transmit_ticks);
  TEST_ASSERT_EQUAL_UINT8(received_tx_info.tx_channel_number,
                          test_tx_info.tx_channel_number);
}

void test_zwave_command_class_supervision_abort_send_data()
{
  // Initiate a transmission of a Supervision Get.
  uint8_t test_encapsulated_frame_data[]
    = {COMMAND_CLASS_SWITCH_BINARY, SWITCH_BINARY_SET, 0xFF, 0xFF, 0x00};

  void *test_user = (void *)0x9385;
  tx_session_id   = NULL;

  zwave_tx_send_data_AddCallback(
    (CMOCK_zwave_tx_send_data_CALLBACK)zwave_tx_send_data_stub);
  zwave_tx_send_data_ExpectAndReturn(&test_connection_info,
                                     sizeof(test_encapsulated_frame_data)
                                       + 4,  // Supervision get adds 4 bytes
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     &tx_session_id,
                                     SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();

  // Generate a new session
  zwave_command_class_supervision_send_data(
    &test_connection_info,
    sizeof(test_encapsulated_frame_data),
    test_encapsulated_frame_data,
    &test_tx_options,
    supervision_complete_callback,
    test_user,
    &tx_session_id);

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(0, supervision_complete_calls);

  // Now, we want to try to abort the tx session:
  zwave_tx_abort_transmission_ExpectAndReturn(tx_session_id,
                                              SL_STATUS_HAS_OVERFLOWED);
  TEST_ASSERT_EQUAL(
    SL_STATUS_HAS_OVERFLOWED,
    zwave_command_class_supervision_abort_send_data(tx_session_id));
}

void test_zwave_command_class_supervision_send_data_too_large_frame()
{
  // Initiate a transmission of a Supervision Get.
  uint8_t test_encapsulated_frame_data[]
    = {COMMAND_CLASS_SWITCH_BINARY, SWITCH_BINARY_SET, 0xFF, 0xFF, 0x00};

  // Generate a new session, expect a fail
  TEST_ASSERT_EQUAL(SL_STATUS_WOULD_OVERFLOW,
                    zwave_command_class_supervision_send_data(
                      &test_connection_info,
                      SUPERVISION_ENCAPSULATED_COMMAND_MAXIMUM_SIZE + 1,
                      test_encapsulated_frame_data,
                      &test_tx_options,
                      supervision_complete_callback,
                      NULL,
                      NULL));

  TEST_ASSERT_EQUAL(0, send_data_tx_calls);
  TEST_ASSERT_EQUAL(0, supervision_complete_calls);
}

void test_zwave_command_class_supervision_control_handler_wrong_cc()
{
  // Wrong Command Class / Command
  uint8_t test_encapsulated_frame_data[]
    = {COMMAND_CLASS_SWITCH_BINARY, 0xFE, 0xFF, 0xFF, 0x00};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    supervision_handler.control_handler(&test_connection_info,
                                        test_encapsulated_frame_data,
                                        sizeof(test_encapsulated_frame_data)));
}

void test_zwave_command_class_supervision_support_handler_wrong_cc()
{
  // Wrong Command Class / Command
  uint8_t test_encapsulated_frame_data[]
    = {COMMAND_CLASS_SWITCH_BINARY, 0xFE, 0xFF, 0xFF, 0x00};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    supervision_handler.support_handler(&test_connection_info,
                                        test_encapsulated_frame_data,
                                        sizeof(test_encapsulated_frame_data)));
}

void test_zwave_command_class_supervision_support_handler_frame_too_short()
{
  // Initiate a transmission of a Supervision Get.
  uint8_t test_encapsulated_frame_data[]
    = {COMMAND_CLASS_SUPERVISION, SUPERVISION_GET, 0xFF, 0xFF, 0x00};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    supervision_handler.support_handler(&test_connection_info,
                                        test_encapsulated_frame_data,
                                        1));  // Frame must be at least 2 bytes
}

void test_zwave_command_class_supervision_control_handler_frame_too_short()
{
  // Initiate a transmission of a Supervision Get.
  uint8_t test_encapsulated_frame_data[]
    = {COMMAND_CLASS_SUPERVISION, SUPERVISION_GET, 0xFF, 0xFF, 0x00};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    supervision_handler.control_handler(&test_connection_info,
                                        test_encapsulated_frame_data,
                                        1));  // Frame must be at least 2 bytes
}

void test_zwave_command_class_supervision_wake_on_demand_functionality()
{
  test_connection_info.remote.node_id = 7;

  // Add the node:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_supervision_wake_on_demand(
                      test_connection_info.remote.node_id));

  // Remove it:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_supervision_stop_wake_on_demand(
                      test_connection_info.remote.node_id));

  // Remove again should not harm.
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_supervision_stop_wake_on_demand(
                      test_connection_info.remote.node_id));

  // Add the node 0, it should not accept:
  TEST_ASSERT_EQUAL(SL_STATUS_WOULD_OVERFLOW,
                    zwave_command_class_supervision_wake_on_demand(0));

  // Add the node 4233, it should not accept:
  TEST_ASSERT_EQUAL(SL_STATUS_WOULD_OVERFLOW,
                    zwave_command_class_supervision_wake_on_demand(4233));

  // Remove node 0, it should not harm:
  TEST_ASSERT_EQUAL(SL_STATUS_WOULD_OVERFLOW,
                    zwave_command_class_supervision_stop_wake_on_demand(0));
  // Remove node 4233, it should not harm either:
  TEST_ASSERT_EQUAL(SL_STATUS_WOULD_OVERFLOW,
                    zwave_command_class_supervision_stop_wake_on_demand(4233));

  // Node 232 should be accepted:
  test_connection_info.remote.node_id = 232;

  // Add the node:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_supervision_wake_on_demand(
                      test_connection_info.remote.node_id));
  // Remove it:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_supervision_stop_wake_on_demand(
                      test_connection_info.remote.node_id));
}

void test_zwave_command_class_supervision_wake_on_demand_bit()
{
  test_connection_info.remote.node_id = 7;

  // Receive a Supervision Get from somebody that don't need wake up on demand:
  uint8_t test_frame_data[] = {COMMAND_CLASS_SUPERVISION,
                               SUPERVISION_GET,
                               0x23,
                               0x05,
                               COMMAND_CLASS_SWITCH_BINARY,
                               SWITCH_BINARY_REPORT,
                               0x00,
                               0x00,
                               0x00};

  // Prepare some connection info
  test_connection_info.remote.node_id     = 1;
  test_connection_info.remote.endpoint_id = 2;

  zwave_command_handler_dispatch_ExpectAndReturn(&test_connection_info,
                                                 &test_frame_data[4],
                                                 5,
                                                 SL_STATUS_OK);

  wake_up_on_demand_verification(false);

  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY,
    0,
    0,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();

  zwave_tx_send_data_AddCallback(
    (CMOCK_zwave_tx_send_data_CALLBACK)zwave_tx_send_data_stub);

  zwave_tx_send_data_ExpectAndReturn(NULL,
                                     sizeof(ZW_SUPERVISION_REPORT_FRAME),
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_tx_options();

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    supervision_handler.support_handler(&test_connection_info,
                                        test_frame_data,
                                        sizeof(test_frame_data)));

  // Now intercept the Wake Up Request flag and verify that it's not 1
  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(0,
                    received_data[SUPERVISION_REPORT_SESSION_ID_INDEX]
                      & SUPERVISION_REPORT_PROPERTIES1_WAKE_UP_BIT_MASK);

  // Now another node sends us a Supervision Get
  // Prepare some connection info
  test_connection_info.remote.node_id     = 7;
  test_connection_info.remote.endpoint_id = 59;

  zwave_command_handler_dispatch_ExpectAndReturn(&test_connection_info,
                                                 &test_frame_data[4],
                                                 5,
                                                 SL_STATUS_OK);

  wake_up_on_demand_verification(true);

  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY,
    0,
    0,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();

  zwave_tx_send_data_ExpectAndReturn(NULL,
                                     sizeof(ZW_SUPERVISION_REPORT_FRAME),
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_tx_options();

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    supervision_handler.support_handler(&test_connection_info,
                                        test_frame_data,
                                        sizeof(test_frame_data)));

  // Now intercept the Wake Up Request flag and verify that it's 1
  TEST_ASSERT_EQUAL(2, send_data_tx_calls);
  TEST_ASSERT_EQUAL(SUPERVISION_REPORT_PROPERTIES1_WAKE_UP_BIT_MASK,
                    received_data[SUPERVISION_REPORT_SESSION_ID_INDEX]
                      & SUPERVISION_REPORT_PROPERTIES1_WAKE_UP_BIT_MASK);
}

void test_zwave_command_class_supervision_get_handler_tx_fails()
{
  // Receive a Supervision Get
  uint8_t test_frame_data[] = {COMMAND_CLASS_SUPERVISION,
                               SUPERVISION_GET,
                               0x23,
                               0x05,
                               COMMAND_CLASS_SWITCH_BINARY,
                               SWITCH_BINARY_REPORT,
                               0x00,
                               0x00,
                               0x00};

  // Prepare some connection info
  test_connection_info.remote.node_id     = 1;
  test_connection_info.remote.endpoint_id = 2;

  zwave_command_handler_dispatch_ExpectAndReturn(&test_connection_info,
                                                 &test_frame_data[4],
                                                 5,
                                                 SL_STATUS_OK);
  wake_up_on_demand_verification(false);

  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY,
    0,
    0,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();

  // Ensure no callback is added, it would shadow the return value from the "ExpectAndReturn"
  zwave_tx_send_data_AddCallback(NULL);
  zwave_tx_send_data_ExpectAndReturn(NULL,
                                     sizeof(ZW_SUPERVISION_REPORT_FRAME),
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_WOULD_BLOCK);

  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_tx_options();

  TEST_ASSERT_EQUAL(
    SL_STATUS_WOULD_BLOCK,
    supervision_handler.support_handler(&test_connection_info,
                                        test_frame_data,
                                        sizeof(test_frame_data)));
}

void test_zwave_command_class_supervision_get_handler_duplicate()
{
  // Receive a Supervision Get
  uint8_t test_frame_data[] = {COMMAND_CLASS_SUPERVISION,
                               SUPERVISION_GET,
                               0x23,
                               0x05,
                               COMMAND_CLASS_SWITCH_BINARY,
                               SWITCH_BINARY_REPORT,
                               0x00,
                               0x00,
                               0x00};

  // Prepare some non-secure connection info
  test_connection_info.encapsulation      = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  test_connection_info.remote.node_id     = 1;
  test_connection_info.remote.endpoint_id = 1;

  zwave_command_handler_dispatch_ExpectAndReturn(&test_connection_info,
                                                 &test_frame_data[4],
                                                 5,
                                                 SL_STATUS_FAIL);

  wake_up_on_demand_verification(false);

  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY,
    0,
    0,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();

  zwave_tx_send_data_AddCallback(
    (CMOCK_zwave_tx_send_data_CALLBACK)zwave_tx_send_data_stub);
  zwave_tx_send_data_ExpectAndReturn(NULL,
                                     sizeof(ZW_SUPERVISION_REPORT_FRAME),
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_tx_options();

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    supervision_handler.support_handler(&test_connection_info,
                                        test_frame_data,
                                        sizeof(test_frame_data)));

  // Verify that the Status given by the handler is reflected in Supervision Report
  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(SUPERVISION_REPORT_FAIL,
                    received_data[SUPERVISION_REPORT_STATUS_INDEX]);
  TEST_ASSERT_EQUAL(0x23, received_data[SUPERVISION_REPORT_SESSION_ID_INDEX]);
  TEST_ASSERT_EQUAL(
    0,
    received_data[SUPERVISION_REPORT_SESSION_ID_INDEX]
      & SUPERVISION_REPORT_PROPERTIES1_MORE_STATUS_UPDATES_BIT_MASK);

  // Once more with a duplicate, should not trigger any frame handler dispatch or TX:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    supervision_handler.support_handler(&test_connection_info,
                                        test_frame_data,
                                        sizeof(test_frame_data)));

  // No more report must have been sent
  TEST_ASSERT_EQUAL(1, send_data_tx_calls);

  // Weird scenario: now receive the same again but with the multicast bit.
  // Handler should be called, but no Report.
  test_connection_info.local.is_multicast = true;

  zwave_command_handler_dispatch_ExpectAndReturn(&test_connection_info,
                                                 &test_frame_data[4],
                                                 5,
                                                 SL_STATUS_FAIL);

  // Once more with a duplicate, should trigger frame handler dispatch but no TX:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    supervision_handler.support_handler(&test_connection_info,
                                        test_frame_data,
                                        sizeof(test_frame_data)));

  // No more report must have been sent
  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
}

void test_zwave_command_class_supervision_get_handler_status_busy()
{
  test_connection_info.encapsulation
    = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS;
  test_connection_info.remote.node_id = 2;
  // Receive a Supervision Get
  uint8_t test_frame_data[] = {COMMAND_CLASS_SUPERVISION,
                               SUPERVISION_GET,
                               0x23,
                               0x05,
                               COMMAND_CLASS_SWITCH_BINARY,
                               SWITCH_BINARY_REPORT,
                               0x00,
                               0x00,
                               0x00};

  test_connection_info.local.is_multicast = false;

  zwave_command_handler_dispatch_ExpectAndReturn(&test_connection_info,
                                                 &test_frame_data[4],
                                                 5,
                                                 SL_STATUS_BUSY);
  wake_up_on_demand_verification(false);

  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY,
    0,
    0,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();

  zwave_tx_send_data_AddCallback(
    (CMOCK_zwave_tx_send_data_CALLBACK)zwave_tx_send_data_stub);
  zwave_tx_send_data_ExpectAndReturn(NULL,
                                     sizeof(ZW_SUPERVISION_REPORT_FRAME),
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_tx_options();

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    supervision_handler.support_handler(&test_connection_info,
                                        test_frame_data,
                                        sizeof(test_frame_data)));

  // Verify that the BUSY return code gives a FAIL status.
  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(SUPERVISION_REPORT_FAIL,
                    received_data[SUPERVISION_REPORT_STATUS_INDEX]);
  TEST_ASSERT_EQUAL(0x23, received_data[SUPERVISION_REPORT_SESSION_ID_INDEX]);
  TEST_ASSERT_EQUAL(
    0,
    received_data[SUPERVISION_REPORT_SESSION_ID_INDEX]
      & SUPERVISION_REPORT_PROPERTIES1_MORE_STATUS_UPDATES_BIT_MASK);
}

void test_zwave_command_class_supervision_get_handler_status_not_supported()
{
  // Receive a Supervision Get
  uint8_t test_frame_data[] = {COMMAND_CLASS_SUPERVISION,
                               SUPERVISION_GET,
                               0x23,
                               0x05,
                               COMMAND_CLASS_SWITCH_BINARY,
                               SWITCH_BINARY_REPORT,
                               0x00,
                               0x00,
                               0x00};

  // Change something so we do not get filtered as duplicate
  test_connection_info.remote.endpoint_id = 19;

  wake_up_on_demand_verification(false);

  zwave_command_handler_dispatch_ExpectAndReturn(&test_connection_info,
                                                 &test_frame_data[4],
                                                 5,
                                                 SL_STATUS_NOT_SUPPORTED);
  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY,
    0,
    0,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();

  zwave_tx_send_data_AddCallback(
    (CMOCK_zwave_tx_send_data_CALLBACK)zwave_tx_send_data_stub);
  zwave_tx_send_data_ExpectAndReturn(NULL,
                                     sizeof(ZW_SUPERVISION_REPORT_FRAME),
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_tx_options();

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    supervision_handler.support_handler(&test_connection_info,
                                        test_frame_data,
                                        sizeof(test_frame_data)));

  // Verify that the NOT_SUPPORTED return code gives a NO_SUPPORT status.
  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(SUPERVISION_REPORT_NO_SUPPORT,
                    received_data[SUPERVISION_REPORT_STATUS_INDEX]);
  TEST_ASSERT_EQUAL(0x23, received_data[SUPERVISION_REPORT_SESSION_ID_INDEX]);
  TEST_ASSERT_EQUAL(
    0,
    received_data[SUPERVISION_REPORT_SESSION_ID_INDEX]
      & SUPERVISION_REPORT_PROPERTIES1_MORE_STATUS_UPDATES_BIT_MASK);
}

void test_zwave_command_class_supervision_get_handler_status_default()
{
  // Receive a Supervision Get
  uint8_t test_frame_data[] = {COMMAND_CLASS_SUPERVISION,
                               SUPERVISION_GET,
                               0x23,
                               0x05,
                               COMMAND_CLASS_SWITCH_BINARY,
                               SWITCH_BINARY_REPORT,
                               0x00,
                               0x00,
                               0x00};

  test_connection_info.remote.node_id = 19;

  zwave_command_handler_dispatch_ExpectAndReturn(&test_connection_info,
                                                 &test_frame_data[4],
                                                 5,
                                                 SL_STATUS_WOULD_BLOCK);

  wake_up_on_demand_verification(false);

  zwave_tx_scheme_get_node_tx_options_Expect(
    ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY,
    0,
    0,
    NULL);
  zwave_tx_scheme_get_node_tx_options_IgnoreArg_tx_options();

  zwave_tx_send_data_AddCallback(
    (CMOCK_zwave_tx_send_data_CALLBACK)zwave_tx_send_data_stub);
  zwave_tx_send_data_ExpectAndReturn(NULL,
                                     sizeof(ZW_SUPERVISION_REPORT_FRAME),
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_tx_options();

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    supervision_handler.support_handler(&test_connection_info,
                                        test_frame_data,
                                        sizeof(test_frame_data)));

  // Verify that the SL_STATUS_WOULD_BLOCK return code gives a NO_SUPPORT status.
  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(SUPERVISION_REPORT_NO_SUPPORT,
                    received_data[SUPERVISION_REPORT_STATUS_INDEX]);
  TEST_ASSERT_EQUAL(0x23, received_data[SUPERVISION_REPORT_SESSION_ID_INDEX]);
  TEST_ASSERT_EQUAL(
    0,
    received_data[SUPERVISION_REPORT_SESSION_ID_INDEX]
      & SUPERVISION_REPORT_PROPERTIES1_MORE_STATUS_UPDATES_BIT_MASK);
}

void test_zwave_command_class_supervision_send_data_callback_closes_the_session()
{
  // For the report to make sense, we need to initiate a transmission of
  // a Supervision Get.
  uint8_t test_encapsulated_frame_data[]
    = {COMMAND_CLASS_SWITCH_BINARY, SWITCH_BINARY_SET, 0xFF, 0xFF, 0x00};

  test_connection_info.remote.node_id      = 5;
  test_connection_info.remote.endpoint_id  = 4;
  test_connection_info.remote.is_multicast = false;

  test_tx_options.number_of_responses               = 0;
  test_tx_options.discard_timeout_ms                = 300;
  test_tx_options.qos_priority                      = 39485;
  test_tx_options.transport.valid_parent_session_id = false;
  test_tx_options.transport.parent_session_id       = 0;
  test_tx_options.transport.is_test_frame           = false;
  void *test_user                                   = (void *)0x9385;

  zwave_tx_send_data_AddCallback(
    (CMOCK_zwave_tx_send_data_CALLBACK)zwave_tx_send_data_stub);
  zwave_tx_send_data_ExpectAndReturn(&test_connection_info,
                                     sizeof(test_encapsulated_frame_data)
                                       + 4,  // Supervision get adds 4 bytes
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_session();

  // Generate a new session
  zwave_command_class_supervision_send_data(
    &test_connection_info,
    sizeof(test_encapsulated_frame_data),
    test_encapsulated_frame_data,
    &test_tx_options,
    supervision_complete_callback_kills_supervision_session,
    test_user,
    &tx_session_id);

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(0, supervision_complete_calls);
  TEST_ASSERT_EQUAL_UINT32(test_tx_options.discard_timeout_ms,
                           received_tx_options.discard_timeout_ms);
  TEST_ASSERT_EQUAL_UINT8(1, received_tx_options.number_of_responses);
  TEST_ASSERT_EQUAL_PTR(test_tx_options.transport.parent_session_id,
                        received_tx_options.transport.parent_session_id);
  TEST_ASSERT_EQUAL_UINT32(test_tx_options.qos_priority,
                           received_tx_options.qos_priority);
  TEST_ASSERT_EQUAL_UINT8(
    test_tx_options.transport.valid_parent_session_id,
    received_tx_options.transport.valid_parent_session_id);
  TEST_ASSERT_EQUAL_UINT8(test_tx_options.transport.is_test_frame,
                          received_tx_options.transport.is_test_frame);

  TEST_ASSERT_EQUAL_UINT8_ARRAY(
    test_encapsulated_frame_data,
    received_data + SUPERVISION_GET_ENCAPSULATED_COMMANDS_LENGTH_INDEX + 1,
    sizeof(test_encapsulated_frame_data));

  // Get a Z-Wave TX send data callback.
  TEST_ASSERT_NOT_NULL(supervision_on_send_complete);
  supervision_on_send_complete(TRANSMIT_COMPLETE_OK,
                               &test_tx_info,
                               supervision_user);

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(0, supervision_complete_calls);

  // Get through the timer activation.
  contiki_test_helper_run(10);
  // Now timer is started. Let it expire:
  // time should be > SUPERVISION_REPORT_TIME + duration
  contiki_test_helper_run(10000);

  TEST_ASSERT_EQUAL(1, send_data_tx_calls);
  TEST_ASSERT_EQUAL(1, supervision_complete_calls);
  TEST_ASSERT_EQUAL_UINT8(SUPERVISION_REPORT_FAIL, received_supervision_status);
  TEST_ASSERT_EQUAL_PTR(test_user, received_user);
}