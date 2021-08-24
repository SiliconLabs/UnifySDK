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
#include "sl_status.h"

#include "zigpc_protctrl_int.h"

#include "zigpc_net_mgmt_notify_mock.h"
#include "zigpc_protctrl_process_send_mock.h"

void zigpc_protctrl_observer_net_mgmt_on_net_init(void *event_data);
void zigpc_protctrl_observer_net_mgmt_on_state_update(void *event_data);

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

void test_zigpc_protctrl_observers_init_should_be_succeed()
{
  // ARRANGE
  zigpc_net_mgmt_register_observer_ExpectAndReturn(
    ZIGPC_NET_MGMT_NOTIFY_NETWORK_INIT,
    zigpc_protctrl_observer_net_mgmt_on_net_init,
    SL_STATUS_OK);
  zigpc_net_mgmt_register_observer_ExpectAndReturn(
    ZIGPC_NET_MGMT_NOTIFY_STATE_UPDATE,
    zigpc_protctrl_observer_net_mgmt_on_state_update,
    SL_STATUS_OK);

  // ACT
  sl_status_t status = zigpc_protctrl_init_net_mgmt_observers();

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
}

void test_zigpc_protctrl_observers_init_fail_early()
{
  // ARRANGE
  zigpc_net_mgmt_register_observer_ExpectAndReturn(
    ZIGPC_NET_MGMT_NOTIFY_NETWORK_INIT,
    zigpc_protctrl_observer_net_mgmt_on_net_init,
    SL_STATUS_OK);
  zigpc_net_mgmt_register_observer_ExpectAndReturn(
    ZIGPC_NET_MGMT_NOTIFY_STATE_UPDATE,
    zigpc_protctrl_observer_net_mgmt_on_state_update,
    SL_STATUS_FAIL);

  // ACT
  sl_status_t status = zigpc_protctrl_init_net_mgmt_observers();

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, status);
}

void test_zigpc_protctrl_net_init_observer_should_pass_event()
{
  // ARRANGE
  struct zigpc_net_mgmt_on_network_init event_data
    = {.pc_eui64 = {0x1, 0x2, 0x3}};

  zigpc_protctrl_process_send_event_Expect(ZIGPC_PROTCTRL_EVENT_NET_INIT, NULL);
  zigpc_protctrl_process_send_event_IgnoreArg_data();

  // ACT
  zigpc_protctrl_observer_net_mgmt_on_net_init((void *)&event_data);

  // ASSERT (Handled by CMock)
}

void test_zigpc_protctrl_net_state_update_observer_should_pass_event()
{
  // ARRANGE
  struct zigpc_net_mgmt_on_network_state_update event_data
    = {.new_state = 0x777};

  zigpc_protctrl_process_send_event_Expect(
    ZIGPC_PROTCTRL_EVENT_NET_STATE_UPDATE,
    NULL);
  zigpc_protctrl_process_send_event_IgnoreArg_data();

  // ACT
  zigpc_protctrl_observer_net_mgmt_on_state_update((void *)&event_data);

  // ASSERT (Handled by CMock)
}
