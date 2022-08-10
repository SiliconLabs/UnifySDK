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

#include <stdlib.h>
#include <string.h>

// Shared Unify components
#include <sl_log.h>
#include <sl_status.h>

// ZigPC components
#include <zigpc_gateway_notify.h>

#include "attribute_management_int.h"

void zigpc_attrmgmt_callback_on_attribute_frame(
  const zigpc_gateway_on_attribute_reported_t *received_data,
  bool is_read_reponse)
{
  if (received_data != NULL) {
    zigpc_attrmgmt_on_frame_receive_t ev;

    ev.is_read_response = is_read_reponse;

    memcpy(ev.eui64, received_data->eui64, ZIGBEE_EUI64_SIZE);
    ev.endpoint_id = received_data->endpoint_id;
    ev.cluster_id  = received_data->cluster_id;
    ev.frame.size  = received_data->frame.size;
    memcpy(ev.frame.buffer,
           received_data->frame.buffer,
           received_data->frame.size);

    zigpc_attrmgmt_process_send_event(
      ZIGPC_ATTR_MGMT_EVT_ATTR_UPDATE,
      (void *)&ev,
      sizeof(zigpc_attrmgmt_on_frame_receive_t));
  }
}

void zigpc_attrmgmt_callback_on_configure_response_frame(
  const zigpc_gateway_on_attribute_reported_t *received_data)
{
  if (received_data != NULL) {
    zigpc_attrmgmt_on_frame_receive_t ev;

    memcpy(ev.eui64, received_data->eui64, ZIGBEE_EUI64_SIZE);
    ev.endpoint_id = received_data->endpoint_id;
    ev.cluster_id  = received_data->cluster_id;
    ev.frame.size  = received_data->frame.size;
    memcpy(ev.frame.buffer,
           received_data->frame.buffer,
           received_data->frame.size);

    zigpc_attrmgmt_process_send_event(
      ZIGPC_ATTR_MGMT_EVT_CONFIGURE_RESPONSE,
      (void *)&ev,
      sizeof(zigpc_attrmgmt_on_frame_receive_t));
  }
}

void zigpc_attrmgmt_callback_on_attr_report(void *data)
{
  if (data != NULL) {
    zigpc_attrmgmt_callback_on_attribute_frame(
      (const zigpc_gateway_on_attribute_reported_t *)data,
      false);
  }
}
void zigpc_attrmgmt_callback_on_attr_read_response(void *data)
{
  if (data != NULL) {
    zigpc_attrmgmt_callback_on_attribute_frame(
      (const zigpc_gateway_on_attribute_reported_t *)data,
      true);
  }
}

void zigpc_attrmgmt_callback_on_configure_response(void *data)
{
  if (data != NULL) {
    zigpc_attrmgmt_callback_on_configure_response_frame(
      (const zigpc_gateway_on_attribute_reported_t *)data);
  }
}

sl_status_t attr_mgmt_gateway_init_observer(void)
{
  sl_status_t result;

  result
    = zigpc_gateway_register_observer(ZIGPC_GATEWAY_NOTIFY_REPORTED_ATTRIBUTE,
                                      zigpc_attrmgmt_callback_on_attr_report);

  if (result == SL_STATUS_OK) {
    result = zigpc_gateway_register_observer(
      ZIGPC_GATEWAY_NOTIFY_READ_ATTRIBUTE_RESPONSE,
      zigpc_attrmgmt_callback_on_attr_read_response);

    if (result == SL_STATUS_OK) {
      result = zigpc_gateway_register_observer(
        ZIGPC_GATEWAY_NOTIFY_CONFIGURE_REPORTING_RESPONSE,
        zigpc_attrmgmt_callback_on_configure_response);
    }
  }

  return result;
}
