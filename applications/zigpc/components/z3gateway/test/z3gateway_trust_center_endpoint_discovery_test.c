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
#define SIGNED_ENUM
#include "af_mock.h"
#include "af-security_mock.h"
#include "network-creator-security_mock.h"
#include "address-table_mock.h"
#include "device-table_mock.h"

#include "z3gateway.h"
#include "z3gateway_common.h"
#include "test_z3gateway_callbacks_mock.h"

#define INSTALL_CODE_SIZE (16 + 2)

/* Function prototypes tested */
void emberAfPluginDeviceTableNewDeviceCallback(const EmberEUI64 newNodeEui64,
                                               uint8_t endpoint);
void emberAfPluginDeviceTableRejoinDeviceCallback(EmberEUI64 newNodeEui64,
                                                  uint8_t endpoint);

struct z3gatewayState z3gwState;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  static struct z3gatewayCallbacks callbacks
    = {.onTrustCenterDeviceEndpointDiscovered
       = callback_onTrustCenterDeviceEndpointDiscovered};
  z3gwState.callbacks = &callbacks;
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp() {}

void tearDown() {}

void helper_arrange_for_endpoint_discovered_callback(EmberEUI64 test_eui64,
                                                     uint8_t test_endpoint)
{
  // ARRANGE
  EmberNodeId node_id                      = 0x5678;
  EmberAfPluginDeviceTableEntry test_entry = {
    .endpoint                = test_endpoint,
    .deviceId                = 0x1,
    .clusterIds              = {1, 2},
    .clusterOutStartPosition = 1,
  };

  emberAfLookupAddressTableEntryByEui64_ExpectAndReturn(test_eui64, 1);
  emberAfPluginAddressTableLookupNodeIdByIndex_ExpectAndReturn(1, node_id);

  emAfDeviceTableFindIndexNodeIdEndpoint_ExpectAndReturn(node_id,
                                                         test_endpoint,
                                                         55);
  emberAfDeviceTableFindDeviceTableEntry_ExpectAndReturn(55, &test_entry);
  callback_onTrustCenterDeviceEndpointDiscovered_Expect(test_eui64, NULL);
  callback_onTrustCenterDeviceEndpointDiscovered_IgnoreArg_endpointInfo();
}

void test_start_discovery_should_call_device_table()
{
  // ARRANGE
  EmberEUI64 test_eui64 = {0xA, 0xB, 0xC, 0xD};
  emAfDeviceTableStartDeviceDiscovery_ExpectAndReturn(test_eui64,
                                                      EMBER_SUCCESS);

  // ACT
  EmberStatus status = z3gatewayTrustCenterStartDiscovery(test_eui64);

  // ASSERT (Handled by CMock)
  TEST_ASSERT_EQUAL(EMBER_SUCCESS, status);
}

void test_device_table_new_dev_callback_should_invoke_endpoint_discovered_callback()
{
  // ARRANGE
  EmberEUI64 test_eui64 = {0x1, 0x2, 0x3, 0x4};
  uint8_t test_endpoint = 0x3;
  helper_arrange_for_endpoint_discovered_callback(test_eui64, test_endpoint);

  // ACT
  emberAfPluginDeviceTableNewDeviceCallback(test_eui64, test_endpoint);

  // ASSERT (Handled by CMock)
}

void test_device_table_rejoin_dev_callback_should_invoke_endpoint_discovered_callback()
{
  // ARRANGE
  EmberEUI64 test_eui64 = {0x5, 0x6, 0x7, 0x8};
  uint8_t test_endpoint = 0x32;
  helper_arrange_for_endpoint_discovered_callback(test_eui64, test_endpoint);

  // ACT
  emberAfPluginDeviceTableRejoinDeviceCallback(test_eui64, test_endpoint);

  // ASSERT (Handled by CMock)
}
