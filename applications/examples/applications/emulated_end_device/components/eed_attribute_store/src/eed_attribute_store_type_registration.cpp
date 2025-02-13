/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "eed_attribute_store_attribute_types.h"
#include "eed_attribute_store_type_registration.h"

// Unify components
#include "attribute_store_type_registration.h"
#include <vector>

// defines
#define UNSIGNED_LONG_STORAGE_TYPE                                \
  ((sizeof(unsigned long) == sizeof(uint32_t)) ? U32_STORAGE_TYPE \
                                               : U64_STORAGE_TYPE)
typedef struct {
  attribute_store_type_t type;
  const char *name;
  attribute_store_type_t parent_type;
  attribute_store_storage_type_t storage_type;
} attribute_schema_t;

// clang-format off
static const std::vector<attribute_schema_t> attribute_schema = {
  {ATTRIBUTE_TREE_ROOT,   "Root node",   ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,   EMPTY_STORAGE_TYPE},

  // Generic Application attributes
  {ATTRIBUTE_UNID, "UNID", ATTRIBUTE_TREE_ROOT, C_STRING_STORAGE_TYPE},
  {ATTRIBUTE_ENDPOINT_ID, "Endpoint ID", ATTRIBUTE_UNID, U8_STORAGE_TYPE},
};
// clang-format on

// This main function will serve as a registration bookkeeper for all our
// Z-Wave attributes
sl_status_t eed_attribute_store_register_known_attribute_types()
{
  sl_status_t status = SL_STATUS_OK;
  for (auto const &a: attribute_schema) {
    status |= attribute_store_register_type(a.type,
                                            a.name,
                                            a.parent_type,
                                            a.storage_type);
  }
  return status;
}
