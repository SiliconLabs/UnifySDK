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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Shared UIC includes
#include "sl_log.h"

// ZigPC includes
#include <zigpc_datastore.h>

#include "zigpc_gateway_process.h"
#include "zigpc_gateway_process_send.h"
#include "zigpc_gateway_int.h"
#include "gateway_sleepy_queue.h"

#define LOG_TAG "zigpc_gateway"

sl_status_t
  zigpc_gateway_add_node_install_code(const zigbee_eui64_t *node_eui64,
                                      const zigbee_install_code_t *install_code,
                                      const uint8_t install_code_length)
{
  sl_status_t status = SL_STATUS_OK;
  struct zigpc_gateway_dispatch_event dispatch_event;

  if ((node_eui64 == NULL) || (install_code == NULL)) {
    status = SL_STATUS_NULL_POINTER;
  }

  if (status == SL_STATUS_OK) {
    memcpy(dispatch_event.add_node.install_code,
           install_code,
           sizeof(zigbee_install_code_t));
    zigbee_eui64_copy_switch_endian(dispatch_event.add_node.eui64, *node_eui64);
    dispatch_event.add_node.install_code_length = install_code_length;

    dispatch_event.type = ZIGPC_GW_DISPATCH_ADD_NODE_INSTALL_CODE;

    status = zigpc_gateway_process_send_event(
      ZIGPC_GW_EVENT_DISPATCH,
      (void *)&dispatch_event,
      sizeof(struct zigpc_gateway_dispatch_event));
  }

  return status;
}

sl_status_t zigpc_gateway_network_init(void)
{
  sl_status_t status = SL_STATUS_OK;
  struct zigpc_gateway_dispatch_event dispatch_event;
  dispatch_event.type = ZIGPC_GW_DISPATCH_NETWORK_INIT;

  status = zigpc_gateway_process_send_event(
    ZIGPC_GW_EVENT_DISPATCH,
    (void *)&dispatch_event,
    sizeof(struct zigpc_gateway_dispatch_event));

  return status;
}

sl_status_t zigpc_gateway_network_permit_joins(bool enable)
{
  sl_status_t status = SL_STATUS_OK;
  struct zigpc_gateway_dispatch_event dispatch_event;
  dispatch_event.type                = ZIGPC_GW_DISPATCH_PERMIT_JOINS;
  dispatch_event.permit_joins.enable = enable;

  status = zigpc_gateway_process_send_event(
    ZIGPC_GW_EVENT_DISPATCH,
    (void *)&dispatch_event,
    sizeof(struct zigpc_gateway_dispatch_event));

  return status;
}

sl_status_t
  zigpc_gateway_send_zcl_command_frame(const zigbee_eui64_t eui64,
                                       zigbee_endpoint_id_t endpoint_id,
                                       zcl_cluster_id_t cluster_id,
                                       const zcl_frame_t *frame)
{
  sl_status_t status = SL_STATUS_OK;
  zigpc_gateway_dispatch_zcl_frame_t zcl_frame_data;

  if ((eui64 == NULL) || (frame == NULL)) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    zcl_frame_data.send_as_multicast = false;
    zigbee_eui64_copy_switch_endian(zcl_frame_data.eui64, eui64);
    zcl_frame_data.endpoint_id = endpoint_id;

    zcl_frame_data.cluster_id               = cluster_id;
    zcl_frame_data.frame.size               = frame->size;
    zcl_frame_data.frame.offset_sequence_id = frame->offset_sequence_id;
    memcpy(zcl_frame_data.frame.buffer,
           frame->buffer,
           ZCL_FRAME_BUFFER_SIZE_MAX);

    bool sleepy_cluster_found
      = zigpc_datastore_contains_cluster(eui64,
                                         endpoint_id,
                                         ZCL_CLUSTER_SERVER_SIDE,
                                         ZIGPC_ZCL_CLUSTER_POLL_CONTROL);

    if (!sleepy_cluster_found) {
      status = zigpc_gateway_process_send_event(
        ZIGPC_GW_EVENT_ZCL_FRAME,
        (void *)&zcl_frame_data,
        sizeof(zigpc_gateway_dispatch_zcl_frame_t));
    } else {
      status = zigpc_gateway_store_sleepy_frame(zcl_frame_data);
    }
  }

  return status;
}

sl_status_t zigpc_gateway_unload_sleepy_messages(const zigbee_eui64_t eui64)
{
  sl_status_t status       = SL_STATUS_OK;
  unsigned int num_retries = 0;

  //send a check-in response
  z3gatewaySendPollingCheckInResponse();

  while ((status != SL_STATUS_EMPTY) && (status != SL_STATUS_FAIL)
         && (num_retries < MAX_SLEEPY_RETRIES)) {
    zigpc_gateway_dispatch_zcl_frame_t zcl_frame_data;

    status = zigpc_gateway_retrieve_sleepy_frame(eui64, &zcl_frame_data);

    if (status == SL_STATUS_OK) {
      status = zigpc_gateway_process_send_event(
        ZIGPC_GW_EVENT_ZCL_FRAME,
        (void *)&zcl_frame_data,
        sizeof(zigpc_gateway_dispatch_zcl_frame_t));
    }

    num_retries = num_retries + 1;
  }

  return status;
}

sl_status_t
  zigpc_gateway_send_zcl_frame_multicast(zigbee_multicast_id_t multicast_id,
                                         zcl_cluster_id_t cluster_id,
                                         const zcl_frame_t *frame)
{
  sl_status_t status = SL_STATUS_OK;
  zigpc_gateway_dispatch_zcl_frame_t zcl_frame_data;

  if (frame == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    zcl_frame_data.send_as_multicast = true;
    zcl_frame_data.multicast_id      = multicast_id;

    zcl_frame_data.cluster_id               = cluster_id;
    zcl_frame_data.frame.size               = frame->size;
    zcl_frame_data.frame.offset_sequence_id = frame->offset_sequence_id;
    memcpy(zcl_frame_data.frame.buffer,
           frame->buffer,
           ZCL_FRAME_BUFFER_SIZE_MAX);

    status = zigpc_gateway_process_send_event(
      ZIGPC_GW_EVENT_ZCL_FRAME,
      (void *)&zcl_frame_data,
      sizeof(zigpc_gateway_dispatch_zcl_frame_t));
  }

  return status;
}

sl_status_t zigpc_gateway_request_binding(const zigbee_eui64_t eui64,
                                          uint8_t endpoint_id,
                                          zcl_cluster_id_t cluster_id)
{
  sl_status_t status = SL_STATUS_OK;

  zigpc_gateway_dispatch_bind_request_t bind_data;

  zigbee_eui64_copy_switch_endian(bind_data.eui64, eui64);
  bind_data.endpoint_id = endpoint_id;
  bind_data.cluster_id  = cluster_id;

  status = zigpc_gateway_process_send_event(
    ZIGPC_GW_EVENT_CONFIG_BINDING,
    (void *)&bind_data,
    sizeof(zigpc_gateway_dispatch_bind_request_t));

  return status;
}

sl_status_t
  zigpc_gateway_request_binding_endpoint(const zigbee_eui64_t eui64,
                                         const zigbee_endpoint_t endpoint)
{
  const unsigned int num_clusters = endpoint.cluster_count;
  for (unsigned int i = 0; i < num_clusters; i++) {
    sl_status_t status
      = zigpc_gateway_request_binding(eui64,
                                      endpoint.endpoint_id,
                                      endpoint.cluster_list[i].cluster_id);

    if (SL_STATUS_OK != status) {
      sl_log_warning(LOG_TAG,
                     "Error sending binding request for eui64 %llx - endpoint "
                     "%u - cluster %u",
                     eui64,
                     endpoint.endpoint_id,
                     endpoint.cluster_list[i].cluster_id);
    }
  }
  return SL_STATUS_OK;
}

sl_status_t zigpc_gateway_configure_reports(const zigbee_eui64_t eui64,
                                            uint8_t endpoint_id,
                                            zcl_cluster_id_t cluster_id,
                                            const uint8_t *report_record,
                                            unsigned int record_size)

{
  zigpc_gateway_dispatch_init_report_t report_data;
  sl_status_t status = SL_STATUS_OK;

  if ((eui64 == NULL) || (report_record == NULL)) {
    status = SL_STATUS_NULL_POINTER;
  }

  if (status == SL_STATUS_OK) {
    zigbee_eui64_copy_switch_endian(report_data.eui64, eui64);
    report_data.endpoint_id   = endpoint_id;
    report_data.cluster_id    = cluster_id;
    report_data.report_record = malloc(sizeof(uint8_t) * record_size);
    memcpy(report_data.report_record, report_record, record_size);
    report_data.record_size = record_size;

    status = zigpc_gateway_process_send_event(
      ZIGPC_GW_EVENT_INIT_REPORTS,
      (void *)&report_data,
      sizeof(zigpc_gateway_dispatch_init_report_t));
  }

  return status;
}

sl_status_t zigpc_gateway_remove_node(const zigbee_eui64_t eui64)
{
  sl_status_t status = SL_STATUS_OK;
  struct zigpc_gateway_dispatch_event dispatch_event;

  if (eui64 == NULL) {
    status = SL_STATUS_NULL_POINTER;
  }

  if (status == SL_STATUS_OK) {
    zigbee_eui64_copy_switch_endian(dispatch_event.remove_node.eui64, eui64);
    dispatch_event.type = ZIGPC_GW_DISPATCH_REMOVE_NODE;

    zigpc_gateway_process_send_event(
      ZIGPC_GW_EVENT_DISPATCH,
      (void *)&dispatch_event,
      sizeof(struct zigpc_gateway_dispatch_event));
  }

  return status;
}

sl_status_t zigpc_gateway_interview_node(const zigbee_eui64_t eui64)
{
  sl_status_t status = SL_STATUS_OK;
  struct zigpc_gateway_dispatch_event dispatch_event;

  dispatch_event.type = ZIGPC_GW_DISPATCH_INTERVIEW_NODE;
  zigbee_eui64_copy_switch_endian(dispatch_event.interview_node.eui64, eui64);

  status = zigpc_gateway_process_send_event(
    ZIGPC_GW_EVENT_DISPATCH,
    (void *)&dispatch_event,
    sizeof(struct zigpc_gateway_dispatch_event));

  return status;
}

sl_status_t zigpc_gateway_add_ota_image(const char *filename,
                                        unsigned int filename_size)
{
  sl_status_t status = SL_STATUS_OK;

  zigpc_gateway_dispatch_add_ota_image_t ota_image_data;

  if ((filename != NULL) && (filename_size > 0)
      && (filename_size < ZCL_DEFAULT_STR_LENGTH)) {
    snprintf(ota_image_data.filename, ZCL_DEFAULT_STR_LENGTH, "%s", filename);
    ota_image_data.filename_size = filename_size;

    status = zigpc_gateway_process_send_event(
      ZIGPC_GW_EVENT_OTA_FILE,
      (void *)&ota_image_data,
      sizeof(zigpc_gateway_dispatch_add_ota_image_t));
  } else {
    status = SL_STATUS_FAIL;
  }

  return status;
}

void zigpc_gateway_command_print_info(void)
{
  z3gatewayCommandPrintInfo();
}

void zigpc_gateway_command_print_nwk_key(void)
{
  EmberKeyStruct nwk_key;

  EmberStatus status
    = z3gatewayGetEmberKey(EMBER_CURRENT_NETWORK_KEY, &nwk_key);
  if (status != EMBER_SUCCESS) {
    sl_log_error(LOG_TAG, "Failed to read NWK key: EmberStatus(0x%X)", status);
  } else {
    char nwk_key_str[EMBER_ENCRYPTION_KEY_SIZE * 3 + 1];
    size_t nwk_key_str_size = 0;
    for (uint8_t i = 0; i < EMBER_ENCRYPTION_KEY_SIZE; i++) {
      nwk_key_str_size += snprintf(nwk_key_str + nwk_key_str_size,
                                   4,
                                   " %02X",
                                   emberKeyContents(&(nwk_key.key))[i]);
    }
    sl_log_info(LOG_TAG, "NWK Key:%s", nwk_key_str);
  }
}
