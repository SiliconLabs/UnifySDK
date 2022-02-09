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
 * @file zwave_api_demo_commands.h
 * @ingroup ZWAVE_API_DEMO
 * @brief Parses an input command and calls the corresponding command
 * if it exists
 *
 * @{
 */

#ifndef ZWAVE_API_DEMO_COMMANDS_H
#define ZWAVE_API_DEMO_COMMANDS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief read the input command and calls the corresponding available command
 */
void process_command(char *input_command);

#ifdef __cplusplus
}
#endif

#endif  //ZWAVE_API_DEMO_COMMANDS_H
/** @} end ZWAVE_API_DEMO */
