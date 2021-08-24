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
#include "device-table_mock.h"
#include "ezsp_mock.h"

#include "z3gateway.h"
#include "z3gateway_common.h"
#include "test_z3gateway_callbacks_mock.h"

#define INSTALL_CODE_SIZE (16 + 2)

/* Function prototypes tested */
void z3gatewayZigbeeKeyEstablishmentCallback(EmberEUI64 partner,
                                             EmberKeyStatus status);
void emberAfPluginDeviceTableTrustCenterJoinCallback(const EmberEUI64 eui64);

struct z3gatewayState z3gwState;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  static struct z3gatewayCallbacks callbacks = {
    .onTrustCenterDeviceJoinStart    = callback_onTrustCenterDeviceJoinStart,
    .onTrustCenterDeviceJoinComplete = callback_onTrustCenterDeviceJoinComplete,
  };
  z3gwState.callbacks = &callbacks;
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp() {}

void tearDown() {}

/**
 * @brief Test scenario of handling invalit input
 * 
 */
void test_trust_center_add_device_install_code_should_handle_invalid_args()
{
  EmberEUI64 test_eui64                        = {0};
  uint8_t test_install_code[INSTALL_CODE_SIZE] = {0};

  TEST_ASSERT_EQUAL(z3gatewayTrustCenterAddDeviceInstallCode(NULL, NULL, 0),
                    EMBER_BAD_ARGUMENT);
  TEST_ASSERT_EQUAL(
    z3gatewayTrustCenterAddDeviceInstallCode(test_eui64, NULL, 0),
    EMBER_BAD_ARGUMENT);
  TEST_ASSERT_EQUAL(
    z3gatewayTrustCenterAddDeviceInstallCode(NULL, test_install_code, 0),
    EMBER_BAD_ARGUMENT);
  TEST_ASSERT_EQUAL(
    z3gatewayTrustCenterAddDeviceInstallCode(test_eui64, test_install_code, 0),
    EMBER_BAD_ARGUMENT);
}

/**
 * @brief Test success path
 * 
 */
void test_trust_center_add_device_install_code_should_parse_link_key_and_call_join_network()
{
  // ARRANGE
  EmberStatus status;
  EmberEUI64 test_eui64 = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
  uint8_t test_install_code[INSTALL_CODE_SIZE]
    = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
  EmberKeyData test_link_key = {{0x55, 0x66, 0x77, 0x88, 0x99}};

  emberAfNetworkState_ExpectAndReturn(EMBER_JOINED_NETWORK);
  emAfInstallCodeToKey_ExpectAndReturn(test_install_code,
                                       10,
                                       NULL,
                                       EMBER_SUCCESS);
  emAfInstallCodeToKey_ReturnThruPtr_key(&test_link_key);
  emAfInstallCodeToKey_IgnoreArg_key();

  ezspAddTransientLinkKey_ExpectAndReturn(test_eui64,
                                          &test_link_key,
                                          EMBER_SUCCESS);

  emberGetKey_ExpectAndReturn(EMBER_CURRENT_NETWORK_KEY, NULL, EMBER_SUCCESS);
  emberGetKey_IgnoreArg_keyStruct();

  // ACT
  status = z3gatewayTrustCenterAddDeviceInstallCode(test_eui64,
                                                    test_install_code,
                                                    10);
  // ASSERT
  TEST_ASSERT_EQUAL(status, EMBER_SUCCESS);
}
void test_trust_center_add_device_install_code_should_return_on_join_network_error()
{
  EmberStatus status;
  EmberEUI64 test_eui64                        = {0x77, 0x88};
  uint8_t test_install_code[INSTALL_CODE_SIZE] = {0x55, 0x66};
  EmberKeyData test_link_key                   = {{0xFF, 0xEE, 0xDD}};
  EmberStatus test_status                      = 0xCC;

  emberAfNetworkState_ExpectAndReturn(EMBER_JOINED_NETWORK);
  emAfInstallCodeToKey_ExpectAndReturn(test_install_code,
                                       10,
                                       NULL,
                                       EMBER_SUCCESS);
  emAfInstallCodeToKey_ReturnThruPtr_key(&test_link_key);
  emAfInstallCodeToKey_IgnoreArg_key();

  ezspAddTransientLinkKey_ExpectAndReturn(test_eui64,
                                          &test_link_key,
                                          test_status);

  status = z3gatewayTrustCenterAddDeviceInstallCode(test_eui64,
                                                    test_install_code,
                                                    10);
  TEST_ASSERT_EQUAL(status, test_status);
}

void test_trust_center_add_device_install_code_should_return_on_key_parse_error()
{
  EmberStatus status;
  EmberEUI64 test_eui64                        = {0x12, 0x34};
  uint8_t test_install_code[INSTALL_CODE_SIZE] = {0xAA, 0xBB};
  EmberStatus test_status                      = 0xFA;

  emberAfNetworkState_ExpectAndReturn(EMBER_JOINED_NETWORK);
  emAfInstallCodeToKey_ExpectAndReturn(test_install_code,
                                       10,
                                       NULL,
                                       test_status);
  emAfInstallCodeToKey_IgnoreArg_key();

  status = z3gatewayTrustCenterAddDeviceInstallCode(test_eui64,
                                                    test_install_code,
                                                    10);
  TEST_ASSERT_EQUAL(status, test_status);
}

void test_device_table_trust_center_join_callback_should_invoke_join_start_callback(
  void)
{
  // ARRANGE
  EmberEUI64 test_eui64 = {0x1, 0x2, 0x3, 0x4};
  callback_onTrustCenterDeviceJoinStart_Expect(test_eui64);

  // ACT
  emberAfPluginDeviceTableTrustCenterJoinCallback(test_eui64);

  // ASSERT (Handled by CMock)
}

void test_zigbee_key_establishment_callback_should_invoke_join_complete_callback(
  void)
{
  // ARRANGE
  EmberEUI64 test_eui64 = {0x1, 0x2, 0x3, 0x4};
  callback_onTrustCenterDeviceJoinComplete_Expect(test_eui64);

  // ACT
  z3gatewayZigbeeKeyEstablishmentCallback(
    test_eui64,
    EMBER_TC_REQUESTER_VERIFY_KEY_SUCCESS);

  // ASSERT (Handled by CMock)
}
