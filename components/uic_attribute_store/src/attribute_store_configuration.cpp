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
#include "attribute_store_configuration.h"
#include "attribute_store_configuration_internal.h"
#include "attribute_store_internal.h"

#include "sl_log.h"
constexpr char LOG_TAG[] = "attribute_store_configuration";

// Private variables.
namespace
{
// Should the Attribute Store save to the datastore after each modification?
// It was enabled when first released, so it is enabled by default.
bool auto_save_enabled = true;
// Should the Attribute Store perform additional validation when values
// Based on the registered type informat?
bool type_valiation_enabled = false;
}  // namespace

///////////////////////////////////////////////////////////////////////////////
// Public functions, declared in attribute_store_configuration.h
///////////////////////////////////////////////////////////////////////////////
void attribute_store_configuration_set_type_validation(bool enabled)
{
  type_valiation_enabled = enabled;
  sl_log_info(LOG_TAG,
              "Attribute Store type validation: %s",
              type_valiation_enabled ? "Enabled" : "Disabled");
}

void attribute_store_configuration_set_auto_save(bool enabled)
{
  if ((auto_save_enabled == false) && (enabled == true)) {
    // Save everything we missed until the auto-save got enabled:
    attribute_store_save_to_datastore();
  }
  auto_save_enabled = enabled;
  sl_log_info(LOG_TAG,
              "Attribute Store auto-save: %s",
              (auto_save_enabled == true) ? "Enabled" : "Disabled");
}

///////////////////////////////////////////////////////////////////////////////
// Internal functions, declared in attribute_store_configuration_internal.h
///////////////////////////////////////////////////////////////////////////////
bool attribute_store_is_type_validation_enabled()
{
  return type_valiation_enabled;
}

bool attribute_store_is_auto_save_enabled()
{
  return auto_save_enabled;
}
