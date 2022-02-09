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

#include "zigpc_net_mgmt_fsm_mock.h"
#include "zigpc_net_mgmt_process_send_mock.h"

#include "zigpc_net_mgmt.h"

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

/**
 * @brief Check success path of sending node add request.
 *
 */
void test_gateway_node_add_should_call_process_send(void)
{
  // ARRANGE
  zigbee_eui64_t test_eui64 = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF};
  zigbee_install_code_t test_install_code = {0xA};
  uint8_t test_install_code_length        = 12;

  zigpc_net_mgmt_process_send_event_ExpectAndReturn(
    ZIGPC_NET_MGMT_EVENT_FSM,
    NULL,
    sizeof(zigpc_net_mgmt_process_data_fsm_t),
    SL_STATUS_OK);
  zigpc_net_mgmt_process_send_event_IgnoreArg_data();

  // ACT
  sl_status_t test_status = zigpc_net_mgmt_add_node(test_eui64,
                                                    test_install_code,
                                                    test_install_code_length);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
}

/**
 * @brief Check success path of sending node remove request.
 *
 */
void test_net_mgmt_node_remove_should_call_process_send(void)
{
  // ARRANGE
  zigbee_eui64_t test_eui64 = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF};

  zigpc_net_mgmt_fsm_get_state_ExpectAndReturn(ZIGPC_NET_MGMT_FSM_STATE_IDLE);

  zigpc_net_mgmt_process_send_event_ExpectAndReturn(
    ZIGPC_NET_MGMT_EVENT_FSM,
    NULL,
    sizeof(zigpc_net_mgmt_process_data_fsm_t),
    SL_STATUS_OK);
  zigpc_net_mgmt_process_send_event_IgnoreArg_data();

  // ACT
  sl_status_t test_status = zigpc_net_mgmt_remove_node(test_eui64);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
}

/**
 * @brief Check success path of sending state change request.
 *
 */
void test_net_mgmt_change_state_request_should_call_process_send(void)
{
  // ARRANGE
  zigpc_net_mgmt_state_change_request_t test_request = {.requested_state = 2};

  zigpc_net_mgmt_process_send_event_ExpectAndReturn(
    ZIGPC_NET_MGMT_EVENT_FSM,
    NULL,
    sizeof(zigpc_net_mgmt_process_data_fsm_t),
    SL_STATUS_OK);
  zigpc_net_mgmt_process_send_event_IgnoreArg_data();

  // ACT
  sl_status_t test_status = zigpc_net_mgmt_state_change_request(&test_request);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
}
