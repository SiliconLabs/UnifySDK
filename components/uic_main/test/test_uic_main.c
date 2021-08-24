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

#include "unity.h"
#include "process.h"
#include "sys/etimer.h"
#include "sl_status.h"
#include "sl_log.h"
#include "uic_component_fixtures_internal.h"
#include "uic_init.h"
#include "uic_main_loop.h"
#include "uic_main_externals.h"

#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>

#define LOG_TAG "test_uic_main"

// Test fixtures
sl_status_t uic_fixt_setup_test();
int uic_fixt_shutdown_test();
static uic_fixt_setup_step_t setup_fixt[]
  = {{uic_fixt_setup_test, "Fixture setup test"}, {NULL, "Terminator"}};
static uic_fixt_shutdown_step_t shutdown_fixt[]
  = {{uic_fixt_shutdown_test, "Fixture shutdown test"}, {NULL, "Terminator"}};
uic_fixt_setup_step_t *uic_fixt_setup_steps       = setup_fixt;
uic_fixt_shutdown_step_t *uic_fixt_shutdown_steps = shutdown_fixt;

static int uic_fixt_setup_test_called    = 0;
static int uic_fixt_shutdown_test_called = 0;
sl_status_t uic_fixt_setup_test()
{
  uic_fixt_setup_test_called++;
  return SL_STATUS_OK;
}

int uic_fixt_shutdown_test()
{
  uic_fixt_shutdown_test_called++;
  return 0;
}

// test process
PROCESS_NAME(uic_main_test_process);
PROCESS(uic_main_test_process, "uic_init_test_process");

static int uic_main_test_process_init_called    = 0;
static int uic_main_test_process_timeout_called = 0;
static int uic_main_test_fd_handler_called      = 0;
struct etimer test_terminate_timer;
struct etimer test_fd_write_timer;

// Pipe filedescripters
static int fildes[2];
#define FILEDES_TEST_STRING "filedes test string"

// Function to test fildescriptor handling
static bool uic_main_test_fd_handler(int fd)
{
  uic_main_test_fd_handler_called++;
  char my_line[512];
  if (read(fildes[0], my_line, sizeof(my_line)) > 0) {
    TEST_ASSERT_EQUAL_STRING_MESSAGE(
      FILEDES_TEST_STRING,
      my_line,
      "Expect the string sent from the first timeout in the test process");
    sl_log_debug(LOG_TAG,
                 "uic_main_test_fd_handler, got message '%s'",
                 my_line);
  } else {
    TEST_ASSERT_TRUE_MESSAGE(false, "Failed to do read");
  }
  return true;
}

sl_status_t uic_main_test_setup_fixt(void)
{
  process_start(&uic_main_test_process, NULL);
  return SL_STATUS_OK;
}

void test_uic_main()
{
  // Test init
  uic_fixt_setup_step_t app_setup_fixt[]
    = {{uic_main_test_setup_fixt, "start uic_main_test_process"}, {NULL, ""}};
  char *argv[] = {"test_uic_main"};
  TEST_ASSERT_TRUE(uic_init(app_setup_fixt, 1, argv, "test version 1.0"));
  TEST_ASSERT_EQUAL(1, uic_fixt_setup_test_called);
  TEST_ASSERT_EQUAL(0, uic_fixt_shutdown_test_called);

  // Create pipe filedescripters
  pipe(fildes);

  uic_main_ext_register_rfd(fildes[0],
                            uic_main_test_fd_handler,
                            &uic_main_test_process);
  // Test main
  TEST_ASSERT_TRUE_MESSAGE(uic_main_loop(),
                           "Expect uic_main_loop to return true");
  TEST_ASSERT_EQUAL_MESSAGE(1,
                            uic_main_test_process_init_called,
                            "Expect uic_main_test_process to be started, "
                            "and init called once");
  TEST_ASSERT_EQUAL_MESSAGE(
    1,
    uic_main_test_process_timeout_called,
    "Expect timeout to occur in the process to stop the test");
  TEST_ASSERT_EQUAL_MESSAGE(1,
                            uic_main_test_fd_handler_called,
                            "Expect fd handler to be called once");
  close(fildes[0]);
  close(fildes[1]);
}

PROCESS_THREAD(uic_main_test_process, ev, data)
{
  PROCESS_BEGIN()
  sl_log_debug(LOG_TAG, "begin called");
  etimer_set(&test_fd_write_timer, 1);
  etimer_set(&test_terminate_timer, 100);
  while (1) {
    if (ev == PROCESS_EVENT_INIT) {
      uic_main_test_process_init_called++;
      sl_log_debug(LOG_TAG, "init called");
    } else if (ev == PROCESS_EVENT_TIMER) {
      if (data == &test_terminate_timer) {
        sl_log_debug(LOG_TAG, "test_terminate_timer timeout");
        uic_main_test_process_timeout_called++;
        process_post(PROCESS_BROADCAST, PROCESS_EVENT_EXIT, 0);
      } else if (data == &test_fd_write_timer) {
        sl_log_debug(LOG_TAG, "test_fd_write_timer timeout");
        TEST_ASSERT_EQUAL_MESSAGE(
          sizeof(FILEDES_TEST_STRING),
          write(fildes[1], FILEDES_TEST_STRING, sizeof(FILEDES_TEST_STRING)),
          "Expect write to return size of string written");
      } else {
        TEST_ASSERT_TRUE_MESSAGE(
          false,
          "Timer shall be either test_terminate_timer or test_fd_write_timer)");
      }

      /* If the test case uses another etimer than test_terminate_timer, timeout are handled here */
    } else if (ev == PROCESS_EVENT_EXIT) {
      sl_log_debug(LOG_TAG, "exit called");
    }
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END()
}
