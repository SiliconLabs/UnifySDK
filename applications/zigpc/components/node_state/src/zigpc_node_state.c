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

/* Contiki includes */
#include "sl_status.h"

#include "zigpc_node_state.h"
#include "zigpc_node_state_int.h"

sl_status_t zigpc_node_state_setup(void)
{
  sl_status_t status = zigpc_node_state_init_observers();

  return status;
}

int zigpc_node_state_shutdown(void)
{
  return 0;
}
