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

// Component includes
#include "zigpc_discovery.hpp"
#include "zigpc_discovery_callbacks.hpp"
#include "zigpc_discovery_events.hpp"
#include "zigpc_discovery_process.hpp"

void zigpc_discovery_on_device_discovered(void *data);
void zigpc_discovery_on_endpoint_discovered(void *data);

extern "C" {

// Shared UIC includes
#include <unity.h>
#include <string.h>
#include <sys/process.h>
#include <contiki_test_helper.h>

// ZigPC includes
#include "zigpc_gateway_notify.h"

// Component includes
#include "zigpc_discovery.h"

// Mocks
#include "zigpc_datastore_mock.h"
#include "zigpc_gateway_mock.h"
#include "zigpc_gateway_notify_mock.h"
#include "zigpc_discovery_test_callbacks_mock.h"

PROCESS_NAME(zigpc_discovery_process);

/**
 * Setup the test suite (called once before all test_xxx functions are called)
 *
 */
void suiteSetUp(void) {}

/**
 * Teardown the test suite (called once after all test_xxx functions are called)
 *
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/**
 * Setup before each test case
 *
 */
void setUp(void)
{
  contiki_test_helper_init();
  zigpc_gateway_register_observer_IgnoreAndReturn(SL_STATUS_OK);
  zigpc_discovery_fixt_setup();
  contiki_test_helper_run_once(0);
}

/**
 * Teardown after each test case
 *
 */
void tearDown(void)
{
  process_exit(&zigpc_discovery_process);
  zigpc_discovery_fixt_shutdown();
  contiki_test_helper_run_once(0);
}

/**
 * @brief Test helper to expect datastore read/modify/write behaviour.
 *
 * @param eui64       Reference to EUI64 to use.
 * @param device_data Reference to device data to use.
 */
static void helper_expect_datastore_device_read_modify_write(
  zigbee_eui64_t &eui64, zigpc_device_data_t &device_data)
{
  zigpc_datastore_read_device_ExpectWithArrayAndReturn(eui64,
                                                       sizeof(zigbee_eui64_t),
                                                       NULL,
                                                       1,
                                                       SL_STATUS_OK);
  zigpc_datastore_read_device_IgnoreArg_data();
  zigpc_datastore_read_device_ReturnThruPtr_data(&device_data);

  zigpc_datastore_write_device_ExpectWithArrayAndReturn(eui64,
                                                        sizeof(zigbee_eui64_t),
                                                        &device_data,
                                                        1,
                                                        SL_STATUS_OK);
}

/**
 * @brief Helper to expect device_interview request behaviour.
 *
 * @param eui64       Reference to EUI64 to use.
 * @param device_data Reference to device data to use.
 */
static void
  helper_expect_device_discover_request(zigbee_eui64_t &eui64,
                                        zigpc_device_data_t &device_data)
{
  zigpc_datastore_remove_device_children_ExpectWithArrayAndReturn(
    eui64,
    sizeof(zigbee_eui64_t),
    SL_STATUS_OK);

  device_data.network_status = ZIGBEE_NODE_STATUS_INTERVIEWING;

  helper_expect_datastore_device_read_modify_write(eui64, device_data);

  zigpc_gateway_discover_device_ExpectWithArrayAndReturn(eui64,
                                                         sizeof(zigbee_eui64_t),
                                                         SL_STATUS_OK);
}

/**
 * @brief Helper to expect endpoint_interview request behaviour.
 *
 * @param eui64       Reference to EUI64 to use.
 * @param device_data Reference to device data to use.
 */
static void
  helper_expect_endpoint_discover_request(zigbee_eui64_t &eui64,
                                          zigbee_endpoint_id_t &endpoint_id,
                                          zigpc_device_data_t &device_data)
{
  device_data.network_status = ZIGBEE_NODE_STATUS_INTERVIEWING;

  helper_expect_datastore_device_read_modify_write(eui64, device_data);

  zigpc_gateway_discover_endpoint_ExpectWithArrayAndReturn(
    eui64,
    sizeof(zigbee_eui64_t),
    endpoint_id,
    SL_STATUS_OK);
}

void test_zigpc_discovery_interview_device_request(void)
{
  zigbee_eui64_t eui64 {0x01, 0x5F, 0xAC, 0xBB, 0xE3, 0x9A, 0x00, 0x20};
  zigpc_device_data_t device_data {};

  // ARRANGE
  helper_expect_device_discover_request(eui64, device_data);

  test_discovery_status_callback_Expect(
    zigbee_eui64_to_uint(eui64),
    zigpc_discovery_status::DISCOVERY_START);

  // ACT
  sl_status_t status
    = zigpc_discovery_interview_device(zigbee_eui64_to_uint(eui64),
                                       test_discovery_status_callback);

  // ASSERT

  // validate request
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  TEST_ASSERT_NOT_NULL(
    dynamic_cast<zigpc_discovery::events::DeviceDiscoverRequest *>(
      zigpc_discovery::process::peek_event()->get()));

  // process advance to run event handler
  contiki_test_helper_run_once(0);
}

void test_zigpc_discovery_interview_device_timeout_reached(void)
{
  zigbee_eui64_t eui64 {0xF2, 0xAF, 0x3C, 0xBB, 0xE3, 0x9A, 0x04, 0x21};
  zigpc_device_data_t device_data {};

  // Part 1: Request device discovery

  // ARRANGE 1
  helper_expect_device_discover_request(eui64, device_data);

  test_discovery_status_callback_Expect(
    zigbee_eui64_to_uint(eui64),
    zigpc_discovery_status::DISCOVERY_START);

  // ACT 1
  sl_status_t status
    = zigpc_discovery_interview_device(zigbee_eui64_to_uint(eui64),
                                       test_discovery_status_callback);

  // ASSERT 1 (validate before/after contiki_process advance)
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  TEST_ASSERT_NOT_NULL(
    dynamic_cast<zigpc_discovery::events::DeviceDiscoverRequest *>(
      zigpc_discovery::process::peek_event()->get()));
  contiki_test_helper_run_once(0);

  // Part 2: Timeout reach and no response received

  // ARRANGE 2 (expect device status to be set to INTERVIEW_FAIL)
  zigpc_device_data_t timeout_device_data {};
  timeout_device_data.network_status = ZIGBEE_NODE_STATUS_INTERVIEW_FAIL;

  helper_expect_datastore_device_read_modify_write(eui64, timeout_device_data);

  test_discovery_status_callback_Expect(
    zigbee_eui64_to_uint(eui64),
    zigpc_discovery_status::DEVICE_DISCOVERY_FAIL);

  // ACT 2 (advance contiki runtime until timeout reached)
  contiki_test_helper_run_once(
    zigpc_discovery::events::DISCOVERY_REQUEST_TIMEOUT);

  // ASSERT 2 (Handled by CMock)
}

void test_zigpc_discovery_device_discover_response_requests_first_endpoint_discovery(
  void)
{
  zigbee_eui64_t eui64 {0x02, 0xDE, 0x3C, 0xBA, 0x23, 0x9C, 0xCC, 0x8E};
  std::vector<zigbee_endpoint_id_t> endpoints = {1, 3, 6, 2};

  // ARRANGE

  for (zigbee_endpoint_id_t &endpoint_id: endpoints) {
    zigpc_datastore_remove_endpoint_ExpectWithArrayAndReturn(
      eui64,
      sizeof(zigbee_eui64_t),
      endpoint_id,
      SL_STATUS_OK);
    zigpc_datastore_create_endpoint_ExpectWithArrayAndReturn(
      eui64,
      sizeof(zigbee_eui64_t),
      endpoint_id,
      SL_STATUS_OK);
  }

  zigpc_device_data_t ep_disc_device_data {};
  ep_disc_device_data.network_status       = ZIGBEE_NODE_STATUS_INTERVIEWING;
  ep_disc_device_data.endpoint_total_count = endpoints.size();
  ep_disc_device_data.endpoint_discovered_count = 0;

  helper_expect_datastore_device_read_modify_write(eui64, ep_disc_device_data);

  // ACT
  zigpc_gateway_on_node_discovered_t ev {};
  memcpy(ev.eui64, eui64, sizeof(zigbee_eui64_t));
  ev.endpoint_count = endpoints.size();
  ev.endpoint_list  = endpoints.data();

  zigpc_discovery_on_device_discovered((void *)&ev);

  // ASSERT
  TEST_ASSERT_NOT_NULL(
    dynamic_cast<zigpc_discovery::events::DeviceDiscoverResponse *>(
      zigpc_discovery::process::peek_event()->get()));
  contiki_test_helper_run_once(0);
  // expect endpoint discovery event to be queued
  TEST_ASSERT_NOT_NULL(
    dynamic_cast<zigpc_discovery::events::EndpointDiscoverRequest *>(
      zigpc_discovery::process::peek_event()->get()));
}

void test_zigpc_discovery_interview_endpoint_request(void)
{
  zigbee_eui64_t eui64 {0xA2, 0x8F, 0x72, 0x11, 0x1B, 0x32, 0x01, 0x10};
  zigbee_endpoint_id_t endpoint_id = 1;
  zigpc_device_data_t device_data {};

  // ARRANGE
  helper_expect_endpoint_discover_request(eui64, endpoint_id, device_data);

  // ACT
  sl_status_t status
    = zigpc_discovery_partial_interview_endpoint(zigbee_eui64_to_uint(eui64),
                                                 endpoint_id);

  // ASSERT

  // validate request
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  TEST_ASSERT_NOT_NULL(
    dynamic_cast<zigpc_discovery::events::EndpointDiscoverRequest *>(
      zigpc_discovery::process::peek_event()->get()));

  // process advance to run event handler
  contiki_test_helper_run_once(0);
}

void test_zigpc_discovery_interview_endpoint_timeout_reached(void)
{
  zigbee_eui64_t eui64 {0x33, 0x73, 0xAA, 0xB0, 0x22, 0x00, 0x10, 0xFD};
  zigbee_endpoint_id_t endpoint_id = 4;
  zigpc_device_data_t device_data {};

  // Part 1: Request device discovery

  // ARRANGE 1
  helper_expect_endpoint_discover_request(eui64, endpoint_id, device_data);

  // ACT 1
  sl_status_t status
    = zigpc_discovery_add_listener(test_discovery_status_callback);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  status
    = zigpc_discovery_partial_interview_endpoint(zigbee_eui64_to_uint(eui64),
                                                 endpoint_id);

  // ASSERT 1 (validate before/after contiki_process advance)
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
  TEST_ASSERT_NOT_NULL(
    dynamic_cast<zigpc_discovery::events::EndpointDiscoverRequest *>(
      zigpc_discovery::process::peek_event()->get()));
  contiki_test_helper_run_once(0);

  // Part 2: Timeout reach and no response received

  // ARRANGE 2 (expect device status to be set to INTERVIEW_FAIL)
  zigpc_device_data_t timeout_device_data {};
  timeout_device_data.network_status = ZIGBEE_NODE_STATUS_INTERVIEW_FAIL;

  helper_expect_datastore_device_read_modify_write(eui64, timeout_device_data);

  test_discovery_status_callback_Expect(
    zigbee_eui64_to_uint(eui64),
    zigpc_discovery_status::ENDPOINT_DISCOVERY_FAIL);

  // ACT 2 (advance contiki runtime until timeout reached)
  contiki_test_helper_run_once(
    zigpc_discovery::events::DISCOVERY_REQUEST_TIMEOUT);

  // ASSERT 2 (Handled by CMock)
}

void test_zigpc_discovery_ep_disc_resp_starts_next_endpoint_discovery(void)
{
  zigbee_eui64_t eui64 {0x0A, 0x0D, 0x0E, 0x00, 0x93, 0x44, 0x55, 0xF3};
  zigbee_endpoint_id_t endpoint_id              = 2;
  zigbee_endpoint_id_t next_endpoint_id         = 6;
  std::vector<zcl_cluster_id_t> server_clusters = {0x0001, 0x0006};
  std::vector<zcl_cluster_id_t> client_clusters = {0x0020};

  // ARRANGE

  for (zcl_cluster_id_t cluster_id: server_clusters) {
    zigpc_datastore_create_cluster_ExpectWithArrayAndReturn(
      eui64,
      sizeof(zigbee_eui64_t),
      endpoint_id,
      ZCL_CLUSTER_SERVER_SIDE,
      cluster_id,
      SL_STATUS_OK);
  }

  for (zcl_cluster_id_t cluster_id: client_clusters) {
    zigpc_datastore_create_cluster_ExpectWithArrayAndReturn(
      eui64,
      sizeof(zigbee_eui64_t),
      endpoint_id,
      ZCL_CLUSTER_CLIENT_SIDE,
      cluster_id,
      SL_STATUS_OK);
  }

  zigpc_datastore_find_endpoint_by_index_ExpectWithArrayAndReturn(
    eui64,
    sizeof(zigbee_eui64_t),
    1,
    nullptr,
    sizeof(zigbee_endpoint_id_t),
    SL_STATUS_OK);
  zigpc_datastore_find_endpoint_by_index_IgnoreArg_endpoint_id();
  zigpc_datastore_find_endpoint_by_index_ReturnMemThruPtr_endpoint_id(
    &next_endpoint_id,
    sizeof(zigbee_endpoint_id_t));

  zigpc_device_data_t ep_disc_device_data {};
  ep_disc_device_data.network_status = ZIGBEE_NODE_STATUS_INTERVIEWING;
  ep_disc_device_data.endpoint_discovered_count = 0;
  ep_disc_device_data.endpoint_total_count      = 2;

  helper_expect_datastore_device_read_modify_write(eui64, ep_disc_device_data);

  // ACT
  sl_status_t status
    = zigpc_discovery_add_listener(test_discovery_status_callback);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);

  zigpc_gateway_on_node_endpoint_discovered_t ev {};
  zigbee_endpoint_t &ep = ev.endpoint;
  memcpy(ev.eui64, eui64, sizeof(zigbee_eui64_t));
  ep.endpoint_id = endpoint_id;

  ep.cluster_count = 0U;
  for (zcl_cluster_id_t cluster_id: server_clusters) {
    ep.cluster_list[ep.cluster_count].cluster_id = cluster_id;
    ep.cluster_count++;
  }
  ep.client_cluster_count = 0U;
  for (zcl_cluster_id_t cluster_id: client_clusters) {
    ep.client_cluster_list[ep.client_cluster_count].cluster_id = cluster_id;
    ep.client_cluster_count++;
  }

  zigpc_discovery_on_endpoint_discovered((void *)&ev);

  // ASSERT
  TEST_ASSERT_NOT_NULL(
    dynamic_cast<zigpc_discovery::events::EndpointDiscoverResponse *>(
      zigpc_discovery::process::peek_event()->get()));
  contiki_test_helper_run_once(0);

  // ASSERT 2 (expect endpoint discovery event to be queued)
  TEST_ASSERT_NOT_NULL(
    dynamic_cast<zigpc_discovery::events::EndpointDiscoverRequest *>(
      zigpc_discovery::process::peek_event()->get()));
}

void test_zigpc_discovery_ep_disc_resp_complete_discovery(void)
{
  zigbee_eui64_t eui64 {0xB4, 0x68, 0x9E, 0x30, 0x61, 0xD5, 0xA8, 0x22};
  zigbee_endpoint_id_t endpoint_id = 1;
  std::vector<zcl_cluster_id_t> server_clusters
    = {0x0001, 0x0003, 0x0005, 0x0006, 0x0019};
  std::vector<zcl_cluster_id_t> client_clusters = {};

  // ARRANGE

  for (zcl_cluster_id_t cluster_id: server_clusters) {
    zigpc_datastore_create_cluster_ExpectWithArrayAndReturn(
      eui64,
      sizeof(zigbee_eui64_t),
      endpoint_id,
      ZCL_CLUSTER_SERVER_SIDE,
      cluster_id,
      SL_STATUS_OK);
  }

  for (zcl_cluster_id_t cluster_id: client_clusters) {
    zigpc_datastore_create_cluster_ExpectWithArrayAndReturn(
      eui64,
      sizeof(zigbee_eui64_t),
      endpoint_id,
      ZCL_CLUSTER_CLIENT_SIDE,
      cluster_id,
      SL_STATUS_OK);
  }

  zigpc_device_data_t ep_disc_device_data {};
  ep_disc_device_data.network_status            = ZIGBEE_NODE_STATUS_INCLUDED;
  ep_disc_device_data.endpoint_discovered_count = 1;
  ep_disc_device_data.endpoint_total_count      = 2;

  helper_expect_datastore_device_read_modify_write(eui64, ep_disc_device_data);

  test_discovery_status_callback_Expect(
    zigbee_eui64_to_uint(eui64),
    zigpc_discovery_status::DISCOVERY_SUCCESS);

  // ACT
  sl_status_t status
    = zigpc_discovery_add_listener(test_discovery_status_callback);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);

  zigpc_gateway_on_node_endpoint_discovered_t ev {};
  zigbee_endpoint_t &ep = ev.endpoint;
  memcpy(ev.eui64, eui64, sizeof(zigbee_eui64_t));
  ep.endpoint_id = endpoint_id;

  ep.cluster_count = 0U;
  for (zcl_cluster_id_t cluster_id: server_clusters) {
    ep.cluster_list[ep.cluster_count].cluster_id = cluster_id;
    ep.cluster_count++;
  }
  ep.client_cluster_count = 0U;
  for (zcl_cluster_id_t cluster_id: client_clusters) {
    ep.client_cluster_list[ep.client_cluster_count].cluster_id = cluster_id;
    ep.client_cluster_count++;
  }

  zigpc_discovery_on_endpoint_discovered((void *)&ev);

  // ASSERT
  TEST_ASSERT_NOT_NULL(
    dynamic_cast<zigpc_discovery::events::EndpointDiscoverResponse *>(
      zigpc_discovery::process::peek_event()->get()));
  contiki_test_helper_run_once(0);

  // ASSERT 2 (expect endpoint discovery event to be queued)
  TEST_ASSERT_EQUAL(0, zigpc_discovery::process::queue_size());
}

}  // extern "C"
