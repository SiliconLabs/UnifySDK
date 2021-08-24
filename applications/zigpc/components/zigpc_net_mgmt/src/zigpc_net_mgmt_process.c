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

#include "zigpc_net_mgmt_process.h"
#include "zigpc_net_mgmt_process_send.h"
#include "zigpc_net_mgmt_gateway.h"
#include "zigpc_net_mgmt_notify_int.h"
#include "zigpc_net_mgmt_fsm.h"

#include "zigpc_gateway.h"

/* catch for cases when PROCESS_EVENT* and NM_EV_* collide */
#if (ZIGPC_NET_MGMT_EVENT_MAX_VAL > PROCESS_EVENT_NONE)
#error "ZIGPC_NET_MGMT_EVENT_MAX_VAL is bigger than PROCESS_EVENT_NONE"
#endif

PROCESS_NAME(zigpc_net_mgmt_process);

sl_status_t zigpc_net_mgmt_process_setup(void)
{
  sl_status_t result = SL_STATUS_OK;
  zigpc_net_mgmt_fsm_reset();

  result = zigpc_net_mgmt_gateway_init_observer();

  if (result == SL_STATUS_OK) {
    result = zigpc_net_mgmt_reset_observable();
  }

  if (result == SL_STATUS_OK) {
    process_start(&zigpc_net_mgmt_process, NULL);
  }

  return result;
}

int zigpc_net_mgmt_process_shutdown(void)
{
  // FIXME: Is there anything to be done here?
  return 0;
}

sl_status_t zigpc_net_mgmt_process_send_event(
  const enum zigpc_net_mgmt_process_event event,
  const void *data,
  size_t data_size)
{
  sl_status_t status = SL_STATUS_OK;
  void *data_ptr     = NULL;

  if (data == NULL) {
    status = SL_STATUS_NULL_POINTER;
  }

  if ((status == SL_STATUS_OK) && (data_size == 0)) {
    status = SL_STATUS_EMPTY;
  }

  if (status == SL_STATUS_OK) {
    // NOTE: dynamically allocated
    // will be deallocated in zigpc_net_mgmt_process thread below
    data_ptr = malloc(data_size);
    if (data_ptr == NULL) {
      status = SL_STATUS_ALLOCATION_FAILED;
    }
  }

  if (status == SL_STATUS_OK) {
    memcpy(data_ptr, data, data_size);
    process_post(&zigpc_net_mgmt_process, (process_event_t)event, data_ptr);
  }

  return status;
}

PROCESS(zigpc_net_mgmt_process, "zigpc_net_mgmt_process");

PROCESS_THREAD(zigpc_net_mgmt_process, ev, data)
{
  PROCESS_BEGIN();
  while (1) {
    if (ev == PROCESS_EVENT_INIT) {
      // The setup fixture should have initialized the network management.
      zigpc_gateway_network_init();
    } else if (ev == PROCESS_EVENT_EXIT) {
      // The shutdown fixture should have initialized the network management.
    } else if (ev == PROCESS_EVENT_TIMER) {
      if (data == (void *)zigpc_net_mgmt_fsm_get_timer()) {
        zigpc_net_mgmt_fsm_post_event(ZIGPC_NET_MGMT_FSM_EVENT_TIMEOUT, NULL);
      }
    } else if (ev == ZIGPC_NET_MGMT_EVENT_FSM) {
      zigpc_net_mgmt_process_data_fsm_t *process_data
        = (zigpc_net_mgmt_process_data_fsm_t *)data;
      zigpc_net_mgmt_fsm_post_event(process_data->fsm_event,
                                    &process_data->fsm_data);
    }

    switch (ev) {
      case ZIGPC_NET_MGMT_EVENT_FSM:
        // NOTE: dynamically allocated
        // allocated in zigpc_net_mgmt_process_send_event()
        if (data != NULL) {
          free(data);
          data = NULL;
        }
        break;
      default:
        break;
    }
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}
