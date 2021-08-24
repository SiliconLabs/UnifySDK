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

#include "workaround.hpp"
#include "zigpc_attrmgmt_int.hpp"

extern "C" {

#include "unity.h"

#include "zcl_attribute_info_mock.h"
#include "zigpc_common_zigbee_mock.h"
#include "zcl_profiles_mock.h"
#include "zcl_util_mock.h"
#include "zcl_definitions.h"
#include "zigpc_gateway_mock.h"

#include "attribute_management_int.h"

static const size_t TEST_SEND_LIMIT
  = ZIGPC_ATTRMGMT_READ_RECORDS_LIMIT_PER_MESSAGE;

static zigbee_eui64_t TEST_EUI64
  = {0x04, 0x54, 0x02, 0xF2, 0x0A, 0x4C, 0x99, 0x10};
static zigbee_endpoint_id_t TEST_ENDPOINT_ID = 2;
static zcl_cluster_id_t TEST_CLUSTER_ID      = 0x2334;

/**
 * @brief Setup the test suite (called once before all test_xxx functions are called)
 *
 */
void suiteSetUp(void) {}

/**
 * @brief Teardown the test suite (called once after all test_xxx functions are called)
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
  zcl_util_mock_Init();
}

/**
 * @brief Teardown after each test case
 *
 */
void tearDown(void) {}

void test_reads_split_should_sanitize_inputs(void)
{
  // ARRANGE
  sl_status_t status;
  std::vector<zcl_attribute_id_t> attr_ids;

  // ACT
  status = zigpc_attrmgmt_send_split_read_cmds(NULL, 1, 0, attr_ids);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
}

void helper_expect_zcl_frame_calls(const zigbee_eui64_t eui64,
                                   zigbee_endpoint_id_t endpoint_id,
                                   zcl_cluster_id_t cluster_id,
                                   size_t expected_sets,
                                   std::vector<zcl_attribute_id_t> attr_ids)
{
  size_t attr_id_left = attr_ids.size();
  // Expect the number of times called to init + second ZCL frame as the
  // Expected number of messages to be constructed and sent per set
  for (size_t i = 0U; i < expected_sets; i++) {
    size_t attr_ids_to_send
      = (attr_id_left > TEST_SEND_LIMIT) ? TEST_SEND_LIMIT : attr_id_left;
    attr_id_left -= TEST_SEND_LIMIT;

    zigpc_zcl_build_command_frame_ExpectAndReturn(
      nullptr,
      ZIGPC_ZCL_FRAME_TYPE_GLOBAL_CMD_TO_SERVER,
      cluster_id,
      ZIGPC_ZCL_GLOBAL_COMMAND_READ_ATTRIBUTES,
      attr_ids_to_send,
      nullptr,
      SL_STATUS_OK);
    zigpc_zcl_build_command_frame_IgnoreArg_frame();
    zigpc_zcl_build_command_frame_IgnoreArg_command_arg_list();

    zigpc_gateway_send_zcl_command_frame_ExpectAndReturn(eui64,
                                                         endpoint_id,
                                                         cluster_id,
                                                         nullptr,
                                                         SL_STATUS_OK);
    zigpc_gateway_send_zcl_command_frame_IgnoreArg_frame();
  }
}

void test_reads_split_with_one_record(void)
{
  // ARRANGE
  sl_status_t status;
  std::vector<zcl_attribute_id_t> attr_ids = {0x123F};

  helper_expect_zcl_frame_calls(TEST_EUI64,
                                TEST_ENDPOINT_ID,
                                TEST_CLUSTER_ID,
                                1,
                                attr_ids);

  // ACT
  status = zigpc_attrmgmt_send_split_read_cmds(TEST_EUI64,
                                               TEST_ENDPOINT_ID,
                                               TEST_CLUSTER_ID,
                                               attr_ids);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_reads_split_with_record_count_less_than_send_limit(void)
{
  // ARRANGE
  sl_status_t status;
  size_t attr_count = TEST_SEND_LIMIT - 1;
  std::vector<zcl_attribute_id_t> attr_ids;

  for (size_t i = 0; i < attr_count; i++) {
    attr_ids.push_back(0xFFFF - i);
  }

  helper_expect_zcl_frame_calls(TEST_EUI64,
                                TEST_ENDPOINT_ID,
                                TEST_CLUSTER_ID,
                                1,
                                attr_ids);

  // ACT
  status = zigpc_attrmgmt_send_split_read_cmds(TEST_EUI64,
                                               TEST_ENDPOINT_ID,
                                               TEST_CLUSTER_ID,
                                               attr_ids);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_reads_split_with_record_count_equal_to_send_limit(void)
{
  // ARRANGE
  sl_status_t status;
  size_t attr_count = TEST_SEND_LIMIT;
  std::vector<zcl_attribute_id_t> attr_ids;

  for (size_t i = 0; i < attr_count; i++) {
    attr_ids.push_back(0xFFFF - i);
  }

  helper_expect_zcl_frame_calls(TEST_EUI64,
                                TEST_ENDPOINT_ID,
                                TEST_CLUSTER_ID,
                                1,
                                attr_ids);

  // ACT
  status = zigpc_attrmgmt_send_split_read_cmds(TEST_EUI64,
                                               TEST_ENDPOINT_ID,
                                               TEST_CLUSTER_ID,
                                               attr_ids);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_reads_split_with_record_count_greater_than_send_limit(void)
{
  // ARRANGE
  sl_status_t status;
  size_t attr_count = TEST_SEND_LIMIT + 1;
  std::vector<zcl_attribute_id_t> attr_ids;

  for (size_t i = 0; i < attr_count; i++) {
    attr_ids.push_back(0xFFFF - i);
  }

  helper_expect_zcl_frame_calls(TEST_EUI64,
                                TEST_ENDPOINT_ID,
                                TEST_CLUSTER_ID,
                                2,
                                attr_ids);

  // ACT
  status = zigpc_attrmgmt_send_split_read_cmds(TEST_EUI64,
                                               TEST_ENDPOINT_ID,
                                               TEST_CLUSTER_ID,
                                               attr_ids);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_reads_split_with_large_record_count(void)
{
  // ARRANGE
  sl_status_t status;
  size_t expected_messages = 4;
  size_t attr_count        = (expected_messages * TEST_SEND_LIMIT) + 7;
  std::vector<zcl_attribute_id_t> attr_ids;

  for (size_t i = 0; i < attr_count; i++) {
    attr_ids.push_back(0xFFFF - i);
  }

  helper_expect_zcl_frame_calls(TEST_EUI64,
                                TEST_ENDPOINT_ID,
                                TEST_CLUSTER_ID,
                                expected_messages + 1,
                                attr_ids);

  // ACT
  status = zigpc_attrmgmt_send_split_read_cmds(TEST_EUI64,
                                               TEST_ENDPOINT_ID,
                                               TEST_CLUSTER_ID,
                                               attr_ids);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_defer_read_attr_should_handle_invalid_inputs(void)
{
  // ARRANGE
  sl_status_t status;

  // ACT
  status = zigpc_attrmgmt_send_read_attributes_command(NULL, 0, 0);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
}

void test_endpoint_read_one_cluster_sanity(void)
{
  // ARRANGE
  sl_status_t status;
  size_t attr_count = 21U;
  std::vector<zcl_attribute_id_t> attr_ids;
  std::vector<zcl_attribute_t> attr_info_list;

  for (size_t i = 0; i < attr_count; i++) {
    attr_ids.push_back(0xFFFF - i);
    attr_info_list.push_back({0, attr_ids[i]});
  }

  // Expect functions called in zigpc_attrmgmt_send_split_read_cmds()
  helper_expect_zcl_frame_calls(TEST_EUI64,
                                TEST_ENDPOINT_ID,
                                TEST_CLUSTER_ID,
                                1,
                                attr_ids);

  zigpc_zclprofiles_get_cluster_attribute_count_ExpectAndReturn(TEST_CLUSTER_ID,
                                                                attr_count);

  zigpc_zclprofiles_get_cluster_attribute_list_ExpectAndReturn(TEST_CLUSTER_ID,
                                                               NULL,
                                                               SL_STATUS_OK);
  zigpc_zclprofiles_get_cluster_attribute_list_IgnoreArg_attribute_list();
  zigpc_zclprofiles_get_cluster_attribute_list_ReturnArrayThruPtr_attribute_list(
    attr_info_list.data(),
    attr_info_list.size());

  // ACT
  status = zigpc_attrmgmt_send_read_attributes_command(TEST_EUI64,
                                                       TEST_ENDPOINT_ID,
                                                       TEST_CLUSTER_ID);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

}  // extern "C"
