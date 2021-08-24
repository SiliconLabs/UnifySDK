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

#ifndef DEV_CLI_COMMAND_HANDLER
#define DEV_CLI_COMMAND_HANDLER

// standard libraries
#include <iostream>
#include <vector>
#include <algorithm>

/**
 * @brief This class is a generic class for implementing a command into the
 * command client interface tool. The class has a void function
 * command_to_execute which should be populated by each command src file, thus
 * the command handler knows for each object which function to execute when a
 * specific command is requested in the CLI.
 * @ingroup dev_cli_run Developer CLI command runner and manager
 *
 * @param command_name the command name for executing specific command in CLI
 * @param info helping information for using the command
 * @param arg_help usage for aguments help
 * @param input_command_ptr a pointer pointing to a specific function which is
 * going to execute a command main function \ingroup DEV CLI
 */
class CommandTemplate
{
  private:
  int (*command_ptr)();

  public:
  // Constructor appending to the static commands_list
  CommandTemplate(std::string command_name,
                  std::string info,
                  std::string arg_help,
                  int (*input_command_ptr)())
  {
    command   = command_name;
    info_help = info;
    commands_list.emplace_back(this);
    command_ptr    = input_command_ptr;
    arguments_help = arg_help;
  };
  // Destructor removing all the commands_list class memebers
  ~CommandTemplate()
  {
    auto p = std::remove(commands_list.begin(), commands_list.end(), this);
    commands_list.erase(p, commands_list.end());
  };

  // All command classes saved in this static variable which is consistent
  // across all initiated classes
  static std::vector<CommandTemplate *> commands_list;

  // Generic function which will be executed from main program and depending on
  // the command it will change.
  int command_to_execute();

  // Command input
  std::string command;
  // Help for commando
  std::string info_help;
  // Arguments description and requirements
  std::string arguments_help;
};

#endif