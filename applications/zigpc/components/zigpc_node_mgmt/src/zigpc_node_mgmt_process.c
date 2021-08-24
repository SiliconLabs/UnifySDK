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
#include <regex.h>
#include <stdio.h>

/* Shared component */
#include "sl_log.h"
#include "dotdot_mqtt.h"

#include "zigpc_common_zigbee.h"
#include "zigpc_common_unid.h"
#include "zigpc_node_mgmt.h"
#include "zigpc_node_mgmt_int.h"
#include "zigpc_node_map.h"
#include "attribute_management.h"
#include "zigpc_ota_zigbee.h"
#include "zigpc_gateway.h"

#include "process.h"

static const char LOG_TAG[] = "zigpc_node_management";

sl_status_t zigpc_node_mgmt_setup(void)
{
  sl_status_t status = SL_STATUS_OK;
  status             = zigpc_node_mgmt_init_observers();

  if (status == SL_STATUS_OK) {
    process_start(&zigpc_node_mgmt_process, NULL);
  }
  return status;
}

int zigpc_node_mgmt_shutdown(void)
{
  return 0;
}

PROCESS(zigpc_node_mgmt_process, "zigpc_node_mgmt_process");

PROCESS_THREAD(zigpc_node_mgmt_process, ev, data)
{
  PROCESS_BEGIN();

  while (1) {
    sl_status_t status = SL_STATUS_INVALID_TYPE;
    if (ev == PROCESS_EVENT_EXIT) {
      // unretain messages from the nodes and the pc
      zigpc_node_mgmt_unretain_all_topics();
    } else if (ev == NODE_MGMT_MANAGE_EVT) {
      const node_mgmt_manage_node_data_t *event_data
        = (node_mgmt_manage_node_data_t *)data;
      status = nodemap_manage_node(event_data->node);

      configure_attributes_node(event_data->node);

    } else if (ev == NODE_MGMT_MANAGE_EUI64_EVT) {
      const node_mgmt_manage_eui64_data_t *event_data
        = (node_mgmt_manage_eui64_data_t *)data;
      status = nodemap_manage_node_eui64(event_data->eui64);

    } else if (ev == NODE_MGMT_ADD_END_EVT) {
      const node_mgmt_add_endpoint_data_t *event_data
        = (node_mgmt_add_endpoint_data_t *)data;

      status = zigpc_node_process_configure_endpoint(*event_data);
    } else if (ev == NODE_MGMT_UPDATE_EVT) {
      const node_mgmt_update_node_data_t *event_data
        = (node_mgmt_update_node_data_t *)data;
      status = nodemap_update_node(event_data->node);
    } else if (ev == NODE_MGMT_REMOVE_EVT) {
      const node_mgmt_remove_node_data_t *event_data
        = (node_mgmt_remove_node_data_t *)data;
      status = nodemap_remove_node(event_data->eui64);
    }
    sl_log_debug(LOG_TAG, "Event 0x%X handler status: 0x%X", ev, status);
    // must be freed!
    // allocated in zigpc_node_mgmt.c
    if ((data != NULL) && (ev < PROCESS_EVENT_NONE)) {
      free(data);
      data = NULL;
    }

    PROCESS_WAIT_EVENT();
  }
  PROCESS_END();
}

sl_status_t zigpc_node_process_send_event(const enum zigpc_node_mgmt_evt event,
                                          void *data,
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
    // will be deallocated in zigpc_node_mgmt_process thread up there
    data_ptr = malloc(data_size);
    if (data_ptr == NULL) {
      status = SL_STATUS_ALLOCATION_FAILED;
    }
  }

  if (status == SL_STATUS_OK) {
    memcpy(data_ptr, data, data_size);
    process_post(&zigpc_node_mgmt_process, (process_event_t)event, data_ptr);
  }

  return status;
}

sl_status_t zigpc_node_process_configure_endpoint(const node_mgmt_add_endpoint_data_t add_data)
{
    sl_status_t status = SL_STATUS_OK;
      status
        = nodemap_add_node_endpoint(add_data.eui64, add_data.endpoint);

      if (SL_STATUS_OK == status) {
        status = zigpc_gateway_request_binding_endpoint(add_data.eui64,
                                                        add_data.endpoint);
      }

      if (status == SL_STATUS_OK) {
        //TODO replace with OTA enum from zcl_util
        bool has_ota
          = zigpc_nodemgmt_managed_node_contains_cluster(add_data.eui64,
                                                         0x0019);

        if (has_ota) {
          status = zigpc_ota_configure_node(add_data.eui64);
        }

        status = configure_attributes_endpoint(add_data.eui64,
                                               add_data.endpoint);
      } else {
        sl_log_debug(
          LOG_TAG,
          "Failed to add endpoint to node map or configure bindings: 0x%X",
          status);
      }

      size_t unid_len = ZIGBEE_EUI64_HEX_STR_LENGTH + strlen(ZIGPC_UNID_PREFIX);
      char unid[unid_len];
      if (status == SL_STATUS_OK) {
        status = zigpc_common_eui64_to_unid(add_data.eui64, unid, unid_len);
      }

      if (status == SL_STATUS_OK) {
        uic_mqtt_dotdot_publish_supported_commands(
          unid,
          add_data.endpoint.endpoint_id);
      } else {
        sl_log_debug(LOG_TAG, "Failed to convert EUI64 to UNID: 0x%X", status);
      }

    return SL_STATUS_OK;
}
