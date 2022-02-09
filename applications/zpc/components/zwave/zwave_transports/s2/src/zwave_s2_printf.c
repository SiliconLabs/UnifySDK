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
#include "S2.h"
#include<stdio.h>
#include <stdarg.h>
#include "sl_log.h"

#define LOG_TAG "zwave_s2_printf"

void S2_dbg_printf(const char *format, ...)
{
  char buffer[256];
  va_list args;
  va_start (args, format);
  vsnprintf (buffer,sizeof(buffer),format, args);
  sl_log_debug(LOG_TAG, "%s",buffer);
  va_end (args);
}

void S2_err_printf(const char *format, ...)
{
  char buffer[256];
  va_list args;
  va_start (args, format);
  vsnprintf (buffer,sizeof(buffer),format, args);
  sl_log_error(LOG_TAG, "%s",buffer);
  va_end (args);
}
