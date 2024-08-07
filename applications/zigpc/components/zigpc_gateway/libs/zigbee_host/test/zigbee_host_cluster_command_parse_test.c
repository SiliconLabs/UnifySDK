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

// Zigbee stack includes
#define SIGNED_ENUM
#include "zap-type.h"
#include "af_mock.h"
#include "address-table_mock.h"

#include "zigbee_host.h"
#include "zigbee_host_common.h"
#include "test_zigbee_host_callbacks_mock.h"

#define INSTALL_CODE_SIZE (16 + 2)

/* Function prototypes tested */
sl_zigbee_af_status_t
  emberAfClusterServiceCallback(sl_service_opcode_t opcode,
                                sl_service_function_context_t *context);
struct zigbeeHostState z3gwState;

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
  static struct zigbeeHostCallbacks callbacks = {
    .onClusterCommandReceived = callback_onClusterCommandReceived,
  };
  z3gwState.callbacks = &callbacks;
}

/**
 * @brief Teardown after each test case
 *
 */
void tearDown(void) {}

void test_zigbee_host_cluster_command_received_callback_processed(void)
{
  uint8_t test_buffer[28]      = {0};
  sl_802154_long_addr_t test_eui64        = {0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x0, 0x10};
  sl_zigbee_aps_frame_t test_aps_frame = {
    .sourceEndpoint = 2,
    .clusterId      = 0x0003,
  };
  sl_zigbee_af_cluster_command_t input_command = {
    .apsFrame          = &test_aps_frame,
    .source            = 0xFF10,
    .bufLen            = 28,
    .buffer            = test_buffer,
    .commandId         = 0x20,
    .direction         = ZCL_DIRECTION_SERVER_TO_CLIENT,
    .payloadStartIndex = 10,
  };

  sl_zigbee_af_get_current_sender_eui64_ExpectAndReturn(NULL, SL_STATUS_OK );
  sl_zigbee_af_get_current_sender_eui64_IgnoreArg_address();
  sl_zigbee_af_get_current_sender_eui64_ReturnThruPtr_address(test_eui64);

  callback_onClusterCommandReceived_ExpectAndReturn(
    test_eui64,
    test_aps_frame.sourceEndpoint,
    test_aps_frame.clusterId,
    input_command.commandId,
    true,
    test_buffer,
    input_command.bufLen,
    input_command.payloadStartIndex,
    SL_ZIGBEE_ZCL_STATUS_SUCCESS);

  // ACT
  sl_service_function_context_t context = {.data = &input_command};
  sl_zigbee_af_status_t status
    = emberAfClusterServiceCallback(SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND,
                                    &context);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_ZIGBEE_ZCL_STATUS_SUCCESS, status);
}

void test_zigbee_host_cluster_command_received_callback_not_found(void)
{
  uint8_t test_buffer[28]      = {0};
  sl_zigbee_aps_frame_t test_aps_frame = {
    .sourceEndpoint = 2,
    .clusterId      = 0x0003,
  };
  sl_zigbee_af_cluster_command_t input_command = {
    .apsFrame          = &test_aps_frame,
    .source            = 0xFF10,
    .bufLen            = 28,
    .buffer            = test_buffer,
    .commandId         = 0x20,
    .direction         = ZCL_DIRECTION_SERVER_TO_CLIENT,
    .payloadStartIndex = 10,
  };

  z3gwState.callbacks->onClusterCommandReceived = NULL;

  // ACT
  sl_service_function_context_t context = {.data = &input_command};
  sl_zigbee_af_status_t status
    = emberAfClusterServiceCallback(SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND,
                                    &context);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_ZIGBEE_ZCL_STATUS_UNSUPPORTED_CLUSTER, status);
}

void test_zigbee_host_cluster_command_received_device_not_found(void)
{
  uint8_t test_buffer[28]      = {0};
  sl_zigbee_aps_frame_t test_aps_frame = {
    .sourceEndpoint = 2,
    .clusterId      = 0x0003,
  };
  sl_zigbee_af_cluster_command_t input_command = {
    .apsFrame          = &test_aps_frame,
    .source            = 0xFF10,
    .bufLen            = 28,
    .buffer            = test_buffer,
    .commandId         = 0x20,
    .direction         = ZCL_DIRECTION_SERVER_TO_CLIENT,
    .payloadStartIndex = 10,
  };

  z3gwState.callbacks->onClusterCommandReceived
    = callback_onClusterCommandReceived;

  sl_zigbee_af_get_current_sender_eui64_ExpectAndReturn(NULL, SL_STATUS_INVALID_STATE );
  sl_zigbee_af_get_current_sender_eui64_IgnoreArg_address();

  // ACT
  sl_service_function_context_t context = {.data = &input_command};
  sl_zigbee_af_status_t status
    = emberAfClusterServiceCallback(SL_SERVICE_FUNCTION_TYPE_ZCL_COMMAND,
                                    &context);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_ZIGBEE_ZCL_STATUS_FAILURE, status);
}