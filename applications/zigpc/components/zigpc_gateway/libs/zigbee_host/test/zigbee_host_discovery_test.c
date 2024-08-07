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

#include <unity.h>

// Zigbee stack includes
#define SIGNED_ENUM
#include "af_mock.h"
#include "address-table_mock.h"

#include "zigbee_host.h"
#include "zigbee_host_common.h"
#include "test_zigbee_host_callbacks_mock.h"

extern struct zigbeeHostState z3gwState;

/**
 * @brief  Setup the test suite
 * (called once before all test_xxx functions are called)
 *
 */
void suiteSetUp(void)
{
  static struct zigbeeHostCallbacks callbacks = {
    .onZdoActiveEndpointsResponse  = callback_onDeviceEndpointsDiscovered,
    .onZdoSimpleDescriptorResponse = callback_onEndpointClustersDiscovered,
  };
  z3gwState.callbacks = &callbacks;
}

/**
 * @brief  Teardown the test suite
 * (called once after all test_xxx functions are called)
 *
 * @param num_failures
 * @return int
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/**
 * @brief Setup before each test.
 *
 */
void setUp(void) {}

/**
 * @brief Teardown after each test.
 *
 */
void tearDown(void) {}

void test_zigbee_host_endoints_discovery_sanity(void)
{
  sl_802154_long_addr_t eui64    = {1, 2, 3, 4, 5, 6, 7, 8};
  sl_802154_short_addr_t node_id = 0x1234;

  // ARRANGE
  sl_zigbee_af_lookup_address_table_entry_by_eui64_ExpectAndReturn(eui64, 0);
  sl_zigbee_af_address_table_lookup_node_id_by_index_ExpectAndReturn(0, node_id);

  sl_zigbee_af_find_active_endpoints_ExpectAndReturn(
    node_id,
    zigbeeHostZdoActiveEndpointsResponseCallback,
    SL_STATUS_OK );

  // ACT
  sl_status_t status = zigbeeHostZdoActiveEndpointsRequest(eui64);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK , status);
}

void test_zigbee_host_clusters_discovery_sanity(void)
{
  sl_802154_long_addr_t eui64    = {1, 2, 3, 4, 5, 6, 7, 8};
  uint8_t endpoint    = 6;
  sl_802154_short_addr_t node_id = 0x1234;

  // ARRANGE
  sl_zigbee_af_lookup_address_table_entry_by_eui64_ExpectAndReturn(eui64, 0);
  sl_zigbee_af_address_table_lookup_node_id_by_index_ExpectAndReturn(0, node_id);

  sl_zigbee_af_find_clusters_by_device_and_endpoint_ExpectAndReturn(
    node_id,
    endpoint,
    zigbeeHostZdoSimpleDescriptorResponseCallback,
    SL_STATUS_OK );

  // ACT
  sl_status_t status = zigbeeHostZdoSimpleDescriptorRequest(eui64, endpoint);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK , status);
}

void test_zigbee_host_cb_device_endpoints_discovery_sanity(void)
{
  sl_802154_long_addr_t eui64                = {2, 2, 3, 4, 5, 6, 7, 8};
  uint8_t activeEpCount           = 3;
  uint8_t activeEpList[3]         = {1, 3, 6};
  sl_zigbee_af_endpoint_list_t activeEpRes = {
    .count = activeEpCount,
    .list  = activeEpList,
  };
  sl_zigbee_af_service_discovery_result_t res = {
    .status = SL_ZIGBEE_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
    .zdoRequestClusterId = ACTIVE_ENDPOINTS_REQUEST,
    .responseData        = (const void *)&activeEpRes,
  };

  // ARRANGE
  sl_zigbee_af_get_current_sender_eui64_ExpectAndReturn(NULL, SL_STATUS_OK );
  sl_zigbee_af_get_current_sender_eui64_IgnoreArg_address();
  sl_zigbee_af_get_current_sender_eui64_ReturnMemThruPtr_address(eui64,
                                                        sizeof(sl_802154_long_addr_t));

  callback_onDeviceEndpointsDiscovered_Expect(eui64,
                                              activeEpCount,
                                              activeEpList);

  // ACT
  zigbeeHostZdoActiveEndpointsResponseCallback(&res);
}

void test_zigbee_host_cb_endpoint_clusters_discovery_sanity(void)
{
  sl_802154_long_addr_t eui64                 = {2, 2, 3, 4, 5, 6, 7, 8};
  uint8_t endpoint                 = 5;
  uint16_t deviceId                = 0x2301;
  uint16_t inClusters[2]           = {2, 4};
  uint16_t outClusters[1]          = {6};
  sl_zigbee_af_cluster_list_t simpleDescRes = {
    .inClusterCount  = 2,
    .inClusterList   = inClusters,
    .outClusterCount = 1,
    .outClusterList  = outClusters,
    .endpoint        = endpoint,
    .deviceId        = deviceId,
  };
  sl_zigbee_af_service_discovery_result_t res = {
    .status = SL_ZIGBEE_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
    .zdoRequestClusterId = SIMPLE_DESCRIPTOR_REQUEST,
    .responseData        = (const void *)&simpleDescRes,
  };

  sl_zigbee_af_cluster_list_t expectedInfo = {
    .endpoint        = simpleDescRes.endpoint,
    .deviceId        = simpleDescRes.deviceId,
    .inClusterCount  = simpleDescRes.inClusterCount,
    .inClusterList   = (uint16_t *)simpleDescRes.inClusterList,
    .outClusterCount = simpleDescRes.outClusterCount,
    .outClusterList  = (uint16_t *)simpleDescRes.outClusterList,
  };

  // ARRANGE
  sl_zigbee_af_get_current_sender_eui64_ExpectAndReturn(NULL, SL_STATUS_OK );
  sl_zigbee_af_get_current_sender_eui64_IgnoreArg_address();
  sl_zigbee_af_get_current_sender_eui64_ReturnMemThruPtr_address(eui64,
                                                        sizeof(sl_802154_long_addr_t));

  callback_onEndpointClustersDiscovered_Expect(eui64, &expectedInfo);

  // ACT
  zigbeeHostZdoSimpleDescriptorResponseCallback(&res);
}
