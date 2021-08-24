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

/**
 * @file zwapi_timestamp_platform.h
 * @brief Platform specific zwapi timestamp definitions
 *
 */

#ifndef ZWAPI_TIMER_PLATFORM_H
#define ZWAPI_TIMER_PLATFORM_H
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zwapi_timestamp {
  struct timespec tm;
};

#ifdef __cplusplus
}
#endif

#endif  // ZWAPI_TIMER_PLATFORM_H