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
#include "unify_stdin_attribute_store_command_handler.h"
#include "uic_stdin_command_handler.h"
#include "unity.h"

// Mock includes
#include "attribute_store_configuration_mock.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  unify_stdin_add_attribute_store_commands();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_attribute_store_configuration_set_auto_save_cooldown_interval()
{
  sl_status_t state;
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_cooldown_interval");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_cooldown_interval zzz");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_cooldown_interval 1,2");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_store_configuration_set_auto_save_cooldown_interval_Expect(1);
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_cooldown_interval 1");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  attribute_store_configuration_set_auto_save_cooldown_interval_Expect(999);
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_cooldown_interval 999");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  attribute_store_configuration_set_auto_save_cooldown_interval_Expect(0);
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_cooldown_interval 0");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_attribute_store_configuration_set_auto_save_safety_interval()
{
  sl_status_t state;
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_safety_interval");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_safety_interval zzz");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_safety_interval 1,2");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_store_configuration_set_auto_save_safety_interval_Expect(1);
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_safety_interval 1");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  attribute_store_configuration_set_auto_save_safety_interval_Expect(999);
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_safety_interval 999");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  attribute_store_configuration_set_auto_save_safety_interval_Expect(0);
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_safety_interval 0");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_attribute_store_configuration_set_type_validation()
{
  sl_status_t state;
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_type_validation");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_type_validation zzz");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_type_validation 1,2");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_store_configuration_set_type_validation_Expect(true);
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_type_validation 1");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  attribute_store_configuration_set_type_validation_Expect(false);
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_type_validation 0");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}
