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

#include <time.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <ctimer.h>
#include <stdbool.h>

// Includes from this component
#include "zwave_command_class_indicator.h"
#include "zwave_command_class_test_helper.h"

// Includes from other components
#include "zwave_controller_connection_info.h"

#include "ZW_classcmd.h"

// Test includes
#include "unity.h"

typedef void (*sighandler_t)(int);

/* Capture parameter passed to execve() */
#define MAX_NUM_EXECVE_ARGS (4)
char execve_args[MAX_NUM_EXECVE_ARGS][300] = {{0}};

static bool exit_called = false;

sighandler_t signal(int sig, sighandler_t handler)
{
  return SIG_DFL;
}

pid_t fork(void)
{
  return 0;
}

void platform_exec(const char *file,
                   char *const *args,
                   char *const *environment)
{
  for (int i = 0; i < MAX_NUM_EXECVE_ARGS; i++) {
    strncpy(execve_args[i],
            (args[i] != 0) ? args[i] : "",
            sizeof(execve_args[i]));

    /* If there's a terminating NULL then the buffer was large enough */
    assert(execve_args[i][sizeof(execve_args[i]) - 1] == 0);

    /* Stop after last arg (signified by zero) */
    if (args[i] == 0) {
      break;
    }
  }
}

bool timer_running = 0;

void ctimer_set(struct ctimer *c, clock_time_t t, void (*f)(void *), void *ptr)
{
  timer_running = true;
}

void ctimer_stop(struct ctimer *c)
{
  timer_running = false;
}

void set_indicator_to_default(void)
{
  uint8_t cmd_frame[] = {COMMAND_CLASS_INDICATOR_V3,
                         INDICATOR_SET_V3,
                         0x00,  // Indicator0 value (ignore)
                         0x03,  // Reserved:3 ObjectCount:5
                         INDICATOR_SET_NODE_IDENTIFY_V3,
                         INDICATOR_SET_ON_OFF_PERIOD_V3,
                         0x00,
                         INDICATOR_SET_NODE_IDENTIFY_V3,
                         INDICATOR_SET_ON_OFF_CYCLES_V3,
                         0x00,
                         INDICATOR_SET_NODE_IDENTIFY_V3,
                         INDICATOR_SET_ONE_TIME_ON_OFF_PERIOD_V3,
                         0x00};

  execute_frame_expect_result(zwave_command_class_indicator_support_handler,
                              cmd_frame,
                              sizeof(cmd_frame),
                              SL_STATUS_OK);
}

void suiteSetUp()
{
  exit_called = 0;
  memset(execve_args, 0, sizeof(execve_args));
}

int suiteTearDown(int num_failures)
{
  return num_failures;
}

/**
 * Check the arguments that were passed to the external blink script
 */
static void check_blink_script_args(unsigned int exp_on_time_ms,
                                    unsigned int exp_off_time_ms,
                                    unsigned int exp_num_cycles)
{
  unsigned int arg = 0;

  TEST_ASSERT_MESSAGE(strlen(execve_args[0]) > 0, "scriptname not set!");

  TEST_ASSERT_MESSAGE(execve_args[1] != 0
                        && sscanf(execve_args[1], "%u", &arg) == 1
                        && arg == exp_on_time_ms,
                      "on_time_ms argument for execve");

  TEST_ASSERT_MESSAGE(execve_args[2] != 0
                        && sscanf(execve_args[2], "%u", &arg) == 1
                        && arg == exp_off_time_ms,
                      "off_time_ms argument for execve");

  TEST_ASSERT_MESSAGE(execve_args[3] != 0
                        && sscanf(execve_args[3], "%u", &arg) == 1
                        && arg == exp_num_cycles,
                      "num_cycles argument for execve");
}

void test_zwave_command_class_indicator_multicast_not_allowed()
{
  uint8_t cm_get_V3[] = {COMMAND_CLASS_INDICATOR_V3, INDICATOR_GET_V3};
  assert_dissallow_multicast(zwave_command_class_indicator_support_handler,
                             cm_get_V3);
  uint8_t cm_get_V2[] = {COMMAND_CLASS_INDICATOR_V2, INDICATOR_GET_V2};
  assert_dissallow_multicast(zwave_command_class_indicator_support_handler,
                             cm_get_V2);
  uint8_t cm_get[] = {COMMAND_CLASS_INDICATOR, INDICATOR_GET};
  assert_dissallow_multicast(zwave_command_class_indicator_support_handler,
                             cm_get);
}

void test_zwave_command_class_indicator_get_set_v1(void)
{
  const uint8_t cmd_frame_on[4] = {
    COMMAND_CLASS_INDICATOR_V3,
    INDICATOR_SET_V3,
    0xFF  // Indicator0 value
  };

  execute_frame_expect_result(zwave_command_class_indicator_support_handler,
                              cmd_frame_on,
                              sizeof(cmd_frame_on),
                              SL_STATUS_OK);

  check_blink_script_args(25500, 0, 0);

  const uint8_t cmd_frame_get[]
    = {COMMAND_CLASS_INDICATOR_V3, INDICATOR_GET_V3};
  const uint8_t exp_frame_get_on[] = {
    COMMAND_CLASS_INDICATOR_V3,
    INDICATOR_REPORT,
    0xFF  // Indicator0 value
  };

  execute_frame_expect_frame(zwave_command_class_indicator_support_handler,
                             cmd_frame_get,
                             sizeof(cmd_frame_get),
                             exp_frame_get_on,
                             sizeof(exp_frame_get_on),
                             SL_STATUS_OK);

  const uint8_t cmd_frame_off[] = {
    COMMAND_CLASS_INDICATOR_V3,
    INDICATOR_SET_V3,
    0x00  // Indicator0 value
  };

  execute_frame_expect_result(zwave_command_class_indicator_support_handler,
                              cmd_frame_off,
                              sizeof(cmd_frame_off),
                              SL_STATUS_OK);

  check_blink_script_args(0, 0, 0);

  uint8_t exp_frame_get_off[] = {
    COMMAND_CLASS_INDICATOR_V3,
    INDICATOR_REPORT,
    0x00  // Indicator0 value
  };

  execute_frame_expect_frame(zwave_command_class_indicator_support_handler,
                             cmd_frame_get,
                             sizeof(cmd_frame_get),
                             exp_frame_get_off,
                             sizeof(exp_frame_get_off),
                             SL_STATUS_OK);
}

/**
 * Test the version three set/get commands
 */
void test_zwave_command_class_indicator_get_set_v3(void)
{
  uint8_t cmd_frame_blink[] = {COMMAND_CLASS_INDICATOR_V3,
                               INDICATOR_SET_V3,
                               0x00,  // Indicator0 value (ignore)
                               0x03,  // Reserved:3 ObjectCount:5
                               INDICATOR_SET_NODE_IDENTIFY_V3,
                               INDICATOR_SET_ON_OFF_PERIOD_V3,
                               0x0A,
                               INDICATOR_SET_NODE_IDENTIFY_V3,
                               INDICATOR_SET_ON_OFF_CYCLES_V3,
                               0x02,
                               INDICATOR_SET_NODE_IDENTIFY_V3,
                               INDICATOR_SET_ONE_TIME_ON_OFF_PERIOD_V3,
                               0x00};

  execute_frame_expect_result(zwave_command_class_indicator_support_handler,
                              cmd_frame_blink,
                              sizeof(cmd_frame_blink),
                              SL_STATUS_OK);

  check_blink_script_args(500, 500, 2);

  /*----------------------------------------------------------*/
  uint8_t cmd_frame_get[] = {COMMAND_CLASS_INDICATOR_V3,
                             INDICATOR_GET_V3,
                             INDICATOR_GET_NODE_IDENTIFY_V3};

  uint8_t exp_frame_get_blink[]
    = {COMMAND_CLASS_INDICATOR_V3,
       INDICATOR_REPORT,
       0xFF,                               // Indicator0 value
       0x03,                               // Reserved:3 ObjectCount:5
       INDICATOR_REPORT_NODE_IDENTIFY_V3,  // Indicator ID
       INDICATOR_REPORT_ON_OFF_PERIOD_V3,
       0x0A,
       INDICATOR_REPORT_NODE_IDENTIFY_V3,  // Indicator ID
       INDICATOR_REPORT_ON_OFF_CYCLES_V3,
       0x02,
       INDICATOR_REPORT_NODE_IDENTIFY_V3,  // Indicator ID
       INDICATOR_REPORT_ONE_TIME_ON_OFF_PERIOD_V3,
       0x00};
  execute_frame_expect_frame(zwave_command_class_indicator_support_handler,
                             cmd_frame_get,
                             sizeof(cmd_frame_get),
                             exp_frame_get_blink,
                             sizeof(exp_frame_get_blink),
                             SL_STATUS_OK);

  /*----------------------------------------------------------*/
  uint8_t cmd_frame_err0[] = {COMMAND_CLASS_INDICATOR_V3,
                              0xEE,  // Unknown command
                              INDICATOR_SET_NODE_IDENTIFY_V3};

  execute_frame_expect_result(zwave_command_class_indicator_support_handler,
                              cmd_frame_err0,
                              sizeof(cmd_frame_err0),
                              SL_STATUS_NOT_SUPPORTED);

  /*----------------------------------------------------------*/
  uint8_t cmd_frame_err1[] = {
    COMMAND_CLASS_INDICATOR_V3,
    INDICATOR_GET_V3,
    0x01  // Unknown indicator ID
  };

  uint8_t exp_frame_err1[] = {COMMAND_CLASS_INDICATOR_V3,
                              INDICATOR_REPORT,
                              0xFF,  // Indicator0 value
                              0x01,  // Reserved:3 ObjectCount:5
                              INDICATOR_REPORT_NA_V3,  // Dummy indicator ID
                              0x00,
                              0x00};

  execute_frame_expect_frame(zwave_command_class_indicator_support_handler,
                             cmd_frame_err1,
                             sizeof(cmd_frame_err1),
                             exp_frame_err1,
                             sizeof(exp_frame_err1),
                             SL_STATUS_OK);

  /*----------------------------------------------------------*/
  uint8_t cmd_frame_err2[] = {
    COMMAND_CLASS_INDICATOR_V3,
    INDICATOR_SET_V3,
    0x00,
    0x03,  // Reserved:3 ObjectCount:5
    INDICATOR_SET_NODE_IDENTIFY_V3,
    INDICATOR_SET_ON_OFF_PERIOD_V3
    // Truncated frame!!
  };

  execute_frame_expect_result(zwave_command_class_indicator_support_handler,
                              cmd_frame_err2,
                              sizeof(cmd_frame_err2),
                              SL_STATUS_FAIL);
}

void test_zwave_command_class_indicator_supported_get_v3(void)
{
  /*----------------------------------------------------------*/
  uint8_t cmd_frame_00[] = {
    COMMAND_CLASS_INDICATOR_V3,
    INDICATOR_SUPPORTED_GET_V3,
    0x00  // Indicator ID  0x00 = discovery
  };

  uint8_t exp_frame_00[]
    = {COMMAND_CLASS_INDICATOR_V3,
       INDICATOR_SUPPORTED_REPORT_V3,
       INDICATOR_SUPPORTED_GET_NODE_IDENTIFY_V3,   // Indicator ID
       0x00,                                       // Next Indicator ID
       0x01,                                       // Reserved:3 BitmaskLen:5
       (1 << INDICATOR_REPORT_ON_OFF_PERIOD_V3) |  // Bitmask
         (1 << INDICATOR_REPORT_ON_OFF_CYCLES_V3)
         | (1 << INDICATOR_REPORT_ONE_TIME_ON_OFF_PERIOD_V3)};

  execute_frame_expect_frame(zwave_command_class_indicator_support_handler,
                             cmd_frame_00,
                             sizeof(cmd_frame_00),
                             exp_frame_00,
                             sizeof(exp_frame_00),
                             SL_STATUS_OK);

  /*----------------------------------------------------------*/
  uint8_t cmd_frame_51[] = {
    COMMAND_CLASS_INDICATOR_V3,
    INDICATOR_SUPPORTED_GET_V3,
    INDICATOR_SUPPORTED_GET_NODE_IDENTIFY_V3  // Indicator ID
  };

  uint8_t exp_frame_51[]
    = {COMMAND_CLASS_INDICATOR_V3,
       INDICATOR_SUPPORTED_REPORT_V3,
       INDICATOR_SUPPORTED_GET_NODE_IDENTIFY_V3,   // Indicator ID
       0x00,                                       // Next Indicator ID
       0x01,                                       // Reserved:3 BitmaskLen:5
       (1 << INDICATOR_REPORT_ON_OFF_PERIOD_V3) |  // Bitmask
         (1 << INDICATOR_REPORT_ON_OFF_CYCLES_V3)
         | (1 << INDICATOR_REPORT_ONE_TIME_ON_OFF_PERIOD_V3)};

  execute_frame_expect_frame(zwave_command_class_indicator_support_handler,
                             cmd_frame_51,
                             sizeof(cmd_frame_51),
                             exp_frame_51,
                             sizeof(exp_frame_51),
                             SL_STATUS_OK);

  /*----------------------------------------------------------*/
  uint8_t cmd_frame_nn[] = {
    COMMAND_CLASS_INDICATOR_V3,
    INDICATOR_SUPPORTED_GET_V3,
    0x01  // Indicator ID (unknown)
  };

  uint8_t exp_frame_nn[] = {
    COMMAND_CLASS_INDICATOR_V3,
    INDICATOR_SUPPORTED_REPORT_V3,
    0x00,  // Indicator ID (invalid)
    0x00,  // Next Indicator ID
    0x00   // Reserved:3 BitmaskLen:5
  };

  execute_frame_expect_frame(zwave_command_class_indicator_support_handler,
                             cmd_frame_nn,
                             sizeof(cmd_frame_nn),
                             exp_frame_nn,
                             sizeof(exp_frame_nn),
                             SL_STATUS_OK);
}

void test_zwave_command_class_indicator_default_set(void)
{
  uint8_t cmd_frame_get[]       = {COMMAND_CLASS_INDICATOR_V3,
                             INDICATOR_GET_V3,
                             INDICATOR_SET_NODE_IDENTIFY_V3};
  uint8_t cmd_frame_blink[]     = {COMMAND_CLASS_INDICATOR_V3,
                               INDICATOR_SET_V3,
                               0x00,
                               0x03,
                               INDICATOR_SET_NODE_IDENTIFY_V3,
                               INDICATOR_SET_ON_OFF_PERIOD_V3,
                               0x0A,
                               INDICATOR_SET_NODE_IDENTIFY_V3,
                               INDICATOR_SET_ON_OFF_CYCLES_V3,
                               0x02,
                               INDICATOR_SET_NODE_IDENTIFY_V3,
                               INDICATOR_SET_ONE_TIME_ON_OFF_PERIOD_V3,
                               0x00};
  uint8_t exp_frame_get_blink[] = {COMMAND_CLASS_INDICATOR_V3,
                                   INDICATOR_REPORT,
                                   0xFF,
                                   0x03,
                                   INDICATOR_SET_NODE_IDENTIFY_V3,
                                   INDICATOR_SET_ON_OFF_PERIOD_V3,
                                   0x0A,
                                   INDICATOR_SET_NODE_IDENTIFY_V3,
                                   INDICATOR_SET_ON_OFF_CYCLES_V3,
                                   0x02,
                                   INDICATOR_SET_NODE_IDENTIFY_V3,
                                   INDICATOR_SET_ONE_TIME_ON_OFF_PERIOD_V3,
                                   0x00};

  execute_frame_expect_result(zwave_command_class_indicator_support_handler,
                              cmd_frame_blink,
                              sizeof(cmd_frame_blink),
                              SL_STATUS_OK);

  check_blink_script_args(500, 500, 2);
  TEST_ASSERT_TRUE(timer_running);

  execute_frame_expect_frame(zwave_command_class_indicator_support_handler,
                             cmd_frame_blink,
                             sizeof(cmd_frame_blink),
                             exp_frame_get_blink,
                             sizeof(exp_frame_get_blink),
                             SL_STATUS_OK);

  set_indicator_to_default();
  check_blink_script_args(0, 0, 0);
  TEST_ASSERT_FALSE(timer_running);

  exp_frame_get_blink[2]  = 0;
  exp_frame_get_blink[6]  = 0;
  exp_frame_get_blink[9]  = 0;
  exp_frame_get_blink[12] = 0;

  execute_frame_expect_frame(zwave_command_class_indicator_support_handler,
                             cmd_frame_get,
                             sizeof(cmd_frame_get),
                             exp_frame_get_blink,
                             sizeof(exp_frame_get_blink),
                             SL_STATUS_OK);
}