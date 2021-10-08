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
 * @file ota_time.hpp
 * @brief Time API for OTA POSIX specific.
 *
 * This API allows conversion of date-time strings to UTC time as an absolute time measure.
 *
 * @{
 */

#ifndef OTA_TIME_HPP
#define OTA_TIME_HPP

#include <time.h>
#include <string>

/**
 * @brief Conversion from date string to UTC time in seconds.
 *
 * @param date_time date-time string in ISO8601 RFC 3339 format
 * image.
 *
 * @returns time in seconds for date time in UTC.
 *
 * @throws std::invalid_argument unable to parse time string.
 */
time_t calculate_utc_time_from_rfc_3339(const std::string &date_time);

/**
 * @brief Conversion from time to string to UTC time.
 *
 * @param time_input_utc time_t variable time in seconds.
 *
 * @returns date-time string from time input.
 */
std::string get_utc_time_string_from_utc_time(time_t time_input_utc);

#endif  //OTA_TIME_HPP
/** @} end ota_time */
