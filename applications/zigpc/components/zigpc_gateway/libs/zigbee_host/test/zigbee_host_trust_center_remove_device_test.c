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
#define SIGNED_ENUM
#include "af_mock.h"
#include "af-main_mock.h"
#include "af-security_mock.h"
#include "network-creator-security_mock.h"
#include "address-table_mock.h"
#include "zigbee-device-common_mock.h"

#include "zigbee_host.h"
#include "zigbee_host_common.h"
#include "test_zigbee_host_callbacks_mock.h"

/* Function prototypes tested */
struct zigbeeHostState z3gwState;

/**
 * @brief Setup the test suite (called once before all test_xxx functions are
 * called)
 *
 */
void suiteSetUp()
{
  static struct zigbeeHostCallbacks callbacks = {
    .onNetworkDeviceLeaveResponse = callback_onNetworkDeviceLeaveResponse,
  };
  z3gwState.callbacks = &callbacks;
}

/**
 * @brief Teardown the test suite (called once after all test_xxx functions are
 * called)
 *
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/**
 * @brief Setup before each test case
 *
 */
void setUp() {}

/**
 * @brief Teardown after each test case
 *
 */
void tearDown() {}

/**
 * @brief Test scenario of handling invalit input
 *
 */
void test_trust_center_remove_device_should_handle_invalid_argument()
{
  // ARRANGE
  EmberStatus status;

  // ACT
  status = zigbeeHostNetworkDeviceLeaveRequest(NULL);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_BAD_ARGUMENT, status);
}

/**
 * @brief Test success path
 *
 */
void test_trust_center_remove_device_should_use_device_leave_api()
{
  // ARRANGE
  EmberEUI64 eui64    = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF};
  EmberNodeId node_id = 0x1234;

  // ARRANGE

  emberAfLookupAddressTableEntryByEui64_ExpectAndReturn(eui64, 0);
  emberAfPluginAddressTableLookupNodeIdByIndex_ExpectAndReturn(0, node_id);

  emberLeaveRequest_ExpectAndReturn(
    node_id,
    eui64,
    0x00,
    (EMBER_APS_OPTION_RETRY | EMBER_APS_OPTION_ENABLE_ROUTE_DISCOVERY
     | EMBER_APS_OPTION_ENABLE_ADDRESS_DISCOVERY),
    EMBER_SUCCESS);

  // ACT
  EmberStatus status = zigbeeHostNetworkDeviceLeaveRequest(eui64);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(status, EMBER_SUCCESS);
}

void test_trust_center_remove_device_should_return_on_device_not_found()
{
  // ARRANGE
  EmberEUI64 eui64 = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF};

  emberAfLookupAddressTableEntryByEui64_ExpectAndReturn(
    eui64,
    EMBER_NULL_ADDRESS_TABLE_INDEX);

  // ACT
  EmberStatus status = zigbeeHostNetworkDeviceLeaveRequest(eui64);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(status, EMBER_INVALID_CALL);
}

void test_device_left_callback_should_invoke_device_removed_callback(void)
{
  // ARRANGE
  EmberEUI64 eui64 = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF};
  callback_onNetworkDeviceLeaveResponse_Expect(eui64);

  // ACT
  emAfPluginGatewayInterfaceTrustCenterLeaveHandler(eui64);

  // ASSERT (Handled by CMock)
}
