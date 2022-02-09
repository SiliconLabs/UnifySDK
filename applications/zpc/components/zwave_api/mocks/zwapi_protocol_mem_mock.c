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

#include "zwapi_protocol_mem.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef ZWAPI_APPLICATION_NVM_MOCK_FILE
#define ZWAPI_APPLICATION_NVM_MOCK_FILE "zwapi_protocol_mem.dat"
#endif

/// Use weak attribute on all functions in mock,
/// to allow to override funtions in tests
#define WEAK_ATTRIBUTE __attribute__((weak))

WEAK_ATTRIBUTE int zwapi_memory_get_open_and_seek(unsigned int offset)
{
  int nvm_file = open(ZWAPI_APPLICATION_NVM_MOCK_FILE, O_RDWR | O_CREAT, 0644);

  if (nvm_file < 0) {
    fprintf(stderr,
            "Error opening NVM file %s\n",
            ZWAPI_APPLICATION_NVM_MOCK_FILE);
    perror("");
    exit(1);
  }
  if (lseek(nvm_file, offset, SEEK_SET) < 0) {
    fprintf(stderr,
            "Error seeeking in NVM file %s\n",
            ZWAPI_APPLICATION_NVM_MOCK_FILE);
    perror("");
    exit(1);
  }
  return nvm_file;
}

WEAK_ATTRIBUTE sl_status_t zwapi_nvr_get_value(uint8_t bOffset,
                                               uint8_t bLength,
                                               uint8_t *bRetBuffer)
{
  return SL_STATUS_FAIL;
}

WEAK_ATTRIBUTE sl_status_t zwapi_memory_get_ids(zwave_home_id_t *home_id,
                                                zwave_node_id_t *node_id)
{
  return SL_STATUS_FAIL;
}

WEAK_ATTRIBUTE sl_status_t zwapi_memory_get_byte(uint16_t offset,
                                                 uint8_t *byte_value)
{
  return zwapi_memory_get_buffer(offset, byte_value, 1);
}

WEAK_ATTRIBUTE uint8_t zwapi_memory_put_byte(uint16_t offset, uint8_t bData)
{
  return zwapi_memory_put_buffer(offset, &bData, 1, NULL);
}

WEAK_ATTRIBUTE sl_status_t zwapi_memory_get_buffer(uint16_t offset,
                                                   uint8_t *buffer,
                                                   uint8_t length)
{
  int nvm_file = zwapi_memory_get_open_and_seek(offset);
  if (read(nvm_file, buffer, length) != length) {
    perror(__FUNCTION__);
  }
  close(nvm_file);
  return SL_STATUS_OK;
}

WEAK_ATTRIBUTE uint8_t zwapi_memory_put_buffer(uint16_t offset,
                                               const uint8_t *buffer,
                                               uint16_t length,
                                               void (*callback_function)(void))
{
  int nvm_file = zwapi_memory_get_open_and_seek(offset);
  if (write(nvm_file, buffer, length) != length) {
    perror(__FUNCTION__);
  }
  close(nvm_file);
  if (callback_function) {
    callback_function();
  }
  return length;
}
