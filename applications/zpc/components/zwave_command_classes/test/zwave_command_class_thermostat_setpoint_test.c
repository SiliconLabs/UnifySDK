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
#include "unity.h"
#include "zwave_command_class_thermostat_setpoint.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "zpc_attribute_store_type_registration.h"
#include "zwave_unid.h"

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
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_##type

// Static variables
static attribute_resolver_function_t thermostat_setpoint_set = NULL;
static attribute_resolver_function_t thermostat_setpoint_get = NULL;
static attribute_resolver_function_t thermostat_setpoint_capabilities_get
  = NULL;
static attribute_resolver_function_t thermostat_setpoint_supported_get = NULL;
static zwave_command_handler_t thermostat_handler                      = {};

static uint8_t received_frame[255]  = {};
static uint16_t received_frame_size = 0;

// Stub functions
static sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  if (node_type == ATTRIBUTE(VALUE)) {
    TEST_ASSERT_NOT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    thermostat_setpoint_set = set_func;
    thermostat_setpoint_get = get_func;
  } else if (node_type == ATTRIBUTE(MIN_VALUE)) {
    TEST_ASSERT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    thermostat_setpoint_capabilities_get = get_func;
  } else if (node_type == ATTRIBUTE(SUPPORTED_SETPOINT_TYPES)) {
    TEST_ASSERT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    thermostat_setpoint_supported_get = get_func;
  } else {
    TEST_FAIL_MESSAGE("Attribute rule registration on a wrong type");
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  thermostat_handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    thermostat_handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_THERMOSTAT_SETPOINT_V3,
                    thermostat_handler.command_class);
  TEST_ASSERT_EQUAL(THERMOSTAT_SETPOINT_VERSION_V3, thermostat_handler.version);
  TEST_ASSERT_NOT_NULL(thermostat_handler.control_handler);
  TEST_ASSERT_NULL(thermostat_handler.support_handler);
  TEST_ASSERT_FALSE(thermostat_handler.manual_security_validation);

  return SL_STATUS_OK;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  zpc_attribute_store_register_known_attribute_types();
  zwave_unid_set_home_id(home_id);
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

static void zwave_command_class_thermostat_setpoint_init_verification()
{
  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_thermostat_setpoint_init());
}

/// Called before each and every test
void setUp()
{
  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size = 0;
  memset(&thermostat_handler, 0, sizeof(zwave_command_handler_t));
  thermostat_setpoint_set              = NULL;
  thermostat_setpoint_get              = NULL;
  thermostat_setpoint_capabilities_get = NULL;
  thermostat_setpoint_supported_get    = NULL;

  zpc_attribute_store_test_helper_create_network();

  zwave_command_class_thermostat_setpoint_init_verification();
}

void test_zwave_command_class_thermostat_setpoint_set_v1()
{
  TEST_ASSERT_NOT_NULL(thermostat_setpoint_set);

  // Version 1 node
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(VERSION),
                                     &version,
                                     sizeof(version));
  const uint8_t type = 34;
  attribute_store_node_t setpoint_type_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(TYPE),
                              &type,
                              sizeof(type));

  int32_t desired_value = 123456;
  attribute_store_node_t value_node
    = attribute_store_add_node(ATTRIBUTE(VALUE), setpoint_type_node);
  attribute_store_set_desired(value_node,
                              &desired_value,
                              sizeof(desired_value));

  TEST_ASSERT_NOT_NULL(thermostat_setpoint_set);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    thermostat_setpoint_set(value_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_THERMOSTAT_SETPOINT_V3,
                                    THERMOSTAT_SETPOINT_SET_V3,
                                    type,
                                    SET_DEFAULT_PRECISION | SET_DEFAULT_SIZE,
                                    (desired_value & 0xFF000000) >> 24,  // MSB
                                    (desired_value & 0x00FF0000) >> 16,
                                    (desired_value & 0x0000FF00) >> 8,
                                    (desired_value & 0x000000FF)};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_zwave_command_class_thermostat_setpoint_set_v3_clip_lower_bound()
{
  TEST_ASSERT_NOT_NULL(thermostat_setpoint_set);

  // Version 1 node
  const zwave_cc_version_t version = 3;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(VERSION),
                                     &version,
                                     sizeof(version));
  const uint8_t type = 2;
  attribute_store_node_t setpoint_type_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(TYPE),
                              &type,
                              sizeof(type));

  int32_t desired_value = DEFAULT_MIN_VALUE - 10;
  attribute_store_node_t value_node
    = attribute_store_add_node(ATTRIBUTE(VALUE), setpoint_type_node);
  attribute_store_set_desired(value_node,
                              &desired_value,
                              sizeof(desired_value));

  const int32_t min_value = DEFAULT_MIN_VALUE;
  attribute_store_set_child_reported(setpoint_type_node,
                                     ATTRIBUTE(MIN_VALUE),
                                     &min_value,
                                     sizeof(min_value));

  TEST_ASSERT_NOT_NULL(thermostat_setpoint_set);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    thermostat_setpoint_set(value_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_THERMOSTAT_SETPOINT_V3,
                                    THERMOSTAT_SETPOINT_SET_V3,
                                    type,
                                    SET_DEFAULT_PRECISION | SET_DEFAULT_SIZE,
                                    (min_value & 0xFF000000) >> 24,  // MSB
                                    (min_value & 0x00FF0000) >> 16,
                                    (min_value & 0x0000FF00) >> 8,
                                    (min_value & 0x000000FF)};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_zwave_command_class_thermostat_setpoint_set_v3_clip_upper_bound()
{
  TEST_ASSERT_NOT_NULL(thermostat_setpoint_set);

  // Version 1 node
  const zwave_cc_version_t version = 3;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(VERSION),
                                     &version,
                                     sizeof(version));
  const uint8_t type = 2;
  attribute_store_node_t setpoint_type_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(TYPE),
                              &type,
                              sizeof(type));

  int32_t desired_value = DEFAULT_MAX_VALUE + 10;
  attribute_store_node_t value_node
    = attribute_store_add_node(ATTRIBUTE(VALUE), setpoint_type_node);
  attribute_store_set_desired(value_node,
                              &desired_value,
                              sizeof(desired_value));

  const int32_t max_value = DEFAULT_MAX_VALUE;
  attribute_store_set_child_reported(setpoint_type_node,
                                     ATTRIBUTE(MAX_VALUE),
                                     &max_value,
                                     sizeof(max_value));

  TEST_ASSERT_NOT_NULL(thermostat_setpoint_set);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    thermostat_setpoint_set(value_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_THERMOSTAT_SETPOINT_V3,
                                    THERMOSTAT_SETPOINT_SET_V3,
                                    type,
                                    SET_DEFAULT_PRECISION | SET_DEFAULT_SIZE,
                                    (max_value & 0xFF000000) >> 24,  // MSB
                                    (max_value & 0x00FF0000) >> 16,
                                    (max_value & 0x0000FF00) >> 8,
                                    (max_value & 0x000000FF)};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}