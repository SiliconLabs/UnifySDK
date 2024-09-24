
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

// Base class
#include "zwave_command_class_test_helper.hpp"

// Unify
#include "zwave_command_classes_utils.h"

// C++ includes
#ifdef __cplusplus
#include <string>
#include <cstring>  // memset
#include <boost/format.hpp>
#endif

extern "C" {

// Mocks
#include "zwave_command_handler_mock.h"
#include "attribute_resolver_mock.h"

namespace zwave_command_class_test_helper
{

// Buffers for frame. We use a static C array here to match the C API
uint8_t received_frame[255];
uint16_t received_frame_size;
// ZWave current command handler
zwave_command_handler_t handler;
// Resolver function helper
zwave_resolver_function_helper resolver_function_helper;

// Current current command class id. We use uint8_t here instead of zwave_command_class_t to be able to set it directly in the tested frames.
uint8_t current_command_class_id;
// Endpoint id node wrapper
attribute_store::attribute cpp_endpoint_id_node;
// Used for report frames
zwave_controller_connection_info_t mock_connection_info;

// Keep a copy of handler args for testing in stub function
zwave_struct_handler_args handler_args;

////////////////////////////////////////////////////////////////////////////////////
// Suit setup
////////////////////////////////////////////////////////////////////////////////////

void zwave_setUp(const zwave_struct_handler_args &args,
                 const command_class_init_function &init_function,
                 const resolver_function_map &bindings)
{
  // Create base structure
  zpc_attribute_store_test_helper_create_network();

  // Clear received frame
  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size = 0;

  // Unset previous definition of handler
  memset(&handler, 0, sizeof(zwave_command_handler_t));

  // Resolver function helper
  resolver_function_helper.set_resolver_functions(bindings);
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  // Handler registration
  handler_args = args;
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  // Save current command class
  current_command_class_id = handler_args.command_class_id;

  // Create wrapper around endpoint id node
  cpp_endpoint_id_node = endpoint_id_node;

  // Setup connection info
  mock_connection_info                    = {};
  mock_connection_info.remote.node_id     = node_id;
  mock_connection_info.remote.endpoint_id = endpoint_id;
  mock_connection_info.local.is_multicast = false;

  // Call init function
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    init_function(),
    "Command class init function should have return SL_STATUS_OK");
}

////////////////////////////////////////////////////////////////////////////////////
// Stub defintions
////////////////////////////////////////////////////////////////////////////////////
sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  handler = new_command_class_handler;
  helper_test_handler(handler_args);
  return SL_STATUS_OK;
}

sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  return resolver_function_helper.register_resolver_functions(node_type,
                                                              set_func,
                                                              get_func);
}
////////////////////////////////////////////////////////////////////////////////////
// Frame generation helpers
////////////////////////////////////////////////////////////////////////////////////
void zwave_frame::add(uint8_t value)
{
  this->push_back(value);
}

void zwave_frame::add(uint16_t value)
{
  this->push_back((value & 0xFF00) >> 8);
  this->push_back(value & 0x00FF);
}

void zwave_frame::add(uint32_t value)
{
  this->push_back((value & 0xFF000000) >> 24);
  this->push_back((value & 0x00FF0000) >> 16);
  this->push_back((value & 0x0000FF00) >> 8);
  this->push_back(value & 0x000000FF);
}

void zwave_frame::add(const std::string &value)
{
  this->push_back(static_cast<uint8_t>(value.size()));
  for (const auto &c: value) {
    this->push_back(c);
  }
}

void zwave_frame::add(const std::vector<uint8_t> &bytes)
{
  this->push_back(static_cast<uint8_t>(bytes.size()));
  this->insert(this->end(), bytes.cbegin(), bytes.cend());
}

void zwave_frame::add_bitmask(uint8_t size, uint32_t bitmask)
{
  this->push_back(size);

  for (uint8_t i = 0; i < size; i++) {
    uint8_t current_offset   = i;
    uint8_t current_shift    = 8 * current_offset;
    uint32_t current_bitmask = 0xFF << current_shift;

    uint8_t value_8bit = (current_bitmask & bitmask) >> current_shift;
    this->push_back(value_8bit);
  }
}

////////////////////////////////////////////////////////////////////////////////////
// Attribute store helpers
////////////////////////////////////////////////////////////////////////////////////
void helper_set_version(const zwave_cc_version_t &version)
{
  cpp_endpoint_id_node.add_node(current_command_class_id << 8 | 0x01)
    .set_reported(version);
}

zwave_cc_version_t helper_get_version()
{
  return zwave_command_class_get_version_from_node(cpp_endpoint_id_node,
                                                   current_command_class_id);
}

////////////////////////////////////////////////////////////////////////////////////
// Helpers
////////////////////////////////////////////////////////////////////////////////////
void helper_test_handler(const zwave_struct_handler_args &args)
{
  TEST_ASSERT_EQUAL_MESSAGE(args.scheme,
                            handler.minimal_scheme,
                            "Minimal scheme mismatch in handler");
  TEST_ASSERT_EQUAL_MESSAGE(args.command_class_id,
                            handler.command_class,
                            "Command class mismatch in handler");
  TEST_ASSERT_EQUAL_MESSAGE(args.supported_version,
                            handler.version,
                            "Version mismatch in handler");

  if (args.has_control_handler) {
    TEST_ASSERT_NOT_NULL_MESSAGE(handler.control_handler,
                                 "Handler should have control handler");
  } else {
    TEST_ASSERT_NULL_MESSAGE(handler.control_handler,
                             "Handler should NOT have control handler");
  }

  if (args.has_support_handler) {
    TEST_ASSERT_NOT_NULL_MESSAGE(handler.support_handler,
                                 "Handler should have support handler");
  } else {
    TEST_ASSERT_NULL_MESSAGE(handler.support_handler,
                             "Handler should NOT have support handler");
  }

  TEST_ASSERT_FALSE(handler.manual_security_validation);
}

void helper_test_get_set_frame_happy_case(uint8_t command_id,
                                          attribute_store_node_t base_node,
                                          const std::vector<uint8_t> &args)
{
  auto &resolver_function
    = resolver_function_helper.get_resolver_function(command_id);

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    resolver_function(base_node, received_frame, &received_frame_size),
    (boost::format("Set/Get command '0x%02x' should have worked")
     % unsigned(command_id))
      .str()
      .c_str());

  // Construct frame
  std::vector<uint8_t> expected_frame = {current_command_class_id, command_id};
  expected_frame.insert(expected_frame.end(), args.cbegin(), args.cend());

  TEST_ASSERT_EQUAL_MESSAGE(
    expected_frame.size(),
    received_frame_size,
    (boost::format("Frame size is incorrect for set/get command '0x%02x'")
     % unsigned(command_id))
      .str()
      .c_str());
  TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(
    expected_frame.data(),
    received_frame,
    received_frame_size,
    (boost::format("Frame contents mismatch for set/get command '0x%02x'")
     % unsigned(command_id))
      .str()
      .c_str());
}

void helper_test_get_set_fail_case(uint8_t command_id,
                                   sl_status_t expected_status,
                                   attribute_store_node_t base_node)
{
  auto &resolver_function
    = resolver_function_helper.get_resolver_function(command_id);

  TEST_ASSERT_EQUAL_MESSAGE(
    expected_status,
    resolver_function(base_node, received_frame, &received_frame_size),
    (boost::format("Set/Get command '0x%02x' should NOT have worked")
     % unsigned(command_id))
      .str()
      .c_str());
}

void helper_test_report_frame(uint8_t command_id,
                              const std::vector<uint8_t> &args,
                              sl_status_t expected_status)
{
  std::vector<uint8_t> report_frame = {current_command_class_id, command_id};
  report_frame.insert(report_frame.end(), args.cbegin(), args.cend());

  TEST_ASSERT_NOT_NULL_MESSAGE(handler.control_handler,
                               "Control handler should be defined. Check that you've call zwave_setUp() and you linked the mock libraries against this test.");
  sl_status_t result = handler.control_handler(&mock_connection_info,
                                               report_frame.data(),
                                               report_frame.size());

  std::string debug_message
    = expected_status == SL_STATUS_OK
        ? "Report frame (0x%02x) should have worked"
        : "Report frame (0x%02x) should NOT have worked";
  debug_message = (boost::format(debug_message) % unsigned(command_id)).str();

  TEST_ASSERT_EQUAL_MESSAGE(expected_status, result, debug_message.c_str());
}

void helper_test_node_existence(attribute_store::attribute attribute,
                                bool should_exists,
                                const std::string &expected_attribute_name,
                                const std::string &expected_parent_name)
{
  TEST_ASSERT_EQUAL_MESSAGE(
    should_exists,
    attribute.is_valid(),
    (boost::format("Attribute '%1%' should %2% exists under '%3%'")
     % expected_attribute_name % (should_exists ? "" : "NOT")
     % expected_parent_name)
      .str()
      .c_str());
}

attribute_store::attribute
  helper_test_and_get_node(attribute_store_type_t node_type,
                           attribute_store::attribute parent)
{
  auto attribute = parent.child_by_type(node_type);

  helper_test_node_existence(attribute,
                             true,
                             attribute_store_get_type_name(node_type),
                             parent.name());
  return attribute;
}

void helper_test_node_exists(attribute_store_type_t node_type,
                             attribute_store::attribute parent)
{
  helper_test_node_existence(parent.child_by_type(node_type),
                             true,
                             attribute_store_get_type_name(node_type),
                             parent.name());
}
void helper_test_node_does_not_exists(attribute_store_type_t node_type,
                                      attribute_store::attribute parent)
{
  helper_test_node_existence(parent.child_by_type(node_type),
                             false,
                             attribute_store_get_type_name(node_type),
                             parent.name());
}

}  // namespace zwave_command_class_test_helper
}  // extern "C"