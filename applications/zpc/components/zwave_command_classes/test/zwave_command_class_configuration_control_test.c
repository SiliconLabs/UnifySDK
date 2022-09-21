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
#include "zwave_command_class_configuration.h"
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

// Interface includes
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"
#include "zwave_utils.h"
#include "zwave_controller_types.h"
#include "zwave_command_class_configuration_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "attribute_resolver_mock.h"
#include "zpc_attribute_resolver_mock.h"
#include "zwave_command_handler_mock.h"
#include "zwave_network_management_mock.h"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_##type

// Static variables
static attribute_resolver_function_t configuration_set                = NULL;
static attribute_resolver_function_t configuration_get                = NULL;
static attribute_resolver_function_t configuration_get_next_parameter = NULL;
static attribute_resolver_function_t configuration_name_get           = NULL;
static attribute_resolver_function_t configuration_info_get           = NULL;
static attribute_resolver_function_t configuration_properties_get     = NULL;
static attribute_resolver_function_t configuration_default_reset      = NULL;
static zpc_resolver_event_notification_function_t
  on_configuration_set_default_complete
  = NULL;
static void (*on_next_supported_parameter_id_not_found)(attribute_store_node_t)
  = NULL;

static zwave_command_handler_t configuration_handler      = {};
static zwave_controller_connection_info_t connection_info = {};

static uint8_t received_frame[255]  = {};
static uint16_t received_frame_size = 0;

static uint8_t u8_value   = 0;
static uint32_t u32_value = 0;
static int64_t i64_value  = 0;

// Stub functions
static sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  if (node_type == ATTRIBUTE(NEXT_SUPPORTED_PARAMETER_ID)) {
    TEST_ASSERT_NULL(set_func);
    configuration_get_next_parameter = get_func;
  } else if (node_type == ATTRIBUTE(PARAMETER_NAME)) {
    TEST_ASSERT_NULL(set_func);
    configuration_name_get = get_func;
  } else if (node_type == ATTRIBUTE(PARAMETER_INFO)) {
    TEST_ASSERT_NULL(set_func);
    configuration_info_get = get_func;
  } else if (node_type == ATTRIBUTE(DEFAULT_RESET_REQUESTED)) {
    configuration_default_reset = set_func;
    TEST_ASSERT_NULL(get_func);
  } else if (node_type == ATTRIBUTE(PARAMETER_MIN_VALUE)) {
    TEST_ASSERT_NULL(set_func);
    configuration_properties_get = get_func;
  } else if (node_type == ATTRIBUTE(PARAMETER_VALUE)) {
    configuration_set = set_func;
    configuration_get = get_func;
  } else {
    TEST_FAIL_MESSAGE("Attribute rule registration on a wrong type");
  }
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  configuration_handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    configuration_handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_CONFIGURATION_V4,
                    configuration_handler.command_class);
  TEST_ASSERT_EQUAL(CONFIGURATION_VERSION_V4, configuration_handler.version);
  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_NULL(configuration_handler.support_handler);
  TEST_ASSERT_FALSE(configuration_handler.manual_security_validation);

  return SL_STATUS_OK;
}

static sl_status_t register_send_event_handler_stub(
  attribute_store_type_t type,
  zpc_resolver_event_notification_function_t function,
  int cmock_num_calls)
{
  TEST_ASSERT_EQUAL(ATTRIBUTE(DEFAULT_RESET_REQUESTED), type);
  on_configuration_set_default_complete = function;
  return SL_STATUS_OK;
}

static void attribute_resolver_set_resolution_give_up_listener_stub(
  attribute_store_type_t node_type,
  attribute_resolver_callback_t callback,
  int cmock_num_calls)
{
  TEST_ASSERT_EQUAL(ATTRIBUTE(NEXT_SUPPORTED_PARAMETER_ID), node_type);
  on_next_supported_parameter_id_not_found = callback;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

static void zwave_command_class_configuration_init_verification()
{
  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  register_send_event_handler_Stub(&register_send_event_handler_stub);

  attribute_resolver_set_resolution_give_up_listener_Stub(
    &attribute_resolver_set_resolution_give_up_listener_stub);

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_configuration_init());
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
  received_frame_size                      = 0;
  u8_value                                 = 0;
  u32_value                                = 0;
  i64_value                                = 0;
  configuration_set                        = NULL;
  configuration_get                        = NULL;
  configuration_get_next_parameter         = NULL;
  configuration_name_get                   = NULL;
  configuration_info_get                   = NULL;
  configuration_properties_get             = NULL;
  configuration_default_reset              = NULL;
  on_configuration_set_default_complete    = NULL;
  on_next_supported_parameter_id_not_found = NULL;
  memset(&configuration_handler, 0, sizeof(zwave_command_handler_t));

  zwave_command_class_configuration_init_verification();

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

void test_zwave_command_class_version_2_node_discover_parameter()
{
  // Simulate a version 2 node.
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);

  // Version is undefined, no attributes created just yet.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 2;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  // Both next parameter and bulk support should be added:
  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));

  configuration_bulk_support_t bulk_support = false;
  attribute_store_node_t bulk_support_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(BULK_SUPPORT),
                                             0);
  attribute_store_get_reported(bulk_support_node,
                               &bulk_support,
                               sizeof(bulk_support));

  TEST_ASSERT_TRUE(bulk_support);

  // Try to discover parameter 23:
  attribute_store_node_t next_parameter_node
    = attribute_store_get_node_child_by_type(
      endpoint_id_node,
      ATTRIBUTE(NEXT_SUPPORTED_PARAMETER_ID),
      0);
  configuration_parameter_id_t parameter_id = 23;
  attribute_store_set_desired(next_parameter_node,
                              &parameter_id,
                              sizeof(parameter_id));
  attribute_store_undefine_reported(next_parameter_node);

  // For v2, we discover parameters with a Configuraiton Get
  TEST_ASSERT_NOT_NULL(configuration_get_next_parameter);
  configuration_get_next_parameter(next_parameter_node,
                                   received_frame,
                                   &received_frame_size);

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_CONFIGURATION_V4, CONFIGURATION_GET_V4, parameter_id};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));

  // Now try to discover parameter 256
  parameter_id = 256;
  attribute_store_set_desired(next_parameter_node,
                              &parameter_id,
                              sizeof(parameter_id));
  attribute_store_undefine_reported(next_parameter_node);

  // Here we use Bulk Get for parameters > 255
  TEST_ASSERT_NOT_NULL(configuration_get_next_parameter);
  configuration_get_next_parameter(next_parameter_node,
                                   received_frame,
                                   &received_frame_size);

  const uint8_t expected_frame_2[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                      CONFIGURATION_BULK_GET_V4,
                                      1,
                                      (uint8_t)(parameter_id >> 8),
                                      (uint8_t)(parameter_id & 0xFF)};
  TEST_ASSERT_EQUAL(sizeof(expected_frame_2), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame_2,
                                received_frame,
                                sizeof(expected_frame));

  // Finally, just pretend that Bulk is not supported, and try again
  bulk_support = false;
  attribute_store_set_reported(bulk_support_node,
                               &bulk_support,
                               sizeof(bulk_support));

  TEST_ASSERT_NOT_NULL(configuration_get_next_parameter);
  TEST_ASSERT_EQUAL(SL_STATUS_ALREADY_EXISTS,
                    configuration_get_next_parameter(next_parameter_node,
                                                     received_frame,
                                                     &received_frame_size));

  attribute_store_get_desired(next_parameter_node,
                              &parameter_id,
                              sizeof(parameter_id));
  TEST_ASSERT_EQUAL(0, parameter_id);
  parameter_id = 1;
  attribute_store_get_reported(next_parameter_node,
                               &parameter_id,
                               sizeof(parameter_id));
  TEST_ASSERT_EQUAL(0, parameter_id);
}

void test_zwave_command_class_version_3_node_discover_parameter()
{
  // Simulate a version 3 node.
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);

  // Version is undefined, no attributes created just yet.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 3;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  // Both next parameter and bulk support should be added:
  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));

  configuration_bulk_support_t bulk_support = false;
  attribute_store_node_t bulk_support_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(BULK_SUPPORT),
                                             0);
  attribute_store_get_reported(bulk_support_node,
                               &bulk_support,
                               sizeof(bulk_support));

  TEST_ASSERT_TRUE(bulk_support);

  // Try to discover parameter 23:
  attribute_store_node_t next_parameter_node
    = attribute_store_get_node_child_by_type(
      endpoint_id_node,
      ATTRIBUTE(NEXT_SUPPORTED_PARAMETER_ID),
      0);
  configuration_parameter_id_t parameter_id = 23;
  attribute_store_set_desired(next_parameter_node,
                              &parameter_id,
                              sizeof(parameter_id));
  attribute_store_undefine_reported(next_parameter_node);

  // For v3, we discover parameters with a Configuraiton Get
  TEST_ASSERT_NOT_NULL(configuration_get_next_parameter);
  configuration_get_next_parameter(next_parameter_node,
                                   received_frame,
                                   &received_frame_size);

  const uint8_t expected_frame[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                    CONFIGURATION_PROPERTIES_GET_V4,
                                    (uint8_t)(parameter_id >> 8),
                                    (uint8_t)(parameter_id & 0xFF)};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));

  // Now try to discover parameter 256
  parameter_id = 256;
  attribute_store_set_desired(next_parameter_node,
                              &parameter_id,
                              sizeof(parameter_id));
  attribute_store_undefine_reported(next_parameter_node);

  // Here we use Bulk Get for parameters > 255, still should be using Properties Get
  TEST_ASSERT_NOT_NULL(configuration_get_next_parameter);
  configuration_get_next_parameter(next_parameter_node,
                                   received_frame,
                                   &received_frame_size);

  const uint8_t expected_frame_2[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                      CONFIGURATION_PROPERTIES_GET_V4,
                                      (uint8_t)(parameter_id >> 8),
                                      (uint8_t)(parameter_id & 0xFF)};
  TEST_ASSERT_EQUAL(sizeof(expected_frame_2), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame_2,
                                received_frame,
                                sizeof(expected_frame));
}

void test_zwave_command_class_version_8_node_configuration_set()
{
  // Simulate a version 8 node. (does not exist at time of writing this code.)
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);

  // Version is undefined, no attributes created just yet.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 8;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  // Both next parameter and bulk support, default reset should be added:
  TEST_ASSERT_EQUAL(4, attribute_store_get_node_child_count(endpoint_id_node));

  // Simulate an incoming report with Configuration parameter 13:
  configuration_parameter_id_t parameter_id       = 13;
  configuration_parameter_size_t parameter_size   = 1;
  configuration_parameter_value_t parameter_value = -120;
  const uint8_t incoming_frame[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                    CONFIGURATION_REPORT_V4,
                                    (uint8_t)parameter_id,
                                    (uint8_t)parameter_size,
                                    (uint8_t)(parameter_value)};

  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  attribute_store_node_t parameter_id_node
    = attribute_store_get_node_child_by_value(endpoint_id_node,
                                              ATTRIBUTE(PARAMETER_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&parameter_id,
                                              sizeof(parameter_id),
                                              0);
  attribute_store_node_t parameter_value_node
    = attribute_store_get_node_child_by_type(parameter_id_node,
                                             ATTRIBUTE(PARAMETER_VALUE),
                                             0);

  parameter_value = -20;
  attribute_store_set_desired(parameter_value_node,
                              &parameter_value,
                              sizeof(parameter_value));

  // Ask the Command class handler to make a frame for this:
  TEST_ASSERT_NOT_NULL(configuration_set);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_set(parameter_value_node,
                                      received_frame,
                                      &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                    CONFIGURATION_SET_V4,
                                    (uint8_t)parameter_id,
                                    (uint8_t)parameter_size,
                                    (int8_t)(parameter_value)};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_zwave_command_class_version_1_node_configuration_bulk_set_not_supported()
{
  // Simulate a version 1 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);

  // Version is undefined, no attributes created just yet.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  // Both next parameter and bulk support should be added:
  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));

  // Create parameter ID 256
  configuration_parameter_id_t parameter_id = 256;
  attribute_store_node_t parameter_id_node
    = attribute_store_add_node(ATTRIBUTE(PARAMETER_ID), endpoint_id_node);
  attribute_store_set_reported(parameter_id_node,
                               &parameter_id,
                               sizeof(parameter_id));
  attribute_store_node_t parameter_value_node
    = attribute_store_add_node(ATTRIBUTE(PARAMETER_VALUE), parameter_id_node);

  // Ask the Command class handler to make a frame for this, it should
  // just delete this bogus parameter id
  TEST_ASSERT_NOT_NULL(configuration_set);
  TEST_ASSERT_EQUAL(SL_STATUS_ALREADY_EXISTS,
                    configuration_set(parameter_value_node,
                                      received_frame,
                                      &received_frame_size));

  TEST_ASSERT_EQUAL(0, received_frame_size);
  TEST_ASSERT_FALSE(attribute_store_node_exists(parameter_value_node));
  TEST_ASSERT_FALSE(attribute_store_node_exists(parameter_id_node));
}

void test_zwave_command_class_version_2_node_configuration_bulk_set()
{
  // Simulate a version 2 node.
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);

  // Version is undefined, no attributes created just yet.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 2;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  // Both next parameter and bulk support should be added:
  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));

  // Simulate an incoming report with Configuration parameter 13:
  configuration_parameter_id_t parameter_id       = 1213;
  configuration_parameter_size_t parameter_size   = 2;
  configuration_parameter_value_t parameter_value = -12120;
  const uint8_t incoming_frame[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                    CONFIGURATION_BULK_REPORT_V4,
                                    (uint8_t)(parameter_id >> 8),
                                    (uint8_t)(parameter_id & 0xFF),
                                    1,
                                    20,  // 20 reports to follow
                                    (uint8_t)parameter_size,
                                    (int8_t)(parameter_value >> 8),
                                    (int8_t)(parameter_value & 0xFF)};

  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  attribute_store_node_t parameter_id_node
    = attribute_store_get_node_child_by_value(endpoint_id_node,
                                              ATTRIBUTE(PARAMETER_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&parameter_id,
                                              sizeof(parameter_id),
                                              0);
  attribute_store_node_t parameter_value_node
    = attribute_store_get_node_child_by_type(parameter_id_node,
                                             ATTRIBUTE(PARAMETER_VALUE),
                                             0);

  parameter_value = -20;
  attribute_store_set_desired(parameter_value_node,
                              &parameter_value,
                              sizeof(parameter_value));

  // Ask the Command class handler to make a frame for this:
  TEST_ASSERT_NOT_NULL(configuration_set);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_set(parameter_value_node,
                                      received_frame,
                                      &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                    CONFIGURATION_BULK_SET_V4,
                                    (uint8_t)(parameter_id >> 8),
                                    (uint8_t)(parameter_id & 0xFF),
                                    1,
                                    (uint8_t)parameter_size,
                                    (int8_t)(parameter_value >> 8),
                                    (int8_t)(parameter_value & 0xFF)};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_zwave_command_class_version_1_node_configuration_get()
{
  // Simulate a version 1 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);

  // Version is undefined, no attributes created just yet.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  // Both next parameter and bulk support should be added:
  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));

  // Create parameter ID 255
  configuration_parameter_id_t parameter_id = 255;
  attribute_store_node_t parameter_id_node
    = attribute_store_add_node(ATTRIBUTE(PARAMETER_ID), endpoint_id_node);
  attribute_store_set_reported(parameter_id_node,
                               &parameter_id,
                               sizeof(parameter_id));
  attribute_store_node_t parameter_value_node
    = attribute_store_add_node(ATTRIBUTE(PARAMETER_VALUE), parameter_id_node);

  // Ask the Command class handler to resolver the value of the parameter
  TEST_ASSERT_NOT_NULL(configuration_get);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_get(parameter_value_node,
                                      received_frame,
                                      &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                    CONFIGURATION_GET_V4,
                                    (uint8_t)(parameter_id)};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_zwave_command_class_version_2_node_configuration_bulk_get()
{
  // Simulate a version 2 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);

  // Version is undefined, no attributes created just yet.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 2;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  // Both next parameter and bulk support should be added:
  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));

  // Create parameter ID 256
  configuration_parameter_id_t parameter_id = 256;
  attribute_store_node_t parameter_id_node
    = attribute_store_add_node(ATTRIBUTE(PARAMETER_ID), endpoint_id_node);
  attribute_store_set_reported(parameter_id_node,
                               &parameter_id,
                               sizeof(parameter_id));
  attribute_store_node_t parameter_value_node
    = attribute_store_add_node(ATTRIBUTE(PARAMETER_VALUE), parameter_id_node);

  // Ask the Command class handler to resolver the value of the parameter
  TEST_ASSERT_NOT_NULL(configuration_get);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_get(parameter_value_node,
                                      received_frame,
                                      &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                    CONFIGURATION_BULK_GET_V4,
                                    (uint8_t)(parameter_id >> 8),
                                    (uint8_t)(parameter_id & 0xFF),
                                    1};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_zwave_command_class_version_1_node_configuration_bulk_get_no_support()
{
  // Simulate a version 1 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);

  // Version is undefined, no attributes created just yet.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  // Both next parameter and bulk support should be added:
  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));

  // Create parameter ID 256
  configuration_parameter_id_t parameter_id = 256;
  attribute_store_node_t parameter_id_node
    = attribute_store_add_node(ATTRIBUTE(PARAMETER_ID), endpoint_id_node);
  attribute_store_set_reported(parameter_id_node,
                               &parameter_id,
                               sizeof(parameter_id));
  attribute_store_node_t parameter_value_node
    = attribute_store_add_node(ATTRIBUTE(PARAMETER_VALUE), parameter_id_node);

  // Ask the Command class handler to resolver the value of the parameter.
  // It will reject since it is >255 and the node does not support it.
  TEST_ASSERT_NOT_NULL(configuration_get);
  TEST_ASSERT_EQUAL(SL_STATUS_ALREADY_EXISTS,
                    configuration_get(parameter_value_node,
                                      received_frame,
                                      &received_frame_size));

  TEST_ASSERT_EQUAL(0, received_frame_size);
  TEST_ASSERT_FALSE(attribute_store_node_exists(parameter_value_node));
  TEST_ASSERT_FALSE(attribute_store_node_exists(parameter_id_node));
}

void test_zwave_command_class_version_3_node_receive_properties_report_happy_case()
{
  // Simulate a version 3 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);

  // Version is undefined, no attributes created just yet.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 3;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  // Both next parameter and bulk support should be added:
  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(endpoint_id_node));

  configuration_parameter_id_t parameter_id         = 13;
  configuration_parameter_id_t next_parameter_id    = 23;
  configuration_parameter_size_t parameter_size     = 4;
  int32_t parameter_min_value                       = -500000;
  int32_t parameter_max_value                       = 42;
  int32_t parameter_default_value                   = 1;
  configuration_parameter_format_t parameter_format = 0;
  uint8_t incoming_frame[]
    = {COMMAND_CLASS_CONFIGURATION_V4,
       CONFIGURATION_PROPERTIES_REPORT_V4,
       (uint8_t)(parameter_id >> 8),
       (uint8_t)(parameter_id & 0xFF),
       (parameter_format << 3) | parameter_size,
       (parameter_min_value >> 24) & 0xFF,  // Min value MSB
       (parameter_min_value >> 16) & 0xFF,
       (parameter_min_value >> 8) & 0xFF,
       parameter_min_value & 0xFF,          // Min value LSB
       (parameter_max_value >> 24) & 0xFF,  // Max value MSB
       (parameter_max_value >> 16) & 0xFF,
       (parameter_max_value >> 8) & 0xFF,
       parameter_max_value & 0xFF,              // Max value LSB
       (parameter_default_value >> 24) & 0xFF,  // Default value MSB
       (parameter_default_value >> 16) & 0xFF,
       (parameter_default_value >> 8) & 0xFF,
       parameter_default_value & 0xFF,  // Default value LSB
       (uint8_t)(next_parameter_id >> 8),
       (uint8_t)(next_parameter_id & 0xFF)};

  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Go around and check that attribute store looks as expected.
  attribute_store_node_t parameter_id_node
    = attribute_store_get_node_child_by_value(endpoint_id_node,
                                              ATTRIBUTE(PARAMETER_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&parameter_id,
                                              sizeof(parameter_id),
                                              0);

  // Check that the Min Value was accepted:
  attribute_store_node_t parameter_min_value_node
    = attribute_store_get_node_child_by_type(parameter_id_node,
                                             ATTRIBUTE(PARAMETER_MIN_VALUE),
                                             0);
  configuration_parameter_value_t received_value = 0;
  attribute_store_get_reported(parameter_min_value_node,
                               &received_value,
                               sizeof(received_value));
  TEST_ASSERT_EQUAL_INT32(parameter_min_value, (int32_t)received_value);

  // Check that the Max Value was accepted:
  attribute_store_node_t parameter_max_value_node
    = attribute_store_get_node_child_by_type(parameter_id_node,
                                             ATTRIBUTE(PARAMETER_MAX_VALUE),
                                             0);
  received_value = 0;
  attribute_store_get_reported(parameter_max_value_node,
                               &received_value,
                               sizeof(received_value));
  TEST_ASSERT_EQUAL(parameter_max_value, received_value);

  // Check that the Default Value was accepted:
  attribute_store_node_t parameter_default_value_node
    = attribute_store_get_node_child_by_type(parameter_id_node,
                                             ATTRIBUTE(PARAMETER_DEFAULT_VALUE),
                                             0);
  received_value = 0;
  attribute_store_get_reported(parameter_default_value_node,
                               &received_value,
                               sizeof(received_value));
  TEST_ASSERT_EQUAL(parameter_default_value, received_value);

  // Check that the Next ID was processed.:
  attribute_store_node_t next_id_node = attribute_store_get_node_child_by_type(
    endpoint_id_node,
    ATTRIBUTE(NEXT_SUPPORTED_PARAMETER_ID),
    0);
  configuration_parameter_id_t received_id = 0;
  attribute_store_get_desired(next_id_node, &received_id, sizeof(received_id));
  TEST_ASSERT_EQUAL(next_parameter_id, received_id);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(next_id_node, REPORTED_ATTRIBUTE));
}

void test_zwave_command_class_version_4_node_receive_properties_report_happy_case()
{
  // Simulate a version 4 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);
  zwave_cc_version_t version = 4;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  configuration_parameter_id_t parameter_id            = 13;
  configuration_parameter_id_t next_parameter_id       = 23;
  configuration_parameter_size_t parameter_size        = 2;
  int16_t parameter_min_value                          = -3214;
  int16_t parameter_max_value                          = -35;
  int16_t parameter_default_value                      = -100;
  configuration_parameter_format_t parameter_format    = 0;
  configuration_parameter_flag_t read_only             = true;
  configuration_parameter_flag_t altering_capabilities = true;
  configuration_parameter_flag_t advanced              = true;
  configuration_parameter_flag_t no_bulk_support       = true;
  uint8_t incoming_frame[]
    = {COMMAND_CLASS_CONFIGURATION_V4,
       CONFIGURATION_PROPERTIES_REPORT_V4,
       (uint8_t)(parameter_id >> 8),
       (uint8_t)(parameter_id & 0xFF),
       (altering_capabilities << 7) | (read_only << 6) | (parameter_format << 3)
         | parameter_size,
       (parameter_min_value >> 8) & 0xFF,      // Min value MSB
       parameter_min_value & 0xFF,             // Min value LSB
       (parameter_max_value >> 8) & 0xFF,      // Max value MSB
       parameter_max_value & 0xFF,             // Max value LSB
       (parameter_default_value >> 8) & 0xFF,  // Default value MSB
       parameter_default_value & 0xFF,         // Default value LSB
       (uint8_t)(next_parameter_id >> 8),
       (uint8_t)(next_parameter_id & 0xFF),
       ((no_bulk_support << 1) | advanced),
       0x23};

  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Go around and check that attribute store looks as expected.
  attribute_store_node_t parameter_id_node
    = attribute_store_get_node_child_by_value(endpoint_id_node,
                                              ATTRIBUTE(PARAMETER_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&parameter_id,
                                              sizeof(parameter_id),
                                              0);

  // Check that the Min Value was accepted:
  attribute_store_node_t parameter_min_value_node
    = attribute_store_get_node_child_by_type(parameter_id_node,
                                             ATTRIBUTE(PARAMETER_MIN_VALUE),
                                             0);
  configuration_parameter_value_t received_value = 0;
  attribute_store_get_reported(parameter_min_value_node,
                               &received_value,
                               sizeof(received_value));
  TEST_ASSERT_EQUAL_INT16(parameter_min_value, (int16_t)received_value);

  // Check that the Max Value was accepted:
  attribute_store_node_t parameter_max_value_node
    = attribute_store_get_node_child_by_type(parameter_id_node,
                                             ATTRIBUTE(PARAMETER_MAX_VALUE),
                                             0);
  received_value = 0;
  attribute_store_get_reported(parameter_max_value_node,
                               &received_value,
                               sizeof(received_value));
  TEST_ASSERT_EQUAL_INT16(parameter_max_value, (int16_t)received_value);

  // Check that the Default Value was accepted:
  attribute_store_node_t parameter_default_value_node
    = attribute_store_get_node_child_by_type(parameter_id_node,
                                             ATTRIBUTE(PARAMETER_DEFAULT_VALUE),
                                             0);
  received_value = 0;
  attribute_store_get_reported(parameter_default_value_node,
                               &received_value,
                               sizeof(received_value));
  TEST_ASSERT_EQUAL_INT16(parameter_default_value, (int16_t)received_value);

  // Check that the Next ID was processed :
  attribute_store_node_t next_id_node = attribute_store_get_node_child_by_type(
    endpoint_id_node,
    ATTRIBUTE(NEXT_SUPPORTED_PARAMETER_ID),
    0);
  configuration_parameter_id_t received_id = 0;
  attribute_store_get_desired(next_id_node, &received_id, sizeof(received_id));
  TEST_ASSERT_EQUAL(next_parameter_id, received_id);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(next_id_node, REPORTED_ATTRIBUTE));

  // Check the read-only flag:
  attribute_store_node_t read_only_node
    = attribute_store_get_node_child_by_type(parameter_id_node,
                                             ATTRIBUTE(PARAMETER_READ_ONLY),
                                             0);
  configuration_parameter_flag_t received_flag = false;
  attribute_store_get_reported(read_only_node,
                               &received_flag,
                               sizeof(received_flag));
  TEST_ASSERT_EQUAL(read_only, received_flag);

  // Check the advanced flag:
  attribute_store_node_t advanced_node
    = attribute_store_get_node_child_by_type(parameter_id_node,
                                             ATTRIBUTE(PARAMETER_ADVANCED),
                                             0);
  received_flag = false;
  attribute_store_get_reported(advanced_node,
                               &received_flag,
                               sizeof(received_flag));
  TEST_ASSERT_EQUAL(advanced, received_flag);

  // Check the altering capabilities flag:
  attribute_store_node_t altering_capabilities_node
    = attribute_store_get_node_child_by_type(
      parameter_id_node,
      ATTRIBUTE(PARAMETER_ALTERING_CAPABILITIES),
      0);
  received_flag = false;
  attribute_store_get_reported(altering_capabilities_node,
                               &received_flag,
                               sizeof(received_flag));
  TEST_ASSERT_EQUAL(altering_capabilities, received_flag);

  // Check the bulk support flag:
  attribute_store_node_t bulk_support_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(BULK_SUPPORT),
                                             0);
  configuration_bulk_support_t received_bulk_support = false;
  attribute_store_get_reported(bulk_support_node,
                               &received_bulk_support,
                               sizeof(received_bulk_support));
  // flag should be inverted:
  TEST_ASSERT_EQUAL(!received_bulk_support, no_bulk_support);
}

void test_zwave_command_class_version_4_node_receive_properties_report_size_0()
{
  // Simulate a version 3 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);

  // Version is undefined, no attributes created just yet.
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 3;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  configuration_parameter_id_t parameter_id = 256;
  attribute_store_node_t parameter_id_node
    = attribute_store_add_node(ATTRIBUTE(PARAMETER_ID), endpoint_id_node);
  attribute_store_set_reported(parameter_id_node,
                               &parameter_id,
                               sizeof(parameter_id));

  configuration_parameter_id_t next_parameter_id = 0;
  configuration_parameter_size_t parameter_size  = 0;

  const uint8_t incoming_frame[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                    CONFIGURATION_PROPERTIES_REPORT_V4,
                                    (uint8_t)(parameter_id >> 8),
                                    (uint8_t)(parameter_id & 0xFF),
                                    parameter_size,
                                    (uint8_t)(next_parameter_id >> 8),
                                    (uint8_t)(next_parameter_id & 0xFF)};

  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Parameter should have been deleted:
  TEST_ASSERT_FALSE(attribute_store_node_exists(parameter_id_node));

  // Check that the Next ID was processed. If Next ID is to, set the reported value too.
  attribute_store_node_t next_id_node = attribute_store_get_node_child_by_type(
    endpoint_id_node,
    ATTRIBUTE(NEXT_SUPPORTED_PARAMETER_ID),
    0);
  configuration_parameter_id_t received_id = 20;
  attribute_store_get_desired(next_id_node, &received_id, sizeof(received_id));
  TEST_ASSERT_EQUAL(next_parameter_id, received_id);
  TEST_ASSERT_TRUE(attribute_store_is_value_matched(next_id_node));
}

void test_zwave_command_class_version_1_node_receive_report_size_0()
{
  // Simulate a version 1 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);
  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  configuration_parameter_id_t parameter_id     = 8;
  configuration_parameter_size_t parameter_size = 0;
  attribute_store_node_t parameter_id_node
    = attribute_store_add_node(ATTRIBUTE(PARAMETER_ID), endpoint_id_node);
  attribute_store_set_reported(parameter_id_node,
                               &parameter_id,
                               sizeof(parameter_id));

  const uint8_t incoming_frame[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                    CONFIGURATION_REPORT_V4,
                                    (uint8_t)(parameter_id),
                                    parameter_size};

  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Parameter should have been deleted:
  TEST_ASSERT_FALSE(attribute_store_node_exists(parameter_id_node));
}

void test_zwave_command_class_version_2_node_receive_bulk_report_size_0()
{
  // Simulate a version 1 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);
  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  configuration_parameter_id_t parameter_id_1   = 12;
  configuration_parameter_id_t parameter_id_2   = 13;
  configuration_parameter_size_t parameter_size = 0;
  attribute_store_node_t parameter_id_node_1
    = attribute_store_add_node(ATTRIBUTE(PARAMETER_ID), endpoint_id_node);
  attribute_store_set_reported(parameter_id_node_1,
                               &parameter_id_1,
                               sizeof(parameter_id_1));
  attribute_store_node_t parameter_id_node_2
    = attribute_store_add_node(ATTRIBUTE(PARAMETER_ID), endpoint_id_node);
  attribute_store_set_reported(parameter_id_node_2,
                               &parameter_id_2,
                               sizeof(parameter_id_2));

  const uint8_t incoming_frame[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                    CONFIGURATION_BULK_REPORT_V4,
                                    (uint8_t)(parameter_id_1 >> 8),
                                    (uint8_t)(parameter_id_1 & 0xFF),
                                    2,  // 2 parameters
                                    0,
                                    (uint8_t)parameter_size};
  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // both Parameters should have been deleted:
  TEST_ASSERT_FALSE(attribute_store_node_exists(parameter_id_node_1));
  TEST_ASSERT_FALSE(attribute_store_node_exists(parameter_id_node_2));
}

void test_zwave_command_class_version_3_node_receive_info_report()
{
  // Simulate a version 3 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);
  zwave_cc_version_t version = 3;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  configuration_parameter_id_t parameter_id       = 121;
  configuration_parameter_size_t parameter_size   = 1;
  configuration_parameter_size_t parameter_format = 1;
  configuration_parameter_value_t parameter_value = 42;
  const uint8_t incoming_frame[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                    CONFIGURATION_REPORT_V4,
                                    (uint8_t)parameter_id,
                                    (uint8_t)parameter_size,
                                    (int8_t)((parameter_value >> 0) & 0xFF)};

  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Check that the parameter is created:
  attribute_store_node_t parameter_id_node
    = attribute_store_get_node_child_by_value(endpoint_id_node,
                                              ATTRIBUTE(PARAMETER_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&parameter_id,
                                              sizeof(parameter_id),
                                              0);

  attribute_store_node_t parameter_min_value_node
    = attribute_store_get_node_child_by_type(parameter_id_node,
                                             ATTRIBUTE(PARAMETER_MIN_VALUE),
                                             0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, parameter_min_value_node);

  // Now ask a properties Get for a v3 node. It will give us a frame
  TEST_ASSERT_NOT_NULL(configuration_properties_get);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_properties_get(parameter_min_value_node,
                                                 received_frame,
                                                 &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                    CONFIGURATION_PROPERTIES_GET_V4,
                                    (parameter_id >> 8),
                                    (parameter_id & 0xFF)};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));

  // Simulate an answer:
  configuration_parameter_value_t parameter_min_value     = 0;
  configuration_parameter_value_t parameter_max_value     = 200;
  configuration_parameter_value_t parameter_default_value = 123;
  const uint8_t incoming_frame_2[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                      CONFIGURATION_PROPERTIES_REPORT_V4,
                                      (parameter_id >> 8),
                                      (parameter_id & 0xFF),
                                      parameter_size | (parameter_format << 3),
                                      parameter_min_value,
                                      parameter_max_value,
                                      parameter_default_value,
                                      0,
                                      0};

  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame_2,
                                          sizeof(incoming_frame_2)));

  // Check if the values we expect are here:
  // Min value
  configuration_parameter_value_t received_value = 0;
  attribute_store_get_reported(parameter_min_value_node,
                               &received_value,
                               sizeof(received_value));
  TEST_ASSERT_EQUAL(parameter_min_value, received_value);

  // Max value
  attribute_store_node_t parameter_max_value_node
    = attribute_store_get_node_child_by_type(parameter_id_node,
                                             ATTRIBUTE(PARAMETER_MAX_VALUE),
                                             0);

  attribute_store_get_reported(parameter_max_value_node,
                               &received_value,
                               sizeof(received_value));
  TEST_ASSERT_EQUAL(parameter_max_value, received_value);

  // Default
  attribute_store_node_t parameter_default_value_node
    = attribute_store_get_node_child_by_type(parameter_id_node,
                                             ATTRIBUTE(PARAMETER_DEFAULT_VALUE),
                                             0);

  attribute_store_get_reported(parameter_default_value_node,
                               &received_value,
                               sizeof(received_value));
  TEST_ASSERT_EQUAL(parameter_default_value, received_value);
}

void test_zwave_command_class_version_1_node_receive_report_set_default_values()
{
  // Simulate a version 1 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);
  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  configuration_parameter_id_t parameter_id     = 105;
  configuration_parameter_size_t parameter_size = 4;
  int32_t parameter_value                       = 1234567;
  uint8_t incoming_frame[]                      = {
    COMMAND_CLASS_CONFIGURATION_V4,
    CONFIGURATION_REPORT_V4,
    (uint8_t)parameter_id,
    (uint8_t)parameter_size,
    (parameter_value >> 24) & 0xFF,  // value MSB
    (parameter_value >> 16) & 0xFF,
    (parameter_value >> 8) & 0xFF,
    parameter_value & 0xFF,  // value LSB
  };

  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Check that the parameter is created:
  attribute_store_node_t parameter_id_node
    = attribute_store_get_node_child_by_value(endpoint_id_node,
                                              ATTRIBUTE(PARAMETER_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&parameter_id,
                                              sizeof(parameter_id),
                                              0);

  attribute_store_node_t parameter_min_value_node
    = attribute_store_get_node_child_by_type(parameter_id_node,
                                             ATTRIBUTE(PARAMETER_MIN_VALUE),
                                             0);

  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, parameter_min_value_node);

  // Now ask a properties Get for a v1 node. It will just fill the data automatically
  TEST_ASSERT_NOT_NULL(configuration_properties_get);
  TEST_ASSERT_EQUAL(SL_STATUS_ALREADY_EXISTS,
                    configuration_properties_get(parameter_min_value_node,
                                                 received_frame,
                                                 &received_frame_size));

  // Check if the values we expect are here:
  // Min value
  configuration_parameter_value_t received_value = 0;
  attribute_store_get_reported(parameter_min_value_node,
                               &received_value,
                               sizeof(received_value));
  TEST_ASSERT_EQUAL(INT32_MIN, received_value);

  // Max value
  attribute_store_node_t parameter_max_value_node
    = attribute_store_get_node_child_by_type(parameter_id_node,
                                             ATTRIBUTE(PARAMETER_MAX_VALUE),
                                             0);

  attribute_store_get_reported(parameter_max_value_node,
                               &received_value,
                               sizeof(received_value));
  TEST_ASSERT_EQUAL(INT32_MAX, received_value);

  // Default
  attribute_store_node_t parameter_default_value_node
    = attribute_store_get_node_child_by_type(parameter_id_node,
                                             ATTRIBUTE(PARAMETER_DEFAULT_VALUE),
                                             0);

  attribute_store_get_reported(parameter_default_value_node,
                               &received_value,
                               sizeof(received_value));
  TEST_ASSERT_EQUAL_INT32(parameter_value, (int32_t)received_value);
}

void test_zwave_command_class_version_1_node_name_report()
{
  // Simulate a version 1 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);
  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  configuration_parameter_id_t parameter_id       = 121;
  configuration_parameter_size_t parameter_size   = 1;
  configuration_parameter_size_t parameter_format = 1;
  configuration_parameter_value_t parameter_value = 42;
  const uint8_t incoming_frame[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                    CONFIGURATION_REPORT_V4,
                                    (uint8_t)parameter_id,
                                    parameter_size | (parameter_format << 3),
                                    (int8_t)((parameter_value >> 0) & 0xFF)};

  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Check that the parameter is created:
  attribute_store_node_t parameter_id_node
    = attribute_store_get_node_child_by_value(endpoint_id_node,
                                              ATTRIBUTE(PARAMETER_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&parameter_id,
                                              sizeof(parameter_id),
                                              0);

  attribute_store_node_t parameter_name_node
    = attribute_store_get_node_child_by_type(parameter_id_node,
                                             ATTRIBUTE(PARAMETER_NAME),
                                             0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, parameter_name_node);

  // Now ask a Name Get for a v1 node. It will fill the data automatically
  TEST_ASSERT_NOT_NULL(configuration_name_get);
  TEST_ASSERT_EQUAL(SL_STATUS_ALREADY_EXISTS,
                    configuration_name_get(parameter_name_node,
                                           received_frame,
                                           &received_frame_size));

  // Copy the string into our "frame buffer"
  attribute_store_get_reported_string(parameter_name_node,
                                      (char *)received_frame,
                                      sizeof(received_frame));

  TEST_ASSERT_EQUAL_STRING("Parameter 121", received_frame);
}

void test_zwave_command_class_version_1_node_info_report()
{
  // Simulate a version 1 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);
  zwave_cc_version_t version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  configuration_parameter_id_t parameter_id       = 121;
  configuration_parameter_size_t parameter_size   = 1;
  configuration_parameter_size_t parameter_format = 1;
  configuration_parameter_value_t parameter_value = 42;
  const uint8_t incoming_frame[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                    CONFIGURATION_REPORT_V4,
                                    (uint8_t)parameter_id,
                                    parameter_size | (parameter_format << 3),
                                    (int8_t)((parameter_value >> 0) & 0xFF)};

  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Check that the parameter is created:
  attribute_store_node_t parameter_id_node
    = attribute_store_get_node_child_by_value(endpoint_id_node,
                                              ATTRIBUTE(PARAMETER_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&parameter_id,
                                              sizeof(parameter_id),
                                              0);

  attribute_store_node_t parameter_info_node
    = attribute_store_get_node_child_by_type(parameter_id_node,
                                             ATTRIBUTE(PARAMETER_INFO),
                                             0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, parameter_info_node);

  // Now ask a Info Get for a v1 node. It will fill the data automatically
  TEST_ASSERT_NOT_NULL(configuration_info_get);
  TEST_ASSERT_EQUAL(SL_STATUS_ALREADY_EXISTS,
                    configuration_info_get(parameter_info_node,
                                           received_frame,
                                           &received_frame_size));

  // Copy the string into our "frame buffer"
  attribute_store_get_reported_string(parameter_info_node,
                                      (char *)received_frame,
                                      sizeof(received_frame));

  TEST_ASSERT_EQUAL_STRING("No information available", received_frame);
}

void test_zwave_command_class_version_3_node_info_report()
{
  // Simulate a version 3 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);
  zwave_cc_version_t version = 3;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  configuration_parameter_id_t parameter_id       = 32;
  configuration_parameter_size_t parameter_size   = 1;
  configuration_parameter_size_t parameter_format = 1;
  configuration_parameter_value_t parameter_value = 42;
  const uint8_t incoming_frame[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                    CONFIGURATION_REPORT_V4,
                                    (uint8_t)parameter_id,
                                    parameter_size | (parameter_format << 3),
                                    (int8_t)((parameter_value >> 0) & 0xFF)};

  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Check that the parameter is created:
  attribute_store_node_t parameter_id_node
    = attribute_store_get_node_child_by_value(endpoint_id_node,
                                              ATTRIBUTE(PARAMETER_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&parameter_id,
                                              sizeof(parameter_id),
                                              0);

  attribute_store_node_t parameter_info_node
    = attribute_store_get_node_child_by_type(parameter_id_node,
                                             ATTRIBUTE(PARAMETER_INFO),
                                             0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, parameter_info_node);

  // Now ask a info Get for a v3 node.
  TEST_ASSERT_NOT_NULL(configuration_info_get);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_info_get(parameter_info_node,
                                           received_frame,
                                           &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                    CONFIGURATION_INFO_GET_V4,
                                    parameter_id >> 8,
                                    parameter_id & 0xFF};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));

  // Simulate an answer
  const uint8_t incoming_frame_2[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                      CONFIGURATION_INFO_REPORT_V4,
                                      parameter_id >> 8,
                                      parameter_id & 0xFF,
                                      1,
                                      'F',
                                      'u',
                                      'n',
                                      'n',
                                      'y',
                                      ' ',
                                      'P',
                                      'a',
                                      'r',
                                      'a',
                                      'm'};

  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame_2,
                                          sizeof(incoming_frame_2)));

  // Copy the string into our "frame buffer" to check its value.
  attribute_store_get_reported_string(parameter_info_node,
                                      (char *)received_frame,
                                      sizeof(received_frame));

  // Simulate a report to follow
  const uint8_t incoming_frame_3[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                      CONFIGURATION_INFO_REPORT_V4,
                                      parameter_id >> 8,
                                      parameter_id & 0xFF,
                                      0,
                                      'e',
                                      't',
                                      'e',
                                      'r',
                                      ' ',
                                      ':',
                                      ')'};

  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame_3,
                                          sizeof(incoming_frame_3)));

  // Copy the string into our "frame buffer" to check its value.
  attribute_store_get_reported_string(parameter_info_node,
                                      (char *)received_frame,
                                      sizeof(received_frame));

  TEST_ASSERT_EQUAL_STRING("Funny Parameter :)", received_frame);
}

void test_zwave_command_class_version_3_node_name_report()
{
  // Simulate a version 3 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);
  zwave_cc_version_t version = 3;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  configuration_parameter_id_t parameter_id       = 32;
  configuration_parameter_size_t parameter_size   = 1;
  configuration_parameter_size_t parameter_format = 1;
  configuration_parameter_value_t parameter_value = 42;
  const uint8_t incoming_frame[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                    CONFIGURATION_REPORT_V4,
                                    (uint8_t)parameter_id,
                                    parameter_size | (parameter_format << 3),
                                    (int8_t)((parameter_value >> 0) & 0xFF)};

  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Check that the parameter is created:
  attribute_store_node_t parameter_id_node
    = attribute_store_get_node_child_by_value(endpoint_id_node,
                                              ATTRIBUTE(PARAMETER_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&parameter_id,
                                              sizeof(parameter_id),
                                              0);

  attribute_store_node_t parameter_name_node
    = attribute_store_get_node_child_by_type(parameter_id_node,
                                             ATTRIBUTE(PARAMETER_NAME),
                                             0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, parameter_name_node);

  // Now ask a name Get for a v3 node.
  TEST_ASSERT_NOT_NULL(configuration_name_get);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_name_get(parameter_name_node,
                                           received_frame,
                                           &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                    CONFIGURATION_NAME_GET_V4,
                                    parameter_id >> 8,
                                    parameter_id & 0xFF};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));

  // Simulate an answer
  const uint8_t incoming_frame_2[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                      CONFIGURATION_NAME_REPORT_V4,
                                      parameter_id >> 8,
                                      parameter_id & 0xFF,
                                      1,
                                      'F',
                                      'u',
                                      'n',
                                      'n',
                                      'y',
                                      ' ',
                                      'P',
                                      'a',
                                      'r',
                                      'a',
                                      'm'};

  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame_2,
                                          sizeof(incoming_frame_2)));

  // Copy the string into our "frame buffer" to check its value.
  attribute_store_get_reported_string(parameter_name_node,
                                      (char *)received_frame,
                                      sizeof(received_frame));

  // Simulate a report to follow
  const uint8_t incoming_frame_3[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                      CONFIGURATION_NAME_REPORT_V4,
                                      parameter_id >> 8,
                                      parameter_id & 0xFF,
                                      0,
                                      'e',
                                      't',
                                      'e',
                                      'r',
                                      ' ',
                                      ':',
                                      ')'};

  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame_3,
                                          sizeof(incoming_frame_3)));

  // Copy the string into our "frame buffer" to check its value.
  attribute_store_get_reported_string(parameter_name_node,
                                      (char *)received_frame,
                                      sizeof(received_frame));

  TEST_ASSERT_EQUAL_STRING("Funny Parameter :)", received_frame);
}

void test_zwave_command_class_configuration_unknown_command()
{
  const uint8_t incoming_frame[] = {COMMAND_CLASS_CONFIGURATION_V4, 0xFE};

  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));
}

void test_zwave_command_class_configuration_no_command()
{
  const uint8_t incoming_frame[] = {COMMAND_CLASS_CONFIGURATION_V4};

  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));
}

void test_zwave_command_class_configuration_non_existing_parameter()
{
  // Create parameter ID 0
  configuration_parameter_id_t parameter_id = 0;
  attribute_store_node_t parameter_id_node
    = attribute_store_add_node(ATTRIBUTE(PARAMETER_ID), endpoint_id_node);
  attribute_store_set_reported(parameter_id_node,
                               &parameter_id,
                               sizeof(parameter_id));
  attribute_store_node_t parameter_value_node
    = attribute_store_add_node(ATTRIBUTE(PARAMETER_VALUE), parameter_id_node);

  // Ask the Command class handler to resolver the value of the parameter
  // We will just delete this parameter
  TEST_ASSERT_NOT_NULL(configuration_get);
  TEST_ASSERT_EQUAL(SL_STATUS_ALREADY_EXISTS,
                    configuration_get(parameter_value_node,
                                      received_frame,
                                      &received_frame_size));

  TEST_ASSERT_FALSE(attribute_store_node_exists(parameter_value_node));
  TEST_ASSERT_FALSE(attribute_store_node_exists(parameter_id_node));
}

void test_zwave_command_class_configuration_default_reset_v3_node()
{
  // Simulate a version 3 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);
  zwave_cc_version_t version = 3;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  attribute_store_node_t default_reset_node
    = attribute_store_add_node(ATTRIBUTE(DEFAULT_RESET_REQUESTED),
                               endpoint_id_node);

  TEST_ASSERT_NOT_NULL(configuration_default_reset);
  TEST_ASSERT_EQUAL(SL_STATUS_ALREADY_EXISTS,
                    configuration_default_reset(default_reset_node,
                                                received_frame,
                                                &received_frame_size));
  TEST_ASSERT_FALSE(attribute_store_node_exists(default_reset_node));
}

void test_zwave_command_class_configuration_default_reset_v4_node()
{
  // Simulate a version 4 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);
  zwave_cc_version_t version = 4;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  attribute_store_node_t default_reset_node
    = attribute_store_add_node(ATTRIBUTE(DEFAULT_RESET_REQUESTED),
                               endpoint_id_node);

  TEST_ASSERT_NOT_NULL(configuration_default_reset);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_default_reset(default_reset_node,
                                                received_frame,
                                                &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_CONFIGURATION_V4, CONFIGURATION_DEFAULT_RESET_V4};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_zwave_command_class_configuration_on_default_reset_complete_get_rule()
{
  attribute_store_node_t default_reset_node
    = attribute_store_add_node(ATTRIBUTE(DEFAULT_RESET_REQUESTED),
                               endpoint_id_node);

  command_status_values_t state = NEEDS_MULTIPLE_COMMANDS;
  attribute_store_set_reported(default_reset_node, &state, sizeof(state));
  attribute_store_set_desired(default_reset_node, &state, sizeof(state));

  TEST_ASSERT_NOT_NULL(on_configuration_set_default_complete);
  on_configuration_set_default_complete(
    default_reset_node,
    RESOLVER_GET_RULE,
    FRAME_SENT_EVENT_OK_SUPERVISION_WORKING);

  TEST_ASSERT_TRUE(attribute_store_is_value_matched(default_reset_node));
  attribute_store_get_reported(default_reset_node, &state, sizeof(state));
  TEST_ASSERT_EQUAL(FINAL_STATE, state);
}

void test_zwave_command_class_configuration_on_default_reset_complete()
{
  attribute_store_node_t default_reset_node
    = attribute_store_add_node(ATTRIBUTE(DEFAULT_RESET_REQUESTED),
                               endpoint_id_node);

  command_status_values_t state = NEEDS_MULTIPLE_COMMANDS;
  attribute_store_set_reported(default_reset_node, &state, sizeof(state));
  state = NEEDS_ONE_COMMAND;
  attribute_store_set_desired(default_reset_node, &state, sizeof(state));

  TEST_ASSERT_NOT_NULL(on_configuration_set_default_complete);
  on_configuration_set_default_complete(
    default_reset_node,
    RESOLVER_SET_RULE,
    FRAME_SENT_EVENT_OK_SUPERVISION_WORKING);

  TEST_ASSERT_FALSE(attribute_store_is_value_matched(default_reset_node));
  attribute_store_get_reported(default_reset_node, &state, sizeof(state));
  TEST_ASSERT_EQUAL(NEEDS_MULTIPLE_COMMANDS, state);

  // Try again with Supervision SUCCESS:
  on_configuration_set_default_complete(
    default_reset_node,
    RESOLVER_SET_RULE,
    FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS);

  TEST_ASSERT_TRUE(attribute_store_is_value_matched(default_reset_node));
  attribute_store_get_reported(default_reset_node, &state, sizeof(state));
  TEST_ASSERT_EQUAL(FINAL_STATE, state);
}

void test_zwave_command_class_configuration_on_default_reset_complete_no_support()
{
  attribute_store_node_t default_reset_node
    = attribute_store_add_node(ATTRIBUTE(DEFAULT_RESET_REQUESTED),
                               endpoint_id_node);

  command_status_values_t state = NEEDS_MULTIPLE_COMMANDS;
  attribute_store_set_reported(default_reset_node, &state, sizeof(state));
  state = NEEDS_ONE_COMMAND;
  attribute_store_set_desired(default_reset_node, &state, sizeof(state));

  TEST_ASSERT_NOT_NULL(on_configuration_set_default_complete);
  on_configuration_set_default_complete(
    default_reset_node,
    RESOLVER_SET_RULE,
    FRAME_SENT_EVENT_OK_SUPERVISION_NO_SUPPORT);

  TEST_ASSERT_FALSE(attribute_store_node_exists(default_reset_node));
}

void test_zwave_command_class_configuration_on_default_reset_complete_fail()
{
  attribute_store_node_t default_reset_node
    = attribute_store_add_node(ATTRIBUTE(DEFAULT_RESET_REQUESTED),
                               endpoint_id_node);

  command_status_values_t state = NEEDS_MULTIPLE_COMMANDS;
  attribute_store_set_reported(default_reset_node, &state, sizeof(state));
  state = NEEDS_ONE_COMMAND;
  attribute_store_set_desired(default_reset_node, &state, sizeof(state));

  TEST_ASSERT_NOT_NULL(on_configuration_set_default_complete);
  on_configuration_set_default_complete(default_reset_node,
                                        RESOLVER_SET_RULE,
                                        FRAME_SENT_EVENT_OK_SUPERVISION_FAIL);

  TEST_ASSERT_TRUE(attribute_store_is_value_matched(default_reset_node));
  attribute_store_get_reported(default_reset_node, &state, sizeof(state));
  TEST_ASSERT_EQUAL(FINAL_STATE, state);
}

void test_zwave_command_class_version_3_node_name_report_larger_than_capacity()
{
  // Simulate a version 3 node
  attribute_store_node_t version_node = attribute_store_add_node(
    ZWAVE_CC_VERSION_ATTRIBUTE(COMMAND_CLASS_CONFIGURATION_V4),
    endpoint_id_node);
  zwave_cc_version_t version = 3;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  configuration_parameter_id_t parameter_id       = 32;
  configuration_parameter_size_t parameter_size   = 1;
  configuration_parameter_size_t parameter_format = 1;
  configuration_parameter_value_t parameter_value = 42;
  const uint8_t incoming_frame[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                    CONFIGURATION_REPORT_V4,
                                    (uint8_t)parameter_id,
                                    parameter_size | (parameter_format << 3),
                                    (int8_t)((parameter_value >> 0) & 0xFF)};

  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame,
                                          sizeof(incoming_frame)));

  // Check that the parameter is created:
  attribute_store_node_t parameter_id_node
    = attribute_store_get_node_child_by_value(endpoint_id_node,
                                              ATTRIBUTE(PARAMETER_ID),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&parameter_id,
                                              sizeof(parameter_id),
                                              0);

  attribute_store_node_t parameter_name_node
    = attribute_store_get_node_child_by_type(parameter_id_node,
                                             ATTRIBUTE(PARAMETER_NAME),
                                             0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, parameter_name_node);

  // Now ask a name Get for a v3 node.
  TEST_ASSERT_NOT_NULL(configuration_name_get);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_name_get(parameter_name_node,
                                           received_frame,
                                           &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                    CONFIGURATION_NAME_GET_V4,
                                    parameter_id >> 8,
                                    parameter_id & 0xFF};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));

  // Simulate an answer, too long and contains weird chars.
  // clang-format off
  const uint8_t incoming_frame_2[] = {COMMAND_CLASS_CONFIGURATION_V4,
                                      CONFIGURATION_NAME_REPORT_V4,
                                      parameter_id >> 8,
                                      parameter_id & 0xFF,
                                      0,
  254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
  254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
  254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
  254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
  254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
  254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
  254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
  254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
  254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
  254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
  254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
  254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
  254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
  254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254
  };
  // clang-format on

  TEST_ASSERT_NOT_NULL(configuration_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    configuration_handler.control_handler(&connection_info,
                                          incoming_frame_2,
                                          sizeof(incoming_frame_2)));

  // Copy the string into our "frame buffer" to check its value.
  attribute_store_get_reported_string(parameter_name_node,
                                      (char *)received_frame,
                                      sizeof(received_frame));

  TEST_ASSERT_EQUAL_STRING("", received_frame);
}

void test_give_up_on_finding_next_parameter_id()
{
  // Create a test node
  attribute_store_node_t next_id_node
    = attribute_store_add_node(ATTRIBUTE(NEXT_SUPPORTED_PARAMETER_ID),
                               endpoint_id_node);

  TEST_ASSERT_NOT_NULL(on_next_supported_parameter_id_not_found);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(next_id_node, REPORTED_ATTRIBUTE));

  on_next_supported_parameter_id_not_found(next_id_node);
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(next_id_node, REPORTED_ATTRIBUTE));
}