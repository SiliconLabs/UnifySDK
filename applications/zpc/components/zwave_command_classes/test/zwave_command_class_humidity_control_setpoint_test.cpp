/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include <stdlib.h>
#include "workaround_for_test.hpp"

extern "C" {
#include "zwave_command_class_humidity_control_setpoint.h"
#include "zwave_command_class_humidity_control_types.h"
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

#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_##type

constexpr uint8_t PRECISION_SHIFT = 5;
constexpr uint8_t SCALE_SHIFT     = 3;

static zwave_command_handler_t handler = {};

static attribute_resolver_function_t supported_types_get = NULL;
static attribute_resolver_function_t capabilities_get    = NULL;
static attribute_resolver_function_t setpoint_get        = NULL;
static attribute_resolver_function_t setpoint_set        = NULL;
// Buffer for frame
static uint8_t received_frame[255]  = {};
static uint16_t received_frame_size = 0;
// Custom types
enum humidity_control_setpoint_value_type {
  SETPOINT_CURRENT_VALUE,
  SETPOINT_MIN_VALUE,
  SETPOINT_MAX_VALUE
};
enum set_bytes { SET_1BYTES = 1, SET_2BYTES = 2, SET_4BYTES = 4 };

/////////////////////////////////////////////////////
// HELPERS
/////////////////////////////////////////////////////
attribute_store_node_t create_type_node(humidity_control_setpoint_type_t type)
{
  attribute_store_node_t type_node = attribute_store_emplace(endpoint_id_node,
                                                             ATTRIBUTE(TYPE),
                                                             &type,
                                                             sizeof(type));

  TEST_ASSERT_NOT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                                type_node,
                                "Should be able to crate ATTRIBUTE(TYPE)");
  return type_node;
}

void helper_test_type_values(
  enum humidity_control_setpoint_value_type value_type,
  attribute_store_node_t value_node,
  humidity_control_setpoint_precision_t precision,
  humidity_control_setpoint_scale_t scale,
  humidity_control_setpoint_size_t size,
  uint8_t *value_buffer)
{
  // Determine used types
  attribute_store_type_t value_scale_type;
  attribute_store_type_t value_precision_type;
  switch (value_type) {
    case SETPOINT_CURRENT_VALUE:
      printf("Testing current value type\n");
      value_scale_type     = ATTRIBUTE(VALUE_SCALE);
      value_precision_type = ATTRIBUTE(VALUE_PRECISION);
      break;
    case SETPOINT_MIN_VALUE:
      printf("Testing min value type\n");
      value_scale_type     = ATTRIBUTE(MIN_VALUE_SCALE);
      value_precision_type = ATTRIBUTE(MIN_VALUE_PRECISION);
      break;
    case SETPOINT_MAX_VALUE:
      printf("Testing max value type\n");
      value_scale_type     = ATTRIBUTE(MAX_VALUE_SCALE);
      value_precision_type = ATTRIBUTE(MAX_VALUE_PRECISION);
      break;
    default:
      TEST_ABORT();
  }

  // Check Value
  humidity_control_setpoint_value_t reported_value, expected_value;
  attribute_store_get_reported(value_node,
                               &reported_value,
                               sizeof(reported_value));

  expected_value = get_signed_value_from_frame_and_size(&value_buffer[0], size);

  TEST_ASSERT_EQUAL_MESSAGE(expected_value,
                            reported_value,
                            "VALUE is incorrect");

  humidity_control_setpoint_scale_t reported_scale;
  sl_status_t status
    = attribute_store_get_child_reported(value_node,
                                         value_scale_type,
                                         &reported_scale,
                                         sizeof(reported_scale));
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    status,
    "Should have access to reported value of SCALE attribute");

  TEST_ASSERT_EQUAL_MESSAGE(scale, reported_scale, "SCALE value is incorrect");

  humidity_control_setpoint_precision_t reported_precision;
  status = attribute_store_get_child_reported(value_node,
                                              value_precision_type,
                                              &reported_precision,
                                              sizeof(reported_precision));
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    status,
    "Should have access to reported value of PRECISION attribute");

  TEST_ASSERT_EQUAL_MESSAGE(precision,
                            reported_precision,
                            "PRECISION value is incorrect");
}

// Return value node
attribute_store_node_t
  create_setpoint_structure(humidity_control_setpoint_type_t type,
                            humidity_control_setpoint_value_t current_value,
                            humidity_control_setpoint_precision_t precision,
                            humidity_control_setpoint_scale_t scale,
                            humidity_control_setpoint_value_t min_value,
                            humidity_control_setpoint_value_t max_value)
{
  attribute_store_node_t type_node = create_type_node(type);

  auto min_value_node = attribute_store_emplace(type_node,
                          ATTRIBUTE(MIN_VALUE),
                          &min_value,
                          sizeof(min_value));
  attribute_store_emplace(min_value_node,
                          ATTRIBUTE(MIN_VALUE_PRECISION),
                          &precision,
                          sizeof(precision));
  auto max_value_node = attribute_store_emplace(type_node,
                          ATTRIBUTE(MAX_VALUE),
                          &max_value,
                          sizeof(max_value));
  attribute_store_emplace(max_value_node,
                          ATTRIBUTE(MAX_VALUE_PRECISION),
                          &precision,
                          sizeof(precision));

  attribute_store_node_t value_node
    = attribute_store_emplace_desired(type_node,
                                      ATTRIBUTE(VALUE),
                                      &current_value,
                                      sizeof(current_value));
  attribute_store_emplace(value_node,
                          ATTRIBUTE(VALUE_PRECISION),
                          &precision,
                          sizeof(precision));
  attribute_store_emplace(value_node,
                          ATTRIBUTE(VALUE_SCALE),
                          &scale,
                          sizeof(scale));

  return value_node;
}

uint8_t
  helper_get_attribute_field(humidity_control_setpoint_precision_t precision,
                             humidity_control_setpoint_scale_t scale,
                             humidity_control_setpoint_size_t size)
{
  return (precision << PRECISION_SHIFT) | (scale << SCALE_SHIFT) | size;
}

std::vector<uint8_t> explode_value(uint8_t size,
                                   humidity_control_setpoint_value_t value)
{
  std::vector<uint8_t> exploded_value;
  for (uint8_t i = size; i > 0; i--) {
    uint8_t offset   = i - 1;
    uint8_t shift    = 8 * offset;
    uint32_t bitmask = 0xFF << shift;

    uint8_t value_8bit = (value & bitmask) >> shift;
    exploded_value.push_back(value_8bit);
  }

  return exploded_value;
}

void helper_set_setpoint(enum set_bytes bytes_count, bool happy_case)
{
  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL(setpoint_set);
  // Not supported with invalid node
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    setpoint_set(0, received_frame, &received_frame_size));

  humidity_control_setpoint_type_t type
    = HUMIDITY_CONTROL_SETPOINT_GET_SETPOINT_TYPE_HUMIDIFIER;
  humidity_control_setpoint_value_t current_value = 50;
  humidity_control_setpoint_precision_t precision = 3;
  humidity_control_setpoint_scale_t scale         = 1;
  humidity_control_setpoint_value_t min_value     = -100;
  humidity_control_setpoint_value_t max_value     = 100;
  std::vector<uint8_t> expected_value;

  switch (bytes_count) {
    case SET_1BYTES:
      current_value = -50;
      precision     = 3;
      scale         = 1;
      min_value     = happy_case ? -100 : -10;
      max_value     = 10;
      break;
    case SET_2BYTES:
      current_value = INT16_MAX - 2;
      precision     = 4;
      scale         = 1;
      min_value     = -100;
      max_value     = happy_case ? INT16_MAX : 10;
      break;
    case SET_4BYTES:
      current_value = INT32_MAX - 2;
      precision     = 1;
      scale         = 0;
      min_value     = -100;
      max_value     = happy_case ? INT32_MAX : 10;
      break;
    default:
      TEST_ABORT();
  }

  // Transform 32 bit value into chunk of 8 bit values
  expected_value = explode_value(bytes_count, current_value);

  attribute_store_node_t value_node = create_setpoint_structure(type,
                                                                current_value,
                                                                precision,
                                                                scale,
                                                                min_value,
                                                                max_value);

  TEST_ASSERT_EQUAL(
    happy_case ? SL_STATUS_OK : SL_STATUS_NOT_SUPPORTED,
    setpoint_set(value_node, received_frame, &received_frame_size));

  if (happy_case) {
    std::vector<uint8_t> expected_frame {
      COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT,
      HUMIDITY_CONTROL_SETPOINT_SET,
      type,
      helper_get_attribute_field(precision, scale, bytes_count)};

    for (uint8_t value: expected_value) {
      expected_frame.emplace_back(value);
    }

    TEST_ASSERT_EQUAL(expected_frame.size(), received_frame_size);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame.data(),
                                  received_frame,
                                  received_frame_size);
  }
}

void helper_report_setpoint(enum set_bytes bytes_count, bool happy_case)
{
  humidity_control_setpoint_type_t expected_type
    = HUMIDITY_CONTROL_SETPOINT_REPORT_SETPOINT_TYPE_HUMIDIFIER;

  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  attribute_store_node_t value_node;

  humidity_control_setpoint_precision_t precision;
  humidity_control_setpoint_scale_t scale;
  humidity_control_setpoint_value_t value;

  humidity_control_setpoint_size_t size = bytes_count;

  switch (size) {
    case SET_1BYTES:
      precision  = 2;
      scale      = 1;
      value      = -40;
      value_node = create_setpoint_structure(expected_type,
                                             0,
                                             precision,
                                             0,
                                             happy_case ? -420 : 20,
                                             500);
      break;
    case SET_2BYTES:
      precision  = 5;
      scale      = 1;
      value      = INT16_MIN + 5;
      value_node = create_setpoint_structure(expected_type,
                                             0,
                                             precision,
                                             0,
                                             happy_case ? INT16_MIN : 0,
                                             50);
      break;
      break;
    case SET_4BYTES:
      precision = 1;
      scale     = 0;
      value     = INT16_MAX + 3256;
      value_node
        = create_setpoint_structure(expected_type,
                                    0,
                                    precision,
                                    0,
                                    happy_case ? INT16_MAX : INT32_MAX - 1,
                                    INT32_MAX);
      break;
  }

  std::vector<uint8_t> expected_frame {
    COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT,
    HUMIDITY_CONTROL_SETPOINT_REPORT,
    expected_type,
    helper_get_attribute_field(precision, scale, size)};

  for (uint8_t field: explode_value(size, value)) {
    expected_frame.emplace_back(field);
  }

  // Should be ok
  TEST_ASSERT_EQUAL_MESSAGE(happy_case ? SL_STATUS_OK : SL_STATUS_NOT_SUPPORTED,
                            handler.control_handler(&info,
                                                    expected_frame.data(),
                                                    expected_frame.size()),
                            "Report handler returned incorrect value");

  if (happy_case) {
    humidity_control_setpoint_value_t reported_value;
    attribute_store_get_reported(value_node,
                                 &reported_value,
                                 sizeof(reported_value));
    TEST_ASSERT_EQUAL_MESSAGE(
      get_signed_value_from_frame_and_size(explode_value(size, value).data(),
                                           size),
      reported_value,
      "Setpoint value is not correct");

    humidity_control_setpoint_precision_t reported_precision;
    attribute_store_get_child_reported(value_node,
                                       ATTRIBUTE(VALUE_PRECISION),
                                       &reported_precision,
                                       sizeof(reported_precision));
    TEST_ASSERT_EQUAL_MESSAGE(precision,
                              reported_precision,
                              "Precision is not correct");

    humidity_control_setpoint_scale_t reported_scale;
    attribute_store_get_child_reported(value_node,
                                       ATTRIBUTE(VALUE_SCALE),
                                       &reported_scale,
                                       sizeof(reported_scale));
    TEST_ASSERT_EQUAL_MESSAGE(scale, reported_scale, "Scale is not correct");
  }
}

/////////////////////////////////////////////////////
// Test case
/////////////////////////////////////////////////////
// Stub functions
static sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  if (node_type == ATTRIBUTE(SUPPORTED_TYPES)) {
    TEST_ASSERT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    supported_types_get = get_func;
  } else if (node_type == ATTRIBUTE(MIN_VALUE)) {
    TEST_ASSERT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    capabilities_get = get_func;
  } else if (node_type == ATTRIBUTE(VALUE)) {
    TEST_ASSERT_NOT_NULL(get_func);
    TEST_ASSERT_NOT_NULL(set_func);
    setpoint_get = get_func;
    setpoint_set = set_func;
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT,
                    handler.command_class);
  TEST_ASSERT_EQUAL(2, handler.version);
  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_NULL(handler.support_handler);
  TEST_ASSERT_FALSE(handler.manual_security_validation);

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

/// Called before each and every test
void setUp()
{
  zpc_attribute_store_test_helper_create_network();

  // Unset previous definition get/set functions
  supported_types_get = NULL;
  setpoint_get        = NULL;
  setpoint_set        = NULL;
  capabilities_get    = NULL;
  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size = 0;
  // Unset previous definition of handler
  memset(&handler, 0, sizeof(zwave_command_handler_t));

  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);
  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);
  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_humidity_control_setpoint_init());
}

/// Called after each and every test
void tearDown() {}

////////////////////////////////////////////////////////////////////////////
// Setpoint Set/Get/Report
////////////////////////////////////////////////////////////////////////////
void test_humidity_control_setpoint_get_happy_case()
{
  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL(setpoint_get);
  // Not supported with invalid node
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    setpoint_get(0, received_frame, &received_frame_size));

  humidity_control_setpoint_type_t expected_type
    = HUMIDITY_CONTROL_SETPOINT_GET_SETPOINT_TYPE_HUMIDIFIER;
  attribute_store_node_t type_node = create_type_node(expected_type);

  // This is the attribute that trigger the setpoint_get
  attribute_store_node_t value_node
    = attribute_store_add_node(ATTRIBUTE(VALUE), type_node);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    setpoint_get(value_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT,
                                    HUMIDITY_CONTROL_SETPOINT_GET,
                                    expected_type};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_humidity_control_setpoint_set_1bytes_happy_case()
{
  helper_set_setpoint(SET_1BYTES, true);
}

void test_humidity_control_setpoint_set_2bytes_happy_case()
{
  helper_set_setpoint(SET_2BYTES, true);
}

void test_humidity_control_setpoint_set_4bytes_happy_case()
{
  helper_set_setpoint(SET_4BYTES, true);
}

void test_humidity_control_setpoint_set_1bytes_out_of_bounds()
{
  helper_set_setpoint(SET_1BYTES, false);
}

void test_humidity_control_setpoint_set_2bytes_out_of_bounds()
{
  helper_set_setpoint(SET_2BYTES, false);
}

void test_humidity_control_setpoint_set_4bytes_out_of_bounds()
{
  helper_set_setpoint(SET_4BYTES, false);
}

void test_humidity_control_setpoint_report_1bytes_happy_case()
{
  helper_report_setpoint(SET_1BYTES, true);
}

void test_humidity_control_setpoint_report_2bytes_happy_case()
{
  helper_report_setpoint(SET_2BYTES, true);
}

void test_humidity_control_setpoint_report_4bytes_happy_case()
{
  helper_report_setpoint(SET_4BYTES, true);
}

void test_humidity_control_setpoint_report_1bytes_out_of_bounds()
{
  helper_report_setpoint(SET_1BYTES, false);
}
void test_humidity_control_setpoint_report_2bytes_out_of_bounds()
{
  helper_report_setpoint(SET_2BYTES, false);
}
void test_humidity_control_setpoint_report_4bytes_out_of_bounds()
{
  helper_report_setpoint(SET_4BYTES, false);
}

////////////////////////////////////////////////////////////////////////////
// Capabilities types Get/Report
////////////////////////////////////////////////////////////////////////////
void test_humidity_control_setpoint_capabilities_get_happy_case()
{
  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL(capabilities_get);
  // Not supported with invalid node
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    capabilities_get(0, received_frame, &received_frame_size));

  humidity_control_setpoint_type_t expected_type
    = HUMIDITY_CONTROL_SETPOINT_GET_SETPOINT_TYPE_HUMIDIFIER;
  attribute_store_node_t type_node
    = attribute_store_add_node(ATTRIBUTE(TYPE), endpoint_id_node);

  attribute_store_set_reported(type_node,
                               &expected_type,
                               sizeof(expected_type));

  // This is the attribute that trigger the capabilities_get
  attribute_store_node_t min_node
    = attribute_store_add_node(ATTRIBUTE(MIN_VALUE), type_node);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    capabilities_get(min_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT,
                                    HUMIDITY_CONTROL_SETPOINT_CAPABILITIES_GET,
                                    expected_type};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_humidity_control_setpoint_capabilities_report_happy_case()
{
  humidity_control_setpoint_type_t expected_type
    = HUMIDITY_CONTROL_SETPOINT_CAPABILITIES_REPORT_SETPOINT_TYPE_DEHUMIDIFIER_V2;

  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  humidity_control_setpoint_precision_t precision_min = 2;
  humidity_control_setpoint_scale_t scale_min         = 0;
  humidity_control_setpoint_size_t size_min           = 2;
  uint8_t value_min[]                                 = {12, 13};

  humidity_control_setpoint_precision_t precision_max = 4;
  humidity_control_setpoint_scale_t scale_max         = 1;
  humidity_control_setpoint_size_t size_max           = 4;
  uint8_t value_max[]                                 = {14, 15, 16, 17};

  const uint8_t frame[] = {
    COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT,
    HUMIDITY_CONTROL_SETPOINT_CAPABILITIES_REPORT,
    (uint8_t)(0b11110000
              | expected_type),  // This test if we properly ignore the first 4 bits
    helper_get_attribute_field(precision_min, scale_min, size_min),
    value_min[0],
    value_min[1],
    helper_get_attribute_field(precision_max, scale_max, size_max),
    value_max[0],
    value_max[1],
    value_max[2],
    value_max[3]};

  // Type not found
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&info, frame, sizeof(frame)));

  // Create type
  attribute_store_node_t type_node = create_type_node(expected_type);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info, frame, sizeof(frame)));

  attribute_store_node_t min_node
    = attribute_store_get_node_child_by_type(type_node,
                                             ATTRIBUTE(MIN_VALUE),
                                             0);

  attribute_store_node_t max_node
    = attribute_store_get_node_child_by_type(type_node,
                                             ATTRIBUTE(MAX_VALUE),
                                             0);

  TEST_ASSERT_NOT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                                min_node,
                                "Min value node should be defined");
  TEST_ASSERT_NOT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                                max_node,
                                "Max value node should be defined");

  helper_test_type_values(SETPOINT_MIN_VALUE,
                          min_node,
                          precision_min,
                          scale_min,
                          size_min,
                          value_min);

  helper_test_type_values(SETPOINT_MAX_VALUE,
                          max_node,
                          precision_max,
                          scale_max,
                          size_max,
                          value_max);
}

void test_humidity_control_setpoint_capabilities_report_fail_only_min()
{
  humidity_control_setpoint_type_t expected_type
    = HUMIDITY_CONTROL_SETPOINT_CAPABILITIES_REPORT_SETPOINT_TYPE_DEHUMIDIFIER_V2;

  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  uint8_t precision_min = 2;
  uint8_t scale_min     = 0;
  uint8_t size_min      = 2;
  uint8_t value_min[]   = {12, 13};

  const uint8_t frame[]
    = {COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT,
       HUMIDITY_CONTROL_SETPOINT_CAPABILITIES_REPORT,
       expected_type,
       helper_get_attribute_field(precision_min, scale_min, size_min),
       value_min[0],
       value_min[1]};

  // Type not found
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&info, frame, sizeof(frame)));
}

void test_humidity_control_setpoint_capabilities_report_fail_incorrect_size_field()
{
  humidity_control_setpoint_type_t expected_type
    = HUMIDITY_CONTROL_SETPOINT_CAPABILITIES_REPORT_SETPOINT_TYPE_DEHUMIDIFIER_V2;

  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  uint8_t precision_min = 2;
  uint8_t scale_min     = 0;
  uint8_t size_min      = 12;  // incorrect size
  uint8_t value_min[]   = {12, 13};

  uint8_t precision_max = 4;
  uint8_t scale_max     = 1;
  uint8_t size_max      = 4;
  uint8_t value_max[]   = {14, 15, 16, 17};

  const uint8_t frame[] = {
    COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT,
    HUMIDITY_CONTROL_SETPOINT_CAPABILITIES_REPORT,
    (uint8_t)(0b11110000
              | expected_type),  // This test if we properly ignore the first 4 bits
    helper_get_attribute_field(precision_min, scale_min, size_min),
    value_min[0],
    value_min[1],
    helper_get_attribute_field(precision_max, scale_max, size_max),
    value_max[0],
    value_max[1],
    value_max[2],
    value_max[3]};

  // Type not found
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&info, frame, sizeof(frame)));
}

void test_humidity_control_setpoint_capabilities_report_fail_incorrect_frame_size()
{
  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  const uint8_t frame[] = {COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT,
                           HUMIDITY_CONTROL_SETPOINT_CAPABILITIES_REPORT,
                           12,
                           13};
  // Type not found
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&info, frame, sizeof(frame)));
}

////////////////////////////////////////////////////////////////////////////
// Supported types Get/Report
////////////////////////////////////////////////////////////////////////////
void test_humidity_control_setpoint_supported_types_get_happy_case()
{
  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL(supported_types_get);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    supported_types_get(0, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT,
                                    HUMIDITY_CONTROL_SETPOINT_SUPPORTED_GET};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_humidity_control_setpoint_supported_types_report_happy_case()
{
  size_t expected_type_count = 3;
  humidity_control_setpoint_supported_types_t expected_supported_types
    = 0b00001110;

  const uint8_t frame[] = {COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT,
                           HUMIDITY_CONTROL_SETPOINT_SUPPORTED_REPORT,
                           expected_supported_types};

  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  // Simulate previous node
  create_type_node(0x0F);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info, frame, sizeof(frame)));

  humidity_control_setpoint_supported_types_t reported_supported_types;

  sl_status_t status
    = attribute_store_get_child_reported(endpoint_id_node,
                                         ATTRIBUTE(SUPPORTED_TYPES),
                                         &reported_supported_types,
                                         sizeof(reported_supported_types));

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            status,
                            "Should be able to get SUPPORTED_TYPES value");

  TEST_ASSERT_EQUAL_MESSAGE(
    expected_supported_types,
    reported_supported_types,
    "Supported type should be the same as in the report frame");

  size_t type_count
    = attribute_store_get_node_child_count_by_type(endpoint_id_node,
                                                   ATTRIBUTE(TYPE));

  TEST_ASSERT_EQUAL_MESSAGE(expected_type_count,
                            type_count,
                            "TYPE node count is incorrect");

  for (size_t i = 0; i < expected_type_count; i++) {
    attribute_store_node_t node
      = attribute_store_get_node_child_by_type(endpoint_id_node,
                                               ATTRIBUTE(TYPE),
                                               i);

    humidity_control_setpoint_type_t reported_value;
    sl_status_t status = attribute_store_get_reported(node,
                                                      &reported_value,
                                                      sizeof(reported_value));
    TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                              status,
                              "Should be able to get TYPE reported value");

    TEST_ASSERT_EQUAL_MESSAGE(i + 1,
                              reported_value,
                              "Incorrect TYPE value set");
  }
}

////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////
void test_attribute_creation_happy_case()
{
  attribute_store_node_t supported_types_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(SUPPORTED_TYPES));

  TEST_ASSERT_EQUAL_MESSAGE(
    ATTRIBUTE_STORE_INVALID_NODE,
    supported_types_node,
    "Supported type node shouldn't exist at this stage");

  zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(VERSION),
                                     &version,
                                     sizeof(version));

  supported_types_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(SUPPORTED_TYPES));

  TEST_ASSERT_NOT_EQUAL_MESSAGE(
    ATTRIBUTE_STORE_INVALID_NODE,
    supported_types_node,
    "Supported type node should exist at this stage");
}

void test_scale_bounds()
{
  attribute_store_node_t type_node = create_type_node(2);

  std::map<attribute_store_type_t, attribute_store_type_t>
    correspondence_table {{ATTRIBUTE(VALUE), ATTRIBUTE(VALUE_SCALE)},
                          {ATTRIBUTE(MIN_VALUE), ATTRIBUTE(MIN_VALUE_SCALE)},
                          {ATTRIBUTE(MAX_VALUE), ATTRIBUTE(MAX_VALUE_SCALE)}};

  std::map<attribute_store_node_t, attribute_store_type_t> scale_table;
  // Create correspondence between parent node ID and scale attribute
  for (auto &table_entry: correspondence_table) {
    attribute_store_node_t parent_node
      = attribute_store_add_node(table_entry.first, type_node);

    scale_table[parent_node] = table_entry.second;
  }

  for (auto &scale: scale_table) {
    humidity_control_setpoint_scale_t scale_value = 12;
    printf("Testing scale attribute #%d\n", scale.second);
    // Test at attribute creation
    attribute_store_node_t scale_node
      = attribute_store_emplace(scale.first,
                                scale.second,
                                &scale_value,
                                sizeof(scale_value));

    attribute_store_get_reported(scale_node, &scale_value, sizeof(scale_value));

    TEST_ASSERT_EQUAL_MESSAGE(
      0,
      scale_value,
      "Scale value should be equal to 0 if overflows (>1)");

    // Test at attribute update
    scale_value = 4;
    attribute_store_set_reported(scale_node, &scale_value, sizeof(scale_value));

    attribute_store_get_reported(scale_node, &scale_value, sizeof(scale_value));
    TEST_ASSERT_EQUAL_MESSAGE(
      0,
      scale_value,
      "Scale value should be equal to 0 if overflows (>1)");
  }
}

}  // extern "C"