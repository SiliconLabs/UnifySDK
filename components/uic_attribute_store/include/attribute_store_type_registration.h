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

#ifndef ATTRIBUTE_STORE_TYPE_REGISTRATION_H
#define ATTRIBUTE_STORE_TYPE_REGISTRATION_H

// Includes from this component
#include "attribute_store.h"

// Includes from other components
#include "sl_status.h"

/**
 * @defgroup attribute_store_type_registration Attribute Store Type registration
 * @ingroup attribute_store_api
 * @brief Type registration to the Attribute Store
 *
 * This component allows to register type properties to the attribute store.
 *
 * Types can only be registered once, and errors will occur if types are
 * attempted to be registered multiple times.
 *
 * @{
 */

/**
 * @brief Enum for the possible storage types in the attribute store
 */
typedef enum attribute_store_storage_type {
  // The storage type is unknown (or variable) for this attribute.
  // No validation should be performed on read/writes.
  UNKNOWN_STORAGE_TYPE,
  // The storage type is empty, meaning that this attribute never gets a desired
  // or reported value
  EMPTY_STORAGE_TYPE,
  // Unsigned 8-bit integers are used for this attribute
  U8_STORAGE_TYPE,
  // Unsigned 16-bit integers are used for this attribute
  U16_STORAGE_TYPE,
  // Unsigned 32-bit integers are used for this attribute
  U32_STORAGE_TYPE,
  // Unsigned 64-bit integers are used for this attribute
  U64_STORAGE_TYPE,
  // Signed 8-bit integers are used for this attribute
  I8_STORAGE_TYPE,
  // Signed 16-bit integers are used for this attribute
  I16_STORAGE_TYPE,
  // Signed 32-bit integers are used for this attribute
  I32_STORAGE_TYPE,
  // Signed 64-bit integers are used for this attribute
  I64_STORAGE_TYPE,
  // Float are used for this attribute
  FLOAT_STORAGE_TYPE,
  // Double are used for this attribute
  DOUBLE_STORAGE_TYPE,
  // Null terminated string are used for this attribute
  C_STRING_STORAGE_TYPE,
  // Variable length byte arrays are used for this attribute
  BYTE_ARRAY_STORAGE_TYPE,
  // Fixed length structs/byte arrays are used for this attribute
  FIXED_SIZE_STRUCT_STORAGE_TYPE,
  // Value delimiter for the list of avialable storage types.
  INVALID_STORAGE_TYPE
} attribute_store_storage_type_t;

// There is a unit test that should fail if this value is not correct.
#define ENUM_STORAGE_TYPE U32_STORAGE_TYPE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register associated data to an Attribute Store Type
 *
 * @param type           The attribute store type to register
 * @param name           A name to assign to the attribute
 * @param parent_type    The type that the parent should have. Set this to
 *                       ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE to allow
 *                       placing an attribute type under any other attribute type.
 * @param storage_type   The type of storage used for the attribute type.
 *
 * @returns SL_STATUS_OK if registration was successful, SL_STATUS fail in case of error.
 */
sl_status_t
  attribute_store_register_type(attribute_store_type_t type,
                                const char *name,
                                attribute_store_type_t parent_type,
                                attribute_store_storage_type_t storage_type);

/**
 * @brief Returns the registered name of an attribute
 *
 * @param type          The attribute_store_type_t for which the name is requested
 * @return const char*  C string for the type name.
 */
const char *attribute_store_get_type_name(attribute_store_type_t type);

/**
 * @brief Allows to runtime modify the attribute type name
 *
 * @param type          The attribute_store_type_t for which the name must
 *                      be updated. The attribute must be registered prior to
 *                      invoking this function
 * @param name          C string for the type name to assign to the attribute
 *                      type.
 * @returns SL_STATUS_OK if the type name was updated, SL_STATUS_FAIL otherwise.
 */
sl_status_t attribute_store_set_type_name(attribute_store_type_t type,
                                          const char *name);

/**
 * @brief Finds the first attribute store type that is registered with a
 * name
 *
 * @param name          C String for the type name to look for.
 * @returns first type that is registered with the provided name.
 *          ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE if no type is registered with the name
 */
attribute_store_type_t attribute_store_get_type_by_name(const char *name);

/**
 * @brief Returns the name of an attribute store node (looking up its type name registration).
 *
 * @param node        Attribute Store node to look for.
 * @returns name of the attribute type
 */
const char *
  attribute_store_type_get_node_type_name(attribute_store_node_t node);

/** @brief Checks an attribute type is already registered.
 *
 * @param type The attribute_store_type_t to verify if it is registered
 *
 * @returns true if the type is registered, false otherwise
 */
bool attribute_store_is_type_registered(attribute_store_type_t type);

/**
 * @brief Returns the storage type of a given attribute
 *
 * @param type    The attribute_store_type_t for which we want to know the storage
 *                type
 * @returns The registered storage type for a given attribute store type.
 *          returns UNKNOWN_STORAGE_TYPE if the attribute is not registered
 */
attribute_store_storage_type_t
  attribute_store_get_storage_type(attribute_store_type_t type);

/**
 * @brief Finds if there is a mandatory parent type for an attribute type.
 *
 * @param type    The attribute_store_type_t for which we want to know
 *                if the parent must of of a certain type
 * @returns The mandatory type of the parent, if registered.
 *          ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE if no parent type restriction is active
 */
attribute_store_type_t
  attribute_store_get_registered_parent_type(attribute_store_type_t type);

/**
 * @brief Logs the attribute registration type information
 *
 * @param type    The attribute_store_type_t for which we want to know
 *                the registered data
 */
void attribute_store_log_attribute_type_information(
  attribute_store_type_t type);

#ifdef __cplusplus
}
#endif

/** @} end attribute_store_type_registration */
#endif  //ATTRIBUTE_STORE_TYPE_REGISTRATION_H
