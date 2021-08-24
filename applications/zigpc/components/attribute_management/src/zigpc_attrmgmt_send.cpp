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

#include <cstring>
#include <vector>

#include "sl_log.h"
#include "sl_status.h"

#include "zcl_attribute_info.h"
#include "zcl_definitions.h"
#include "zcl_util.h"
#include "zigpc_common_zigbee.h"
#include "zigpc_gateway.h"

#include "attribute_management_int.h"
#include "zigpc_attrmgmt_int.hpp"

static const char LOG_TAG[] = "zigpc_attrmgmt";

size_t zigpc_attrmgmt_get_records_to_send_in_message(size_t start_index,
                                                     size_t count,
                                                     size_t limit)
{
  /*
   * In order to split a `count`-sized record array into a maximum of
   * `limit`-sized record sets, the following calculations are performed:
   *
   * 1. Determine if the `start_index` passed in is part of the last set to
   *    send: if `start_index >= (count - limit)`
   *    NOTE: Since we are dealing with unsigned numbers, `limit` is added to
   *    both sides of the inequality.
   *
   * 2. If `start_index` is NOT part of the last set to send, keep sending the
   *    `limit` sized records in a set. If not, send the last few records that
   *    is less than the limit.
   *
   * e.g. For a 10-count array with 3 being the limit:
   *  - for start_index = 0, return 3
   *  - for start_index = 3, return 3
   *  - for start_index = 6, return 3
   *  - for start_index = 9, return 1
   *
   */

  size_t records_to_send;

  if (start_index >= count) {
    records_to_send = 0U;
  } else {
    bool last_message = (start_index + limit) >= count;
    if (last_message == true) {
      records_to_send = (count - start_index);
    } else {
      records_to_send = limit;
    }
  }

  return records_to_send;
}

void zigpc_attrmgmt_send_delayed_read_command_callback(void *data)
{
  if (data != NULL) {
    const zigpc_attr_mgmt_read_data_t *read_data
      = (zigpc_attr_mgmt_read_data_t *)data;

    sl_status_t status = zigpc_attribute_management_read_cluster_attributes(
      read_data->eui64,
      read_data->endpoint_id,
      read_data->cluster_id);

    sl_log_debug(LOG_TAG,
                 "Delayed READ_ATTRIBUTES handler status: 0x%X",
                 status);

    // NOTE: allocated in  zigpc_attrmgmt_send_delayed_read_command
    delete read_data;
  }
}

void zigpc_attrmgmt_send_delayed_read_command(const zigbee_eui64_t eui64,
                                              zigbee_endpoint_id_t endpoint_id,
                                              zcl_cluster_id_t cluster_id)
{
  if (eui64 != NULL) {
    // NOTE: will be freed in zigpc_attrmgmt_send_delayed_read_command_callback
    zigpc_attr_mgmt_read_data_t *read_data = new zigpc_attr_mgmt_read_data_t;

    std::memcpy(read_data->eui64, eui64, ZIGBEE_EUI64_SIZE);
    read_data->endpoint_id = endpoint_id;
    read_data->cluster_id  = cluster_id;

    ctimer_set(&read_data->timer,
               ZIGPC_ATTR_MGMT_DELAY_READ_ATTRIBUTES,
               zigpc_attrmgmt_send_delayed_read_command_callback,
               static_cast<void *>(read_data));
  }
}

sl_status_t zigpc_attrmgmt_send_split_report_config_cmds(
  const zigbee_eui64_t eui64,
  zigbee_endpoint_id_t endpoint_id,
  zcl_cluster_id_t cluster_id,
  const std::vector<zigpc_zcl_configure_reporting_record_t> &records)
{
  sl_status_t status = SL_STATUS_OK;

  if (eui64 == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else if (!records.empty()) {
    /* The following loop splits the attribute record list received into a set
     * of records of ZIGPC_ATTRMGMT_REPORT_RECORDS_LIMIT_PER_MESSAGE count max
     */
    for (size_t i = 0; (i < records.size()) && (status == SL_STATUS_OK);
         i += ZIGPC_ATTRMGMT_REPORT_RECORDS_LIMIT_PER_MESSAGE) {
      size_t records_to_send = zigpc_attrmgmt_get_records_to_send_in_message(
        i,
        records.size(),
        ZIGPC_ATTRMGMT_REPORT_RECORDS_LIMIT_PER_MESSAGE);

      status = zigpc_gateway_configure_reports(
        eui64,
        endpoint_id,
        cluster_id,
        reinterpret_cast<const uint8_t *>(records.data() + i),
        records_to_send * sizeof(zigpc_zcl_configure_reporting_record_t));

      if (status != SL_STATUS_OK) {
        sl_log_debug(LOG_TAG,
                     "Failed to send attribute report config: 0x%X",
                     status);
      }
    }
  }
  return status;
}

sl_status_t
  zigpc_attrmgmt_configure_reports_for_endpoint(const zigbee_eui64_t eui64,
                                                zigbee_endpoint_t endpoint)
{
  sl_status_t status = SL_STATUS_OK;

  if (!is_valid_zigbee_endpoint(endpoint) || (eui64 == nullptr)) {
    status = SL_STATUS_INVALID_PARAMETER;
  }

  for (size_t i = 0U; (i < endpoint.cluster_count) && (status == SL_STATUS_OK);
       i++) {
    zigbee_endpoint_id_t endpoint_id = endpoint.endpoint_id;
    zcl_cluster_id_t cluster_id      = endpoint.cluster_list[i].cluster_id;
    std::vector<zigpc_zcl_configure_reporting_record_t> records;

    status = zigpc_attrmgmt_build_configure_report_records(cluster_id, records);
    if ((status == SL_STATUS_OK) && (records.size() > 0)) {
      status = zigpc_attrmgmt_send_split_report_config_cmds(eui64,
                                                            endpoint_id,
                                                            cluster_id,
                                                            records);
    }
    /*
     * NOTE: Request attribute reads regardless of if reportable attributes
     * have been configured.
     */
    if (status == SL_STATUS_OK) {
      status = zigpc_attribute_management_read_cluster_attributes(eui64,
                                                                  endpoint_id,
                                                                  cluster_id);
    }
  }

  return status;
}

sl_status_t zigpc_attrmgmt_send_split_read_cmds(
  const zigbee_eui64_t eui64,
  zigbee_endpoint_id_t endpoint_id,
  zcl_cluster_id_t cluster_id,
  const std::vector<zcl_attribute_id_t> &attr_ids)
{
  sl_status_t status = SL_STATUS_OK;
  zcl_frame_t zcl_frame;

  if (eui64 == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  } else if (!attr_ids.empty()) {
    /* The following loop splits the attribute record list received into a set
     * of records of max count ZIGPC_ATTRMGMT_READ_RECORDS_LIMIT_PER_MESSAGE
     */
    for (size_t i = 0; (i < attr_ids.size()) && (status == SL_STATUS_OK);
         i += ZIGPC_ATTRMGMT_READ_RECORDS_LIMIT_PER_MESSAGE) {
      size_t reads_to_send = zigpc_attrmgmt_get_records_to_send_in_message(
        i,
        attr_ids.size(),
        ZIGPC_ATTRMGMT_READ_RECORDS_LIMIT_PER_MESSAGE);

      std::vector<zigpc_zcl_frame_data_t> command_args;
      for (size_t j = 0; j < reads_to_send; j++) {
        command_args.push_back(
          {ZIGPC_ZCL_DATA_TYPE_ATTRIB_ID, (attr_ids.data() + i + j)});
      }

      status = zigpc_zcl_build_command_frame(
        &zcl_frame,
        ZIGPC_ZCL_FRAME_TYPE_GLOBAL_CMD_TO_SERVER,
        cluster_id,
        ZCL_GLOBAL_COMMAND_ID_READ_ATTRIBUTES,
        command_args.size(),
        command_args.data());
      if (status != SL_STATUS_OK) {
        sl_log_error(LOG_TAG,
                     "Failed to build global attribute read command: 0x%X ",
                     status);
      } else {
        status = zigpc_gateway_send_zcl_command_frame(eui64,
                                                      endpoint_id,
                                                      cluster_id,
                                                      &zcl_frame);
        if (status != SL_STATUS_OK) {
          sl_log_error(LOG_TAG,
                       "Failed to send global attribute read command: 0x%X",
                       status);
        }
      }
    }
  }
  return status;
}

sl_status_t
  zigpc_attrmgmt_send_read_attributes_command(const zigbee_eui64_t eui64,
                                              zigbee_endpoint_id_t endpoint_id,
                                              zcl_cluster_id_t cluster_id)
{
  sl_status_t status  = SL_STATUS_OK;
  size_t record_count = 0U;

  if (eui64 == NULL) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    record_count = zigpc_zclprofiles_get_cluster_attribute_count(cluster_id);
    if (record_count == 0U) {
      sl_log_warning(LOG_TAG,
                     "Attributes cannot be retrieved for cluster 0x%04X: 0x%X ",
                     status);
      status = SL_STATUS_NOT_SUPPORTED;
    }
  }

  if (status == SL_STATUS_OK) {
    std::vector<zcl_attribute_t> attribute_list(record_count);

    status
      = zigpc_zclprofiles_get_cluster_attribute_list(cluster_id,
                                                     attribute_list.data());

    if (status == SL_STATUS_OK) {
      std::vector<zcl_attribute_id_t> attr_ids;

      for (zcl_attribute_t &attr: attribute_list) {
        attr_ids.push_back(attr.attribute_id);
      }

      status = zigpc_attrmgmt_send_split_read_cmds(eui64,
                                                   endpoint_id,
                                                   cluster_id,
                                                   attr_ids

      );
    }
  }

  return status;
}
