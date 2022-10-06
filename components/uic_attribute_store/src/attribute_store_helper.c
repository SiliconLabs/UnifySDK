
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

// Includes from this component
#include "attribute_store_helper.h"
#include "attribute_store_type_registration.h"
#include "sl_log.h"

// Generic includes
#include <stdbool.h>
#include <float.h>
#include <math.h>
#include <string.h>  // for memcpy, strncpy, strlen

// Receive buffer for attribute store values (reported / desired)
static uint8_t received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];
static uint8_t received_value_size = 0;

bool attribute_store_is_value_defined(
  attribute_store_node_t node, attribute_store_node_value_state_t value_state)
{
  received_value_size = 0;
  attribute_store_get_node_attribute_value(node,
                                           value_state,
                                           received_value,
                                           &received_value_size);
  if (received_value_size == 0) {
    return false;
  } else {
    return true;
  }
}

bool attribute_store_is_value_matched(attribute_store_node_t node)
{
  uint8_t desired_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {0};
  uint8_t desired_value_size                                  = 0;
  received_value_size                                         = 0;
  attribute_store_get_node_attribute_value(node,
                                           REPORTED_ATTRIBUTE,
                                           received_value,
                                           &received_value_size);
  attribute_store_get_node_attribute_value(node,
                                           DESIRED_ATTRIBUTE,
                                           desired_value,
                                           &desired_value_size);
  if ((received_value_size == desired_value_size)
      && (memcmp(received_value, desired_value, received_value_size) == 0)) {
    return true;
  } else {
    return false;
  }
}

sl_status_t attribute_store_set_desired_as_reported(attribute_store_node_t node)
{
  sl_status_t read_status
    = attribute_store_get_node_attribute_value(node,
                                               REPORTED_ATTRIBUTE,
                                               received_value,
                                               &received_value_size);
  if (read_status == SL_STATUS_OK) {
    return attribute_store_set_node_attribute_value(node,
                                                    DESIRED_ATTRIBUTE,
                                                    received_value,
                                                    received_value_size);
  } else {
    return read_status;
  }
}

sl_status_t attribute_store_set_reported_as_desired(attribute_store_node_t node)
{
  sl_status_t read_status
    = attribute_store_get_node_attribute_value(node,
                                               DESIRED_ATTRIBUTE,
                                               received_value,
                                               &received_value_size);
  if (read_status == SL_STATUS_OK) {
    return attribute_store_set_node_attribute_value(node,
                                                    REPORTED_ATTRIBUTE,
                                                    received_value,
                                                    received_value_size);
  } else {
    return read_status;
  }
}

void attribute_store_undefine_reported(attribute_store_node_t node)
{
  attribute_store_set_node_attribute_value(node, REPORTED_ATTRIBUTE, NULL, 0);
}

sl_status_t attribute_store_set_reported(attribute_store_node_t node,
                                         const void *value,
                                         uint8_t value_size)
{
  return attribute_store_set_node_attribute_value(node,
                                                  REPORTED_ATTRIBUTE,
                                                  value,
                                                  value_size);
}

sl_status_t attribute_store_get_reported(attribute_store_node_t node,
                                         void *value,
                                         size_t expected_size)
{
  return attribute_store_read_value(node,
                                    REPORTED_ATTRIBUTE,
                                    value,
                                    expected_size);
}

/**
 * @brief Sanitizes a char* pointer and copies a C string into the static
 * received_value
 *
 * @param string    C string to be sanitized and copied into the received_value
 *                  buffer.
 * @returns The length of the string stored in received_value
 */
static uint8_t sanitize_string_to_local_buffer(const char *string)
{
  // Sanitize the length of the string, using our local buffer
  strncpy((char *)received_value, string, sizeof(received_value));
  // Ensure NULL termination
  received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH - 1] = '\0';
  // Return the string length
  return (uint8_t)(strlen((char *)received_value) + sizeof(char));
}

sl_status_t attribute_store_set_reported_string(attribute_store_node_t node,
                                                const char *string)
{
  uint8_t string_length = sanitize_string_to_local_buffer(string);
  return attribute_store_set_node_attribute_value(
    node,
    REPORTED_ATTRIBUTE,
    (const uint8_t *)received_value,
    string_length);
}

sl_status_t attribute_store_set_desired_string(attribute_store_node_t node,
                                               const char *string)
{
  uint8_t string_length = sanitize_string_to_local_buffer(string);
  return attribute_store_set_node_attribute_value(
    node,
    DESIRED_ATTRIBUTE,
    (const uint8_t *)received_value,
    string_length);
}

sl_status_t
  attribute_store_concatenate_to_reported_string(attribute_store_node_t node,
                                                 const char *string)
{
  // Read the existing string in our local buffer
  uint8_t string_length = 0;
  attribute_store_get_node_attribute_value(node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)received_value,
                                           &string_length);

  // Previous value was undefined, just set the new string value.
  if (string_length == 0) {
    return attribute_store_set_reported_string(node, string);
  }

  // Now try to append as much as possible from the new string to the old string
  strncat((char *)received_value,
          string,
          sizeof(received_value) - string_length);
  // Ensure NULL termination
  received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH - 1] = '\0';

  // Push it to the attribute store:
  string_length = (uint8_t)(strlen((char *)received_value) + sizeof(char));
  return attribute_store_set_node_attribute_value(
    node,
    REPORTED_ATTRIBUTE,
    (const uint8_t *)received_value,
    string_length);
}

/**
 * @brief Helper function that fetches the Reported or Desired value of a string
 *
 * Note: Does not work with the DESIRED_OR_REPORTED attribute value type.
 *
 * @param node            Attribute Store node to read the value from
 * @param string          Pointer where to store the read value
 * @param maximum_size    Maximum capacity of the pointer.
 * @param value_state     Attribute value state, reported or desired.
 * @return sl_status_t
 */
static sl_status_t
  attribute_store_get_string(attribute_store_node_t node,
                             char *string,
                             size_t maximum_size,
                             attribute_store_node_value_state_t value_state)
{
  // Parameter validation
  if (string == NULL || maximum_size == 0) {
    return SL_STATUS_FAIL;
  }
  // Ensure null termination of the user buffer if we abort:
  string[0] = '\0';
  if (false == attribute_store_is_value_defined(node, value_state)) {
    return SL_STATUS_FAIL;
  }

  // Retrive the data from the attribute store in our buffer.
  uint8_t string_length = 0;
  attribute_store_get_node_attribute_value(node,
                                           value_state,
                                           (uint8_t *)received_value,
                                           &string_length);

  if (string_length > maximum_size) {
    return SL_STATUS_FAIL;
  }

  // The attribute store should contain the null terminations, but who knows
  if (received_value[string_length - 1] != '\0') {
    return SL_STATUS_FAIL;
  }

  // Looks safe, copy the data to the user pointer.
  strncpy(string, (const char *)received_value, maximum_size);
  return SL_STATUS_OK;
}

sl_status_t attribute_store_get_reported_string(attribute_store_node_t node,
                                                char *string,
                                                size_t maximum_size)
{
  return attribute_store_get_string(node,
                                    string,
                                    maximum_size,
                                    REPORTED_ATTRIBUTE);
}

sl_status_t attribute_store_get_desired_string(attribute_store_node_t node,
                                               char *string,
                                               size_t maximum_size)
{
  return attribute_store_get_string(node,
                                    string,
                                    maximum_size,
                                    DESIRED_ATTRIBUTE);
}

sl_status_t attribute_store_get_desired_else_reported_string(
  attribute_store_node_t node, char *string, size_t maximum_size)
{
  if (attribute_store_is_value_defined(node, DESIRED_ATTRIBUTE)) {
    return attribute_store_get_desired_string(node, string, maximum_size);
  } else {
    return attribute_store_get_reported_string(node, string, maximum_size);
  }
}

void attribute_store_undefine_desired(attribute_store_node_t node)
{
  attribute_store_set_node_attribute_value(node, DESIRED_ATTRIBUTE, NULL, 0);
}

sl_status_t attribute_store_set_desired(attribute_store_node_t node,
                                        const void *value,
                                        uint8_t value_size)
{
  return attribute_store_set_node_attribute_value(node,
                                                  DESIRED_ATTRIBUTE,
                                                  value,
                                                  value_size);
}

sl_status_t attribute_store_get_desired(attribute_store_node_t node,
                                        void *value,
                                        size_t expected_size)
{
  return attribute_store_read_value(node,
                                    DESIRED_ATTRIBUTE,
                                    value,
                                    expected_size);
}

sl_status_t attribute_store_get_desired_else_reported(
  attribute_store_node_t node, void *value, size_t expected_size)
{
  return attribute_store_read_value(node,
                                    DESIRED_OR_REPORTED_ATTRIBUTE,
                                    value,
                                    expected_size);
}

sl_status_t
  attribute_store_copy_value(attribute_store_node_t source_node,
                             attribute_store_node_t destination_node,
                             attribute_store_node_value_state_t value_state)
{
  // Read the data from the source node
  sl_status_t read_status
    = attribute_store_get_node_attribute_value(source_node,
                                               value_state,
                                               received_value,
                                               &received_value_size);

  if (read_status != SL_STATUS_OK) {
    return read_status;
  }

  // Attempt to copy the value to the destination node.
  return attribute_store_set_node_attribute_value(destination_node,
                                                  value_state,
                                                  received_value,
                                                  received_value_size);
}

sl_status_t
  attribute_store_read_value(attribute_store_node_t node,
                             attribute_store_node_value_state_t value_state,
                             void *read_value,
                             size_t expected_size)
{
  received_value_size = 0;
  attribute_store_get_node_attribute_value(node,
                                           value_state,
                                           received_value,
                                           &received_value_size);

  // If the value is undefined, we return SL_STATUS_FAIL.
  if (received_value_size == 0) {
    return SL_STATUS_FAIL;
  }

  if (expected_size != received_value_size) {
    return SL_STATUS_FAIL;
  }

  // Copy the received data to the target pointer.
  memcpy(read_value, received_value, expected_size);

  return SL_STATUS_OK;
}

sl_status_t attribute_store_set_child_reported(attribute_store_node_t parent,
                                               attribute_store_type_t type,
                                               const void *value,
                                               uint8_t value_size)
{
  attribute_store_node_t child_node
    = attribute_store_get_first_child_by_type(parent, type);
  if (child_node == ATTRIBUTE_STORE_INVALID_NODE) {
    child_node = attribute_store_add_node(type, parent);
  }
  return attribute_store_set_reported(child_node, value, value_size);
}

sl_status_t attribute_store_set_child_desired(attribute_store_node_t parent,
                                              attribute_store_type_t type,
                                              const void *value,
                                              uint8_t value_size)
{
  attribute_store_node_t child_node
    = attribute_store_get_first_child_by_type(parent, type);
  if (child_node == ATTRIBUTE_STORE_INVALID_NODE) {
    child_node = attribute_store_add_node(type, parent);
  }
  return attribute_store_set_desired(child_node, value, value_size);
}

sl_status_t attribute_store_get_child_reported(attribute_store_node_t parent,
                                               attribute_store_type_t type,
                                               void *value,
                                               size_t expected_size)
{
  return attribute_store_get_reported(
    attribute_store_get_first_child_by_type(parent, type),
    value,
    expected_size);
}

sl_status_t attribute_store_set_child_reported_only_if_missing(
  attribute_store_node_t parent,
  attribute_store_type_t type,
  const void *value,
  uint8_t value_size)
{
  attribute_store_node_t child_node
    = attribute_store_get_first_child_by_type(parent, type);
  if (child_node != ATTRIBUTE_STORE_INVALID_NODE) {
    return SL_STATUS_ALREADY_EXISTS;
  }
  child_node = attribute_store_add_node(type, parent);
  return attribute_store_set_reported(child_node, value, value_size);
}

void attribute_store_walk_tree(attribute_store_node_t top,
                               void (*function)(attribute_store_node_t))
{
  // Apply the function on the current node:
  function(top);
  for (size_t i = 0; i < attribute_store_get_node_child_count(top); i++) {
    attribute_store_walk_tree(attribute_store_get_node_child(top, i), function);
  }
}

void attribute_store_add_if_missing(attribute_store_node_t parent_node,
                                    const attribute_store_type_t attributes[],
                                    uint32_t count)
{
  for (uint32_t i = 0; i < count; i++) {
    // Verify if there is already one value attribute for this endpoint
    attribute_store_node_t value_node
      = attribute_store_get_first_child_by_type(parent_node, attributes[i]);

    // If not, create it. We only need this attribute.
    if (value_node == ATTRIBUTE_STORE_INVALID_NODE) {
      attribute_store_add_node(attributes[i], parent_node);
    }
  }
}

sl_status_t attribute_store_register_callback_by_type_to_array(
  attribute_store_node_changed_callback_t callback_function,
  const attribute_store_type_t types[],
  uint32_t types_count)
{
  sl_status_t status = SL_STATUS_OK;
  for (uint32_t i = 0; i < types_count; i++) {
    status
      |= attribute_store_register_callback_by_type(callback_function, types[i]);
  }
  return status;
}

sl_status_t attribute_store_delete_all_children(attribute_store_node_t node)
{
  attribute_store_node_t child_node = attribute_store_get_node_child(node, 0);

  while (child_node != ATTRIBUTE_STORE_INVALID_NODE) {
    if (SL_STATUS_OK != attribute_store_delete_node(child_node)) {
      return SL_STATUS_FAIL;
    }
    child_node = attribute_store_get_node_child(node, 0);
  }
  return SL_STATUS_OK;
}

/**
 * @brief Helper function that sets the Reported or Desired value of a number
 *
 * Note: Does not work with the DESIRED_OR_REPORTED attribute value type.
 *
 * @param node            Attribute Store node to read the value from
 * @param value           The value to set
 * @param value_state     The value state/type to set (desired or reported)
 * @return sl_status_t
 */
static sl_status_t
  attribute_store_set_number(attribute_store_node_t node,
                             number_t value,
                             attribute_store_node_value_state_t value_state)
{
  // Check if the attribute storage type is known
  attribute_store_storage_type_t storage_type
    = attribute_store_get_storage_type(attribute_store_get_node_type(node));

  switch (storage_type) {
    case U8_STORAGE_TYPE: {
      uint8_t new_value = (uint8_t)round(value);
      return attribute_store_set_node_attribute_value(node,
                                                      value_state,
                                                      &new_value,
                                                      sizeof(new_value));
    }
    case U16_STORAGE_TYPE: {
      uint16_t new_value = (uint16_t)round(value);
      return attribute_store_set_node_attribute_value(node,
                                                      value_state,
                                                      (uint8_t *)&new_value,
                                                      sizeof(new_value));
    }
    case U32_STORAGE_TYPE: {
      uint32_t new_value = (uint32_t)round(value);
      return attribute_store_set_node_attribute_value(node,
                                                      value_state,
                                                      (uint8_t *)&new_value,
                                                      sizeof(new_value));
    }
    case U64_STORAGE_TYPE: {
      uint64_t new_value = (uint64_t)round(value);
      return attribute_store_set_node_attribute_value(node,
                                                      value_state,
                                                      (uint8_t *)&new_value,
                                                      sizeof(new_value));
    }
    case I8_STORAGE_TYPE: {
      int8_t new_value = (int8_t)round(value);
      return attribute_store_set_node_attribute_value(node,
                                                      value_state,
                                                      (uint8_t *)&new_value,
                                                      sizeof(new_value));
    }
    case I16_STORAGE_TYPE: {
      int16_t new_value = (int16_t)round(value);
      return attribute_store_set_node_attribute_value(node,
                                                      value_state,
                                                      (uint8_t *)&new_value,
                                                      sizeof(new_value));
    }
    case UNKNOWN_STORAGE_TYPE:
    case I32_STORAGE_TYPE: {
      int32_t new_value = (int32_t)round(value);
      return attribute_store_set_node_attribute_value(node,
                                                      value_state,
                                                      (uint8_t *)&new_value,
                                                      sizeof(new_value));
    }
    case I64_STORAGE_TYPE: {
      int64_t new_value = (int64_t)round(value);
      return attribute_store_set_node_attribute_value(node,
                                                      value_state,
                                                      (uint8_t *)&new_value,
                                                      sizeof(new_value));
    }
    case FLOAT_STORAGE_TYPE: {
      float new_value = (float)value;
      return attribute_store_set_node_attribute_value(node,
                                                      value_state,
                                                      (uint8_t *)&new_value,
                                                      sizeof(new_value));
    }
    case DOUBLE_STORAGE_TYPE: {
      double new_value = (double)value;
      return attribute_store_set_node_attribute_value(node,
                                                      value_state,
                                                      (uint8_t *)&new_value,
                                                      sizeof(new_value));
    }

    default:
      return SL_STATUS_FAIL;
  }
}

sl_status_t attribute_store_set_reported_number(attribute_store_node_t node,
                                                number_t value)
{
  return attribute_store_set_number(node, value, REPORTED_ATTRIBUTE);
}

sl_status_t attribute_store_set_desired_number(attribute_store_node_t node,
                                               number_t value)
{
  return attribute_store_set_number(node, value, DESIRED_ATTRIBUTE);
}

/**
 * @brief Helper function that sets the Reported or Desired value of a number
 *
 * Note: Does not work with the DESIRED_OR_REPORTED attribute value type.
 *
 * @param node            Attribute Store node to read the value from
 * @param value           The value to set
 * @return sl_status_t
 */
static number_t
  attribute_store_get_number(attribute_store_node_t node,
                             attribute_store_node_value_state_t value_state)
{
  attribute_store_storage_type_t storage_type
    = attribute_store_get_storage_type(attribute_store_get_node_type(node));

  switch (storage_type) {
    case U8_STORAGE_TYPE: {
      uint8_t value;
      return (attribute_store_read_value(node,
                                         value_state,
                                         &value,
                                         sizeof(value))
              == SL_STATUS_OK)
               ? (number_t)value
               : FLT_MIN;
    }
    case U16_STORAGE_TYPE: {
      uint16_t value;
      return (attribute_store_read_value(node,
                                         value_state,
                                         &value,
                                         sizeof(value))
              == SL_STATUS_OK)
               ? (number_t)value
               : FLT_MIN;
    }
    case U32_STORAGE_TYPE: {
      uint32_t value;
      return (attribute_store_read_value(node,
                                         value_state,
                                         &value,
                                         sizeof(value))
              == SL_STATUS_OK)
               ? (number_t)value
               : FLT_MIN;
    }
    case U64_STORAGE_TYPE: {
      uint64_t value;
      return (attribute_store_read_value(node,
                                         value_state,
                                         &value,
                                         sizeof(value))
              == SL_STATUS_OK)
               ? (number_t)value
               : FLT_MIN;
    }
    case I8_STORAGE_TYPE: {
      int8_t value;
      return (attribute_store_read_value(node,
                                         value_state,
                                         &value,
                                         sizeof(value))
              == SL_STATUS_OK)
               ? (number_t)value
               : FLT_MIN;
    }
    case I16_STORAGE_TYPE: {
      int16_t value;
      return (attribute_store_read_value(node,
                                         value_state,
                                         &value,
                                         sizeof(value))
              == SL_STATUS_OK)
               ? (number_t)value
               : FLT_MIN;
    }
    case UNKNOWN_STORAGE_TYPE:
    case I32_STORAGE_TYPE: {
      int32_t value;
      return (attribute_store_read_value(node,
                                         value_state,
                                         &value,
                                         sizeof(value))
              == SL_STATUS_OK)
               ? (number_t)value
               : FLT_MIN;
    }
    case I64_STORAGE_TYPE: {
      int64_t value;
      return (attribute_store_read_value(node,
                                         value_state,
                                         &value,
                                         sizeof(value))
              == SL_STATUS_OK)
               ? (number_t)value
               : FLT_MIN;
    }
    case FLOAT_STORAGE_TYPE: {
      float value;
      return (attribute_store_read_value(node,
                                         value_state,
                                         &value,
                                         sizeof(value))
              == SL_STATUS_OK)
               ? (number_t)value
               : FLT_MIN;
    }
    case DOUBLE_STORAGE_TYPE: {
      double value;
      return (attribute_store_read_value(node,
                                         value_state,
                                         &value,
                                         sizeof(value))
              == SL_STATUS_OK)
               ? (number_t)value
               : FLT_MIN;
    }

    default:
      return FLT_MIN;
  }
}

number_t attribute_store_get_reported_number(attribute_store_node_t node)
{
  return attribute_store_get_number(node, REPORTED_ATTRIBUTE);
}

number_t attribute_store_get_desired_number(attribute_store_node_t node)
{
  return attribute_store_get_number(node, DESIRED_ATTRIBUTE);
}

attribute_store_node_t
  attribute_store_emplace(attribute_store_node_t parent_node,
                          attribute_store_type_t type,
                          const void *value,
                          uint8_t value_size)
{
  attribute_store_node_t child_node
    = attribute_store_get_node_child_by_value(parent_node,
                                              type,
                                              REPORTED_ATTRIBUTE,
                                              value,
                                              value_size,
                                              0);

  if (child_node != ATTRIBUTE_STORE_INVALID_NODE) {
    return child_node;
  }

  child_node = attribute_store_add_node(type, parent_node);
  attribute_store_set_reported(child_node, value, value_size);
  return child_node;
}

attribute_store_node_t
  attribute_store_emplace_desired(attribute_store_node_t parent_node,
                                  attribute_store_type_t type,
                                  const void *value,
                                  uint8_t value_size)
{
  attribute_store_node_t child_node
    = attribute_store_get_node_child_by_value(parent_node,
                                              type,
                                              DESIRED_ATTRIBUTE,
                                              value,
                                              value_size,
                                              0);

  if (child_node != ATTRIBUTE_STORE_INVALID_NODE) {
    return child_node;
  }

  child_node = attribute_store_add_node(type, parent_node);
  attribute_store_set_desired(child_node, value, value_size);
  return child_node;
}