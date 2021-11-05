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

#include <stdlib.h>

/* Contiki include */
#include "process.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"

/* Logger includes */
#include "sl_log.h"

#include "zigpc_common_zigbee.h"
#include "zigpc_gateway.h"
#include "zigpc_gateway_notify.h"

#include "attribute_management_int.h"
#include "attribute_management_process.h"

#include "zcl_attribute_info.h"
#include "attribute_map.h"

static const char LOG_TAG[] = "attribute_management";

PROCESS_NAME(attribute_management_process);

sl_status_t attribute_management_setup(void)
{
  sl_status_t status = SL_STATUS_OK;

  status = attr_mgmt_gateway_init_observer();

  if (status == SL_STATUS_OK) {
    process_start(&attribute_management_process, NULL);
  }
  return status;
}

int attribute_management_shutdown(void)
{
  return 0;
}

sl_status_t zigpc_attrmgmt_process_send_event(const enum attr_mgmt_evt event,
                                              const void *data,
                                              size_t data_size)
{
  sl_status_t status = SL_STATUS_OK;
  void *data_ptr     = NULL;

  if (data == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else if (data_size == 0) {
    status = SL_STATUS_EMPTY;
  } else {
    // NOTE: dynamically allocated
    // will be deallocated in attribute_management_process protothread
    data_ptr = malloc(data_size);
    if (data_ptr == NULL) {
      status = SL_STATUS_ALLOCATION_FAILED;
    }
  }

  if (status == SL_STATUS_OK) {
    memcpy(data_ptr, data, data_size);
    process_post(&attribute_management_process,
                 (process_event_t)event,
                 data_ptr);
  }

  return status;
}

PROCESS(attribute_management_process, "attribute_management_process");

PROCESS_THREAD(attribute_management_process, ev, data)
{
  static struct etimer zigpc_attrmgmt_poll_timer;

  PROCESS_BEGIN();

  etimer_set(&zigpc_attrmgmt_poll_timer, ZIGPC_ATTR_MGMT_POLL_PERIOD_MS);

  while (1) {
    sl_status_t status = SL_STATUS_INVALID_TYPE;

    if (ev == PROCESS_EVENT_INIT) {
      //do nothing, should be handled by setup fixture
    } else if (ev == ZIGPC_ATTR_MGMT_EVT_INIT_NODE) {
      const node_config_report_data_t *node_data
        = (node_config_report_data_t *)data;
      status = node_send_report_config_cmd(node_data->node);

      if (status == SL_STATUS_OK) {
        status = register_node_attributes(node_data->node);
      }

    } else if (ev == ZIGPC_ATTR_MGMT_EVT_INIT_ENDPOINT) {
      const endpoint_config_report_data_t *endpoint_data
        = (endpoint_config_report_data_t *)data;
      status = zigpc_attrmgmt_configure_reports_for_endpoint(
        endpoint_data->eui64,
        endpoint_data->endpoint);

      if (status == SL_STATUS_OK) {
        status = register_endpoint_attributes(endpoint_data->eui64,
                                              endpoint_data->endpoint);
      } else {
        sl_log_error(LOG_TAG, "Failed to send report config: 0x%X", status);
      }

      if (status != SL_STATUS_OK) {
        sl_log_error(LOG_TAG, "Failed to register attributes: 0x%X", status);
      }

    } else if (ev == ZIGPC_ATTR_MGMT_EVT_READ_ATTRIBUTES_COMMAND) {
      zigpc_attr_mgmt_read_data_t *read_data
        = (zigpc_attr_mgmt_read_data_t *)data;

      status
        = zigpc_attrmgmt_send_read_attributes_command(read_data->eui64,
                                                      read_data->endpoint_id,
                                                      read_data->cluster_id);

    } else if (ev == ZIGPC_ATTR_MGMT_EVT_ATTR_UPDATE) {
      const zigpc_attrmgmt_on_frame_receive_t *receive_data
        = (zigpc_attrmgmt_on_frame_receive_t *)data;

      status
        = zigpc_attrmgmt_receive_attribute_frame(receive_data->eui64,
                                                 receive_data->endpoint_id,
                                                 receive_data->cluster_id,
                                                 receive_data->is_read_response,
                                                 &receive_data->frame);
    } else if (ev == ZIGPC_ATTR_MGMT_EVT_CONFIGURE_RESPONSE) {
      const zigpc_attrmgmt_on_frame_receive_t *receive_data
        = (zigpc_attrmgmt_on_frame_receive_t *)data;

      status = zigpc_attrmgmt_receive_configure_response_frame(
        receive_data->eui64,
        receive_data->endpoint_id,
        receive_data->cluster_id,
        &receive_data->frame);
    } else if ((ev == PROCESS_EVENT_TIMER)
               && (data == &zigpc_attrmgmt_poll_timer)) {
      status = zigpc_attrmgmt_send_poll_attributes();

      etimer_reset(&zigpc_attrmgmt_poll_timer);
    }

    // must be freed!
    // allocated in zigpc_attrmgmt_process_send_event())
    if ((data != NULL) && (ev < PROCESS_EVENT_NONE)) {
      free(data);
      data = NULL;
    }

    sl_log_debug(LOG_TAG, "Event 0x%X handler status: 0x%X", ev, status);

    PROCESS_WAIT_EVENT();
  }
  PROCESS_END();
}

sl_status_t node_send_report_config_cmd(const zigbee_node_t node)
{
  sl_status_t status = SL_STATUS_OK;
  if (is_valid_zigbee_node(node)) {
    unsigned int i;
    for (i = 0; (i < node.endpoint_count) && (status == SL_STATUS_OK); i++) {
      status
        = zigpc_attrmgmt_configure_reports_for_endpoint(node.eui64,
                                                        node.endpoint_list[i]);
    }
  } else {
    status = SL_STATUS_FAIL;
  }
  return status;
}

#ifdef COMPILE_UNUSED_CODE
sl_status_t update_attribute_from_report(const attribute_report_t report)
{
  sl_status_t status = SL_STATUS_OK;
  attribute_key key;

  memcpy(key.eui64, report.eui64, ZIGBEE_EUI64_SIZE);
  key.endpoint_id = report.endpoint_id;
  key.cluster_id  = report.cluster_id;

  zcl_attribute_id_t attribute_id = report.attribute_id;
  zcl_attribute_t attribute;

  status = read_single_attribute(key, attribute_id, &attribute);

  if ((status == SL_STATUS_OK) && (attribute.type == report.type)) {
    status = attribute_val_to_string(report.data,
                                     attribute.type,
                                     attribute.reported_payload,
                                     ZCL_DEFAULT_STR_LENGTH);
  }

  if (status == SL_STATUS_OK) {
    status = update_attributes(key, &attribute, 1);
  }

  return status;
}
#endif  // COMPILE_UNUSED_CODE
