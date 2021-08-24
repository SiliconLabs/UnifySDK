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

#ifndef DEV_CLI_COMMAND_RUNNER
#define DEV_CLI_COMMAND_RUNNER

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
 * @brief Run a command based on an input string, which should be
 * a valid command.
 *
 * @ingroup dev_cli_run Developer CLI command runner and manager
 * @param command_input User-specific command used as a pointer
 * to char array \ingroup DEV CLI
 * @returns Boolean for successful command run
 */
bool run_command(const char *command_input);

/**
 * @brief Print all commands so users can see which commands are available.
 *
 * @ingroup dev_cli_run Developer CLI command runner and manager
 * @returns Boolean for successful printing help
 */
bool print_help();

/**
 * @brief Return the number of initiated command classes.
 * @ingroup dev_cli_run Developer CLI command runner and manager
 * @returns Integer number of commands
 */
int return_number_of_commands();

#ifdef __cplusplus
}
#endif
#endif
