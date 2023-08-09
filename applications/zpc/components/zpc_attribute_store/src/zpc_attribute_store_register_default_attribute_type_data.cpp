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
#include "zpc_attribute_store_register_default_attribute_type_data.h"

// Use the attribute type map from the define types.
#define ATTRIBUTE_TYPE_MAP
#include "ZW_classcmd.h"

// Unify components
#include "attribute_store_type_registration.h"
#include "attribute_store.h"

// Generic includes
#include <vector>

sl_status_t zpc_attribute_store_register_default_attribute_type_data()
{
  const std::vector<std::pair<attribute_store_type_t, const char *>> attributes
    = {
#include "zpc_attribute_store_all_ids.h"
    };

  sl_status_t status = SL_STATUS_OK;

  for (const auto &a: attributes) {
    if (false == attribute_store_is_type_registered(a.first)) {
      status
        |= attribute_store_register_type(a.first,
                                         a.second,
                                         ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
                                         UNKNOWN_STORAGE_TYPE);
    }
  }

  return status;
}
