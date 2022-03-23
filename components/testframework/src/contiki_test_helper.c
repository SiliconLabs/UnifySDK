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
#include "sys/process.h"
#include "sys/clock.h"
#include "sys/etimer.h"

#include "sys/ctimer.h"

#include "sys/procinit.h"
#include "contiki_test_helper.h"

static clock_time_t test_time = 0;

clock_time_t clock_time(void)
{
  return test_time;
}

unsigned long clock_seconds(void)
{
  return clock_time() / 1000;
}

PROCESS(test_process, "Test Process");

PROCESS_THREAD(test_process, ev, data)
{
  PROCESS_BEGIN();
  while (1) {
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END();
}

void contiki_test_helper_increase_clock(clock_time_t ms)
{
  test_time += ms;
}

void contiki_test_helper_run_once(clock_time_t ms)
{
  PROCESS_CURRENT() = NULL;

  test_time += ms;
  etimer_request_poll();

  process_run();
  PROCESS_CURRENT() = &test_process;
}

void contiki_test_helper_run(clock_time_t ms)
{
  PROCESS_CURRENT() = NULL;

  test_time += ms;
  etimer_request_poll();

  while (process_run()) {
  }
  PROCESS_CURRENT() = &test_process;
}

void contiki_test_helper_run_clock_increases_for_each_event(clock_time_t ms)
{
  PROCESS_CURRENT() = NULL;
  etimer_request_poll();

  while (process_run()) {
    test_time += ms;
  }
  PROCESS_CURRENT() = &test_process;
}

void contiki_test_helper_init()
{
  test_time = 0;
  process_init();
  ctimer_init();
  process_start(&etimer_process, 0);
  process_start(&test_process, 0);
  contiki_test_helper_run(0);
}
