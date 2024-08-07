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
#include "zwave_command_class_thermostat_mode.h"
#include "zwave_command_classes_utils.h"
#include "unity.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "zpc_attribute_store_type_registration.h"

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
#include "dotdot_mqtt_mock.h"
#include "dotdot_mqtt_generated_commands_mock.h"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_##type

// Static variables
static attribute_resolver_function_t thermostat_mode_set           = NULL;
static attribute_resolver_function_t thermostat_mode_get           = NULL;
static attribute_resolver_function_t thermostat_mode_supported_get = NULL;
static zwave_command_handler_t thermostat_mode_handler             = {};

static uint8_t received_frame[255]  = {};
static uint16_t received_frame_size = 0;

// Stub functions
static sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  if (node_type == ATTRIBUTE(SUPPORTED_MODES)) {
    TEST_ASSERT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    thermostat_mode_supported_get = get_func;
  } else if (node_type == ATTRIBUTE(MODE)) {
    TEST_ASSERT_NOT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    thermostat_mode_get = get_func;
    thermostat_mode_set = set_func;
  } else {
    TEST_FAIL_MESSAGE("Attribute rule registration on a wrong type");
  }
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  thermostat_mode_handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    thermostat_mode_handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_THERMOSTAT_MODE_V3,
                    thermostat_mode_handler.command_class);
  TEST_ASSERT_EQUAL(THERMOSTAT_MODE_VERSION_V3,
                    thermostat_mode_handler.version);
  TEST_ASSERT_NOT_NULL(thermostat_mode_handler.control_handler);
  TEST_ASSERT_NULL(thermostat_mode_handler.support_handler);
  TEST_ASSERT_FALSE(thermostat_mode_handler.manual_security_validation);

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

static void zwave_command_class_thermostat_mode_init_verification()
{
  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_thermostat_mode_init());
}

/// Called before each and every test
void setUp()
{
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_set_home_id(home_id);

  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size           = 0;
  thermostat_mode_set           = NULL;
  thermostat_mode_get           = NULL;
  thermostat_mode_supported_get = NULL;
  memset(&thermostat_mode_handler, 0, sizeof(zwave_command_handler_t));

  zwave_command_class_thermostat_mode_init_verification();
}

void test_thermostat_mode_get_happy_case()
{
  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL(thermostat_mode_get);
  thermostat_mode_get(0, received_frame, &received_frame_size);
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_THERMOSTAT_MODE_V3, THERMOSTAT_MODE_GET_V3};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_thermostat_mode_supported_get_happy_case()
{
  TEST_ASSERT_NOT_NULL(thermostat_mode_supported_get);
  thermostat_mode_supported_get(0, received_frame, &received_frame_size);
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_THERMOSTAT_MODE_V3, THERMOSTAT_MODE_SUPPORTED_GET_V3};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_thermostat_mode_set_happy_case()
{
  // Create a mode node under the endpoint
  attribute_store_node_t mode_node
    = attribute_store_add_node(ATTRIBUTE(MODE), endpoint_id_node);

  int32_t mode = 3;
  attribute_store_set_desired(mode_node, &mode, sizeof(mode));

  // Check for the
  TEST_ASSERT_NOT_NULL(thermostat_mode_get);
  thermostat_mode_set(mode_node, received_frame, &received_frame_size);
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_THERMOSTAT_MODE_V3, THERMOSTAT_MODE_SET_V3, mode};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_thermostat_mode_publish_incoming_set_off()
{
  TEST_ASSERT_NOT_NULL(thermostat_mode_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  // Just receive a Thermostat Mode Set with 0
  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_THERMOSTAT_MODE_V3, THERMOSTAT_MODE_SET_V3, 0x00};

  uic_mqtt_dotdot_thermostat_state_t expected_attribute_values
    = {.system_mode = 0};
  const uic_mqtt_dotdot_thermostat_updated_state_t expected_attribute_list
    = {.system_mode = true};

  // Push it to MQTT
  uic_mqtt_dotdot_thermostat_publish_generated_write_attributes_command_Expect(
    NULL,
    connection_info.remote.endpoint_id,
    expected_attribute_values,
    expected_attribute_list);
  uic_mqtt_dotdot_thermostat_publish_generated_write_attributes_command_IgnoreArg_unid();

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    thermostat_mode_handler.control_handler(&connection_info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));
}

static void
  thermostat_mode_publish_incoming_set_verification(uint8_t zwave_mode,
                                                    uint8_t zcl_mode)
{
  TEST_ASSERT_NOT_NULL(thermostat_mode_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  // Just receive a Thermostat Mode Set with 0
  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_THERMOSTAT_MODE_V3, THERMOSTAT_MODE_SET_V3, zwave_mode};

  uic_mqtt_dotdot_thermostat_state_t expected_attribute_values
    = {.system_mode = zcl_mode};
  const uic_mqtt_dotdot_thermostat_updated_state_t expected_attribute_list
    = {.system_mode = true};

  // Push it to MQTT
  uic_mqtt_dotdot_thermostat_publish_generated_write_attributes_command_Expect(
    NULL,
    connection_info.remote.endpoint_id,
    expected_attribute_values,
    expected_attribute_list);
  uic_mqtt_dotdot_thermostat_publish_generated_write_attributes_command_IgnoreArg_unid();

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    thermostat_mode_handler.control_handler(&connection_info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));
}

void test_thermostat_mode_publish_incoming_set_values()
{
  thermostat_mode_publish_incoming_set_verification(0, 0);
  thermostat_mode_publish_incoming_set_verification(1, 4);
  thermostat_mode_publish_incoming_set_verification(2, 3);
  thermostat_mode_publish_incoming_set_verification(3, 1);
  thermostat_mode_publish_incoming_set_verification(6, 7);
  thermostat_mode_publish_incoming_set_verification(8, 8);
  thermostat_mode_publish_incoming_set_verification(9, 1);
  thermostat_mode_publish_incoming_set_verification(10, 1);
  thermostat_mode_publish_incoming_set_verification(11, 1);
  thermostat_mode_publish_incoming_set_verification(255, 1);
}

void test_thermostat_mode_no_publish_when_frame_too_short()
{
  TEST_ASSERT_NOT_NULL(thermostat_mode_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  // Just receive a Thermostat Mode Set with no mode field, nothing should happen.
  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_THERMOSTAT_MODE_V3, THERMOSTAT_MODE_SET_V3};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    thermostat_mode_handler.control_handler(&connection_info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));
}

void test_thermostat_mode_on_version_update()
{
  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(endpoint_id_node));

  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE(MODE_VERSION), endpoint_id_node);

  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 2;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));

  // Delete for test coverage
  attribute_store_delete_node(version_node);
  TEST_ASSERT_EQUAL(2, attribute_store_get_node_child_count(endpoint_id_node));
}

void test_thermostat_mode_incoming_report_happy_case()
{
  TEST_ASSERT_NOT_NULL(thermostat_mode_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  connection_info.remote.node_id                     = node_id;
  connection_info.remote.endpoint_id                 = endpoint_id;

  const uint32_t mode                   = 0xFF;
  const uint8_t incoming_report_frame[] = {COMMAND_CLASS_THERMOSTAT_MODE_V3,
                                           THERMOSTAT_MODE_REPORT_V3,
                                           (uint8_t)mode,
                                           1,
                                           3,
                                           4,
                                           5,
                                           6};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    thermostat_mode_handler.control_handler(&connection_info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  int32_t saved_mode               = 0;
  attribute_store_node_t mode_node = attribute_store_get_first_child_by_type(
    endpoint_id_node,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_MODE);
  attribute_store_get_reported(mode_node, &saved_mode, sizeof(saved_mode));

  const int32_t expected_mode = 0x1F;
  TEST_ASSERT_EQUAL(expected_mode, saved_mode);
  TEST_ASSERT_TRUE(attribute_store_is_value_matched(mode_node));
}

void test_thermostat_mode_incoming_report_too_short()
{
  TEST_ASSERT_NOT_NULL(thermostat_mode_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  connection_info.remote.node_id                     = node_id;
  connection_info.remote.endpoint_id                 = endpoint_id;

  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_THERMOSTAT_MODE_V3, THERMOSTAT_MODE_REPORT_V3};

  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    thermostat_mode_handler.control_handler(&connection_info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));
}

void test_thermostat_mode_incoming_supported_report_happy_case()
{
  TEST_ASSERT_NOT_NULL(thermostat_mode_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  connection_info.remote.node_id                     = node_id;
  connection_info.remote.endpoint_id                 = endpoint_id;

  attribute_store_node_t supported_modes_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SUPPORTED_MODES,
    endpoint_id_node);

  const uint8_t bit1 = 0b0000101;
  const uint8_t bit2 = 0b0000011;
  const uint8_t bit3 = 0b1111111;
  const uint8_t incoming_report_frame[] = {COMMAND_CLASS_THERMOSTAT_MODE_V3,
                                           THERMOSTAT_MODE_SUPPORTED_REPORT_V3,
                                           bit1,
                                           bit2,
                                           bit3};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    thermostat_mode_handler.control_handler(&connection_info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));

  uint32_t received_bitmask  = 0;
  attribute_store_get_reported(supported_modes_node,
                               &received_bitmask,
                               sizeof(received_bitmask));

  TEST_ASSERT_EQUAL_MESSAGE((bit3) << 16 | (bit2) << 8 | bit1,
                            received_bitmask,
                            "Received Bitmask mismatch");
}

void test_thermostat_mode_incoming_supported_report_too_short()
{
  TEST_ASSERT_NOT_NULL(thermostat_mode_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  connection_info.remote.node_id                     = node_id;
  connection_info.remote.endpoint_id                 = endpoint_id;

  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_THERMOSTAT_MODE_V3, THERMOSTAT_MODE_SUPPORTED_REPORT_V3};

  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    thermostat_mode_handler.control_handler(&connection_info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));
}

void test_thermostat_mode_incoming_unknown_command()
{
  TEST_ASSERT_NOT_NULL(thermostat_mode_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  connection_info.remote.node_id                     = node_id;
  connection_info.remote.endpoint_id                 = endpoint_id;

  const uint8_t incoming_report_frame[]
    = {COMMAND_CLASS_THERMOSTAT_MODE_V3, 0xFF};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    thermostat_mode_handler.control_handler(&connection_info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));
}

void test_thermostat_mode_command_too_short()
{
  TEST_ASSERT_NOT_NULL(thermostat_mode_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  connection_info.remote.node_id                     = node_id;
  connection_info.remote.endpoint_id                 = endpoint_id;

  const uint8_t incoming_report_frame[] = {COMMAND_CLASS_THERMOSTAT_MODE_V3};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    thermostat_mode_handler.control_handler(&connection_info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));
}

void test_thermostat_mode_wrong_command_class()
{
  TEST_ASSERT_NOT_NULL(thermostat_mode_handler.control_handler);
  zwave_controller_connection_info_t connection_info = {};
  connection_info.remote.node_id                     = node_id;
  connection_info.remote.endpoint_id                 = endpoint_id;

  const uint8_t incoming_report_frame[] = {0xFF, 0xFF};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    thermostat_mode_handler.control_handler(&connection_info,
                                            incoming_report_frame,
                                            sizeof(incoming_report_frame)));
}