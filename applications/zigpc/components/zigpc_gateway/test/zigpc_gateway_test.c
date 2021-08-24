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
#include <stdlib.h>
#include <string.h>

#include "unity.h"

#include "zigpc_common_zigbee_mock.h"
#include "zigpc_node_mgmt_mock.h"

#include "zigpc_gateway.h"
#include "zigpc_gateway_int.h"
#include "zigpc_gateway_process_send_mock.h"

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

static const zigbee_eui64_t test_eui64
  = {0xD, 0xE, 0xA, 0xD, 0xB, 0xE, 0xE, 0xF};

/**
 * Setup before each test case
 *
 */
void setUp(void) {}

/**
 * Teardown after each test case
 *
 */
void tearDown(void) {}

void test_zigpc_gateway_network_init_calls_process(void)
{
  // ARRANGE
  sl_status_t test_status;
  zigbee_eui64_copy_switch_endian_Ignore();
  zigpc_gateway_process_send_event_ExpectAndReturn(
    ZIGPC_GW_EVENT_DISPATCH,
    NULL,
    sizeof(struct zigpc_gateway_dispatch_event),
    SL_STATUS_OK);
  zigpc_gateway_process_send_event_IgnoreArg_data();

  // ACT
  test_status = zigpc_gateway_network_init();

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
}

void test_zigpc_gateway_add_node_install_code_calls_process(void)
{
  // ARRANGE
  sl_status_t test_status;
  const zigbee_install_code_t install_code = {};
  zigbee_eui64_copy_switch_endian_Ignore();
  zigpc_gateway_process_send_event_ExpectAndReturn(
    ZIGPC_GW_EVENT_DISPATCH,
    NULL,
    sizeof(struct zigpc_gateway_dispatch_event),
    SL_STATUS_OK);
  zigpc_gateway_process_send_event_IgnoreArg_data();

  // ACT
  test_status
    = zigpc_gateway_add_node_install_code(test_eui64, install_code, 0);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
}

void test_zigpc_gateway_interview_node_calls_process(void)
{
  // ARRANGE
  sl_status_t test_status;
  zigbee_eui64_copy_switch_endian_Ignore();
  zigpc_gateway_process_send_event_ExpectAndReturn(
    ZIGPC_GW_EVENT_DISPATCH,
    NULL,
    sizeof(struct zigpc_gateway_dispatch_event),
    SL_STATUS_OK);
  zigpc_gateway_process_send_event_IgnoreArg_data();

  // ACT
  test_status = zigpc_gateway_interview_node(test_eui64);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
}

void test_zigpc_gateway_send_cluster_command_calls_process(void)
{
  // ARRANGE
  sl_status_t test_status;
  zcl_cluster_id_t test_zcl_cluster = 0x05;
  zcl_frame_t test_zcl_frame        = {0};
  zigbee_eui64_copy_switch_endian_Ignore();
  zigbee_eui64_to_str_IgnoreAndReturn(SL_STATUS_OK);

  zigpc_node_check_sleepy_ExpectAndReturn(test_eui64, false);
  zigpc_gateway_process_send_event_ExpectAndReturn(
    ZIGPC_GW_EVENT_ZCL_FRAME,
    NULL,
    sizeof(zigpc_gateway_dispatch_zcl_frame_t),
    SL_STATUS_OK);
  zigpc_gateway_process_send_event_IgnoreArg_data();

  // ACT
  test_status = zigpc_gateway_send_zcl_command_frame(test_eui64,
                                                     0,
                                                     test_zcl_cluster,
                                                     &test_zcl_frame);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
}

void test_zigpc_gateway_remove_node_calls_process(void)
{
  // ARRANGE
  sl_status_t test_status;
  zigbee_eui64_copy_switch_endian_Ignore();
  zigpc_gateway_process_send_event_ExpectAndReturn(
    ZIGPC_GW_EVENT_DISPATCH,
    NULL,
    sizeof(struct zigpc_gateway_dispatch_event),
    SL_STATUS_OK);
  zigpc_gateway_process_send_event_IgnoreArg_data();

  // ACT
  test_status = zigpc_gateway_remove_node(test_eui64);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, test_status);
}
