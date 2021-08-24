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
#include "string.h"

#include "zigpc_gateway_notify_mock.h"
#include "zigpc_net_mgmt_fsm_mock.h"
#include "zigpc_net_mgmt_process_send_mock.h"

#include "zigpc_net_mgmt_gateway.h"

/**
 * Prototypes of functions under test
 */

void zigpc_net_mgmt_callback_on_network_initialized(void *event_data);
void zigpc_net_mgmt_callback_on_node_add_complete(void *event_data);
void zigpc_net_mgmt_callback_on_node_discovered(void *event_data);
void zigpc_net_mgmt_callback_on_node_endpoint_discovered(void *event_data);
void zigpc_net_mgmt_callback_on_node_removed(void *event_data);

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

void test_gateway_registrations_should_succeed(void)
{
  // ARRANGE
  enum zigpc_gateway_notify_event expected_events[] = {
    ZIGPC_GATEWAY_NOTIFY_NETWORK_INIT,
    ZIGPC_GATEWAY_NOTIFY_NODE_ADD_START,
    ZIGPC_GATEWAY_NOTIFY_NODE_ADD_COMPLETE,
    ZIGPC_GATEWAY_NOTIFY_NODE_DISCOVERED,
    ZIGPC_GATEWAY_NOTIFY_NODE_ENDPOINT_DISCOVERED,
    ZIGPC_GATEWAY_NOTIFY_NODE_REMOVED,
  };

  for (int i = 0, length = sizeof(expected_events) / sizeof(expected_events[0]);
       i < length;
       i++) {
    zigpc_gateway_register_observer_ExpectAndReturn(expected_events[i],
                                                    NULL,
                                                    SL_STATUS_OK);
    zigpc_gateway_register_observer_IgnoreArg_callback();
  }

  // ACT
  sl_status_t test_status = zigpc_net_mgmt_gateway_init_observer();

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
}

void test_gateway_network_init_callback_should_call_process_send(void)
{
  // ARRANGE
  struct zigpc_gateway_on_network_init callback_data;
  zigpc_net_mgmt_fsm_get_state_ExpectAndReturn(ZIGPC_NET_MGMT_FSM_STATE_INIT);

  zigpc_net_mgmt_process_send_event_ExpectAndReturn(
    ZIGPC_NET_MGMT_EVENT_FSM,
    NULL,
    sizeof(zigpc_net_mgmt_process_data_fsm_t),
    SL_STATUS_OK);
  zigpc_net_mgmt_process_send_event_IgnoreArg_data();

  // ACT
  zigpc_net_mgmt_callback_on_network_initialized(&callback_data);

  // ASSERT (Handled by CMock)
}

void test_gateway_node_added_callback_should_call_process_send(void)
{
  // ARRANGE
  struct zigpc_gateway_on_node_add callback_data;
  zigpc_net_mgmt_fsm_get_state_ExpectAndReturn(
    ZIGPC_NET_MGMT_FSM_STATE_NODE_ADD);

  zigpc_net_mgmt_process_send_event_ExpectAndReturn(
    ZIGPC_NET_MGMT_EVENT_FSM,
    NULL,
    sizeof(zigpc_net_mgmt_process_data_fsm_t),
    SL_STATUS_OK);
  zigpc_net_mgmt_process_send_event_IgnoreArg_data();

  // ACT
  zigpc_net_mgmt_callback_on_node_add_complete(&callback_data);

  // ASSERT (Handled by CMock)
}
/**
 * @brief Check success path of sending FSM event when gateway node discovered
 * event is received
 *
 */
void test_gateway_node_discovered_callback_should_call_process_send(void)
{
  // ARRANGE
  zigpc_gateway_on_node_discovered_t callback_data = {
    .eui64          = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF},
    .endpoint_count = 5,
  };

  zigpc_net_mgmt_process_send_event_ExpectAndReturn(
    ZIGPC_NET_MGMT_EVENT_FSM,
    NULL,
    sizeof(zigpc_net_mgmt_process_data_fsm_t),
    SL_STATUS_OK);
  zigpc_net_mgmt_process_send_event_IgnoreArg_data();

  // ACT
  zigpc_net_mgmt_callback_on_node_discovered(&callback_data);

  // ASSERT (Handled by CMock)
}

/**
 * @brief Check success path of sending FSM event when gateway node endpoint
 * discovered event is received
 *
 */
void test_gateway_node_endpoint_discovered_callback_should_call_process_send(
  void)
{
  // ARRANGE
  zcl_cluster_type_t test_cluster_list[] = {{.cluster_id = 0x6}};
  zigpc_gateway_on_node_endpoint_discovered_t callback_data
    = {.eui64    = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF},
       .endpoint = {.endpoint_id = 1, .cluster_count = 1}};

  memcpy(callback_data.endpoint.cluster_list,
         test_cluster_list,
         sizeof(zcl_cluster_type_t));

  zigpc_net_mgmt_process_send_event_ExpectAndReturn(
    ZIGPC_NET_MGMT_EVENT_FSM,
    NULL,
    sizeof(zigpc_net_mgmt_process_data_fsm_t),
    SL_STATUS_OK);
  zigpc_net_mgmt_process_send_event_IgnoreArg_data();

  // ACT
  zigpc_net_mgmt_callback_on_node_endpoint_discovered(&callback_data);

  // ASSERT (Handled by CMock)
}

void test_gateway_node_removed_callback_should_call_process_send(void)
{
  // ARRANGE
  zigpc_gateway_on_node_removed_t callback_data;
  zigpc_net_mgmt_fsm_get_state_ExpectAndReturn(
    ZIGPC_NET_MGMT_FSM_STATE_NODE_REMOVE);

  zigpc_net_mgmt_process_send_event_ExpectAndReturn(
    ZIGPC_NET_MGMT_EVENT_FSM,
    NULL,
    sizeof(zigpc_net_mgmt_process_data_fsm_t),
    SL_STATUS_OK);
  zigpc_net_mgmt_process_send_event_IgnoreArg_data();

  // ACT
  zigpc_net_mgmt_callback_on_node_removed(&callback_data);

  // ASSERT (Handled by CMock)
}
