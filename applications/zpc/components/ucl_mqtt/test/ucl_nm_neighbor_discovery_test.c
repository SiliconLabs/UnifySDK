/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "ucl_nm_neighbor_discovery.h"

// Test helpers
#include "contiki_test_helper.h"

// Mocks
#include "zwave_controller_callbacks_mock.h"
#include "zwave_network_management_mock.h"
#include "zwave_utils_mock.h"
#include "attribute_resolver_mock.h"
#include "attribute_store_helper_mock.h"
#include "zpc_attribute_store_network_helper_mock.h"

const zwave_controller_callbacks_t *nm_neighbor_discovery_callbacks;
static attribute_resolver_callback_t on_nl_node_resolution_resumed;

static sl_status_t my_nm_callback_save(const zwave_controller_callbacks_t *cb,
                                       int n)
{
  nm_neighbor_discovery_callbacks = cb;
  return SL_STATUS_OK;
}

void attribute_resolver_set_resolution_resumption_listener_stub(
  attribute_store_node_t node,
  attribute_resolver_callback_t callback,
  int cmock_num_calls)
{
  on_nl_node_resolution_resumed = callback;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void tearDown() {}

/// Called before each and every test
void setUp()
{
  contiki_test_helper_init();

  on_nl_node_resolution_resumed   = NULL;
  nm_neighbor_discovery_callbacks = NULL;
  zwave_controller_register_callbacks_AddCallback(my_nm_callback_save);
  zwave_controller_register_callbacks_ExpectAndReturn(NULL, SL_STATUS_OK);
  zwave_controller_register_callbacks_IgnoreArg_callbacks();
  ucl_nm_neighbor_discovery_init();

  TEST_ASSERT_NOT_NULL(nm_neighbor_discovery_callbacks);
}

void test_ucl_nm_trigger_node_neighbor_update()
{
  zwave_node_id_t node_id_test = 0x33;
  zwave_network_management_get_state_ExpectAndReturn(NM_IDLE);
  zwave_network_management_request_node_neighbor_discovery_ExpectAndReturn(
    node_id_test,
    SL_STATUS_OK);
  zwave_get_inclusion_protocol_ExpectAndReturn(node_id_test, PROTOCOL_ZWAVE);
  zwave_get_operating_mode_ExpectAndReturn(node_id_test, OPERATING_MODE_AL);
  ucl_nm_trigger_node_neighbor_update(node_id_test);
}

void test_ucl_nm_trigger_node_neighbor_update_with_cb_trigger()
{
  zwave_node_id_t node_id_test   = 0x12;
  zwave_node_id_t node_id_test_2 = 0x34;
  zwave_network_management_get_state_ExpectAndReturn(NM_WAITING_FOR_ADD);
  zwave_get_inclusion_protocol_ExpectAndReturn(node_id_test, PROTOCOL_ZWAVE);
  zwave_get_operating_mode_ExpectAndReturn(node_id_test, OPERATING_MODE_AL);
  ucl_nm_trigger_node_neighbor_update(node_id_test);

  zwave_network_management_get_state_ExpectAndReturn(NM_WAITING_FOR_ADD);
  zwave_get_inclusion_protocol_ExpectAndReturn(node_id_test_2, PROTOCOL_ZWAVE);
  zwave_get_operating_mode_ExpectAndReturn(node_id_test_2, OPERATING_MODE_FL);
  ucl_nm_trigger_node_neighbor_update(node_id_test_2);

  zwave_network_management_request_node_neighbor_discovery_ExpectAndReturn(
    node_id_test,
    SL_STATUS_OK);
  zwave_network_management_get_state_ExpectAndReturn(NM_IDLE);
  TEST_ASSERT_NOT_NULL(nm_neighbor_discovery_callbacks->on_state_updated);
  nm_neighbor_discovery_callbacks->on_state_updated(NM_IDLE);

  // Now the discovery is ongoing. We won't start a new one until we get a
  // status update or timeout
  TEST_ASSERT_NOT_NULL(
    nm_neighbor_discovery_callbacks->on_request_neighbor_update);
  nm_neighbor_discovery_callbacks->on_request_neighbor_update(
    UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_STARTED);

  // If we get the network management back to idle, it does not matter at that point
  nm_neighbor_discovery_callbacks->on_state_updated(NM_IDLE);

  // Now we are done with the first neighbor discovery, move on to the next one.
  zwave_network_management_request_node_neighbor_discovery_ExpectAndReturn(
    node_id_test_2,
    SL_STATUS_OK);
  zwave_network_management_get_state_ExpectAndReturn(NM_IDLE);
  nm_neighbor_discovery_callbacks->on_request_neighbor_update(
    UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_COMPLETED);

  // Wait very long, so long that we give up waiting for a status:
  contiki_test_helper_run(UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_DEFAULT_TIMEOUT
                          + 10);
}

void test_unknown_callback_status()
{
  zwave_node_id_t node_id_test   = 500;
  zwave_node_id_t node_id_test_2 = 2;

  zwave_network_management_get_state_ExpectAndReturn(NM_IDLE);
  zwave_get_inclusion_protocol_ExpectAndReturn(node_id_test, PROTOCOL_ZWAVE);
  zwave_get_operating_mode_ExpectAndReturn(node_id_test, OPERATING_MODE_AL);
  zwave_network_management_request_node_neighbor_discovery_ExpectAndReturn(
    node_id_test,
    SL_STATUS_OK);
  ucl_nm_trigger_node_neighbor_update(node_id_test);

  zwave_get_inclusion_protocol_ExpectAndReturn(node_id_test_2, PROTOCOL_ZWAVE);
  zwave_get_operating_mode_ExpectAndReturn(node_id_test_2, OPERATING_MODE_FL);
  ucl_nm_trigger_node_neighbor_update(node_id_test_2);

  // Now the discovery is ongoing. Get a status 0.
  zwave_network_management_get_state_ExpectAndReturn(NM_IDLE);
  zwave_network_management_request_node_neighbor_discovery_ExpectAndReturn(
    node_id_test_2,
    SL_STATUS_OK);
  TEST_ASSERT_NOT_NULL(
    nm_neighbor_discovery_callbacks->on_request_neighbor_update);
  nm_neighbor_discovery_callbacks->on_request_neighbor_update(0);

  // Now we are done with the first neighbor discovery, move on to the next one.
  nm_neighbor_discovery_callbacks->on_request_neighbor_update(
    UCL_NM_REQUEST_NODE_NEIGHBOR_UPDATE_NOT_SUPPORTED);
}

void test_ucl_nm_trigger_node_neighbor_update_long_range_node()
{
  zwave_node_id_t node_id_test = 0x100;
  zwave_get_inclusion_protocol_ExpectAndReturn(node_id_test,
                                               PROTOCOL_ZWAVE_LONG_RANGE);
  ucl_nm_trigger_node_neighbor_update(node_id_test);
}

void test_ucl_nm_trigger_node_neighbor_update_nl_node()
{
  TEST_ASSERT_NULL(on_nl_node_resolution_resumed);
  attribute_resolver_set_resolution_resumption_listener_Stub(
    &attribute_resolver_set_resolution_resumption_listener_stub);

  zwave_node_id_t node_id_test             = 56;
  attribute_store_node_t test_node_id_node = 34;
  zwave_get_inclusion_protocol_ExpectAndReturn(node_id_test, PROTOCOL_ZWAVE);
  zwave_get_operating_mode_ExpectAndReturn(node_id_test, OPERATING_MODE_NL);
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    node_id_test,
    test_node_id_node);

  ucl_nm_trigger_node_neighbor_update(node_id_test);
  TEST_ASSERT_NOT_NULL(on_nl_node_resolution_resumed);

  // tell that we resumed resolution on this NodeID:
  zwave_network_management_get_state_ExpectAndReturn(NM_IDLE);
  attribute_store_get_reported_ExpectAndReturn(test_node_id_node,
                                               NULL,
                                               sizeof(zwave_node_id_t),
                                               SL_STATUS_OK);
  attribute_store_get_reported_IgnoreArg_value();
  attribute_store_get_reported_ReturnThruPtr_value(&node_id_test);
  zwave_network_management_request_node_neighbor_discovery_ExpectAndReturn(
    node_id_test,
    SL_STATUS_OK);
  attribute_resolver_clear_resolution_resumption_listener_Expect(
    test_node_id_node,
    on_nl_node_resolution_resumed);
  on_nl_node_resolution_resumed(test_node_id_node);
}

void test_ucl_nm_trigger_node_neighbor_update_nl_node_zwave_api_fails()
{
  TEST_ASSERT_NULL(on_nl_node_resolution_resumed);
  attribute_resolver_set_resolution_resumption_listener_Stub(
    &attribute_resolver_set_resolution_resumption_listener_stub);

  zwave_node_id_t node_id_test             = 56;
  attribute_store_node_t test_node_id_node = 34;
  zwave_get_inclusion_protocol_ExpectAndReturn(node_id_test, PROTOCOL_ZWAVE);
  zwave_get_operating_mode_ExpectAndReturn(node_id_test, OPERATING_MODE_NL);
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    node_id_test,
    test_node_id_node);

  ucl_nm_trigger_node_neighbor_update(node_id_test);
  TEST_ASSERT_NOT_NULL(on_nl_node_resolution_resumed);

  // tell that we resumed resolution on this NodeID:
  zwave_network_management_get_state_ExpectAndReturn(NM_IDLE);
  attribute_store_get_reported_ExpectAndReturn(test_node_id_node,
                                               NULL,
                                               sizeof(zwave_node_id_t),
                                               SL_STATUS_OK);
  attribute_store_get_reported_IgnoreArg_value();
  attribute_store_get_reported_ReturnThruPtr_value(&node_id_test);
  zwave_network_management_request_node_neighbor_discovery_ExpectAndReturn(
    node_id_test,
    SL_STATUS_FAIL);
  on_nl_node_resolution_resumed(test_node_id_node);
}
