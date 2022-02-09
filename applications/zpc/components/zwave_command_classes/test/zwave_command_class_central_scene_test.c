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
#include "zwave_command_class_central_scene.h"
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

// Interface includes
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"
#include "zwave_utils.h"
#include "zwave_controller_types.h"
#include "zwave_command_class_central_scene_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "attribute_resolver_mock.h"
#include "attribute_timeouts_mock.h"
#include "zpc_attribute_resolver_mock.h"
#include "zwave_command_handler_mock.h"
#include "zwave_tx_mock.h"
#include "zwave_tx_scheme_selector_mock.h"
#include "dotdot_mqtt_mock.h"
#include "dotdot_mqtt_supported_generated_commands_mock.h"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_CENTRAL_SCENE_##type

// Constants
static const uic_mqtt_dotdot_scenes_supported_commands_t
  expected_supported_generated_commands
  = {.recall_scene = true};

// Static variables
static attribute_resolver_function_t central_scene_configuration_set = NULL;
static attribute_resolver_function_t central_scene_configuration_get = NULL;
static attribute_resolver_function_t central_scene_supported_get     = NULL;
static zwave_command_handler_t central_scene_handler                 = {};
static zwave_controller_connection_info_t connection_info            = {};
static attribute_timeout_callback_t return_scene_to_idle             = NULL;

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
  if (node_type == ATTRIBUTE(NUMBER_OF_SCENES)) {
    TEST_ASSERT_NULL(set_func);
    central_scene_supported_get = get_func;
  } else if (node_type == ATTRIBUTE(SLOW_REFRESH)) {
    central_scene_configuration_set = set_func;
    central_scene_configuration_get = get_func;
  } else {
    TEST_FAIL_MESSAGE("Attribute rule registration on a wrong type");
  }
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  central_scene_handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    central_scene_handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_CENTRAL_SCENE_V3,
                    central_scene_handler.command_class);
  TEST_ASSERT_EQUAL(CENTRAL_SCENE_VERSION_V3, central_scene_handler.version);
  TEST_ASSERT_NOT_NULL(central_scene_handler.control_handler);
  TEST_ASSERT_NULL(central_scene_handler.support_handler);
  TEST_ASSERT_FALSE(central_scene_handler.manual_security_validation);

  return SL_STATUS_OK;
}

static sl_status_t attribute_timeout_set_callback_stub(
  attribute_store_node_t node,
  clock_time_t duration,
  attribute_timeout_callback_t callback_function,
  int cmock_num_calls)
{
  TEST_ASSERT_EQUAL(endpoint_id_node, node);
  return_scene_to_idle = callback_function;
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

static void zwave_command_class_central_scene_init_verification()
{
  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_central_scene_init());
}

static void verify_attribute_store_data_supported_report(
  central_scene_scene_t supported_scenes,
  central_scene_key_attribute_t expected_max_attribute)
{
  central_scene_scene_t saved_scenes = 0xFFFFFF;
  attribute_store_node_t number_of_scenes_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(NUMBER_OF_SCENES),
                                             0);
  attribute_store_get_reported(number_of_scenes_node,
                               &saved_scenes,
                               sizeof(saved_scenes));
  TEST_ASSERT_EQUAL(supported_scenes * expected_max_attribute, saved_scenes);

  central_scene_key_attribute_t max_attribute = 0xFF;
  attribute_store_node_t max_attribute_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(MAX_KEY_ATTRIBUTE),
                                             0);
  attribute_store_get_reported(max_attribute_node,
                               &max_attribute,
                               sizeof(max_attribute));
  TEST_ASSERT_EQUAL(expected_max_attribute, max_attribute);
}

static void verify_attribute_store_data_notification(
  central_scene_scene_t active_scene,
  central_scene_sequence_number_t sequence_number)
{
  central_scene_scene_t saved_active_scene = 0xFFFFFF;
  attribute_store_node_t active_scene_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(ACTIVE_SCENE),
                                             0);
  attribute_store_get_reported(active_scene_node,
                               &saved_active_scene,
                               sizeof(saved_active_scene));
  TEST_ASSERT_EQUAL(active_scene, saved_active_scene);

  attribute_store_node_t sequence_number_node
    = attribute_store_get_node_child_by_type(
      active_scene_node,
      ATTRIBUTE(ACTIVE_SCENE_SEQUENCE_NUMBER),
      0);

  central_scene_sequence_number_t saved_sequence_number = 0xFF;
  attribute_store_get_reported(sequence_number_node,
                               &saved_sequence_number,
                               sizeof(saved_sequence_number));
  TEST_ASSERT_EQUAL(sequence_number, saved_sequence_number);
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
  central_scene_configuration_set = NULL;
  central_scene_configuration_get = NULL;
  central_scene_configuration_get = NULL;
  central_scene_supported_get     = NULL;
  return_scene_to_idle            = NULL;
  memset(&central_scene_handler, 0, sizeof(zwave_command_handler_t));

  zwave_command_class_central_scene_init_verification();

  // Prepare a default connection info with the supporting node
  memset(&connection_info, 0, sizeof(zwave_controller_connection_info_t));
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));
}

void test_zwave_command_class_supported_get_happy_case()
{
  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL(central_scene_supported_get);
  central_scene_supported_get(0, received_frame, &received_frame_size);
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_CENTRAL_SCENE_V3, CENTRAL_SCENE_SUPPORTED_GET_V3};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_central_scene_configuration_get_happy_case()
{
  // Create a version node under the endpoint
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);

  uic_mqtt_dotdot_scenes_publish_supported_generated_commands_ExpectWithArray(
    NULL,
    endpoint_id,
    &expected_supported_generated_commands,
    sizeof(expected_supported_generated_commands));
  uic_mqtt_dotdot_scenes_publish_supported_generated_commands_IgnoreArg_unid();
  zwave_cc_version_t version = 3;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  attribute_store_node_t slow_refresh_node
    = attribute_store_add_node(ATTRIBUTE(SLOW_REFRESH), endpoint_id_node);

  // Now ask for a Get Command:
  TEST_ASSERT_NOT_NULL(central_scene_configuration_get);
  central_scene_configuration_get(slow_refresh_node,
                                  received_frame,
                                  &received_frame_size);
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_CENTRAL_SCENE_V3, CENTRAL_SCENE_CONFIGURATION_GET_V3};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_central_scene_configuration_get_not_supported()
{
  // Create a version node under the endpoint
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);

  uic_mqtt_dotdot_scenes_publish_supported_generated_commands_ExpectWithArray(
    NULL,
    endpoint_id,
    &expected_supported_generated_commands,
    sizeof(expected_supported_generated_commands));
  uic_mqtt_dotdot_scenes_publish_supported_generated_commands_IgnoreArg_unid();

  zwave_cc_version_t version = 2;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  attribute_store_node_t slow_refresh_node
    = attribute_store_add_node(ATTRIBUTE(SLOW_REFRESH), endpoint_id_node);

  // Now ask for a Get Command:
  TEST_ASSERT_NOT_NULL(central_scene_configuration_get);
  central_scene_configuration_get(slow_refresh_node,
                                  received_frame,
                                  &received_frame_size);

  TEST_ASSERT_EQUAL(0, received_frame_size);

  central_scene_slow_refresh_t slow_refresh = true;
  attribute_store_get_reported(slow_refresh_node,
                               &slow_refresh,
                               sizeof(slow_refresh));
  TEST_ASSERT_FALSE(slow_refresh);
}

void test_zwave_command_class_central_scene_configuration_set_slow_refresh()
{
  // Create a version node under the endpoint
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);

  uic_mqtt_dotdot_scenes_publish_supported_generated_commands_ExpectWithArray(
    NULL,
    endpoint_id,
    &expected_supported_generated_commands,
    sizeof(expected_supported_generated_commands));
  uic_mqtt_dotdot_scenes_publish_supported_generated_commands_IgnoreArg_unid();
  zwave_cc_version_t version = 3;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  attribute_store_node_t slow_refresh_node
    = attribute_store_add_node(ATTRIBUTE(SLOW_REFRESH), endpoint_id_node);
  central_scene_slow_refresh_t slow_refresh = true;
  attribute_store_set_desired(slow_refresh_node,
                              &slow_refresh,
                              sizeof(slow_refresh));

  // Now ask for a Set Command:
  TEST_ASSERT_NOT_NULL(central_scene_configuration_set);
  central_scene_configuration_set(slow_refresh_node,
                                  received_frame,
                                  &received_frame_size);
  const uint8_t expected_frame[] = {COMMAND_CLASS_CENTRAL_SCENE_V3,
                                    CENTRAL_SCENE_CONFIGURATION_SET_V3,
                                    0x80};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_central_scene_configuration_set_no_slow_refresh()
{
  // Create a version node under the endpoint
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);

  uic_mqtt_dotdot_scenes_publish_supported_generated_commands_ExpectWithArray(
    NULL,
    endpoint_id,
    &expected_supported_generated_commands,
    sizeof(expected_supported_generated_commands));
  uic_mqtt_dotdot_scenes_publish_supported_generated_commands_IgnoreArg_unid();
  zwave_cc_version_t version = 3;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  attribute_store_node_t slow_refresh_node
    = attribute_store_add_node(ATTRIBUTE(SLOW_REFRESH), endpoint_id_node);
  central_scene_slow_refresh_t slow_refresh = false;
  attribute_store_set_desired(slow_refresh_node,
                              &slow_refresh,
                              sizeof(slow_refresh));

  // Now ask for a Set Command:
  TEST_ASSERT_NOT_NULL(central_scene_configuration_set);
  central_scene_configuration_set(slow_refresh_node,
                                  received_frame,
                                  &received_frame_size);
  const uint8_t expected_frame[] = {COMMAND_CLASS_CENTRAL_SCENE_V3,
                                    CENTRAL_SCENE_CONFIGURATION_SET_V3,
                                    0x00};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_central_scene_frame_too_short()
{
  TEST_ASSERT_NOT_NULL(central_scene_handler.control_handler);
  const uint8_t incoming_frame[] = {COMMAND_CLASS_CENTRAL_SCENE_V3};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    central_scene_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));
}

void test_zwave_command_class_central_scene_frame_wrong_command_class()
{
  TEST_ASSERT_NOT_NULL(central_scene_handler.control_handler);
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_SECURITY_2, CENTRAL_SCENE_CONFIGURATION_REPORT_V3};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    central_scene_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));
}

void test_zwave_command_class_central_scene_frame_unknown_command()
{
  TEST_ASSERT_NOT_NULL(central_scene_handler.control_handler);
  const uint8_t incoming_frame[] = {COMMAND_CLASS_CENTRAL_SCENE_V3, 0xFF};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    central_scene_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));
}

void test_zwave_command_class_central_scene_supported_report_too_short()
{
  TEST_ASSERT_NOT_NULL(central_scene_handler.control_handler);
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_CENTRAL_SCENE_V3, CENTRAL_SCENE_SUPPORTED_REPORT_V3};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    central_scene_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Attribute store verifications
  verify_attribute_store_data_supported_report(0, 0);

  attribute_store_node_t slow_refresh_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SLOW_REFRESH),
                                             0);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(slow_refresh_node, DESIRED_ATTRIBUTE));
}

void test_zwave_command_class_central_scene_supported_report_happy_case()
{
  TEST_ASSERT_NOT_NULL(central_scene_handler.control_handler);
  const uint8_t supported_scenes      = 3;
  const uint8_t properties_1          = (0 << 7) | (1 << 1) | 0;
  const uint8_t highest_key_attribute = 5;
  const uint8_t incoming_frame[]      = {COMMAND_CLASS_CENTRAL_SCENE_V3,
                                    CENTRAL_SCENE_SUPPORTED_REPORT_V3,
                                    supported_scenes,
                                    properties_1,
                                    1,
                                    1,
                                    1 << highest_key_attribute};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    central_scene_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Attribute store verifications
  verify_attribute_store_data_supported_report(supported_scenes,
                                               highest_key_attribute + 1);

  attribute_store_node_t slow_refresh_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(MAX_KEY_ATTRIBUTE),
                                             0);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(slow_refresh_node, DESIRED_ATTRIBUTE));
}

void test_zwave_command_class_central_scene_supported_report_not_identical()
{
  TEST_ASSERT_NOT_NULL(central_scene_handler.control_handler);
  const uint8_t supported_scenes      = 2;
  const uint8_t properties_1          = (1 << 7) | (2 << 1) | 0;
  const uint8_t highest_key_attribute = 3;
  const uint8_t incoming_frame[]      = {COMMAND_CLASS_CENTRAL_SCENE_V3,
                                    CENTRAL_SCENE_SUPPORTED_REPORT_V3,
                                    supported_scenes,
                                    properties_1,
                                    1 << highest_key_attribute,
                                    1 << highest_key_attribute,
                                    1 << highest_key_attribute,
                                    0};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    central_scene_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Attribute store verifications
  verify_attribute_store_data_supported_report(supported_scenes,
                                               (highest_key_attribute + 1) + 8);

  attribute_store_node_t slow_refresh_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SLOW_REFRESH),
                                             0);
  central_scene_slow_refresh_t slow_refresh = false;
  attribute_store_get_desired(slow_refresh_node,
                              &slow_refresh,
                              sizeof(slow_refresh));

  TEST_ASSERT_TRUE(slow_refresh);
}

void test_zwave_command_class_central_scene_supported_report_happy_bitmask_too_short_v0()
{
  TEST_ASSERT_NOT_NULL(central_scene_handler.control_handler);
  const uint8_t supported_scenes      = 4;
  const uint8_t properties_1          = (1 << 7) | (1 << 1) | 0;
  const uint8_t highest_key_attribute = 3;
  const uint8_t incoming_frame[]      = {COMMAND_CLASS_CENTRAL_SCENE_V3,
                                    CENTRAL_SCENE_SUPPORTED_REPORT_V3,
                                    supported_scenes,
                                    properties_1,
                                    1 << highest_key_attribute,
                                    1 << highest_key_attribute,
                                    1 << highest_key_attribute};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    central_scene_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Attribute store verifications
  verify_attribute_store_data_supported_report(supported_scenes, 0);

  attribute_store_node_t slow_refresh_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SLOW_REFRESH),
                                             0);
  central_scene_slow_refresh_t slow_refresh = false;
  attribute_store_get_desired(slow_refresh_node,
                              &slow_refresh,
                              sizeof(slow_refresh));

  TEST_ASSERT_TRUE(slow_refresh);
}

void test_zwave_command_class_central_scene_supported_report_happy_bitmask_too_short_v1()
{
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);

  uic_mqtt_dotdot_scenes_publish_supported_generated_commands_ExpectWithArray(
    NULL,
    endpoint_id,
    &expected_supported_generated_commands,
    sizeof(expected_supported_generated_commands));
  uic_mqtt_dotdot_scenes_publish_supported_generated_commands_IgnoreArg_unid();

  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  TEST_ASSERT_NOT_NULL(central_scene_handler.control_handler);
  const uint8_t supported_scenes      = 2;
  const uint8_t properties_1          = (1 << 7) | (2 << 1) | 0;
  const uint8_t highest_key_attribute = 3;
  const uint8_t incoming_frame[]      = {COMMAND_CLASS_CENTRAL_SCENE_V3,
                                    CENTRAL_SCENE_SUPPORTED_REPORT_V3,
                                    supported_scenes,
                                    properties_1,
                                    1 << highest_key_attribute,
                                    1 << highest_key_attribute,
                                    1 << highest_key_attribute};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    central_scene_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Attribute store verifications
  verify_attribute_store_data_supported_report(supported_scenes, 3);

  attribute_store_node_t slow_refresh_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SLOW_REFRESH),
                                             0);
  central_scene_slow_refresh_t slow_refresh = false;
  attribute_store_get_desired(slow_refresh_node,
                              &slow_refresh,
                              sizeof(slow_refresh));

  TEST_ASSERT_TRUE(slow_refresh);
}

void test_zwave_command_class_central_scene_supported_report_happy_bitmask_too_short_v3()
{
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);

  uic_mqtt_dotdot_scenes_publish_supported_generated_commands_ExpectWithArray(
    NULL,
    endpoint_id,
    &expected_supported_generated_commands,
    sizeof(expected_supported_generated_commands));
  uic_mqtt_dotdot_scenes_publish_supported_generated_commands_IgnoreArg_unid();

  zwave_cc_version_t version = 3;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  TEST_ASSERT_NOT_NULL(central_scene_handler.control_handler);
  const uint8_t supported_scenes      = 2;
  const uint8_t properties_1          = (1 << 7) | (3 << 1) | 0;
  const uint8_t highest_key_attribute = 3;
  const uint8_t incoming_frame[]      = {COMMAND_CLASS_CENTRAL_SCENE_V3,
                                    CENTRAL_SCENE_SUPPORTED_REPORT_V3,
                                    supported_scenes,
                                    properties_1,
                                    1 << highest_key_attribute,
                                    1 << highest_key_attribute,
                                    1 << highest_key_attribute};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    central_scene_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Attribute store verifications
  verify_attribute_store_data_supported_report(supported_scenes, 7);

  attribute_store_node_t slow_refresh_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SLOW_REFRESH),
                                             0);
  central_scene_slow_refresh_t slow_refresh = false;
  attribute_store_get_desired(slow_refresh_node,
                              &slow_refresh,
                              sizeof(slow_refresh));

  TEST_ASSERT_TRUE(slow_refresh);
}

void test_zwave_command_class_central_scene_supported_report_happy_bitmask_very_long()
{
  TEST_ASSERT_NOT_NULL(central_scene_handler.control_handler);
  const uint8_t supported_scenes      = 3;
  const uint8_t properties_1          = (1 << 7) | (3 << 1) | 0;
  const uint8_t highest_key_attribute = 5;
  const uint8_t incoming_frame[]      = {COMMAND_CLASS_CENTRAL_SCENE_V3,
                                    CENTRAL_SCENE_SUPPORTED_REPORT_V3,
                                    supported_scenes,
                                    properties_1,
                                    1 << 7,
                                    1 << 7,
                                    1 << 2,
                                    1 << 7,
                                    1 << 7,
                                    1 << 3,
                                    1 << 7,
                                    1 << 7,
                                    1 << highest_key_attribute};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    central_scene_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Attribute store verifications
  verify_attribute_store_data_supported_report(
    supported_scenes,
    ((highest_key_attribute + 1) + (2 * 8)));

  attribute_store_node_t slow_refresh_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SLOW_REFRESH),
                                             0);
  central_scene_slow_refresh_t slow_refresh = false;
  attribute_store_get_desired(slow_refresh_node,
                              &slow_refresh,
                              sizeof(slow_refresh));

  TEST_ASSERT_TRUE(slow_refresh);
}

void test_zwave_command_class_central_scene_supported_report_identical_scenes()
{
  TEST_ASSERT_NOT_NULL(central_scene_handler.control_handler);
  const uint8_t supported_scenes      = 3;
  const uint8_t properties_1          = (1 << 7) | (1 << 1) | 1;
  const uint8_t highest_key_attribute = 5;
  const uint8_t incoming_frame[]      = {COMMAND_CLASS_CENTRAL_SCENE_V3,
                                    CENTRAL_SCENE_SUPPORTED_REPORT_V3,
                                    supported_scenes,
                                    properties_1,
                                    1 << highest_key_attribute};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    central_scene_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Attribute store verifications
  verify_attribute_store_data_supported_report(supported_scenes,
                                               (highest_key_attribute + 1));

  attribute_store_node_t slow_refresh_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SLOW_REFRESH),
                                             0);
  central_scene_slow_refresh_t slow_refresh = false;
  attribute_store_get_desired(slow_refresh_node,
                              &slow_refresh,
                              sizeof(slow_refresh));

  TEST_ASSERT_TRUE(slow_refresh);
}

void test_zwave_command_class_central_scene_configuration_report_happy_case()
{
  TEST_ASSERT_NOT_NULL(central_scene_handler.control_handler);
  const uint8_t properties_1     = (1 << 7);
  const uint8_t incoming_frame[] = {COMMAND_CLASS_CENTRAL_SCENE_V3,
                                    CENTRAL_SCENE_CONFIGURATION_REPORT_V3,
                                    properties_1};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    central_scene_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Attribute store verifications
  attribute_store_node_t slow_refresh_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SLOW_REFRESH),
                                             0);
  central_scene_slow_refresh_t slow_refresh = false;
  attribute_store_get_reported(slow_refresh_node,
                               &slow_refresh,
                               sizeof(slow_refresh));

  TEST_ASSERT_TRUE(slow_refresh);
}

void test_zwave_command_class_central_scene_configuration_report_happy_case_no_slow_refresh()
{
  TEST_ASSERT_NOT_NULL(central_scene_handler.control_handler);
  const uint8_t properties_1     = (0 << 7);
  const uint8_t incoming_frame[] = {COMMAND_CLASS_CENTRAL_SCENE_V3,
                                    CENTRAL_SCENE_CONFIGURATION_REPORT_V3,
                                    properties_1};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    central_scene_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Attribute store verifications
  attribute_store_node_t slow_refresh_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SLOW_REFRESH),
                                             0);
  central_scene_slow_refresh_t slow_refresh = true;
  attribute_store_get_reported(slow_refresh_node,
                               &slow_refresh,
                               sizeof(slow_refresh));

  TEST_ASSERT_FALSE(slow_refresh);
}

void test_zwave_command_class_central_scene_configuration_report_too_short()
{
  TEST_ASSERT_NOT_NULL(central_scene_handler.control_handler);
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_CENTRAL_SCENE_V3, CENTRAL_SCENE_CONFIGURATION_REPORT_V3};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    central_scene_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Attribute store verifications
  attribute_store_node_t slow_refresh_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(SLOW_REFRESH),
                                             0);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, slow_refresh_node);
}

void test_zwave_command_class_central_scene_notification()
{
  attribute_store_add_node(ATTRIBUTE(ACTIVE_SCENE), endpoint_id_node);
  TEST_ASSERT_NOT_NULL(central_scene_handler.control_handler);
  const uint8_t sequence_number  = 30;
  const uint8_t properties_1     = (1 << 7) | (0);
  const uint8_t scene_number     = 1;
  const uint8_t incoming_frame[] = {COMMAND_CLASS_CENTRAL_SCENE_V3,
                                    CENTRAL_SCENE_NOTIFICATION_V3,
                                    sequence_number,
                                    properties_1,
                                    scene_number};

  central_scene_scene_t expected_active_scene
    = ((scene_number - 1) * 7) + (properties_1 & 7) + 1;
  uic_mqtt_dotdot_scenes_command_recall_scene_fields_t expected_fields = {};
  expected_fields.sceneid = expected_active_scene - 1;
  uic_mqtt_dotdot_scenes_publish_generated_recall_scene_command_Expect(
    NULL,
    3,
    &expected_fields);
  uic_mqtt_dotdot_scenes_publish_generated_recall_scene_command_IgnoreArg_unid();

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    central_scene_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Attribute store verifications
  verify_attribute_store_data_notification(expected_active_scene,
                                           sequence_number);
}

void test_zwave_command_class_central_scene_notification_key_down_repeated_with_slow_refresh()
{
  attribute_store_add_node(ATTRIBUTE(ACTIVE_SCENE), endpoint_id_node);
  central_scene_key_attribute_t max_attributes = 5;
  attribute_store_node_t supported_attributes_node
    = attribute_store_add_node(ATTRIBUTE(MAX_KEY_ATTRIBUTE), endpoint_id_node);
  attribute_store_set_reported(supported_attributes_node,
                               &max_attributes,
                               sizeof(max_attributes));

  TEST_ASSERT_NOT_NULL(central_scene_handler.control_handler);
  const uint8_t sequence_number  = 3;
  const uint8_t properties_1     = (1 << 7) | (2);
  const uint8_t scene_number     = 1;
  const uint8_t incoming_frame[] = {COMMAND_CLASS_CENTRAL_SCENE_V3,
                                    CENTRAL_SCENE_NOTIFICATION_V3,
                                    sequence_number,
                                    properties_1,
                                    scene_number};

  central_scene_scene_t expected_active_scene
    = ((scene_number - 1) * max_attributes) + (properties_1 & 7) + 1;
  uic_mqtt_dotdot_scenes_command_recall_scene_fields_t expected_fields = {};
  expected_fields.sceneid = expected_active_scene - 1;
  uic_mqtt_dotdot_scenes_publish_generated_recall_scene_command_Expect(
    NULL,
    3,
    &expected_fields);
  uic_mqtt_dotdot_scenes_publish_generated_recall_scene_command_IgnoreArg_unid();

  attribute_timeout_set_callback_ExpectAndReturn(endpoint_id_node,
                                                 60000,
                                                 NULL,
                                                 SL_STATUS_OK);
  attribute_timeout_set_callback_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    central_scene_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Duplicate will not re-start the timer or incoming message
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    central_scene_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Attribute store verifications
  verify_attribute_store_data_notification(expected_active_scene,
                                           sequence_number);
}

void test_zwave_command_class_central_scene_notification_too_short()
{
  TEST_ASSERT_NOT_NULL(central_scene_handler.control_handler);
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_CENTRAL_SCENE_V3, CENTRAL_SCENE_NOTIFICATION_V3};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    central_scene_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));
  // Attribute store verifications
  attribute_store_node_t active_scene_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(ACTIVE_SCENE),
                                             0);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(active_scene_node, REPORTED_ATTRIBUTE));
}

void test_zwave_command_class_central_scene_notification_receive_scene_0()
{
  attribute_store_add_node(ATTRIBUTE(ACTIVE_SCENE), endpoint_id_node);
  central_scene_key_attribute_t max_attributes = 2;
  attribute_store_node_t supported_attributes_node
    = attribute_store_add_node(ATTRIBUTE(MAX_KEY_ATTRIBUTE), endpoint_id_node);
  attribute_store_set_reported(supported_attributes_node,
                               &max_attributes,
                               sizeof(max_attributes));

  TEST_ASSERT_NOT_NULL(central_scene_handler.control_handler);
  const uint8_t sequence_number  = 5;
  const uint8_t properties_1     = (0 << 7) | (2);
  const uint8_t scene_number     = 0;
  const uint8_t incoming_frame[] = {COMMAND_CLASS_CENTRAL_SCENE_V3,
                                    CENTRAL_SCENE_NOTIFICATION_V3,
                                    sequence_number,
                                    properties_1,
                                    scene_number};

  // Scene 0 does not trigger any dotdot callback.
  attribute_timeout_set_callback_ExpectAndReturn(endpoint_id_node,
                                                 5000,
                                                 NULL,
                                                 SL_STATUS_OK);
  attribute_timeout_set_callback_IgnoreArg_callback_function();

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    central_scene_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Attribute store verifications
  verify_attribute_store_data_notification(0, sequence_number);
}

void test_zwave_command_class_central_scene_notification_receive_too_high_key_attribute()
{
  attribute_store_add_node(ATTRIBUTE(ACTIVE_SCENE), endpoint_id_node);
  central_scene_key_attribute_t max_attributes = 2;
  attribute_store_node_t supported_attributes_node
    = attribute_store_add_node(ATTRIBUTE(MAX_KEY_ATTRIBUTE), endpoint_id_node);
  attribute_store_set_reported(supported_attributes_node,
                               &max_attributes,
                               sizeof(max_attributes));

  TEST_ASSERT_NOT_NULL(central_scene_handler.control_handler);
  const uint8_t sequence_number  = 4;
  const uint8_t properties_1     = (1 << 7) | (5);  // key attribute 5, too high
  const uint8_t scene_number     = 3;
  const uint8_t incoming_frame[] = {COMMAND_CLASS_CENTRAL_SCENE_V3,
                                    CENTRAL_SCENE_NOTIFICATION_V3,
                                    sequence_number,
                                    properties_1,
                                    scene_number};

  // Scene is invalid, expect no Scene Notification on DotDot.

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    central_scene_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Attribute store verifications
  verify_attribute_store_data_notification(0, sequence_number);
}

void test_zwave_command_class_central_scene_notification_return_scene_to_idle()
{
  attribute_store_add_node(ATTRIBUTE(ACTIVE_SCENE), endpoint_id_node);
  central_scene_key_attribute_t max_attributes = 4;
  attribute_store_node_t supported_attributes_node
    = attribute_store_add_node(ATTRIBUTE(MAX_KEY_ATTRIBUTE), endpoint_id_node);
  attribute_store_set_reported(supported_attributes_node,
                               &max_attributes,
                               sizeof(max_attributes));

  TEST_ASSERT_NOT_NULL(central_scene_handler.control_handler);
  const uint8_t sequence_number  = 4;
  const uint8_t properties_1     = (1 << 7) | (2);
  const uint8_t scene_number     = 3;
  const uint8_t incoming_frame[] = {COMMAND_CLASS_CENTRAL_SCENE_V3,
                                    CENTRAL_SCENE_NOTIFICATION_V3,
                                    sequence_number,
                                    properties_1,
                                    scene_number};

  central_scene_scene_t expected_active_scene
    = ((scene_number - 1) * max_attributes) + (properties_1 & 7) + 1;
  uic_mqtt_dotdot_scenes_command_recall_scene_fields_t expected_fields = {};
  expected_fields.sceneid = expected_active_scene - 1;
  uic_mqtt_dotdot_scenes_publish_generated_recall_scene_command_Expect(
    NULL,
    3,
    &expected_fields);
  uic_mqtt_dotdot_scenes_publish_generated_recall_scene_command_IgnoreArg_unid();

  attribute_timeout_set_callback_Stub(attribute_timeout_set_callback_stub);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    central_scene_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Attribute store verifications
  verify_attribute_store_data_notification(expected_active_scene,
                                           sequence_number);

  // Now verify that the return to idle would do its job
  TEST_ASSERT_NOT_NULL(return_scene_to_idle);
  return_scene_to_idle(endpoint_id_node);
  verify_attribute_store_data_notification(0, 0);
}