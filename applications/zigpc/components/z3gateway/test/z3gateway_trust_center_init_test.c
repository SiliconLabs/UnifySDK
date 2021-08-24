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
#include "af_mock.h"
#include "network-creator_mock.h"

#include "z3gateway.h"
#include "z3gateway_common.h"
#include "test_z3gateway_callbacks_mock.h"

struct z3gatewayState z3gwState;

/* Prototypes used for callback tests */
bool emberAfStackStatusCallback(EmberStatus status);

/**
 * @brief Setup the test suite (called once before all test_xxx functions are called)
 *
 */
void suiteSetUp()
{
  static struct z3gatewayCallbacks callbacks
    = {.onTrustCenterInitialized = callback_onTrustCenterInitialized};

  z3gwState.callbacks = &callbacks;
}

/**
 * @brief Teardown the test suite (called once after all test_xxx functions are called)
 *
 * @param num_failures
 * @return int
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/**
 * @brief Set up before each test case
 *
 */
void setUp() {}

/**
 * @brief Teardown after each test case
 *
 */
void tearDown() {}

/**
 * @brief Test scenario when the network is down. In this case, the network
 * creator plugin should be called
 *
 */
void test_trust_center_init_should_create_network_on_network_absence()
{
  EmberStatus status;

  emberAfNetworkState_ExpectAndReturn(EMBER_NO_NETWORK);
  emberAfPluginNetworkCreatorStart_ExpectAndReturn(true, EMBER_SUCCESS);
  status = z3gatewayTrustCenterInit();

  TEST_ASSERT_EQUAL(status, EMBER_SUCCESS);
}

/**
 * @brief Test scenario when the network is already up. In this case, the onTrustCenterInitialied
 * callback should be called
 *
 */
void test_trust_center_init_should_return_network_info_on_network_joined()
{
  EmberStatus status;
  EmberEUI64 test_eui64 = {0};

  emberAfNetworkState_ExpectAndReturn(EMBER_JOINED_NETWORK);
  emberAfGetNetworkParameters_IgnoreAndReturn(EMBER_SUCCESS);

  emberAfGetEui64_Expect(NULL);
  emberAfGetEui64_IgnoreArg_returnEui64();
  emberAfGetEui64_ReturnThruPtr_returnEui64(test_eui64);

  callback_onTrustCenterInitialized_Expect(test_eui64, NULL);
  callback_onTrustCenterInitialized_IgnoreArg_network();

  status = z3gatewayTrustCenterInit();
  TEST_ASSERT_EQUAL(status, EMBER_SUCCESS);
}

/**
 * @brief Test scenario when getting network information from the Network creator
 * complete callback.
 *
 */
void test_trust_center_init_callback_should_be_called_by_network_creator_complete()
{
  // ARRANGE
  EmberEUI64 test_eui64 = {0};

  emberAfGetEui64_Expect(NULL);
  emberAfGetEui64_IgnoreArg_returnEui64();
  emberAfGetEui64_ReturnThruPtr_returnEui64(test_eui64);
  emberAfGetNetworkParameters_IgnoreAndReturn(EMBER_SUCCESS);

  callback_onTrustCenterInitialized_Expect(test_eui64, NULL);
  callback_onTrustCenterInitialized_IgnoreArg_network();

  // ACT
  emberAfStackStatusCallback(EMBER_NETWORK_UP);

  // ASSERT (Handled by CMock)
}
