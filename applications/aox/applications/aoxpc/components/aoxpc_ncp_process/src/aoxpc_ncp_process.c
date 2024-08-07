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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "uic_main_externals.h"
#include "process.h"
#include "sl_log.h"
#include "aoxpc_config.h"
#include "sl_bt_ncp_host.h"
#include "sl_bt_api.h"
#include "aoxpc_ncp_process_fixt.h"
#include "aoxpc_ncp_process.h"
#include "aoxpc_ncp_core.h"

// Log tag used in this module
#define LOG_TAG "aoxpc_ncp_process"

typedef enum { BT_EVENT, BT_RESET } ncp_process_event_t;

// NCP ready flag
static bool ncp_ready = false;

// Bluetooth on event callback function pointer
static void(*ncp_bt_on_event)(sl_bt_msg_t *evt);

// Static function declarations
static void ncp_poll(void);

// NCP process definition
PROCESS(ncp_process, "NCP process");

// NCP process implementation
PROCESS_THREAD(ncp_process, ev, data)
{
  PROCESS_POLLHANDLER(ncp_poll());

  PROCESS_BEGIN()
  while (1) {
    switch (ev) {
      case PROCESS_EVENT_INIT:
        ncp_flush();
        sl_log_info(LOG_TAG, "Reset NCP");
        sl_bt_system_reboot();
        break;
      case BT_EVENT:
        if (NULL != ncp_bt_on_event) {
          ncp_bt_on_event((sl_bt_msg_t *)data);
        }
        free(data);
        break;
      case BT_RESET:
        sl_bt_system_reboot();
        break;
    }
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}

// Setup fixture, called on start-up
sl_status_t aoxpc_ncp_process_fixt_setup(void)
{
  sl_status_t status;

  status = uic_main_ext_register_rfd(ncp_get_fd(), NULL, &ncp_process);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Start the NCP process
  process_start(&ncp_process, NULL);

  return SL_STATUS_OK;
}

void ncp_set_cb(void(*bt_on_event)(sl_bt_msg_t *evt))
{
  ncp_bt_on_event = bt_on_event;
}

void ncp_reset(void) {
  if (ncp_ready) {
    ncp_ready = false;
    process_post(&ncp_process, BT_RESET, NULL);
  }
}

static void ncp_poll(void)
{
  sl_bt_msg_t evt;
  sl_bt_msg_t *evt_ptr;

  // Pop (non-blocking) a Bluetooth stack event from event queue.
  sl_status_t status = sl_bt_pop_event(&evt);
  if (status != SL_STATUS_OK) {
    return;
  }

  // Ignore all events until the boot event.
  if (sl_bt_evt_system_boot_id == SL_BT_MSG_ID(evt.header)) {
    ncp_ready = true;
  }

  if (false == ncp_ready) {
    return;
  }

  evt_ptr = malloc(sizeof(sl_bt_msg_t));
  if (evt_ptr != NULL) {
    memcpy(evt_ptr, &evt, sizeof(sl_bt_msg_t));
    process_post(&ncp_process, BT_EVENT, evt_ptr);
  } else {
    sl_log_error(LOG_TAG, "Memory allocation failed, BT event lost.");
  }
}
