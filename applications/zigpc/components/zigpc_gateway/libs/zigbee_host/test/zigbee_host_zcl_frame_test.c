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

extern uint16_t zclMessageBufferLen;
extern uint8_t zclMessageBuffer[ZCL_BUFFER_SIZE];

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
  sl_status_t status;

  // ACT
  status = zigbeeHostFillZclFrame(NULL, 0, 0);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_PARAMETER, status);
}

void test_fill_zcl_frame_should_handle_insufficient_size()
{
  // ARRANGE
  sl_status_t status;
  uint8_t buffer = 0;

  // ACT
  status = zigbeeHostFillZclFrame(&buffer, SL_ZIGBEE_AF_ZCL_OVERHEAD - 1, 0);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_INDEX , status);
}

void test_fill_zcl_frame_should_handle_buffer_overflow()
{
  // ARRANGE
  sl_status_t status;
  uint8_t buffer = 0;

  // ACT
  status = zigbeeHostFillZclFrame(&buffer, ZCL_BUFFER_SIZE + 1, 0);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_INDEX , status);
}

void test_fill_zcl_frame_should_copy_buffer()
{
  // ARRANGE
  sl_status_t status;
  size_t bufferSequenceIdIndex     = 10;
  const uint8_t SEQUENCE_ID_RETURN = 0xDF;
  uint8_t buffer[ZCL_BUFFER_SIZE];

  for (size_t i = 0; i < ZCL_BUFFER_SIZE; i++) {
    buffer[i] = i;
  }

  sl_zigbee_af_next_sequence_ExpectAndReturn(SEQUENCE_ID_RETURN);

  // ACT
  status
    = zigbeeHostFillZclFrame(buffer, ZCL_BUFFER_SIZE, bufferSequenceIdIndex);

  // ASSERT
  buffer[bufferSequenceIdIndex] = SEQUENCE_ID_RETURN;
  TEST_ASSERT_EQUAL(SL_STATUS_OK , status);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(buffer, zclMessageBuffer, ZCL_BUFFER_SIZE);
}

/************************************************************
 * zigbeeHostSendZclFrameUnicast tests
 ***********************************************************/

void test_send_zcl_frame_unicast_should_handle_invalid_argument()
{
  // ARRANGE
  sl_status_t status_null_eui64;
  sl_status_t status_null_cluster;
  sl_802154_long_addr_t eui64;

  // ACT
  status_null_eui64 = zigbeeHostSendZclFrameUnicast(NULL, 1, 0);
  // 0xFFFF is the NULL Cluster ID
  status_null_cluster = zigbeeHostSendZclFrameUnicast(eui64, 1, 0xFFFF);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_PARAMETER, status_null_eui64);
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_PARAMETER, status_null_cluster);
}

void test_send_zcl_frame_unicast_should_handle_unknown_device()
{
  // ARRANGE
  sl_status_t status;
  sl_802154_long_addr_t eui64        = "\x12\x03\xA7\xBB\x3D\x02\x00\x42";
  uint8_t gatewayEndpoint = 1;

  sl_zigbee_af_primary_endpoint_for_current_network_index_ExpectAndReturn(gatewayEndpoint);

  sl_zigbee_af_send_unicast_to_eui64_ExpectAndReturn(eui64,
                                            NULL,
                                            zclMessageBufferLen,
                                            zclMessageBuffer,
                                            SL_STATUS_INVALID_STATE );
  sl_zigbee_af_send_unicast_to_eui64_IgnoreArg_apsFrame();

  // ACT
  status = zigbeeHostSendZclFrameUnicast(eui64, 1, 0x0001);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND , status);
}

void test_send_zcl_frame_unicast_should_handle_happy_path()
{
  // ARRANGE
  sl_status_t status;
  sl_802154_long_addr_t eui64        = "\x02\x03\xA7\xBB\x8D\x02\x00\x42";
  uint8_t gatewayEndpoint = 4;

  sl_zigbee_af_primary_endpoint_for_current_network_index_ExpectAndReturn(gatewayEndpoint);

  sl_zigbee_af_send_unicast_to_eui64_ExpectAndReturn(eui64,
                                            NULL,
                                            zclMessageBufferLen,
                                            zclMessageBuffer,
                                            SL_STATUS_OK );
  sl_zigbee_af_send_unicast_to_eui64_IgnoreArg_apsFrame();

  // ACT
  status = zigbeeHostSendZclFrameUnicast(eui64, 1, 0x0001);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK , status);
}

/************************************************************
 * zigbeeHostSendZclFrameMulticast tests
 ***********************************************************/

void test_send_zcl_frame_multicast_should_handle_invalid_argument()
{
  // ARRANGE
  sl_status_t status;

  // ACT
  // 0xFFFF is the NULL Cluster ID
  status = zigbeeHostSendZclFrameMulticast(0x1, 0xFFFF);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_PARAMETER, status);
}

void test_send_zcl_frame_multicast_should_handle_happy_path()
{
  // ARRANGE
  sl_status_t status;
  sl_zigbee_multicast_id_t multicastId = 0x1234;
  sl_802154_short_addr_t alias = SL_ZIGBEE_NULL_NODE_ID;
  uint8_t nwkSeq = 0;
  uint8_t gatewayEndpoint      = 2;

  sl_zigbee_af_primary_endpoint_for_current_network_index_ExpectAndReturn(gatewayEndpoint);

  sl_zigbee_af_send_multicast_ExpectAndReturn(multicastId,
                                       alias,
                                       nwkSeq, 
                                       NULL,
                                       zclMessageBufferLen,
                                       zclMessageBuffer,
                                       SL_STATUS_OK );
  sl_zigbee_af_send_multicast_IgnoreArg_apsFrame();

  // ACT
  status = zigbeeHostSendZclFrameMulticast(multicastId, 0x0001);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK , status);
}
