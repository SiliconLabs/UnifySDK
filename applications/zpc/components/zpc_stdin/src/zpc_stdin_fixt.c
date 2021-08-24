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

// Component includes
#include "zpc_stdin_fixt.h"
#include "zpc_stdin_command_handling.h"

sl_status_t zpc_stdin_setup_fixt(void)
{
  zpc_setup_cli();
  return SL_STATUS_OK;
}
