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
#include "attribute_management_int.h"
#include "attribute_management.h"

sl_status_t configure_attributes_node(const zigbee_node_t node)
{
  sl_status_t status = SL_STATUS_OK;
  if (is_valid_zigbee_node(node)) {
    node_config_report_data_t data;
    copy_node(node, &(data.node));

    status
      = zigpc_attrmgmt_process_send_event(ZIGPC_ATTR_MGMT_EVT_INIT_NODE,
                                          (void *)&data,
                                          sizeof(node_config_report_data_t));
  } else {
    status = SL_STATUS_FAIL;
  }

  return status;
}

sl_status_t configure_attributes_endpoint(const zigbee_eui64_t eui64,
                                          const zigbee_endpoint_t endpoint)
{
  sl_status_t status = SL_STATUS_OK;
  if (is_valid_zigbee_endpoint(endpoint) && eui64 != NULL) {
    endpoint_config_report_data_t data;
    copy_endpoint(endpoint, &(data.endpoint));
    memcpy(data.eui64, eui64, ZIGBEE_EUI64_SIZE);

    status = zigpc_attrmgmt_process_send_event(
      ZIGPC_ATTR_MGMT_EVT_INIT_ENDPOINT,
      (void *)&data,
      sizeof(endpoint_config_report_data_t));
  } else {
    status = SL_STATUS_FAIL;
  }

  return status;
}

sl_status_t zigpc_attribute_management_read_cluster_attributes(
  const zigbee_eui64_t eui64,
  zigbee_endpoint_id_t endpoint_id,
  zcl_cluster_id_t cluster_id)
{
  sl_status_t status = SL_STATUS_OK;

  if (eui64 == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    zigpc_attr_mgmt_read_data_t read_attribute_data;
    memcpy(read_attribute_data.eui64, eui64, ZIGBEE_EUI64_SIZE);
    read_attribute_data.endpoint_id = endpoint_id;
    read_attribute_data.cluster_id  = cluster_id;

    status = zigpc_attrmgmt_process_send_event(
      ZIGPC_ATTR_MGMT_EVT_READ_ATTRIBUTES_COMMAND,
      (void *)&read_attribute_data,
      sizeof(zigpc_attr_mgmt_read_data_t));
  }

  return status;
}
