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
// Test lib
#include "unity.h"

// Tested class
#include "zwave_frame_parser.hpp"

// Attribute store
#include "zpc_attribute_store_all_ids.h"
#include "zpc_attribute_store_type_registration.h"
#include "attribute_store_fixt.h"
#include "datastore_fixt.h"
#include "attribute.hpp"

// Test Helper
#include "zpc_attribute_store_test_helper.h"

extern "C" {

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_fixt_setup(":memory:");
  attribute_store_init();
  zpc_attribute_store_register_known_attribute_types();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  datastore_fixt_teardown();
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  zpc_attribute_store_test_helper_create_network();
}

#define TEST_ASSERT_EXCEPTION_MESSAGE(f, msg) \
  try {                                       \
    f;                                        \
    TEST_FAIL_MESSAGE(msg);                   \
  } catch (...) {                             \
  };

void test_is_frame_size_valid()
{
  const uint8_t tested_frame[] = {0x01, 0x02, 0x03, 0x04};
  uint8_t expected_frame_size  = sizeof(tested_frame);

  zwave_frame_parser parser(tested_frame, expected_frame_size);

  TEST_ASSERT_TRUE_MESSAGE(parser.is_frame_size_valid(expected_frame_size),
                           "Frame size should be valid");
  TEST_ASSERT_FALSE_MESSAGE(parser.is_frame_size_valid(expected_frame_size + 1),
                            "Frame size shouldn't be valid");
}

void test_read_happy_case()
{
  const uint8_t tested_frame[] = {0x01, 0x02, 0x03, 0x04};
  // In this test we perform 2 read and then test if it goes
  // out of bounds so we need it to be strictly equal to 4
  uint8_t expected_frame_size = 4;

  zwave_frame_parser parser(tested_frame, expected_frame_size);

  TEST_ASSERT_TRUE_MESSAGE(parser.is_frame_size_valid(expected_frame_size),
                           "Frame size should be valid");

  uint8_t value = parser.read_byte();
  TEST_ASSERT_EQUAL_MESSAGE(
    tested_frame[2],
    value,
    "First read should return the 3rd byte of the frame");

  // We need an attribute that is registered as uint8_t
  auto tested_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_VERSION,
                               endpoint_id_node);

  value = parser.read_byte(tested_node);
  TEST_ASSERT_EQUAL_MESSAGE(
    tested_frame[3],
    value,
    "Second read should return the 4th byte of the frame");

  attribute_store::attribute cpp_tested_node(tested_node);
  TEST_ASSERT_EQUAL_MESSAGE(tested_frame[3],
                            cpp_tested_node.get<uint8_t>(REPORTED_ATTRIBUTE),
                            "Tested node should have the same reported value "
                            "as the 4th byte of the frame");
}

// This function will assume that the parser have those values inside :
//
// Frame start
// // expected_int8_value
// 0x0F
// // expected_int16_value
// 0x0F
// 0xFF
// // expected_int32_value
// 0x0F
// 0xFF
// 0xFF
// 0xFF
// Frame end
void helper_test_signed_value(zwave_frame_parser &parser,
                              int8_t expected_int8_value,
                              int16_t expected_int16_value,
                              int32_t expected_int32_value)
{
  // Any Int32 value will do the trick here
  attribute_store::attribute int32_node
    = attribute_store_add_node(ATTRIBUTE_MANUFACTURER_SPECIFIC_MANUFACTURER_ID,
                               endpoint_id_node);

  // 8 bits
  int8_t returned_int8_value = parser.read_sequential<int32_t>(1, int32_node);
  TEST_ASSERT_EQUAL_MESSAGE(expected_int8_value,
                            returned_int8_value,
                            "Returned value mismatch for 8 bit signed value");
  // We need to test the reported value of a int32_t and not cast it to see if value matches
  TEST_ASSERT_EQUAL_MESSAGE(
    expected_int8_value,
    int32_node.reported<int32_t>(),
    "Node reported value mismatch for 8 bit signed value");

  // 16 bits
  int16_t returned_int16_value = parser.read_sequential<int32_t>(2, int32_node);
  TEST_ASSERT_EQUAL_MESSAGE(expected_int16_value,
                            returned_int16_value,
                            "Returned value mismatch for 16 bit signed value");
  // We need to test the reported value of a int32_t and not cast it to see if value matches
  TEST_ASSERT_EQUAL_MESSAGE(
    expected_int16_value,
    int32_node.reported<int32_t>(),
    "Node reported value mismatch for 16 bit signed value");

  // 16 bits
  int32_t returned_int32_value = parser.read_sequential<int32_t>(4, int32_node);
  TEST_ASSERT_EQUAL_MESSAGE(expected_int32_value,
                            returned_int32_value,
                            "Returned value mismatch for 32 bit signed value");
  // We need to test the reported value of a int32_t and not cast it to see if value matches
  TEST_ASSERT_EQUAL_MESSAGE(
    expected_int32_value,
    int32_node.reported<int32_t>(),
    "Node reported value mismatch for 32 bit signed value");
}

void test_read_signed_negative_values_happy_case()
{
  const uint8_t tested_frame[] = {0x01,
                                  0x02,
                                  // Signed 8 bit value (negative)
                                  0xF4,
                                  // Signed 16 bit value (negative)
                                  0xFB,
                                  0x44,
                                  // Signed 32 bit value(negative)
                                  0xFF,
                                  0xFE,
                                  0x26,
                                  0x84};

  uint8_t expected_frame_size = sizeof(tested_frame);

  zwave_frame_parser parser(tested_frame, expected_frame_size);

  TEST_ASSERT_TRUE_MESSAGE(parser.is_frame_size_valid(expected_frame_size),
                           "Frame size should be valid");

  helper_test_signed_value(parser, -12, -1212, -121212);
}

void test_read_signed_positive_values_happy_case()
{
  const uint8_t tested_frame[] = {0x01,
                                  0x02,
                                  // Signed 8 bit value (positive)
                                  0x0C,
                                  // Signed 16 bit value (positive)
                                  0x04,
                                  0xBC,
                                  // Signed 32 bit value(positive)
                                  0x00,
                                  0x01,
                                  0xD9,
                                  0x7C};

  uint8_t expected_frame_size = sizeof(tested_frame);

  zwave_frame_parser parser(tested_frame, expected_frame_size);

  TEST_ASSERT_TRUE_MESSAGE(parser.is_frame_size_valid(expected_frame_size),
                           "Frame size should be valid");

  helper_test_signed_value(parser, 12, 1212, 121212);
}

void test_read_signed_values_incompatible_size()
{
  const uint8_t tested_frame[] = {0x01, 0x02, 0x0C, 0x04, 0xBC};

  uint8_t expected_frame_size = sizeof(tested_frame);

  zwave_frame_parser parser(tested_frame, expected_frame_size);

  TEST_ASSERT_TRUE_MESSAGE(parser.is_frame_size_valid(expected_frame_size),
                           "Frame size should be valid");

  TEST_ASSERT_EXCEPTION_MESSAGE(
    parser.read_sequential<int32_t>(3),
    "Should throw an exception when trying to read incompatible size");
}

void test_read_out_of_bounds()
{
  const uint8_t tested_frame[] = {0x01, 0x02};
  uint8_t expected_frame_size  = sizeof(tested_frame);

  zwave_frame_parser parser(tested_frame, expected_frame_size);

  TEST_ASSERT_EXCEPTION_MESSAGE(
    parser.read_byte(),
    "Should throw an exception when trying to read out of bounds");

  auto tested_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_VERSION,
                               endpoint_id_node);
  TEST_ASSERT_EXCEPTION_MESSAGE(
    parser.read_byte(tested_node),
    "Should throw an exception when trying to read out of bounds");
}

struct read_sequential_nodes {
  attribute_store::attribute uint8_node      = {ATTRIBUTE_STORE_INVALID_NODE};
  attribute_store::attribute uint16_node     = {ATTRIBUTE_STORE_INVALID_NODE};
  attribute_store::attribute uint32_node     = {ATTRIBUTE_STORE_INVALID_NODE};
  attribute_store::attribute byte_array_node = {ATTRIBUTE_STORE_INVALID_NODE};
  attribute_store::attribute c_string_node   = {ATTRIBUTE_STORE_INVALID_NODE};
};

void helper_test_read_sequential_happy_case(read_sequential_nodes nodes = {})
{
  const uint8_t tested_frame[] = {0x00,
                                  0x00,
                                  // Read 1 byte
                                  0x12,
                                  // Read 2 bytes
                                  0x34,
                                  0x56,
                                  // Read 4 bytes
                                  0x78,
                                  0x9A,
                                  0xBC,
                                  0xDE,
                                  // Vector read 3 bytes,
                                  0xF0,
                                  0x00,
                                  0x0F,
                                  // String read (DoUzE)
                                  0x44,
                                  0x6F,
                                  0x55,
                                  0x7A,
                                  0x45};

  uint8_t expected_frame_size = sizeof(tested_frame);

  // Init parser
  zwave_frame_parser parser(tested_frame, expected_frame_size);

  // uint8_t
  uint8_t tested_uint8_value = tested_frame[2];
  TEST_ASSERT_EQUAL_MESSAGE(
    tested_uint8_value,
    parser.read_sequential<uint8_t>(1, nodes.uint8_node),
    "Should be able to read 1 byte from frame");

  if (nodes.uint8_node != ATTRIBUTE_STORE_INVALID_NODE) {
    TEST_ASSERT_EQUAL_MESSAGE(tested_uint8_value,
                              nodes.uint8_node.get<uint8_t>(REPORTED_ATTRIBUTE),
                              "Invalid uint8_t value in attribute store");
  }

  // uint16_t
  uint16_t tested_uint16_value = tested_frame[3] << 8 | tested_frame[4];
  TEST_ASSERT_EQUAL_MESSAGE(
    tested_uint16_value,
    parser.read_sequential<uint16_t>(2, nodes.uint16_node),
    "Should be able to read 2 bytes from frame");
  if (nodes.uint16_node != ATTRIBUTE_STORE_INVALID_NODE) {
    TEST_ASSERT_EQUAL_MESSAGE(
      tested_uint16_value,
      nodes.uint16_node.get<uint16_t>(REPORTED_ATTRIBUTE),
      "Invalid uint16_t value in attribute store");
  }

  // uint32_t
  uint32_t tested_uint32_value = tested_frame[5] << 24 | tested_frame[6] << 16
                                 | tested_frame[7] << 8 | tested_frame[8];
  TEST_ASSERT_EQUAL_MESSAGE(
    tested_uint32_value,
    parser.read_sequential<uint32_t>(4, nodes.uint32_node),
    "Should be able to read 4 bytes from frame");

  if (nodes.uint32_node != ATTRIBUTE_STORE_INVALID_NODE) {
    TEST_ASSERT_EQUAL_MESSAGE(
      tested_uint32_value,
      nodes.uint32_node.get<uint32_t>(REPORTED_ATTRIBUTE),
      "Invalid uint32_t value in attribute store");
  }

  // uint8_t vector
  std::vector<uint8_t> tested_vector_value = {0xF0, 0x00, 0x0F};
  std::vector<uint8_t> reported_vector_value
    = parser.read_sequential<std::vector<uint8_t>>(3, nodes.byte_array_node);

  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(
    tested_vector_value.data(),
    reported_vector_value.data(),
    tested_vector_value.size(),
    "Should be able to read 3 bytes from frame");

  if (nodes.byte_array_node != ATTRIBUTE_STORE_INVALID_NODE) {
    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(
      tested_vector_value.data(),
      nodes.byte_array_node.get<std::vector<uint8_t>>(REPORTED_ATTRIBUTE)
        .data(),
      tested_vector_value.size(),
      "Invalid uint8_t array value in attribute store");
  }

  // String
  const char *tested_string_value = "DoUzE";
  TEST_ASSERT_EQUAL_STRING_MESSAGE(
    tested_string_value,
    parser.read_sequential<std::string>(5, nodes.c_string_node).c_str(),
    "Should be able to read 5 bytes from frame");

  if (nodes.c_string_node != ATTRIBUTE_STORE_INVALID_NODE) {
    TEST_ASSERT_EQUAL_STRING_MESSAGE(
      tested_string_value,
      nodes.c_string_node.get<std::string>(REPORTED_ATTRIBUTE).c_str(),
      "Invalid c string value in attribute store");
  }

  // Test exception
  TEST_ASSERT_EXCEPTION_MESSAGE(
    parser.read_sequential<uint8_t>(1),
    "Should throw an exception when trying to read out of bounds");
}

void test_read_sequential_no_attributes_happy_case()
{
  helper_test_read_sequential_happy_case();
}
void test_read_sequential_with_attributes_happy_case()
{
  // Any nodes that have the right type and under endpoint_id will do the trick here
  read_sequential_nodes data = {
    .uint8_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_VERSION,
                               endpoint_id_node),
    .uint16_node = attribute_store_add_node(
      ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_NEXT_SUPPORTED_PARAMETER_ID,
      endpoint_id_node),
    .uint32_node = attribute_store_add_node(
      ATTRIBUTE_COMMAND_CLASS_CENTRAL_SCENE_ACTIVE_SCENE,
      endpoint_id_node),
    .byte_array_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_METER_SUPPORTED_SCALES,
                               endpoint_id_node),
    .c_string_node
    = attribute_store_add_node(ATTRIBUTE_NODE_NAME, endpoint_id_node),
  };

  helper_test_read_sequential_happy_case(data);
}

void test_read_sequential_out_of_bounds_int()
{
  const uint8_t tested_frame[] = {0x01, 0x02, 0x03};
  uint8_t expected_frame_size  = sizeof(tested_frame);

  zwave_frame_parser parser(tested_frame, expected_frame_size);

  TEST_ASSERT_EXCEPTION_MESSAGE(
    parser.read_sequential<uint16_t>(2),
    "Should throw an exception when trying to read out of bounds");
}

void test_read_sequential_out_of_bounds_vector()
{
  const uint8_t tested_frame[] = {0x01, 0x02, 0x03};
  uint8_t expected_frame_size  = sizeof(tested_frame);

  zwave_frame_parser parser(tested_frame, expected_frame_size);

  TEST_ASSERT_EXCEPTION_MESSAGE(
    parser.read_sequential<std::vector<uint8_t>>(2),
    "Should throw an exception when trying to read out of bounds");
}

void test_read_sequential_out_of_bounds_string()
{
  const uint8_t tested_frame[] = {0x01, 0x02, 0x03};
  uint8_t expected_frame_size  = sizeof(tested_frame);

  zwave_frame_parser parser(tested_frame, expected_frame_size);

  TEST_ASSERT_EXCEPTION_MESSAGE(
    parser.read_sequential<std::string>(2),
    "Should throw an exception when trying to read out of bounds");
}

void test_read_byte_with_bitmask_no_attribute_happy_case()
{
  const uint8_t tested_frame[] = {0x01, 0x02, 0b00001110, 0b10100111};
  uint8_t expected_frame_size  = sizeof(tested_frame);

  zwave_frame_parser parser(tested_frame, expected_frame_size);

  // Read first value
  constexpr uint8_t BITMASK_SINGLE                = 0b00000011;
  zwave_frame_parser::bitmask_data single_bitmask = {.bitmask = BITMASK_SINGLE};
  zwave_frame_parser::zwave_parser_bitmask_result reported_values
    = parser.read_byte_with_bitmask(single_bitmask);
  zwave_frame_parser::zwave_parser_bitmask_result tested_reported_values
    = {{BITMASK_SINGLE, 0b10}};

  TEST_ASSERT_TRUE_MESSAGE(
    reported_values == tested_reported_values,
    "Should be able to read first value with only one bitmask");

  // Read second value
  constexpr uint8_t BITMASK_MULTIPLE_A = 0b11000000;
  constexpr uint8_t BITMASK_MULTIPLE_B = 0b00111000;
  constexpr uint8_t BITMASK_MULTIPLE_C = 0b00000011;

  reported_values
    = parser.read_byte_with_bitmask({{.bitmask = BITMASK_MULTIPLE_A},
                                     {.bitmask = BITMASK_MULTIPLE_B},
                                     {.bitmask = BITMASK_MULTIPLE_C}});
  tested_reported_values = {{BITMASK_MULTIPLE_A, 0b10},
                            {BITMASK_MULTIPLE_B, 0b100},
                            {BITMASK_MULTIPLE_C, 0b11}};

  TEST_ASSERT_TRUE_MESSAGE(
    reported_values == tested_reported_values,
    "Should be able to read bitmask value with 3 bitmasks");
}

void test_read_byte_with_bitmask_with_attribute_happy_case()
{
  const uint8_t tested_frame[] = {0x01, 0x02, 0b00001110, 0b10100111};
  uint8_t expected_frame_size  = sizeof(tested_frame);

  zwave_frame_parser parser(tested_frame, expected_frame_size);

  // Any nodes that have the right type (uint8_t) and under endpoint_id will do the trick here
  attribute_store::attribute endpoint_id_node_cpp(endpoint_id_node);
  std::vector<attribute_store::attribute> tested_nodes = {
    endpoint_id_node_cpp.add_node(ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_VERSION),
    endpoint_id_node_cpp.add_node(
      ATTRIBUTE_COMMAND_CLASS_BARRIER_OPERATOR_VERSION),
    endpoint_id_node_cpp.add_node(ATTRIBUTE_COMMAND_CLASS_SWITCH_COLOR_VERSION),
    endpoint_id_node_cpp.add_node(ATTRIBUTE_COMMAND_CLASS_BASIC_VERSION)};

  // Read first value
  constexpr uint8_t BITMASK_SINGLE = 0b00000011;
  zwave_frame_parser::bitmask_data single_bitmask
    = {.bitmask = BITMASK_SINGLE, .destination_node = tested_nodes[0]};
  zwave_frame_parser::zwave_parser_bitmask_result reported_values
    = parser.read_byte_with_bitmask(single_bitmask);
  zwave_frame_parser::zwave_parser_bitmask_result tested_reported_values
    = {{BITMASK_SINGLE, 0b10}};

  TEST_ASSERT_TRUE_MESSAGE(
    reported_values == tested_reported_values,
    "Should be able to read first value with only one bitmask");

  TEST_ASSERT_EQUAL_MESSAGE(tested_reported_values[BITMASK_SINGLE],
                            tested_nodes[0].get<uint8_t>(REPORTED_ATTRIBUTE),
                            "Value on node_1 should match the returned value");

  // Read second value
  constexpr uint8_t BITMASK_MULTIPLE_A = 0b11000000;
  constexpr uint8_t BITMASK_MULTIPLE_B = 0b00111000;
  constexpr uint8_t BITMASK_MULTIPLE_C = 0b00000011;
  std::vector<zwave_frame_parser::bitmask_data> data
    = {{.bitmask = BITMASK_MULTIPLE_A, .destination_node = tested_nodes[1]},
       {.bitmask = BITMASK_MULTIPLE_B, .destination_node = tested_nodes[2]},
       {.bitmask = BITMASK_MULTIPLE_C, .destination_node = tested_nodes[3]}};
  reported_values = parser.read_byte_with_bitmask(data);

  tested_reported_values = {{BITMASK_MULTIPLE_A, 0b10},
                            {BITMASK_MULTIPLE_B, 0b100},
                            {BITMASK_MULTIPLE_C, 0b11}};

  TEST_ASSERT_TRUE_MESSAGE(
    reported_values == tested_reported_values,
    "Should be able to read bitmask value with 3 bitmasks");

  TEST_ASSERT_EQUAL_MESSAGE(
    tested_reported_values[BITMASK_MULTIPLE_A],
    tested_nodes[1].get<uint8_t>(REPORTED_ATTRIBUTE),
    "Value on node should match the returned value (tested_nodes[1])");
  TEST_ASSERT_EQUAL_MESSAGE(
    tested_reported_values[BITMASK_MULTIPLE_B],
    tested_nodes[2].get<uint8_t>(REPORTED_ATTRIBUTE),
    "Value on node should match the returned value (tested_nodes[2])");
  TEST_ASSERT_EQUAL_MESSAGE(
    tested_reported_values[BITMASK_MULTIPLE_C],
    tested_nodes[3].get<uint8_t>(REPORTED_ATTRIBUTE),
    "Value on node should match the returned value (tested_nodes[3])");
}

void test_read_bitmask_no_attribute_happy_case()
{
  const uint8_t tested_frame[] = {0x01, 0x02, 2, 0xFF, 0x02};
  uint8_t expected_frame_size  = sizeof(tested_frame);

  zwave_frame_parser parser(tested_frame, expected_frame_size);

  // Read first value
  zwave_report_bitmask_t reported_value = parser.read_bitmask();
  zwave_report_bitmask_t tested_value   = 0x02FF;

  TEST_ASSERT_EQUAL_MESSAGE(tested_value,
                            reported_value,
                            "Should be able to read support bitmask value");
}

void test_read_bitmask_attribute_happy_case()
{
  const uint8_t tested_frame[] = {0x01, 0x02, 3, 0x12, 0xFF, 0x02};
  uint8_t expected_frame_size  = sizeof(tested_frame);

  zwave_frame_parser parser(tested_frame, expected_frame_size);

  // Any nodes that have the right type (zwave_report_bitmask_t) and under endpoint_id will do the trick here
  attribute_store::attribute endpoint_id_node_cpp(endpoint_id_node);
  auto bitmask_node = endpoint_id_node_cpp.add_node(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SUPPORTED_MODES);

  // Read first value
  zwave_report_bitmask_t reported_value = parser.read_bitmask(bitmask_node);
  zwave_report_bitmask_t tested_value   = 0x02FF12;

  TEST_ASSERT_EQUAL_MESSAGE(tested_value,
                            reported_value,
                            "Should be able to read support bitmask value");

  TEST_ASSERT_EQUAL_MESSAGE(
    tested_value,
    bitmask_node.get<zwave_report_bitmask_t>(REPORTED_ATTRIBUTE),
    "Value on node should match the returned value");
}

void test_read_bitmask_value_too_big()
{
  const uint8_t tested_frame[] = {0x01, 0x02, 5, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  uint8_t expected_frame_size = sizeof(tested_frame);
  zwave_frame_parser parser(tested_frame, expected_frame_size);

  TEST_ASSERT_EXCEPTION_MESSAGE(parser.read_bitmask(),
                                "Should throw an exception when trying to read "
                                "a support bitmask value that is too big");
}

void test_read_string_no_attribute_happy_case()
{
  const uint8_t tested_frame[] = {0x01, 0x02, 5, 'D', 'o', 'U', 'z', 'E'};
  uint8_t expected_frame_size  = sizeof(tested_frame);

  zwave_frame_parser parser(tested_frame, expected_frame_size);

  // Read first value
  std::string reported_value = parser.read_string();
  const char *tested_value   = "DoUzE";

  TEST_ASSERT_EQUAL_STRING_MESSAGE(tested_value,
                                   reported_value.c_str(),
                                   "Should be able to read string value");
}

void test_read_string_with_attribute_happy_case()
{
  const uint8_t tested_frame[]
    = {0x01, 0x02, 10, 'C', 'H', 'O', 'K', 'B', 'A', 'R', '2', 'B', 'Z'};
  uint8_t expected_frame_size = sizeof(tested_frame);

  zwave_frame_parser parser(tested_frame, expected_frame_size);

  // Any nodes that have the right type (zwave_report_bitmask_t) and under endpoint_id will do the trick here
  attribute_store::attribute endpoint_id_node_cpp(endpoint_id_node);
  auto string_node = endpoint_id_node_cpp.add_node(ATTRIBUTE_NODE_NAME);

  // Read first value
  std::string reported_value = parser.read_string(string_node);
  const char *tested_value   = "CHOKBAR2BZ";

  TEST_ASSERT_EQUAL_STRING_MESSAGE(tested_value,
                                   reported_value.c_str(),
                                   "Should be able to read string value");

  TEST_ASSERT_EQUAL_STRING_MESSAGE(
    tested_value,
    string_node.get<std::string>(REPORTED_ATTRIBUTE).c_str(),
    "Value on node should match the returned value");
}

}  // extern "C"
