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
#include "zwave_command_class_thermostat_setpoint_types.h"

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

///////////////////////////////////////
// Helpers
///////////////////////////////////////

/**
 * @brief Create structure of setpoint value (precision, scale and value) under the setpoint_type_node
 * 
 * @param setpoint_type_node Setpoint Type node 
 * @param value_type         Value attribute type (ATTRIBUTE(VALUE), ATTRIBUTE(MIN_VALUE), ATTRIBUTE(MAX_VALUE)) 
 * @param value              Setpoint Value 
 * @param scale              Setpoint Scale (0: °C, 1: F)
 * @param precision          Setpoint precision
 * @param value_state        State of setpoint value (reported or desired)
 * 
 * @return attribute_store_node_t Setpoint Value node
 */
attribute_store_node_t helper_create_setpoint_attribute(
  attribute_store_node_t setpoint_type_node,
  attribute_store_type_t value_type,
  thermostat_setpoint_value_t value,
  thermostat_setpoint_scale_t scale,
  thermostat_setpoint_precision_t precision,
  attribute_store_node_value_state_t value_state)
{
  // Define scale & precision first
  attribute_store_set_child_reported(setpoint_type_node,
                                     value_type
                                       + SETPOINT_SCALE_ATTRIBUTE_ID_OFFSET,
                                     &scale,
                                     sizeof(scale));

  attribute_store_set_child_reported(setpoint_type_node,
                                     value_type
                                       + SETPOINT_PRECISION_ATTRIBUTE_ID_OFFSET,
                                     &precision,
                                     sizeof(precision));

  // Do not add this if already present
  attribute_store_node_t value_node
    = attribute_store_get_first_child_by_type(setpoint_type_node, value_type);
  if (value_node == ATTRIBUTE_STORE_INVALID_NODE) {
    value_node = attribute_store_add_node(value_type, setpoint_type_node);
  }

  attribute_store_set_node_attribute_value(value_node,
                                           value_state,
                                           (uint8_t *)&value,
                                           sizeof(value));

  return value_node;
}

/**
 * @brief Create specified setpoint type and set to default capabilities
 * @note Force version to 1
 * @param type Setpoint type
 * @return setpoint type node 
 */
attribute_store_node_t set_default_capabilities(thermostat_setpoint_type_t type)
{
  TEST_ASSERT_NOT_NULL(thermostat_setpoint_capabilities_get);

  attribute_store_node_t setpoint_type_node
    = attribute_store_get_node_child_by_value(endpoint_id_node,
                                              ATTRIBUTE(TYPE),
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&type,
                                              sizeof(type),
                                              0);
  // Node already exists we don't need to do this
  if (setpoint_type_node != ATTRIBUTE_STORE_INVALID_NODE) {
    return setpoint_type_node;
  }

  // Simulate version 1 to have default capabilities
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(VERSION),
                                     &version,
                                     sizeof(version));

  // Create setpoint
  
  setpoint_type_node  = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(TYPE),
                              &type,
                              sizeof(type));

  // Get function listen to min_value so we have to create it
  attribute_store_node_t min_value_node
    = attribute_store_emplace(setpoint_type_node,
                              ATTRIBUTE(MIN_VALUE),
                              &type,
                              sizeof(type));

  sl_status_t status
    = thermostat_setpoint_capabilities_get(min_value_node,
                                           received_frame,
                                           &received_frame_size);

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_ALREADY_EXISTS,
    status,
    "Get capabilites should have returned SL_STATUS_ALREADY_EXISTS");

  return setpoint_type_node;
}

attribute_store_type_t get_ucl_type(thermostat_setpoint_type_t setpoint_type)
{
  attribute_store_type_t ucl_type;
  switch (setpoint_type) {
    case SETPOINT_TYPE_HEATING:
      ucl_type = DOTDOT_ATTRIBUTE_ID_THERMOSTAT_OCCUPIED_HEATING_SETPOINT;
      break;
    case SETPOINT_TYPE_COOLING:
      ucl_type = DOTDOT_ATTRIBUTE_ID_THERMOSTAT_OCCUPIED_COOLING_SETPOINT;
      break;
    default:
      TEST_FAIL_MESSAGE("Unkown setpoint type in get_ucl_type");
  }

  return ucl_type;
}

void helper_test_ucl_bindings(thermostat_setpoint_type_t setpoint_type,
                              thermostat_setpoint_value_t desired_value,
                              thermostat_setpoint_precision_t precision,
                              thermostat_setpoint_scale_t scale,
                              int16_t expected_ucl_value)
{
  attribute_store_type_t ucl_type = get_ucl_type(setpoint_type);
  attribute_store_node_t setpoint_type_node
    = set_default_capabilities(setpoint_type);

  helper_create_setpoint_attribute(setpoint_type_node,
                                   ATTRIBUTE(VALUE),
                                   desired_value,
                                   scale,
                                   precision,
                                   REPORTED_ATTRIBUTE);

  int16_t ucl_value = 0;
  attribute_store_get_child_reported(
    endpoint_id_node,
    ucl_type,
    &ucl_value,
    sizeof(ucl_value));

  // Rounded down to precision of 2
  TEST_ASSERT_EQUAL_MESSAGE(expected_ucl_value,
                            ucl_value,
                            "UCL value should have it's value defined");
}


void helper_test_zwave_bindings(thermostat_setpoint_type_t setpoint_type,
                              int16_t ucl_value,
                              thermostat_setpoint_precision_t precision,
                              thermostat_setpoint_scale_t scale,
                              thermostat_setpoint_value_t expected_zwave_value)
{
  attribute_store_type_t ucl_type = get_ucl_type(setpoint_type);

  attribute_store_node_t setpoint_type_node
    = set_default_capabilities(setpoint_type);

  attribute_store_node_t value_node = helper_create_setpoint_attribute(setpoint_type_node,
                                   ATTRIBUTE(VALUE),
                                   0,
                                   scale,
                                   precision,
                                   REPORTED_ATTRIBUTE);

  attribute_store_node_t ucl_node
    = attribute_store_get_node_child_by_type(endpoint_id_node, ucl_type, 0);
  // Simulate ZCL value set
  attribute_store_set_desired(ucl_node, &ucl_value, sizeof(ucl_value));

  // Check if desired value of Zwave is correct
  thermostat_setpoint_value_t reported_zwave_value;
  attribute_store_get_desired(value_node,
                              &reported_zwave_value,
                              sizeof(reported_zwave_value));
  TEST_ASSERT_EQUAL_MESSAGE(expected_zwave_value,
                            reported_zwave_value,
                            "ZWave desired value is not set correctly");
  
  // Simulate get
  attribute_store_set_reported_as_desired(value_node);
  attribute_store_undefine_desired(value_node);

  // Check if ucl value is correctly undefined
  TEST_ASSERT_FALSE_MESSAGE(
    attribute_store_is_desired_defined(ucl_node),
    "UCL node should have its value undefined. This allows the calling command "
    "to be in a correct state.");
}

///////////////////////////////////////
// Binding values
///////////////////////////////////////
void test_zwave_command_class_thermostat_setpoint_update_ucl_heat_celsius_happy_case()
{
  // No conversion only scale down to precision 2
  helper_test_ucl_bindings(SETPOINT_TYPE_HEATING,
                           10123,
                           3,
                           CELSIUS_SCALE,
                           1012);
}
void test_zwave_command_class_thermostat_setpoint_update_ucl_heat_fahrenheit_happy_case()
{
  // Scale to precision 2 and convert to C
  helper_test_ucl_bindings(SETPOINT_TYPE_HEATING,
                           720,
                           1,
                           FAHRENHEIT_SCALE,
                           2222);
}
void test_zwave_command_class_thermostat_setpoint_update_ucl_cool_celsius_happy_case()
{
    // No conversion only scale to precision 2
  helper_test_ucl_bindings(SETPOINT_TYPE_COOLING, 12, 0, CELSIUS_SCALE, 1200);
}
void test_zwave_command_class_thermostat_setpoint_update_ucl_cool_fahrenheit_happy_case()
{
  // Scale down to precision 3 and convert to C
  helper_test_ucl_bindings(SETPOINT_TYPE_COOLING,
                           73255,
                           3,
                           FAHRENHEIT_SCALE,
                           2291);
}

void test_zwave_command_class_thermostat_setpoint_update_zwave_heat_celsius_happy_case()
{
  // No changes
  helper_test_zwave_bindings(SETPOINT_TYPE_HEATING,
                           1234, // Ucl value
                           2,
                           CELSIUS_SCALE,
                           1234 // Zwave value
                           );
}
void test_zwave_command_class_thermostat_setpoint_update_zwave_heat_fahrenheit_happy_case()
{
  // Scale to precision 0 + convert into F
  helper_test_zwave_bindings(SETPOINT_TYPE_HEATING,
                           1200, // Ucl value
                           0,
                           FAHRENHEIT_SCALE,
                           53 // Zwave value
                           );
}

void test_zwave_command_class_thermostat_setpoint_update_zwave_cool_celsius_happy_case()
{
  // Scale down to precision 1
  helper_test_zwave_bindings(SETPOINT_TYPE_COOLING,
                           -1000, // Ucl value
                           1,
                           CELSIUS_SCALE,
                           -100 // Zwave value
                           );
}
void test_zwave_command_class_thermostat_setpoint_update_zwave_cool_fahrenheit_happy_case()
{
  // Scale to precision 3 + convert into F
  helper_test_zwave_bindings(SETPOINT_TYPE_COOLING,
                           -500, // Ucl value
                           3,
                           FAHRENHEIT_SCALE,
                           23000 // Zwave value
                           );
}

void test_zwave_command_class_thermostat_setpoint_update_zwave_same_fahrenheit_value()
{
  // First try with a precision of 2
  helper_test_zwave_bindings(SETPOINT_TYPE_COOLING,
                             -5800,  // Ucl value
                             2,
                             FAHRENHEIT_SCALE,
                             -7240  // Zwave value
  );
  // No problem here
  helper_test_zwave_bindings(SETPOINT_TYPE_COOLING,
                             -5810,  // Ucl value
                             2,
                             FAHRENHEIT_SCALE,
                             -7258  // Zwave value
  );

  // Now test with a precision of 0
  helper_test_zwave_bindings(SETPOINT_TYPE_COOLING,
                             -5812,  // Ucl value
                             0,
                             FAHRENHEIT_SCALE,
                             -72  // Zwave value
  );
  // Should be the same value since Zwave is not precise enough 
  helper_test_zwave_bindings(SETPOINT_TYPE_COOLING,
                             -5815,  // Ucl value
                             0,
                             FAHRENHEIT_SCALE,
                             -72  // Zwave value
  );
}
///////////////////////////////////////
// SETPOINT SET/GET/REPORT
///////////////////////////////////////
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

  thermostat_setpoint_value_t desired_value = 123456;
  thermostat_setpoint_precision_t precision = 7;
  thermostat_setpoint_scale_t scale         = 1;

  attribute_store_node_t value_node
    = helper_create_setpoint_attribute(setpoint_type_node,
                                       ATTRIBUTE(VALUE),
                                       desired_value,
                                       scale,
                                       precision,
                                       DESIRED_ATTRIBUTE);
  // Check that those parameters are ignored for versions < 3
  helper_create_setpoint_attribute(setpoint_type_node,
                                   ATTRIBUTE(MIN_VALUE),
                                   0,
                                   scale,
                                   0,
                                   REPORTED_ATTRIBUTE);
  helper_create_setpoint_attribute(setpoint_type_node,
                                   ATTRIBUTE(MAX_VALUE),
                                   0,
                                   scale,
                                   0,
                                   REPORTED_ATTRIBUTE);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    thermostat_setpoint_set(value_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_THERMOSTAT_SETPOINT_V3,
                                    THERMOSTAT_SETPOINT_SET_V3,
                                    type,
                                    (precision << 5) | (scale << 3) | 4,
                                    (desired_value & 0xFF000000) >> 24,  // MSB
                                    (desired_value & 0x00FF0000) >> 16,
                                    (desired_value & 0x0000FF00) >> 8,
                                    (desired_value & 0x000000FF)};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}
void test_zwave_command_class_thermostat_setpoint_set_v2()
{
  TEST_ASSERT_NOT_NULL(thermostat_setpoint_set);

  // Version 2 node
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(VERSION),
                                     &version,
                                     sizeof(version));
  const uint8_t type = 1;
  attribute_store_node_t setpoint_type_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(TYPE),
                              &type,
                              sizeof(type));

  thermostat_setpoint_value_t desired_value = 12;
  thermostat_setpoint_precision_t precision = 5;
  thermostat_setpoint_scale_t scale         = 1;

  attribute_store_node_t value_node
    = helper_create_setpoint_attribute(setpoint_type_node,
                                       ATTRIBUTE(VALUE),
                                       desired_value,
                                       scale,
                                       precision,
                                       DESIRED_ATTRIBUTE);
  // Check that those parameters are ignored for versions < 3
  helper_create_setpoint_attribute(setpoint_type_node,
                                   ATTRIBUTE(MIN_VALUE),
                                   0,
                                   scale,
                                   0,
                                   REPORTED_ATTRIBUTE);
  helper_create_setpoint_attribute(setpoint_type_node,
                                   ATTRIBUTE(MAX_VALUE),
                                   0,
                                   scale,
                                   0,
                                   REPORTED_ATTRIBUTE);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    thermostat_setpoint_set(value_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_THERMOSTAT_SETPOINT_V3,
                                    THERMOSTAT_SETPOINT_SET_V3,
                                    type,
                                    (precision << 5) | (scale << 3) | 1,
                                    (desired_value & 0x000000FF)};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

attribute_store_node_t helper_create_setpoint_attribute(attribute_store_node_t setpoint_type_node,
                                      attribute_store_type_t value_type,
                                      thermostat_setpoint_value_t value,
                                      thermostat_setpoint_scale_t scale,
                                      thermostat_setpoint_precision_t precision,
                                      attribute_store_node_value_state_t value_state)
{
  attribute_store_node_t value_node
    = attribute_store_add_node(value_type, setpoint_type_node);
  attribute_store_set_node_attribute_value(value_node,
                                           value_state,
                                           (uint8_t*)&value,
                                           sizeof(value));

  attribute_store_set_child_reported(setpoint_type_node,
                                     value_type + 1,
                                     &scale,
                                     sizeof(scale));

  attribute_store_set_child_reported(setpoint_type_node,
                                     value_type + 2,
                                     &precision,
                                     sizeof(precision));

  return value_node;
}

void test_zwave_command_class_thermostat_setpoint_set_v3_clip_lower_bound()
{
  TEST_ASSERT_NOT_NULL(thermostat_setpoint_set);

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

  thermostat_setpoint_value_t desired_value = DEFAULT_MIN_VALUE - 10;
  thermostat_setpoint_precision_t precision = 2;
  thermostat_setpoint_scale_t scale         = 0;

  attribute_store_node_t value_node
    = helper_create_setpoint_attribute(setpoint_type_node,
                                       ATTRIBUTE(VALUE),
                                       desired_value,
                                       scale,
                                       precision,
                                       DESIRED_ATTRIBUTE);

  // Precision offset *10
  // Here we test if the precision is adjusted correctly
  thermostat_setpoint_value_t min_value = DEFAULT_MIN_VALUE * 10;
  precision                             = 3;
  helper_create_setpoint_attribute(setpoint_type_node,
                                   ATTRIBUTE(MIN_VALUE),
                                   min_value,
                                   scale,
                                   precision,
                                   REPORTED_ATTRIBUTE);
  helper_create_setpoint_attribute(setpoint_type_node,
                                   ATTRIBUTE(MAX_VALUE),
                                   DEFAULT_MAX_VALUE,
                                   scale,
                                   precision,
                                   REPORTED_ATTRIBUTE);

  // Precision of 2
  precision                                  = 2;
  thermostat_setpoint_value_t expected_value = DEFAULT_MIN_VALUE;

  TEST_ASSERT_NOT_NULL(thermostat_setpoint_set);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    thermostat_setpoint_set(value_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_THERMOSTAT_SETPOINT_V3,
                                    THERMOSTAT_SETPOINT_SET_V3,
                                    type,
                                    (precision << 5) | (scale << 3) | 2,
                                    (expected_value & 0x0000FF00) >> 8,
                                    (expected_value & 0x000000FF)};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_zwave_command_class_thermostat_setpoint_set_v3_clip_upper_bound()
{
  TEST_ASSERT_NOT_NULL(thermostat_setpoint_set);

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

  thermostat_setpoint_value_t desired_value = DEFAULT_MAX_VALUE + 10;
  thermostat_setpoint_precision_t precision = 2;
  thermostat_setpoint_scale_t scale         = 0;

  attribute_store_node_t value_node
    = helper_create_setpoint_attribute(setpoint_type_node,
                                       ATTRIBUTE(VALUE),
                                       desired_value,
                                       scale,
                                       precision,
                                       DESIRED_ATTRIBUTE);

  thermostat_setpoint_value_t max_value = DEFAULT_MAX_VALUE;
  helper_create_setpoint_attribute(setpoint_type_node,
                                   ATTRIBUTE(MIN_VALUE),
                                   DEFAULT_MIN_VALUE,
                                   scale,
                                   precision,
                                   REPORTED_ATTRIBUTE);
  helper_create_setpoint_attribute(setpoint_type_node,
                                   ATTRIBUTE(MAX_VALUE),
                                   max_value,
                                   scale,
                                   precision,
                                   REPORTED_ATTRIBUTE);

  thermostat_setpoint_value_t expected_value = DEFAULT_MAX_VALUE;

  TEST_ASSERT_NOT_NULL(thermostat_setpoint_set);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    thermostat_setpoint_set(value_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_THERMOSTAT_SETPOINT_V3,
                                    THERMOSTAT_SETPOINT_SET_V3,
                                    type,
                                    (precision << 5) | (scale << 3) | 4,
                                    (expected_value & 0xFF000000) >> 24,  // MSB
                                    (expected_value & 0x00FF0000) >> 16,
                                    (expected_value & 0x0000FF00) >> 8,
                                    (expected_value & 0x000000FF)};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_zwave_command_class_thermostat_setpoint_report_happy_case()
{
  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  uint8_t setpoint_type = 1;
  uint8_t precision     = 0b010;
  uint8_t scale         = 0b00;
  uint8_t size          = 2;
  int32_t value         = 1212;

  uint8_t report_frame[] = {COMMAND_CLASS_THERMOSTAT_SETPOINT,
                            THERMOSTAT_SETPOINT_REPORT,
                            setpoint_type,
                            (precision << 5) | (scale << 3) | size,
                            (value & 0x0000FF00) >> 8,
                            (value & 0x000000FF)};

  // Report should fail since the setpoint_type doesn't exists
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    thermostat_handler.control_handler(&info,
                                                       report_frame,
                                                       sizeof(report_frame)));

  // Create setpoint_type
  attribute_store_node_t setpoint_type_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(TYPE),
                              &setpoint_type,
                              sizeof(setpoint_type));

  // Need to create an empty value node first since it isn't created automaticaly
  attribute_store_add_node(ATTRIBUTE(VALUE), setpoint_type_node);

  // Now we can do it
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    thermostat_handler.control_handler(&info,
                                                       report_frame,
                                                       sizeof(report_frame)));

  int32_t reported_scale = 0;
  attribute_store_get_child_reported(setpoint_type_node,
                                     ATTRIBUTE(VALUE_SCALE),
                                     &reported_scale,
                                     sizeof(reported_scale));
  TEST_ASSERT_EQUAL_MESSAGE(scale, reported_scale, "Scale value should match");

  uint8_t reported_precision = 0;
  attribute_store_get_child_reported(setpoint_type_node,
                                     ATTRIBUTE(VALUE_PRECISION),
                                     &reported_precision,
                                     sizeof(reported_precision));
  TEST_ASSERT_EQUAL_MESSAGE(precision,
                            reported_precision,
                            "Precision value should match");

  int32_t reported_value;
  attribute_store_get_child_reported(setpoint_type_node,
                                     ATTRIBUTE(VALUE),
                                     &reported_value,
                                     sizeof(reported_value));
  TEST_ASSERT_EQUAL_MESSAGE(value,
                            reported_value,
                            "Current value should match");
}