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

#include "zigpc_protctrl_nwmgmt_write_callback.h"

#include "zigpc_common_zigbee_mock.h"
#include "zigpc_net_mgmt_mock.h"

/* Prototypes */
sl_status_t zigpc_protctrl_nwmgmt_write_extract_known_state_parameters(
  zigpc_net_mgmt_state_change_request_t *state_change,
  const zigpc_protctrl_state_parameter_t *parameter);
sl_status_t zigpc_protctrl_on_write_update_callback(
  const char *state,
  const zigpc_protctrl_state_parameter_t *state_parameter_list,
  uint8_t state_parameter_count);

/**
 * @brief Copy of NWMGMT State to FSM string states defined by UIC. See
 * zigpc_protctrl_nwmgmt.c for reference data structure.
 *
 */
extern char
  *const zigpc_protctrl_nwmgmt_state_str[ZIGPC_NET_MGMT_FSM_STATE_MAX_VAL];

/**
 * @brief Setup the test suite (called once before all test_xxx functions are
 * called)
 *
 */
void suiteSetUp() {}

/**
 * @brief Teardown the test suite (called once after all test_xxx functions are
 * called)
 *
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp() {}

void tearDown() {}

/********************
 * Tests for zigpc_protctrl_nwmgmt_write_extract_known_state_parameters
 *******************/

void test_zigpc_protctrl_parameter_extraction_should_handle_invalid_inputs(void)
{
  // ARRANGE
  zigpc_net_mgmt_state_change_request_t test_state_change;
  zigpc_protctrl_state_parameter_t test_param = {
    .key_size = 0,
  };

  // ACT
  sl_status_t status_null_1
    = zigpc_protctrl_nwmgmt_write_extract_known_state_parameters(NULL, NULL);
  sl_status_t status_null_2
    = zigpc_protctrl_nwmgmt_write_extract_known_state_parameters(
      &test_state_change,
      NULL);
  sl_status_t status_empty_key
    = zigpc_protctrl_nwmgmt_write_extract_known_state_parameters(
      &test_state_change,
      &test_param);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_NULL_POINTER, status_null_1);
  TEST_ASSERT_EQUAL(SL_STATUS_NULL_POINTER, status_null_2);
  TEST_ASSERT_EQUAL(SL_STATUS_EMPTY, status_empty_key);
}

void test_zigpc_protctrl_parameter_extraction_should_fill_known_type(void)
{
  // ARRANGE
  zigbee_eui64_t expected_eui64 = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF};
  zigpc_net_mgmt_state_change_request_t test_state_change;
  zigpc_protctrl_state_parameter_t test_param = {
    .key        = "Unid",
    .key_size   = 4,
    .value      = "testing-0D0E0A0D0B0E0E0F",
    .value_size = 24,
  };

  str_to_zigbee_eui64_ExpectAndReturn(test_param.value,
                                      test_param.value_size,
                                      NULL,
                                      SL_STATUS_OK);
  str_to_zigbee_eui64_IgnoreArg_eui64();
  str_to_zigbee_eui64_ReturnArrayThruPtr_eui64(expected_eui64,
                                               ZIGBEE_EUI64_SIZE);

  // ACT
  sl_status_t status
    = zigpc_protctrl_nwmgmt_write_extract_known_state_parameters(
      &test_state_change,
      &test_param);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_eui64,
                                test_state_change.param_eui64,
                                sizeof(zigbee_eui64_t));
}

void test_zigpc_protctrl_on_write_update_callback_should_handle_null_pointer(
  void)
{
  // ARRANGE

  // ACT
  sl_status_t test_null_1
    = zigpc_protctrl_on_write_update_callback(NULL, NULL, 0);
  sl_status_t test_null_2
    = zigpc_protctrl_on_write_update_callback("", NULL, 0);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_NULL_POINTER, test_null_1);
  TEST_ASSERT_EQUAL(SL_STATUS_EMPTY, test_null_2);
}

void test_zigpc_protctrl_on_write_update_callback_should_handle_invalid_state(
  void)
{
  // ARRANGE
  char *test_state = "testing";
  zigpc_protctrl_state_parameter_t test_parameter;

  // ACT
  sl_status_t status
    = zigpc_protctrl_on_write_update_callback(test_state, &test_parameter, 0);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_TYPE, status);
}

void test_zigpc_protctrl_on_write_update_callback_should_handle_known_state_with_no_params(
  void)
{
  // ARRANGE
  char *test_state = "remove node";
  zigpc_protctrl_state_parameter_t test_parameter;
  zigpc_net_mgmt_state_change_request_t expected_state_change = {
    .requested_state    = ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE,
    .param_eui64_filled = false,
  };

  zigpc_net_mgmt_state_change_request_ExpectAndReturn(&expected_state_change,
                                                      SL_STATUS_OK);

  // ACT
  sl_status_t status
    = zigpc_protctrl_on_write_update_callback(test_state, &test_parameter, 0);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
}

void test_zigpc_protctrl_on_write_update_callback_should_handle_known_state_with_params(
  void)
{
  // ARRANGE
  char *test_state                                            = "remove node";
  zigpc_net_mgmt_state_change_request_t expected_state_change = {
    .requested_state    = ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE,
    .param_eui64        = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF},
    .param_eui64_filled = true,
  };
  zigpc_protctrl_state_parameter_t test_parameter = {
    .key        = "Unid",
    .key_size   = 4,
    .value      = "testing-0D0E0A0D0B0E0E0F",
    .value_size = 24,
  };

  str_to_zigbee_eui64_ExpectAndReturn(test_parameter.value,
                                      test_parameter.value_size,
                                      NULL,
                                      SL_STATUS_OK);
  str_to_zigbee_eui64_IgnoreArg_eui64();
  str_to_zigbee_eui64_ReturnArrayThruPtr_eui64(
    expected_state_change.param_eui64,
    ZIGBEE_EUI64_SIZE);
  zigpc_net_mgmt_state_change_request_ExpectAndReturn(&expected_state_change,
                                                      SL_STATUS_OK);

  // ACT
  sl_status_t status
    = zigpc_protctrl_on_write_update_callback(test_state, &test_parameter, 1);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
}