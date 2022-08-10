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
#define ATTRIBUTE_TYPE_MAP

#include "attribute_store_type_registration.h"
#include "attribute_store.h"
#include <vector>

extern "C" sl_status_t aoxpc_attribute_store_attr_type_name_helper_init(void)
{
  const std::vector<std::pair<attribute_store_type_t, const char *>> attributes
    = {
#include "aoxpc_attribute_store_defined_attribute_types.h"
    };

  for (auto &a: attributes) {
    attribute_store_register_type(a.first,
                                  a.second,
                                  ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
                                  UNKNOWN_STORAGE_TYPE);
  }

  return SL_STATUS_OK;
}
