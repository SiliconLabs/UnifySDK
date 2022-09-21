/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
 * @defgroup unify_stdin_attribute_store_command_handler Unfiy Attribute Store Commands
 * @ingroup uic_stdin
 * @brief Set of Attribute Store commands available by default with the stdin console
 *
 * This module provides commands to interfact with the attribute store.
 *
 * @{
 */

#ifndef UNIFY_STDIN_ATTRIBUTE_STORE_COMMAND_HANDLER_H
#define UNIFY_STDIN_ATTRIBUTE_STORE_COMMAND_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Adds Attribute Store commands to the list of default stdin commands.
 */
void unify_stdin_add_attribute_store_commands();

#ifdef __cplusplus
}
#endif

#endif  //UNIFY_STDIN_ATTRIBUTE_STORE_COMMAND_HANDLER_H
/** @} end unify_stdin_attribute_store_command_handler */
