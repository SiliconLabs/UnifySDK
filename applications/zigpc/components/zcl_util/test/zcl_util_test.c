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
#include "sl_status.h"

#include "zigpc_common_zigbee.h"
#include "zcl_util.h"

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
void setUp(void) {}

/**
 * @brief Teardown after each test case
 *
 */
void tearDown(void) {}

void test_zcl_frame_init_command_input_sanity(void)
{
  // ARRANGE
  sl_status_t status_null_pointer;
  sl_status_t status_ok;
  zcl_frame_t frame;
  // ACT
  status_null_pointer = zigpc_zcl_frame_init_command(NULL, 0, 0);
  status_ok           = zigpc_zcl_frame_init_command(&frame, 10, 0xFF);
  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_NULL_POINTER, status_null_pointer);
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status_ok);
  TEST_ASSERT_EQUAL(3, frame.size);
}

void test_zcl_frame_fill_data_array_sanity(void)
{
  // ARRANGE
  sl_status_t status;
  zcl_frame_t frame = {.size = 0};
  uint8_t test_data;

  // ACT
  status = zigpc_zcl_frame_fill_data_array(NULL,
                                           ZIGPC_ZCL_DATA_TYPE_NODATA,
                                           0,
                                           NULL);
  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_NULL_POINTER, status);
  TEST_ASSERT_EQUAL(0, frame.size);

  // ACT
  status = zigpc_zcl_frame_fill_data_array(&frame,
                                           ZIGPC_ZCL_DATA_TYPE_NODATA,
                                           0,
                                           NULL);
  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_NULL_POINTER, status);
  TEST_ASSERT_EQUAL(0, frame.size);

  // ACT
  status = zigpc_zcl_frame_fill_data_array(&frame,
                                           ZIGPC_ZCL_DATA_TYPE_UINT8,
                                           0,
                                           &test_data);
  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_INVALID_COUNT, status);
  TEST_ASSERT_EQUAL(0, frame.size);
}

void test_zcl_frame_fill_data_array_nodata_should_not_modify_frame(void)
{
  // ARRANGE
  sl_status_t status;
  zcl_frame_t frame = {.size = 0};
  uint8_t test_data;

  // ACT
  status = zigpc_zcl_frame_fill_data_array(&frame,
                                           ZIGPC_ZCL_DATA_TYPE_NODATA,
                                           1,
                                           &test_data);
  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_INVALID_TYPE, status);
  TEST_ASSERT_EQUAL(0, frame.size);
}

void test_zcl_frame_fill_data_array_uint8_data(void)
{
  // ARRANGE
  sl_status_t status;
  zcl_frame_t frame = {.size = 0};
  uint8_t test_data = 0xF9;

  // ACT
  status = zigpc_zcl_frame_fill_data_array(&frame,
                                           ZIGPC_ZCL_DATA_TYPE_UINT8,
                                           1,
                                           &test_data);
  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(1, frame.size);
  TEST_ASSERT_EQUAL_HEX8(test_data, frame.buffer[0]);
}

void test_zcl_frame_fill_data_array_multiple_uint8_data(void)
{
  // ARRANGE
  sl_status_t status;
  zcl_frame_t frame    = {.size = 0};
  uint8_t test_data[]  = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC};
  size_t test_data_len = 6;

  // ACT
  status = zigpc_zcl_frame_fill_data_array(&frame,
                                           ZIGPC_ZCL_DATA_TYPE_UINT8,
                                           test_data_len,
                                           test_data);
  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(6, frame.size);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(test_data, frame.buffer, test_data_len);
}

void test_zcl_build_command_frame_sanity(void)
{
  // ARRANGE
  sl_status_t status;
  zcl_frame_t frame;
  zcl_cluster_id_t cluster_id = 0x0A2F;
  zcl_command_id_t command_id = 0x47;
  size_t arg_count            = 0;

  // ACT
  status = zigpc_zcl_build_command_frame(&frame,
                                         ZIGPC_ZCL_FRAME_TYPE_CMD_TO_SERVER,
                                         cluster_id,
                                         command_id,
                                         arg_count,
                                         NULL);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(3, frame.size);
}

void test_zcl_build_command_frame_sanity_multiple_args(void)
{
  // ARRANGE
  sl_status_t status;
  zcl_frame_t frame;
  zcl_cluster_id_t cluster_id = 0x0A2F;
  zcl_command_id_t command_id = 0x47;
  size_t arg_count            = 3;
  uint8_t arg1_data           = 0x7F;
  uint16_t arg2_data          = 0xFABC;
  char *arg3_data             = "Hello";
  size_t arg3_len             = strlen(arg3_data);

  zigpc_zcl_frame_data_t arg_list[] = {
    {.type = ZIGPC_ZCL_DATA_TYPE_UINT8, .data = &arg1_data},
    {.type = ZIGPC_ZCL_DATA_TYPE_UINT16, .data = &arg2_data},
    {.type = ZIGPC_ZCL_DATA_TYPE_STRING, .data = arg3_data},
  };

  // ACT
  status = zigpc_zcl_build_command_frame(&frame,
                                         ZIGPC_ZCL_FRAME_TYPE_CMD_TO_SERVER,
                                         cluster_id,
                                         command_id,
                                         arg_count,
                                         arg_list);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
  // 3 header + 1 (uint8) + 2 (uint16) + 6 (length + 5 chars)
  TEST_ASSERT_EQUAL(12, frame.size);

  TEST_ASSERT_EQUAL_HEX8(arg1_data, frame.buffer[3]);

  // Stored in little endian
  uint16_t frame_arg2 = (frame.buffer[5] << 8) | frame.buffer[4];
  TEST_ASSERT_EQUAL_HEX16(arg2_data, frame_arg2);

  TEST_ASSERT_EQUAL_HEX8(arg3_len, frame.buffer[6]);
  TEST_ASSERT_EQUAL_STRING_LEN(arg3_data, &frame.buffer[7], arg3_len);
}
