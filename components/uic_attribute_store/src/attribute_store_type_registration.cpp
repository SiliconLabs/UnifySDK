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
#include "attribute_store_type_registration.h"
#include "attribute_store_type_registration_internal.h"
#include "attribute_store.h"

// Generic includes
#include <map>
#include <string>
#include <stdexcept>

// Includes from other components
#include "sl_status.h"
#include "sl_log.h"

// Struct in which we save the attribute store data.
using attribute_type_information_t = struct {
  // Name assigned to the attribute store type.
  std::string type_name;
  // Type of the parent of this attribute type
  attribute_store_type_t parent_type;
  // Storage type associated with this attribute type.
  attribute_store_storage_type_t storage_type;
};

// Private variables: Map between type IDs and their text descriptions (KeyHandles)
namespace
{
std::map<attribute_store_type_t, attribute_type_information_t>
  attribute_information_map;
}

constexpr char LOG_TAG[] = "attribute_store_type_registration";

///////////////////////////////////////////////////////////////////////////////
// Function shared within the component
///////////////////////////////////////////////////////////////////////////////
sl_status_t attribute_store_reset_registered_attribute_types()
{
  attribute_information_map.clear();
  return SL_STATUS_OK;
}

///////////////////////////////////////////////////////////////////////////////
// Public interface functions, declared in attribute_store_type_registration.h
///////////////////////////////////////////////////////////////////////////////
sl_status_t
  attribute_store_register_type(attribute_store_type_t type,
                                const char *name,
                                attribute_store_type_t parent_type,
                                attribute_store_storage_type_t storage_type)
{
  // if such type ID has been already registered - return an error
  if (attribute_information_map.count(type) != 0) {
    sl_log_error(LOG_TAG,
                 "Error: attribute type [0x%08X:'%s'] is already registered.",
                 type,
                 name);
    return SL_STATUS_FAIL;
  }

  // Check the parent type:
  if (parent_type == type) {
    sl_log_error(LOG_TAG,
                 "Cannot register an attribute type 0x%08X as its own parent. "
                 "Please use ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE for the "
                 "parent type in this case.",
                 type);
    return SL_STATUS_FAIL;
  }

  // Check the storage type:
  if (storage_type >= INVALID_STORAGE_TYPE) {
    sl_log_error(LOG_TAG,
                 "Invalid storage type when attempting to register attribute "
                 "type (0x%08X). Please use one of the defined values.",
                 type);
    return SL_STATUS_FAIL;
  }

  attribute_type_information_t info = {.type_name    = std::string(name),
                                       .parent_type  = parent_type,
                                       .storage_type = storage_type};
  attribute_information_map.insert({type, info});
  return SL_STATUS_OK;
}

const char *attribute_store_get_type_name(attribute_store_type_t type)
{
  try {
    return attribute_information_map.at(type).type_name.c_str();
  } catch (const std::out_of_range &e) {
    //FIXME: It's ugly here.
    static char type_name[100] = {'\0'};
    uint16_t index
      = snprintf(type_name, sizeof(type_name), "Unknown 0x%08X", type);
    type_name[index] = '\0';
    return type_name;
  }
}

bool attribute_store_is_type_registered(const attribute_store_type_t type)
{
  return (attribute_information_map.count(type) != 0);
}

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
                                          const char *name)
{
  if (false == attribute_store_is_type_registered(type)) {
    sl_log_warning(LOG_TAG,
                   "Attempt to modify the name of an unregistered "
                   "Attribute Type (0x%08X). Ignoring",
                   type);
    return SL_STATUS_FAIL;
  }

  attribute_information_map[type].type_name = std::string(name);
  return SL_STATUS_OK;
}

attribute_store_type_t attribute_store_get_type_by_name(const char *name)
{
  std::string name_to_find = std::string(name);

  for (auto it = attribute_information_map.begin();
       it != attribute_information_map.end();
       ++it) {
    if (it->second.type_name == name_to_find) {
      return it->first;
    }
  }

  return ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE;
}

const char *attribute_store_type_get_node_type_name(attribute_store_node_t node)
{
  return attribute_store_get_type_name(attribute_store_get_node_type(node));
}

attribute_store_storage_type_t
  attribute_store_get_storage_type(attribute_store_type_t type)
{
  if (false == attribute_store_is_type_registered(type)) {
    return UNKNOWN_STORAGE_TYPE;
  }

  return attribute_information_map[type].storage_type;
}

attribute_store_type_t
  attribute_store_get_registered_parent_type(attribute_store_type_t type)
{
  if (false == attribute_store_is_type_registered(type)) {
    return ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE;
  }

  return attribute_information_map[type].parent_type;
}

void attribute_store_log_attribute_type_information(attribute_store_type_t type)
{
  if (false == attribute_store_is_type_registered(type)) {
    sl_log_debug(LOG_TAG,
                 "Type 0x%X has no registration data. \n- Name = \"\""
                 "\n- Parent Type = 0,\n- Storage Type = Unknown",
                 type);
    return;
  }

  sl_log_debug(LOG_TAG,
               "Type 0x%X registration data: \n- Name = \"%s\""
               "\n- Parent Type = 0x%X,\n- Storage Type = %d",
               type,
               attribute_information_map[type].type_name.c_str(),
               attribute_information_map[type].parent_type,
               attribute_information_map[type].storage_type);
}