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
#include "af-main_mock.h"
#include "af-security_mock.h"
#include "ezsp_mock.h"

#include "zigbee_host.h"
#include "zigbee_host_common.h"
#include "test_zigbee_host_callbacks_mock.h"

#define INSTALL_CODE_SIZE (16 + 2)

struct zigbeeHostState z3gwState;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  static struct zigbeeHostCallbacks callbacks = {
    .onNetworkDeviceJoin             = callback_onNetworkDeviceJoin,
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

  TEST_ASSERT_EQUAL(zigbeeHostTrustCenterAddDeviceInstallCode(NULL, NULL, 0),
                    EMBER_BAD_ARGUMENT);
  TEST_ASSERT_EQUAL(
    zigbeeHostTrustCenterAddDeviceInstallCode(test_eui64, NULL, 0),
    EMBER_BAD_ARGUMENT);
  TEST_ASSERT_EQUAL(
    zigbeeHostTrustCenterAddDeviceInstallCode(NULL, test_install_code, 0),
    EMBER_BAD_ARGUMENT);
  TEST_ASSERT_EQUAL(
    zigbeeHostTrustCenterAddDeviceInstallCode(test_eui64, test_install_code, 0),
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

  sli_zigbee_af_install_code_to_key_ExpectAndReturn(test_install_code,
                                       10,
                                       NULL,
                                       EMBER_SUCCESS);
  sli_zigbee_af_install_code_to_key_ReturnThruPtr_key(&test_link_key);
  sli_zigbee_af_install_code_to_key_IgnoreArg_key();

  ezspAddTransientLinkKey_ExpectAndReturn(test_eui64,
                                          &test_link_key,
                                          EMBER_SUCCESS);

  // ACT
  status = zigbeeHostTrustCenterAddDeviceInstallCode(test_eui64,
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

  sli_zigbee_af_install_code_to_key_ExpectAndReturn(test_install_code,
                                       10,
                                       NULL,
                                       EMBER_SUCCESS);
  sli_zigbee_af_install_code_to_key_ReturnThruPtr_key(&test_link_key);
  sli_zigbee_af_install_code_to_key_IgnoreArg_key();

  ezspAddTransientLinkKey_ExpectAndReturn(test_eui64,
                                          &test_link_key,
                                          test_status);

  status = zigbeeHostTrustCenterAddDeviceInstallCode(test_eui64,
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

  sli_zigbee_af_install_code_to_key_ExpectAndReturn(test_install_code,
                                       10,
                                       NULL,
                                       test_status);
  sli_zigbee_af_install_code_to_key_IgnoreArg_key();

  status = zigbeeHostTrustCenterAddDeviceInstallCode(test_eui64,
                                                     test_install_code,
                                                     10);
  TEST_ASSERT_EQUAL(status, test_status);
}

void test_trust_center_join_callback_should_invoke_join_start_callback(void)
{
  // ARRANGE
  EmberEUI64 test_eui64    = {0x1, 0x2, 0x3, 0x4};
  EmberNodeId test_node_id = 0x3523;
  callback_onNetworkDeviceJoin_Expect(test_eui64);

  emberAfAddAddressTableEntry_ExpectAndReturn(test_eui64,
                                              test_node_id,
                                              EMBER_SUCCESS);

  // ACT
  emAfPluginGatewayInterfaceTrustCenterJoinHandler(test_eui64,
                                                   test_node_id,
                                                   EMBER_USE_PRECONFIGURED_KEY);

  // ASSERT (Handled by CMock)
}

/* Function prototypes tested */
void emberAfZigbeeKeyEstablishmentCallback(EmberEUI64 partner,
                                           EmberKeyStatus status);

void test_zigbee_key_establishment_callback_should_invoke_join_complete_callback(
  void)
{
  // ARRANGE
  EmberEUI64 test_eui64 = {0x1, 0x2, 0x3, 0x4};
  callback_onTrustCenterDeviceJoinComplete_Expect(test_eui64);

  // ACT
  emberAfZigbeeKeyEstablishmentCallback(test_eui64,
                                        EMBER_TC_REQUESTER_VERIFY_KEY_SUCCESS);

  // ASSERT (Handled by CMock)
}
