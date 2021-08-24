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
#include "af-security_mock.h"
#include "network-creator-security_mock.h"
#include "device-table_mock.h"

#include "z3gateway.h"
#include "z3gateway_common.h"
#include "test_z3gateway_callbacks_mock.h"

/* Function prototypes tested */
void emberAfPluginDeviceTableDeviceLeftCallback(EmberEUI64 newNodeEui64);

struct z3gatewayState z3gwState;

/**
 * @brief Setup the test suite (called once before all test_xxx functions are
 * called)
 *
 */
void suiteSetUp()
{
  static struct z3gatewayCallbacks callbacks = {
    .onTrustCenterDeviceRemoved = callback_onTrustCenterDeviceRemoved,
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
  status = z3gatewayTrustCenterRemoveDevice(NULL);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_BAD_ARGUMENT, status);
}

/**
 * @brief Test success path
 *
 */
void test_trust_center_remove_device_should_use_device_table_leave_api()
{
  // ARRANGE
  EmberStatus status;
  EmberEUI64 eui64 = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF};

  emberAfDeviceTableGetFirstIndexFromEui64_ExpectAndReturn(eui64, 1);
  emberAfPluginDeviceTableSendLeave_ExpectAndReturn(1, EMBER_SUCCESS);

  // ACT
  status = z3gatewayTrustCenterRemoveDevice(eui64);

  // ASSERT
  TEST_ASSERT_EQUAL(status, EMBER_SUCCESS);
}

void test_trust_center_remove_device_should_return_on_device_not_found()
{
  // ARRANGE
  EmberStatus status;
  EmberEUI64 eui64 = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF};

  emberAfDeviceTableGetFirstIndexFromEui64_ExpectAndReturn(
    eui64,
    EMBER_AF_PLUGIN_DEVICE_TABLE_NULL_INDEX);

  // ACT
  status = z3gatewayTrustCenterRemoveDevice(eui64);

  // ASSERT
  TEST_ASSERT_EQUAL(status, EMBER_NOT_FOUND);
}

void test_device_table_left_callback_should_invoke_device_removed_callback(void)
{
  // ARRANGE
  EmberEUI64 eui64 = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF};
  callback_onTrustCenterDeviceRemoved_Expect(eui64);

  // ACT
  emberAfPluginDeviceTableDeviceLeftCallback(eui64);

  // ASSERT (Handled by CMock)
}
