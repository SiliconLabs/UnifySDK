/* # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 */

// Standard library
#include <unistd.h>

// Import command handler class and project libraries
#include "dev_cli_command_handler.hpp"
#include "dev_cli_command_runner.h"

// Process events for a command
#include "dev_cli_run.h"

// UIC component
#include "sys/process.h"
#include "sl_log.h"

// Pre decleration of the main function for command
int dev_cli_help_main();

/**
 * @brief This command will print help and usage for the other commands
 *
 * @ingroup dev_cli_commands Developer CLI commands
 *
 */
CommandTemplate
  DevCliUiHelp("help", "Print commands and usage", "", dev_cli_help_main);

int dev_cli_help_main()
{
  sl_log_set_level(SL_LOG_ERROR);
  print_help();

  // End command process
  process_post(PROCESS_BROADCAST, PROCESS_COMMAND_DONE, 0);
  return 0;
}