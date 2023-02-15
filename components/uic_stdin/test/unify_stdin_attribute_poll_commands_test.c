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
#include "unify_stdin_attribute_poll_command_handler.h"
#include "uic_stdin_command_handler.h"
#include "unity.h"

#include "attribute_poll_mock.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  unify_stdin_add_attribute_poll_commands();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_unify_stdin_attribute_poll_commands_register()
{
  sl_status_t state = uic_stdin_handle_command("attribute_poll_register");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_poll_register fish");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_poll_register_ExpectAndReturn(42, 0, SL_STATUS_OK);
  state = uic_stdin_handle_command("attribute_poll_register 42");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  attribute_poll_register_ExpectAndReturn(1234, 42, SL_STATUS_OK);
  state = uic_stdin_handle_command("attribute_poll_register 1234,42");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_unify_stdin_attribute_poll_commands_deregister()
{
  sl_status_t state = uic_stdin_handle_command("attribute_poll_deregister");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_poll_deregister fish");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_poll_deregister_ExpectAndReturn(42, SL_STATUS_OK);
  state = uic_stdin_handle_command("attribute_poll_deregister 42");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_handle_attribute_poll_print()
{
  attribute_poll_print_Expect();
  sl_status_t state
    = uic_stdin_handle_command("attribute_poll_print 1,2,3,4,5");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}