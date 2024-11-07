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
#include "zwave_frame_generator.hpp"

// Attribute store
#include "attribute_store_defined_attribute_types.h"
#include "zpc_attribute_store_type_registration.h"
#include "attribute_store_fixt.h"
#include "datastore_fixt.h"
#include "attribute.hpp"

// Test Helper
#include "zpc_attribute_store_test_helper.h"

extern "C" {

////////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////////
// Value does not matter, we are only testing the frame generation
static const uint8_t MAIN_COMMAND_CLASS = COMMAND_CLASS_SWITCH_BINARY;
static const uint8_t MAIN_COMMAND_ID    = 0x12;

// Frame generator static init, we don't have to create it for every tests
static zwave_frame_generator frame_generator(MAIN_COMMAND_CLASS);
static attribute_store::attribute endpoint_id_node_cpp;

////////////////////////////////////////////////////////////////////////
// Helper class
////////////////////////////////////////////////////////////////////////
class helper_test_generated_frame
{
  public:
  helper_test_generated_frame(zwave_frame_generator &frame_generator) :
    frame_generator(frame_generator)
  {}

  void initialize_frame(const std::vector<uint8_t> &expected_tested_frame,
                        uint8_t command_id = MAIN_COMMAND_ID)
  {
    expected_frame = expected_tested_frame;

    generated_frame.clear();
    generated_frame.resize(expected_frame.size());

    frame_generator.initialize_frame(command_id,
                                     generated_frame.data(),
                                     generated_frame.size());
  }

  void validate_generated_frame()
  {
    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(expected_frame.data(),
                                          generated_frame.data(),
                                          generated_frame.size(),
                                          "Frame mismatch");

    uint16_t reported_size = 0;
    frame_generator.validate_frame(&reported_size);
    TEST_ASSERT_EQUAL_MESSAGE(expected_frame.size(),
                              reported_size,
                              "Frame size mismatch");
  }

  private:
  zwave_frame_generator &frame_generator;

  std::vector<uint8_t> generated_frame;
  std::vector<uint8_t> expected_frame;
};

static helper_test_generated_frame helper_test_frame(frame_generator);

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
  endpoint_id_node_cpp = attribute_store::attribute(endpoint_id_node);
}

#define TEST_ASSERT_EXCEPTION_MESSAGE(f, msg) \
  try {                                       \
    f;                                        \
    TEST_FAIL_MESSAGE(msg);                   \
  } catch (...) {                             \
  };

////////////////////////////////////////////////////////////////////////
// Helper functions
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// Test functions
////////////////////////////////////////////////////////////////////////
void test_zwave_frame_generator_add_raw_value_happy_case()
{
  // First test with a single byte
  uint8_t command_id
    = MAIN_COMMAND_ID;  // Value does not matter, we are only testing the frame generation
  uint8_t tested_value   = 12;
  uint8_t tested_value_2 = 127;

  // Create & test frame 1
  std::vector<uint8_t> expected_frame
    = {MAIN_COMMAND_CLASS, command_id, tested_value};

  helper_test_frame.initialize_frame(expected_frame);
  frame_generator.add_raw_byte(tested_value);

  helper_test_frame.validate_generated_frame();

  // Second test with a multiples byte
  command_id   = 0x13;
  tested_value = 15;
  // Create frame 2
  expected_frame
    = {MAIN_COMMAND_CLASS, command_id, tested_value, tested_value_2};

  helper_test_frame.initialize_frame(expected_frame, command_id);
  frame_generator.add_raw_byte(tested_value);
  frame_generator.add_raw_byte(tested_value_2);

  helper_test_frame.validate_generated_frame();
}

void test_zwave_frame_generator_add_value_happy_case()
{
  // Setup attribute store
  uint8_t uint8_value = 12;
  auto uint8_node     = endpoint_id_node_cpp.add_node(
    ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_VERSION);
  uint8_node.set_reported(uint8_value);

  uint16_t uint16_value = 1212;
  auto uint16_node      = endpoint_id_node_cpp.add_node(
    ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_NODE_ID);
  uint16_node.set_desired(uint16_value);

  std::string string_value = "DoUzE";
  auto string_node         = endpoint_id_node_cpp.add_node(ATTRIBUTE_NODE_NAME);
  string_node.set_desired(string_value);

  std::vector<uint8_t> raw_data = {112, 113, 114};
  auto raw_data_node            = endpoint_id_node_cpp.add_node(
    ATTRIBUTE_COMMAND_CLASS_SENSOR_MULTILEVEL_SUPPORTED_SENSOR_TYPES);
  raw_data_node.set_reported(raw_data);

  // Create the expected frame
  // Test the frame
  std::vector<uint8_t> expected_frame
    = {MAIN_COMMAND_CLASS, MAIN_COMMAND_ID, uint8_value};
  // Uint16
  // MSB
  expected_frame.push_back((uint16_value & 0xFF00) >> 8);
  // LSB
  expected_frame.push_back((uint16_value & 0x00FF));
  // String
  for (auto c: string_value) {
    expected_frame.push_back(static_cast<uint8_t>(c));
  }
  // Raw data
  for (auto c: raw_data) {
    expected_frame.push_back(c);
  }

  // Generate the frame
  helper_test_frame.initialize_frame(expected_frame);
  frame_generator.add_value(uint8_node, DESIRED_OR_REPORTED_ATTRIBUTE);
  frame_generator.add_value(uint16_node, DESIRED_ATTRIBUTE);
  frame_generator.add_value(string_node, DESIRED_OR_REPORTED_ATTRIBUTE);
  frame_generator.add_value(raw_data_node);

  helper_test_frame.validate_generated_frame();
}

void test_zwave_frame_generator_add_value_invalid_node()
{
  std::vector<uint8_t> frame;
  uint16_t expected_size = 3;
  frame.resize(expected_size);

  // Generate the frame
  frame_generator.initialize_frame(MAIN_COMMAND_ID,
                                   frame.data(),
                                   expected_size);

  // Test with an invalid node
  TEST_ASSERT_EXCEPTION_MESSAGE(
    frame_generator.add_value(ATTRIBUTE_STORE_INVALID_NODE),
    "Invalid node should throw an exception");
}

void test_zwave_frame_generator_raw_shifted_value_happy_case()
{
  // Create expected frame
  std::vector<uint8_t> expected_frame
    = {MAIN_COMMAND_CLASS, MAIN_COMMAND_ID, 0b1100, 0b101100};

  std::vector<zwave_frame_generator::shifted_value> values = {
    {.left_shift = 2, .raw_value = 0b11},  // 0000 1100
    {.left_shift = 5, .raw_value = 0b1},   // 0010 0000
  };

  helper_test_frame.initialize_frame(expected_frame);
  frame_generator.add_shifted_values(values[0]);  // Should be 0b1100
  frame_generator.add_shifted_values(values);     // Should be 0b101100

  helper_test_frame.validate_generated_frame();
}

void test_zwave_frame_generator_node_shifted_value_happy_case()
{
  const std::vector<uint8_t> shift_values = {0, 3, 6};
  const std::vector<uint8_t> node_values  = {0b11, 0b1, 0b11};
  const std::vector<attribute_store_node_value_state_t> node_value_states
    = {REPORTED_ATTRIBUTE, DESIRED_ATTRIBUTE, DESIRED_OR_REPORTED_ATTRIBUTE};

  std::vector<attribute_store::attribute> attributes;
  std::vector<zwave_frame_generator::shifted_value> values;

  TEST_ASSERT_TRUE_MESSAGE(
    node_values.size() == node_value_states.size()
      && node_values.size() == shift_values.size(),
    "Node values and node value states should have the same size");

  for (size_t i = 0; i < node_values.size(); i++) {
    auto current_cpp_node = endpoint_id_node_cpp.add_node(
      ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_VERSION);

    attributes.push_back(current_cpp_node);

    auto current_state = node_value_states[i];
    current_state      = current_state == DESIRED_OR_REPORTED_ATTRIBUTE
                           ? DESIRED_ATTRIBUTE
                           : current_state;
    auto current_value = node_values[i];

    current_cpp_node.set<uint8_t>(current_state, current_value);

    values.push_back({.left_shift       = shift_values[i],
                      .node             = current_cpp_node,
                      .node_value_state = current_state});
  }

  std::vector<zwave_frame_generator::shifted_value> values_mix = {
    {.left_shift       = 2,
     .node             = attributes[0],
     .node_value_state = node_value_states[0]},
    {.left_shift = 7, .raw_value = 0b1},
  };

  // Create expected frame
  std::vector<uint8_t> expected_frame
    = {MAIN_COMMAND_CLASS, MAIN_COMMAND_ID, 0b11001011, 0b10001100};

  // Generate the frame
  helper_test_frame.initialize_frame(expected_frame);
  frame_generator.add_shifted_values(values);      // Should be 0b11001011
  frame_generator.add_shifted_values(values_mix);  // Should be 0b10001100

  helper_test_frame.validate_generated_frame();
}

void test_zwave_frame_generator_node_shifted_value_invalid_node()
{
  std::vector<uint8_t> frame;
  uint16_t expected_size = 3;
  frame.resize(expected_size);

  // Node 1212 should exists here
  zwave_frame_generator::shifted_value value = {.left_shift = 2, .node = 1212};
  // Generate the frame
  frame_generator.initialize_frame(MAIN_COMMAND_ID,
                                   frame.data(),
                                   expected_size);

  // Test with an invalid node
  TEST_ASSERT_EXCEPTION_MESSAGE(frame_generator.add_shifted_values(value),
                                "Invalid node ID should throw an exception");
}

void test_zwave_frame_generator_out_of_bounds()
{
  std::vector<uint8_t> frame;
  uint16_t expected_size = 2;
  frame.resize(expected_size);

  // Node 1212 should exists here
  zwave_frame_generator::shifted_value value
    = {.left_shift = 2, .raw_value = 12};
  // Generate the frame
  frame_generator.initialize_frame(MAIN_COMMAND_ID,
                                   frame.data(),
                                   expected_size);

  // Test with shifted value
  TEST_ASSERT_EXCEPTION_MESSAGE(
    frame_generator.add_shifted_values(value),
    "Out of bound write should throw an exception for add_shifted_values");

  // Test raw byte
  TEST_ASSERT_EXCEPTION_MESSAGE(
    frame_generator.add_raw_byte(12),
    "Out of bound write should throw an exception for add_raw_byte");

  uint16_t uint16_value = 1212;
  auto uint16_node      = endpoint_id_node_cpp.add_node(
    ATTRIBUTE_COMMAND_CLASS_ALARM_SENSOR_NODE_ID);
  uint16_node.set_reported<uint16_t>(uint16_value);
  // Test raw byte
  TEST_ASSERT_EXCEPTION_MESSAGE(
    frame_generator.add_value(uint16_node),
    "Out of bound write should throw an exception for add_value");
}

void test_zwave_frame_generator_generate_no_args_frame()
{
  // Create expected frame
  std::vector<uint8_t> expected_frame = {MAIN_COMMAND_CLASS, MAIN_COMMAND_ID};

  std::vector<uint8_t> generated_frame;
  generated_frame.resize(expected_frame.size());

  uint16_t frame_length = 0;
  sl_status_t status
    = frame_generator.generate_no_args_frame(MAIN_COMMAND_ID,
                                             generated_frame.data(),
                                             &frame_length);

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK, status, "Failed to generate frame");

  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(expected_frame.data(),
                                        generated_frame.data(),
                                        generated_frame.size(),
                                        "Frame mismatch");

  TEST_ASSERT_EQUAL_MESSAGE(expected_frame.size(),
                            frame_length,
                            "Frame size mismatch");
}

}  // extern "C"