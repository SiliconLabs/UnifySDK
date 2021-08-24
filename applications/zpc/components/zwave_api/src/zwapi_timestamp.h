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
 * @file zwapi_timestamp.h
 * @brief Timestamp API allowing to set timestamps and verify if they are 
 * past/expired
 *
 */

#ifndef ZWAPI_TIMER_H
#define ZWAPI_TIMER_H

#include <stdbool.h>
#include "zwapi_timestamp_platform.h"  // Include platform specific timestamp definition
#ifdef __cplusplus
extern "C" {
#endif

typedef struct zwapi_timestamp zwapi_timestamp_t;

/**
 * @brief get the timestamp 
 * 
 * @param timestamp 
 * @param period_ms 
 */
void zwapi_timestamp_get(zwapi_timestamp_t *timestamp, int period_ms);

/**
 * @brief compare the current timestamp with the timestamp passed as an argument 
 * 
 * @param timestamp 
 * 
 * @returns true if the timestamp is greater than the current timestamp
 */
bool zwapi_is_timestamp_elapsed(const zwapi_timestamp_t *timestamp);

#ifdef __cplusplus
}
#endif

#endif  // ZWAPI_TIMER_H
