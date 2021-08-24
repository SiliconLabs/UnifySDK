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

#include "zigpc_protctrl_int.h"

#include "uic_mqtt_mock.h"
#include "zigpc_common_zigbee_mock.h"
#include "zigpc_protctrl_nwmgmt_write_mock.h"

/* Prototypes */
sl_status_t zigpc_protctrl_str_list_join(const char *const *str_list,
                                         uint8_t str_num,
                                         char *prefix,
                                         char *delim,
                                         char *postfix,
                                         char *buffer,
                                         uint16_t buffer_size);

sl_status_t zigpc_protctrl_build_state_str_list(
  char **str_list,
  uint8_t str_count,
  const enum zigpc_net_mgmt_fsm_state *state_list,
  uint8_t state_count);

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
 * Tests for zigpc_protctrl_str_list_join
 *******************/

void test_zigpc_protctrl_str_list_join_should_check_invalid_params()
{
  // ARRANGE
  const char *test_strings[] = {"test"};
  char test_buffer[1];

  // ACT
  sl_status_t status_null_all
    = zigpc_protctrl_str_list_join(NULL, 0, NULL, NULL, NULL, NULL, 0);
  sl_status_t status_null_prefix
    = zigpc_protctrl_str_list_join(test_strings, 1, NULL, NULL, NULL, NULL, 0);
  sl_status_t status_null_delim
    = zigpc_protctrl_str_list_join(test_strings, 1, "", NULL, NULL, NULL, 0);
  sl_status_t status_null_postfix
    = zigpc_protctrl_str_list_join(test_strings, 1, "", "", NULL, NULL, 0);
  sl_status_t status_null_buffer
    = zigpc_protctrl_str_list_join(test_strings, 1, "", "", "", NULL, 0);
  sl_status_t status_insufficient_size
    = zigpc_protctrl_str_list_join(test_strings, 1, "", "", "", test_buffer, 1);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_NULL_POINTER, status_null_all);
  TEST_ASSERT_EQUAL(SL_STATUS_NULL_POINTER, status_null_prefix);
  TEST_ASSERT_EQUAL(SL_STATUS_NULL_POINTER, status_null_delim);
  TEST_ASSERT_EQUAL(SL_STATUS_NULL_POINTER, status_null_postfix);
  TEST_ASSERT_EQUAL(SL_STATUS_NULL_POINTER, status_null_buffer);
  TEST_ASSERT_EQUAL(SL_STATUS_WOULD_OVERFLOW, status_insufficient_size);
}

void test_zigpc_protctrl_str_list_join_should_run_with_empty_list()
{
  // ARRANGE
  char test_buffer[30];
  const char *test_strings[] = {};

  // ACT
  sl_status_t status = zigpc_protctrl_str_list_join(test_strings,
                                                    0,
                                                    "PRE",
                                                    ",",
                                                    "POST",
                                                    test_buffer,
                                                    30);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_STRING("PREPOST", test_buffer);
}

void test_zigpc_protctrl_str_list_join_should_run_with_one_item_in_list()
{
  // ARRANGE
  char test_buffer[30];
  const char *test_strings[] = {"test"};

  // ACT
  sl_status_t status = zigpc_protctrl_str_list_join(test_strings,
                                                    1,
                                                    "PRE",
                                                    ",",
                                                    "POST",
                                                    test_buffer,
                                                    30);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_STRING("PREtestPOST", test_buffer);
}

void test_zigpc_protctrl_str_list_join_should_run_with_empty_prefix_delim_posfix()
{
  // ARRANGE
  char test_buffer[30];
  const char *test_strings[] = {"one", "two"};

  // ACT
  sl_status_t status = zigpc_protctrl_str_list_join(test_strings,
                                                    2,
                                                    "",
                                                    "",
                                                    "",
                                                    test_buffer,
                                                    30);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_STRING("onetwo", test_buffer);
}

void test_zigpc_protctrl_str_list_join_should_include_prefix_delim_postfix()
{
  // ARRANGE
  char test_buffer[50];
  const char *test_strings[] = {"one", "two"};

  // ACT
  sl_status_t status = zigpc_protctrl_str_list_join(test_strings,
                                                    2,
                                                    "PREFIX ",
                                                    " | ",
                                                    " [POSTFIX]",
                                                    test_buffer,
                                                    30);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_STRING("PREFIX one | two [POSTFIX]", test_buffer);
}

/********************
 * Tests for zigpc_protctrl_build_state_str_list
 *******************/

void test_zigpc_protctrl_build_state_str_list_should_check_invalid_params()
{
  // ARRANGE
  char *test_string_list[5];
  const enum zigpc_net_mgmt_fsm_state test_state_list[] = {
    ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD,
  };

  // ACT
  sl_status_t status_null_check
    = zigpc_protctrl_build_state_str_list(NULL, 0, NULL, 0);

  sl_status_t status_null_str_list
    = zigpc_protctrl_build_state_str_list(NULL, 0, test_state_list, 2);

  sl_status_t status_null_state_list
    = zigpc_protctrl_build_state_str_list(test_string_list, 1, NULL, 0);

  sl_status_t status_not_enough_space
    = zigpc_protctrl_build_state_str_list(test_string_list,
                                          1,
                                          test_state_list,
                                          2);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_NULL_POINTER, status_null_check);
  TEST_ASSERT_EQUAL(SL_STATUS_NULL_POINTER, status_null_str_list);
  TEST_ASSERT_EQUAL(SL_STATUS_NULL_POINTER, status_null_state_list);
  TEST_ASSERT_EQUAL(SL_STATUS_WOULD_OVERFLOW, status_not_enough_space);
}

void test_zigpc_protctrl_build_state_str_list_should_build_state_str_list()
{
  // ARRANGE
  char *test_string_list[5];
  const enum zigpc_net_mgmt_fsm_state test_state_list[] = {
    ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD,
    ZIGPC_NET_MGMT_FSM_STATE_IDLE,
  };

  // ACT
  sl_status_t test_status
    = zigpc_protctrl_build_state_str_list(test_string_list,
                                          5,
                                          test_state_list,
                                          2);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
  TEST_ASSERT_EQUAL_STRING("add node", test_string_list[0]);
  TEST_ASSERT_EQUAL_STRING("idle", test_string_list[1]);
}

/********************
 * Tests for process handlers
 *******************/

void test_zigpc_protctrl_should_set_pc_unid_on_net_init()
{
  // ARRANGE
  char *test_eui64_str = "0102030405060708";
  const struct zigpc_net_mgmt_on_network_init net_init_data
    = {.pc_eui64 = {1, 2, 3, 4, 5, 6, 7, 8}};

  zigbee_eui64_to_str_ExpectAndReturn(net_init_data.pc_eui64,
                                      NULL,
                                      ZIGBEE_EUI64_HEX_STR_LENGTH,
                                      SL_STATUS_OK);
  zigbee_eui64_to_str_IgnoreArg_str_buf();
  zigbee_eui64_to_str_ReturnArrayThruPtr_str_buf(test_eui64_str,
                                                 strlen(test_eui64_str) + 1);

  zigpc_protctrl_nwmgmt_write_init_ExpectAndReturn(test_eui64_str,
                                                   SL_STATUS_OK);

  // ACT
  sl_status_t status = zigpc_protctrl_process_event_net_init(&net_init_data);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_MEMORY(net_init_data.pc_eui64,
                           protctrl_state.pc_eui64,
                           sizeof(zigbee_eui64_t));
  TEST_ASSERT_EQUAL_MEMORY(test_eui64_str,
                           protctrl_state.pc_eui64_str,
                           ZIGBEE_EUI64_HEX_STR_LENGTH);
}

void test_zigpc_protctrl_should_handle_invalid_states_on_net_state_update()
{
  // ARRANGE
  const struct zigpc_net_mgmt_on_network_state_update state_update_data = {
    .new_state                   = ZIGPC_NET_MGMT_FSM_STATE_INIT,
    .next_supported_states_count = 0,
    .next_supported_states_list  = {},
  };

  // ACT
  sl_status_t status
    = zigpc_protctrl_process_event_net_state_update(&state_update_data);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND, status);
}

void test_zigpc_protctrl_should_publish_on_net_state_update()
{
  // ARRANGE
  struct zigpc_net_mgmt_on_network_state_update state_update_data
    = {
        .new_state                        = ZIGPC_NET_MGMT_FSM_STATE_IDLE,
        .next_supported_states_list       = {
          ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD,
        },
        .next_supported_states_count      = 1,
        .requested_state_parameter_count  = 1,
  };

  state_update_data.requested_state_parameter_list[0] = "param1";

  uic_mqtt_publish_Ignore();

  // ACT
  sl_status_t status
    = zigpc_protctrl_process_event_net_state_update(&state_update_data);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
}
