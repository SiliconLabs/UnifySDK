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

#include "zigpc_group_mgmt_mock.h"
#include "zigpc_common_zigbee_mock.h"
#include "zigpc_gateway_mock.h"
#include "zcl_util_mock.h"

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

void test_zigpc_command_mapper_send_multicast_invalid_group_id(void)
{
  // ARRANGE
  sl_status_t status;

  // ACT
  status = zigpc_command_mapper_send_multicast(0, 0, 0, 0, 0, NULL);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_INVALID_RANGE, status);
}

void test_zigpc_command_mapper_send_multicast_sanity_not_servicable(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_group_id_t group_id        = 0x2;
  zigpc_zcl_frame_type_t frame_type = 0x80;
  zcl_cluster_id_t cluster_id       = 0x0034;
  zcl_command_id_t cmd_id           = 0x03;

  zigpc_groupmgmt_get_member_count_ExpectAndReturn(group_id, 0);

  // ACT
  status = zigpc_command_mapper_send_multicast(group_id,
                                               frame_type,
                                               cluster_id,
                                               cmd_id,
                                               0,
                                               NULL);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_NOT_SUPPORTED, status);
}

void test_zigpc_command_mapper_send_multicast_sanity_no_args(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_group_id_t group_id        = 0x1;
  zigpc_zcl_frame_type_t frame_type = 0x80;
  zcl_cluster_id_t cluster_id       = 0x1234;
  zcl_command_id_t cmd_id           = 0xFE;

  zigpc_groupmgmt_get_member_count_ExpectAndReturn(group_id, 2);

  zigpc_zcl_build_command_frame_ExpectAndReturn(NULL,
                                                frame_type,
                                                cluster_id,
                                                cmd_id,
                                                0,
                                                NULL,
                                                SL_STATUS_OK);
  zigpc_zcl_build_command_frame_IgnoreArg_frame();
  zigpc_gateway_send_zcl_frame_multicast_ExpectAndReturn(group_id,
                                                         cluster_id,
                                                         NULL,
                                                         SL_STATUS_OK);
  zigpc_gateway_send_zcl_frame_multicast_IgnoreArg_frame();

  // ACT
  status = zigpc_command_mapper_send_multicast(group_id,
                                               frame_type,
                                               cluster_id,
                                               cmd_id,
                                               0,
                                               NULL);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}

void test_zigpc_command_mapper_send_multicast_sanity_multiple_args(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_group_id_t group_id        = 0x20;
  zigpc_zcl_frame_type_t frame_type = 0xF2;
  zcl_cluster_id_t cluster_id       = 0x1234;
  zcl_command_id_t cmd_id           = 0xFE;
  size_t cmd_args_count             = 2;
  uint32_t arg1_data                = 0x34128293;
  char *arg2_data                   = "Testing";
  zigpc_zcl_frame_data_t cmd_args_list[]
    = {{
         .type = ZIGPC_ZCL_DATA_TYPE_UINT32,
         .data = &arg1_data,
       },
       {
         .type = ZIGPC_ZCL_DATA_TYPE_STRING,
         .data = &arg2_data,
       }};

  zigpc_groupmgmt_get_member_count_ExpectAndReturn(group_id, 3);

  zigpc_zcl_build_command_frame_ExpectAndReturn(NULL,
                                                frame_type,
                                                cluster_id,
                                                cmd_id,
                                                cmd_args_count,
                                                cmd_args_list,
                                                SL_STATUS_OK);
  zigpc_zcl_build_command_frame_IgnoreArg_frame();

  zigpc_gateway_send_zcl_frame_multicast_ExpectAndReturn(group_id,
                                                         cluster_id,
                                                         NULL,
                                                         SL_STATUS_OK);

  zigpc_gateway_send_zcl_frame_multicast_IgnoreArg_frame();

  // ACT
  status = zigpc_command_mapper_send_multicast(group_id,
                                               frame_type,
                                               cluster_id,
                                               cmd_id,
                                               cmd_args_count,
                                               cmd_args_list);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}

void test_zigpc_command_mapper_redirect_groups_multicast_to_unicast(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_group_id_t group_id        = 0x1;
  zigpc_zcl_frame_type_t frame_type = 0x80;
  zcl_cluster_id_t cluster_id       = zigpc_group_get_cluster_id();
  zcl_command_id_t cmd_id           = 0xFE;
  size_t member_count               = 2;
  char *test_unid                   = "02433912E8B28ABD";
  zigpc_group_member_t member_list[2]
    = {{
         .eui64       = "\x02\x43\x39\x12\xE8\xB2\x8A\xBD",
         .endpoint_id = 3,
       },
       {
         .eui64       = "\x02\x43\x39\x12\xE8\xB2\x8A\xBD",
         .endpoint_id = 1,

       }};

  zigpc_groupmgmt_get_member_count_ExpectAndReturn(group_id, member_count);
  zigpc_groupmgmt_get_member_count_ExpectAndReturn(group_id, member_count);

  zigpc_groupmgmt_get_member_list_ExpectAndReturn(group_id,
                                                  NULL,
                                                  member_count,
                                                  SL_STATUS_OK);
  zigpc_groupmgmt_get_member_list_IgnoreArg_member_list();
  zigpc_groupmgmt_get_member_list_ReturnArrayThruPtr_member_list(member_list,
                                                                 member_count);

  for (size_t i = 0; i < member_count; i++) {
    zigbee_eui64_to_str_ExpectAndReturn(NULL,
                                        NULL,
                                        ZIGBEE_EUI64_HEX_STR_LENGTH,
                                        SL_STATUS_OK);
    zigbee_eui64_to_str_IgnoreArg_eui64();
    zigbee_eui64_to_str_IgnoreArg_str_buf();
    zigbee_eui64_to_str_ReturnArrayThruPtr_str_buf(test_unid,
                                                   strlen(test_unid) + 1);

    str_to_zigbee_eui64_ExpectAndReturn(test_unid,
                                        strlen(test_unid),
                                        NULL,
                                        SL_STATUS_OK);
    str_to_zigbee_eui64_IgnoreArg_eui64();
    str_to_zigbee_eui64_ReturnMemThruPtr_eui64(member_list[i].eui64,
                                               sizeof(zigbee_eui64_t));

    zigpc_zcl_build_command_frame_ExpectAndReturn(NULL,
                                                  frame_type,
                                                  cluster_id,
                                                  cmd_id,
                                                  0,
                                                  NULL,
                                                  SL_STATUS_OK);
    zigpc_zcl_build_command_frame_IgnoreArg_frame();

    zigpc_gateway_send_zcl_command_frame_ExpectAndReturn(
      member_list[i].eui64,
      member_list[i].endpoint_id,
      cluster_id,
      NULL,
      SL_STATUS_OK);
    zigpc_gateway_send_zcl_command_frame_IgnoreArg_frame();
  }

  // ACT
  status = zigpc_command_mapper_send_multicast(group_id,
                                               frame_type,
                                               cluster_id,
                                               cmd_id,
                                               0,
                                               NULL);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}
