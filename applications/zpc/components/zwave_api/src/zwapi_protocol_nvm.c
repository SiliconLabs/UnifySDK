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

#include "zwapi_protocol_nvm.h"
#include "zwapi_func_ids.h"
#include "zwapi_session.h"
#include "zwapi_init.h"
#include "zwapi_internal.h"

uint8_t zwapi_nvm_close(void)
{
  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  request_buffer[index++]                     = ZWAVE_API_NVM_CLOSE_OPERATION;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_NVM_BACKUP_RESTORE,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > IDX_DATA) {
    return response_buffer[IDX_DATA];
  }
  // Error return value
  return ZWAVE_API_NVM_RETURN_CODE_ERROR;
}

uint32_t zwapi_nvm_open(void)
{
  uint32_t nvm_length     = 0;
  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};
  request_buffer[index++]                     = ZWAVE_API_NVM_OPEN_OPERATION;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_NVM_BACKUP_RESTORE,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  if (send_command_status == SL_STATUS_OK && response_length > (IDX_DATA + 3)) {
    if (response_buffer[IDX_DATA] == ZWAVE_API_NVM_RETURN_CODE_OK) {
      nvm_length = response_buffer[IDX_DATA + 2] << 8;
      nvm_length = nvm_length | response_buffer[IDX_DATA + 3];
      if (nvm_length != 0) {
        /* FIXME: 700 series return 1 more byte than the size of the NVM, it seems */
        nvm_length += 1;
      }
    }
  }
  return nvm_length;
}

zwapi_protocol_nvm_status_t zwapi_nvm_read(uint32_t offset,
                                           uint8_t *buf,
                                           uint8_t length,
                                           uint8_t *length_read)
{
  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};

  request_buffer[index++] = ZWAVE_API_NVM_READ_OPERATION;
  request_buffer[index++] = length;
  request_buffer[index++] = (offset) >> 8;
  request_buffer[index++] = (offset)&0xFF;

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_NVM_BACKUP_RESTORE,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  *length_read = 0;
  if (send_command_status == SL_STATUS_OK && response_length > (IDX_DATA + 1)) {
    *length_read = response_buffer[IDX_DATA + 1];

    for (uint8_t i = 0; i < *length_read; i++) {
      if (i < length && (IDX_DATA + 4 + i) < response_length) {
        buf[i] = response_buffer[IDX_DATA + 4 + i];
      } else {
        return ZWAPI_PROTOCOL_NVM_STATUS_ERROR;
      }
    }

    switch (response_buffer[IDX_DATA]) {
      case ZWAVE_API_NVM_RETURN_CODE_OK:
        return ZWAPI_PROTOCOL_NVM_STATUS_OK;

      case ZWAVE_API_NVM_RETURN_CODE_END_OF_FILE:
        return ZWAPI_PROTOCOL_NVM_STATUS_EOF;
    }
  }
  return ZWAPI_PROTOCOL_NVM_STATUS_ERROR;
}

zwapi_protocol_nvm_status_t zwapi_nvm_write(uint32_t offset,
                                            const uint8_t *buf,
                                            uint8_t length,
                                            uint8_t *length_written)
{
  uint8_t response_length = 0, index = 0;
  uint8_t request_buffer[REQUEST_BUFFER_SIZE] = {0},
          response_buffer[FRAME_LENGTH_MAX]   = {0};

  request_buffer[index++] = ZWAVE_API_NVM_WRITE_OPERATION;
  request_buffer[index++] = length;
  request_buffer[index++] = (offset) >> 8;
  request_buffer[index++] = (offset)&0xFF;

  for (uint8_t i = 0; i < length; i++) {
    request_buffer[index++] = buf[i];
  }

  sl_status_t send_command_status
    = zwapi_send_command_with_response(FUNC_ID_NVM_BACKUP_RESTORE,
                                       request_buffer,
                                       index,
                                       response_buffer,
                                       &response_length);

  *length_written = 0;
  if (send_command_status == SL_STATUS_OK && response_length > (IDX_DATA + 1)) {
    *length_written = response_buffer[IDX_DATA + 1];

    switch (response_buffer[IDX_DATA]) {
      case ZWAVE_API_NVM_RETURN_CODE_OK:
        return ZWAPI_PROTOCOL_NVM_STATUS_OK;

      case ZWAVE_API_NVM_RETURN_CODE_END_OF_FILE:
        return ZWAPI_PROTOCOL_NVM_STATUS_EOF;
    }
  }

  return ZWAPI_PROTOCOL_NVM_STATUS_ERROR;
}
