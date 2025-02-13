/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef ATTRIBUTE_CALLBACKS_HPP
#define ATTRIBUTE_CALLBACKS_HPP

#include "sl_status.h"
#include "attribute_store.h"

#include <functional>

// Your code here
namespace attribute_store {

using node_changed_callback
  = std::function<void(attribute_store_node_t, attribute_store_change_t)>;


/**
 * @brief Register a callback function to any node with a given type.
 *
 * @note We can't check uniqueness of the callback function, so it's up to the user
 * to make sure that the same callback function is not registered multiple times.
 * 
 * @param callback_function The function to invoke when the node's
 *                          value is updated.
 * @param type              The attribute node type for which a
 *                          callback will be registered.
 */
void register_callback_by_type(node_changed_callback callback_function,
                               attribute_store_type_t type);

/**
 * @brief Register a callback function to any node with a given type and value state.
 * 
 * @note We can't check uniqueness of the callback function, so it's up to the user
 * to make sure that the same callback function is not registered multiple times.
 * 
 * @param callback_function The function to invoke when the node's
 *                         value is updated.  
 * @param type             The attribute node type for which a
 *                        callback will be registered.
 * @param value_state      The value state for which a callback will be registered.
 * 
 */
void register_callback_by_type_and_state(
  node_changed_callback callback_function,
  attribute_store_type_t type,
  attribute_store_node_value_state_t value_state);

} // namespace attribute_store
#endif // ATTRIBUTE_CALLBACKS_HPP