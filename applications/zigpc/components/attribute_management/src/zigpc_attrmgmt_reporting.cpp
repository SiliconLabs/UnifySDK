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

#include <vector>
#include <cstring>

#include "sl_log.h"
#include "sl_status.h"

#include "zcl_attribute_info.h"
#include "zcl_definitions.h"
#include "zigpc_common_zigbee.h"

#include "attribute_management_int.h"
#include "zigpc_attrmgmt_int.hpp"

static const char LOG_TAG[] = "zigpc_attrmgmt";

sl_status_t zigpc_attrmgmt_build_configure_report_records(
  zcl_cluster_id_t cluster_id,
  std::vector<zigpc_zcl_configure_reporting_record_t> &record_list)
{
  sl_status_t status = SL_STATUS_OK;

  size_t attr_count = zigpc_zclprofiles_get_cluster_attribute_count(cluster_id);
  if (attr_count > 0) {
    std::vector<zcl_attribute_t> attr_list(attr_count);

    status = zigpc_zclprofiles_get_cluster_attribute_list(cluster_id,
                                                          attr_list.data());
    if (status == SL_STATUS_OK) {
      for (const zcl_attribute_t &attr: attr_list) {
        if (attr.is_reportable == true) {
          zigpc_zcl_configure_reporting_record_t record = {
            ZIGPC_ZCL_GLOBAL_REPORTING_ROLE_GENERATOR,  // direction
            attr.attribute_id,                          // attribute_identifier
            static_cast<uint8_t>(attr.type),            // attribute_type
            ZIGPC_ATTRMGMT_REPORT_INTERVAL_MIN_DEFAULT,  // minimum_reporting_interval
            ZIGPC_ATTRMGMT_REPORT_INTERVAL_MAX_DEFAULT,  // maximum_reporting_interval
            ZIGPC_ATTRMGMT_REPORT_CHANGE_DEFAULT,        // reportable_change
          };
          record_list.push_back(record);
        }
      }
    }
  }

  return status;
}

sl_status_t zigpc_attrmgmt_buffer_to_configure_status(
  const configure_response_buffer_t buffer,
  zigpc_attrmgmt_configure_status_record_t &record)
{
  record.status       = buffer[0];
  record.direction    = buffer[1];
  record.attribute_id = (buffer[3] << 8) | buffer[2];

  return SL_STATUS_OK;
}

sl_status_t zigpc_attrmgmt_buffer_to_configure_status_list(
  const uint8_t *buffer,
  size_t buffer_len,
  std::list<zigpc_attrmgmt_configure_status_record_t> &record_list)
{
  sl_status_t status = SL_STATUS_OK;

  for (unsigned int i = 0; i < buffer_len; i = i + 4) {
    configure_response_buffer_t temp_buffer;
    zigpc_attrmgmt_configure_status_record_t record;
    memcpy(temp_buffer, buffer + i, 4);

    status = zigpc_attrmgmt_buffer_to_configure_status(temp_buffer, record);

    if (status == SL_STATUS_OK) {
      record_list.push_back(record);
    }
  }

  return SL_STATUS_OK;
}

sl_status_t zigpc_attrmgmt_receive_configure_response_frame(
  const zigbee_eui64_t eui64,
  zigbee_endpoint_id_t endpoint_id,
  zcl_cluster_id_t cluster_id,
  const zcl_frame_t *frame)
{
  sl_status_t status = SL_STATUS_OK;
  std::list<zigpc_attrmgmt_configure_status_record_t> record_list;

  status = zigpc_attrmgmt_buffer_to_configure_status_list(frame->buffer,
                                                          frame->size,
                                                          record_list);

  for (auto record: record_list) {
    if (record.status != 0) {
      status = zigpc_attrmgmt_add_poll_entry(eui64, endpoint_id, cluster_id);
      break;
    }
  }

  return status;
}
