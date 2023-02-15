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

// Shared Unify includes
#include <unity.h>
#include <sys/clock.h>
#include <sys/process.h>
#include <contiki_test_helper.h>

// ZigPC includes
#include <zcl_definitions.h>
#include "zigpc_datastore_mock.h"
#include "zigbee_host_mock.h"

// Component includes
#include "zigpc_gateway.h"
#include "zigpc_gateway_int.h"

PROCESS_NAME(zigpc_gateway_process);

static const clock_time_t TICK_DURATION_MS = (CLOCK_SECOND / 80);

static const zigbee_eui64_t TEST_EUI64
  = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF};

static const zigbee_eui64_t TEST_EUI64_LE
  = {0xF, 0xE, 0xE, 0xB, 0xD, 0xA, 0xE, 0xD};

static const EmberEUI64 TEST_EUI64_EMBER =
    {0xF, 0xE, 0xE, 0xB, 0xD, 0xA, 0xE, 0xD};

static const size_t DEFER_CYCLES_DEFAULT               = 2U;
static const size_t DEFER_CYCLES_MESSAGE_LIMIT_REACHED = 10U;
static const size_t DEFER_CYCLES_START_BACKOFF         = 20U;
static const size_t DEFER_CYCLES_LOST_CONNECTION       = 50U;

/**
 * Setup the test suite (called once before all test_xxx functions are called)
 *
 */
void suiteSetUp(void) {}

/**
 * Teardown the test suite (called once after all test_xxx functions are called)
 *
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/**
 * @brief Mock helper to expect ZigbeeHostTicks
 *
 * @param defer_cycles RequestQueue defer cycles
 */
void helper_expect_zigbee_host_tick_calls(size_t defer_cycles)
{
  while (defer_cycles--) {
    zigbeeHostTick_Expect();
  }
}

/**
 * Setup before each test case
 *
 */
void setUp(void)
{
  zigbee_host_mock_Init();
  contiki_test_helper_init();
  process_start(&zigpc_gateway_process, NULL);
  contiki_test_helper_run(0);

  // Expect calls until RequestQueue is ready to send commands
  zigbeeHostSetEzspPolicy_IgnoreAndReturn(EZSP_SUCCESS);
  helper_expect_zigbee_host_tick_calls(DEFER_CYCLES_START_BACKOFF);
  zigpc_gateway_on_ncp_post_reset(false);
  contiki_test_helper_run_once(TICK_DURATION_MS * DEFER_CYCLES_START_BACKOFF);
}

/**
 * Teardown after each test case
 *
 */
void tearDown(void)
{
  process_exit(&zigpc_gateway_process);
  contiki_test_helper_run(0);
  zigbee_host_mock_Verify();
  zigbee_host_mock_Destroy();
}

void test_zigpc_gateway_network_init_call(void)
{
  // ARRANGE
  zigbeeHostTrustCenterInit_ExpectAndReturn(EMBER_SUCCESS);
  helper_expect_zigbee_host_tick_calls(DEFER_CYCLES_DEFAULT);

  // ACT
  sl_status_t status = zigpc_gateway_network_init();

  contiki_test_helper_run(TICK_DURATION_MS * 2);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_gateway_network_permit_joins_call(void)
{
  // ARRANGE
  zigbeeHostTrustCenterJoinOpen_ExpectAndReturn(true, EMBER_SUCCESS);
  helper_expect_zigbee_host_tick_calls(DEFER_CYCLES_DEFAULT);

  // ignoring call to print NWK key
  zigbeeHostGetEmberKey_IgnoreAndReturn(EMBER_NOT_FOUND);

  // ACT
  sl_status_t status = zigpc_gateway_network_permit_joins(true);

  contiki_test_helper_run(TICK_DURATION_MS * 2);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_gateway_network_deny_joins_call(void)
{
  // ARRANGE
  zigbeeHostTrustCenterJoinClose_ExpectAndReturn(EMBER_SUCCESS);
  helper_expect_zigbee_host_tick_calls(DEFER_CYCLES_DEFAULT);

  // ACT
  sl_status_t status = zigpc_gateway_network_permit_joins(false);

  contiki_test_helper_run(TICK_DURATION_MS * 2);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_gateway_add_node_install_code_call(void)
{
  zigbee_install_code_t install_code = {};
  uint8_t install_code_length        = 10U;

  // ARRANGE
  zigbeeHostTrustCenterAddDeviceInstallCode_ExpectAndReturn(TEST_EUI64_LE,
                                                            install_code,
                                                            install_code_length,
                                                            EMBER_SUCCESS);
  helper_expect_zigbee_host_tick_calls(DEFER_CYCLES_DEFAULT);

  // ACT
  sl_status_t status_null_eui64
    = zigpc_gateway_add_node_install_code(NULL,
                                          install_code,
                                          install_code_length);
  sl_status_t status_null_ic
    = zigpc_gateway_add_node_install_code(TEST_EUI64,
                                          NULL,
                                          install_code_length);
  sl_status_t status = zigpc_gateway_add_node_install_code(TEST_EUI64,
                                                           install_code,
                                                           install_code_length);

  contiki_test_helper_run(TICK_DURATION_MS * 2);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status_null_eui64);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status_null_ic);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_gateway_discover_device_call(void)
{
  // ARRANGE
  zigbeeHostZdoActiveEndpointsRequest_ExpectAndReturn(TEST_EUI64_LE,
                                                      EMBER_SUCCESS);
  helper_expect_zigbee_host_tick_calls(DEFER_CYCLES_DEFAULT);

  // ACT
  sl_status_t status_null_eui64 = zigpc_gateway_discover_device(NULL);
  sl_status_t status            = zigpc_gateway_discover_device(TEST_EUI64);

  contiki_test_helper_run(TICK_DURATION_MS * 2);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status_null_eui64);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_gateway_remove_node_call(void)
{
  // ARRANGE
  zigbeeHostNetworkDeviceLeaveRequest_ExpectAndReturn(TEST_EUI64_LE,
                                                      EMBER_SUCCESS);
  helper_expect_zigbee_host_tick_calls(DEFER_CYCLES_DEFAULT);

  // ACT
  sl_status_t status_null_eui64 = zigpc_gateway_remove_node(NULL);
  sl_status_t status            = zigpc_gateway_remove_node(TEST_EUI64);

  contiki_test_helper_run(TICK_DURATION_MS * 2);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status_null_eui64);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_gateway_send_zcl_command_frame_call(void)
{
  zigbee_endpoint_id_t test_endpoint = 1;
  zcl_cluster_id_t test_zcl_cluster  = 0x05;
  zcl_frame_t test_zcl_frame         = {0};

  // ARRANGE
  zigpc_datastore_contains_cluster_ExpectAndReturn(
    TEST_EUI64,
    test_endpoint,
    ZCL_CLUSTER_SERVER_SIDE,
    ZIGPC_ZCL_CLUSTER_POLL_CONTROL,
    false);

  zigbeeHostFillZclFrame_ExpectAndReturn(test_zcl_frame.buffer,
                                         test_zcl_frame.size,
                                         test_zcl_frame.offset_sequence_id,
                                         EMBER_SUCCESS);

  zigbeeHostSendZclFrameUnicast_ExpectAndReturn(TEST_EUI64_LE,
                                                test_endpoint,
                                                test_zcl_cluster,
                                                EMBER_SUCCESS);
  helper_expect_zigbee_host_tick_calls(DEFER_CYCLES_DEFAULT);

  // ACT
  sl_status_t status_null_eui64
    = zigpc_gateway_send_zcl_command_frame(NULL,
                                           test_endpoint,
                                           test_zcl_cluster,
                                           &test_zcl_frame);
  sl_status_t status_null_frame
    = zigpc_gateway_send_zcl_command_frame(TEST_EUI64,
                                           test_endpoint,
                                           test_zcl_cluster,
                                           NULL);
  sl_status_t status = zigpc_gateway_send_zcl_command_frame(TEST_EUI64,
                                                            test_endpoint,
                                                            test_zcl_cluster,
                                                            &test_zcl_frame);

  contiki_test_helper_run(TICK_DURATION_MS * 2);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status_null_eui64);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status_null_frame);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_gateway_send_zcl_frame_multicast_call(void)
{
  zigbee_group_id_t test_group      = 0x4;
  zcl_cluster_id_t test_zcl_cluster = 0x05;
  zcl_frame_t test_zcl_frame        = {0};

  // ARRANGE
  zigbeeHostFillZclFrame_ExpectAndReturn(test_zcl_frame.buffer,
                                         test_zcl_frame.size,
                                         test_zcl_frame.offset_sequence_id,
                                         EMBER_SUCCESS);

  zigbeeHostSendZclFrameMulticast_ExpectAndReturn(test_group,
                                                  test_zcl_cluster,
                                                  EMBER_SUCCESS);
  helper_expect_zigbee_host_tick_calls(DEFER_CYCLES_DEFAULT);

  // ACT
  sl_status_t status_null_frame
    = zigpc_gateway_send_zcl_frame_multicast(test_group,
                                             test_zcl_cluster,
                                             NULL);
  sl_status_t status = zigpc_gateway_send_zcl_frame_multicast(test_group,
                                                              test_zcl_cluster,
                                                              &test_zcl_frame);

  contiki_test_helper_run(TICK_DURATION_MS * 2);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status_null_frame);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_gateway_configure_reports_call(void)
{
  zigbee_endpoint_id_t test_endpoint = 2;
  zcl_cluster_id_t test_zcl_cluster  = 0x05;
  zcl_frame_t test_zcl_frame         = {0};

  test_zcl_frame.size = 10;

  // ARRANGE
  zigbeeHostInitReporting_ExpectAndReturn(TEST_EUI64_LE,
                                          test_endpoint,
                                          test_zcl_cluster,
                                          test_zcl_frame.buffer,
                                          test_zcl_frame.size,
                                          EMBER_SUCCESS);

  helper_expect_zigbee_host_tick_calls(DEFER_CYCLES_DEFAULT);

  // ACT
  sl_status_t status_null_eui64
    = zigpc_gateway_configure_reports(NULL,
                                      test_endpoint,
                                      test_zcl_cluster,
                                      test_zcl_frame.buffer,
                                      test_zcl_frame.size);
  sl_status_t status_null_frame
    = zigpc_gateway_configure_reports(TEST_EUI64,
                                      test_endpoint,
                                      test_zcl_cluster,
                                      NULL,
                                      test_zcl_frame.size);
  sl_status_t status = zigpc_gateway_configure_reports(TEST_EUI64,
                                                       test_endpoint,
                                                       test_zcl_cluster,
                                                       test_zcl_frame.buffer,
                                                       test_zcl_frame.size);

  contiki_test_helper_run(TICK_DURATION_MS * 2);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status_null_eui64);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status_null_frame);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_gateway_request_binding_call(void)
{
  zigbee_endpoint_id_t test_endpoint = 7;
  zcl_cluster_id_t test_zcl_cluster  = 0x08;
  EmberEUI64 destEui64 = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
  EmberEUI64 destEui64_le = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint8_t destEndpoint = 1;

  // ARRANGE
  zigbeeHostInitBinding_ExpectAndReturn(TEST_EUI64_EMBER,
                                        (uint8_t)test_endpoint,
                                        (uint16_t)test_zcl_cluster,
                                        0,
                                        destEui64_le,
                                        destEndpoint,
                                        true,
                                        EMBER_SUCCESS);

  helper_expect_zigbee_host_tick_calls(DEFER_CYCLES_DEFAULT);

  // ACT
  sl_status_t status_null_eui64
    = zigpc_gateway_request_binding(
            NULL, 
            test_endpoint, 
            test_zcl_cluster,
            destEui64,
            destEndpoint);

  sl_status_t status = zigpc_gateway_request_binding(
                        TEST_EUI64,
                        test_endpoint,
                        test_zcl_cluster,
                        destEui64,
                        destEndpoint);

  contiki_test_helper_run(TICK_DURATION_MS * 2);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status_null_eui64);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_gateway_add_ota_image_call(void)
{
  const char *ota_path       = "/tmp/ota.s37";
  unsigned int ota_path_size = sizeof(ota_path) / sizeof(char);

  // ARRANGE
  zigbeeHostAddOtaImage_ExpectAndReturn(ota_path, EMBER_SUCCESS);

  helper_expect_zigbee_host_tick_calls(DEFER_CYCLES_DEFAULT);

  // ACT
  sl_status_t status_null_path
    = zigpc_gateway_add_ota_image(NULL, ota_path_size);
  sl_status_t status = zigpc_gateway_add_ota_image(ota_path, ota_path_size);

  contiki_test_helper_run(TICK_DURATION_MS * 2);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_FAIL, status_null_path);
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_gateway_too_many_messages_defer_call(void)
{
  // ARRANGE
  zigbeeHostTrustCenterJoinOpen_ExpectAndReturn(
    true,
    EMBER_MAX_MESSAGE_LIMIT_REACHED);

  // ignoring call to print NWK key
  zigbeeHostGetEmberKey_IgnoreAndReturn(EMBER_NOT_FOUND);

  helper_expect_zigbee_host_tick_calls(DEFER_CYCLES_MESSAGE_LIMIT_REACHED + 3);

  zigbeeHostTrustCenterJoinOpen_ExpectAndReturn(true, EMBER_SUCCESS);

  // ignoring call to print NWK key
  zigbeeHostGetEmberKey_IgnoreAndReturn(EMBER_NOT_FOUND);

  // ACT
  sl_status_t status = zigpc_gateway_network_permit_joins(true);

  for (size_t i = 0; i < (DEFER_CYCLES_MESSAGE_LIMIT_REACHED + 3); i++) {
    contiki_test_helper_run(TICK_DURATION_MS);
  }

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

zcl_cluster_id_t test_sleepy_endpoint    = 3;
zcl_cluster_id_t test_sleepy_zcl_cluster = 0x05;
static zcl_frame_t test_sleepy_zcl_frame = {
  .buffer             = "\x2\x4\x6\x8",
  .size               = 4,
  .offset_sequence_id = 1,
};
void test_zigpc_gateway_store_zcl_command_frame_sleepy_call(void)
{
  // ARRANGE
  zigpc_datastore_contains_cluster_ExpectAndReturn(
    TEST_EUI64,
    test_sleepy_endpoint,
    ZCL_CLUSTER_SERVER_SIDE,
    ZIGPC_ZCL_CLUSTER_POLL_CONTROL,
    true);
  helper_expect_zigbee_host_tick_calls(DEFER_CYCLES_DEFAULT);

  // ACT
  sl_status_t status
    = zigpc_gateway_send_zcl_command_frame(TEST_EUI64,
                                           test_sleepy_endpoint,
                                           test_sleepy_zcl_cluster,
                                           &test_sleepy_zcl_frame);
  contiki_test_helper_run(TICK_DURATION_MS * 2);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_zigpc_gateway_unload_zcl_command_frame_sleepy_call(void)
{
  // ARRANGE
  zigbeeHostSendPollingCheckInResponse_ExpectAndReturn(true, EMBER_SUCCESS);

  zigbeeHostFillZclFrame_ExpectAndReturn(
    test_sleepy_zcl_frame.buffer,
    test_sleepy_zcl_frame.size,
    test_sleepy_zcl_frame.offset_sequence_id,
    EMBER_SUCCESS);

  zigbeeHostSendZclFrameUnicast_ExpectAndReturn(TEST_EUI64_LE,
                                                test_sleepy_endpoint,
                                                test_sleepy_zcl_cluster,
                                                EMBER_SUCCESS);
  helper_expect_zigbee_host_tick_calls(DEFER_CYCLES_DEFAULT);

  // ACT
  sl_status_t status = zigpc_gateway_unload_sleepy_messages(TEST_EUI64);
  contiki_test_helper_run(TICK_DURATION_MS * 2);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_EMPTY, status);

  // Check to see if no messages are unloaded again
  zigbeeHostSendPollingCheckInResponse_ExpectAndReturn(false, EMBER_SUCCESS);
  
  // ARRANGE
  helper_expect_zigbee_host_tick_calls(DEFER_CYCLES_DEFAULT);

  // ACT
  status = zigpc_gateway_unload_sleepy_messages(TEST_EUI64);
  contiki_test_helper_run(TICK_DURATION_MS * 2);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_EMPTY, status);
}

/**
 * @brief This scenario is for a SINGLE send failure case due to the EZSP
 * receiver not being available.
 *
 */
void test_zigpc_gateway_pause_dispatching_on_ncp_reset(void)
{
  // ARRANGE (pre-reset)
  helper_expect_zigbee_host_tick_calls(DEFER_CYCLES_DEFAULT);

  // ACT (pre-reset)
  // Simulate NCP start resetting
  zigpc_gateway_on_ncp_pre_reset(EZSP_ASH_NCP_FATAL_ERROR);
  // Send Network close request
  sl_status_t status = zigpc_gateway_network_permit_joins(false);
  contiki_test_helper_run(TICK_DURATION_MS * DEFER_CYCLES_DEFAULT);

  // ASSERT (pre-reset)
  // Ensure request is accepted by queue
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);

  // ARRANGE (post-reset)
  zigbeeHostTrustCenterJoinClose_ExpectAndReturn(EMBER_SUCCESS);
  helper_expect_zigbee_host_tick_calls(DEFER_CYCLES_START_BACKOFF
                                       + DEFER_CYCLES_DEFAULT);
  zigbeeHostSetEzspPolicy_IgnoreAndReturn(EZSP_SUCCESS);

  // ACT (post-reset)
  // Simulate NCP complete reset + init
  zigpc_gateway_on_ncp_post_reset(false);
  contiki_test_helper_run_once(TICK_DURATION_MS * DEFER_CYCLES_START_BACKOFF);
  contiki_test_helper_run(TICK_DURATION_MS * DEFER_CYCLES_DEFAULT);

  // ASSERT (post-reset): Handled by CMock
}

/**
 * @brief This scenario is for MULTIPLE send failure case due to the EZSP
 * receiver not being available.
 *
 * In this scenario, the Permitjoins request should be kept in the
 * RequestQueue until EZSP_NOT_CONNECTED status is not returned.
 *
 */
void test_zigpc_gateway_keep_retrying_on_ncp_not_ready(void)
{
  // ARRANGE (pre-reset)
  helper_expect_zigbee_host_tick_calls(DEFER_CYCLES_DEFAULT);

  // ACT (pre-reset)
  // Simulate NCP start resetting
  zigpc_gateway_on_ncp_pre_reset(EZSP_NOT_CONNECTED);
  // Send Network close request
  sl_status_t status = zigpc_gateway_network_permit_joins(false);
  contiki_test_helper_run(TICK_DURATION_MS * DEFER_CYCLES_DEFAULT);

  // ASSERT (pre-reset)
  // Ensure request is accepted by queue
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);

  // ARRANGE (post-reset-ncp-not-ready)
  zigbeeHostSetEzspPolicy_IgnoreAndReturn(EZSP_SUCCESS);
  zigbeeHostTrustCenterJoinClose_ExpectAndReturn(EZSP_NOT_CONNECTED);
  helper_expect_zigbee_host_tick_calls(DEFER_CYCLES_LOST_CONNECTION
                                       + DEFER_CYCLES_DEFAULT);

  // ACT (post-reset-ncp-not-ready)
  // Simulate NCP post reset
  zigpc_gateway_on_ncp_post_reset(false);
  contiki_test_helper_run_once(TICK_DURATION_MS * DEFER_CYCLES_LOST_CONNECTION);
  contiki_test_helper_run(TICK_DURATION_MS * DEFER_CYCLES_DEFAULT);

  // ASSERT (post-reset-ncp-not-ready): Handled by CMock

  // ARRANGE (post-reset-ncp-ready)
  zigbeeHostTrustCenterJoinClose_ExpectAndReturn(EMBER_SUCCESS);
  helper_expect_zigbee_host_tick_calls(DEFER_CYCLES_START_BACKOFF
                                       + DEFER_CYCLES_DEFAULT);

  // ACT (post-reset-ncp-ready)
  zigpc_gateway_on_ncp_post_reset(false);
  contiki_test_helper_run_once(TICK_DURATION_MS * DEFER_CYCLES_START_BACKOFF);
  contiki_test_helper_run(TICK_DURATION_MS * DEFER_CYCLES_DEFAULT);

  // ASSERT (post-reset-ncp-ready): Handled by CMock
}
