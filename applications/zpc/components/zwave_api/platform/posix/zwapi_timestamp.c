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

#include <stdlib.h>
#include <time.h>
#include "zwapi_timestamp.h"
#include <assert.h>
#include <stdbool.h>
#include "sl_log.h"


void zwapi_timestamp_get(zwapi_timestamp_t *timestamp, int period_ms)
{
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  timestamp->tm = now;

  timestamp->tm.tv_sec += (period_ms / 1000);
  timestamp->tm.tv_nsec += ((period_ms % 1000) * 1000000);
}

bool zwapi_is_timestamp_elapsed(const zwapi_timestamp_t *timestamp_1)
{
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);

  if (timestamp_1->tm.tv_sec == now.tv_sec) {
    return timestamp_1->tm.tv_nsec < now.tv_nsec;
  } else {
    return timestamp_1->tm.tv_sec < now.tv_sec;
  }
}
