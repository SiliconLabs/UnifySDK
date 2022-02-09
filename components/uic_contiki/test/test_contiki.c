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

#include "unity.h"
#include "contiki_test_helper.h"
#include "sys/process.h"
#include "sys/etimer.h"
#include "sl_log.h"
#include <stdlib.h>
#include <string.h>
#define LOG_TAG "test_contiki"
PROCESS(contiki_test_process, "Test Contiki Process");
struct etimer test_timer;

static int process_begin_num_calls  = 0;
static int process_init_num_calls   = 0;
static int process_exit_num_calls   = 0;
static int process_test_timer_calls = 0;
static int process_test_event_calls = 0;

/**
 * @brief Test event definitions
 */
typedef enum {
  /// Send the next message in the TX Queue.
  SLOW_EVENT,
  /// The ongoing transmission is now completed.
  VERY_SLOW_EVENT,
} test_events_t;

void setUp()
{
  contiki_test_helper_init();

  process_begin_num_calls  = 0;
  process_init_num_calls   = 0;
  process_exit_num_calls   = 0;
  process_test_timer_calls = 0;
  process_test_event_calls = 0;
}

void test_contiki_max_events()
{
  for (int i = 0; i < PROCESS_CONF_NUMEVENTS; i++) {
    process_post(&contiki_test_process, i,NULL );
  }
  TEST_ASSERT_EQUAL(PROCESS_ERR_FULL, process_post(&contiki_test_process, 33, NULL));
}


void test_contiki_events_data() {
    process_test_event_calls =0 ;

    process_start(&contiki_test_process, NULL);
    contiki_test_helper_run(0);

    int i;
    for(i=0; i < 1000; ) {

      for(int j=0; j < (rand() & 0xf); j++ ) {
        i++;
        char* buf = malloc(64);
        memset(buf,'0' + (i & 0xf),64);
        TEST_ASSERT_EQUAL(PROCESS_ERR_OK, process_post(&contiki_test_process, 42, buf) );
      }
      contiki_test_helper_run(1);
    }
    TEST_ASSERT_EQUAL( i,process_test_event_calls );
}

void test_contiki_process()
{
  // Test init and begin
  process_start(&contiki_test_process, NULL);
  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL_MESSAGE(1,
                            process_begin_num_calls,
                            "contiki_test_process begin should be called once");
  TEST_ASSERT_EQUAL_MESSAGE(1,
                            process_init_num_calls,
                            "contiki_test_process init should be called once");
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_MESSAGE(
    1,
    process_init_num_calls,
    "contiki_test_process init should only be called once");
  TEST_ASSERT_EQUAL_MESSAGE(
    1,
    process_begin_num_calls,
    "contiki_test_process begin should only be called once");

  // Timer test
  TEST_ASSERT_EQUAL_MESSAGE(0,
                            process_test_timer_calls,
                            "test_timer should not have timed out yet");
  contiki_test_helper_run(1000);
  TEST_ASSERT_EQUAL_MESSAGE(1,
                            process_test_timer_calls,
                            "test_timer should have timed out now");
  contiki_test_helper_run(1000);
  TEST_ASSERT_EQUAL_MESSAGE(2,
                            process_test_timer_calls,
                            "test_timer should have timed twice now");

  // Test exit
  process_exit(&contiki_test_process);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL_MESSAGE(
    1,
    process_exit_num_calls,
    "contiki_test_process exit should only be called once");
}

void test_contiki_slow_events()
{
  process_start(&contiki_test_process, NULL);
  contiki_test_helper_run(0);

  // Nothings happens here really, apart from warnings/errors on the log.
  process_post(&contiki_test_process, SLOW_EVENT, NULL);
  contiki_test_helper_run(0);
  process_post(&contiki_test_process, VERY_SLOW_EVENT, NULL);
  contiki_test_helper_run(0);

  process_exit(&contiki_test_process);
  contiki_test_helper_run(0);
}

PROCESS_THREAD(contiki_test_process, ev, data)
{
  PROCESS_BEGIN()
  process_begin_num_calls++;
  sl_log_debug(LOG_TAG, "begin called");
  etimer_set(&test_timer, 1 * CLOCK_SECOND);

  while (1) {
    if (ev == PROCESS_EVENT_INIT) {
      sl_log_debug(LOG_TAG, "init called");
      process_init_num_calls++;
    } else if (ev == PROCESS_EVENT_TIMER) {
      TEST_ASSERT_EQUAL_PTR_MESSAGE(data,
                                    &test_timer,
                                    "Timer shall be the test_timer timer");
      if (data == &test_timer) {
        sl_log_debug(LOG_TAG, "test_timer timeout");
        process_test_timer_calls++;
        etimer_restart(&test_timer);
      }

      /* If the test case uses another etimer than test_timer, timeout are handled here */
    } else if (ev == PROCESS_EVENT_EXIT) {
      sl_log_debug(LOG_TAG, "exit called");
      process_exit_num_calls++;
    } else if (ev == SLOW_EVENT) {
      contiki_test_helper_increase_clock(101);
    } else if (ev == VERY_SLOW_EVENT) {
      contiki_test_helper_increase_clock(301);
    } else if (ev == 42) {
      process_test_event_calls++;
      char* buf = (char*)data;
      for(int i=0; i < 64; i++) {
        if( buf[i] != buf[0] ) {
          sl_log_debug(LOG_TAG, "There is a problem at event number %i ",process_test_event_calls);
          break;
        }
      }
      free(data);
    }
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}
