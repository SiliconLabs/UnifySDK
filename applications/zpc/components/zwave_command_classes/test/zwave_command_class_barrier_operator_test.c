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
#include "zwave_command_class_barrier_operator.h"
#include "zwave_command_classes_utils.h"
#include "unity.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "zpc_attribute_store_network_helper.h"
#include "zpc_attribute_store_type_registration.h"
#include "sl_log.h"
#include "dotdot_mqtt_mock.h"
#include "dotdot_mqtt_generated_commands_mock.h"

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
#include "zwave_network_management_mock.h"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_BARRIER_OPERATOR_##type
#define LOG_TAG         "zwave_command_class_barrier_operator_test"

// Static variables
static attribute_resolver_function_t barrier_operator_get           = NULL;
static attribute_resolver_function_t barrier_operator_set           = NULL;
static attribute_resolver_function_t barrier_operator_supported_get = NULL;
static attribute_resolver_function_t barrier_operator_signal_get    = NULL;
static attribute_resolver_function_t barrier_operator_signal_set    = NULL;

static zwave_command_handler_t barrier_operator_handler   = {};
static zwave_controller_connection_info_t connection_info = {};

static uint8_t received_frame[255]  = {};
static uint16_t received_frame_size = 0;

// Stub functions
static sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  if (node_type == ATTRIBUTE(BITMASK)) {
    barrier_operator_supported_get = get_func;
  } else if (node_type == ATTRIBUTE(SUBSYSTEM_STATE)) {
    barrier_operator_signal_get = get_func;
    barrier_operator_signal_set = set_func;

  } else if (node_type == ATTRIBUTE(STATE)) {
    barrier_operator_get = get_func;
    barrier_operator_set = set_func;

  } else {
    TEST_FAIL_MESSAGE("Attribute rule registration on a wrong type");
  }
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  barrier_operator_handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    barrier_operator_handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_BARRIER_OPERATOR,
                    barrier_operator_handler.command_class);
  TEST_ASSERT_EQUAL(BARRIER_OPERATOR_VERSION, barrier_operator_handler.version);
  TEST_ASSERT_NOT_NULL(barrier_operator_handler.control_handler);
  TEST_ASSERT_NULL(barrier_operator_handler.support_handler);
  TEST_ASSERT_FALSE(barrier_operator_handler.manual_security_validation);

  return SL_STATUS_OK;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  zpc_attribute_store_register_known_attribute_types();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

static void zwave_command_class_barrier_operator_init_verification()
{
  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_barrier_operator_init());
}

/// Called before each and every test
void setUp()
{
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_set_home_id(home_id);
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);

  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size            = 0;
  barrier_operator_get           = NULL;
  barrier_operator_set           = NULL;
  barrier_operator_supported_get = NULL;
  barrier_operator_signal_get    = NULL;
  barrier_operator_signal_set    = NULL;
  memset(&barrier_operator_handler, 0, sizeof(zwave_command_handler_t));

  zwave_command_class_barrier_operator_init_verification();

  // Prepare a default connection info with the supporting node
  memset(&connection_info, 0, sizeof(zwave_controller_connection_info_t));
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));
}

/// Called after each and every test
void tearDown() {}

void test_zwave_command_class_barrier_operator_get_happy_case()
{
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_BARRIER_OPERATOR),
    endpoint_id_node);

  // Version is undefined, no attributes created just yet.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));
  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));

  uint8_t state = 3;
  attribute_store_node_t state_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(STATE),
                                             0);

  attribute_store_get_reported(state_node, &state, sizeof(state));

  uint8_t type = 0;
  attribute_store_node_t type_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SUBSYSTEM_TYPE),
                                             0);
  attribute_store_get_reported(type_node, &type, sizeof(type));

  TEST_ASSERT_EQUAL(0, type);

  uint8_t subsystem_state = 0;
  attribute_store_node_t subsystem_state_node
    = attribute_store_get_node_child_by_type(type_node,
                                             ATTRIBUTE(SUBSYSTEM_STATE),
                                             0);
  attribute_store_get_reported(subsystem_state_node,
                               &subsystem_state,
                               sizeof(subsystem_state));

  TEST_ASSERT_EQUAL(0, subsystem_state);

  // Now ask to generate a command
  TEST_ASSERT_NOT_NULL(barrier_operator_get);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    barrier_operator_get(state_node, received_frame, &received_frame_size));
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_BARRIER_OPERATOR, BARRIER_OPERATOR_GET};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_barrier_operator_signal_supported_get_happy_case()
{
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_BARRIER_OPERATOR),
    endpoint_id_node);

  // Version is undefined, no attributes created just yet.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));
  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));

  uint8_t state = 0;
  attribute_store_node_t state_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(STATE),
                                             0);
  attribute_store_get_reported(state_node, &state, sizeof(state));

  TEST_ASSERT_EQUAL(0, state);

  uint8_t bitmask[] = {0};
  attribute_store_node_t bitmask_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(BITMASK),
                                             0);
  attribute_store_get_reported(bitmask_node, &bitmask, sizeof(bitmask));

  // Now ask to generate a command
  TEST_ASSERT_NOT_NULL(barrier_operator_supported_get);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    barrier_operator_supported_get(bitmask_node,
                                                   received_frame,
                                                   &received_frame_size));
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_BARRIER_OPERATOR, BARRIER_OPERATOR_SIGNAL_SUPPORTED_GET};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_barrier_operator_signal_get_happy_case()
{
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_BARRIER_OPERATOR),
    endpoint_id_node);

  // Version is undefined, no attributes created just yet.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));
  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));

  uint8_t state = 0;
  attribute_store_node_t state_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(STATE),
                                             0);
  attribute_store_get_reported(state_node, &state, sizeof(state));

  TEST_ASSERT_EQUAL(0, state);

  uint8_t type = 1;

  attribute_store_node_t type_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(SUBSYSTEM_TYPE),
                              &type,
                              sizeof(type));
  attribute_store_node_t subsystem_state_node
    = attribute_store_add_node(ATTRIBUTE(SUBSYSTEM_STATE), type_node);

  // Now ask to generate a command
  TEST_ASSERT_NOT_NULL(barrier_operator_signal_get);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    barrier_operator_signal_get(subsystem_state_node,
                                                received_frame,
                                                &received_frame_size));
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_BARRIER_OPERATOR, BARRIER_OPERATOR_SIGNAL_GET, type};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_barrier_operator_signal_get_invalid_type()
{
  uint8_t type = 0;
  attribute_store_node_t type_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(SUBSYSTEM_TYPE),
                              &type,
                              sizeof(type));
  attribute_store_node_t subsystem_state_node
    = attribute_store_add_node(ATTRIBUTE(SUBSYSTEM_STATE), type_node);

  // Now ask to generate a command
  TEST_ASSERT_NOT_NULL(barrier_operator_signal_get);
  TEST_ASSERT_EQUAL(SL_STATUS_ALREADY_EXISTS,
                    barrier_operator_signal_get(subsystem_state_node,
                                                received_frame,
                                                &received_frame_size));
}

void test_zwave_command_class_barrier_operator_report_happy_case()
{
  // Simulate a version 1 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_BARRIER_OPERATOR),
    endpoint_id_node);

  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  uint8_t state = 255;
  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));
  attribute_store_node_t state_node = attribute_store_emplace(endpoint_id_node,
                                                              ATTRIBUTE(STATE),
                                                              &state,
                                                              sizeof(state));
  uint8_t incoming_frame[]
    = {COMMAND_CLASS_BARRIER_OPERATOR, BARRIER_OPERATOR_REPORT, state & 0xFF};

  TEST_ASSERT_NOT_NULL(barrier_operator_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    barrier_operator_handler.control_handler(&connection_info,
                                             incoming_frame,
                                             sizeof(incoming_frame)));

  state_node          = attribute_store_get_node_child_by_type(endpoint_id_node,
                                                      ATTRIBUTE(STATE),
                                                      0);
  uint8_t state_value = 0;
  attribute_store_get_reported(state_node, &state_value, sizeof(state_value));
  TEST_ASSERT_EQUAL(state, state_value);
}

void test_zwave_command_class_barrier_operator_supported_report_happy_case()
{
  // Simulate a version 1 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_BARRIER_OPERATOR),
    endpoint_id_node);

  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  uint8_t bitmask[2] = {01, 02};

  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));

  uint8_t incoming_frame[] = {COMMAND_CLASS_BARRIER_OPERATOR,
                              BARRIER_OPERATOR_SIGNAL_SUPPORTED_REPORT,
                              bitmask[0],
                              bitmask[1]};

  TEST_ASSERT_NOT_NULL(barrier_operator_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    barrier_operator_handler.control_handler(&connection_info,
                                             incoming_frame,
                                             sizeof(incoming_frame)));

  attribute_store_node_t bitmask_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(BITMASK),
                                             0);
  // Check if the 2 type nodes created
  uint8_t bitmask_replied[] = {0, 0};
  attribute_store_get_reported(bitmask_node,
                               &bitmask_replied,
                               sizeof(bitmask_replied));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(bitmask, bitmask_replied, 2);

  attribute_store_node_t node_type
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SUBSYSTEM_TYPE),
                                             0);

  uint8_t type_value = 0;
  attribute_store_get_reported(node_type, &type_value, sizeof(type_value));
  TEST_ASSERT_EQUAL(1, type_value);

  node_type  = attribute_store_get_node_child_by_type(endpoint_id_node,
                                                     ATTRIBUTE(SUBSYSTEM_TYPE),
                                                     1);
  type_value = 0;
  attribute_store_get_reported(node_type, &type_value, sizeof(type_value));
  TEST_ASSERT_EQUAL(10, type_value);
}

void test_zwave_command_class_barrier_operator_signal_report_happy_case()
{
  uint8_t subsystem_type  = 1;
  uint8_t subsystem_state = 0;

  attribute_store_node_t type_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(SUBSYSTEM_TYPE),
                              &subsystem_type,
                              sizeof(subsystem_type));
  const attribute_store_type_t type_attribute[] = {ATTRIBUTE(SUBSYSTEM_STATE)};

  attribute_store_add_if_missing(type_node,
                                 type_attribute,
                                 COUNT_OF(type_attribute));

  uint8_t incoming_frame[] = {COMMAND_CLASS_BARRIER_OPERATOR,
                              BARRIER_OPERATOR_SIGNAL_REPORT,
                              subsystem_type & 0xFF,
                              subsystem_state & 0xFF};

  TEST_ASSERT_NOT_NULL(barrier_operator_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    barrier_operator_handler.control_handler(&connection_info,
                                             incoming_frame,
                                             sizeof(incoming_frame)));

  attribute_store_node_t subsystem_type_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SUBSYSTEM_TYPE),
                                             0);
  // Check if the 2 type nodes created
  uint8_t subsystem_t = 0;
  attribute_store_get_reported(subsystem_type_node,
                               &subsystem_t,
                               sizeof(subsystem_t));
  TEST_ASSERT_EQUAL(subsystem_type, subsystem_t);

  attribute_store_node_t subsystem_state_node
    = attribute_store_get_node_child_by_type(subsystem_type_node,
                                             ATTRIBUTE(SUBSYSTEM_STATE),
                                             0);

  uint8_t state_value = 0;
  attribute_store_get_reported(subsystem_state_node,
                               &state_value,
                               sizeof(state_value));
  TEST_ASSERT_EQUAL(subsystem_state, state_value);
}

void test_zwave_command_class_barrier_operator_set_happy_case()
{
  attribute_store_node_t state_node
    = attribute_store_add_node(ATTRIBUTE(STATE), endpoint_id_node);
  uint8_t state = 255;
  attribute_store_set_desired(state_node, &state, sizeof(state));

  TEST_ASSERT_NOT_NULL(barrier_operator_set);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    barrier_operator_set(state_node, received_frame, &received_frame_size));
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_BARRIER_OPERATOR, BARRIER_OPERATOR_SET, state};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_barrier_operator_signal_set_happy_case()
{
  uint8_t subsystem_state = 255;
  uint8_t subsystem_type  = 1;
  attribute_store_node_t type_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(SUBSYSTEM_TYPE),
                              &subsystem_type,
                              sizeof(subsystem_type));
  const attribute_store_type_t type_attribute[] = {ATTRIBUTE(SUBSYSTEM_STATE)};

  attribute_store_add_if_missing(type_node,
                                 type_attribute,
                                 COUNT_OF(type_attribute));

  attribute_store_node_t state_node
    = attribute_store_get_node_child_by_type(type_node,
                                             ATTRIBUTE(SUBSYSTEM_STATE),
                                             0);

  attribute_store_set_desired(state_node,
                              &subsystem_state,
                              sizeof(subsystem_state));
  TEST_ASSERT_NOT_NULL(barrier_operator_signal_set);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    barrier_operator_signal_set(state_node,
                                                received_frame,
                                                &received_frame_size));
  const uint8_t expected_frame[] = {COMMAND_CLASS_BARRIER_OPERATOR,
                                    BARRIER_OPERATOR_SIGNAL_SET,
                                    subsystem_type,
                                    subsystem_state};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_barrier_operator_signal_set_invalid_type()
{
  uint8_t subsystem_state = 255;
  uint8_t subsystem_type  = 0;
  attribute_store_node_t type_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(SUBSYSTEM_TYPE),
                              &subsystem_type,
                              sizeof(subsystem_type));
  const attribute_store_type_t type_attribute[] = {ATTRIBUTE(SUBSYSTEM_STATE)};

  attribute_store_add_if_missing(type_node,
                                 type_attribute,
                                 COUNT_OF(type_attribute));

  attribute_store_node_t state_node
    = attribute_store_get_node_child_by_type(type_node,
                                             ATTRIBUTE(SUBSYSTEM_STATE),
                                             0);
  attribute_store_set_desired(state_node,
                              &subsystem_state,
                              sizeof(subsystem_state));
  TEST_ASSERT_NOT_NULL(barrier_operator_signal_set);
  TEST_ASSERT_EQUAL(SL_STATUS_ALREADY_EXISTS,
                    barrier_operator_signal_set(state_node,
                                                received_frame,
                                                &received_frame_size));

  TEST_ASSERT_FALSE(attribute_store_node_exists(state_node));
  TEST_ASSERT_FALSE(attribute_store_node_exists(type_node));
}

void test_zwave_command_class_barrier_operator_report_missing_state()
{
  // Simulate a version 1 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_BARRIER_OPERATOR),
    endpoint_id_node);

  // Version is undefined, no attributes created just yet.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));

  uint8_t incoming_frame[]
    = {COMMAND_CLASS_BARRIER_OPERATOR, BARRIER_OPERATOR_REPORT};

  TEST_ASSERT_NOT_NULL(barrier_operator_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    barrier_operator_handler.control_handler(&connection_info,
                                             incoming_frame,
                                             sizeof(incoming_frame)));
}

void test_zwave_command_class_barrier_operator_supported_report_missing_bitmask()
{
  // Simulate a version 1 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_BARRIER_OPERATOR),
    endpoint_id_node);

  // Version is undefined, no attributes created just yet.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));

  uint8_t incoming_frame[] = {COMMAND_CLASS_BARRIER_OPERATOR,
                              BARRIER_OPERATOR_SIGNAL_SUPPORTED_REPORT};

  TEST_ASSERT_NOT_NULL(barrier_operator_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    barrier_operator_handler.control_handler(&connection_info,
                                             incoming_frame,
                                             sizeof(incoming_frame)));
}
void test_zwave_command_class_barrier_operator_signal_report_missing_smaller_frame()
{
  // Simulate a version 1 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_BARRIER_OPERATOR),
    endpoint_id_node);

  // Version is undefined, no attributes created just yet.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));

  uint8_t subsystem_type   = 0x10;
  uint8_t incoming_frame[] = {COMMAND_CLASS_BARRIER_OPERATOR,
                              BARRIER_OPERATOR_SIGNAL_REPORT,
                              subsystem_type};

  TEST_ASSERT_NOT_NULL(barrier_operator_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    barrier_operator_handler.control_handler(&connection_info,
                                             incoming_frame,
                                             sizeof(incoming_frame)));

  attribute_store_node_t subsystem_type_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SUBSYSTEM_TYPE),
                                             0);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, subsystem_type_node);
}

void test_zwave_command_class_barrier_operator_unknown_command()
{
  const uint8_t incoming_frame[] = {COMMAND_CLASS_BARRIER_OPERATOR, 0x10};

  TEST_ASSERT_NOT_NULL(barrier_operator_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    barrier_operator_handler.control_handler(&connection_info,
                                             incoming_frame,
                                             sizeof(incoming_frame)));
}

void test_zwave_command_class_barrier_operator_no_command()
{
  const uint8_t incoming_frame[] = {COMMAND_CLASS_BARRIER_OPERATOR};

  TEST_ASSERT_NOT_NULL(barrier_operator_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    barrier_operator_handler.control_handler(&connection_info,
                                             incoming_frame,
                                             sizeof(incoming_frame)));
}

void test_zwave_command_class_barrier_operator_wrong_command_class()
{
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_THERMOSTAT_MODE_V3, BARRIER_OPERATOR_REPORT};

  TEST_ASSERT_NOT_NULL(barrier_operator_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    barrier_operator_handler.control_handler(&connection_info,
                                             incoming_frame,
                                             sizeof(incoming_frame)));
}

void test_zwave_command_class_barrier_operator_generated_go_to_percent_command()
{
  // Simulate an incoming Barrier Operator Set, with non-zero value
  TEST_ASSERT_NOT_NULL(barrier_operator_handler.control_handler);

  zwave_controller_connection_info_t connection_info = {};
  connection_info.remote.node_id                     = node_id;
  connection_info.remote.endpoint_id                 = endpoint_id;

  uint8_t value = 0xff;
  uic_mqtt_dotdot_barrier_control_command_go_to_percent_fields_t target
    = {.percent_open = 0xff};
  const uint8_t incoming_set_frame[]
    = {COMMAND_CLASS_BARRIER_OPERATOR, BARRIER_OPERATOR_SET, (uint8_t)value};

  uic_mqtt_dotdot_barrier_control_publish_generated_go_to_percent_command_Expect(
    NULL,
    endpoint_id,
    &target);
  uic_mqtt_dotdot_barrier_control_publish_generated_go_to_percent_command_IgnoreArg_unid();

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    barrier_operator_handler.control_handler(&connection_info,
                                             incoming_set_frame,
                                             sizeof(incoming_set_frame)));
}

void test_zwave_command_class_barrier_operator_generated_go_to_percent_command_partial()
{
  uint8_t value = 0x60;
  uic_mqtt_dotdot_barrier_control_command_go_to_percent_fields_t target
    = {.percent_open = 0x60};
  const uint8_t incoming_set_frame[]
    = {COMMAND_CLASS_BARRIER_OPERATOR, BARRIER_OPERATOR_SET, (uint8_t)value};

  uic_mqtt_dotdot_barrier_control_publish_generated_go_to_percent_command_Expect(
    NULL,
    endpoint_id,
    &target);
  uic_mqtt_dotdot_barrier_control_publish_generated_go_to_percent_command_IgnoreArg_unid();

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    barrier_operator_handler.control_handler(&connection_info,
                                             incoming_set_frame,
                                             sizeof(incoming_set_frame)));
}

void test_zwave_command_class_barrier_operator_generated_go_to_percent_command_small_frame()
{
  // Simulate an incoming Binary Switch Set that's too short
  TEST_ASSERT_NOT_NULL(barrier_operator_handler.control_handler);

  zwave_controller_connection_info_t connection_info = {};
  connection_info.remote.node_id                     = node_id;
  connection_info.remote.endpoint_id                 = endpoint_id;

  const uint8_t incoming_set_frame[]
    = {COMMAND_CLASS_BARRIER_OPERATOR, BARRIER_OPERATOR_SET};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    barrier_operator_handler.control_handler(&connection_info,
                                             incoming_set_frame,
                                             sizeof(incoming_set_frame)));
}

void test_zwave_command_class_barrier_operator_generated_go_to_percent_command_not_supported()
{
  uint8_t value = 0x64;
  const uint8_t incoming_set_frame[]
    = {COMMAND_CLASS_BARRIER_OPERATOR, BARRIER_OPERATOR_SET, (uint8_t)value};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    barrier_operator_handler.control_handler(&connection_info,
                                             incoming_set_frame,
                                             sizeof(incoming_set_frame)));
}
