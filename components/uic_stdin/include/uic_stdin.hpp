/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

/**
 * @file uic_stdin.hpp
 * @defgroup uic_stdin Unify Stdin
 * @ingroup uic_components
 * @brief Unify Stdin interface for other applications
 *
 * Functions enabling adding commands to the CLI and integrating
 * applications speific command prompt.
 *
 * @{
 */

#ifndef UIC_STDIN_H
#define UIC_STDIN_H

// C++ stdlib
#include <map>
#include <vector>
#include <string>
/**
 * @brief Arguments type
 *
 */
typedef std::vector<std::string> handle_args_t;

/**
 * @brief Handler function callback type
 *
 * @param arg Arguments of a command
 */
typedef sl_status_t (*handler_func)(const handle_args_t &arg);

/**
 * @brief Add commands to command line interface
 * This function is used at compile time to add commands to command
 * line interface.
 *
 * @param append_commands Dictionary with a command name, command usage
 * and a function pointer to command functionality
 */
void uic_stdin_add_commands(
  const std::map<std::string, std::pair<std::string, handler_func>>
    &append_commands);

/**
 * @brief Set prompt string for command line interface
 * This function sets the prompt string for the CLI.
 *
 * @param des_prompt_string The desired prompt string
 */
void uic_stdin_set_prompt_string(const std::string &des_prompt_string);

/**
 * @brief Get stdout file number
 *
 */
int uic_stdin_get_output_fd();

/**
 * @brief Run a command manually from CLI
 *
 */
sl_status_t uic_stdin_handle_command(const char *command);

#endif /** @} */
//UIC_STDIN_HPP
