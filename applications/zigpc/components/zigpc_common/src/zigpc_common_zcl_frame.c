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

#include <string.h>

// Unify Shared components
#include "sl_status.h"

#include "zigpc_common_zigbee.h"

#define ZCL_FRAME_SEQUENCE_NUMBER_PLACEHOLDER 0x00

sl_status_t zigpc_common_zcl_frame_init_command(zcl_frame_t *const frame,
                                                zcl_command_id_t command_id,
                                                uint8_t frame_control)
{
  sl_status_t status = SL_STATUS_OK;

  if (frame == NULL) {
    status = SL_STATUS_NULL_POINTER;
  }

  if (status == SL_STATUS_OK) {
    frame->size = 0;

    frame->buffer[frame->size++] = frame_control;
    frame->buffer[frame->size]   = ZCL_FRAME_SEQUENCE_NUMBER_PLACEHOLDER;

    // Store sequence ID offset to be passed into ZigbeeHost
    frame->offset_sequence_id = frame->size++;

    frame->buffer[frame->size++] = command_id;
  }

  return status;
}

sl_status_t zigpc_common_zcl_frame_fill_data(zcl_frame_t *const frame,
                                             enum zcl_frame_data_type type,
                                             const void *data)
{
  sl_status_t status = SL_STATUS_OK;

  if ((frame == NULL) || (data == NULL)) {
    status = SL_STATUS_NULL_POINTER;
  }

  if ((status == SL_STATUS_OK)
      && ((type == ZCL_DATA_TYPE_UNKNOWN) || (type >= ZCL_DATA_TYPE_MAX_VAL))) {
    status = SL_STATUS_INVALID_TYPE;
  }

  if (status == SL_STATUS_OK) {
    switch (type) {
      case ZCL_DATA_TYPE_BOOL:
        memcpy(&frame->buffer[frame->size], data, 1);
        frame->buffer[frame->size] &= 1;
        frame->size += 1;
        break;
      case ZCL_DATA_TYPE_MAP8:
      case ZCL_DATA_TYPE_UINT8:
      case ZCL_DATA_TYPE_INT8:
      case ZCL_DATA_TYPE_ENUM8:
        memcpy(&frame->buffer[frame->size], data, 1);
        frame->size += 1;
        break;
      case ZCL_DATA_TYPE_UINT16:
      case ZCL_DATA_TYPE_INT16:
      case ZCL_DATA_TYPE_ENUM16:
        memcpy(&frame->buffer[frame->size], data, 2);
        frame->size += 2;
        break;
      case ZCL_DATA_TYPE_UINT32:
      case ZCL_DATA_TYPE_INT32:
        memcpy(&frame->buffer[frame->size], data, 4);
        frame->size += 4;
        break;
      default:
        status = SL_STATUS_INVALID_TYPE;
    }
  }

  return status;
}

sl_status_t zigpc_common_zcl_frame_fill_raw(zcl_frame_t *const frame,
                                            const uint8_t *const data,
                                            unsigned int data_size)
{
  sl_status_t status = SL_STATUS_OK;
  if (frame == NULL || data == NULL) {
    status = SL_STATUS_NULL_POINTER;
  }

  if (status == SL_STATUS_OK) {
    if ((frame->size + data_size) < ZCL_FRAME_BUFFER_SIZE_MAX) {
      memcpy(&frame->buffer[frame->size], data, data_size);
      frame->size = frame->size + data_size;
    } else {
      status = SL_STATUS_WOULD_OVERFLOW;
    }
  }

  return status;
}
