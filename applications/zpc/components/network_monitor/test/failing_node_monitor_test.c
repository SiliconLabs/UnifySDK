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
// Component being tested
#include "failing_node_monitor.h"

// Unify Components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "attribute_timeouts.h"

// Interfaces
#include "attribute_store_defined_attribute_types.h"
#include "zwave_node_id_definitions.h"
#include "zwave_command_class_wake_up_types.h"

#include "unify_dotdot_attribute_store_node_state.h"

// Test helpers
#include "unity.h"
#include "zpc_attribute_store_test_helper.h"
#include "contiki_test_helper.h"

// Mock includes
#include "zwave_controller_utils_mock.h"

// Private variables
static on_zwave_tx_send_data_complete_t on_send_nop_completed = NULL;
static sl_status_t zwave_send_nop_to_node_return_value        = SL_STATUS_OK;
static int send_nop_calls                                     = 0;
static zwave_node_id_t expected_node_id                       = 0;

// Stub
sl_status_t zwave_send_nop_to_node_stub(
  zwave_node_id_t node_id,
  uint32_t qos_priority,
  uint32_t discard_timeout_ms,
  const on_zwave_tx_send_data_complete_t on_send_complete,
  void *user,
  int cmock_num_calls)
{
  send_nop_calls++;
  TEST_ASSERT_EQUAL(expected_node_id, node_id);
  TEST_ASSERT_EQUAL(ZWAVE_TX_QOS_MIN_PRIORITY, qos_priority);
  TEST_ASSERT_EQUAL(NOP_DISCARD_TIMEOUT_MS, discard_timeout_ms);
  on_send_nop_completed = on_send_complete;
  return zwave_send_nop_to_node_return_value;
}

// Helper functions
/**
 * @brief Create a al node in the attribute store and set it to online functional
 *
 * @param node_id     Z-Wave NodeID to create in the attribute store
 * @returns network_status_node for the NodeID.
 */
static attribute_store_node_t create_al_node(zwave_node_id_t node_id)
{
  attribute_store_node_t node_id_node
    = attribute_store_get_node_child_by_value(home_id_node,
                                              ATTRIBUTE_NODE_ID,
                                              REPORTED_ATTRIBUTE,
                                              (const uint8_t *)&node_id,
                                              sizeof(node_id),
                                              0);
  if (node_id_node == ATTRIBUTE_STORE_INVALID_NODE) {
    node_id_node = attribute_store_add_node(ATTRIBUTE_NODE_ID, home_id_node);
    attribute_store_set_reported(node_id_node, &node_id, sizeof(node_id));
  }

  uint8_t listening_protocol
    = ZWAVE_NODE_INFO_LISTENING_PROTOCOL_LISTENING_MASK;
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
                                     &listening_protocol,
                                     sizeof(listening_protocol));

  attribute_store_node_t network_status_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                                             0);
  if (network_status_node == ATTRIBUTE_STORE_INVALID_NODE) {
    network_status_node
      = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS, node_id_node);
  }
  NodeStateNetworkStatus network_status = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  return network_status_node;
}

static attribute_store_node_t
  get_attribute_store_node_from_node_id(zwave_node_id_t node_id)
{
  return attribute_store_get_node_child_by_value(home_id_node,
                                                 ATTRIBUTE_NODE_ID,
                                                 REPORTED_ATTRIBUTE,
                                                 (const uint8_t *)&node_id,
                                                 sizeof(node_id),
                                                 0);
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  // Init the test helpers
  zpc_attribute_store_test_helper_create_network();
  contiki_test_helper_init();
  attribute_timeouts_init();

  // Register our stub
  zwave_send_nop_to_node_Stub(&zwave_send_nop_to_node_stub);
  on_send_nop_completed               = NULL;
  zwave_send_nop_to_node_return_value = SL_STATUS_OK;
  zwave_send_nop_to_node_return_value = SL_STATUS_OK;
  send_nop_calls                      = 0;
  expected_node_id                    = 0;

  // Init the failing node monitor
  failing_node_monitor_init();
  contiki_test_helper_run(1);
}

void test_monitoring_failed_al_node_happy_case()
{
  // Configure the node to be "Always listening"
  uint8_t listening_protocol
    = ZWAVE_NODE_INFO_LISTENING_PROTOCOL_LISTENING_MASK;
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
                                     &listening_protocol,
                                     sizeof(listening_protocol));
  contiki_test_helper_run(MAX_PING_TIME_INTERVAL);

  // Set the node to just included
  attribute_store_node_t network_status_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS, node_id_node);
  NodeStateNetworkStatus network_status
    = ZCL_NODE_STATE_NETWORK_STATUS_COMMISIONING_STARTED;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  contiki_test_helper_run(MAX_PING_TIME_INTERVAL);

  // Make it to unavailable, nothing should happen
  network_status = ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  contiki_test_helper_run(MAX_PING_TIME_INTERVAL);

  // Make it to an invalid value, nothing should happen
  network_status = 10000;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  contiki_test_helper_run(MAX_PING_TIME_INTERVAL);

  // Make it to a interviewing, nothing should happen
  network_status = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_INTERVIEWING;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  contiki_test_helper_run(MAX_PING_TIME_INTERVAL);

  // Now start monitoring the node:
  network_status = ZCL_NODE_STATE_NETWORK_STATUS_OFFLINE;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  // Now we expect the NOP to be queued:
  expected_node_id = node_id;
  contiki_test_helper_run(AL_NODE_PING_TIME_INTERVAL);
  TEST_ASSERT_EQUAL(1, send_nop_calls);
  // Nothing should happen also if we fast forward very far:
  contiki_test_helper_run(MAX_PING_TIME_INTERVAL);
  TEST_ASSERT_EQUAL(1, send_nop_calls);

  TEST_ASSERT_NOT_NULL(on_send_nop_completed);
  on_send_nop_completed(0, NULL, (void *)((intptr_t)node_id_node));

  // Second stop should be 8 seconds later
  contiki_test_helper_run(AL_NODE_PING_TIME_INTERVAL * AL_NODE_PING_TIME_FACTOR
                          - 1);
  TEST_ASSERT_EQUAL(1, send_nop_calls);

  contiki_test_helper_run(2);
  TEST_ASSERT_EQUAL(2, send_nop_calls);

  on_send_nop_completed(0, NULL, (void *)((intptr_t)node_id_node));

  // Third stop should be 16 seconds later
  contiki_test_helper_run(AL_NODE_PING_TIME_INTERVAL * AL_NODE_PING_TIME_FACTOR
                            * AL_NODE_PING_TIME_FACTOR
                          - 1);
  TEST_ASSERT_EQUAL(2, send_nop_calls);

  contiki_test_helper_run(2);
  TEST_ASSERT_EQUAL(3, send_nop_calls);

  on_send_nop_completed(0, NULL, (void *)((intptr_t)node_id_node));

  // finally it is no longer failing:
  network_status = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  // Nothing happens after 32 seconds
  contiki_test_helper_run(MAX_PING_TIME_INTERVAL);
  TEST_ASSERT_EQUAL(3, send_nop_calls);
}

void test_monitoring_failed_fl_node_happy_case()
{
  // Configure the node to be FL
  uint8_t listening_protocol = 0;
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
                                     &listening_protocol,
                                     sizeof(listening_protocol));
  uint8_t optional_protocol
    = ZWAVE_NODE_INFO_OPTIONAL_PROTOCOL_SENSOR_1000MS_MASK;
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL,
                                     &optional_protocol,
                                     sizeof(optional_protocol));
  contiki_test_helper_run(MAX_PING_TIME_INTERVAL);

  // Set the node to interviewing
  attribute_store_node_t network_status_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS, node_id_node);
  NodeStateNetworkStatus network_status = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_INTERVIEWING;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  contiki_test_helper_run(MAX_PING_TIME_INTERVAL);

  // Make it to Interview fail, it will start monitoring
  network_status = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_NON_FUNCTIONAL;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  // Now we expect the NOP to be queued:
  contiki_test_helper_run(FL_NODE_PING_TIME_INTERVAL - 1);
  expected_node_id = node_id;
  TEST_ASSERT_EQUAL(0, send_nop_calls);
  contiki_test_helper_run(2);
  TEST_ASSERT_EQUAL(1, send_nop_calls);
  // Nothing should happen also if we fast forward very far:
  contiki_test_helper_run(MAX_PING_TIME_INTERVAL);
  TEST_ASSERT_EQUAL(1, send_nop_calls);

  TEST_ASSERT_NOT_NULL(on_send_nop_completed);
  on_send_nop_completed(0, NULL, (void *)((intptr_t)node_id_node));

  // Second retry:
  contiki_test_helper_run(FL_NODE_PING_TIME_INTERVAL * FL_NODE_PING_TIME_FACTOR
                          - 1);
  TEST_ASSERT_EQUAL(1, send_nop_calls);
  contiki_test_helper_run(2);
  TEST_ASSERT_EQUAL(2, send_nop_calls);

  on_send_nop_completed(0, NULL, (void *)((intptr_t)node_id_node));

  // Third retry:
  contiki_test_helper_run(FL_NODE_PING_TIME_INTERVAL * FL_NODE_PING_TIME_FACTOR
                            * FL_NODE_PING_TIME_FACTOR
                          - 1);
  TEST_ASSERT_EQUAL(2, send_nop_calls);
  contiki_test_helper_run(2);
  TEST_ASSERT_EQUAL(3, send_nop_calls);

  on_send_nop_completed(0, NULL, (void *)((intptr_t)node_id_node));

  // finally it is no longer failing:
  network_status = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_INTERVIEWING;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  // Nothing happens again:
  contiki_test_helper_run(MAX_PING_TIME_INTERVAL);
  TEST_ASSERT_EQUAL(3, send_nop_calls);
}

void test_monitoring_failed_2_al_nodes_with_back_off()
{
  // Create node 2 and 3:
  attribute_store_node_t network_status_node_2 = create_al_node(200);
  attribute_store_node_t network_status_node_3 = create_al_node(300);

  contiki_test_helper_run(MAX_PING_TIME_INTERVAL);
  // set both nodes as failing:
  NodeStateNetworkStatus network_status
    = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_NON_FUNCTIONAL;
  attribute_store_set_reported(network_status_node_2,
                               &network_status,
                               sizeof(network_status));
  attribute_store_set_reported(network_status_node_3,
                               &network_status,
                               sizeof(network_status));

  // Only 1 NOP will be queued:
  contiki_test_helper_run(AL_NODE_PING_TIME_INTERVAL - 1);
  expected_node_id = 200;
  TEST_ASSERT_EQUAL(0, send_nop_calls);

  contiki_test_helper_run(3);
  TEST_ASSERT_EQUAL(1, send_nop_calls);
  contiki_test_helper_run(NOP_REQUEUE_BACK_OFF_MS - 10);
  TEST_ASSERT_EQUAL(1, send_nop_calls);

  // Get the callback, we will queue the next nop:
  TEST_ASSERT_NOT_NULL(on_send_nop_completed);
  on_send_nop_completed(
    0,
    NULL,
    (void *)((intptr_t)get_attribute_store_node_from_node_id(200)));
  expected_node_id = 300;

  TEST_ASSERT_EQUAL(1, send_nop_calls);
  contiki_test_helper_run(20);
  TEST_ASSERT_EQUAL(2, send_nop_calls);
}

void test_monitoring_failed_al_node_max_interval()
{
  // Create node 2:
  attribute_store_node_t network_status_node_2 = create_al_node(1050);

  contiki_test_helper_run(MAX_PING_TIME_INTERVAL);
  // set both nodes as failing:
  NodeStateNetworkStatus network_status
    = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_NON_FUNCTIONAL;
  attribute_store_set_reported(network_status_node_2,
                               &network_status,
                               sizeof(network_status));

  // Only 1 NOP will be queued:
  clock_time_t expected_interval = AL_NODE_PING_TIME_INTERVAL;
  expected_node_id               = 1050;
  int expected_call_number       = 0;
  TEST_ASSERT_EQUAL(expected_call_number, send_nop_calls);
  while (expected_interval < MAX_PING_TIME_INTERVAL) {
    contiki_test_helper_run(expected_interval - 1);
    TEST_ASSERT_EQUAL(expected_call_number, send_nop_calls);
    contiki_test_helper_run(3);
    expected_call_number += 1;
    TEST_ASSERT_EQUAL(expected_call_number, send_nop_calls);

    // Send data callback:
    TEST_ASSERT_NOT_NULL(on_send_nop_completed);
    on_send_nop_completed(
      0,
      NULL,
      (void *)((intptr_t)get_attribute_store_node_from_node_id(1050)));
    expected_interval *= AL_NODE_PING_TIME_FACTOR;
  }

  // Now it's using the Max interval:
  contiki_test_helper_run(MAX_PING_TIME_INTERVAL - 1);
  TEST_ASSERT_EQUAL(expected_call_number, send_nop_calls);
  contiki_test_helper_run(3);
  expected_call_number += 1;
  TEST_ASSERT_EQUAL(expected_call_number, send_nop_calls);

  // Get the callback, we will queue the next nop:
  TEST_ASSERT_NOT_NULL(on_send_nop_completed);
  on_send_nop_completed(
    0,
    NULL,
    (void *)((intptr_t)get_attribute_store_node_from_node_id(1050)));

  // Try once more, still using the max:
  contiki_test_helper_run(MAX_PING_TIME_INTERVAL - 1);
  TEST_ASSERT_EQUAL(expected_call_number, send_nop_calls);
  contiki_test_helper_run(3);
  expected_call_number += 1;
  TEST_ASSERT_EQUAL(expected_call_number, send_nop_calls);

  // Get the callback, we will queue the next nop:
  TEST_ASSERT_NOT_NULL(on_send_nop_completed);
  on_send_nop_completed(
    0,
    NULL,
    (void *)((intptr_t)get_attribute_store_node_from_node_id(1050)));
}

void test_monitoring_failed_al_ping_interval_gets_deleted_while_transmitting_nop()
{
  // Create node 2:
  attribute_store_node_t network_status_node_2 = create_al_node(2);

  contiki_test_helper_run(MAX_PING_TIME_INTERVAL);
  // Mark the node as failing:
  NodeStateNetworkStatus network_status = ZCL_NODE_STATE_NETWORK_STATUS_OFFLINE;
  attribute_store_set_reported(network_status_node_2,
                               &network_status,
                               sizeof(network_status));

  // Pass the time where we want to send the frame:
  contiki_test_helper_run(AL_NODE_PING_TIME_INTERVAL - 1);
  expected_node_id = 2;
  TEST_ASSERT_EQUAL(0, send_nop_calls);

  contiki_test_helper_run(3);
  TEST_ASSERT_EQUAL(1, send_nop_calls);

  attribute_store_node_t node_id_2_node
    = attribute_store_get_first_parent_with_type(network_status_node_2,
                                                 ATTRIBUTE_NODE_ID);
  attribute_store_node_t ping_interval_node
    = attribute_store_get_node_child_by_type(
      node_id_2_node,
      ATTRIBUTE_ZWAVE_FAILING_NODE_PING_INTERVAL,
      0);

  attribute_store_delete_node(ping_interval_node);

  // Get the callback, we will queue the next nop:
  TEST_ASSERT_NOT_NULL(on_send_nop_completed);
  on_send_nop_completed(
    0,
    NULL,
    (void *)((intptr_t)get_attribute_store_node_from_node_id(2)));

  // Nothing more should happen:
  contiki_test_helper_run(MAX_PING_TIME_INTERVAL + 1);
  TEST_ASSERT_EQUAL(1, send_nop_calls);
}

void test_monitoring_failed_al_network_status_gets_deleted_while_transmitting_nop()
{
  // Create node 2:
  attribute_store_node_t network_status_node_2 = create_al_node(21);

  contiki_test_helper_run(MAX_PING_TIME_INTERVAL);
  // Mark the node as failing:
  NodeStateNetworkStatus network_status
    = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_NON_FUNCTIONAL;
  attribute_store_set_reported(network_status_node_2,
                               &network_status,
                               sizeof(network_status));

  // Pass the time where we want to send the frame:
  contiki_test_helper_run(AL_NODE_PING_TIME_INTERVAL - 1);
  expected_node_id = 21;
  TEST_ASSERT_EQUAL(0, send_nop_calls);
  contiki_test_helper_run(3);
  TEST_ASSERT_EQUAL(1, send_nop_calls);

  attribute_store_delete_node(network_status_node_2);

  // Get the callback, we will queue the next nop:
  TEST_ASSERT_NOT_NULL(on_send_nop_completed);
  on_send_nop_completed(
    0,
    NULL,
    (void *)((intptr_t)get_attribute_store_node_from_node_id(2)));

  // NOthing more should happen:
  contiki_test_helper_run(MAX_PING_TIME_INTERVAL + 1);
  TEST_ASSERT_EQUAL(1, send_nop_calls);
}

void test_monitoring_failed_al_network_status_gets_deleted_after_transmitting_nop()
{
  // Create node 2:
  attribute_store_node_t network_status_node_2 = create_al_node(30);

  contiki_test_helper_run(MAX_PING_TIME_INTERVAL);
  // Mark the node as failing:
  NodeStateNetworkStatus network_status
    = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_NON_FUNCTIONAL;
  attribute_store_set_reported(network_status_node_2,
                               &network_status,
                               sizeof(network_status));

  // Pass the time where we want to send the frame:
  contiki_test_helper_run(AL_NODE_PING_TIME_INTERVAL - 1);
  expected_node_id = 30;
  TEST_ASSERT_EQUAL(0, send_nop_calls);
  contiki_test_helper_run(3);
  TEST_ASSERT_EQUAL(1, send_nop_calls);

  // Get the callback, we will queue the next nop:
  TEST_ASSERT_NOT_NULL(on_send_nop_completed);
  on_send_nop_completed(
    0,
    NULL,
    (void *)((intptr_t)get_attribute_store_node_from_node_id(2)));

  attribute_store_delete_node(network_status_node_2);
  // Nothing more should happen:
  contiki_test_helper_run(MAX_PING_TIME_INTERVAL + 1);
  TEST_ASSERT_EQUAL(1, send_nop_calls);
}

void test_monitoring_failed_unknown_operating_mode_not_monitored()
{
  // Add a network status
  attribute_store_node_t network_status_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS, node_id_node);
  NodeStateNetworkStatus network_status
    = ZCL_NODE_STATE_NETWORK_STATUS_COMMISIONING_STARTED;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  contiki_test_helper_run(MAX_PING_TIME_INTERVAL);

  // Mark the node as failing:
  network_status = ZCL_NODE_STATE_NETWORK_STATUS_OFFLINE;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  // Nothing should happen
  contiki_test_helper_run(MAX_PING_TIME_INTERVAL + 1);
  contiki_test_helper_run(MAX_PING_TIME_INTERVAL + 1);
  TEST_ASSERT_EQUAL(0, send_nop_calls);
}

void test_monitoring_failed_tx_queue_rejects_nop()
{
  // Create node 20 and 3:
  attribute_store_node_t network_status_node_20 = create_al_node(20);
  attribute_store_node_t network_status_node_3  = create_al_node(3);

  contiki_test_helper_run(MAX_PING_TIME_INTERVAL);
  // set both nodes as failing:
  NodeStateNetworkStatus network_status
    = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_NON_FUNCTIONAL;
  attribute_store_set_reported(network_status_node_20,
                               &network_status,
                               sizeof(network_status));

  // Wait 1 second before the other one gets failing
  contiki_test_helper_run(1000);
  attribute_store_set_reported(network_status_node_3,
                               &network_status,
                               sizeof(network_status));

  zwave_send_nop_to_node_return_value = SL_STATUS_FAIL;
  expected_node_id                    = 20;
  TEST_ASSERT_EQUAL(0, send_nop_calls);
  contiki_test_helper_run(AL_NODE_PING_TIME_INTERVAL - 1);
  // first nop get rejected
  TEST_ASSERT_EQUAL(1, send_nop_calls);

  // second nop gets accepted
  expected_node_id                    = 3;
  zwave_send_nop_to_node_return_value = SL_STATUS_OK;
  contiki_test_helper_run(3);
  TEST_ASSERT_EQUAL(2, send_nop_calls);

  // Nothing more should happen until send_data callback for NOP 3.
  contiki_test_helper_run(AL_NODE_PING_TIME_INTERVAL - 1);
  TEST_ASSERT_EQUAL(2, send_nop_calls);
}

void test_monitoring_failed_al_node_already_has_a_ping_interval()
{
  // Create node 99:
  attribute_store_node_t network_status_99_node = create_al_node(99);
  attribute_store_node_t node_id_99_node
    = attribute_store_get_first_parent_with_type(network_status_99_node,
                                                 ATTRIBUTE_NODE_ID);

  contiki_test_helper_run(MAX_PING_TIME_INTERVAL);

  // Create a ping interval value:
  clock_time_t ping_interval = AL_NODE_PING_TIME_INTERVAL
                               * AL_NODE_PING_TIME_FACTOR
                               * AL_NODE_PING_TIME_FACTOR;
  attribute_store_set_child_reported(node_id_99_node,
                                     ATTRIBUTE_ZWAVE_FAILING_NODE_PING_INTERVAL,
                                     &ping_interval,
                                     sizeof(ping_interval));

  // set the node as failing:
  NodeStateNetworkStatus network_status
    = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_NON_FUNCTIONAL;
  attribute_store_set_reported(network_status_99_node,
                               &network_status,
                               sizeof(network_status));

  // Now it should respect the pre-defined interval:
  contiki_test_helper_run(ping_interval - 1);

  expected_node_id = 99;
  TEST_ASSERT_EQUAL(0, send_nop_calls);
  contiki_test_helper_run(2);
  TEST_ASSERT_EQUAL(1, send_nop_calls);
}

void test_monitoring_failed_al_node_undefined_node_id()
{
  // Create node 989:
  attribute_store_node_t network_status_989_node = create_al_node(989);
  attribute_store_node_t node_id_989_node
    = attribute_store_get_first_parent_with_type(network_status_989_node,
                                                 ATTRIBUTE_NODE_ID);

  contiki_test_helper_run(MAX_PING_TIME_INTERVAL);

  attribute_store_undefine_reported(node_id_989_node);

  // set the node as failing:
  NodeStateNetworkStatus network_status
    = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_NON_FUNCTIONAL;
  attribute_store_set_reported(network_status_989_node,
                               &network_status,
                               sizeof(network_status));

  // Now it should not do anything.
  contiki_test_helper_run(MAX_PING_TIME_INTERVAL + 1);
  TEST_ASSERT_EQUAL(0, send_nop_calls);
}

void test_monitoring_failed_al_node_undefined_node_id_after_scheduling_send_nop()
{
  // Create node 7:
  attribute_store_node_t network_status_7_node = create_al_node(7);
  attribute_store_node_t node_id_7_node
    = attribute_store_get_first_parent_with_type(network_status_7_node,
                                                 ATTRIBUTE_NODE_ID);

  // set the node as failing:
  NodeStateNetworkStatus network_status = ZCL_NODE_STATE_NETWORK_STATUS_OFFLINE;
  attribute_store_set_reported(network_status_7_node,
                               &network_status,
                               sizeof(network_status));

  // Undefine the NodeID now
  attribute_store_undefine_reported(node_id_7_node);

  // It will try to queue to NodeID 0, which should not harm.
  expected_node_id = 0;
  contiki_test_helper_run(MAX_PING_TIME_INTERVAL + 1);
  TEST_ASSERT_EQUAL(1, send_nop_calls);
}
