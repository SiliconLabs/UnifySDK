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

#include <map>
#include <string>

// Unify components
#include "sl_status.h"
#include "sl_log.h"
#include "uic_stdin.hpp"

// ZigPC components
#include "zigpc_common_unid.h"
#include "zigpc_common_zigbee.h"
#include "zigpc_datastore.h"
#include "zigpc_gateway.h"

#include "zigpc_stdin_fixt.h"

constexpr char LOG_TAG[] = "zigpc_stdin";

static sl_status_t handle_print_emberaf_info(const handle_args_t &arg)
{
  zigpc_gateway_command_print_info();

  return SL_STATUS_OK;
}

static sl_status_t handle_ds_log_device(const handle_args_t &arg)
{
  sl_status_t status = SL_STATUS_OK;

  for (size_t i = 1; i < arg.size(); i++) {
    zigbee_eui64_t eui64;
    const std::string &unid = arg[i];
    if (unid.size() < (ZIGBEE_EUI64_SIZE * 2)) {
      sl_log_warning(LOG_TAG, "Invalid UNID format.");
      continue;
    }
    status = str_to_zigbee_eui64(unid.c_str(), unid.size(), eui64);

    if (status == SL_STATUS_OK) {
      zigpc_datastore_log_device_disc_state(eui64);
    }
  }
  return status;
}

static sl_status_t handle_print_emberaf_nwk_key(const handle_args_t &arg)
{
  zigpc_gateway_command_print_nwk_key();

  return SL_STATUS_OK;
}

/**
 * @brief Map that holds all the commands (used for printing help)
 *
 * Entries are {<command>, {<help message>, <handler_func>}},
 * <command>: The command written on the command interface (e.g. "help")
 * <help_message>: Help message printed to output when "help" is executed
 * <handler_func>: callback, that is called whenever the command is receied
 */
const std::map<std::string, std::pair<std::string, handler_func>> commands = {
  {"datastore_log_device",
   {"<unid> :Log persisted device discovery data based on device UNID",
    &handle_ds_log_device}},
  {"info", {" :Alias for 'zigbee_embercli_info", &handle_print_emberaf_info}},
  {"zigbee_embercli_info",
   {" :Execute EmberCLI 'info' command that prints information about the "
    "Zigbee gateway device network state, clusters, and endpoints",
    &handle_print_emberaf_info}},
  {"zigbee_log_nwk_key",
   {" :Log current Zigbee Network key", &handle_print_emberaf_nwk_key}},
};

// Adding zpc commands to the Unify CLI
sl_status_t zigpc_stdin_setup_fixt(void)
{
  uic_stdin_add_commands(commands);
  uic_stdin_set_prompt_string("ZigPC>");

  return SL_STATUS_OK;
}
