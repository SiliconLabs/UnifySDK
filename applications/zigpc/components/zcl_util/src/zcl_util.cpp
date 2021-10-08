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
#include <cstdarg>
#include <unordered_map>

/* Shared components */
#include "sl_log.h"

#include "zcl_util.h"
#include "zcl_definitions.h"
#include "zcl_util_int.hpp"

#define ZIGPC_ZCL_FRAME_SEQUENCE_NUMBER_PLACEHOLDER 0x00

static const char LOG_TAG[] = "zigpc_zcl_util";

size_t zigpc_zcl_get_data_type_size(zigpc_zcl_data_type_t type)
{
  size_t size = 0;
  const std::unordered_map<zigpc_zcl_data_type_t, size_t>
    *zcl_data_type_size_map = zigpc_zcl_get_zcl_data_type_size_map();

  auto type_size_found = zcl_data_type_size_map->find(type);
  if (type_size_found == zcl_data_type_size_map->end()) {
    sl_log_debug(LOG_TAG, "Unable to find size for data type");
  } else {
    size = type_size_found->second;
  }

  return size;
}

sl_status_t zigpc_zcl_frame_init_command(zcl_frame_t *const frame,
                                         zcl_command_id_t command_id,
                                         uint8_t frame_control)
{
  sl_status_t status = SL_STATUS_OK;

  if (frame == nullptr) {
    status = SL_STATUS_NULL_POINTER;
  }

  if (status == SL_STATUS_OK) {
    frame->size = 0;

    frame->buffer[frame->size++] = frame_control;
    frame->buffer[frame->size]   = ZIGPC_ZCL_FRAME_SEQUENCE_NUMBER_PLACEHOLDER;

    // Store sequence ID offset to be passed into Z3Gateway API
    frame->offset_sequence_id = frame->size++;

    frame->buffer[frame->size++] = command_id;
  }

  return status;
}

sl_status_t zigpc_zcl_frame_fill_data_array(zcl_frame_t *const frame,
                                            zigpc_zcl_data_type_t type,
                                            size_t type_count,
                                            const void *data)
{
  sl_status_t status = SL_STATUS_OK;
  size_t type_size   = 0;

  if ((frame == nullptr) || (data == nullptr)) {
    status = SL_STATUS_NULL_POINTER;
  } else if (type_count == 0U) {
    status = SL_STATUS_INVALID_COUNT;
  } else {
    type_size = zigpc_zcl_get_data_type_size(type);
    if (type_size == 0) {
      status = SL_STATUS_INVALID_TYPE;
    }
  }

  if (status == SL_STATUS_OK) {
    // Check if data copy size would overflow the frame buffer
    size_t copy_size = type_size * type_count;

    if ((copy_size + frame->size) > ZCL_FRAME_BUFFER_SIZE_MAX) {
      status = SL_STATUS_WOULD_OVERFLOW;
      sl_log_error(LOG_TAG, "Data size would overflow ZCL frame: 0x%X", status);
    }
  }

  if (status == SL_STATUS_OK) {
    memcpy(&frame->buffer[frame->size], data, type_size * type_count);
    frame->size += (type_size * type_count);
  }

  return status;
}

sl_status_t zigpc_zcl_frame_fill_data(zcl_frame_t *const frame,
                                      zigpc_zcl_data_type_t type,
                                      const void *data)
{
  return zigpc_zcl_frame_fill_data_array(frame, type, 1, data);
}

sl_status_t zigpc_zcl_add_command_frame_arg(zcl_frame_t *const frame,
                                            zigpc_zcl_data_type_t arg_type,
                                            const void *arg_data)
{
  sl_status_t status = SL_STATUS_OK;

  if ((frame == nullptr) || (arg_data == nullptr)) {
    status = SL_STATUS_NULL_POINTER;
  } else if ((arg_type == ZIGPC_ZCL_DATA_TYPE_STRING)
             || (arg_type == ZIGPC_ZCL_DATA_TYPE_OCTSTR)) {
    /**
     * oct/str case: the length of oct/str w/o terminator needs to be copied
     * over as first byte. On empty strings, the length byte of 0x00 should
     * be sent. Length of 0xFF is reserved for invalid oct/str. Hence
     * any oct/str containing more than 254 characters will return an invalid
     * signature error.
     */
    uint8_t arg_str_len = 0U;
    size_t str_len      = strlen((const char *)arg_data);
    if (str_len >= 0xFF) {
      status = SL_STATUS_INVALID_SIGNATURE;
      sl_log_error(LOG_TAG,
                   "Invalid oct/str command argument size: 0x%X",
                   status);
    } else {
      // NOTE: null character is not counted in this length
      arg_str_len = static_cast<uint8_t>(str_len & 0xFF);

      // Add string length even if it's zero (as per spec)
      status = zigpc_zcl_frame_fill_data(frame,
                                         ZIGPC_ZCL_DATA_TYPE_UINT8,
                                         &arg_str_len);
    }

    if ((status == SL_STATUS_OK) && (arg_str_len > 0U)) {
      status = zigpc_zcl_frame_fill_data_array(frame,
                                               arg_type,
                                               arg_str_len,
                                               arg_data);
    }
  } else {
    /**
     * Primitive type handling.
     */
    status = zigpc_zcl_frame_fill_data(frame, arg_type, arg_data);
  }

  return status;
}

sl_status_t zigpc_zcl_build_command_frame(
  zcl_frame_t *const frame,
  const zigpc_zcl_frame_type_t frame_type,
  const zcl_cluster_id_t cluster_id,
  const zcl_command_id_t command_id,
  const size_t command_arg_count,
  const zigpc_zcl_frame_data_t *const command_arg_list)
{
  sl_status_t status
    = zigpc_zcl_frame_init_command(frame, command_id, frame_type & 0xFF);
  if (status != SL_STATUS_OK) {
    sl_log_error(LOG_TAG,
                 "Failed to initialize ZCL Frame for cluster[0x%04X] "
                 "command[0x%02X]: 0x%X",
                 cluster_id,
                 command_id,
                 status);
  }

  for (size_t cmd_i = 0;
       (status == SL_STATUS_OK) && (cmd_i < command_arg_count);
       cmd_i++) {
    zigpc_zcl_data_type_t arg_type = command_arg_list[cmd_i].type;
    const void *arg_data           = command_arg_list[cmd_i].data;

    status = zigpc_zcl_add_command_frame_arg(frame, arg_type, arg_data);

    if (status != SL_STATUS_OK) {
      sl_log_error(LOG_TAG,
                   "Failed to add cluster[0x%04X] command[0x%02X] arg[%u] to "
                   "ZCL frame: 0x%X",
                   cluster_id,
                   command_id,
                   cmd_i,
                   status);
    }
  }

  return status;
}

size_t zigpc_zcl_get_type_size(zigpc_zcl_data_type_t type)
{
  size_t size = 0;

  const std::unordered_map<zigpc_zcl_data_type_t, size_t> type_size_map
    = *(zigpc_zcl_get_zcl_data_type_size_map());

  auto size_index = type_size_map.find(type);
  if (size_index != type_size_map.end()) {
    size = size_index->second;
  } else {
    size = 0;
  }

  return size;
}
