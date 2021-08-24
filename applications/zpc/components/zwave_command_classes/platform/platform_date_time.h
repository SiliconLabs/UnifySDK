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

#ifndef PLATFORM_DATE_TIME_H
#define PLATFORM_DATE_TIME_H

typedef struct {
  int sec;  /* seconds after the minute [0-60] */
  int min;  /* minutes after the hour [0-59] */
  int hour; /* hours since midnight [0-23] */
  int day;  /* day of the month [1-31] */
  int mon;  /* months since January [0-11] */
  int year; /* years since 1900 */
} date_time_t;

/**
 * @brief platform abstraction for retrieving local system time.
 */
date_time_t platform_get_date_time();

#endif
