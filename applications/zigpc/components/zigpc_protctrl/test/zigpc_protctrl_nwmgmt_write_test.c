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

#include <string.h>
#include <malloc.h>

#include "unity.h"
#include "sl_status.h"

#include "zigpc_protctrl_nwmgmt_write.h"
#include "zigpc_protctrl_nwmgmt_write_callback_mock.h"
#include "zigpc_protctrl_int.h"

static char *write_callback_stub_expected_state;
static zigpc_protctrl_state_parameter_t
  write_callback_stub_expected_params_list[10];
static uint8_t write_callback_stub_expected_params_count;
static sl_status_t write_callback_stub_return_status;

sl_status_t stub_write_callback(
  const char *state,
  const zigpc_protctrl_state_parameter_t *state_parameter_list,
  uint8_t state_parameter_count,
  int cmock_num_calls)
{
  TEST_ASSERT_EQUAL_STRING(write_callback_stub_expected_state, state);
  TEST_ASSERT_EQUAL(write_callback_stub_expected_params_count,
                    state_parameter_count);
  for (size_t i = 0; i < state_parameter_count; i++) {
    TEST_ASSERT_EQUAL(state_parameter_list[i].key_size,
                      write_callback_stub_expected_params_list[i].key_size);
    TEST_ASSERT_EQUAL(state_parameter_list[i].value_size,
                      write_callback_stub_expected_params_list[i].value_size);
    TEST_ASSERT_EQUAL_STRING(state_parameter_list[i].key,
                             write_callback_stub_expected_params_list[i].key);
    TEST_ASSERT_EQUAL_STRING(state_parameter_list[i].value,
                             write_callback_stub_expected_params_list[i].value);
  }
  return write_callback_stub_return_status;
}

/**
 * @brief Setup the test suite (called once before all test_xxx functions are
 * called)
 *
 */
void suiteSetUp(void) {}

/**
 * @brief Teardown the test suite (called once after all test_xxx functions are
 * called)
 *
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp(void) {}

void tearDown(void) {}

void test_zigpc_protctrl_on_write_update_callback_should_handle_invalid_args(
  void)
{
  // ARRANGE
  char *test_message = NULL;

  // ACT
  sl_status_t status = zigpc_protctrl_nwmgmt_write_update(test_message);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_NULL_POINTER, status);
}

void test_zigpc_protctrl_on_write_update_callback_should_handle_empty_message(
  void)
{
  // ARRANGE
  char *test_message = "";

  // ACT
  sl_status_t status = zigpc_protctrl_nwmgmt_write_update(test_message);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_EMPTY, status);
}

void test_zigpc_protctrl_on_write_update_callback_should_handle_non_json_message(
  void)
{
  // ARRANGE
  char *test_message = "{";

  // ACT
  sl_status_t status = zigpc_protctrl_nwmgmt_write_update(test_message);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OBJECT_READ, status);
}

void test_zigpc_protctrl_on_write_update_callback_should_handle_json_without_state_field(
  void)
{
  // ARRANGE
  char *test_message = "{"
                       "\"Test\": 2"
                       "}";

  // ACT
  sl_status_t status = zigpc_protctrl_nwmgmt_write_update(test_message);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_CONFIGURATION, status);
}

void test_zigpc_protctrl_on_write_update_callback_should_handle_json_with_only_state_field(
  void)
{
  // ARRANGE
  char *test_message = "{"
                       "\"State\": \"New State\""
                       "}";

  zigpc_protctrl_on_write_update_callback_ExpectAndReturn("New State",
                                                          NULL,
                                                          0,
                                                          SL_STATUS_OK);
  zigpc_protctrl_on_write_update_callback_IgnoreArg_state_parameter_list();

  // ACT
  sl_status_t status = zigpc_protctrl_nwmgmt_write_update(test_message);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
}

void test_zigpc_protctrl_on_write_update_callback_should_handle_json_with_no_state_parameters(
  void)
{
  // ARRANGE
  char *test_message = "{"
                       "\"State\": \"New State 2\","
                       "\"StateParameters\": {}"
                       "}";

  zigpc_protctrl_on_write_update_callback_ExpectAndReturn("New State 2",
                                                          NULL,
                                                          0,
                                                          SL_STATUS_OK);
  zigpc_protctrl_on_write_update_callback_IgnoreArg_state_parameter_list();

  // ACT
  sl_status_t status = zigpc_protctrl_nwmgmt_write_update(test_message);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
}

void test_zigpc_protctrl_on_write_update_callback_should_handle_json_with_single_state_parameter(
  void)
{
  // ARRANGE
  char *test_message = "{"
                       "\"State\": \"idle\","
                       "\"StateParameters\": {"
                       "\"field1\": 3"
                       "}"
                       "}";

  write_callback_stub_expected_state                     = "idle";
  write_callback_stub_expected_params_count              = 1;
  write_callback_stub_expected_params_list[0].key        = "field1";
  write_callback_stub_expected_params_list[0].key_size   = 6;
  write_callback_stub_expected_params_list[0].value      = "3";
  write_callback_stub_expected_params_list[0].value_size = 1;
  write_callback_stub_return_status                      = SL_STATUS_OK;
  zigpc_protctrl_on_write_update_callback_StubWithCallback(stub_write_callback);

  // ACT
  sl_status_t status = zigpc_protctrl_nwmgmt_write_update(test_message);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
}

void test_zigpc_protctrl_on_write_update_callback_should_handle_json_with_multiple_state_parameters(
  void)
{
  // ARRANGE
  char *test_message = "{"
                       "\"State\": \"add node\","
                       "\"StateParameters\": {"
                       "\"UNID\": 3,"
                       "\"prompt\": \"Hello\","
                       "\"deny\": true,"
                       "\"treshold\": 3.14159"
                       "}"
                       "}";

  write_callback_stub_expected_state                     = "add node";
  write_callback_stub_expected_params_count              = 4;
  write_callback_stub_expected_params_list[0].key        = "UNID";
  write_callback_stub_expected_params_list[0].key_size   = 4;
  write_callback_stub_expected_params_list[0].value      = "3";
  write_callback_stub_expected_params_list[0].value_size = 1;
  write_callback_stub_expected_params_list[1].key        = "prompt";
  write_callback_stub_expected_params_list[1].key_size   = 6;
  write_callback_stub_expected_params_list[1].value      = "Hello";
  write_callback_stub_expected_params_list[1].value_size = 5;
  write_callback_stub_expected_params_list[2].key        = "deny";
  write_callback_stub_expected_params_list[2].key_size   = 4;
  write_callback_stub_expected_params_list[2].value      = "true";
  write_callback_stub_expected_params_list[2].value_size = 4;
  write_callback_stub_expected_params_list[3].key        = "treshold";
  write_callback_stub_expected_params_list[3].key_size   = 8;
  write_callback_stub_expected_params_list[3].value      = "3.14159";
  write_callback_stub_expected_params_list[3].value_size = 7;
  write_callback_stub_return_status                      = SL_STATUS_OK;
  zigpc_protctrl_on_write_update_callback_StubWithCallback(stub_write_callback);

  // ACT
  sl_status_t status = zigpc_protctrl_nwmgmt_write_update(test_message);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
}
