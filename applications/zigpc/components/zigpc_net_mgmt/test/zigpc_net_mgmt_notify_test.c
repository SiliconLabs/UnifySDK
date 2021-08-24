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

#include "zigpc_common_observable_mock.h"

#include "zigpc_net_mgmt_notify.h"
#include "zigpc_net_mgmt_gateway.h"
#include "zigpc_net_mgmt_notify_int.h"

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

/**
 * @brief Test to check delegating reset to common observer
 *
 */
void test_zigpc_net_mgmt_notify_reset_should_call_common_observer(void)
{
  // ARRANGE
  zigpc_observable_clear_Expect(&zigpc_net_mgmt_observable);
  zigpc_observable_init_ExpectAndReturn(&zigpc_net_mgmt_observable,
                                        NULL,
                                        ZIGPC_NET_MGMT_NOTIFY_MAX,
                                        SL_STATUS_OK);
  zigpc_observable_init_IgnoreArg_event_list();

  // ACT
  sl_status_t status = zigpc_net_mgmt_reset_observable();

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
}

/**
 * @brief Test to check failing to reset common observer
 *
 */
void test_zigpc_net_mgmt_notify_reset_should_error_on_failed_common_observer(
  void)
{
  // ARRANGE
  zigpc_observable_clear_Expect(&zigpc_net_mgmt_observable);
  zigpc_observable_init_ExpectAndReturn(&zigpc_net_mgmt_observable,
                                        NULL,
                                        ZIGPC_NET_MGMT_NOTIFY_MAX,
                                        SL_STATUS_FAIL);
  zigpc_observable_init_IgnoreArg_event_list();

  // ACT
  sl_status_t status = zigpc_net_mgmt_reset_observable();

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, status);
}

/**
 * @brief Test to check delegating registering to common observer
 *
 */
void test_zigpc_net_mgmt_notify_register_should_call_common_observer(void)
{
  // ARRANGE
  sl_status_t status;

  zigpc_observable_register_ExpectAndReturn(&zigpc_net_mgmt_observable,
                                            2,
                                            NULL,
                                            SL_STATUS_OK);

  // ACT
  status = zigpc_net_mgmt_register_observer(2, NULL);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
}

/**
 * @brief Test to check delegating unregistering to common observer
 *
 */
void test_zigpc_net_mgmt_notify_unregister_should_call_common_observer(void)
{
  // ARRANGE
  sl_status_t status;

  zigpc_observable_unregister_ExpectAndReturn(&zigpc_net_mgmt_observable,
                                              1,
                                              NULL,
                                              SL_STATUS_OK);

  // ACT
  status = zigpc_net_mgmt_unregister_observer(1, NULL);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
}

/**
 * @brief Test to check sending network_init event via common observer notify
 *
 */
void test_zigpc_net_mgmt_network_init_should_call_common_notify(void)
{
  // ARRANGE
  zigbee_eui64_t test_eui64 = {0xA, 0xB, 0xC, 0xD, 0x0};

  zigpc_observable_notify_ExpectAndReturn(&zigpc_net_mgmt_observable,
                                          ZIGPC_NET_MGMT_NOTIFY_NETWORK_INIT,
                                          NULL,
                                          SL_STATUS_OK);
  zigpc_observable_notify_IgnoreArg_data();

  // ACT
  zigpc_net_mgmt_notify_network_init(test_eui64);

  // ASSERT Handled by CMock
}

/**
 * @brief Test to check sending FSM state update event via common observer notify
 *
 */
void test_zigpc_net_mgmt_state_update_should_check_invalid_params(void)
{
  // ARRANGE
  enum zigpc_net_mgmt_fsm_state test_state = ZIGPC_NET_MGMT_FSM_STATE_IDLE;

  // ACT
  sl_status_t status = zigpc_net_mgmt_notify_state_update(test_state, NULL);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_NULL_POINTER, status);
}

/**
 * @brief Test to check sending FSM state update event via common observer notify
 *
 */
void test_zigpc_net_mgmt_state_update_should_call_common_notify(void)
{
  // ARRANGE
  enum zigpc_net_mgmt_fsm_state test_state = ZIGPC_NET_MGMT_FSM_STATE_IDLE;
  enum zigpc_net_mgmt_fsm_state test_next_states[5] = {0};

  zigpc_observable_notify_ExpectAndReturn(&zigpc_net_mgmt_observable,
                                          ZIGPC_NET_MGMT_NOTIFY_STATE_UPDATE,
                                          NULL,
                                          SL_STATUS_OK);
  zigpc_observable_notify_IgnoreArg_data();

  // ACT
  sl_status_t status
    = zigpc_net_mgmt_notify_state_update(test_state, test_next_states);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
}

/**
 * @brief Test to check sending node_added event via common observer notify
 *
 */
void test_zigpc_net_mgmt_node_added_should_call_common_notify(void)
{
  // ARRANGE
  zigbee_eui64_t test_eui64 = {0x1, 0x0B, 0xC, 0xD, 0x0A};

  zigpc_observable_notify_ExpectAndReturn(&zigpc_net_mgmt_observable,
                                          ZIGPC_NET_MGMT_NOTIFY_NODE_ADDED,
                                          NULL,
                                          SL_STATUS_OK);
  zigpc_observable_notify_IgnoreArg_data();

  // ACT
  zigpc_net_mgmt_notify_node_added(test_eui64);

  // ASSERT Handled by CMock
}

void test_zigpc_net_mgmt_notify_add_requested_parameter_success_path(void)
{
  // ARRANGE
  sl_status_t test_status;

  // ACT
  test_status = zigpc_net_mgmt_notify_add_requested_parameter("param1");

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
}

void test_zigpc_net_mgmt_notify_add_requested_parameter_should_handle_invalid_inputs(
  void)
{
  // ARRANGE
  sl_status_t test_null;
  sl_status_t test_empty;

  // ACT
  test_null  = zigpc_net_mgmt_notify_add_requested_parameter(NULL);
  test_empty = zigpc_net_mgmt_notify_add_requested_parameter("");

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_NULL_POINTER, test_null);
  TEST_ASSERT_EQUAL(SL_STATUS_EMPTY, test_empty);
}
