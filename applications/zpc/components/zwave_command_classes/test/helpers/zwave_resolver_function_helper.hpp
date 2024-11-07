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

#ifndef ZWAVE_RESOLVER_FUNCTION_HELPER_HPP
#define ZWAVE_RESOLVER_FUNCTION_HELPER_HPP

// C++ includes
#include <map>
#include <iostream>

// Unify
#include "attribute_store.h"
#include "attribute_store_type_registration.h"
#include "attribute_resolver.h"

extern "C" {
namespace zwave_command_class_test_helper
{

/**
 * @brief Z-Wave functions ID
 * 
 * @warning This implementation assumes that the ID are all > 0
 */
struct zwave_functions {
  uint8_t get_func_id;
  uint8_t set_func_id;
};

/**
 * @brief Map of attribute type to resolver functions
 * 
 * If you have declared in your command class :
 * 
 * @code{.cpp}
   attribute_resolver_register_rule(ATTRIBUTE(VALUE),
                                   zwave_command_class_binary_switch_set,
                                   zwave_command_class_binary_switch_get);
 * @endcode
 * 
 * You can bind the functions like this :
 * 
 * @code{.cpp}
    resolver_helper.set_resolver_functions({
      {ATTRIBUTE(VALUE), {SWITCH_BINARY_GET, SWITCH_BINARY_SET}},
    };
 * @endcode
 *
 * This way you'll be able to get the get/set functions with the function id :
 * 
 * @code{.cpp}
   // Call the function zwave_command_class_binary_switch_get
   auto get_func = resolver_helper.get_resolver_function(SWITCH_BINARY_GET);
 * @endcode
 *
 */
using resolver_function_map = std::map<attribute_store_type_t, zwave_functions>;

/**
 * @brief Helper class to manage resolver functions
 * 
 * @note This class isn't meant for you to use directly on your tests. 
 *       Use the top level wrapper zwave_command_class_test_helper instead.
 * 
 * @warning This class can only register Get/Set function that are in the same
 *          Command Class. Otherwise ID will overlap.
 * 
 * This class allows you to get the get or set functions for a given attribute.
 * It allows you to test easily the resolver functions.
 * 
 * First setup your resolver functions bindings :
 * 
 * @code{.cpp}
 * 
 * zwave_resolver_function_helper resolver_helper;
 * 
 * resolver_helper.set_resolver_functions({
 *  {ATTRIBUTE(NUMBER_OF_USERS), {USER_CAPABILITIES_GET, 0}},
 *  {ATTRIBUTE(USER_UNIQUE_ID), {USER_GET, USER_SET}},
 * });
 * @endcode
 * 
 * In attribute_resolver_register_rule_stub callback :
 * 
 * @code{.cpp}
 * sl_status_t
 *  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
 *                                       attribute_resolver_function_t set_func,
 *                                       attribute_resolver_function_t get_func,
 *                                       int cmock_num_calls)
 * {
 *   return resolver_function_helper.register_resolver_functions(node_type,
 *                                                             set_func,
 *                                                             get_func);
 * }
 * @endcode
 * 
 * Then you can get your set/get function with : 
 * 
 * @code{.cpp}
 * auto user_get = resolver_function_helper.get_resolver_function(USER_GET);
 * user_get(current_node, frame, &frame_size); // Will call the get function of the command class
 * 
 * // Will make the test fail since USER_CAPABILITIES_SET is not bound
 * // resolver_function_helper.get_resolver_function(USER_CAPABILITIES_SET);
 * @endcode
 * 
 */
class zwave_resolver_function_helper
{
  public:
  /**
   * @brief Set the resolver functions bindings that will be used to register the resolver functions
   * 
   * @param bindings Functions bindings. See class description to know how to use it
   * 
   * @note This function should be called before calling register_resolver_functions
   * 
   * @see register_resolver_functions
   */
  void set_resolver_functions(const resolver_function_map &bindings);

  /**
 * @brief Register the resolver functions for a given node type
 * 
 * The node_type, set_func and get_func arguments match the ones 
 * in attribute_resolver_register_rule_stub
 * 
 * @note Use the bindings set with set_resolver_functions to register the functions
 * 
 * @param node_type Node type to register the functions
 * @param set_func Function to set the attribute value
 * @param get_func Function to get the attribute value
 * 
 * @return sl_status_t SL_STATUS_OK if the functions were registered successfully
 * 
 */
  sl_status_t
    register_resolver_functions(attribute_store_type_t node_type,
                                attribute_resolver_function_t set_func,
                                attribute_resolver_function_t get_func);

  /**
   * @brief Get the resolver function for a given function id
   * 
   * The test will automatically fails if the binding doesn't exists.
   * 
   * @note This function should be called after calling register_resolver_functions, or it will always fails
   * 
   * @param function_id Function id to get
   * 
   * @return attribute_resolver_function_t& The resolver function
   */
  attribute_resolver_function_t &get_resolver_function(uint8_t function_id);

  private:
  resolver_function_map attributes_binding;
  std::map<uint8_t, attribute_resolver_function_t> resolver_functions;

};  // class zwave_resolver_function_helper
}  // namespace zwave_command_class_test_helper
}  // extern "C"

#endif  // ZWAVE_RESOLVER_FUNCTION_HELPER_HPP
