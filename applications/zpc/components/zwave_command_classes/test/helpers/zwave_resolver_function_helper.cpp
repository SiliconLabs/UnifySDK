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
#include "zwave_resolver_function_helper.hpp"

// Unify
#include "sl_status.h"

// C++ includes
#include <string>

// Test fuctions
#include "unity.h"

extern "C" {
namespace zwave_command_class_test_helper
{

void zwave_resolver_function_helper::set_resolver_functions(
  const resolver_function_map &bindings)
{
  attributes_binding = bindings;
  // Clear previous resolver functions
  resolver_functions.clear();
};

sl_status_t zwave_resolver_function_helper::register_resolver_functions(
  attribute_store_type_t node_type,
  attribute_resolver_function_t set_func,
  attribute_resolver_function_t get_func)
{
  std::string message;
  std::string node_name = attribute_store_get_type_name(node_type);

  if (attributes_binding.find(node_type) != attributes_binding.end()) {
    const auto func = attributes_binding.at(node_type);

    if (func.get_func_id == 0) {
      message = "GET function should not be defined for " + node_name
                + ". Did you forget to add it to the binding attribute ?";
      TEST_ASSERT_NULL_MESSAGE(get_func, message.c_str());
    } else {
      message = "SET function should be defined for " + node_name
                + ". Did you forget to remove it to the binding attribute ?";
      TEST_ASSERT_NOT_NULL_MESSAGE(get_func, message.c_str());
      resolver_functions[func.get_func_id] = get_func;
    }

    if (func.set_func_id == 0) {
      message = "SET function should not be defined for " + node_name
                + ". Did you forget to add it to the binding attribute ?";
      TEST_ASSERT_NULL_MESSAGE(set_func, message.c_str());
    } else {
      message = "SET function should be defined for " + node_name
                + ". Did you forget to remove it to the binding attribute ?";
      TEST_ASSERT_NOT_NULL_MESSAGE(set_func, message.c_str());
      resolver_functions[func.set_func_id] = set_func;
    }
  }

  return SL_STATUS_OK;
}

attribute_resolver_function_t &
  zwave_resolver_function_helper::get_resolver_function(uint8_t function_id)
{
  if (resolver_functions.find(function_id) == resolver_functions.end()) {
    TEST_FAIL_MESSAGE("Resolver function not found.");
  }

  return resolver_functions[function_id];
}

}  // namespace zwave_command_class_test_helper
}  // extern "C"