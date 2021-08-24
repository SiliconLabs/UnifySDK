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

#include <string.h>

#include "unity.h"

#include "zcl_definitions.h"
#include "zcl_util_mock.h"
#include "zigpc_common_zigbee_mock.h"
#include "zigpc_gateway_mock.h"
#include "zigpc_node_mgmt_mock.h"

#include "zigpc_command_mapper_int.h"

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
 * Setup before each test case
 *
 */
void setUp(void) {}

/**
 * Teardown after each test case
 *
 */
void tearDown(void) {}

void test_zigpc_command_mapper_populate_eui64_invalid_input(void)
{
  // ARRANGE
  sl_status_t test_null;
  sl_status_t test_null_unid;
  sl_status_t test_null_eui64;
  char test_unid;
  zigbee_eui64_t test_eui64;

  // ACT
  test_null       = zigpc_command_mapper_populate_eui64(NULL, NULL);
  test_null_unid  = zigpc_command_mapper_populate_eui64(NULL, test_eui64);
  test_null_eui64 = zigpc_command_mapper_populate_eui64(&test_unid, NULL);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_NULL_POINTER, test_null);
  TEST_ASSERT_EQUAL(SL_STATUS_NULL_POINTER, test_null_unid);
  TEST_ASSERT_EQUAL(SL_STATUS_NULL_POINTER, test_null_eui64);
}

void test_zigpc_command_mapper_populate_eui64_happy_path(void)
{
  // ARRANGE
  sl_status_t status;
  char *test_unid = "zb-0011223344556677";
  zigbee_eui64_t test_eui64;
  zigbee_eui64_t expected_eui64
    = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};

  str_to_zigbee_eui64_ExpectAndReturn(test_unid, 19, NULL, SL_STATUS_OK);
  str_to_zigbee_eui64_IgnoreArg_eui64();
  str_to_zigbee_eui64_ReturnArrayThruPtr_eui64(expected_eui64,
                                               ZIGBEE_EUI64_SIZE);

  // ACT
  status = zigpc_command_mapper_populate_eui64(test_unid, test_eui64);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_MEMORY(expected_eui64, test_eui64, sizeof(zigbee_eui64_t));
}

void test_zigpc_command_mapper_populate_eui64_invalid_unid(void)
{
  // ARRANGE
  sl_status_t status;
  char *test_unid = "zb-0000-1111-2222-3333";
  zigbee_eui64_t test_eui64;

  str_to_zigbee_eui64_IgnoreAndReturn(SL_STATUS_FAIL);

  // ACT
  status = zigpc_command_mapper_populate_eui64(test_unid, test_eui64);

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_SIGNATURE, status);
}

void test_zigpc_command_mapper_cluster_support_check_invalid_input(void)
{
  // ARRANGE
  sl_status_t null_unid;

  // ACT
  null_unid = zigpc_command_mapper_cluster_support_check(NULL, 0, 0);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_NULL_POINTER, null_unid);
}

void test_zigpc_command_mapper_cluster_support_check_entry_not_found(void)
{
  // ARRANGE
  sl_status_t status;
  dotdot_unid_t unid   = "zb-0011223344556677";
  zigbee_eui64_t eui64 = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};

  str_to_zigbee_eui64_ExpectAndReturn(unid, strlen(unid), NULL, SL_STATUS_OK);
  str_to_zigbee_eui64_IgnoreArg_eui64();
  str_to_zigbee_eui64_ReturnMemThruPtr_eui64(eui64, sizeof(zigbee_eui64_t));
  zigpc_nodemgmt_fetch_node_ep_cluster_ExpectAndReturn(eui64,
                                                       0,
                                                       0,
                                                       NULL,
                                                       SL_STATUS_FAIL);
  zigpc_nodemgmt_fetch_node_ep_cluster_IgnoreArg_cluster();

  // ACT
  status = zigpc_command_mapper_cluster_support_check(unid, 0, 0);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_NOT_FOUND, status);
}

void test_zigpc_command_mapper_cluster_support_check_entry_found(void)
{
  // ARRANGE
  sl_status_t status;
  dotdot_unid_t unid   = "zb-0011223344556677";
  zigbee_eui64_t eui64 = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};

  str_to_zigbee_eui64_ExpectAndReturn(unid, strlen(unid), NULL, SL_STATUS_OK);
  str_to_zigbee_eui64_IgnoreArg_eui64();
  str_to_zigbee_eui64_ReturnMemThruPtr_eui64(eui64, sizeof(zigbee_eui64_t));
  zigpc_nodemgmt_fetch_node_ep_cluster_ExpectAndReturn(eui64,
                                                       0,
                                                       0,
                                                       NULL,
                                                       SL_STATUS_OK);
  zigpc_nodemgmt_fetch_node_ep_cluster_IgnoreArg_cluster();

  // ACT
  status = zigpc_command_mapper_cluster_support_check(unid, 0, 0);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}

void test_zigpc_command_mapper_send_unicast_invalid_input(void)
{
  // ARRANGE
  sl_status_t null_unid_status;

  // ACT
  null_unid_status
    = zigpc_command_mapper_send_unicast(NULL, 0, 0, 0, 0, 0, NULL);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_NULL_POINTER, null_unid_status);
}

void test_zigpc_command_mapper_send_unicast_sanity_no_args(void)
{
  // ARRANGE
  sl_status_t status;
  dotdot_unid_t unid   = "zb-0011223344556677";
  zigbee_eui64_t eui64 = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
  zigbee_endpoint_id_t endpoint_id = 3;
  zcl_cluster_id_t cluster_id      = 0x1234;
  zcl_command_id_t cmd_id          = 0xFE;

  str_to_zigbee_eui64_ExpectAndReturn(unid, strlen(unid), NULL, SL_STATUS_OK);
  str_to_zigbee_eui64_IgnoreArg_eui64();
  str_to_zigbee_eui64_ReturnMemThruPtr_eui64(eui64, sizeof(zigbee_eui64_t));
  zigpc_zcl_build_command_frame_ExpectAndReturn(
    NULL,
    ZIGPC_ZCL_FRAME_TYPE_CMD_TO_SERVER,
    cluster_id,
    cmd_id,
    0,
    NULL,
    SL_STATUS_OK);
  zigpc_zcl_build_command_frame_IgnoreArg_frame();
  zigpc_gateway_send_zcl_command_frame_ExpectAndReturn(eui64,
                                                       endpoint_id,
                                                       cluster_id,
                                                       NULL,
                                                       SL_STATUS_OK);
  zigpc_gateway_send_zcl_command_frame_IgnoreArg_frame();

  // ACT
  status = zigpc_command_mapper_send_unicast(unid,
                                             endpoint_id,
                                             ZIGPC_ZCL_FRAME_TYPE_CMD_TO_SERVER,
                                             cluster_id,
                                             cmd_id,
                                             0,
                                             NULL);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}

void test_zigpc_command_mapper_send_unicast_sanity_multiple_args(void)
{
  // ARRANGE
  sl_status_t status;
  dotdot_unid_t unid   = "zb-0011223344556677";
  zigbee_eui64_t eui64 = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
  zigbee_endpoint_id_t endpoint_id = 3;
  zcl_cluster_id_t cluster_id      = 0x1234;
  zcl_command_id_t cmd_id          = 0xFE;
  size_t cmd_args_count            = 2;
  uint32_t arg1_data               = 0x34128293;
  char *arg2_data                  = "Testing";
  zigpc_zcl_frame_data_t cmd_args_list[]
    = {{
         .type = ZIGPC_ZCL_DATA_TYPE_UINT32,
         .data = &arg1_data,
       },
       {
         .type = ZIGPC_ZCL_DATA_TYPE_STRING,
         .data = &arg2_data,
       }};

  str_to_zigbee_eui64_ExpectAndReturn(unid, strlen(unid), NULL, SL_STATUS_OK);
  str_to_zigbee_eui64_IgnoreArg_eui64();
  str_to_zigbee_eui64_ReturnMemThruPtr_eui64(eui64, sizeof(zigbee_eui64_t));
  zigpc_zcl_build_command_frame_ExpectAndReturn(
    NULL,
    ZIGPC_ZCL_FRAME_TYPE_CMD_TO_SERVER,
    cluster_id,
    cmd_id,
    cmd_args_count,
    cmd_args_list,
    SL_STATUS_OK);
  zigpc_zcl_build_command_frame_IgnoreArg_frame();
  zigpc_gateway_send_zcl_command_frame_ExpectAndReturn(eui64,
                                                       endpoint_id,
                                                       cluster_id,
                                                       NULL,
                                                       SL_STATUS_OK);
  zigpc_gateway_send_zcl_command_frame_IgnoreArg_frame();

  // ACT
  status = zigpc_command_mapper_send_unicast(unid,
                                             endpoint_id,
                                             ZIGPC_ZCL_FRAME_TYPE_CMD_TO_SERVER,
                                             cluster_id,
                                             cmd_id,
                                             cmd_args_count,
                                             cmd_args_list);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}
