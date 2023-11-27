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

#include "uic_stdin_command_handler.h"
#include "uic_stdin.hpp"
#include "uic_stdin_test.h"

// Unify Components
#include "sl_log.h"
#include "attribute_store.h"

// Contiki includes
#include "process.h"

// STDOUT file descriptor initalize
#include <unistd.h>

// C++ stdlib includes
#include <cstdio>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <map>
#include <string>
#include <vector>
#include <stdexcept>

/// File descripter for output stream
static int out_stream;

// Unify handler functions
static sl_status_t handle_help(const handle_args_t &arg);
static sl_status_t handle_exit(const handle_args_t &arg);
static sl_status_t handle_set_log_level(const handle_args_t &arg);

/// Map that holds all the commands (used for printing help)
///
/// Entries are {<command>, {<help message>, <handler_func>}},
/// <command>: The command written on the command interface (e.g. "help")
/// <help_message>: Help message printed to output when "help" is executed
/// <handler_func>: callback, that is called whenever the command is receied
static command_map_t commands
  = {{"help", {" :Prints help", handle_help}},
     {"exit", {" :Exit the application", handle_exit}},
     {"log_level",
      {COLOR_START "d|i|n|w|e|c" COLOR_END " :Set log level - "
                   "DEBUG | INFO | NOTICE | WARNING | ERROR | CRITICAL",
       handle_set_log_level}}};

// The prompt string for the command line interface
std::string prompt_string = "Unify> ";

// Public  functions
void uic_stdin_add_commands(
  const std::map<std::string, std::pair<std::string, handler_func>>
    &append_commands)
{
  commands.insert(append_commands.begin(), append_commands.end());
}

void uic_stdin_set_prompt_string(const std::string &des_prompt_string)
{
  prompt_string = des_prompt_string;
}

int uic_stdin_get_output_fd()
{
  return out_stream;
}

// Component functions
const char *get_prompt_string()
{
  return prompt_string.c_str();
}

void uic_stdin_init()
{
  out_stream = STDOUT_FILENO;
}

// Private Handler functions implementation
static sl_status_t handle_help(const handle_args_t &arg)
{
  dprintf(out_stream, "==================================================\n");
  dprintf(out_stream,
          COLOR_START "Unify Command line interface Help:\n" COLOR_END);
  dprintf(out_stream, "==================================================\n");
  for (auto elem: commands) {
    dprintf(out_stream,
            COLOR_START "%s" COLOR_END " %s\n",
            elem.first.c_str(),
            elem.second.first.c_str());
  }
  dprintf(out_stream, "==================================================\n");
  dprintf(out_stream,
          "Tip: log_level c to reduce the"
          " verbosity of log\n");
  dprintf(out_stream, "==================================================\n");
  return SL_STATUS_OK;
}

static sl_status_t handle_exit(const handle_args_t &arg)
{
  process_post(PROCESS_BROADCAST, PROCESS_EVENT_EXIT, 0);
  return SL_STATUS_OK;
}

static sl_status_t handle_set_log_level(const handle_args_t &arg)
{
  // Get log level from string
  auto get_log_level = [](const std::string &level) {
    if (level == "d") {
      return SL_LOG_DEBUG;
    } else if (level == "i") {
      return SL_LOG_INFO;
    } else if (level == "n") {
      return SL_LOG_NOTICE;
    } else if (level == "w") {
      return SL_LOG_WARNING;
    } else if (level == "e") {
      return SL_LOG_ERROR;
    } else if (level == "c") {
      return SL_LOG_CRITICAL;
    } else {
      throw std::invalid_argument("Invalid argument, expected d,i,n,w,e, or c");
    }
  };
  try {
    if (arg.size() == 2) {
      sl_log_set_level(get_log_level(arg[1]));
      dprintf(out_stream, "Setting log level to: %s\n", arg[1].c_str());
      return SL_STATUS_OK;
    } else if (arg.size() == 3) {
      if (arg[2] == "off") {
        sl_log_unset_tag_level(arg[1].c_str());
        dprintf(out_stream,
                "Unsetting log level for tag: %s\n",
                arg[1].c_str());
      } else {
        sl_log_set_tag_level(arg[1].c_str(), get_log_level(arg[2]));
        dprintf(out_stream,
                "Setting log level for tag: %s to %s\n",
                arg[1].c_str(),
                arg[2].c_str());
      }
      return SL_STATUS_OK;
    } else {
      dprintf(
        out_stream,
        "Invalid number of arguments, expected arg: <level> or <tag>,"
        "<level>\n to disable a tag specific log level use %s <tag>,off\n",
        arg[0].c_str());

      return SL_STATUS_FAIL;
    }
  } catch (const std::invalid_argument &e) {
    dprintf(out_stream, "%s\n", e.what());
    return SL_STATUS_FAIL;
  }
}

// Framework functions
sl_status_t uic_stdin_handle_command(const char *command)
{
  std::string command_str(command);
  std::string cmd(command_str.substr(0, command_str.find(' ')));
  auto iter = commands.find(cmd);
  if (iter != commands.end()) {
    handle_args_t args;
    // add <command> as first arg
    args.push_back(cmd);
    // If there is a space after the command, look for "," seperated args
    // e.g. <command> <arg1>,<arg2>,<arg3>
    size_t pos = 0;
    if ((pos = command_str.find(' ')) != std::string::npos) {
      // erase "<command> " from the command_str.
      command_str.erase(0, pos + 1);
      const std::string delimiter(",");
      // look for "," seperated args in command_str
      while ((pos = command_str.find(delimiter)) != std::string::npos) {
        args.push_back(command_str.substr(0, pos));
        // erase <arg> from command_str
        command_str.erase(0, pos + delimiter.length());
      }
      // add last argument as well
      args.push_back(command_str);
    }
    return iter->second.second(args);
  }

  return SL_STATUS_FAIL;
}

char *uic_stdin_command_generator(const char *text, int state)
{
  static int list_index;
  static int len;

  if (!state) {
    list_index = 0;
    len        = strlen(text);
  }
  int k = 0;
  for (auto command: commands) {
    const std::string &cmd = command.first;
    if ((k++) < list_index) {
      continue;
    }
    list_index++;
    if (cmd.rfind(text, len) == 0) {
      return strdup(cmd.data());
    }
  }
  return NULL;
}
