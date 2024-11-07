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

#ifndef ZWAVE_COMMAND_CLASS_TEST_HELPER_HPP
#define ZWAVE_COMMAND_CLASS_TEST_HELPER_HPP

// Note :
// Put c++ includes here, and C includes below extern "C" to avoid error
// "error while loading shared libraries: unexpected PLT reloc type 0x00"

// Unify cpp
#include "attribute.hpp"
#include "zwave_resolver_function_helper.hpp"

// C++ includes
#include <vector>
#include <functional>

extern "C" {
// Test framework
#include "unity.h"

// Helper class
#include "zpc_attribute_store_test_helper.h"

// Unify
#include "zwave_command_handler.h"

namespace zwave_command_class_test_helper
{
////////////////////////////////////////////////////////////////////////////////////
// Alias
////////////////////////////////////////////////////////////////////////////////////
using command_class_init_function = std::function<sl_status_t(void)>;

////////////////////////////////////////////////////////////////////////////////////
// Struct
////////////////////////////////////////////////////////////////////////////////////
struct zwave_struct_handler_args {
  zwave_command_class_t command_class_id;
  zwave_cc_version_t supported_version;
  zwave_controller_encapsulation_scheme_t scheme
    = ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME;
  bool has_control_handler = true;
  bool has_support_handler = false;
};

////////////////////////////////////////////////////////////////////////////////////
// CONST
////////////////////////////////////////////////////////////////////////////////////
static const uint8_t MAX_CHAR_SIZE = 64;

////////////////////////////////////////////////////////////////////////////////////
// Global variables
// Must be declared as "extern" and defined in the cpp to avoid multiple definition
// More information : https://stackoverflow.com/questions/11478152/how-to-work-with-variable-in-namespace
////////////////////////////////////////////////////////////////////////////////////

// Buffers for frame. We use a static C array here to match the C API
extern uint8_t received_frame[255];
extern uint16_t received_frame_size;
// ZWave current command handler
extern zwave_command_handler_t handler;
// Resolver function helper
extern zwave_resolver_function_helper resolver_function_helper;

// Current current command class id. We use uint8_t here instead of zwave_command_class_t to be able to set it directly in the tested frames.
extern uint8_t current_command_class_id;
// Endpoint id node wrapper
extern attribute_store::attribute cpp_endpoint_id_node;
// Used for report frames
extern zwave_controller_connection_info_t mock_connection_info;

////////////////////////////////////////////////////////////////////////////////////
// Suit setup
////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Setup the Z-Wave test suite
 * 
 * It will setup all the variables needed for the tests. 
 *
 * @code
 * // Tested command class handler
    const zwave_struct_handler_args command_class_handler
      = {.command_class_id  = COMMAND_CLASS_SWITCH_BINARY, // Will be stored in current_command_class_id
        .supported_version = 2,
        .scheme            = ZWAVE_CONTROLLER_ENCAPSULATION_NONE};
    // Get Set function map
    const resolver_function_map attribute_bindings = {
      {ATTRIBUTE(VALUE), {SWITCH_BINARY_GET, SWITCH_BINARY_SET}},
    };

    /// Called before each and every test
    void setUp()
    {
      zwave_setUp(command_class_handler,
                  &zwave_command_class_binary_switch_init,
                  attribute_bindings);
    }
 * @endcode
 * 
 * One the initial setup is done, you can use all the function in the namespace to help you : 
 * This function will call the resolver function you've set with attribute_bindings and check it return value.
 * 
 * @code
    void test_binary_switch_get_happy_case()
    {
      helper_test_get_set_frame_happy_case(SWITCH_BINARY_GET);
    }
 * @endcode
 * 
 * @param args Test data against the zwave handler. Checks command class, version, handlers...
 * @param init_function The function to call to initialize the command class. Check if it returns SL_STATUS_OK.
 * @param bindings The resolver functions to bind to the command class. Allows to use helper_test_get_set_frame_happy_case() helpers functions
 * 
 */
void zwave_setUp(const zwave_struct_handler_args &args,
                 const command_class_init_function &init_function,
                 const resolver_function_map &bindings = {});

////////////////////////////////////////////////////////////////////////////////////
// Stub defintions
////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Associated callback for zwave_command_handler_register_handler_Stub
 */
sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls);

/**
 * @brief Associated callback for attribute_resolver_register_rule_Stub
 */
sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls);

////////////////////////////////////////////////////////////////////////////////////
// Frame generation helpers
////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Helper to generate a frame contents. Can be used to generate 
 * any zwave_frame and test it.
 * 
 * @note This class inherits from vector directly.
 **/
class zwave_frame : public std::vector<uint8_t> {
 public:
  zwave_frame() = default;
  /**
   * @brief Add a byte to the frame
   * 
   * @param byte Byte to add
   */
  void add(uint8_t byte);
  /**
   * @brief Add a 16 bits value to the frame (msb first, lsb second)
   * 
   * @param value Value to add
   */
  void add(uint16_t value);
  /**
   * @brief Add a 32 bits value to the frame (msb first, lsb last)
   * 
   * @param value Value to add
   */
  void add(uint32_t value);
  /**
   * @brief Add a string to the frame (with its size before, without NULL terminator)
   * 
   * > String size N
   * > Char 1
   * > Char 2
   * > ...
   * > Char N
   * 
   * @param value String to add
   */
  void add(const std::string &value);
  /**
   * @brief Add a vector of bytes to the frame (with its size before)
   * 
   * > Vector size N
   * > Byte 1
   * > Byte 2
   * > ...
   * > Byte N
   * 
   * @param bytes Bytes to add
   */
  void add(const std::vector<uint8_t> &bytes);

  /**
   * @brief Add a ZWave bitmask to the frame (with its size before, lsb first msb last)
   * 
   * > Bitmask size N
   * > Bitmask first byte (LSB)
   * > Bitmask second byte
   * > ...
   * > Bitmask N byte (MSB)
   * 
   * @param size Size of the bitmask
   * @param bitmask Bitmask to add
   */
  void add_bitmask(uint8_t size, uint32_t bitmask);
};

////////////////////////////////////////////////////////////////////////////////////
// Attribute store helpers
////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Set version for current class
 * 
 * @note Use current_command_class_id global variable to determine the current current class
 * 
 * @brief Command class version
 */
void helper_set_version(const zwave_cc_version_t &version);

/**
 * @brief Get version for current class
 * 
 * @note Use current_command_class_id global variable to determine the current current class
 * 
 * @return Command class version
 */
zwave_cc_version_t helper_get_version();

////////////////////////////////////////////////////////////////////////////////////
// Global Test Helpers
////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Test current handler with given arguments
 * 
 * Make the test fail if any value in the struct doesn't match the handler value
 * 
 * @param args Test data against the zwave handler. Checks all the paramaters in the struct
 * 
 */
void helper_test_handler(const zwave_struct_handler_args &args);

////////////////////////////////////////////////////////////////////////////////////
// Get/Set Test Helpers
////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Call get/set function and check the frame contents and return value (SL_STATUS_OK)
 * 
 * This function will check internally the associated function with given command_id. 
 * It will then call it with the given base_node and checks the frame contents.
 * The function needs to return SL_STATUS_OK to pass the test.
 * 
 * @note Only works correctly if you've set the bindings in zwave_setUp()
 * 
 * @param command_id Command id to test
 * @param base_node Base node that will be given to the resolver function
 * @param args Expected frame contents after {current_command_class_id, command_id}. Can be left empty.
 */
void helper_test_get_set_frame_happy_case(uint8_t command_id,
                                          attribute_store_node_t base_node
                                          = ATTRIBUTE_STORE_INVALID_NODE,
                                          const std::vector<uint8_t> &args
                                          = {});

/**
 * @brief Call get/set function and check that it has failed
 * 
 * This function will check internally the associated function with given command_id.
 * It will then call it with the given base_node and checks the return value.
 * The function needs to return the expected_status to pass the test.
 * 
 * @note Only works correctly if you've set the bindings in zwave_setUp()
 * 
 * @param command_id Command id to test
 * @param expected_status Expected return value of the function
 * @param base_node Base node that will be given to the resolver function
 */
void helper_test_get_set_fail_case(uint8_t command_id,
                                   sl_status_t expected_status
                                   = SL_STATUS_NOT_SUPPORTED,
                                   attribute_store_node_t base_node
                                   = ATTRIBUTE_STORE_INVALID_NODE);

////////////////////////////////////////////////////////////////////////////////////
// Report Test Helpers
////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Call report function with given frame and check it return status
 * 
 * @param command_id Command id to test
 * @param args Frame contents after {current_command_class_id, command_id}
 * @param expected_status Expected return value of the function
 */
void helper_test_report_frame(uint8_t command_id,
                              const std::vector<uint8_t> &args,
                              sl_status_t expected_status = SL_STATUS_OK);

////////////////////////////////////////////////////////////////////////////////////
// Node/Attribute Test Helpers
////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Get a node and check that it exists
 * 
 * @note Test will fail if node doesn't exists
 * 
 * @param node_type Node type to get
 * @param parent Parent node of the node to get (default to current endpoint)
 * 
 * @return attribute_store::attribute Node that was found (garmented to exists)
 */
attribute_store::attribute
  helper_test_and_get_node(attribute_store_type_t node_type,
                           attribute_store::attribute parent
                           = cpp_endpoint_id_node);

/**
 * @brief Test that a node exists
 * 
 * @param node_type Node type to test
 * @param parent Parent node of the node to get (default to current endpoint)
 */
void helper_test_node_exists(attribute_store_type_t node_type,
                             attribute_store::attribute parent
                             = cpp_endpoint_id_node);
/**
 * @brief Test that a node doesn't exists
 * 
 * @param node_type Node type to test
 * @param parent Parent node of the node to get (default to current endpoint)
 */
void helper_test_node_does_not_exists(attribute_store_type_t node_type,
                                      attribute_store::attribute parent
                                      = cpp_endpoint_id_node);

}  // namespace zwave_command_class_test_helper

}  // extern "C"

// Cpp template functions
namespace zwave_command_class_test_helper
{
template<typename T> attribute_store::attribute helper_test_attribute_value(
  attribute_store_type_t node_type,
  T expected_value,
  attribute_store::attribute parent        = cpp_endpoint_id_node,
  attribute_store_node_value_state_t state = REPORTED_ATTRIBUTE)
{
  auto current_node = helper_test_and_get_node(node_type, parent);

  try {
    const std::string error_message
      = (std::string("Value mismatch for ") + current_node.name_and_id())
          .c_str();

    if constexpr (std::is_same<T, std::vector<uint8_t>>::value) {
      TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(
        expected_value.data(),
        current_node.reported<std::vector<uint8_t>>().data(),
        expected_value.size(),
        error_message.c_str());
    } else if constexpr (std::is_same<T, std::string>::value) {
      TEST_ASSERT_EQUAL_STRING_MESSAGE(
        expected_value.c_str(),
        current_node.reported<std::string>().c_str(),
        error_message.c_str());
    } else {
      TEST_ASSERT_EQUAL_MESSAGE(expected_value,
                                current_node.get<T>(state),
                                error_message.c_str());
    }
  } catch (std::exception &e) {
    TEST_FAIL_MESSAGE(e.what());
  }

  return current_node;
}
}  // namespace zwave_command_class_test_helper

#endif  // ZWAVE_COMMAND_CLASS_TEST_HELPER_HPP