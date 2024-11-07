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
 * @file attribute.hpp
 * @brief attribute store C++ wrapper.
 * @ingroup attribute_store
 *
 * This file defines a C++ wrapper around the attribute store. Using the wrapper
 * will in general make the attribute store access more readable.
 *
 * @{
 */

#ifndef ATTRIBUTE_HPP
#define ATTRIBUTE_HPP

#ifdef __cplusplus

#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_type_registration.h"

#include <vector>
#include <stdexcept>
#include <string>
#include <iterator>
#include <functional>
#include <cstring>
#include <sstream>  // for ostringstream
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
   * Get the first parent of a give type. If this node match the type
   * this node is returned.
   * @ref attribute_store_get_first_parent_with_type
   */
  inline attribute
    first_parent_or_self(attribute_store_type_t parent_type) const
  {
    if (type() == parent_type) {
      return *this;
    } else {
      return first_parent(parent_type);
    }
  }

  /**
   * @ref attribute_store_get_first_parent_with_type
   */
  attribute first_parent(attribute_store_type_t type) const
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
      if constexpr (std::is_same<T, std::string>::value) {
        return std::string(reinterpret_cast<char *>(buffer));
      } else if constexpr (
        is_iterable<T>::value) {  // For iterable containers we copy the data
        typedef typename T::value_type v_t;
        //Make sure that value type of the container int,float,bool ... etc
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
          "Unexpected attribute size for node " + name_and_id() + ".Expected: "
          + std::to_string(sizeof(T)) + ", actual: " + std::to_string(size)
          + " value state: " + std::to_string(value_type) + ".");
      }
    } else {
      throw std::invalid_argument("Unable to read attribute " + name_and_id());
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
      throw std::invalid_argument("Unable to write attribute " + name_and_id());
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
   * @param value_type  Desired or reported value
   * @param value Value to write
   */

  template<typename T> inline attribute &
    set(attribute_store_node_value_state_t value_type, const T &value)
  {
    if constexpr (std::is_same<T, char const *>::value) {
      // We are storing strings as C Strings with the null termination character in the attribute store.
      return set(value_type, std::string(value));
    } else if constexpr (std::is_same<T, std::string>::value) {
      // We are storing strings as C Strings with the null termination character in the attribute store.
      return set(value_type,
                 reinterpret_cast<const uint8_t *>(value.c_str()),
                 value.size() + 1);
    } else if constexpr (is_iterable<T>::value) {
      typedef typename T::value_type v_t;
      //Make sure that value type of the container int,float,bool ... etc
      static_assert(std::is_trivially_copyable<v_t>::value);

      //The purpose of this is to convert the iterable container into a linear buffer
      std::vector<v_t> v;
      std::copy(value.begin(), value.end(), std::back_insert_iterator(v));
      return set(value_type,
                 reinterpret_cast<const uint8_t *>(v.data()),
                 v.size() * sizeof(v_t));
    } else {
      static_assert(std::is_trivially_copyable<T>::value);
      return set(value_type,
                 reinterpret_cast<const uint8_t *>(&value),
                 sizeof(T));
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
   * @brief Helper to read the reported value
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

  bool exists(attribute_store_node_value_state_t state) const
  {
    return attribute_store_is_value_defined(_n, state);
  }

  /**
   * @brief Returns true if this attribute has a desired value
   */
  bool desired_exists() const
  {
    return exists(DESIRED_ATTRIBUTE);
  }

  /**
   * @brief Returns true if this attribute has a reported value
   */

  bool reported_exists() const
  {
    return exists(REPORTED_ATTRIBUTE);
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
   * @brief Helper to write the reported value
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
   * @brief Returns a vector of children of this attribute with a given type.
   *
   * @param type        Attribute Store Type.
   *
   * @return std::vector<attribute>
   */
  std::vector<attribute> children(attribute_store_type_t type) const
  {
    std::vector<attribute> _children;
    for (size_t i = 0; i < child_count(); i++) {
      if (child(i).type() == type) {
        _children.push_back(child(i));
      }
    }
    return _children;
  }

  /**
   * @brief Get child by value (reported value)
   *
   * Get a child by the reported value of that child. If no matching child is
   * found an invalid node is returned.
   *
   * @tparam T Type of value
   * @param type Attribute store type
   * @param value The value to search for
   * @return attribute
   */
  template<typename T>
  attribute child_by_type_and_value(attribute_store_type_t type, T value) const
  {
    for (auto c: children(type)) {
      if (c.reported_exists() && c.reported<T>() == value) {
        return c;
      }
    }
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  /**
   * @brief Get child by value and state
   *
   * Get a child by the given state of that child. If no matching child is
   * found an invalid node is returned.
   *
   * @tparam T Type of value
   * @param type Attribute store type
   * @param value The value to search for
   * @param value_type The value state
   * @return attribute
   */
  template<typename T> attribute
    child_by_type_and_value(attribute_store_type_t type,
                            T value,
                            attribute_store_node_value_state_t value_type) const
  {
    for (auto c: children(type)) {
      if (c.exists(value_type) && c.get<T>(value_type) == value) {
        return c;
      }
    }
    return ATTRIBUTE_STORE_INVALID_NODE;
  }

  /**
   * @brief Checks if this node is valid
   *
   * @return true
   * @return false
   */
  bool is_valid() const
  {
    return attribute_store_node_exists(_n);
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
   * @brief Get or add a child of a given attribute type and value.
   *
   * This function will check if this nodes as a child  of a given type and value.
   * If a child exists the child is returned, if it does not exist the child is
   * created and the reported value is set.
   */
  template<typename T>
  attribute emplace_node(attribute_store_type_t type,
                         T val,
                         attribute_store_node_value_state_t value_state
                         = REPORTED_ATTRIBUTE)
  {
    attribute a = child_by_type_and_value(type, val, value_state);
    if (a.is_valid()) {
      return a;
    } else {
      return add_node(type).set(value_state, val);
    }
  }

  /**
   * @brief Get or add a child of a given attribute type.
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

  sl_status_t change_parent(attribute new_parent)
  {
    return attribute_store_change_parent(this->_n, new_parent);
  }

  /**
   * @brief Visit all nodes in the tree.
   *
   * This function visits all tree nodes going from the top and down. It will
   * continue to the node children if the status returned by the function is
   * SL_STATUS_OK. If the function returns SL_STATUS_ABORT, no more nodes are
   * visited. Otherwise, siblings will be visited.
   *
   * @param func function to execute
   * @param depth The current depth of the top node.
   */
  inline sl_status_t visit(
    const std::function<sl_status_t(attribute_store::attribute &, int)> &func,
    int depth = 0)
  {
    sl_status_t status = func(*this, depth);
    if (SL_STATUS_OK == status) {
      for (auto c: children()) {
        status = c.visit(func, depth + 1);
        if (status == SL_STATUS_ABORT) {
          return status;
        }
      }
    }
    return status;
  }

  std::string name() const
  {
    return attribute_store_get_type_name(this->type());
  }

  std::string name_and_id() const
  {
    std::ostringstream out;
    out << name() << " (Node " << _n << ")";
    return out.str();
  }


  std::string value_to_string() const
  {
    
    std::string str_value;
    try {
      switch (attribute_store_get_storage_type(this->type())) {
        case UNKNOWN_STORAGE_TYPE:
        case EMPTY_STORAGE_TYPE:
        case FIXED_SIZE_STRUCT_STORAGE_TYPE:
        case INVALID_STORAGE_TYPE:
          return "Not displayable value for " + name_and_id();
        case U8_STORAGE_TYPE:
        case U16_STORAGE_TYPE:
        case U32_STORAGE_TYPE:
        case U64_STORAGE_TYPE:
          str_value = _get_str_value<uint64_t>();
          break;
        case I8_STORAGE_TYPE:
        case I16_STORAGE_TYPE:
        case I32_STORAGE_TYPE:
        case I64_STORAGE_TYPE:
          str_value = _get_str_value<int64_t>();
          break;
        case FLOAT_STORAGE_TYPE:
        case DOUBLE_STORAGE_TYPE:
          str_value = _get_str_value<double>();
          break;
        case C_STRING_STORAGE_TYPE:
          str_value = _get_str_value<std::string>();
          break;
        case BYTE_ARRAY_STORAGE_TYPE:
          str_value = _get_str_value<std::vector<uint8_t>>();
          break;
        default:
          return "Unkown value type for " + name_and_id();
      }
    } catch (...) {
      return "Can't get values of " + name_and_id();
    }

    return name_and_id() + " values : " + str_value;
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

  template<typename T>
  std::string _get_str_value() const {
    auto desired_value = _get_str_value_with_state<T>(DESIRED_ATTRIBUTE);
    auto reported_value = _get_str_value_with_state<T>(REPORTED_ATTRIBUTE);

    return reported_value + " (" +  desired_value + ")";
  }

  template<typename T>
  std::string _get_str_value_with_state(attribute_store_node_value_state_t state) const {
    if (!exists(state)) {
      return "<>";
    }
    
    std::string str_value = "";

    if constexpr (std::is_same<T, std::vector<uint8_t>>::value) {
      str_value = "[";
      for (uint8_t byte: get<std::vector<uint8_t>>(state)) {
        str_value += std::to_string(byte) + ",";
      }
      str_value.pop_back();
      str_value += "]";
    } else if constexpr (std::is_same<T, int64_t>::value
                         || std::is_same<T, uint64_t>::value
                         || std::is_same<T, double>::value) {
      str_value = std::to_string(get<T>(state));
    } else if constexpr (std::is_same<T, std::string>::value) {
      str_value = get<std::string>(state);
    }

    return str_value;
  }
};

}  // namespace attribute_store

#endif
#endif  //ATTRIBUTE_HPP
/** @} end attribute */
