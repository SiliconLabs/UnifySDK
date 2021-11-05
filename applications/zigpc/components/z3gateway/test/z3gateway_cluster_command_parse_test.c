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

#include "enums.h"
#include "af_mock.h"
#include "address-table_mock.h"
#include "device-table_mock.h"

#include "z3gateway.h"
#include "z3gateway_common.h"
#include "test_z3gateway_callbacks_mock.h"

#define INSTALL_CODE_SIZE (16 + 2)

/* Function prototypes tested */
EmberAfStatus emberAfClusterSpecificCommandParse(EmberAfClusterCommand *cmd);
struct z3gatewayState z3gwState;

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
void setUp(void)
{
  device_table_mock_Init();
  static struct z3gatewayCallbacks callbacks = {
    .onClusterCommandReceived = callback_onClusterCommandReceived,
  };
  z3gwState.callbacks = &callbacks;
}

/**
 * @brief Teardown after each test case
 *
 */
void tearDown(void)
{
  device_table_mock_Destroy();
}

void test_z3gateway_cluster_command_received_callback_processed(void)
{
  uint8_t test_buffer[28]      = {0};
  EmberEUI64 test_eui64        = {0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x0, 0x10};
  EmberApsFrame test_aps_frame = {
    .sourceEndpoint = 2,
    .clusterId      = 0x0003,
  };
  EmberAfClusterCommand input_command = {
    .apsFrame          = &test_aps_frame,
    .source            = 0xFF10,
    .bufLen            = 28,
    .buffer            = test_buffer,
    .commandId         = 0x20,
    .direction         = ZCL_DIRECTION_SERVER_TO_CLIENT,
    .payloadStartIndex = 10,
  };

  emberAfGetCurrentSenderEui64_ExpectAndReturn(NULL, EMBER_SUCCESS);
  emberAfGetCurrentSenderEui64_IgnoreArg_address();
  emberAfGetCurrentSenderEui64_ReturnThruPtr_address(test_eui64);

  callback_onClusterCommandReceived_ExpectAndReturn(
    test_eui64,
    test_aps_frame.sourceEndpoint,
    test_aps_frame.clusterId,
    input_command.commandId,
    true,
    test_buffer,
    input_command.bufLen,
    input_command.payloadStartIndex,
    EMBER_ZCL_STATUS_SUCCESS);

  // ACT
  EmberAfStatus status = emberAfClusterSpecificCommandParse(&input_command);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_ZCL_STATUS_SUCCESS, status);
}

void test_z3gateway_cluster_command_received_callback_not_found(void)
{
  uint8_t test_buffer[28]      = {0};
  EmberEUI64 test_eui64        = {0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x0, 0x10};
  EmberApsFrame test_aps_frame = {
    .sourceEndpoint = 2,
    .clusterId      = 0x0003,
  };
  EmberAfClusterCommand input_command = {
    .apsFrame          = &test_aps_frame,
    .source            = 0xFF10,
    .bufLen            = 28,
    .buffer            = test_buffer,
    .commandId         = 0x20,
    .direction         = ZCL_DIRECTION_SERVER_TO_CLIENT,
    .payloadStartIndex = 10,
  };

  emberAfDeviceTableGetEui64FromNodeId_ExpectAndReturn(input_command.source,
                                                       NULL,
                                                       true);
  emberAfDeviceTableGetEui64FromNodeId_ReturnThruPtr_eui64(test_eui64);
  emberAfDeviceTableGetEui64FromNodeId_IgnoreArg_eui64();

  z3gwState.callbacks->onClusterCommandReceived = NULL;

  // ACT
  EmberAfStatus status = emberAfClusterSpecificCommandParse(&input_command);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER, status);
}

void test_z3gateway_cluster_command_received_device_not_found(void)
{
  uint8_t test_buffer[28]      = {0};
  EmberApsFrame test_aps_frame = {
    .sourceEndpoint = 2,
    .clusterId      = 0x0003,
  };
  EmberAfClusterCommand input_command = {
    .apsFrame          = &test_aps_frame,
    .source            = 0xFF10,
    .bufLen            = 28,
    .buffer            = test_buffer,
    .commandId         = 0x20,
    .direction         = ZCL_DIRECTION_SERVER_TO_CLIENT,
    .payloadStartIndex = 10,
  };

  emberAfDeviceTableGetEui64FromNodeId_ExpectAndReturn(input_command.source,
                                                       NULL,
                                                       false);
  emberAfDeviceTableGetEui64FromNodeId_IgnoreArg_eui64();

  // ACT
  EmberAfStatus status = emberAfClusterSpecificCommandParse(&input_command);

  // ASSERT
  TEST_ASSERT_EQUAL(EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER, status);
}