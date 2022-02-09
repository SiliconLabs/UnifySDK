/* # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 */

/**
 * @file uic_stdin_command_handler.h
 * @brief Command handler and generator
 *
 * This component implements default commands like exit, help and log_level.
 * Also it has the basic functionalities of handling these commands and possibility
 * to add more commands to the existing CLI.
 *
 * @{
 */

#ifndef UIC_STDIN_COMMAND_HANDLER_H
#define UIC_STDIN_COMMAND_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sl_status.h"
#include <stdint.h>

/**
 * @brief Handle a command e.g. from stdio
 *
 * @param command Zero terminated string with the command, e.g. "help"
 */
sl_status_t uic_stdin_handle_command(const char *command);

/**
 * @brief Command generator for editline
 *
 * This function will be called several times when we are trying to
 * autocomplete a word. See man rl_completion_matches
 * @param text   See man rl_completion_matches
 * @param state  See man rl_completion_matches
 * @return char* See man rl_completion_matches
 */
char *uic_stdin_command_generator(const char *text, int state);

/**
 * @brief Get the prompt string for CLI.
 *
 */
const char *get_prompt_string();

/**
 * @brief Initialize uic stdin/stdout
 *
 */
void uic_stdin_init();

#ifdef __cplusplus
}
#endif

/** @} */
#endif  // UIC_STDIN_COMMAND_HANDLER_H
