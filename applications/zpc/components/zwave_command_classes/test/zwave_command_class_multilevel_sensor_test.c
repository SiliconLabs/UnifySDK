/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "zwave_command_class_multilevel_sensor.h"
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

// ZPC includes
#include "zpc_attribute_store_type_registration.h"
#include "zwave_utils.h"
#include "zwave_controller_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "attribute_resolver_mock.h"
#include "zpc_attribute_resolver_mock.h"
#include "zwave_command_handler_mock.h"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_SENSOR_MULTILEVEL_##type

// Static variables
static attribute_resolver_function_t sensor_supported_get = NULL;
static attribute_resolver_function_t supported_scale_get  = NULL;
static attribute_resolver_function_t sensor_get           = NULL;
static zwave_command_handler_t multilevel_sensor_handler  = {};

static uint8_t received_frame[255]  = {};
static uint16_t received_frame_size = 0;

// Stub functions
static sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  TEST_ASSERT_NULL(set_func);
  if (node_type == ATTRIBUTE(SUPPORTED_SENSOR_TYPES)) {
    sensor_supported_get = get_func;
  } else if (node_type == ATTRIBUTE(SUPPORTED_SCALES)) {
    supported_scale_get = get_func;
  } else if (node_type == ATTRIBUTE(SENSOR_VALUE)) {
    sensor_get = get_func;
  } else {
    TEST_FAIL_MESSAGE("Attribute rule registration on a wrong type");
  }
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  multilevel_sensor_handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    multilevel_sensor_handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SENSOR_MULTILEVEL_V11,
                    multilevel_sensor_handler.command_class);
  TEST_ASSERT_EQUAL(SENSOR_MULTILEVEL_VERSION_V11,
                    multilevel_sensor_handler.version);
  TEST_ASSERT_NOT_NULL(multilevel_sensor_handler.control_handler);
  TEST_ASSERT_NULL(multilevel_sensor_handler.support_handler);
  TEST_ASSERT_FALSE(multilevel_sensor_handler.manual_security_validation);

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

static void init_verification()
{
  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_multilevel_sensor_init());
}

/// Called before each and every test
void setUp()
{
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_set_home_id(home_id);

  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size = 0;

  sensor_supported_get = NULL;
  supported_scale_get  = NULL;
  sensor_get           = NULL;
  memset(&multilevel_sensor_handler, 0, sizeof(zwave_command_handler_t));

  init_verification();
}

void test_on_version_update()
{
  attribute_store_node_t multilevel_sensor_version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);

  const zwave_cc_version_t version = 3;
  attribute_store_set_reported(multilevel_sensor_version_node,
                               &version,
                               sizeof(version));

  TEST_ASSERT_NOT_EQUAL(
    ATTRIBUTE_STORE_INVALID_NODE,
    attribute_store_get_first_child_by_type(endpoint_id_node,
                                            ATTRIBUTE(SUPPORTED_SENSOR_TYPES)));
}

void test_zwave_command_class_multilevel_sensor_get_v5_happy_case()
{
  const zwave_cc_version_t version = 5;
  attribute_store_emplace(endpoint_id_node,
                          ATTRIBUTE(VERSION),
                          &version,
                          sizeof(version));
  const uint8_t sensor_type = 3;
  attribute_store_node_t sensor_type_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(SENSOR_TYPE),
                              &sensor_type,
                              sizeof(sensor_type));

  const uint8_t supported_scales = 6;
  attribute_store_emplace(sensor_type_node,
                          ATTRIBUTE(SUPPORTED_SCALES),
                          &supported_scales,
                          sizeof(supported_scales));

  attribute_store_node_t value_node
    = attribute_store_add_node(ATTRIBUTE(SENSOR_VALUE), sensor_type_node);

  // Now ask to generate a command
  TEST_ASSERT_NOT_NULL(sensor_get);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    sensor_get(value_node, received_frame, &received_frame_size));
  const uint8_t expected_frame[] = {COMMAND_CLASS_SENSOR_MULTILEVEL_V11,
                                    SENSOR_MULTILEVEL_GET_V11,
                                    sensor_type,
                                    1 << 3};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_multilevel_sensor_get_v4_happy_case()
{
  const zwave_cc_version_t version = 4;
  attribute_store_emplace(endpoint_id_node,
                          ATTRIBUTE(VERSION),
                          &version,
                          sizeof(version));
  const uint8_t sensor_type = 3;
  attribute_store_node_t sensor_type_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(SENSOR_TYPE),
                              &sensor_type,
                              sizeof(sensor_type));

  const uint8_t supported_scales = 6;
  attribute_store_emplace(sensor_type_node,
                          ATTRIBUTE(SUPPORTED_SCALES),
                          &supported_scales,
                          sizeof(supported_scales));

  attribute_store_node_t value_node
    = attribute_store_add_node(ATTRIBUTE(SENSOR_VALUE), sensor_type_node);

  // Now ask to generate a command
  TEST_ASSERT_NOT_NULL(sensor_get);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    sensor_get(value_node, received_frame, &received_frame_size));
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_SENSOR_MULTILEVEL_V11, SENSOR_MULTILEVEL_GET_V11, 0, 0};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_multilevel_sensor_get_v5_no_sensor_type_value()
{
  const zwave_cc_version_t version = 5;
  attribute_store_emplace(endpoint_id_node,
                          ATTRIBUTE(VERSION),
                          &version,
                          sizeof(version));
  attribute_store_node_t sensor_type_node
    = attribute_store_add_node(ATTRIBUTE(SENSOR_TYPE), endpoint_id_node);

  const uint8_t supported_scales = 6;
  attribute_store_emplace(sensor_type_node,
                          ATTRIBUTE(SUPPORTED_SCALES),
                          &supported_scales,
                          sizeof(supported_scales));

  attribute_store_node_t value_node
    = attribute_store_add_node(ATTRIBUTE(SENSOR_VALUE), sensor_type_node);

  // Now ask to generate a command
  TEST_ASSERT_NOT_NULL(sensor_get);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    sensor_get(value_node, received_frame, &received_frame_size));
  TEST_ASSERT_EQUAL(0, received_frame_size);
}

void test_zwave_command_class_multilevel_sensor_get_v5_no_supported_scales()
{
  const zwave_cc_version_t version = 5;
  attribute_store_emplace(endpoint_id_node,
                          ATTRIBUTE(VERSION),
                          &version,
                          sizeof(version));
  const uint8_t sensor_type = 3;
  attribute_store_node_t sensor_type_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(SENSOR_TYPE),
                              &sensor_type,
                              sizeof(sensor_type));

  attribute_store_add_node(ATTRIBUTE(SUPPORTED_SCALES), sensor_type_node);

  attribute_store_node_t value_node
    = attribute_store_add_node(ATTRIBUTE(SENSOR_VALUE), sensor_type_node);

  // Now ask to generate a command
  TEST_ASSERT_NOT_NULL(sensor_get);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    sensor_get(value_node, received_frame, &received_frame_size));
  TEST_ASSERT_EQUAL(0, received_frame_size);
}

void test_zwave_command_class_multilevel_sensor_handle_supported_scale_report()
{
  zwave_controller_connection_info_t connection_info = {};
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  const zwave_cc_version_t version = 5;
  attribute_store_emplace(endpoint_id_node,
                          ATTRIBUTE(VERSION),
                          &version,
                          sizeof(version));
  const uint8_t sensor_type_3 = 3;
  attribute_store_emplace(endpoint_id_node,
                          ATTRIBUTE(SENSOR_TYPE),
                          &sensor_type_3,
                          sizeof(sensor_type_3));
  const uint8_t sensor_type = 1;
  attribute_store_node_t sensor_type_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(SENSOR_TYPE),
                              &sensor_type,
                              sizeof(sensor_type));

  attribute_store_node_t supported_scales_node
    = attribute_store_add_node(ATTRIBUTE(SUPPORTED_SCALES), sensor_type_node);

  const uint8_t supported_scales = 23;
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_SENSOR_MULTILEVEL_V11,
       SENSOR_MULTILEVEL_SUPPORTED_SCALE_REPORT_V11,
       sensor_type,
       supported_scales};

  // Process incoming command:
  TEST_ASSERT_NOT_NULL(multilevel_sensor_handler.control_handler);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    multilevel_sensor_handler.control_handler(&connection_info,
                                              incoming_frame,
                                              sizeof(incoming_frame)));

  uint8_t saved_supported_scales = 0;
  attribute_store_get_reported(supported_scales_node,
                               &saved_supported_scales,
                               sizeof(saved_supported_scales));
  TEST_ASSERT_EQUAL(supported_scales, saved_supported_scales);
}
