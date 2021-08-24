/* © 2019 Silicon Laboratories Inc. */
/******************************************************************************
 * @file zwave_network_management.c
 * @brief Implementation of zwave_network_management
 *
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#include "zwave_network_management_fixt.h"
#include "nm_state_machine.h"
#include "zwave_network_management_process.h"
#include "sl_status.h"
#include "zwave_controller_callbacks.h"

sl_status_t zwave_network_management_fixt_setup(void)
{
  zwave_controller_register_reset_step(
    &zwave_network_management_set_default,
    ZWAVE_NETWORK_MANAGEMENT_RESET_STEP_PRIORITY);

  nm_state_machine_init();
  process_start(&zwave_network_management_process, NULL);
  return SL_STATUS_OK;
}

int zwave_network_management_fixt_teardown(void)
{
  // FIXME: Is there anything to be done here?
  return 0;
}
