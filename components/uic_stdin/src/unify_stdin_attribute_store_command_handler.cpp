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
#include "uic_stdin.hpp"
#include "unify_stdin_attribute_store_command_handler.h"

// Unify components
#include "attribute_store.h"
#include "attribute_store_configuration.h"
#include "sl_log.h"

#include <stdexcept>

///////////////////////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////////////////////
static sl_status_t
  handle_attribute_store_configuration_set_auto_save_cooldown_interval(
    const handle_args_t &arg);

static sl_status_t
  handle_attribute_store_configuration_set_auto_save_safety_interval(
    const handle_args_t &arg);

static sl_status_t handle_attribute_store_configuration_set_type_validation(
  const handle_args_t &arg);

///////////////////////////////////////////////////////////////////////////////
// Additional Attribute Store commands map
///////////////////////////////////////////////////////////////////////////////
const command_map_t attribute_store_commands = {
  {"attribute_store_configuration_set_auto_save_cooldown_interval",
   {COLOR_START "<Interval>" COLOR_END
                " :Configures how long the Attribute Store waits after an "
                "update before saving to the datastore. Use the value 0 to "
                "save everything sequentially, any other "
                "value for delay in seconds to wait after attributes updates",
    &handle_attribute_store_configuration_set_auto_save_cooldown_interval}},
  {"attribute_store_configuration_set_auto_save_safety_interval",
   {COLOR_START "<Interval>" COLOR_END
                " :Configures how long the Attribute Store waits in the wost "
                "case scenario before it saves to the datastore.",
    &handle_attribute_store_configuration_set_auto_save_safety_interval}},
  {"attribute_store_configuration_set_type_validation",
   {COLOR_START
    "<Enabled>" COLOR_END
    " :Configures if the Attribute Store type write validation is enabled."
    "Use the value 0 for disabled, 1 for enabled",
    &handle_attribute_store_configuration_set_type_validation}},
};

///////////////////////////////////////////////////////////////////////////////
// Command handling functions
///////////////////////////////////////////////////////////////////////////////
static sl_status_t
  handle_attribute_store_configuration_set_auto_save_cooldown_interval(
    const handle_args_t &arg)
{
  try {
    if (arg.size() == 2) {
      // Parse the Auto-save value argument
      int auto_save_interval = std::stoi(arg[1].c_str(), nullptr, 10);
      attribute_store_configuration_set_auto_save_cooldown_interval(
        auto_save_interval);
      return SL_STATUS_OK;
    }
  } catch (const std::invalid_argument &e) {
    dprintf(uic_stdin_get_output_fd(), "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_FAIL;
}

static sl_status_t
  handle_attribute_store_configuration_set_auto_save_safety_interval(
    const handle_args_t &arg)
{
  try {
    if (arg.size() == 2) {
      // Parse the Auto-save value argument
      int auto_save_interval = std::stoi(arg[1].c_str(), nullptr, 10);
      attribute_store_configuration_set_auto_save_safety_interval(
        auto_save_interval);
      return SL_STATUS_OK;
    }
  } catch (const std::invalid_argument &e) {
    dprintf(uic_stdin_get_output_fd(), "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_FAIL;
}

static sl_status_t handle_attribute_store_configuration_set_type_validation(
  const handle_args_t &arg)
{
  try {
    if (arg.size() == 2) {
      // Parse the "Enabled" argument
      bool enabled = static_cast<bool>(std::stoi(arg[1].c_str(), nullptr, 10));
      attribute_store_configuration_set_type_validation(enabled);
      return SL_STATUS_OK;
    }
  } catch (const std::invalid_argument &e) {
    dprintf(uic_stdin_get_output_fd(), "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_FAIL;
}

///////////////////////////////////////////////////////////////////////////////
// Functions shared within the component
///////////////////////////////////////////////////////////////////////////////
void unify_stdin_add_attribute_store_commands()
{
  uic_stdin_add_commands(attribute_store_commands);
}