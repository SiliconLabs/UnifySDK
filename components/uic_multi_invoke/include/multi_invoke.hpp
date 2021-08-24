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

/**
 * @file multi_invoke.hpp
 * @ingroup uic_components
 * @brief Template for multiple invoke callback maps
 *
 * Template that be used for handling callback maps, where each key may contain
 * multiple callback functions.
 *
 * Example of Usage:
 * void my_cb(int arg) {
 *   std::cout << arg << std:endl;
 * }
 * void my_2nd_cb(int arg) {
 *   std::cout << arg << std:endl;
 * }
 *
 * void my_init() {
 *   multi_invoke<int, int> my_multi_invoke;
 *   my_multi_invoke.add(1, my_cb);
 *   my_multi_invoke.add(1, my_2nd_cb);
 *   my_multi_invoke(1, 42); // Will call my_cb and my_2nd_cb with "42" as arg
 *   my_multi_invoke.erase(1, my_cb); // remove callback to my_cb under key 1
 * }
 *
 * @{
 */

#ifndef MULTI_INVOKE_HPP
#define MULTI_INVOKE_HPP
#ifdef __cplusplus
#include <map>
#include <vector>
/**
 * @brief Class that supports handling multiple callback functions to a key.
 *
 * @tparam key Type for the key
 * @tparam function_args Arguments to the callback function
 */
template<typename key, typename... function_args> class multi_invoke
{
  public:
  typedef void (*callback_type_t)(function_args...);
  /**
   * @brief Call all callback functions registered under k
   *
   * @param k Key to call all callbacks for
   * @param args Argument to callback functions
   */
  void operator()(key k, function_args... args) const
  {
    std::vector<callback_type_t> funcs;
    // find all entries that match the key
    auto range = callbacks.equal_range(k);
    // to protect against the case where the callback modifies the list
    // cache the result
    for (auto i = range.first; i != range.second; i++) {
      funcs.push_back((*i).second);
    }
    for (auto cb: funcs) {
      cb(args...);
    }
  }

  /**
   * @brief Add an entry to multi_invoke
   *
   * @param k Key to add
   * @param cb Callback function to add to Key
   */
  void add(key k, callback_type_t cb)
  {
    // Prevent inserting same node/callback pair several times
    auto range = callbacks.equal_range(k);
    for (auto it = range.first; it != range.second; ++it) {
      if (it->second == cb) {
        return;
      }
    }

    callbacks.insert(std::make_pair(k, cb));
  }

  /**
   * @brief Remove an entry
   *
   * @param k Key of which to remove cb
   * @param cb Callback function to remove from Key
   */
  void remove(key k, callback_type_t cb)
  {
    auto range = callbacks.equal_range(k);
    for (auto it = range.first; it != range.second; ++it) {
      if (it->second == cb) {
        callbacks.erase(it);
        break;
      }
    }
  }

  /**
   * @brief Check if a key is contained within the multi_invoke
   *
   * @param k Key to check if multi_invoke contains
   * @return true Key exists
   * @return false Key doesn't exist
   */
  bool contains(key k) const
  {
    return callbacks.count(k) > 0;
  }

  /**
   * @brief Erase all callbacks under a given key
   *
   * @param k Key to erase all callbacks under
   */
  void erase(key k)
  {
    callbacks.erase(k);
  }

  /**
   * @brief Clear all entries in multi_invoke
   */
  void clear()
  {
    callbacks.clear();
  }

  /**
   * @brief Verify if the list of callbacks is empty
   * @returns true if empty, false otherwise
   */
  bool empty() const
  {
    return callbacks.empty();
  }

  private:
  std::multimap<key, callback_type_t> callbacks;
};

#endif  // __cplusplus

#endif  //MULTI_INVOKE_HPP
/** @} end multi_invoke */
