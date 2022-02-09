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

#include "zpc_converters.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

sl_status_t zpc_converters_dsk_str_to_internal(const char *src,
                                               zwave_dsk_t dst)
{
  unsigned int dst_idx = 0;
  const char *idx      = src;
  const char *end_idx  = src + strnlen(src, 100);
  do {
    char *next_idx = NULL;
    if (dst_idx >= ZWAVE_DSK_LENGTH) {
      return SL_STATUS_WOULD_OVERFLOW;
    }
    errno       = 0;
    int num_val = strtol(idx, &next_idx, 10);
    if ((0 == errno) && (num_val >= 0) && (num_val <= 0xFFFF)) {
      dst[dst_idx++] = (uint8_t)((num_val >> 8) & 0xFF);
      dst[dst_idx++] = (uint8_t)(num_val & 0xFF);
      idx            = next_idx + 1;
    } else {
      return SL_STATUS_FAIL;
    }
  } while (idx < end_idx);
  return SL_STATUS_OK;
}

sl_status_t zpc_converters_dsk_to_str(const zwave_dsk_t src,
                                      char *dst,
                                      size_t dst_max_len)
{
  if (dst_max_len < DSK_STR_LEN) {
    return SL_STATUS_WOULD_OVERFLOW;
  }
  size_t index = 0;
  for (int i = 0; i < sizeof(zwave_dsk_t); i += 2) {
    int d = (src[i] << 8) | src[i + 1];
    index += snprintf(&dst[index], dst_max_len - index, "%05i-", d);
  }
  // Erase the last "-"
  if (index > 0) {
    dst[index - 1] = '\0';
  }
  return SL_STATUS_OK;
}
