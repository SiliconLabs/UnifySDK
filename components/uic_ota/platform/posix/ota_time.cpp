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

// This component
#include "ota_time.h"
#include "ota_time.hpp"

// Unify Components
#include "sl_log.h"

// Standard library
#include <time.h>
#include <string>
#include <stdexcept>

constexpr char LOG_TAG[]                  = "uic_ota_time";
constexpr uint8_t time_string_buffer_size = 100;

time_t calculate_utc_time_from_rfc_3339(const std::string &date_time)
{
  struct tm input_time = {0};
  input_time.tm_isdst  = -1;

  //Remove : timezone in RFC 3339 timestamps as strptime parsese timezones pr RFC 822
  std::string date_time_new = date_time;
  if (*(date_time_new.end() - 3) == ':') {
    date_time_new.erase(date_time_new.end() - 3);
  }

  if (strptime(date_time_new.c_str(), "%Y-%m-%dT%T%z", &input_time)
      == nullptr) {
    sl_log_error(LOG_TAG,
                 "Please provide the correct date-time string format: "
                 "2021-07-13T15:13:59+02:00");
    throw std::invalid_argument("Incorrect date time format");
  }

  long input_offset          = input_time.tm_gmtoff;
  time_t time_without_offset = mktime(&input_time);

  return time_without_offset - input_offset;
}

std::string get_utc_time_string_from_utc_time(time_t time_input_utc)
{
  struct tm utc_time = {0};
  // Using localtime else the local offset will be applied automatically and offset a utc time
  localtime_r(&time_input_utc, &utc_time);
  utc_time.tm_gmtoff = 0;
  utc_time.tm_zone   = (char *)"UTC";

  char buffer[time_string_buffer_size];
  strftime(buffer, time_string_buffer_size, "%Y-%m-%dT%T%z", &utc_time);

  std::string time = std::string(buffer);

  // Mac OSX fix hardcoding UTC time offset
  time.replace(time.length() - 3, 1, "0");
  time.replace(time.length() - 2, 1, "0");

  // Insert : for UTC Offset
  time.insert(time.length() - 2, ":");

  return time;
}

time_t calculate_utc_current_time_difference(const time_t time_utc)
{
  time_t current_time;
  current_time = time(NULL);

  tm *tm_gmt              = localtime(&current_time);
  time_t current_utc_time = current_time - tm_gmt->tm_gmtoff;

  return time_utc - current_utc_time;
}

time_t get_current_utc_current_time()
{
  time_t current_time;
  current_time            = time(NULL);
  tm *tm_gmt              = localtime(&current_time);
  time_t current_utc_time = current_time - tm_gmt->tm_gmtoff;

  return current_utc_time;
}