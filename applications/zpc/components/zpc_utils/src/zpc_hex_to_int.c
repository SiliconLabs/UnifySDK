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
#include "sl_log.h"
#define LOG_TAG "zpc_hex2int"

#include "zpc_hex_to_int.h"
int zpc_hex2int(char c) {
  if(c >= '0' && c <= '9') {
    return c-'0';
  } else if(c >= 'a' && c <= 'f') {
    return c-'a' + 0xa;
  } else if(c >= 'A' && c <= 'F') {
    return c-'A' + 0xa;
  } else {
    sl_log_error(LOG_TAG, "Trying to convert non hex character to hexi\n");
    return 0;
  }
}


