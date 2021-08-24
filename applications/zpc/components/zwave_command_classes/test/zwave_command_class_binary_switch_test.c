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
#include "zwave_command_class_binary_switch.h"
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

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_BINARY_SWITCH_##type

// Static variables
static attribute_resolver_function_t binary_set                    = NULL;
static attribute_resolver_function_t binary_get                    = NULL;
static zpc_resolver_event_notification_function_t on_send_complete = NULL;
static zwave_command_handler_t binary_handler                      = {};

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
  if (node_type != ATTRIBUTE(STATE)) {
    TEST_FAIL_MESSAGE("Attribute rule registration on a wrong type");
  }

  if (node_type == ATTRIBUTE(STATE)) {
    binary_set = set_func;
    binary_get = get_func;
  }
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  binary_handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    binary_handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SWITCH_BINARY_V2,
                    binary_handler.command_class);
  TEST_ASSERT_EQUAL(SWITCH_BINARY_VERSION_V2, binary_handler.version);
  TEST_ASSERT_NOT_NULL(binary_handler.control_handler);
  TEST_ASSERT_NULL(binary_handler.support_handler);
  TEST_ASSERT_FALSE(binary_handler.manual_security_validation);

  return SL_STATUS_OK;
}

static sl_status_t register_send_event_handler_stub(
  attribute_store_type_t type,
  zpc_resolver_event_notification_function_t function,
  int cmock_num_calls)
{
  on_send_complete = function;
  TEST_ASSERT_NOT_NULL(on_send_complete);
  TEST_ASSERT_EQUAL(ATTRIBUTE(STATE), type);
  return SL_STATUS_OK;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  binary_set       = NULL;
  binary_get       = NULL;
  on_send_complete = NULL;
  memset(&binary_handler, 0, sizeof(zwave_command_handler_t));

  datastore_init(":memory:");
  attribute_store_init();
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_set_home_id(home_id);
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

static void zwave_command_class_switch_binary_init_verification()
{
  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  // On send complete handler
  register_send_event_handler_Stub(&register_send_event_handler_stub);

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_binary_switch_init());
}

/// Called before each and every test
void setUp()
{
  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size = 0;
  u8_value            = 0;
  u32_value           = 0;

  zwave_command_class_switch_binary_init_verification();
}

void test_zwave_command_class_binary_switch_new_supporting_node()
{
  // Add the version node:
  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(endpoint_id_node));
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);

  // No new attribute created due to unresolved version.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  u8_value = 0;
  attribute_store_set_reported(version_node, &u8_value, sizeof(u8_value));
  // No new attribute created due to version = 0.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  u8_value = 1;
  attribute_store_set_reported(version_node, &u8_value, sizeof(u8_value));
  // STATE Attribute got created:
  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(endpoint_id_node));

  // Small check on the VALUE Attribute :
  attribute_store_node_t state_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(STATE),
                                             0);

  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(state_node));

  // Now version 3 comes in:
  u8_value = 3;
  attribute_store_set_reported(version_node, &u8_value, sizeof(u8_value));

  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(state_node));
}

void test_zwave_command_class_switch_binary_probe_state_v2_report()
{
  // First check the frame creation:
  TEST_ASSERT_NOT_NULL(binary_get);

  binary_get(ATTRIBUTE_STORE_INVALID_NODE,
             received_frame,
             &received_frame_size);
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_SWITCH_BINARY_V2, SWITCH_BINARY_GET_V2};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);

  // Then simulate an incoming report
  TEST_ASSERT_NOT_NULL(binary_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  connection_info.remote.node_id                     = node_id;
  connection_info.remote.endpoint_id                 = endpoint_id;
  connection_info.local.is_multicast                 = false;

  const uint32_t current_value          = 0xFF;
  const uint32_t duration               = 0x10;
  const uint32_t target_value           = 0x00;
  const uint8_t incoming_report_frame[] = {COMMAND_CLASS_SWITCH_BINARY_V2,
                                           SWITCH_BINARY_REPORT_V2,
                                           (uint8_t)current_value,
                                           (uint8_t)target_value,
                                           (uint8_t)duration};

  attribute_store_node_t state_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(STATE),
                                             0);
  attribute_store_node_t value_node
    = attribute_store_get_node_child_by_type(state_node, ATTRIBUTE(VALUE), 0);

  attribute_store_node_t duration_node
    = attribute_store_get_node_child_by_type(state_node,
                                             ATTRIBUTE(DURATION),
                                             0);

  attribute_store_undefine_reported(state_node);
  attribute_store_undefine_desired(state_node);
  attribute_store_undefine_reported(value_node);
  attribute_store_undefine_desired(value_node);
  attribute_store_undefine_reported(duration_node);
  attribute_store_undefine_desired(duration_node);

  is_node_pending_set_resolution_ExpectAndReturn(state_node, false);
  is_node_pending_set_resolution_ExpectAndReturn(state_node, true);
  attribute_resolver_restart_set_resolution_ExpectAndReturn(state_node,
                                                            SL_STATUS_OK);
  binary_handler.control_handler(&connection_info,
                                 incoming_report_frame,
                                 sizeof(incoming_report_frame));

  // Verify that the data is according to the report frame
  attribute_store_get_reported(value_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(0xFF, u32_value);

  attribute_store_get_desired(value_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(target_value, u32_value);

  attribute_store_get_reported(duration_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(duration, u32_value);

  attribute_store_get_desired(duration_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(duration, u32_value);

  // State is set to FINAL_STATE
  attribute_store_get_reported(state_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(0x00, u32_value);
  attribute_store_get_desired(state_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(0x00, u32_value);
}

void test_zwave_command_class_switch_binary_probe_state_v1_report()
{
  // First check the frame creation:
  TEST_ASSERT_NOT_NULL(binary_get);

  binary_get(ATTRIBUTE_STORE_INVALID_NODE,
             received_frame,
             &received_frame_size);
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_SWITCH_BINARY_V2, SWITCH_BINARY_GET_V2};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);

  // Then simulate an incoming report
  TEST_ASSERT_NOT_NULL(binary_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  connection_info.remote.node_id                     = node_id;
  connection_info.remote.endpoint_id                 = endpoint_id;
  connection_info.local.is_multicast                 = false;

  const uint32_t current_value          = 99;  // Quirky value
  const uint8_t incoming_report_frame[] = {COMMAND_CLASS_SWITCH_BINARY_V2,
                                           SWITCH_BINARY_REPORT_V2,
                                           (uint8_t)current_value};

  attribute_store_node_t state_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(STATE),
                                             0);
  attribute_store_node_t value_node
    = attribute_store_get_node_child_by_type(state_node, ATTRIBUTE(VALUE), 0);

  attribute_store_node_t duration_node
    = attribute_store_get_node_child_by_type(state_node,
                                             ATTRIBUTE(DURATION),
                                             0);

  attribute_store_undefine_reported(state_node);
  attribute_store_undefine_desired(state_node);
  attribute_store_undefine_reported(value_node);
  attribute_store_undefine_desired(value_node);
  attribute_store_undefine_reported(duration_node);
  attribute_store_undefine_desired(duration_node);

  is_node_pending_set_resolution_ExpectAndReturn(state_node, false);
  is_node_pending_set_resolution_ExpectAndReturn(state_node, false);

  binary_handler.control_handler(&connection_info,
                                 incoming_report_frame,
                                 sizeof(incoming_report_frame));

  // Verify that the data is according to the report frame
  attribute_store_get_reported(value_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(0xFF, u32_value);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(value_node, DESIRED_ATTRIBUTE));

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(duration_node, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(duration_node, DESIRED_ATTRIBUTE));

  // State is set to FINAL_STATE
  attribute_store_get_reported(state_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(0x00, u32_value);
  attribute_store_get_desired(state_node, &u32_value, sizeof(u32_value));
  TEST_ASSERT_EQUAL(0x00, u32_value);
}

void test_zwave_command_class_switch_binary_set_state()
{
  // First check the frame creation.
  TEST_ASSERT_NOT_NULL(binary_set);

  attribute_store_node_t state_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(STATE),
                                             0);

  attribute_store_node_t value_node
    = attribute_store_get_node_child_by_type(state_node, ATTRIBUTE(VALUE), 0);

  uint32_t desired_value = 23;
  attribute_store_set_desired(value_node,
                              &desired_value,
                              sizeof(desired_value));

  attribute_store_node_t duration_node
    = attribute_store_get_node_child_by_type(state_node,
                                             ATTRIBUTE(DURATION),
                                             0);

  uint32_t desired_duration = 34;
  // here we test that it picks reported if there is no desired
  attribute_store_undefine_desired(duration_node);
  attribute_store_set_reported(duration_node,
                               &desired_duration,
                               sizeof(desired_duration));

  binary_set(state_node, received_frame, &received_frame_size);
  const uint8_t expected_frame[] = {COMMAND_CLASS_SWITCH_BINARY_V2,
                                    SWITCH_BINARY_SET_V2,
                                    (uint8_t)desired_value,
                                    (uint8_t)desired_duration};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);

  // Now simulate that we get a Supervision Working:
  TEST_ASSERT_NOT_NULL(on_send_complete);
  on_send_complete(state_node,
                   RESOLVER_SET_RULE,
                   FRAME_SENT_EVENT_OK_SUPERVISION_WORKING);

  // Now simulate that we get a Supervision OK:
  TEST_ASSERT_NOT_NULL(on_send_complete);
  on_send_complete(state_node,
                   RESOLVER_SET_RULE,
                   FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS);
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(state_node, REPORTED_ATTRIBUTE));

  // Now simulate that we get a Supervision Fail:
  TEST_ASSERT_NOT_NULL(on_send_complete);
  on_send_complete(state_node,
                   RESOLVER_SET_RULE,
                   FRAME_SENT_EVENT_OK_SUPERVISION_FAIL);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(state_node, REPORTED_ATTRIBUTE));

  // Now simulate that we get a send data Ok no supervision:
  TEST_ASSERT_NOT_NULL(on_send_complete);
  on_send_complete(state_node,
                   RESOLVER_SET_RULE,
                   FRAME_SENT_EVENT_OK_NO_SUPERVISION);
}
