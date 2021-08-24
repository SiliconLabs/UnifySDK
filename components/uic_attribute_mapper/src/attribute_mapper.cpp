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
#include "config.h"
#include "attribute_mapper_engine.hpp"
#include "uic_version.h"

extern "C" sl_status_t attribute_mapper_init()
{
  const char *path;
  config_get_as_string("mapdir", &path);

  MapperEngine::get_instance().reset();
  if (MapperEngine::get_instance().load_path(path)) {
    MapperEngine::get_instance().set_ep_type(0x0004);
    return SL_STATUS_OK;
  } else {
    return SL_STATUS_FAIL;
  }
}

extern "C" void attribute_mapper_config_init()
{
  config_add_string("mapdir",
                    "directory for attribute mapping files(*.uam)",
                    DEFAULT_RULES_MAPPING_DIR);
}
