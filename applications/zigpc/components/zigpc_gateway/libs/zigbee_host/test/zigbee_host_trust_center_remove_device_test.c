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
  sl_status_t status;

  // ACT
  status = zigbeeHostNetworkDeviceLeaveRequest(NULL);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_PARAMETER, status);
}

/**
 * @brief Test success path
 *
 */
void test_trust_center_remove_device_should_use_device_leave_api()
{
  // ARRANGE
  sl_802154_long_addr_t eui64    = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF};
  sl_802154_short_addr_t node_id = 0x1234;

  // ARRANGE

  sl_zigbee_af_lookup_address_table_entry_by_eui64_ExpectAndReturn(eui64, 0);
  sl_zigbee_af_address_table_lookup_node_id_by_index_ExpectAndReturn(0, node_id);

  sl_zigbee_leave_request_ExpectAndReturn(
    node_id,
    eui64,
    0x00,
    (SL_ZIGBEE_APS_OPTION_RETRY | SL_ZIGBEE_APS_OPTION_ENABLE_ROUTE_DISCOVERY
     | SL_ZIGBEE_APS_OPTION_ENABLE_ADDRESS_DISCOVERY),
    SL_STATUS_OK );

  // ACT
  sl_status_t status = zigbeeHostNetworkDeviceLeaveRequest(eui64);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(status, SL_STATUS_OK );
}

void test_trust_center_remove_device_should_return_on_device_not_found()
{
  // ARRANGE
  sl_802154_long_addr_t eui64 = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF};

  sl_zigbee_af_lookup_address_table_entry_by_eui64_ExpectAndReturn(
    eui64,
    SL_ZIGBEE_NULL_ADDRESS_TABLE_INDEX);

  // ACT
  sl_status_t status = zigbeeHostNetworkDeviceLeaveRequest(eui64);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(status, SL_STATUS_INVALID_STATE );
}

void test_device_left_callback_should_invoke_device_removed_callback(void)
{
  // ARRANGE
  sl_802154_long_addr_t eui64 = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF};
  callback_onNetworkDeviceLeaveResponse_Expect(eui64);

  // ACT
  emAfPluginGatewayInterfaceTrustCenterLeaveHandler(eui64);

  // ASSERT (Handled by CMock)
}
