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
#include "attribute_store_process.h"
#include "attribute_store_internal.h"

#include "sl_log.h"
constexpr char LOG_TAG[] = "attribute_store_configuration";

// Private variables.
namespace
{
// How long should elapse in the worst case before the Attribute Store saves to
// the SQLite storage in. Unit is seconds
// Default will back-up the database every minute.
unsigned int auto_save_safety_interval = 60;
// How long should elapse since last write in the attribute store before we back
// up in the datastore. Unit is seconds
unsigned int auto_save_cooldown_interval = 10;
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

void attribute_store_configuration_set_auto_save_safety_interval(
  unsigned int seconds)
{
  sl_log_info(LOG_TAG,
              "Updating the Attribute Store auto-save safety interval "
              "from %d seconds to %d seconds.",
              auto_save_safety_interval,
              seconds);
  auto_save_safety_interval = seconds;
  attribute_store_process_restart_auto_save_safety_timer();
}

void attribute_store_configuration_set_auto_save_cooldown_interval(
  unsigned int seconds)
{
  sl_log_info(LOG_TAG,
              "Updating the Attribute Store auto-save cooldown interval "
              "from %d seconds to %d seconds.",
              auto_save_cooldown_interval,
              seconds);
  auto_save_cooldown_interval = seconds;
}

///////////////////////////////////////////////////////////////////////////////
// Internal functions, declared in attribute_store_configuration_internal.h
///////////////////////////////////////////////////////////////////////////////
bool attribute_store_is_type_validation_enabled()
{
  return type_valiation_enabled;
}

unsigned int attribute_store_get_auto_save_safety_interval()
{
  return auto_save_safety_interval;
}

unsigned int attribute_store_get_auto_save_cooldown_interval()
{
  return auto_save_cooldown_interval;
}
