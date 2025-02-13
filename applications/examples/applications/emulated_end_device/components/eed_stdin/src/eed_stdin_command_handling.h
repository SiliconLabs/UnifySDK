/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef EED_STDIN_COMMAND_HANDLING_H
#define EED_STDIN_COMMAND_HANDLING_H

#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Adding command line interface commands to Unify process
 *
 */
void eed_setup_cli();

#ifdef __cplusplus
}
#endif
#endif  //EED_STDIN_COMMAND_HANDLING_H
