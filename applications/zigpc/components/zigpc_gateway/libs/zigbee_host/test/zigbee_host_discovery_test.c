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
  EmberEUI64 eui64    = {1, 2, 3, 4, 5, 6, 7, 8};
  EmberNodeId node_id = 0x1234;

  // ARRANGE
  emberAfLookupAddressTableEntryByEui64_ExpectAndReturn(eui64, 0);
  emberAfPluginAddressTableLookupNodeIdByIndex_ExpectAndReturn(0, node_id);

  emberAfFindActiveEndpoints_ExpectAndReturn(
    node_id,
    zigbeeHostZdoActiveEndpointsResponseCallback,
    EMBER_SUCCESS);

  // ACT
  EmberStatus status = zigbeeHostZdoActiveEndpointsRequest(eui64);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_SUCCESS, status);
}

void test_zigbee_host_clusters_discovery_sanity(void)
{
  EmberEUI64 eui64    = {1, 2, 3, 4, 5, 6, 7, 8};
  uint8_t endpoint    = 6;
  EmberNodeId node_id = 0x1234;

  // ARRANGE
  emberAfLookupAddressTableEntryByEui64_ExpectAndReturn(eui64, 0);
  emberAfPluginAddressTableLookupNodeIdByIndex_ExpectAndReturn(0, node_id);

  emberAfFindClustersByDeviceAndEndpoint_ExpectAndReturn(
    node_id,
    endpoint,
    zigbeeHostZdoSimpleDescriptorResponseCallback,
    EMBER_SUCCESS);

  // ACT
  EmberStatus status = zigbeeHostZdoSimpleDescriptorRequest(eui64, endpoint);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_SUCCESS, status);
}

void test_zigbee_host_cb_device_endpoints_discovery_sanity(void)
{
  EmberEUI64 eui64                = {2, 2, 3, 4, 5, 6, 7, 8};
  uint8_t activeEpCount           = 3;
  uint8_t activeEpList[3]         = {1, 3, 6};
  EmberAfEndpointList activeEpRes = {
    .count = activeEpCount,
    .list  = activeEpList,
  };
  EmberAfServiceDiscoveryResult res = {
    .status = EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
    .zdoRequestClusterId = ACTIVE_ENDPOINTS_REQUEST,
    .responseData        = (const void *)&activeEpRes,
  };

  // ARRANGE
  emberAfGetCurrentSenderEui64_ExpectAndReturn(NULL, EMBER_SUCCESS);
  emberAfGetCurrentSenderEui64_IgnoreArg_address();
  emberAfGetCurrentSenderEui64_ReturnMemThruPtr_address(eui64,
                                                        sizeof(EmberEUI64));

  callback_onDeviceEndpointsDiscovered_Expect(eui64,
                                              activeEpCount,
                                              activeEpList);

  // ACT
  zigbeeHostZdoActiveEndpointsResponseCallback(&res);
}

void test_zigbee_host_cb_endpoint_clusters_discovery_sanity(void)
{
  EmberEUI64 eui64                 = {2, 2, 3, 4, 5, 6, 7, 8};
  uint8_t endpoint                 = 5;
  uint16_t deviceId                = 0x2301;
  uint16_t inClusters[2]           = {2, 4};
  uint16_t outClusters[1]          = {6};
  EmberAfClusterList simpleDescRes = {
    .inClusterCount  = 2,
    .inClusterList   = inClusters,
    .outClusterCount = 1,
    .outClusterList  = outClusters,
    .endpoint        = endpoint,
    .deviceId        = deviceId,
  };
  EmberAfServiceDiscoveryResult res = {
    .status = EMBER_AF_UNICAST_SERVICE_DISCOVERY_COMPLETE_WITH_RESPONSE,
    .zdoRequestClusterId = SIMPLE_DESCRIPTOR_REQUEST,
    .responseData        = (const void *)&simpleDescRes,
  };

  EmberAfClusterList expectedInfo = {
    .endpoint        = simpleDescRes.endpoint,
    .deviceId        = simpleDescRes.deviceId,
    .inClusterCount  = simpleDescRes.inClusterCount,
    .inClusterList   = (uint16_t *)simpleDescRes.inClusterList,
    .outClusterCount = simpleDescRes.outClusterCount,
    .outClusterList  = (uint16_t *)simpleDescRes.outClusterList,
  };

  // ARRANGE
  emberAfGetCurrentSenderEui64_ExpectAndReturn(NULL, EMBER_SUCCESS);
  emberAfGetCurrentSenderEui64_IgnoreArg_address();
  emberAfGetCurrentSenderEui64_ReturnMemThruPtr_address(eui64,
                                                        sizeof(EmberEUI64));

  callback_onEndpointClustersDiscovered_Expect(eui64, &expectedInfo);

  // ACT
  zigbeeHostZdoSimpleDescriptorResponseCallback(&res);
}
