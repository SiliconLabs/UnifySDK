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

#include "attribute_store_debug.h"
#include "ZW_classcmd.h"

#include "attribute_store.h"
#include <vector>

extern "C" void zpc_attribute_store_debug_init()
{
  const std::vector<std::pair<attribute_store_type_t, const char *>> attributes
    = {
#include "zpc_attribute_store_all_ids.h"
    };

  for (auto &a: attributes) {
    attribute_store_register_attribute(a.first, a.second);
  }
}
