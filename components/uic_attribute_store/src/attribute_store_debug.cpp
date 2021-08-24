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
#include "attribute_store_debug.h"

#include <map>
#include <string>
#include <string.h>
static std::map<std::string, attribute_store_type_t> name_map;
static std::map<attribute_store_type_t, const char *> type_map;

void attribute_store_register_attribute(attribute_store_type_t id,
                                        const char *name)
{
  name_map.insert(std::make_pair(name, id));
  type_map.insert(std::make_pair(id, name));
}

attribute_store_type_t attribute_store_type_by_name(const char *name)
{
  if (name_map.count(name)) {
    return name_map.at(name);
  } else {
    return 0xFFFFFFFF;
  }
}

const char *attribute_store_name_by_type(attribute_store_type_t t)
{
  static char *name_buf[16];
  if (type_map.count(t)) {
    return type_map.at(t);
  } else {
    snprintf((char *)name_buf, sizeof(name_buf), "0x%08x", t);
    return (const char *)&name_buf[0];
  }
}