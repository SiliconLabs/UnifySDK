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
#include "network_monitor.h"

// Unify Components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute_resolver.h"
#include "attribute_timeouts.h"
#include "sl_log.h"

// ZPC Components
#include "zwave_utils.h"

// Interfaces
#include "zwave_node_id_definitions.h"
#include "zwave_command_class_wake_up_types.h"
#include "ucl_definitions.h"

// Mock includes
#include "zwave_controller_callbacks_mock.h"
#include "zwave_controller_storage_mock.h"
#include "zwave_network_management_mock.h"
#include "zpc_config_mock.h"
#include "zwapi_protocol_basis_mock.h"
#include "uic_mqtt_mock.h"

// Test helpers
#include "unity.h"
#include "zpc_attribute_store_test_helper.h"
#include "contiki_test_helper.h"

const zwave_controller_callbacks_t *zwave_callbacks                = NULL;
const zwave_controller_storage_callback_t *zwave_storage_callbacks = NULL;

// Mocks from this components
void network_monitor_store_span_table_data() {}
void network_monitor_restore_span_table_data() {}
void network_monitor_store_mpan_table_data() {}
void network_monitor_restore_mpan_table_data() {}

/// Stubs
static sl_status_t zwave_controller_register_callbacks_stub(
  const zwave_controller_callbacks_t *callbacks, int n)
{
  zwave_callbacks = callbacks;
  return SL_STATUS_OK;
}

static sl_status_t zwave_controller_storage_callback_register_stub(
  const zwave_controller_storage_callback_t *callbacks, int cmock_num_calls)
{
  zwave_storage_callbacks = callbacks;
  return SL_STATUS_OK;
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
  // Network configuration from the ZPC attribute store network helper
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);
  zwave_network_management_get_node_id_IgnoreAndReturn(zpc_node_id);
  zwave_network_management_get_granted_keys_IgnoreAndReturn(0);

  // It will try to query the Z-Wave API for the node list.
  // Just pretend it failed, so we do not need to mock a bunch of Z-Wave API calls
  zwapi_get_full_node_list_IgnoreAndReturn(SL_STATUS_FAIL);

  // Initialize all variables
  zwave_callbacks         = NULL;
  zwave_storage_callbacks = NULL;

  // Stub setup
  zwave_controller_register_callbacks_Stub(
    zwave_controller_register_callbacks_stub);

  zwave_controller_storage_callback_register_Stub(
    zwave_controller_storage_callback_register_stub);

  // Init the test helpers
  zpc_attribute_store_test_helper_create_network();
  contiki_test_helper_init();
  attribute_timeouts_init();

  // Init the network monitor
  network_state_monitor_init();
  process_start(&network_monitor_process, NULL);
  contiki_test_helper_run(1);
}

void test_monitoring_failed_al_node()
{
  // Mock the configuration
  zpc_config_t test_configuration              = {0};
  test_configuration.accepted_transmit_failure = 2;
  zpc_get_config_IgnoreAndReturn(&test_configuration);

  // Configure the node to be "Always listening"
  uint8_t listening_protocol
    = ZWAVE_NODE_INFO_LISTENING_PROTOCOL_LISTENING_MASK;
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
                                     &listening_protocol,
                                     sizeof(listening_protocol));
  // Set the node to Online Functional
  attribute_store_node_t network_status_node
    = attribute_store_add_node(ATTRIBUTE_NETWORK_STATUS, node_id_node);
  node_state_topic_state_t network_status = NODE_STATE_TOPIC_STATE_INCLUDED;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  // Make a transmission failure:
  zwave_callbacks->on_frame_transmission(false, NULL, node_id);
  contiki_test_helper_run(0);

  // We want 2 transmit failures before going offline, so it should still
  // be online
  network_status = 0xFF;
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  TEST_ASSERT_EQUAL(NODE_STATE_TOPIC_STATE_INCLUDED, network_status);

  // Make a second transmit failure, that's it, you are offline my friend:
  zwave_callbacks->on_frame_transmission(false, NULL, node_id);
  contiki_test_helper_run(0);
  network_status = 0xFF;
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  TEST_ASSERT_EQUAL(NODE_STATE_TOPIC_STATE_OFFLINE, network_status);

  // Make a second transmit success, back online
  zwave_callbacks->on_frame_transmission(true, NULL, node_id);
  contiki_test_helper_run(0);
  network_status = 0xFF;
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  TEST_ASSERT_EQUAL(NODE_STATE_TOPIC_STATE_INCLUDED, network_status);
}

void test_monitoring_failed_al_node_re_interview()
{
  // Mock the configuration
  zpc_config_t test_configuration              = {0};
  test_configuration.accepted_transmit_failure = 2;
  zpc_get_config_IgnoreAndReturn(&test_configuration);

  // Configure the node to be "Always listening"
  uint8_t listening_protocol
    = ZWAVE_NODE_INFO_LISTENING_PROTOCOL_LISTENING_MASK;
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
                                     &listening_protocol,
                                     sizeof(listening_protocol));
  // Set the node to Interviewing
  attribute_store_node_t network_status_node
    = attribute_store_add_node(ATTRIBUTE_NETWORK_STATUS, node_id_node);
  node_state_topic_state_t network_status = NODE_STATE_TOPIC_INTERVIEWING;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  // Make a transmission failure:
  zwave_callbacks->on_frame_transmission(false, NULL, node_id);
  contiki_test_helper_run(0);

  // We want 2 transmit failures before going offline, so it should still
  // be online
  network_status = 0xFF;
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  TEST_ASSERT_EQUAL(NODE_STATE_TOPIC_INTERVIEWING, network_status);

  // Make a second transmit failure, that's it, you are offline my friend:
  zwave_callbacks->on_frame_transmission(false, NULL, node_id);
  contiki_test_helper_run(0);
  network_status = 0xFF;
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  TEST_ASSERT_EQUAL(NODE_STATE_TOPIC_STATE_INTERVIEW_FAIL, network_status);

  // Make a second transmit success, back to interviewing
  zwave_callbacks->on_frame_transmission(true, NULL, node_id);
  contiki_test_helper_run(0);
  network_status = 0xFF;
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  TEST_ASSERT_EQUAL(NODE_STATE_TOPIC_INTERVIEWING, network_status);
}

void test_monitoring_failed_tx_with_nl_node()
{
  // Mock the configuration
  zpc_config_t test_configuration              = {0};
  test_configuration.accepted_transmit_failure = 2;
  zpc_get_config_IgnoreAndReturn(&test_configuration);

  // Configure the node to be "Non listening"
  uint8_t listening_protocol = 0;
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
                                     &listening_protocol,
                                     sizeof(listening_protocol));
  uint8_t optional_protocol = 0;
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL,
                                     &optional_protocol,
                                     sizeof(optional_protocol));

  // Set the node to Online Functional
  attribute_store_node_t network_status_node
    = attribute_store_add_node(ATTRIBUTE_NETWORK_STATUS, node_id_node);
  node_state_topic_state_t network_status = NODE_STATE_TOPIC_STATE_INCLUDED;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  // Save a successful last frame transmission:
  contiki_test_helper_run(1000);
  zwave_callbacks->on_frame_transmission(true, NULL, node_id);
  contiki_test_helper_run(0);

  // Make a bunch of transmission failures. Should not matter since it is an NL node
  zwave_callbacks->on_frame_transmission(false, NULL, node_id);
  contiki_test_helper_run(0);
  zwave_callbacks->on_frame_transmission(false, NULL, node_id);
  contiki_test_helper_run(0);
  zwave_callbacks->on_frame_transmission(false, NULL, node_id);
  contiki_test_helper_run(0);

  //  Still online functional (just asleep)
  network_status = 0xFF;
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  TEST_ASSERT_EQUAL(NODE_STATE_TOPIC_STATE_INCLUDED, network_status);

  // Try to see if we consider the node asleep again if a Tx fail happens
  // more than 10s after the last tx/rx success.
  contiki_test_helper_run(9999);
  zwave_callbacks->on_frame_transmission(false, NULL, node_id);
  contiki_test_helper_run(0);
  TEST_ASSERT_FALSE(is_node_or_parent_paused(node_id_node));

  contiki_test_helper_run(1001);
  zwave_callbacks->on_frame_transmission(false, NULL, node_id);
  contiki_test_helper_run(0);
  TEST_ASSERT_TRUE(is_node_or_parent_paused(node_id_node));
}

void test_monitoring_failed_nl_node()
{
  // Mock the configuration
  zpc_config_t test_configuration                 = {0};
  test_configuration.missing_wake_up_notification = 2;
  test_configuration.accepted_transmit_failure    = 2;
  zpc_get_config_IgnoreAndReturn(&test_configuration);

  // Configure the node to be "Non listening"
  uint8_t listening_protocol = 0;
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
                                     &listening_protocol,
                                     sizeof(listening_protocol));
  uint8_t optional_protocol = 0;
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL,
                                     &optional_protocol,
                                     sizeof(optional_protocol));

  // Set the node to Online Functional
  attribute_store_node_t network_status_node
    = attribute_store_add_node(ATTRIBUTE_NETWORK_STATUS, node_id_node);
  node_state_topic_state_t network_status = NODE_STATE_TOPIC_STATE_INCLUDED;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  // Set up the Wake Up Interval
  attribute_store_node_t wake_up_setting_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_SETTING,
                               endpoint_id_node);

  attribute_store_node_t wake_up_interval_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_INTERVAL,
                               wake_up_setting_node);
  wake_up_interval_t wake_up_interval = 20;
  attribute_store_set_reported(wake_up_interval_node,
                               &wake_up_interval,
                               sizeof(wake_up_interval));

  // Save a successful last frame transmission:
  contiki_test_helper_run(1000);
  zwave_callbacks->on_frame_transmission(true, NULL, node_id);
  contiki_test_helper_run(0);

  // Still online functional (just asleep)
  network_status = 0xFF;
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  TEST_ASSERT_EQUAL(NODE_STATE_TOPIC_STATE_INCLUDED, network_status);

  // Go to the deadline minus 1 ms
  contiki_test_helper_run(test_configuration.missing_wake_up_notification
                            * wake_up_interval * CLOCK_SECOND
                          - 1);

  network_status = 0xFF;
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  TEST_ASSERT_EQUAL(NODE_STATE_TOPIC_STATE_INCLUDED, network_status);

  // Now it was too long without hearing from the node, mark it as failing:
  contiki_test_helper_run(2);
  network_status = 0xFF;
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  TEST_ASSERT_EQUAL(NODE_STATE_TOPIC_STATE_OFFLINE, network_status);

  // At any frame reception, we put it back online functional:
  zwave_callbacks->on_rx_frame_received(node_id);
  contiki_test_helper_run(0);
  network_status = 0xFF;
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  TEST_ASSERT_EQUAL(NODE_STATE_TOPIC_STATE_INCLUDED, network_status);
}

void test_monitoring_failed_nl_node_changing_wake_up_interval()
{
  // Mock the configuration
  zpc_config_t test_configuration                 = {0};
  test_configuration.missing_wake_up_notification = 2;
  test_configuration.accepted_transmit_failure    = 2;
  zpc_get_config_IgnoreAndReturn(&test_configuration);

  // Configure the node to be "Non listening"
  uint8_t listening_protocol = 0;
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
                                     &listening_protocol,
                                     sizeof(listening_protocol));
  uint8_t optional_protocol = 0;
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL,
                                     &optional_protocol,
                                     sizeof(optional_protocol));

  // Set the node to Online Functional
  attribute_store_node_t network_status_node
    = attribute_store_add_node(ATTRIBUTE_NETWORK_STATUS, node_id_node);
  node_state_topic_state_t network_status = NODE_STATE_TOPIC_STATE_INCLUDED;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  // Set up the Wake Up Interval
  attribute_store_node_t wake_up_setting_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_SETTING,
                               endpoint_id_node);

  attribute_store_node_t wake_up_interval_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_WAKE_UP_INTERVAL,
                               wake_up_setting_node);
  wake_up_interval_t wake_up_interval = 20;
  attribute_store_set_reported(wake_up_interval_node,
                               &wake_up_interval,
                               sizeof(wake_up_interval));

  // Save a successful last frame transmission:
  contiki_test_helper_run(1000);
  zwave_callbacks->on_frame_transmission(true, NULL, node_id);
  contiki_test_helper_run(0);

  // Still online functional (just asleep)
  network_status = 0xFF;
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  TEST_ASSERT_EQUAL(NODE_STATE_TOPIC_STATE_INCLUDED, network_status);

  // Go to the deadline minus 1 ms
  contiki_test_helper_run(test_configuration.missing_wake_up_notification
                            * wake_up_interval * CLOCK_SECOND
                          - 1);

  // Update the Wake Up interval:
  wake_up_interval = 0;
  attribute_store_set_reported(wake_up_interval_node,
                               &wake_up_interval,
                               sizeof(wake_up_interval));

  // Now it will never be marked as offline:
  contiki_test_helper_run(500000000);
  network_status = 0xFF;
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  TEST_ASSERT_EQUAL(NODE_STATE_TOPIC_STATE_INCLUDED, network_status);
}

void test_change_node_network_status_to_interviewing_on_nif_creation()
{
  // Set the node to Online Functional
  attribute_store_node_t network_status_node
    = attribute_store_add_node(ATTRIBUTE_NETWORK_STATUS, node_id_node);
  node_state_topic_state_t network_status = NODE_STATE_TOPIC_STATE_INCLUDED;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  // Create a nif:
  attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);
  contiki_test_helper_run(0);

  // Verify that the node is interviewing now:
  network_status = 0xFF;
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  TEST_ASSERT_EQUAL(NODE_STATE_TOPIC_INTERVIEWING, network_status);
}

void test_change_node_network_status_to_interview_fail_on_nif_creation()
{
  // Set the node to Online Functional
  attribute_store_node_t network_status_node
    = attribute_store_add_node(ATTRIBUTE_NETWORK_STATUS, node_id_node);
  node_state_topic_state_t network_status = NODE_STATE_TOPIC_STATE_OFFLINE;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  // Create a nif:
  attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);
  contiki_test_helper_run(0);

  // Verify that the node is "Interview Fail" now:
  network_status = 0xFF;
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  TEST_ASSERT_EQUAL(NODE_STATE_TOPIC_STATE_INTERVIEW_FAIL, network_status);
}

void test_change_node_network_status_to_interview_on_nif_creation_with_unknown_previous_value()
{
  // Create a network status but leave it undefined.
  attribute_store_node_t network_status_node
    = attribute_store_add_node(ATTRIBUTE_NETWORK_STATUS, node_id_node);

  // Create a nif:
  attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);
  contiki_test_helper_run(0);

  // Verify that the node is "Interview Fail" now:
  node_state_topic_state_t network_status = 0xFF;
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  TEST_ASSERT_EQUAL(NODE_STATE_TOPIC_INTERVIEWING, network_status);
}

void test_do_not_put_node_online_if_not_offline()
{
  // Mock the configuration
  zpc_config_t test_configuration = {};
  zpc_get_config_IgnoreAndReturn(&test_configuration);

  // Create a network status but leave it undefined.
  attribute_store_node_t network_status_node
    = attribute_store_add_node(ATTRIBUTE_NETWORK_STATUS, node_id_node);
  // Set the network status to something not offline:
  node_state_topic_state_t network_status = NODE_STATE_TOPIC_STATE_UNAVAILABLE;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  // Simulate a incoming frame:
  TEST_ASSERT_NOT_NULL(zwave_callbacks);
  TEST_ASSERT_NOT_NULL(zwave_callbacks->on_rx_frame_received);
  zwave_callbacks->on_rx_frame_received(node_id);
  contiki_test_helper_run(0);

  // Check that the network status is unchanged:
  network_status = 0xFF;
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  TEST_ASSERT_EQUAL(NODE_STATE_TOPIC_STATE_UNAVAILABLE, network_status);

  // Simulate a outgoing frame:
  TEST_ASSERT_NOT_NULL(zwave_callbacks->on_frame_transmission);
  zwave_callbacks->on_frame_transmission(true, NULL, node_id);
  contiki_test_helper_run(0);

  // Check that the network status is unchanged:
  network_status = 0xFF;
  attribute_store_get_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
  TEST_ASSERT_EQUAL(NODE_STATE_TOPIC_STATE_UNAVAILABLE, network_status);
}

void test_network_monitor_handle_rx_frame_event_for_unknown_node()
{
  // Nothing should happen when frame transmissions are reported with a
  // NodeID that does not exist in the attribute store.
  zwave_callbacks->on_frame_transmission(false, NULL, 0xAA);
  contiki_test_helper_run(0);

  zwave_callbacks->on_frame_transmission(true, NULL, 0xAA);
  contiki_test_helper_run(0);
}

void test_network_monitor_on_node_deleted()
{
  TEST_ASSERT_NOT_NULL(zwave_callbacks);
  TEST_ASSERT_NOT_NULL(zwave_callbacks->on_node_deleted);

  // Nothing bad should happen with non-existing nodes:
  zwave_callbacks->on_node_deleted(0);
  zwave_callbacks->on_node_deleted(0xAA);
  uic_mqtt_unretain_Expect("ucl/by-unid/zw-CAFECAFE-00AA");
  contiki_test_helper_run(0);

  // Now delete the supporting node:
  TEST_ASSERT_TRUE(attribute_store_node_exists(node_id_node));
  uic_mqtt_unretain_Expect("ucl/by-unid/zw-CAFECAFE-0004");
  zwave_callbacks->on_node_deleted(node_id);
  contiki_test_helper_run(0);
  TEST_ASSERT_FALSE(attribute_store_node_exists(node_id_node));
}

void test_network_monitor_on_node_added_no_dsk()
{
  TEST_ASSERT_NOT_NULL(zwave_callbacks);
  TEST_ASSERT_NOT_NULL(zwave_callbacks->on_node_added);

  // Nothing should happen with non-existing nodes:
  // clang-format off
  const zwave_node_info_t nif = {.listening_protocol = 12,.optional_protocol = 13};
  // clang-format on
  const zwave_dsk_t dsk                = {};
  const zwave_keyset_t granted_keys    = 0x87;
  const zwave_kex_fail_type_t kex_fail = 0x65;
  zwave_callbacks->on_node_added(SL_STATUS_OK,
                                 &nif,
                                 node_id,
                                 dsk,
                                 granted_keys,
                                 kex_fail,
                                 PROTOCOL_ZWAVE_LONG_RANGE);
  contiki_test_helper_run(0);

  // Now time to check the attribute store:
  // Check the DSK was not saved, the node has no S2 capability.
  attribute_store_node_t dsk_node
    = attribute_store_get_node_child_by_type(node_id_node, ATTRIBUTE_S2_DSK, 0);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, dsk_node);

  // Check the Kex FAIL was saved correctly
  attribute_store_node_t kex_fail_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_KEX_FAIL_TYPE,
                                             0);
  zwave_kex_fail_type_t received_kex_fail = 0;
  attribute_store_get_reported(kex_fail_node,
                               &received_kex_fail,
                               sizeof(received_kex_fail));
  TEST_ASSERT_EQUAL(kex_fail, received_kex_fail);

  // Check the Granted keys were saved correctly
  attribute_store_node_t granted_keys_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_GRANTED_SECURITY_KEYS,
                                             0);
  zwave_keyset_t received_granted_keys = 0;
  attribute_store_get_reported(granted_keys_node,
                               &received_granted_keys,
                               sizeof(received_granted_keys));
  TEST_ASSERT_EQUAL(granted_keys, received_granted_keys);

  // Check the NIF flags (Protocol Listening)
  attribute_store_node_t protocol_listening_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
                                             0);
  uint8_t received_protocol_listening = 0xFF;
  attribute_store_get_reported(protocol_listening_node,
                               &received_protocol_listening,
                               sizeof(received_protocol_listening));
  TEST_ASSERT_EQUAL(nif.listening_protocol, received_protocol_listening);

  // Check the NIF flags (Optional protocol)
  attribute_store_node_t optional_protocol_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL,
                                             0);
  uint8_t received_optional_protocol = 0xFF;
  attribute_store_get_reported(optional_protocol_node,
                               &received_optional_protocol,
                               sizeof(received_optional_protocol));
  TEST_ASSERT_EQUAL(nif.optional_protocol, received_optional_protocol);

  // Check the Inclusion protocol:
  attribute_store_node_t inclusion_protocol_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                                             0);
  zwave_protocol_t received_protocol = 0xFF;
  attribute_store_get_reported(inclusion_protocol_node,
                               &received_protocol,
                               sizeof(received_protocol));
  TEST_ASSERT_EQUAL(PROTOCOL_ZWAVE_LONG_RANGE, received_protocol);
}

void test_network_monitor_on_node_added_with_dsk()
{
  TEST_ASSERT_NOT_NULL(zwave_callbacks);
  TEST_ASSERT_NOT_NULL(zwave_callbacks->on_node_added);

  // Indicate that the node is S2 capable, so we save the DSK
  zwave_security_validation_set_node_as_s2_capable(node_id);

  // Nothing should happen with non-existing nodes:
  // clang-format off
  const zwave_node_info_t nif = {.listening_protocol = 12,.optional_protocol = 13};
  const zwave_dsk_t dsk = {0x00,0x00,0xb7,0x64,0x73,0xda,0x04,0xf6,0x19,0x12,0x6d,0xe3,0x6f,0x50,0xdf,0xef};
  // clang-format on
  const zwave_keyset_t granted_keys         = 0x00;
  const zwave_kex_fail_type_t kex_fail      = 0x02;
  const zwave_protocol_t inclusion_protocol = PROTOCOL_ZWAVE_LONG_RANGE + 2;
  zwave_callbacks->on_node_added(SL_STATUS_OK,
                                 &nif,
                                 node_id,
                                 dsk,
                                 granted_keys,
                                 kex_fail,
                                 inclusion_protocol);
  contiki_test_helper_run(0);

  // Now time to check the attribute store:
  // Check the DSK was saved correctly
  attribute_store_node_t dsk_node
    = attribute_store_get_node_child_by_type(node_id_node, ATTRIBUTE_S2_DSK, 0);
  zwave_dsk_t received_dsk = {};
  attribute_store_get_reported(dsk_node, &received_dsk, sizeof(received_dsk));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(dsk, received_dsk, sizeof(dsk));

  // Check the Kex FAIL was saved correctly
  attribute_store_node_t kex_fail_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_KEX_FAIL_TYPE,
                                             0);
  zwave_kex_fail_type_t received_kex_fail = 0;
  attribute_store_get_reported(kex_fail_node,
                               &received_kex_fail,
                               sizeof(received_kex_fail));
  TEST_ASSERT_EQUAL(kex_fail, received_kex_fail);

  // Check the Granted keys were saved correctly
  attribute_store_node_t granted_keys_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_GRANTED_SECURITY_KEYS,
                                             0);
  zwave_keyset_t received_granted_keys = 0;
  attribute_store_get_reported(granted_keys_node,
                               &received_granted_keys,
                               sizeof(received_granted_keys));
  TEST_ASSERT_EQUAL(granted_keys, received_granted_keys);

  // Check the NIF flags (Protocol Listening)
  attribute_store_node_t protocol_listening_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
                                             0);
  uint8_t received_protocol_listening = 0xFF;
  attribute_store_get_reported(protocol_listening_node,
                               &received_protocol_listening,
                               sizeof(received_protocol_listening));
  TEST_ASSERT_EQUAL(nif.listening_protocol, received_protocol_listening);

  // Check the NIF flags (Optional protocol)
  attribute_store_node_t optional_protocol_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL,
                                             0);
  uint8_t received_optional_protocol = 0xFF;
  attribute_store_get_reported(optional_protocol_node,
                               &received_optional_protocol,
                               sizeof(received_optional_protocol));
  TEST_ASSERT_EQUAL(nif.optional_protocol, received_optional_protocol);

  // Check the Inclusion protocol:
  attribute_store_node_t inclusion_protocol_node
    = attribute_store_get_node_child_by_type(node_id_node,
                                             ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                                             0);
  zwave_protocol_t received_protocol = 0xFF;
  attribute_store_get_reported(inclusion_protocol_node,
                               &received_protocol,
                               sizeof(received_protocol));
  TEST_ASSERT_EQUAL(inclusion_protocol, received_protocol);
}