#include "platform_date_time.h"
#include <time.h>

date_time_t platform_get_date_time()
{
  date_time_t return_time = {0};
  time_t current_time     = {0};
  struct tm local_time    = {0};

  time(&current_time);
  if (!localtime_r(&current_time, &local_time))
    return return_time;

  return_time.sec  = local_time.tm_sec;
  return_time.min  = local_time.tm_min;
  return_time.hour = local_time.tm_hour;
  return_time.day  = local_time.tm_mday;
  return_time.mon  = local_time.tm_mon;
  return_time.year = local_time.tm_year;
  return return_time;
}
