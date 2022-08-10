/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "attribute_store_validation.h"
#include "attribute_store_type_registration.h"
#include "attribute_store_configuration_internal.h"

#include "sl_log.h"
constexpr char LOG_TAG[] = "attribute_store_validation";

bool is_write_operation_valid(attribute_store_type_t node_type,
                              const uint8_t *value,
                              uint8_t value_size)
{
  if (attribute_store_is_type_validation_enabled() == false) {
    return true;
  }

  // It's always allowed to undefine values:
  if (value_size == 0) {
    return true;
  }

  attribute_store_storage_type_t storage_type
    = attribute_store_get_storage_type(node_type);

  switch (storage_type) {
    case U8_STORAGE_TYPE:
    case I8_STORAGE_TYPE:
      return (value_size == sizeof(int8_t));

    case U16_STORAGE_TYPE:
    case I16_STORAGE_TYPE:
      return (value_size == sizeof(int16_t));

    case U32_STORAGE_TYPE:
    case I32_STORAGE_TYPE:
      return (value_size == sizeof(int32_t));

    case U64_STORAGE_TYPE:
    case I64_STORAGE_TYPE:
      return (value_size == sizeof(int64_t));

    case FLOAT_STORAGE_TYPE:
      return (value_size == sizeof(float));

    case DOUBLE_STORAGE_TYPE:
      return (value_size == sizeof(double));

    case C_STRING_STORAGE_TYPE:
      return (value[value_size - 1] == '\0');

    case EMPTY_STORAGE_TYPE:
      return (value_size == 0);

    default:
      return true;
  }
}

bool is_node_addition_valid(attribute_store_type_t node_type,
                            attribute_store_type_t parent_type)
{
  if (attribute_store_is_type_validation_enabled() == false) {
    return true;
  }

  attribute_store_type_t mandatory_parent_type
    = attribute_store_get_registered_parent_type(node_type);
  if ((mandatory_parent_type != ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE)
      && (parent_type != mandatory_parent_type)) {
    sl_log_error(LOG_TAG,
                 "Attempt to create an Attribute Store node of type "
                 "0x%X under a parent type 0x%X. Attribute Store registration "
                 "mandates that the parent type must be 0x%X.",
                 node_type,
                 parent_type,
                 mandatory_parent_type);
    attribute_store_log_attribute_type_information(node_type);
    return false;
  }
  return true;
}
