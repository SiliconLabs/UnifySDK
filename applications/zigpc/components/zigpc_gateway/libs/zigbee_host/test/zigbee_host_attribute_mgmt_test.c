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
#include "address-table_mock.h"
#include "ota-storage_mock.h"
#include "zigbee-device-common_mock.h"

// SLC project includes
#include "zap-command.h"
#include "zigbee_zcl_callback_dispatcher.h"

#include "zigbee_host.h"
#include "zigbee_host_common.h"

#include "test_zigbee_host_callbacks_mock.h"

extern struct zigbeeHostState z3gwState;

/**
 * @brief  Setup the test suite
 * (called once before all test_xxx functions are called)
 *
 */
void suiteSetUp(void)
{
  static struct zigbeeHostCallbacks callbacks = {
    .onReportedAttributeChange    = callback_onReportedAttributeChange,
    .onReadAttributesResponse     = callback_onReadAttributesResponse,
    .onConfigureReportingResponse = callback_onConfigureReportingResponse,
  };
  z3gwState.callbacks = &callbacks;
}

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

void test_send_bind_request_sanity(void)
{
  sl_802154_long_addr_t gatewayEui64 = "\x00\xF2\x23\x33\x71\x0D\xDD\x01";
  sl_802154_long_addr_t eui64        = "\x23\x4F\x92\x7C\xDE\x12\x00\xFF";
  sl_802154_short_addr_t nodeId      = 0x2131;
  uint8_t gatewayEndpoint = 1;
  uint8_t endpoint        = 2;
  uint16_t clusterId      = 0x0005;
  uint16_t group_id       = 0;

  // ARRANGE

  sl_zigbee_af_lookup_address_table_entry_by_eui64_ExpectAndReturn(eui64, 0);
  sl_zigbee_af_address_table_lookup_node_id_by_index_ExpectAndReturn(0, nodeId);
  sl_zigbee_get_last_zig_dev_request_sequence_IgnoreAndReturn(1);
  
  sl_zigbee_bind_request_ExpectAndReturn(nodeId,
                                   eui64,
                                   endpoint,
                                   clusterId,
                                   UNICAST_BINDING,
                                   gatewayEui64,
                                   group_id,
                                   gatewayEndpoint,
                                   0,
                                   SL_STATUS_OK );
  sl_zigbee_bind_request_IgnoreArg_options();

  // ACT
  sl_status_t status
    = zigbeeHostInitBinding(eui64, endpoint, clusterId, group_id, gatewayEui64, 1, true);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK , status);
}

void test_send_configure_reporting_request_sanity(void)
{
  sl_802154_long_addr_t eui64        = "\x52\x4F\x92\x32\xDE\x12\x00\xFF";
  uint8_t endpoint        = 2;
  uint8_t gatewayEndpoint = 4;
  uint16_t clusterId      = 0x0005;

  size_t reportSize = 2;
  uint8_t reportBuffer[reportSize];

  // ARRANGE
  sl_zigbee_af_primary_endpoint_for_current_network_index_ExpectAndReturn(gatewayEndpoint);
  sl_zigbee_af_fill_external_buffer_ExpectAndReturn(
    (ZCL_GLOBAL_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER),
    clusterId,
    ZCL_CONFIGURE_REPORTING_COMMAND_ID,
    "b",
    6);

  sl_zigbee_af_set_command_endpoints_Expect(gatewayEndpoint, endpoint);

  sl_zigbee_af_send_command_unicast_to_eui64_ExpectAndReturn(eui64, SL_STATUS_OK );

  // ACT
  sl_status_t status = zigbeeHostInitReporting(eui64,
                                               endpoint,
                                               clusterId,
                                               reportBuffer,
                                               reportSize);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK , status);
}

void test_attribute_report_callback_sanity(void)
{
  sl_802154_long_addr_t eui64       = "\x03\x1F\x92\x32\xDE\x12\x00\xFF";
  sl_zigbee_aps_frame_t apsFrame = {
    .sourceEndpoint = 2,
    .clusterId      = 0x2334,
  };
  sl_zigbee_af_cluster_command_t cmd = {
    .apsFrame = &apsFrame,
  };
  size_t reportSize = 2;
  uint8_t reportBuffer[reportSize];

  // ARRANGE
  sl_zigbee_af_get_current_sender_eui64_ExpectAndReturn(NULL, SL_STATUS_OK );
  sl_zigbee_af_get_current_sender_eui64_IgnoreArg_address();
  sl_zigbee_af_get_current_sender_eui64_ReturnThruPtr_address(eui64);

  sl_zigbee_af_current_command_ExpectAndReturn(&cmd);

  callback_onReportedAttributeChange_Expect(eui64,
                                            apsFrame.sourceEndpoint,
                                            apsFrame.clusterId,
                                            reportBuffer,
                                            reportSize);

  // ACT
  emberAfReportAttributesCallback(apsFrame.clusterId, reportBuffer, reportSize);

  // ASSERT (Handled by CMock)
}

void test_attribute_read_response_callback_sanity(void)
{
  sl_802154_long_addr_t eui64       = "\x03\x1F\x92\x32\xDE\x12\x00\xFF";
  sl_zigbee_aps_frame_t apsFrame = {
    .sourceEndpoint = 2,
    .clusterId      = 0x2334,
  };
  sl_zigbee_af_cluster_command_t cmd = {
    .apsFrame  = &apsFrame,
    .commandId = ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
  };
  size_t readSize = 2;
  uint8_t readBuffer[readSize];

  // ARRANGE
  sl_zigbee_af_get_current_sender_eui64_ExpectAndReturn(NULL, SL_STATUS_OK );
  sl_zigbee_af_get_current_sender_eui64_IgnoreArg_address();
  sl_zigbee_af_get_current_sender_eui64_ReturnThruPtr_address(eui64);

  sl_zigbee_af_current_command_ExpectAndReturn(&cmd);

  callback_onReadAttributesResponse_Expect(eui64,
                                           apsFrame.sourceEndpoint,
                                           apsFrame.clusterId,
                                           readBuffer,
                                           readSize);

  // ACT
  emberAfReadAttributesResponseCallback(apsFrame.clusterId,
                                        readBuffer,
                                        readSize);

  // ASSERT (Handled by CMock)
}

bool emberAfConfigureReportingResponseCallback(sl_zigbee_af_cluster_id_t clusterId,
                                               uint8_t *buffer,
                                               uint16_t bufLen);
void test_attribute_configure_report_response_callback_sanity(void)
{
  sl_802154_long_addr_t eui64       = "\x03\x1F\x92\x32\xDE\x12\x00\xFF";
  sl_zigbee_aps_frame_t apsFrame = {
    .sourceEndpoint = 2,
    .clusterId      = 0x2334,
  };
  sl_zigbee_af_cluster_command_t cmd = {
    .apsFrame  = &apsFrame,
    .commandId = ZCL_CONFIGURE_REPORTING_RESPONSE_COMMAND_ID,
  };
  size_t cfgResSize = 2;
  uint8_t cfgResBuffer[cfgResSize];

  // ARRANGE
  sl_zigbee_af_get_current_sender_eui64_ExpectAndReturn(NULL, SL_STATUS_OK );
  sl_zigbee_af_get_current_sender_eui64_IgnoreArg_address();
  sl_zigbee_af_get_current_sender_eui64_ReturnThruPtr_address(eui64);

  sl_zigbee_af_current_command_ExpectAndReturn(&cmd);

  callback_onConfigureReportingResponse_Expect(eui64,
                                               apsFrame.sourceEndpoint,
                                               apsFrame.clusterId,
                                               cfgResBuffer,
                                               cfgResSize);

  // ACT
  emberAfConfigureReportingResponseCallback(apsFrame.clusterId,
                                            cfgResBuffer,
                                            cfgResSize);

  // ASSERT (Handled by CMock)
}
