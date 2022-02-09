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
#include "zwave_s2_fixt.h"
#include "process.h"
#include "zwave_s2_process.h"
#include "sl_status.h"

sl_status_t zwave_s2_fixt_setup(void)
{
  process_start(&zwave_s2_process, NULL);
  return SL_STATUS_OK;
}
