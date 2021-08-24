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

/* Contiki includes */
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "sys/etimer.h"

/* Logger includes */
#include "sl_log.h"

#include "zigpc_config.h"
#include "zigpc_gateway_process.h"
#include "zigpc_gateway_process_send.h"
#include "zigpc_gateway_int.h"

#if (ZIGPC_GW_EVENT_MAX_VAL > PROCESS_EVENT_NONE)
#error "ZIGPC_GW_EVENT_MAX_VAL is bigger than PROCESS_EVENT_NONE"
#endif

#define ZIGPC_GATEWAY_TICK_PERIOD (CLOCK_SECOND / 80)

PROCESS(zigpc_gateway_process, "ZIGPC Gateway process");

sl_status_t zigpc_gateway_process_setup(void)
{
  sl_status_t result = SL_STATUS_OK;
  const zigpc_config_t *zigpc_config;
  struct z3gatewayOpts z3gw_opts;

  result = zigpc_gateway_reset_observers();

  if (result == SL_STATUS_OK) {
    zigpc_config = zigpc_get_config();

    z3gw_opts.serialPort = (char *)zigpc_config->serial_port;

    z3gw_opts.callbacks = &zigpc_gateway_z3gateway_callbacks;

    result = (z3gatewayInit(&z3gw_opts) == EMBER_SUCCESS) ? SL_STATUS_OK
                                                          : SL_STATUS_FAIL;
  }
  if (result == SL_STATUS_OK) {
    result = zigpc_gateway_load_from_datastore();
    if (result != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "Failed to load information from datastore: 0x%X",
                   result);
    }
  }

  if (result == SL_STATUS_OK) {
    process_start(&zigpc_gateway_process, NULL);
  }

  return result;
}

int zigpc_gateway_process_shutdown(void)
{
  sl_status_t status;

  status = zigpc_gateway_save_to_datastore();
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Failed to store information to datastore: 0x%X",
                 status);
  }

  status = z3gatewayTrustCenterJoinClose();
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG, "Failed to close network: 0x%X", status);
  }

  z3gatewayShutdown();
  return 0;
}

sl_status_t
  zigpc_gateway_process_send_event(const enum zigpc_gateway_process_event event,
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
    // will be deallocated in zigpc_gateway_process thread below
    data_ptr = malloc(data_size);
    if (data_ptr == NULL) {
      status = SL_STATUS_ALLOCATION_FAILED;
    }
  }

  if (status == SL_STATUS_OK) {
    memcpy(data_ptr, data, data_size);
    process_post(&zigpc_gateway_process, (process_event_t)event, data_ptr);
  }

  return status;
}

PROCESS_THREAD(zigpc_gateway_process, ev, data)
{
  /**
   * Periodic timer to process messages to and from the NCP via EZSP
   */
  static struct etimer zigpc_gateway_tick;

  PROCESS_BEGIN()

  etimer_set(&zigpc_gateway_tick, ZIGPC_GATEWAY_TICK_PERIOD);

  while (1) {
    if (ev == PROCESS_EVENT_INIT) {
      sl_log_debug(LOG_TAG, "Processing init");

    } else if (ev == ZIGPC_GW_EVENT_DISPATCH) {
      struct zigpc_gateway_dispatch_event *dispatch_data
        = (struct zigpc_gateway_dispatch_event *)data;

      sl_status_t status = zigpc_gateway_dispatch(dispatch_data);
      if (status != SL_STATUS_NULL_POINTER) {
        sl_log_debug(LOG_TAG,
                     "Dispatch %s: %s",
                     zigpc_gateway_dispatch_name(dispatch_data->type),
                     (status == SL_STATUS_OK) ? "successful" : "failed");
      }
    } else if (ev == ZIGPC_GW_EVENT_ZCL_FRAME) {
      sl_status_t success;

      success = zigpc_gateway_dispatch_zcl_frame(
        (zigpc_gateway_dispatch_zcl_frame_t *)data);

      sl_log_debug(LOG_TAG, "ZCL Frame send status: 0x%X", success);

    } else if (ev == ZIGPC_GW_EVENT_INIT_REPORTS) {
      zigpc_gateway_dispatch_init_report_t *report_data
        = (zigpc_gateway_dispatch_init_report_t *)data;

      sl_status_t status = zigpc_gateway_dispatch_init_reports(report_data);

      if (status == SL_STATUS_OK) {
        sl_log_debug(LOG_TAG, "Successfully initialized reporting");
      } else {
        sl_log_warning(LOG_TAG, "Error initializing reporting");
      }
    } else if (ev == ZIGPC_GW_EVENT_CONFIG_BINDING) {
      zigpc_gateway_dispatch_bind_request_t *bind_data
        = (zigpc_gateway_dispatch_bind_request_t *)data;
      sl_status_t status = zigpc_gateway_dispatch_config_binding(bind_data);

      if (status == SL_STATUS_OK) {
        sl_log_debug(LOG_TAG, "Successfully sent bind request");
      } else {
        sl_log_warning(LOG_TAG, "Error sending bind request");
      }
    }
    else if (ev == ZIGPC_GW_EVENT_OTA_FILE){

        zigpc_gateway_dispatch_add_ota_image_t
            *ota_image_data = 
                (zigpc_gateway_dispatch_add_ota_image_t*) data;

        sl_status_t status = 
            zigpc_gateway_dispatch_ota_image(ota_image_data);

        if(SL_STATUS_OK == status)
        {
            sl_log_debug(LOG_TAG, "Success sending OTA Image to ota-server plugin");
        }
        else
        {
            sl_log_warning(LOG_TAG, "Error sending OTA Image to ota-server plugin");
        }
    }
    else if (ev == PROCESS_EVENT_TIMER) {
      if (data == &zigpc_gateway_tick) {
        etimer_reset(&zigpc_gateway_tick);
      }

    } else if (ev == PROCESS_EVENT_EXIT) {
      /* Stop process */
      sl_log_debug(LOG_TAG, "Exit");
      break;

    } else {
      sl_log_warning(LOG_TAG, "Unexpected event received: %u", ev);
    }
    z3gatewayTick();

    // Deallocate data for events received
    if ((data != NULL) && (ev < PROCESS_EVENT_NONE)) {
      // NOTE: dynamically allocated
      // allocated in zigpc_gateway_process_send_event()
      free(data);
      data = NULL;
    }

    PROCESS_WAIT_EVENT();
  }

  PROCESS_END()
}
