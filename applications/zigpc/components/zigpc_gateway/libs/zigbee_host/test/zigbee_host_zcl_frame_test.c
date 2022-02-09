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

#include "af_mock.h"
#include "address-table_mock.h"

#include "zigbee_host.h"
#include "zigbee_host_common.h"

extern uint16_t appZclBufferLen;
extern uint8_t appZclBuffer[ZCL_BUFFER_SIZE];

/**
 * @brief Setup the test suite (called once before all test_xxx functions are
 * called)
 *
 */
void suiteSetUp(void) {}

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
void setUp(void) {}

/**
 * @brief Teardown after each test case
 *
 */
void tearDown(void) {}

/************************************************************
 * zigbeeHostFillZclFrame tests
 ***********************************************************/

void test_fill_zcl_frame_should_handle_invalid_argument()
{
  // ARRANGE
  EmberStatus status;

  // ACT
  status = zigbeeHostFillZclFrame(NULL, 0, 0);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_BAD_ARGUMENT, status);
}

void test_fill_zcl_frame_should_handle_insufficient_size()
{
  // ARRANGE
  EmberStatus status;
  uint8_t buffer;

  // ACT
  status = zigbeeHostFillZclFrame(&buffer, EMBER_AF_ZCL_OVERHEAD - 1, 0);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_INDEX_OUT_OF_RANGE, status);
}

void test_fill_zcl_frame_should_handle_buffer_overflow()
{
  // ARRANGE
  EmberStatus status;
  uint8_t buffer;

  // ACT
  status = zigbeeHostFillZclFrame(&buffer, ZCL_BUFFER_SIZE + 1, 0);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_INDEX_OUT_OF_RANGE, status);
}

void test_fill_zcl_frame_should_copy_buffer()
{
  // ARRANGE
  EmberStatus status;
  size_t bufferSequenceIdIndex     = 10;
  const uint8_t SEQUENCE_ID_RETURN = 0xDF;
  uint8_t buffer[ZCL_BUFFER_SIZE];

  for (size_t i = 0; i < ZCL_BUFFER_SIZE; i++) {
    buffer[i] = i;
  }

  emberAfNextSequence_ExpectAndReturn(SEQUENCE_ID_RETURN);

  // ACT
  status
    = zigbeeHostFillZclFrame(buffer, ZCL_BUFFER_SIZE, bufferSequenceIdIndex);

  // ASSERT
  buffer[bufferSequenceIdIndex] = SEQUENCE_ID_RETURN;
  TEST_ASSERT_EQUAL(EMBER_SUCCESS, status);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(buffer, appZclBuffer, ZCL_BUFFER_SIZE);
}

/************************************************************
 * zigbeeHostSendZclFrameUnicast tests
 ***********************************************************/

void test_send_zcl_frame_unicast_should_handle_invalid_argument()
{
  // ARRANGE
  EmberStatus status_null_eui64;
  EmberStatus status_null_cluster;
  EmberEUI64 eui64;

  // ACT
  status_null_eui64 = zigbeeHostSendZclFrameUnicast(NULL, 1, 0);
  // 0xFFFF is the NULL Cluster ID
  status_null_cluster = zigbeeHostSendZclFrameUnicast(eui64, 1, 0xFFFF);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_BAD_ARGUMENT, status_null_eui64);
  TEST_ASSERT_EQUAL(EMBER_BAD_ARGUMENT, status_null_cluster);
}

void test_send_zcl_frame_unicast_should_handle_unknown_device()
{
  // ARRANGE
  EmberStatus status;
  EmberEUI64 eui64        = "\x12\x03\xA7\xBB\x3D\x02\x00\x42";
  uint8_t gatewayEndpoint = 1;

  emberAfPrimaryEndpointForCurrentNetworkIndex_ExpectAndReturn(gatewayEndpoint);

  emberAfSendUnicastToEui64_ExpectAndReturn(eui64,
                                            NULL,
                                            appZclBufferLen,
                                            appZclBuffer,
                                            EMBER_INVALID_CALL);
  emberAfSendUnicastToEui64_IgnoreArg_apsFrame();

  // ACT
  status = zigbeeHostSendZclFrameUnicast(eui64, 1, 0x0001);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_NOT_FOUND, status);
}

void test_send_zcl_frame_unicast_should_handle_happy_path()
{
  // ARRANGE
  EmberStatus status;
  EmberEUI64 eui64        = "\x02\x03\xA7\xBB\x8D\x02\x00\x42";
  uint8_t gatewayEndpoint = 4;

  emberAfPrimaryEndpointForCurrentNetworkIndex_ExpectAndReturn(gatewayEndpoint);

  emberAfSendUnicastToEui64_ExpectAndReturn(eui64,
                                            NULL,
                                            appZclBufferLen,
                                            appZclBuffer,
                                            EMBER_SUCCESS);
  emberAfSendUnicastToEui64_IgnoreArg_apsFrame();

  // ACT
  status = zigbeeHostSendZclFrameUnicast(eui64, 1, 0x0001);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_SUCCESS, status);
}

/************************************************************
 * zigbeeHostSendZclFrameMulticast tests
 ***********************************************************/

void test_send_zcl_frame_multicast_should_handle_invalid_argument()
{
  // ARRANGE
  EmberStatus status;

  // ACT
  // 0xFFFF is the NULL Cluster ID
  status = zigbeeHostSendZclFrameMulticast(0x1, 0xFFFF);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_BAD_ARGUMENT, status);
}

void test_send_zcl_frame_multicast_should_handle_happy_path()
{
  // ARRANGE
  EmberStatus status;
  EmberMulticastId multicastId = 0x1234;
  uint8_t gatewayEndpoint      = 2;

  emberAfPrimaryEndpointForCurrentNetworkIndex_ExpectAndReturn(gatewayEndpoint);

  emberAfSendMulticast_ExpectAndReturn(multicastId,
                                       NULL,
                                       appZclBufferLen,
                                       appZclBuffer,
                                       EMBER_SUCCESS);
  emberAfSendMulticast_IgnoreArg_apsFrame();

  // ACT
  status = zigbeeHostSendZclFrameMulticast(multicastId, 0x0001);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_SUCCESS, status);
}
