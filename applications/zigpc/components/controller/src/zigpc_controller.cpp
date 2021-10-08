/*******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

// Shared UIC includes
#include <process.h>
#include <sl_log.h>

// ZigPC includes
#include <zigpc_common_zigbee.h>
#include <zigpc_net_mgmt_notify.h>

#include "zigpc_controller.h"
#include "zigpc_controller_int.hpp"

void zigpc_ctrl_on_device_announced(void *event_data)
{
  if (event_data != nullptr) {
    const zigpc_net_mgmt_on_node_added_t &dev_added
      = *static_cast<zigpc_net_mgmt_on_node_added_t *>(event_data);

    sl_status_t status = zigpc_ctrl::on_device_announced(dev_added.eui64);
    if (status != SL_STATUS_OK) {
      sl_log_warning(zigpc_ctrl::LOG_TAG,
                     "Device announced event handler status: 0x%X",
                     status);
    }
  }
}

void zigpc_ctrl_on_device_interview_update(void *event_data)
{
  if (event_data != nullptr) {
    const zigpc_net_mgmt_on_node_interview_status_t &int_status
      = *static_cast<zigpc_net_mgmt_on_node_interview_status_t *>(event_data);

    if (int_status.success == true) {
      sl_status_t status = zigpc_ctrl::on_device_interviewed(int_status.eui64);
      if (status != SL_STATUS_OK) {
        sl_log_warning(zigpc_ctrl::LOG_TAG,
                       "Device interviewed event handler status: 0x%X",
                       status);
      }
    }
  }
}

// CONTIKI PROCESS

PROCESS(zigpc_ctrl_process, "zigpc_ctrl_process");

PROCESS_THREAD(zigpc_ctrl_process, ev, data)
{
  PROCESS_BEGIN();
  while (1) {
    if (ev == PROCESS_EVENT_EXIT) {
      zigpc_ctrl::on_shutdown();
    }
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END();
}

sl_status_t zigpc_ctrl_fixt_setup(void)
{
  sl_status_t status = zigpc_net_mgmt_register_observer(
    ZIGPC_NET_MGMT_NOTIFY_NODE_INTERVIEW_STATUS,
    zigpc_ctrl_on_device_interview_update);

  if (status == SL_STATUS_OK) {
    status = zigpc_net_mgmt_register_observer(ZIGPC_NET_MGMT_NOTIFY_NODE_ADDED,
                                              zigpc_ctrl_on_device_announced);
  }
  if (status == SL_STATUS_OK) {
    process_start(&zigpc_ctrl_process, NULL);
  }
  return status;
}

int zigpc_ctrl_fixt_shutdown(void)
{
  return 0;
}
