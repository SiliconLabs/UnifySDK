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

#include <stdlib.h>
#include <string.h>

/* Contiki includes */
#include "process.h"
#include "sl_log.h"
#include "sl_status.h"

#include "uic_mqtt.h"

#include "zigpc_protctrl_nwmgmt_write.h"
#include "zigpc_protctrl_int.h"
#include "zigpc_protctrl_process.h"
#include "zigpc_protctrl_process_send.h"

static const char LOG_TAG[] = "zigpc_protctrl_process";

/* catch for cases when PROCESS_EVENT* and NM_EV_* collide */
#if (ZIGPC_PROTCTRL_EVENT_MAX_VAL > PROCESS_EVENT_NONE)
#error "ZIGPC_PROTCTRL_EVENT_MAX_VAL is bigger than PROCESS_EVENT_NONE"
#endif

/**
 * @brief Name the of @ref contiki process for Protocol Controller Monitor
 *
 */
PROCESS_NAME(zigpc_protctrl_process);

/* Protocol Controller Globals */
zigpc_protctrl_state_t protctrl_state;

sl_status_t zigpc_protctrl_process_setup(void)
{
  sl_status_t status;

  status = zigpc_protctrl_init_net_mgmt_observers();

  if (status == SL_STATUS_OK) {
    process_start(&zigpc_protctrl_process, NULL);
  }
  return status;
}

int zigpc_protctrl_process_shutdown(void)
{
  return 0;
}

void zigpc_protctrl_process_send_event(
  const enum zigpc_protctrl_process_event event, void *data)
{
  process_post(&zigpc_protctrl_process, event, data);
}

PROCESS(zigpc_protctrl_process, "zigpc_protctrl_process");

PROCESS_THREAD(zigpc_protctrl_process, ev, data)
{
  sl_status_t status;

  PROCESS_BEGIN();
  while (1) {
    if (ev == ZIGPC_PROTCTRL_EVENT_NET_INIT) {
      status = zigpc_protctrl_process_event_net_init(
        (const struct zigpc_net_mgmt_on_network_init *)data);
      sl_log_debug(LOG_TAG, "NET_INIT handler status: 0x%X", status);

    } else if (ev == ZIGPC_PROTCTRL_EVENT_NET_STATE_UPDATE) {
      status = zigpc_protctrl_process_event_net_state_update(
        (const struct zigpc_net_mgmt_on_network_state_update *)data);
      sl_log_debug(LOG_TAG, "NET_STATE_UPDATE handler status: 0x%X", status);
    } else if (ev == ZIGPC_PROTCTRL_EVENT_WRITE_MESSAGE) {
      status = zigpc_protctrl_nwmgmt_write_update((char *)data);
      sl_log_debug(LOG_TAG, "WRITE_MESSAGE handler status: 0x%X", status);

      //must be freed!
      //allocated in zigpc_protctrl_nwmgmt_write.cpp
      if (data != NULL) {
        free(data);
        data = NULL;
      }
    }
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}
