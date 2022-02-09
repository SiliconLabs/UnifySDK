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

// Generic includes
#include <stddef.h>
#include <string.h>

// Includes from this component
#include "zwapi_protocol_mem.h"
#include "zwapi_func_ids.h"
#include "zwapi_protocol_controller.h"  // For ZW_Default()
#include "zwapi_timestamp.h"
#include "zwapi_init.h"
#include "zwapi_session.h"
#include "zwapi_internal.h"
#include "zwapi_utils.h"

// Includes from other components
#include "sl_log.h"
#include "zpc_endian.h"

static volatile bool zwapi_memory_put_buffer_builtin_callback_called = false;

#define LOG_TAG "zwapi_protocol_mem"

sl_status_t
  zwapi_nvr_get_value(uint8_t offset, uint8_t bLength, uint8_t *pNVRValue)
{
  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  request_buffer[index++]                     = offset;
  request_buffer[index++]                     = bLength;
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_NVR_GET_VALUE,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA) {
    for (uint8_t i = 0; i < bLength; i++) {
      if ((IDX_DATA + i) < response_length) {
        pNVRValue[i] = response_buffer[IDX_DATA + i];
      }
    }
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t zwapi_memory_get_ids(zwave_home_id_t *home_id,
                                 zwave_node_id_t *node_id)
{
  uint8_t response_length                   = 0;
  uint8_t response_buffer[FRAME_LENGTH_MAX] = {0};
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_MEMORY_GET_ID,
                                       NULL,
                                       0,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > (IDX_DATA + 4)) {
    /* On the Z-Wave module the home id is stored as big endian. */
    zwave_home_id_t received_home_id = 0;
    memcpy(&received_home_id,
           &response_buffer[IDX_DATA],
           sizeof(received_home_id));

    received_home_id = zpc_ntohl(received_home_id);  // swap to our endian
    *home_id         = received_home_id;

    uint8_t index = IDX_DATA + 4;
    *node_id = zwapi_read_node_id(response_buffer, &index);

    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

sl_status_t zwapi_memory_get_byte(uint16_t offset, uint8_t *byte_value)
{
  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  request_buffer[index++]                     = (offset) >> 8;
  request_buffer[index++]                     = (offset)&0xFF;
  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_MEMORY_GET_BYTE,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA) {
    *byte_value = response_buffer[IDX_DATA];
    return SL_STATUS_OK;
  }

  return SL_STATUS_FAIL;
}

uint8_t zwapi_memory_put_byte(uint16_t offset, uint8_t bData)
{
  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  request_buffer[index++]                     = (offset) >> 8;
  request_buffer[index++]                     = (offset)&0xFF;
  request_buffer[index++]                     = bData;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_MEMORY_PUT_BYTE,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA) {
    return response_buffer[IDX_DATA];
  }

  return 0;
}

sl_status_t
  zwapi_memory_get_buffer(uint16_t offset, uint8_t *buf, uint8_t length)
{
  if (zwapi_support_command_func(FUNC_ID_MEMORY_GET_BUFFER)) {
    uint8_t response_length = 0, index = 0;
    uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
            response_buffer[FRAME_LENGTH_MAX]   = {0};
    request_buffer[index++]                     = (offset) >> 8;
    request_buffer[index++]                     = (offset)&0xFF;
    request_buffer[index++]                     = (uint8_t)length;
    // We already know that the command is supported here, so we call
    // zwapi_session_send_frame_with_response() instead of zwapi_send_command_with_response()
    sl_status_t send_frame_status
      = zwapi_session_send_frame_with_response(FUNC_ID_MEMORY_GET_BUFFER,
                                               request_buffer,
                                               index,
                                               response_buffer,
                                               &response_length);
    if (send_frame_status != SL_STATUS_OK) {
      return SL_STATUS_FAIL;
    }
    for (uint8_t i = 0; i < length; i++) {
      if ((IDX_DATA + i) < response_length) {
        buf[i] = response_buffer[IDX_DATA + i];
      }
    }
    return SL_STATUS_OK;

  } else {
    for (uint8_t i = 0; i < length; i++) {
      sl_status_t byte_copy_status = zwapi_memory_get_byte(offset + i, &buf[i]);
      if (byte_copy_status != SL_STATUS_OK) {
        return SL_STATUS_FAIL;
      }
    }
    return SL_STATUS_OK;
  }
}

uint8_t zwapi_memory_put_buffer(uint16_t offset,
                                const uint8_t *buf,
                                uint16_t length,
                                void (*callback_function)(void))
{
  bool function_buffer_too_small = false;
  if (length > FRAME_LENGTH_MAX - 8) {
    function_buffer_too_small = true;
  }
  // Copy using a single API call with the buffer if possible
  if (zwapi_support_command_func(FUNC_ID_MEMORY_PUT_BUFFER)
      && !function_buffer_too_small) {
    uint8_t func_id, response_length = 0, index = 0;
    uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
            response_buffer[FRAME_LENGTH_MAX]   = {0};

    func_id                 = (callback_function == NULL
                                 ? 0
                                 : 3);  //FIXME: WHy 3 here ? I think it should only be non-zero
    request_buffer[index++] = (offset) >> 8;
    request_buffer[index++] = (offset)&0xFF;
    // CLEAN-UP: Not quite sure why length here should be 2 bytes, given that
    // the Z-Wave Module frames are defined with a frame length of up to 255.
    request_buffer[index++] = (length) >> 8;
    request_buffer[index++] = (length)&0xFF;

    for (uint16_t i = 0; i < length; i++) {
      request_buffer[index++] = buf[i];
    }
    request_buffer[index++]          = func_id;
    zwapi_memory_put_buffer_callback = callback_function;
    // We already know that the command is supported here, so we call
    // zwapi_session_send_frame_with_response() instead of zwapi_send_command_with_response()
    sl_status_t send_frame_status
      = zwapi_session_send_frame_with_response(FUNC_ID_MEMORY_PUT_BUFFER,
                                               request_buffer,
                                               index,
                                               response_buffer,
                                               &response_length);

    if (send_frame_status == SL_STATUS_FAIL || response_length <= IDX_DATA) {
      return 0;
    }
    return response_buffer[IDX_DATA];

  } else {  // Else resort to copy byte-by-byte
    for (uint16_t i = 0; i < length; i++) {
      uint8_t status = zwapi_memory_put_byte(offset + i, buf[i]);
      if (!status) {
        return 0;
      }
    }
    // Make a manual callback, since zwapi_memory_put_byte() won't callback
    if (callback_function) {
      callback_function();
    }
    return length + 1;
  }
}

static void zwapi_memory_put_buffer_builtin_callback()
{
  zwapi_memory_put_buffer_builtin_callback_called = true;
}

uint8_t zwapi_memory_put_buffer_blocking(uint16_t offset,
                                         uint8_t *buffer,
                                         uint16_t length,
                                         uint16_t timeout)
{
  zwapi_timestamp_t callback_timeout;
  zwapi_timestamp_get(&callback_timeout, timeout);
  zwapi_memory_put_buffer_builtin_callback_called = false;

  uint8_t return_value
    = zwapi_memory_put_buffer(offset,
                              buffer,
                              length,
                              zwapi_memory_put_buffer_builtin_callback);

  while (!zwapi_is_timestamp_elapsed(&callback_timeout)
         && zwapi_memory_put_buffer_builtin_callback_called == false) {
  }

  if (zwapi_memory_put_buffer_builtin_callback_called) {
    return return_value;
  }

  return 0;
}
