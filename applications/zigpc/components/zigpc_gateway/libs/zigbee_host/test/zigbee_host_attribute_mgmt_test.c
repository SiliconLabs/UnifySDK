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
  EmberEUI64 gatewayEui64 = "\x00\xF2\x23\x33\x71\x0D\xDD\x01";
  EmberEUI64 eui64        = "\x23\x4F\x92\x7C\xDE\x12\x00\xFF";
  EmberNodeId nodeId      = 0x2131;
  uint8_t gatewayEndpoint = 1;
  uint8_t endpoint        = 2;
  uint16_t clusterId      = 0x0005;
  uint16_t group_id       = 0;

  // ARRANGE

  emberAfLookupAddressTableEntryByEui64_ExpectAndReturn(eui64, 0);
  emberAfPluginAddressTableLookupNodeIdByIndex_ExpectAndReturn(0, nodeId);
  emberGetLastAppZigDevRequestSequence_IgnoreAndReturn(1);
  
  emberBindRequest_ExpectAndReturn(nodeId,
                                   eui64,
                                   endpoint,
                                   clusterId,
                                   UNICAST_BINDING,
                                   gatewayEui64,
                                   group_id,
                                   gatewayEndpoint,
                                   0,
                                   EMBER_SUCCESS);
  emberBindRequest_IgnoreArg_options();

  // ACT
  EmberStatus status
    = zigbeeHostInitBinding(eui64, endpoint, clusterId, group_id, gatewayEui64, 1, true);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(EMBER_SUCCESS, status);
}

void test_send_configure_reporting_request_sanity(void)
{
  EmberEUI64 eui64        = "\x52\x4F\x92\x32\xDE\x12\x00\xFF";
  uint8_t endpoint        = 2;
  uint8_t gatewayEndpoint = 4;
  uint16_t clusterId      = 0x0005;

  size_t reportSize = 2;
  uint8_t reportBuffer[reportSize];

  // ARRANGE
  emberAfPrimaryEndpointForCurrentNetworkIndex_ExpectAndReturn(gatewayEndpoint);
  emberAfFillExternalBuffer_ExpectAndReturn(
    (ZCL_GLOBAL_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER),
    clusterId,
    ZCL_CONFIGURE_REPORTING_COMMAND_ID,
    "b",
    6);

  emberAfSetCommandEndpoints_Expect(gatewayEndpoint, endpoint);

  emberAfSendCommandUnicastToEui64_ExpectAndReturn(eui64, EMBER_SUCCESS);

  // ACT
  EmberStatus status = zigbeeHostInitReporting(eui64,
                                               endpoint,
                                               clusterId,
                                               reportBuffer,
                                               reportSize);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(EMBER_SUCCESS, status);
}

void test_attribute_report_callback_sanity(void)
{
  EmberEUI64 eui64       = "\x03\x1F\x92\x32\xDE\x12\x00\xFF";
  EmberApsFrame apsFrame = {
    .sourceEndpoint = 2,
    .clusterId      = 0x2334,
  };
  EmberAfClusterCommand cmd = {
    .apsFrame = &apsFrame,
  };
  size_t reportSize = 2;
  uint8_t reportBuffer[reportSize];

  // ARRANGE
  emberAfGetCurrentSenderEui64_ExpectAndReturn(NULL, EMBER_SUCCESS);
  emberAfGetCurrentSenderEui64_IgnoreArg_address();
  emberAfGetCurrentSenderEui64_ReturnThruPtr_address(eui64);

  emberAfCurrentCommand_ExpectAndReturn(&cmd);

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
  EmberEUI64 eui64       = "\x03\x1F\x92\x32\xDE\x12\x00\xFF";
  EmberApsFrame apsFrame = {
    .sourceEndpoint = 2,
    .clusterId      = 0x2334,
  };
  EmberAfClusterCommand cmd = {
    .apsFrame  = &apsFrame,
    .commandId = ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
  };
  size_t readSize = 2;
  uint8_t readBuffer[readSize];

  // ARRANGE
  emberAfGetCurrentSenderEui64_ExpectAndReturn(NULL, EMBER_SUCCESS);
  emberAfGetCurrentSenderEui64_IgnoreArg_address();
  emberAfGetCurrentSenderEui64_ReturnThruPtr_address(eui64);

  emberAfCurrentCommand_ExpectAndReturn(&cmd);

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

bool emberAfConfigureReportingResponseCallback(EmberAfClusterId clusterId,
                                               uint8_t *buffer,
                                               uint16_t bufLen);
void test_attribute_configure_report_response_callback_sanity(void)
{
  EmberEUI64 eui64       = "\x03\x1F\x92\x32\xDE\x12\x00\xFF";
  EmberApsFrame apsFrame = {
    .sourceEndpoint = 2,
    .clusterId      = 0x2334,
  };
  EmberAfClusterCommand cmd = {
    .apsFrame  = &apsFrame,
    .commandId = ZCL_CONFIGURE_REPORTING_RESPONSE_COMMAND_ID,
  };
  size_t cfgResSize = 2;
  uint8_t cfgResBuffer[cfgResSize];

  // ARRANGE
  emberAfGetCurrentSenderEui64_ExpectAndReturn(NULL, EMBER_SUCCESS);
  emberAfGetCurrentSenderEui64_IgnoreArg_address();
  emberAfGetCurrentSenderEui64_ReturnThruPtr_address(eui64);

  emberAfCurrentCommand_ExpectAndReturn(&cmd);

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
