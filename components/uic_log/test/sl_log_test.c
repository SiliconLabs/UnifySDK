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
#include "sl_log.h"
#include "unity.h"
#include <stdbool.h>
#define LOG_TAG "log_test"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  sl_log_info(LOG_TAG, "log_test", "suiteSetUp");
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTeardown(int num_failures)
{
  return num_failures;
}

void test_log()
{
  sl_log("test1", SL_LOG_CRITICAL, "Hello %s", "world");
  sl_log_debug(LOG_TAG, "Hello World\n");
  sl_log_info(LOG_TAG, "test2", "This is a number %u\n", 123);
  sl_log_warning(LOG_TAG, "test1", "This is a signed number %d\n", -1234);
  sl_log_critical(LOG_TAG, "test42", "This is an unsigned number %u", 1234);
  sl_log_error(LOG_TAG, "wassap", "This is a string %s", "Test String");
}

void test_log_level()
{
  sl_log_set_tag_level("mytag", SL_LOG_INFO);
  sl_log_debug("mytag", "this shouldn't be logged");
  sl_log_debug("mytag2", "this should be logged 1");
  sl_log_info("mytag", "this should be logged 2");
  sl_log_warning("mytag", "this should be logged 3");
  sl_log_error("mytag", "this should be logged 4");
  sl_log_critical("mytag", "this should be logged 5");

  sl_log_set_tag_level("mytag", SL_LOG_DEBUG);
  sl_log_debug("mytag", "this should be logged 6");

  sl_log_set_level(SL_LOG_ERROR);
  sl_log_warning("mytag", "this should be logged 7");
  sl_log_unset_tag_level("mytag");
  sl_log_warning("mytag", "this shouldn't be logged");
  sl_log_error("mytag", "This should be logged 8");
  sl_log_critical("mytag", "This should be logged 9");
}
