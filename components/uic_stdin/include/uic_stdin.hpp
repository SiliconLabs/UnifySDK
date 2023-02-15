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

/**
 * @file uic_stdin.hpp
 * @defgroup uic_stdin Unify Stdin
 * @ingroup unify_components
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

// Unify definitions
#include "sl_status.h"

// Other component includes
#include "attribute_store.h"

/**
 * @brief Arguments type
 */
using handle_args_t = std::vector<std::string>;

/**
 * @brief Handler function callback type
 *
 * @param arg Arguments of a command
 */
using handler_func = sl_status_t (*)(const handle_args_t &arg);

/**
 * @brief Command map object, used to register a set of functions to handle
 * incoming commands
 */
using command_map_t
  = std::map<std::string, std::pair<std::string, handler_func>>;

/**
 * @brief Add commands to command line interface
 * This function is used at compile time to add commands to command
 * line interface.
 *
 * @param append_commands Dictionary with a command name, command usage
 * and a function pointer to command functionality
 */
void uic_stdin_add_commands(const command_map_t &append_commands);

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

/**
 * @brief Pretty-print a tree of nodes in attribute store. Show nodes' ID, data
 * type and value in a similar fashion to tree command.
 *
 * @param node A root node to print its attributes and children.
 * @param print_total_child_count Print the number of children for each node.
 * @param print_only_n_node Limit the number of nodes printed.
 */
sl_status_t unify_stdin_attribute_store_log_node(attribute_store_node_t node,
                                                 bool print_total_child_count = false,
                                                 bool print_only_n_node       = false);

#endif /** @} */
//UIC_STDIN_HPP
