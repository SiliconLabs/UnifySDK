/* # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 */

// UIC Components
#include "process.h"
#include "unity.h"

// Component
#include "uic_stdin_command_handler.h"
#include "sl_log.h"

// Standard
#include <stdbool.h>

#define LOG_TAG "uic_stdin_test"

void suiteSetUp() {}

void suiteTeardown() {}

void test_get_prompt()
{
  const char *prompt = get_prompt_string();
  TEST_ASSERT_EQUAL_STRING(prompt, "UIC> ");
}

void test_handle_help()
{
  sl_status_t state = uic_stdin_handle_command("help");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("help arg");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_exit()
{
  sl_status_t state = uic_stdin_handle_command("exit");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_wrong_command()
{
  sl_status_t state = uic_stdin_handle_command("random");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);
}

void test_set_log_level()
{
  sl_status_t state = uic_stdin_handle_command("log_level i");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("log_level d");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("log_level w");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("log_level e");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("log_level c");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("log_level i,off");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("log_level c,off");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("log_level wqeqw,off");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("log_level info,i");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("log_level");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("log_level 1231,2131");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);
}

void test_command_generator()
{
  char *test = uic_stdin_command_generator("command", 1);
  TEST_ASSERT_EQUAL_STRING(test, NULL);

  test = uic_stdin_command_generator("command", 0);
  TEST_ASSERT_EQUAL_STRING(test, NULL);
}