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
#include "zwave_command_class_alarm_sensor.h"
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
#include "sl_log.h"
#include "zwave_command_class_alarm_sensor_types.h"

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
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_##type

// Static variables
static attribute_resolver_function_t alarm_sensor_get           = NULL;
static attribute_resolver_function_t alarm_sensor_supported_get = NULL;

static zwave_command_handler_t alarm_sensor_handler       = {};
static zwave_controller_connection_info_t connection_info = {};

static uint8_t received_frame[255]  = {};
static uint16_t received_frame_size = 0;

// Stub functions
static sl_status_t attribute_resolver_register_rule_stub(
  attribute_store_type_t node_type,
  attribute_resolver_function_t get_supported_func,
  attribute_resolver_function_t get_func,
  int cmock_num_calls)
{
  if (node_type == ATTRIBUTE(BITMASK)) {
    alarm_sensor_supported_get = get_func;
  } else if (node_type == ATTRIBUTE(STATE)) {
    alarm_sensor_get = get_func;
  } else {
    TEST_FAIL_MESSAGE("Attribute rule registration on a wrong type");
  }
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  alarm_sensor_handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    alarm_sensor_handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SENSOR_ALARM,
                    alarm_sensor_handler.command_class);
  TEST_ASSERT_EQUAL(SENSOR_ALARM_VERSION, alarm_sensor_handler.version);
  TEST_ASSERT_NOT_NULL(alarm_sensor_handler.control_handler);
  TEST_ASSERT_NULL(alarm_sensor_handler.support_handler);
  TEST_ASSERT_FALSE(alarm_sensor_handler.manual_security_validation);

  return SL_STATUS_OK;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

static void zwave_command_class_sensor_alarm_init_verification()
{
  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_alarm_sensor_init());
}

/// Called before each and every test
void setUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_set_home_id(home_id);
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);

  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size        = 0;
  alarm_sensor_get           = NULL;
  alarm_sensor_supported_get = NULL;

  memset(&alarm_sensor_handler, 0, sizeof(zwave_command_handler_t));

  zwave_command_class_sensor_alarm_init_verification();

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
void tearDown()
{
  attribute_store_teardown();
  datastore_teardown();
}

void test_zwave_command_class_alarm_sensor_get_happy_case()
{
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_SENSOR_ALARM),
    endpoint_id_node);

  // Version is undefined, no attributes created just yet.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));
  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(endpoint_id_node));

  alarm_sensor_type_t type = 0;
  attribute_store_node_t type_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(TYPE),
                                             0);
  attribute_store_get_reported(type_node, &type, sizeof(type));

  TEST_ASSERT_EQUAL(0, type);

  alarm_sensor_state_t state = 3;
  attribute_store_node_t state_node
    = attribute_store_get_node_child_by_type(type_node, ATTRIBUTE(STATE), 0);
  attribute_store_get_reported(state_node, &state, sizeof(state));

  // Now ask to generate a command
  TEST_ASSERT_NOT_NULL(alarm_sensor_get);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    alarm_sensor_get(state_node, received_frame, &received_frame_size));
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_SENSOR_ALARM, SENSOR_ALARM_GET, type};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_alarm_sensor_supported_get_happy_case()
{
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_SENSOR_ALARM),
    endpoint_id_node);

  // Version is undefined, no attributes created just yet.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));
  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(endpoint_id_node));

  alarm_sensor_type_t type = 0;
  attribute_store_node_t type_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(TYPE),
                                             0);
  attribute_store_get_reported(type_node, &type, sizeof(type));

  TEST_ASSERT_EQUAL(0, type);

  uint8_t bitmask[] = {0};
  attribute_store_node_t bitmask_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(BITMASK),
                                             0);
  attribute_store_get_reported(bitmask_node, &bitmask, sizeof(bitmask));

  // Now ask to generate a command
  TEST_ASSERT_NOT_NULL(alarm_sensor_supported_get);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    alarm_sensor_supported_get(bitmask_node,
                                               received_frame,
                                               &received_frame_size));
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_SENSOR_ALARM, SENSOR_ALARM_SUPPORTED_GET};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_alarm_sensor_report_happy_case()
{
  // Simulate a version 1 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_SENSOR_ALARM),
    endpoint_id_node);

  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  alarm_sensor_id_t id           = 9;
  alarm_sensor_type_t type       = 25;
  alarm_sensor_state_t state     = 40;
  alarm_sensor_seconds_t seconds = ((uint16_t)(10) << 8) | 20;

  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(endpoint_id_node));

  attribute_store_node_t type_node = attribute_store_emplace(endpoint_id_node,
                                                             ATTRIBUTE(TYPE),
                                                             &type,
                                                             sizeof(type));

  const attribute_store_type_t type_attribute[]
    = {ATTRIBUTE(STATE), ATTRIBUTE(NODE_ID), ATTRIBUTE(SECONDS)};
  attribute_store_add_if_missing(type_node,
                                 type_attribute,
                                 COUNT_OF(type_attribute));

  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(type_node));

  uint8_t incoming_frame[] = {COMMAND_CLASS_SENSOR_ALARM,
                              SENSOR_ALARM_REPORT,
                              id & 0xFF,
                              type & 0xFF,
                              state & 0xFF,
                              (uint8_t)((seconds >> 8) & 0xFF),  // MSB
                              (uint8_t)((seconds)&0xFF)};        // LSB

  TEST_ASSERT_NOT_NULL(alarm_sensor_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    alarm_sensor_handler.control_handler(&connection_info,
                                         incoming_frame,
                                         sizeof(incoming_frame)));
  attribute_store_node_t node_type
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(TYPE),
                                             0);
  alarm_sensor_type_t type_value = 0;
  attribute_store_get_reported(node_type, &type_value, sizeof(type_value));
  TEST_ASSERT_EQUAL(type, type_value);

  attribute_store_node_t node_id
    = attribute_store_get_node_child_by_type(node_type, ATTRIBUTE(NODE_ID), 0);
  alarm_sensor_id_t received_value = 0;
  attribute_store_get_reported(node_id,
                               &received_value,
                               sizeof(received_value));
  TEST_ASSERT_EQUAL(id, received_value);

  attribute_store_node_t state_node
    = attribute_store_get_node_child_by_type(type_node, ATTRIBUTE(STATE), 0);
  alarm_sensor_state_t state_value = 0;
  attribute_store_get_reported(state_node, &state_value, sizeof(state_value));
  TEST_ASSERT_EQUAL(state, state_value);

  attribute_store_node_t seconds_node
    = attribute_store_get_node_child_by_type(type_node, ATTRIBUTE(SECONDS), 0);

  alarm_sensor_seconds_t received_values = 0;
  attribute_store_get_reported(seconds_node,
                               &received_values,
                               sizeof(received_values));

  TEST_ASSERT_EQUAL_UINT16(seconds, received_values);
}

void test_zwave_command_class_alarm_sensor_supported_report_happy_case()
{
  // Simulate a version 1 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_SENSOR_ALARM),
    endpoint_id_node);

  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  uint8_t number_of_bytes = 2;
  uint8_t bitmask[2]      = {01, 02};

  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(endpoint_id_node));
  const attribute_store_type_t bitmask_attribute[] = {ATTRIBUTE(BITMASK)};
  attribute_store_add_if_missing(endpoint_id_node,
                                 bitmask_attribute,
                                 COUNT_OF(bitmask_attribute));
  uint8_t incoming_frame[] = {COMMAND_CLASS_SENSOR_ALARM,
                              SENSOR_ALARM_SUPPORTED_REPORT,
                              number_of_bytes & 0xFF,
                              bitmask[0] & 0xFF,
                              bitmask[1] & 0xFF};

  TEST_ASSERT_NOT_NULL(alarm_sensor_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    alarm_sensor_handler.control_handler(&connection_info,
                                         incoming_frame,
                                         sizeof(incoming_frame)));
  attribute_store_node_t bitmask_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(BITMASK),
                                             0);
  // Check if the 2 type nodes created
  uint8_t bitmask_v[] = {0, 0};
  attribute_store_get_reported(bitmask_node, &bitmask_v, sizeof(bitmask_v));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(bitmask, bitmask_v, 2);

  attribute_store_node_t node_type
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(TYPE),
                                             0);

  alarm_sensor_type_t type_value = 0;
  attribute_store_get_reported(node_type, &type_value, sizeof(type_value));
  TEST_ASSERT_EQUAL(0, type_value);

  node_type = attribute_store_get_node_child_by_type(endpoint_id_node,
                                                     ATTRIBUTE(TYPE),
                                                     1);

  type_value = 0;
  attribute_store_get_reported(node_type, &type_value, sizeof(type_value));
  TEST_ASSERT_EQUAL(9, type_value);

  attribute_store_node_t node_id
    = attribute_store_get_node_child_by_type(node_type, ATTRIBUTE(NODE_ID), 0);
  alarm_sensor_id_t received_value = 0;
  attribute_store_get_reported(node_id,
                               &received_value,
                               sizeof(received_value));
  TEST_ASSERT_EQUAL(0, received_value);

  attribute_store_node_t state_node
    = attribute_store_get_node_child_by_type(node_type, ATTRIBUTE(STATE), 0);
  alarm_sensor_state_t state_value = 0;
  attribute_store_get_reported(state_node, &state_value, sizeof(state_value));
  TEST_ASSERT_EQUAL(0, state_value);

  attribute_store_node_t seconds_node
    = attribute_store_get_node_child_by_type(node_type, ATTRIBUTE(SECONDS), 0);
  alarm_sensor_seconds_t seconds         = 0;
  alarm_sensor_seconds_t received_values = 0;
  attribute_store_get_reported(seconds_node,
                               &received_values,
                               sizeof(received_values));

  TEST_ASSERT_EQUAL_UINT16(seconds, received_values);
}

void test_zwave_command_class_alarm_sensor_report_smaller_frame()
{
  // Simulate a version 1 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_SENSOR_ALARM),
    endpoint_id_node);

  // Version is undefined, no attributes created just yet.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(endpoint_id_node));

  alarm_sensor_id_t id       = 10;
  alarm_sensor_type_t type   = 25;
  alarm_sensor_state_t state = 40;
  uint8_t incoming_frame[]   = {COMMAND_CLASS_SENSOR_ALARM,
                                SENSOR_ALARM_REPORT,
                                id & 0xFF,
                                type & 0xFF,
                                state & 0xFF};

  TEST_ASSERT_NOT_NULL(alarm_sensor_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    alarm_sensor_handler.control_handler(&connection_info,
                                         incoming_frame,
                                         sizeof(incoming_frame)));
}

void test_zwave_command_class_alarm_sensor_report_last_second_missing()
{
  // Simulate a version 1 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_SENSOR_ALARM),
    endpoint_id_node);

  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  alarm_sensor_id_t id           = 9;
  alarm_sensor_type_t type       = 25;
  alarm_sensor_state_t state     = 40;
  alarm_sensor_seconds_t seconds = ((uint16_t)(10) << 8);

  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(endpoint_id_node));

  attribute_store_node_t type_node = attribute_store_emplace(endpoint_id_node,
                                                             ATTRIBUTE(TYPE),
                                                             &type,
                                                             sizeof(type));

  const attribute_store_type_t type_attribute[]
    = {ATTRIBUTE(STATE), ATTRIBUTE(NODE_ID), ATTRIBUTE(SECONDS)};
  attribute_store_add_if_missing(type_node,
                                 type_attribute,
                                 COUNT_OF(type_attribute));

  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(type_node));

  uint8_t incoming_frame[] = {COMMAND_CLASS_SENSOR_ALARM,
                              SENSOR_ALARM_REPORT,
                              id & 0xFF,
                              type & 0xFF,
                              state & 0xFF,
                              (uint8_t)((seconds >> 8) & 0xFF)};  // MSB

  TEST_ASSERT_NOT_NULL(alarm_sensor_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    alarm_sensor_handler.control_handler(&connection_info,
                                         incoming_frame,
                                         sizeof(incoming_frame)));

  attribute_store_node_t node_type
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(TYPE),
                                             0);
  alarm_sensor_type_t type_value = 0;
  attribute_store_get_reported(node_type, &type_value, sizeof(type_value));
  TEST_ASSERT_EQUAL(type, type_value);

  attribute_store_node_t node_id
    = attribute_store_get_node_child_by_type(type_node, ATTRIBUTE(NODE_ID), 0);
  alarm_sensor_id_t received_value = 0;
  attribute_store_get_reported(node_id,
                               &received_value,
                               sizeof(received_value));
  TEST_ASSERT_EQUAL(id, received_value);

  attribute_store_node_t state_node
    = attribute_store_get_node_child_by_type(type_node, ATTRIBUTE(STATE), 0);
  alarm_sensor_state_t state_value = 0;
  attribute_store_get_reported(state_node, &state_value, sizeof(state_value));
  TEST_ASSERT_EQUAL(state, state_value);

  attribute_store_node_t seconds_node
    = attribute_store_get_node_child_by_type(type_node, ATTRIBUTE(SECONDS), 0);

  alarm_sensor_seconds_t received_values = 0;

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_get_reported(seconds_node,
                                                 &received_values,
                                                 sizeof(received_values)));

  TEST_ASSERT_NOT_EQUAL_UINT16(seconds, received_values);
}

void test_zwave_command_class_alarm_sensor_supported_report_bitmask_missing()
{
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_SENSOR_ALARM),
    endpoint_id_node);

  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(endpoint_id_node));

  uint8_t number_of_bytes  = 40;
  uint8_t incoming_frame[] = {COMMAND_CLASS_SENSOR_ALARM,
                              SENSOR_ALARM_SUPPORTED_REPORT,
                              number_of_bytes & 0xFF};

  TEST_ASSERT_NOT_NULL(alarm_sensor_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    alarm_sensor_handler.control_handler(&connection_info,
                                         incoming_frame,
                                         sizeof(incoming_frame)));
}

void test_zwave_command_class_alarm_sensor_supported_report_bits_missing()
{
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_SENSOR_ALARM),
    endpoint_id_node);

  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(endpoint_id_node));

  uint8_t number_of_bytes = 2;
  uint8_t bitmask[]       = {0, 1};

  uint8_t incoming_frame[] = {COMMAND_CLASS_SENSOR_ALARM,
                              SENSOR_ALARM_SUPPORTED_REPORT,
                              number_of_bytes & 0xFF,
                              bitmask[0] & 0xFF};

  TEST_ASSERT_NOT_NULL(alarm_sensor_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    alarm_sensor_handler.control_handler(&connection_info,
                                         incoming_frame,
                                         sizeof(incoming_frame)));
}

void test_zwave_command_class_alarm_sensor_unknown_command()
{
  const uint8_t incoming_frame[] = {COMMAND_CLASS_SENSOR_ALARM, 0xFE};

  TEST_ASSERT_NOT_NULL(alarm_sensor_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    alarm_sensor_handler.control_handler(&connection_info,
                                         incoming_frame,
                                         sizeof(incoming_frame)));
}

void test_zwave_command_class_alarm_sensor_no_command()
{
  const uint8_t incoming_frame[] = {COMMAND_CLASS_SENSOR_ALARM};

  TEST_ASSERT_NOT_NULL(alarm_sensor_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    alarm_sensor_handler.control_handler(&connection_info,
                                         incoming_frame,
                                         sizeof(incoming_frame)));
}

void test_zwave_command_class_alarm_sensor_wrong_command_class()
{
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_SENSOR_MULTILEVEL_V11, SENSOR_ALARM_REPORT};

  TEST_ASSERT_NOT_NULL(alarm_sensor_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    alarm_sensor_handler.control_handler(&connection_info,
                                         incoming_frame,
                                         sizeof(incoming_frame)));
}
