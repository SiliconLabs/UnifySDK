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

// Shared Unify components
#include <sl_status.h>
#include <sl_log.h>

// ZigPC components
#include <zcl_definitions.h>
#include <zcl_util.h>
#include <zigpc_common_zigbee.h>

#include "attribute_management_int.h"

static const char LOG_TAG[] = "zigpc_attrmgmt";

sl_status_t
  zigpc_attrmgmt_receive_attribute_frame(const zigbee_eui64_t eui64,
                                         zigbee_endpoint_id_t endpoint_id,
                                         zcl_cluster_id_t cluster_id,
                                         bool is_read_response,
                                         const zcl_frame_t *frame)
{
  sl_status_t status = SL_STATUS_OK;

  if ((eui64 == NULL) || (frame == NULL)) {
    status = SL_STATUS_NULL_POINTER;
  } else {
    size_t i              = 0;
    const uint8_t *buffer = frame->buffer;

    // Process frame for each record (read/report)
    bool bytes_left_to_process = i < frame->size;
    while (bytes_left_to_process == true) {
      zcl_attribute_id_t attr_id = ((buffer[i + 1] << 8) | buffer[i]);

      if ((is_read_response == true)
          && (buffer[i + 2] != ZIGPC_ZCL_STATUS_SUCCESS)) {
        sl_log_debug(LOG_TAG,
                     "Read response for attr[0x%04X] failed with ZCL status "
                     "0x%02X. Skipping to next read response record",
                     attr_id,
                     buffer[i + 2]);

        // Skip to next read response record
        i += 3;

      } else {
        // skip over status field in read response records using offset
        size_t offset                   = (is_read_response == true) ? 1 : 0;
        zigpc_zcl_data_type_t attr_type = buffer[i + 2 + offset];
        const uint8_t *attr_data        = buffer + i + 3 + offset;

        size_t attr_data_size = zigpc_zcl_get_data_type_size(attr_type);

        status = zigpc_attrmgmt_publish_reported(zigbee_eui64_to_uint(eui64),
                                                 endpoint_id,
                                                 cluster_id,
                                                 attr_id,
                                                 attr_data);
        if (status != SL_STATUS_OK) {
          sl_log_warning(
            LOG_TAG,
            "Fail to update attribute 0x%04X for cluster 0x%04X: 0x%X",
            attr_id,
            cluster_id,
            status);
        }

        i += (3 + offset) + attr_data_size;
        bytes_left_to_process = i < frame->size;
      }
    }
  }

  return status;
}
