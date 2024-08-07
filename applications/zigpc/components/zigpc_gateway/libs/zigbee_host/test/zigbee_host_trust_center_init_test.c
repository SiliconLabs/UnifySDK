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
#include "af-main_mock.h"
#include "network-creator_mock.h"

#include "zigbee_host.h"
#include "zigbee_host_common.h"
#include "test_zigbee_host_callbacks_mock.h"

struct zigbeeHostState z3gwState;

/* Prototypes used for callback tests */
bool sl_zigbee_af_stack_status_cb(sl_status_t status);

/**
 * @brief Setup the test suite (called once before all test_xxx functions are called)
 *
 */
void suiteSetUp()
{
  static struct zigbeeHostCallbacks callbacks
    = {.onNetworkInitialized = callback_onNetworkInitialized};

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
  sl_status_t status;

  sl_zigbee_af_network_state_ExpectAndReturn(SL_ZIGBEE_NO_NETWORK);
  sl_zigbee_af_network_creator_start_ExpectAndReturn(true, SL_STATUS_OK );
  status = zigbeeHostTrustCenterInit();

  TEST_ASSERT_EQUAL(status, SL_STATUS_OK );
}

/**
 * @brief Test scenario when the network is already up. In this case, the onTrustCenterInitialied
 * callback should be called
 *
 */
void test_trust_center_init_should_return_network_info_on_network_joined()
{
  sl_status_t status;

  sl_zigbee_af_network_state_ExpectAndReturn(SL_ZIGBEE_JOINED_NETWORK);
  sl_zigbee_af_get_network_parameters_IgnoreAndReturn(SL_STATUS_OK );

  status = zigbeeHostTrustCenterInit();
  TEST_ASSERT_EQUAL(status, SL_STATUS_OK );
}

/**
 * @brief Test scenario when getting network information from the Network creator
 * complete callback.
 *
 */
void test_trust_center_init_callback_should_be_called_by_network_creator_complete()
{
  // ARRANGE
  sl_zigbee_network_parameters_t network = {.panId = 0x1234};

  sl_zigbee_af_get_network_parameters_ExpectAndReturn(NULL, NULL, SL_STATUS_OK );
  sl_zigbee_af_get_network_parameters_IgnoreArg_nodeType();
  sl_zigbee_af_get_network_parameters_IgnoreArg_parameters();
  sl_zigbee_af_get_network_parameters_ReturnThruPtr_parameters(&network);

  callback_onNetworkInitialized_ExpectWithArray(&network, 1);

  // ACT
  sl_zigbee_af_stack_status_cb(SL_STATUS_NETWORK_UP);

  // ASSERT (Handled by CMock)
}
