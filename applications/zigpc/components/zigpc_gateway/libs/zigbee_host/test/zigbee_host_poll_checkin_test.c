/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

// SLC project includes
#include "zap-command.h"

#include "zigbee_host.h"

/**
 * @brief  Setup the test suite
 * (called once before all test_xxx functions are called)
 *
 */
void suiteSetUp(void) {}

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

void test_poll_control_send_checkin_response_sanity(void)
{
  // ARRANGE

  sl_zigbee_af_fill_external_buffer_ExpectAndReturn(
    (ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
    ZCL_POLL_CONTROL_CLUSTER_ID,
    ZCL_CHECK_IN_RESPONSE_COMMAND_ID,
    "uv",
    4);

  sl_zigbee_af_send_response_ExpectAndReturn(SL_STATUS_OK );

  // ACT
  sl_status_t status = zigbeeHostSendPollingCheckInResponse(true);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK , status);
}

void test_poll_control_send_checkin_response_failed(void)
{
  // ARRANGE

  sl_zigbee_af_fill_external_buffer_ExpectAndReturn(
    (ZCL_CLUSTER_SPECIFIC_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
    ZCL_POLL_CONTROL_CLUSTER_ID,
    ZCL_CHECK_IN_RESPONSE_COMMAND_ID,
    "uv",
    4);

  sl_zigbee_af_send_response_ExpectAndReturn(SL_STATUS_FAIL);

  // ACT
  sl_status_t status = zigbeeHostSendPollingCheckInResponse(true);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_FAIL, status);
}
