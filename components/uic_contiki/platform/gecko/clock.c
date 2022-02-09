/*
 * @file
 *         Clock implementation for FreeRTOS
 * @copyright
 *         Copyright (c) 2021, Silicon Labs., All Rights Reserved
 */

#include "contiki.h"
#include "FreeRTOS.h"
#include "task.h"

clock_time_t clock_time(void)
{
  return (xTaskGetTickCountFromISR() * portTICK_PERIOD_MS);
}

unsigned long clock_seconds(void)
{
  return (( xTaskGetTickCountFromISR() / (TickType_t)(1000) ) * portTICK_PERIOD_MS);
}

int clock_lte(clock_time_t t1, clock_time_t t2)
{
  clock_time_t diff = t1 - t2;
  if (diff == 0) {
    return 1;
  }
  /* If t1 < t2 return true, but if it has crossed the half-space,
   * return t2 < t1.  */
  clock_time_t inverse_diff = t2 - t1;
  return (inverse_diff < diff);
}

/*
 * See function description for more details.
 */
void clock_delay(unsigned int d)
{
  (void)(d);
  return; //vTaskDelay(pdMS_TO_TICKS(d * 1000));
}
