/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "unify_stdin_attribute_resolver_command_handler.h"
#include "uic_stdin_command_handler.h"
#include "unity.h"

// Mock includes
#include "attribute_resolver_mock.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  unify_stdin_add_attribute_resolver_commands();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_attribute_resolver_log()
{
  sl_status_t state;
  attribute_resolver_state_log_Expect();
  state = uic_stdin_handle_command("attribute_resolver_log");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  attribute_resolver_state_log_Expect();
  state = uic_stdin_handle_command("attribute_resolver_log 1,23,4,6");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}