#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include "unity.h"
#include "zwapi_timestamp.h"

static zwapi_timestamp_t timer;
static struct timespec time_before, time_after, time_diff;

void test_init()
{
  //set a timestamp with 1000ms in the future
  zwapi_timestamp_get(&timer, 1000);

  TEST_ASSERT_FALSE_MESSAGE(zwapi_is_timestamp_elapsed(&timer),
                            " The timer is expired before the interval lapse");

  clock_gettime(CLOCK_MONOTONIC, &time_before);
  while (1) {
    clock_gettime(CLOCK_MONOTONIC, &time_after);
    time_diff.tv_sec = time_after.tv_sec - time_before.tv_sec;
    if (time_diff.tv_sec > 1) {
      break;
    }
  }

  TEST_ASSERT_TRUE_MESSAGE(
    zwapi_is_timestamp_elapsed(&timer),
    " The timer is not expired within defined timeout interval");
}