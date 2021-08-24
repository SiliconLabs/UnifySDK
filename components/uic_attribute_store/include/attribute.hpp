/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @file attribute.hpp
 * @brief attribute store C++ wrapper.
 * @ingroup attribute_store
 *
 * This file defines a C++ wrapper around the attribute store. Using the wrapper
 * will in general make the attribute store access more read able.
 *
 * @{
 */

#ifndef ATTRIBUTE_HPP
#define ATTRIBUTE_HPP

#ifdef __cplusplus

#include "attribute_store.h"
#include "attribute_store_helper.h"
#include <vector>
#include <stdexcept>
#include <string>
#include <iterator>
namespace attribute_store
{
/**
 * @brief SFINAE class to check is a type is an iterable
 * see https://en.cppreference.com/w/cpp/types/void_t
 * use like this
 * is_iterable<int>::value or is_iterable<vector<int>>::value
 */
template<typename T, typename = void> struct is_iterable : std::false_type {};
template<typename T>
struct is_iterable<T,
                   std::void_t<decltype(std::declval<T>().begin()),
                               decltype(std::declval<T>().end()),
                               decltype(std::declval<T>().push_back(
                                 *(std::declval<T>().begin())))>> :
  std::true_type {};

/**
 * @brief C++ wrapper class for the attribute store
 *
 */
class attribute
{
  public:
  /**
   * @brief Construct a new attribute object
   *
   */
  attribute() {};

  /**
   * @brief Construct a new attribute object
   *
   * @param n node to base this object on
   */
  attribute(attribute_store_node_t n) : _n(n) {};

  /**
   * @ref attribute_store_get_node_parent
   */
  attribute parent() const
  {
    return attribute_store_get_node_parent(_n);
  }

  inline bool get(attribute_store_node_value_state_t value_type,
                  uint8_t *buffer,
                  uint8_t *size) const
  {
    return (SL_STATUS_OK
            == attribute_store_get_node_attribute_value(_n,
                                                        value_type,
                                                        buffer,
                                                        size));
  }

  /**
   * @ref attribute_store_get_first_parent_with_type
   */
  attribute first_parent(attribute_store_type_t type)
  {
    return attribute_store_get_first_parent_with_type(_n, type);
  }

  /**
   * @brief Get attribute store value.
   *
   * Gets the desired or reported value of the attribute store node formatted as
   * a specific data type. This function will throw an exception if the node has
   * no data or has a value length that is not consistent with the given data
   * type. If if the attribute node is invalid.
   *
   * @tparam T Data type to read into
   * @param value_type  Desired of reported value
   * @return T Read value
   */

  template<typename T>
  inline T get(attribute_store_node_value_state_t value_type) const
  {
    uint8_t buffer[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];
    uint8_t size = 0;
    if (get(value_type, (uint8_t *)buffer, &size)) {
      // For iterable containers we copy the data
      if constexpr (is_iterable<T>::value) {
        typedef typename T::value_type v_t;
        //Make sure that value type of the conrainter int,float,bool ... etc
        static_assert(std::is_trivially_copyable<v_t>::value);
        T v;
        v_t *start = reinterpret_cast<v_t *>(buffer);
        v_t *end   = reinterpret_cast<v_t *>(buffer + size);
        std::copy(start, end, std::back_insert_iterator(v));
        return v;
      } else if constexpr (std::is_integral<T>::value) {
        if (size == 1) {
          return *(reinterpret_cast<uint8_t *>(buffer));
        } else if (size == 2) {
          return *(reinterpret_cast<uint16_t *>(buffer));
        }
      } else {
        static_assert(std::is_trivially_copyable<T>::value);
      }

      if (size == sizeof(T)) {
        return *(reinterpret_cast<T *>(buffer));
      } else {
        throw std::invalid_argument(
          "Unexpected attribute size for node " + std::to_string(_n)
          + ".Expected: " + std::to_string(sizeof(T))
          + ", actual: " + std::to_string(size)
          + " value state: " + std::to_string(value_type) + ".");
      }
    } else {
      throw std::invalid_argument("Unable to read attribute "
                                  + std::to_string(_n));
    }
  }

  inline attribute &set(attribute_store_node_value_state_t value_type,
                        const uint8_t *value,
                        size_t size)
  {
    if (SL_STATUS_OK
        != attribute_store_set_node_attribute_value(_n,
                                                    value_type,
                                                    value,
                                                    size)) {
      throw std::invalid_argument("Unable to write attribute "
                                  + std::to_string(_n));
    } else {
      return *this;
    }
  }

  /**
   * @brief Set attribute store value.
   *
   * Sets the desired or reported value of the attribute store node formatted as
   * a specific data type. This function will overwrite the previous stored
   * value. If the attribute node is invalid an exception will be thrown.
   *
   * @tparam T Data type to write
   * @param value_type  Desired of reported value
   * @param value Value to write
   */

  template<typename T>
  inline attribute &set(attribute_store_node_value_state_t value_type, T value)
  {
    if constexpr (is_iterable<T>::value) {
      typedef typename T::value_type v_t;
      //Make sure that value type of the conrainter int,float,bool ... etc
      static_assert(std::is_trivially_copyable<v_t>::value);

      //The purpose of this is to convert the iterable container into a linear buffer
      std::vector<v_t> v;
      std::copy(value.begin(), value.end(), std::back_insert_iterator(v));
      return set(value_type,
                 reinterpret_cast<uint8_t *>(v.data()),
                 v.size() * sizeof(v_t));
    } else {
      static_assert(std::is_trivially_copyable<T>::value);
      return set(value_type, reinterpret_cast<uint8_t *>(&value), sizeof(T));
    }
  }

  inline void clear(attribute_store_node_value_state_t value_type)
  {
    if (SL_STATUS_OK
        != attribute_store_set_node_attribute_value(_n, value_type, 0, 0)) {
      throw std::invalid_argument("Unable to write attribute");
    }
  }

  /**
   * @brief Helper to read the repported value
   */
  template<typename T> T reported() const
  {
    return get<T>(REPORTED_ATTRIBUTE);
  }
  /**
   * @brief Helper to read the desired value
   */
  template<typename T> T desired() const
  {
    return get<T>(DESIRED_ATTRIBUTE);
  }

  /**
   * @brief Returns true if this attribute has a desired value
   */
  bool desired_exists()
  {
    return attribute_store_is_value_defined(_n, DESIRED_ATTRIBUTE);
  }

  /**
   * @brief Returns true if this attribute has a reported value
   */

  bool reported_exists()
  {
    return attribute_store_is_value_defined(_n, REPORTED_ATTRIBUTE);
  }

  template<typename T> T desired_or_reported()
  {
    return get<T>(DESIRED_OR_REPORTED_ATTRIBUTE);
  }

  void clear_reported()
  {
    clear(REPORTED_ATTRIBUTE);
  }

  void clear_desired()
  {
    clear(DESIRED_ATTRIBUTE);
  }

  /**
   * @brief Helper to write the repported value
   */
  template<typename T> attribute &set_reported(T v)
  {
    return set<T>(REPORTED_ATTRIBUTE, v);
  }

  /**
   * @brief Helper to write the desired value
   */
  template<typename T> attribute &set_desired(T v)
  {
    return set<T>(DESIRED_ATTRIBUTE, v);
  }

  /**
   * @brief @ref attribute_store_get_node_child_count
   *
   * @return size_t Number of children
   */
  size_t child_count() const
  {
    return attribute_store_get_node_child_count(_n);
  }

  /**
   * @brief @ref attribute_store_get_node_child
   *
   * @param index child to get
   * @return attribute
   */
  attribute child(size_t index) const
  {
    return attribute_store_get_node_child(_n, index);
  }

  /**
   * @brief @ref attribute_store_get_node_child_by_type
   *
   * @param type
   * @param index
   * @return attribute
   */
  attribute child_by_type(attribute_store_type_t type, size_t index = 0) const
  {
    return attribute_store_get_node_child_by_type(_n, type, index);
  }

  /**
   * @brief Returns a vector of children of this attribute
   *
   * @return std::vector<attribute>
   */
  std::vector<attribute> children() const
  {
    std::vector<attribute> _children;
    size_t n_childs = child_count();
    for (size_t i = 0; i < n_childs; i++) {
      _children.push_back(child(i));
    }
    return _children;
  }

  /**
   * @brief Get child by value
   *
   * Get a child by the reppored value of that child. If no matching child is
   * found an invalid node is returned.
   *
   * @tparam T Type of value
   * @param type Desired of repported
   * @param value The value to search for
   * @return attribute
   */
  template<typename T>
  attribute child_by_type_and_value(attribute_store_type_t type, T value) const
  {
    return attribute_store_get_node_child_by_value(
      _n,
      type,
      REPORTED_ATTRIBUTE,
      reinterpret_cast<uint8_t *>(&value),
      sizeof(T),
      0);
  }

  /**
   * @brief Checks if this node is valid
   *
   * @return true
   * @return false
   */
  bool is_valid() const
  {
    return _n != ATTRIBUTE_STORE_INVALID_NODE;
  }

  /**
   * @brief Get the node attribute type
   *
   * @return attribute_store_type_t
   */
  attribute_store_type_t type() const
  {
    return attribute_store_get_node_type(_n);
  }

  /**
   * @brief Get the root of the attribute tree.
   *
   * @return attribute
   */
  static attribute root()
  {
    return attribute_store_get_root();
  }

  /**
   * @brief @ref attribute_store_add_node
   */
  attribute add_node(attribute_store_type_t type)
  {
    return attribute_store_add_node(type, _n);
  }

  /**
   * @brief Get or add a child of a given attribtue type and value.
   * 
   * This function will check if this nodes as a child  of a given type and value. 
   * If a child exists the child is returned, if it does not exist the child is 
   * created and the repported value is set.
   */
  template<typename T>
  attribute emplace_node(attribute_store_type_t type, T val)
  {
    attribute a = child_by_type_and_value(type, val);
    if (a.is_valid()) {
      return a;
    } else {
      return add_node(type).set_reported(val);
    }
  }

  /**
   * @brief Get or add a child of a given attribtue type.
   * 
   * This function will check if this nodes as a child  of a given type. 
   * If a child exists the child is returned, if it does not exist the child is 
   * created.
   */
  attribute emplace_node(attribute_store_type_t type)
  {
    attribute a = child_by_type(type);
    if (a.is_valid()) {
      return a;
    } else {
      return add_node(type);
    }
  }

  /**
   * @brief Delete this node from the attribute tree. This node will invalid after calling this.
   *
   */
  void delete_node()
  {
    attribute_store_delete_node(_n);
    _n = ATTRIBUTE_STORE_INVALID_NODE;
  }

  //type conversion back to attribute_store_node_t
  operator attribute_store_node_t() const
  {
    return _n;
  }

  //Comparison operators
  bool operator==(const attribute &a) const
  {
    return _n == a._n;
  }
  bool operator==(attribute_store_node_t a) const
  {
    return _n == a;
  }

  private:
  attribute_store_node_t _n = ATTRIBUTE_STORE_INVALID_NODE;
};

}  // namespace attribute_store

#endif
#endif  //ATTRIBUTE_HPP
/** @} end attribute */
