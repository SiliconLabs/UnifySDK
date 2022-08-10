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
#include "network_monitor_fixt.h"
#include "network_monitor.h"
#include "smart_start_list_monitor.h"
#include "failing_node_monitor.h"

sl_status_t network_monitor_setup_fixt(void)
{
  network_state_monitor_init();
  process_start(&network_monitor_process, NULL);
  smart_start_list_monitor_init();
  failing_node_monitor_init();
  return SL_STATUS_OK;
}
