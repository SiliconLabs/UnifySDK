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
#include "process.h"
#include "sl_status.h"
#include "ucl_mqtt.h"
#include "ucl_network_management.h"
#include "ucl_node_state.h"

sl_status_t ucl_mqtt_setup_fixt(void)
{
  // Start the processes
  process_start(&ucl_network_management_process, NULL);
  process_start(&ucl_node_state_process, NULL);

  return SL_STATUS_OK;
}

int ucl_mqtt_teardown(void)
{
  return 0;
}
