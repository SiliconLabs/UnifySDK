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
#include "zigbee-security-manager_mock.h"

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
  sl_802154_long_addr_t test_eui64                        = {0};
  uint8_t test_install_code[INSTALL_CODE_SIZE] = {0};

  TEST_ASSERT_EQUAL(zigbeeHostTrustCenterAddDeviceInstallCode(NULL, NULL, 0),
                    SL_STATUS_INVALID_PARAMETER);
  TEST_ASSERT_EQUAL(
    zigbeeHostTrustCenterAddDeviceInstallCode(test_eui64, NULL, 0),
    SL_STATUS_INVALID_PARAMETER);
  TEST_ASSERT_EQUAL(
    zigbeeHostTrustCenterAddDeviceInstallCode(NULL, test_install_code, 0),
    SL_STATUS_INVALID_PARAMETER);
  TEST_ASSERT_EQUAL(
    zigbeeHostTrustCenterAddDeviceInstallCode(test_eui64, test_install_code, 0),
    SL_STATUS_INVALID_PARAMETER);
}

/**
 * @brief Test success path
 * 
 */
void test_trust_center_add_device_install_code_should_parse_link_key_and_call_join_network()
{
  // ARRANGE
  sl_status_t status;
  sl_802154_long_addr_t test_eui64 = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
  uint8_t test_install_code[INSTALL_CODE_SIZE]
    = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
  sl_zigbee_key_data_t test_link_key = {{0x55, 0x66, 0x77, 0x88, 0x99}};

  sli_zigbee_af_install_code_to_key_ExpectAndReturn(test_install_code,
                                       10,
                                       NULL,
                                       SL_STATUS_OK );
  sli_zigbee_af_install_code_to_key_ReturnThruPtr_key(&test_link_key);
  sli_zigbee_af_install_code_to_key_IgnoreArg_key();

  zb_sec_man_import_transient_key_ExpectAndReturn(test_eui64,
                                          (sl_zigbee_sec_man_key_t *)&test_link_key,
                                          ZB_SEC_MAN_FLAG_NONE,
                                          SL_STATUS_OK );

  // ACT
  status = zigbeeHostTrustCenterAddDeviceInstallCode(test_eui64,
                                                     test_install_code,
                                                     10);
  // ASSERT
  TEST_ASSERT_EQUAL(status, SL_STATUS_OK);
}
void test_trust_center_add_device_install_code_should_return_on_join_network_error()
{
  sl_status_t status;
  sl_802154_long_addr_t test_eui64                        = {0x77, 0x88};
  uint8_t test_install_code[INSTALL_CODE_SIZE] = {0x55, 0x66};
  sl_zigbee_key_data_t test_link_key                   = {{0xFF, 0xEE, 0xDD}};
  sl_status_t test_status                      = 0xCC;

  sli_zigbee_af_install_code_to_key_ExpectAndReturn(test_install_code,
                                       10,
                                       NULL,
                                       SL_STATUS_OK );
  sli_zigbee_af_install_code_to_key_ReturnThruPtr_key(&test_link_key);
  sli_zigbee_af_install_code_to_key_IgnoreArg_key();

  zb_sec_man_import_transient_key_ExpectAndReturn(test_eui64,
                                          (sl_zigbee_sec_man_key_t *)&test_link_key,
                                          ZB_SEC_MAN_FLAG_NONE,
                                          test_status);

  status = zigbeeHostTrustCenterAddDeviceInstallCode(test_eui64,
                                                     test_install_code,
                                                     10);
  TEST_ASSERT_EQUAL(status, test_status);
}

void test_trust_center_add_device_install_code_should_return_on_key_parse_error()
{
  sl_status_t status;
  sl_802154_long_addr_t test_eui64                        = {0x12, 0x34};
  uint8_t test_install_code[INSTALL_CODE_SIZE] = {0xAA, 0xBB};
  sl_status_t test_status                      = 0xFA;
  sl_status_t test_ret_status                  = SL_STATUS_FAIL;

  sli_zigbee_af_install_code_to_key_ExpectAndReturn(test_install_code,
                                       10,
                                       NULL,
                                       test_status);
  sli_zigbee_af_install_code_to_key_IgnoreArg_key();

  status = zigbeeHostTrustCenterAddDeviceInstallCode(test_eui64,
                                                     test_install_code,
                                                     10);
  TEST_ASSERT_EQUAL(status, test_ret_status);
}

void test_trust_center_join_callback_should_invoke_join_start_callback(void)
{
  // ARRANGE
  sl_802154_long_addr_t test_eui64    = {0x1, 0x2, 0x3, 0x4};
  sl_802154_short_addr_t test_node_id = 0x3523;
  callback_onNetworkDeviceJoin_Expect(test_eui64);

  sl_zigbee_af_add_address_table_entry_ExpectAndReturn(test_eui64,
                                              test_node_id,
                                              SL_STATUS_OK );

  // ACT
  emAfPluginGatewayInterfaceTrustCenterJoinHandler(test_eui64,
                                                   test_node_id,
                                                   SL_ZIGBEE_USE_PRECONFIGURED_KEY);

  // ASSERT (Handled by CMock)
}

/* Function prototypes tested */
void sl_zigbee_af_zigbee_key_establishment_cb(sl_802154_long_addr_t partner,
                                           sl_zigbee_key_status_t status);

void test_zigbee_key_establishment_callback_should_invoke_join_complete_callback(
  void)
{
  // ARRANGE
  sl_802154_long_addr_t test_eui64 = {0x1, 0x2, 0x3, 0x4};
  callback_onTrustCenterDeviceJoinComplete_Expect(test_eui64);

  // ACT
  sl_zigbee_af_zigbee_key_establishment_cb(test_eui64,
                                        SL_ZIGBEE_TC_REQUESTER_VERIFY_KEY_SUCCESS);

  // ASSERT (Handled by CMock)
}
