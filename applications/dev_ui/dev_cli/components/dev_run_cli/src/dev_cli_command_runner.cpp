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

// Own header file
#include <dev_cli_command_runner.h>

// Libraries
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <iostream>

// Project libraries
#include "dev_cli_command_handler.hpp"

// UIC Component
#include "sl_log.h"
#include "sys/process.h"

#define LOG_TAG "dev_cli_command_runner"

std::vector<CommandTemplate *> CommandTemplate::commands_list;

bool run_command(const char *command_input)
{
  sl_log_debug(LOG_TAG, "Command input: %s\n", command_input);

  std::string command = command_input;
  int success         = false;

  bool command_executed = false;
  for (auto it = CommandTemplate::commands_list.begin();
       it != CommandTemplate::commands_list.end();
       ++it) {
    CommandTemplate *command_iter = (*it);

    if (command.compare(command_iter->command) == 0) {
      success = command_iter->command_to_execute();
      if (success == 0) {
        command_executed = true;
      }
    }
  }

  if (!command_executed) {
    printf("Wrong command argument, use -h or --help as argument to see a list "
           "of available commands\n");
    return false;
  }

  return true;
}

bool print_help()
{
  printf("Commands:\n");
  for (auto it = CommandTemplate::commands_list.begin();
       it != CommandTemplate::commands_list.end();
       ++it) {
    CommandTemplate *command_iter = (*it);

    printf("\t%s %s - %s\n",
           command_iter->command.c_str(),
           command_iter->arguments_help.c_str(),
           command_iter->info_help.c_str());
  }

  return true;
}

int return_number_of_commands()
{
  return (int)CommandTemplate::commands_list.size();
}