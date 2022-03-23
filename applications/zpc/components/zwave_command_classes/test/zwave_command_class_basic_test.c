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
#include "zwave_command_class_basic.h"
#include "zwave_command_classes_utils.h"
#include "unity.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"

// Interface includes
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"
#include "zwave_utils.h"
#include "zwave_controller_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "attribute_resolver_mock.h"
#include "zpc_attribute_resolver_mock.h"
#include "zwave_command_handler_mock.h"
#include "zwave_tx_mock.h"
#include "zwave_tx_scheme_selector_mock.h"
#include "dotdot_mqtt_mock.h"
#include "dotdot_mqtt_supported_generated_commands_mock.h"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_BASIC_##type

// Static variables
static attribute_resolver_function_t basic_set                         = NULL;
static attribute_resolver_function_t basic_get                         = NULL;
static void (*on_endpoint_interview_completed)(attribute_store_node_t) = NULL;
static zwave_command_handler_t basic_handler                           = {};

static uint8_t received_frame[255]  = {};
static uint16_t received_frame_size = 0;

static uint8_t u8_value   = 0;
static uint32_t u32_value = 0;

// Stub functions
static sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  if (node_type != ATTRIBUTE(VALUE)) {
    TEST_FAIL_MESSAGE("Attribute rule registration on a wrong type");
  }

  if (node_type == ATTRIBUTE(VALUE)) {
    basic_set = set_func;
    basic_get = get_func;
  }
  return SL_STATUS_OK;
}

void attribute_resolver_set_resolution_listener_stub(
  attribute_store_node_t node,
  cmock_attribute_resolver_func_ptr1 callback,
  int cmock_num_calls)
{
  on_endpoint_interview_completed = callback;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  basic_handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    basic_handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_BASIC_V2, basic_handler.command_class);
  TEST_ASSERT_EQUAL(BASIC_VERSION_V2, basic_handler.version);
  TEST_ASSERT_NOT_NULL(basic_handler.control_handler);
  TEST_ASSERT_NULL(basic_handler.support_handler);
  TEST_ASSERT_FALSE(basic_handler.manual_security_validation);

  return SL_STATUS_OK;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

static void zwave_command_class_basic_init_verification()
{
  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  attribute_resolver_set_resolution_listener_Stub(
    &attribute_resolver_set_resolution_listener_stub);

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_basic_init());
}

/// Called before each and every test
void setUp()
{
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_set_home_id(home_id);

  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size             = 0;
  u8_value                        = 0;
  u32_value                       = 0;
  basic_set                       = NULL;
  basic_get                       = NULL;
  on_endpoint_interview_completed = NULL;
  memset(&basic_handler, 0, sizeof(zwave_command_handler_t));

  zwave_command_class_basic_init_verification();
}

void test_zwave_command_class_basic_get_happy_case()
{
  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL(basic_get);
  basic_get(0, received_frame, &received_frame_size);
  const uint8_t expected_frame[] = {COMMAND_CLASS_BASIC_V2, BASIC_GET_V2};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_basic_set_happy_case()
{
  // Create a VALUE node under the endpoint
  attribute_store_node_t value_node
    = attribute_store_add_node(ATTRIBUTE(VALUE), endpoint_id_node);

  u32_value = 34;
  attribute_store_set_desired(value_node, &u32_value, sizeof(u32_value));

  // Now ask for a Set Command:
  TEST_ASSERT_NOT_NULL(basic_set);
  basic_set(value_node, received_frame, &received_frame_size);
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_BASIC_V2, BASIC_SET_V2, (uint8_t)u32_value};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_basic_set_duration_non_zero()
{
  // Create a VALUE node under the endpoint
  attribute_store_node_t value_node
    = attribute_store_add_node(ATTRIBUTE(VALUE), endpoint_id_node);

  attribute_store_node_t duration_node
    = attribute_store_add_node(ATTRIBUTE(DURATION), endpoint_id_node);

  u32_value = 34;
  attribute_store_set_desired(value_node, &u32_value, sizeof(u32_value));
  attribute_store_set_reported(duration_node, &u32_value, sizeof(u32_value));

  // Now ask for a Set Command:
  TEST_ASSERT_NOT_NULL(basic_set);
  TEST_ASSERT_EQUAL(
    SL_STATUS_IS_WAITING,
    basic_set(value_node, received_frame, &received_frame_size));

  TEST_ASSERT_EQUAL(0, received_frame_size);
}

void test_zwave_command_class_basic_actuator_in_nif()
{
  // Create a NIF under the endpoint:
  attribute_store_node_t nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);

  zwave_command_handler_controls_IgnoreAndReturn(true);

  // Use the received_frame buffer to fill the NIF data.
  received_frame[0] = COMMAND_CLASS_SECURITY_2;
  received_frame[1] = COMMAND_CLASS_SUPERVISION;
  received_frame[2] = COMMAND_CLASS_MULTI_CHANNEL_V4;
  received_frame[3] = COMMAND_CLASS_ZWAVEPLUS_INFO_V2;
  received_frame[4] = COMMAND_CLASS_SWITCH_BINARY_V2;
  attribute_store_set_reported(nif_node, received_frame, 5 * sizeof(uint8_t));

  attribute_resolver_clear_resolution_listener_Expect(
    endpoint_id_node,
    on_endpoint_interview_completed);
  TEST_ASSERT_NOT_NULL(on_endpoint_interview_completed);
  on_endpoint_interview_completed(endpoint_id_node);
}

void test_zwave_command_class_basic_actuator_in_secure_nif()
{
  // Create NIFs under the endpoint:
  attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);
  attribute_store_node_t secure_nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_SECURE_NIF, endpoint_id_node);

  zwave_command_handler_controls_IgnoreAndReturn(true);

  // Use the received_frame buffer to fill the NIF data.
  received_frame[0] = COMMAND_CLASS_SECURITY_2;
  received_frame[1] = COMMAND_CLASS_SUPERVISION;
  received_frame[2] = COMMAND_CLASS_MULTI_CHANNEL_V4;
  received_frame[3] = COMMAND_CLASS_ZWAVEPLUS_INFO_V2;
  received_frame[4] = COMMAND_CLASS_SWITCH_BINARY_V2;
  attribute_store_set_reported(secure_nif_node,
                               received_frame,
                               5 * sizeof(uint8_t));

  attribute_resolver_clear_resolution_listener_Expect(
    endpoint_id_node,
    on_endpoint_interview_completed);
  TEST_ASSERT_NOT_NULL(on_endpoint_interview_completed);
  on_endpoint_interview_completed(endpoint_id_node);
}

void test_zwave_command_class_basic_filtered_for_root()
{
  // Create NIFs under the endpoint:
  attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);
  attribute_store_node_t secure_nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_SECURE_NIF, endpoint_id_node);

  attribute_store_node_t multi_channel_version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_MULTI_CHANNEL_V4),
    endpoint_id_node);
  // Multi Channel v4
  u8_value = 4;
  attribute_store_set_reported(multi_channel_version_node,
                               &u8_value,
                               sizeof(u8_value));
  // Endpoint 0
  u8_value = 0;
  attribute_store_set_reported(endpoint_id_node, &u8_value, sizeof(u8_value));

  zwave_command_handler_controls_IgnoreAndReturn(true);

  // Use the received_frame buffer to fill the NIF data.
  received_frame[0] = COMMAND_CLASS_SECURITY_2;
  received_frame[1] = COMMAND_CLASS_SUPERVISION;
  received_frame[2] = COMMAND_CLASS_MULTI_CHANNEL_V4;
  received_frame[3] = COMMAND_CLASS_ZWAVEPLUS_INFO_V2;
  attribute_store_set_reported(secure_nif_node,
                               received_frame,
                               4 * sizeof(uint8_t));

  attribute_resolver_clear_resolution_listener_Expect(
    endpoint_id_node,
    on_endpoint_interview_completed);
  TEST_ASSERT_NOT_NULL(on_endpoint_interview_completed);
  on_endpoint_interview_completed(endpoint_id_node);
}

void test_zwave_command_class_basic_probe_no_probe_node_paused()
{
  // Create NIFs under the endpoint:
  attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);
  attribute_store_node_t secure_nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_SECURE_NIF, endpoint_id_node);

  zwave_command_handler_controls_IgnoreAndReturn(true);

  // Use the received_frame buffer to fill the NIF data.
  received_frame[0] = COMMAND_CLASS_SECURITY_2;
  received_frame[1] = COMMAND_CLASS_SUPERVISION;
  received_frame[2] = COMMAND_CLASS_MULTI_CHANNEL_V4;
  received_frame[3] = COMMAND_CLASS_ZWAVEPLUS_INFO_V2;
  attribute_store_set_reported(secure_nif_node,
                               received_frame,
                               4 * sizeof(uint8_t));

  attribute_resolver_clear_resolution_listener_Expect(
    endpoint_id_node,
    on_endpoint_interview_completed);

  // node is paused, reset the resolution listener
  is_node_or_parent_paused_ExpectAndReturn(endpoint_id_node, true);
  attribute_resolver_set_resolution_listener_Expect(
    endpoint_id_node,
    on_endpoint_interview_completed);

  TEST_ASSERT_NOT_NULL(on_endpoint_interview_completed);
  on_endpoint_interview_completed(endpoint_id_node);
}

void test_zwave_command_class_basic_probe_double_probe_protection()
{
  // Create NIFs under the endpoint:
  attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);
  attribute_store_node_t secure_nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_SECURE_NIF, endpoint_id_node);

  // Make as if we requested Basic Get already
  attribute_store_node_t basic_probe_node
    = attribute_store_add_node(ATTRIBUTE(PROBE_STATUS), endpoint_id_node);
  u32_value = 2;
  attribute_store_set_reported(basic_probe_node, &u32_value, sizeof(u32_value));

  zwave_command_handler_controls_IgnoreAndReturn(true);

  // Use the received_frame buffer to fill the NIF data.
  received_frame[0] = COMMAND_CLASS_SECURITY_2;
  received_frame[1] = COMMAND_CLASS_SUPERVISION;
  received_frame[2] = COMMAND_CLASS_MULTI_CHANNEL_V4;
  received_frame[3] = COMMAND_CLASS_ZWAVEPLUS_INFO_V2;
  attribute_store_set_reported(secure_nif_node,
                               received_frame,
                               4 * sizeof(uint8_t));

  attribute_resolver_clear_resolution_listener_Expect(
    endpoint_id_node,
    on_endpoint_interview_completed);

  is_node_or_parent_paused_ExpectAndReturn(endpoint_id_node, false);

  TEST_ASSERT_NOT_NULL(on_endpoint_interview_completed);
  on_endpoint_interview_completed(endpoint_id_node);
}

void test_zwave_command_class_basic_probe_happy_case()
{
  // Create NIFs under the endpoint:
  attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);
  attribute_store_node_t secure_nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_SECURE_NIF, endpoint_id_node);

  zwave_command_handler_controls_IgnoreAndReturn(true);

  // Use the received_frame buffer to fill the NIF data.
  received_frame[0] = COMMAND_CLASS_SECURITY_2;
  received_frame[1] = COMMAND_CLASS_SUPERVISION;
  received_frame[2] = COMMAND_CLASS_MULTI_CHANNEL_V4;
  received_frame[3] = COMMAND_CLASS_ZWAVEPLUS_INFO_V2;
  attribute_store_set_reported(secure_nif_node,
                               received_frame,
                               4 * sizeof(uint8_t));

  attribute_resolver_clear_resolution_listener_Expect(
    endpoint_id_node,
    on_endpoint_interview_completed);

  is_node_or_parent_paused_ExpectAndReturn(endpoint_id_node, false);

  zwave_tx_scheme_get_node_connection_info_Ignore();
  zwave_tx_send_data_IgnoreAndReturn(SL_STATUS_OK);

  TEST_ASSERT_NOT_NULL(on_endpoint_interview_completed);
  on_endpoint_interview_completed(endpoint_id_node);

  // Check the Probe status
  attribute_store_node_t basic_probe_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(PROBE_STATUS),
                                             0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, basic_probe_node);
  attribute_store_get_reported(basic_probe_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(1, u32_value);  // Requested
}

void test_zwave_command_class_basic_handle_report_not_requested()
{
  // Incoming reports must be rejected by default.
  TEST_ASSERT_NOT_NULL(basic_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  const uint32_t current_value          = 99;
  const uint32_t target_value           = 0;
  const uint32_t duration               = 10;
  const uint8_t incoming_report_frame[] = {COMMAND_CLASS_BASIC_V2,
                                           BASIC_REPORT_V2,
                                           (uint8_t)current_value,
                                           (uint8_t)target_value,
                                           (uint8_t)duration};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    basic_handler.control_handler(&connection_info,
                                  incoming_report_frame,
                                  sizeof(incoming_report_frame)));

  // No attribute must have been created under the endpoint_id.
  attribute_store_node_t value_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(VALUE),
                                             0);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, value_node);

  attribute_store_node_t duration_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(DURATION),
                                             0);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, duration_node);
}

void test_zwave_command_class_basic_handle_report_too_short()
{
  // Too short report will do nothing.
  TEST_ASSERT_NOT_NULL(basic_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_BASIC_V2, BASIC_REPORT_V2};

  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    basic_handler.control_handler(&connection_info,
                                  incoming_report_frame,
                                  sizeof(incoming_report_frame)));
}

void test_zwave_command_class_basic_handle_unknown_command()
{
  TEST_ASSERT_NOT_NULL(basic_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  const uint8_t incoming_report_frame[] = {COMMAND_CLASS_BASIC_V2, 0xAA};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    basic_handler.control_handler(&connection_info,
                                  incoming_report_frame,
                                  sizeof(incoming_report_frame)));
}

void test_zwave_command_class_basic_handle_first_report()
{
  // Make us expect a report
  attribute_store_node_t basic_probe_node
    = attribute_store_add_node(ATTRIBUTE(PROBE_STATUS), endpoint_id_node);
  u32_value = 1;  // REQUESTED
  attribute_store_set_reported(basic_probe_node, &u32_value, sizeof(u32_value));

  TEST_ASSERT_NOT_NULL(basic_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  const uint32_t current_value          = 99;
  const uint32_t target_value           = 0;
  const uint32_t duration               = 0;
  const uint8_t incoming_report_frame[] = {COMMAND_CLASS_BASIC_V2,
                                           BASIC_REPORT_V2,
                                           (uint8_t)current_value,
                                           (uint8_t)target_value,
                                           (uint8_t)duration};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    basic_handler.control_handler(&connection_info,
                                  incoming_report_frame,
                                  sizeof(incoming_report_frame)));

  // Check the attribute store updates
  // Value
  attribute_store_node_t value_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(VALUE),
                                             0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, value_node);
  attribute_store_get_reported(value_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(0, u32_value);  // Current is OFF due to 0 duration
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(value_node, DESIRED_ATTRIBUTE));

  // Duration
  attribute_store_node_t duration_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(DURATION),
                                             0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, duration_node);
  attribute_store_get_reported(duration_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(0, u32_value);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(duration_node, DESIRED_ATTRIBUTE));

  // Probe status
  attribute_store_get_reported(basic_probe_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(2, u32_value);  //ANSWERED
}

void test_zwave_command_class_basic_handle_report_v1()
{
  // Make us expect a report
  attribute_store_node_t basic_probe_node
    = attribute_store_add_node(ATTRIBUTE(PROBE_STATUS), endpoint_id_node);
  u32_value = 1;  // ANSWERED
  attribute_store_set_reported(basic_probe_node, &u32_value, sizeof(u32_value));

  TEST_ASSERT_NOT_NULL(basic_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  const uint32_t current_value = 0;
  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_BASIC_V2, BASIC_REPORT_V2, (uint8_t)current_value};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    basic_handler.control_handler(&connection_info,
                                  incoming_report_frame,
                                  sizeof(incoming_report_frame)));

  // Check the attribute store updates
  // Value
  attribute_store_node_t value_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(VALUE),
                                             0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, value_node);
  attribute_store_get_reported(value_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(0, u32_value);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(value_node, DESIRED_ATTRIBUTE));

  // Duration
  attribute_store_node_t duration_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(DURATION),
                                             0);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, duration_node);

  // Probe status
  attribute_store_get_reported(basic_probe_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(2, u32_value);  //ANSWERED
}

void test_zwave_command_class_basic_handle_report_v2()
{
  // Make us expect a report
  attribute_store_node_t basic_probe_node
    = attribute_store_add_node(ATTRIBUTE(PROBE_STATUS), endpoint_id_node);
  u32_value = 1;  // ANSWERED
  attribute_store_set_reported(basic_probe_node, &u32_value, sizeof(u32_value));

  TEST_ASSERT_NOT_NULL(basic_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  const uint32_t current_value          = 99;
  const uint32_t target_value           = 25;
  const uint32_t duration               = 23;
  const uint8_t incoming_report_frame[] = {COMMAND_CLASS_BASIC_V2,
                                           BASIC_REPORT_V2,
                                           (uint8_t)current_value,
                                           (uint8_t)target_value,
                                           (uint8_t)duration};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    basic_handler.control_handler(&connection_info,
                                  incoming_report_frame,
                                  sizeof(incoming_report_frame)));

  // Check the attribute store updates
  // Value
  attribute_store_node_t value_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(VALUE),
                                             0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, value_node);
  attribute_store_get_reported(value_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(0xFF, u32_value);
  attribute_store_get_desired(value_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(0xFF, u32_value);
  ;

  // Duration
  attribute_store_node_t duration_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(DURATION),
                                             0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, duration_node);
  attribute_store_get_reported(duration_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(duration, u32_value);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(duration_node, DESIRED_ATTRIBUTE));

  // Probe status
  attribute_store_get_reported(basic_probe_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(2, u32_value);  //ANSWERED
}

void test_zwave_command_class_basic_handle_report_weird_length()
{
  // Make us expect a report
  attribute_store_node_t basic_probe_node
    = attribute_store_add_node(ATTRIBUTE(PROBE_STATUS), endpoint_id_node);
  u32_value = 1;  // ANSWERED
  attribute_store_set_reported(basic_probe_node, &u32_value, sizeof(u32_value));

  TEST_ASSERT_NOT_NULL(basic_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  const uint32_t current_value          = 99;
  const uint32_t target_value           = 0;
  const uint8_t incoming_report_frame[] = {COMMAND_CLASS_BASIC_V2,
                                           BASIC_REPORT_V2,
                                           (uint8_t)current_value,
                                           (uint8_t)target_value};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    basic_handler.control_handler(&connection_info,
                                  incoming_report_frame,
                                  sizeof(incoming_report_frame)));

  // Check the attribute store updates
  // Value
  attribute_store_node_t value_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(VALUE),
                                             0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, value_node);
  attribute_store_get_reported(value_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(0, u32_value);
  attribute_store_get_desired(value_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(0, u32_value);
  ;

  // Duration
  attribute_store_node_t duration_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(DURATION),
                                             0);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, duration_node);

  // Probe status
  attribute_store_get_reported(basic_probe_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(2, u32_value);  //ANSWERED
}

void test_zwave_command_class_basic_publish_incoming_set_too_short()
{
  TEST_ASSERT_NOT_NULL(basic_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  // Just receive a Basic Set:
  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_BASIC_V2, BASIC_SET_V2};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    basic_handler.control_handler(&connection_info,
                                  incoming_report_frame,
                                  sizeof(incoming_report_frame)));
}

void test_zwave_command_class_basic_publish_incoming_set_on()
{
  TEST_ASSERT_NOT_NULL(basic_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  // Just receive a Basic Set:
  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_BASIC_V2, BASIC_SET_V2, 0xAA};

  // Push it to MQTT
  uic_mqtt_dotdot_on_off_supported_commands_t expected_commands
    = {.on = true, .off = true};
  uic_mqtt_dotdot_on_off_publish_supported_generated_commands_ExpectWithArray(
    NULL,
    connection_info.remote.endpoint_id,
    &expected_commands,
    sizeof(expected_commands));
  uic_mqtt_dotdot_on_off_publish_supported_generated_commands_IgnoreArg_unid();
  uic_mqtt_dotdot_on_off_publish_generated_on_command_Expect(
    NULL,
    connection_info.remote.endpoint_id);
  uic_mqtt_dotdot_on_off_publish_generated_on_command_IgnoreArg_unid();

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    basic_handler.control_handler(&connection_info,
                                  incoming_report_frame,
                                  sizeof(incoming_report_frame)));
}

void test_zwave_command_class_basic_publish_incoming_set_off()
{
  TEST_ASSERT_NOT_NULL(basic_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  // Just receive a Basic Set:
  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_BASIC_V2, BASIC_SET_V2, 0x00};

  // Push it to MQTT
  uic_mqtt_dotdot_on_off_supported_commands_t expected_commands
    = {.on = true, .off = true};
  uic_mqtt_dotdot_on_off_publish_supported_generated_commands_ExpectWithArray(
    NULL,
    connection_info.remote.endpoint_id,
    &expected_commands,
    sizeof(expected_commands));
  uic_mqtt_dotdot_on_off_publish_supported_generated_commands_IgnoreArg_unid();
  uic_mqtt_dotdot_on_off_publish_generated_off_command_Expect(
    NULL,
    connection_info.remote.endpoint_id);
  uic_mqtt_dotdot_on_off_publish_generated_off_command_IgnoreArg_unid();

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    basic_handler.control_handler(&connection_info,
                                  incoming_report_frame,
                                  sizeof(incoming_report_frame)));
}