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

/**
 * @file ota_time.h
 * @brief OTA time interface for C level.
 * @{
 */

#ifndef OTA_TIME_H
#define OTA_TIME_H

#include "time.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate time diffrence between the local time and UTC time.
 *
 * @param time_utc time_t variable time in seconds.
 *
 * @returns time_t indicating the difference in seconds since the epoch began (01/01/1970)
 *          Positive value indicates that we are ahead the UTC time
 *          Negative value indicates that we are behind the UTC time
 */
time_t calculate_utc_current_time_difference(const time_t time_utc);

/**
 * @brief Returns the current UTC time.
 *
 * @returns time_t value indicating the number of seconds since the epoch began (01/01/1970)
 */
time_t get_current_utc_current_time();

#ifdef __cplusplus
}
#endif

#endif  //OTA_TIME_H
/** @} end ota_time */
