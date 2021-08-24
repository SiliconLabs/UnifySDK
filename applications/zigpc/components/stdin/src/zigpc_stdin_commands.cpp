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

// UIC components
#include "sl_status.h"
#include "sl_log.h"
#include "uic_stdin.hpp"

// ZigPC components
#include "zigpc_gateway.h"

#include "zigpc_stdin_fixt.h"

static sl_status_t handle_print_emberaf_info(const handle_args_t &arg)
{
  zigpc_gateway_command_print_info();

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
  {"info",
   {" :Execute 'info' command accessible from the EmberAf CLI",
    &handle_print_emberaf_info}},
};

// Adding zpc commands to the UIC CLI
sl_status_t zigpc_stdin_setup_fixt(void)
{
  uic_stdin_add_commands(commands);
  uic_stdin_set_prompt_string("ZigPC>");

  return SL_STATUS_OK;
}
