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

#ifndef UIC_STDIN_TEST_H
#define UIC_STDIN_TEST_H

#include "sl_status.h"

/**
 * @brief Run a command manually from the CLI.
 *
 */
sl_status_t uic_stdin_handle_command(const char *command);

#endif
