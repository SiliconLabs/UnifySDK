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

extern "C" {

#include <unity.h>

#include <dotdot_mqtt_mock.h>
#include <zigpc_common_zigbee_mock.h>
#include <zcl_util_mock.h>
#include <zcl_definitions.h>

#include "attribute_management_int.h"

static zigbee_eui64_t TEST_EUI64
  = {0x04, 0x54, 0x02, 0xF2, 0x0A, 0x4C, 0x99, 0x10};
static zigbee_endpoint_id_t TEST_ENDPOINT_ID = 2;
static std::string TEST_EUI64_STR            = "zb-045402F20A4C9910";
static std::string BASE_TOPIC = "ucl/by-unid/" + TEST_EUI64_STR + "/ep2";

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

/**
 * @brief Test helper to expect common EUI64 to unid conversion and data type
 * resolution.
 *
 * @param type
 * @param return_size
 */
void helper_expect_unid_and_type(zigpc_zcl_data_type_t type, size_t return_size)
{
  zigpc_zcl_get_data_type_size_ExpectAndReturn(type, return_size);

  zigpc_common_eui64_to_unid_ExpectAndReturn(TEST_EUI64, NULL, 0, SL_STATUS_OK);
  zigpc_common_eui64_to_unid_IgnoreArg_dest_unid();
  zigpc_common_eui64_to_unid_IgnoreArg_dest_unid_length();
  zigpc_common_eui64_to_unid_ReturnMemThruPtr_dest_unid(
    const_cast<char *>(TEST_EUI64_STR.c_str()),
    TEST_EUI64_STR.size() + 1);
}

void test_receive_attribute_frame_should_sanitize_inputs(void)
{
  // ARRANGE
  sl_status_t status;

  // ACT
  status = zigpc_attrmgmt_receive_attribute_frame(NULL, 1, 0, false, NULL);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);

  // ACT
  status
    = zigpc_attrmgmt_receive_attribute_frame(TEST_EUI64, 1, 0, false, NULL);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_NULL_POINTER, status);
}

void test_receive_attribute_frame_should_process_reporting_frame_onoff(void)
{
  // ARRANGE
  sl_status_t status;
  zcl_frame_t frame;

  zcl_attribute_id_t attr_id      = ZIGPC_ZCL_CLUSTER_ON_OFF_ATTR_ON_OFF;
  zigpc_zcl_data_type_t attr_type = ZIGPC_ZCL_DATA_TYPE_BOOL;
  uint8_t attr_data               = 0x1;

  frame.buffer[0] = attr_id;
  frame.buffer[1] = attr_id >> 8;
  frame.buffer[2] = attr_type;
  frame.buffer[3] = attr_data;
  frame.size      = 4;

  helper_expect_unid_and_type(attr_type, 1);

  uic_mqtt_dotdot_on_off_on_off_publish_ExpectAndReturn(
    BASE_TOPIC.c_str(),
    attr_data,
    UCL_MQTT_PUBLISH_TYPE_REPORTED,
    SL_STATUS_OK);

  // ACT
  status = zigpc_attrmgmt_receive_attribute_frame(TEST_EUI64,
                                                  TEST_ENDPOINT_ID,
                                                  ZIGPC_ZCL_CLUSTER_ON_OFF,
                                                  false,
                                                  &frame);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_receive_attribute_frame_should_process_read_resp_frame_onoff(void)
{
  // ARRANGE
  sl_status_t status;
  zcl_frame_t frame;

  zcl_attribute_id_t attr_id      = ZIGPC_ZCL_CLUSTER_ON_OFF_ATTR_ON_OFF;
  zigpc_zcl_data_type_t attr_type = ZIGPC_ZCL_DATA_TYPE_BOOL;
  uint8_t attr_data               = 0x1;

  frame.buffer[0] = attr_id;
  frame.buffer[1] = attr_id >> 8;
  frame.buffer[2] = ZIGPC_ZCL_STATUS_SUCCESS;
  frame.buffer[3] = attr_type;
  frame.buffer[4] = attr_data;
  frame.size      = 5;

  helper_expect_unid_and_type(attr_type, 1);

  uic_mqtt_dotdot_on_off_on_off_publish_ExpectAndReturn(
    BASE_TOPIC.c_str(),
    attr_data,
    UCL_MQTT_PUBLISH_TYPE_REPORTED,
    SL_STATUS_OK);

  // ACT
  status = zigpc_attrmgmt_receive_attribute_frame(TEST_EUI64,
                                                  TEST_ENDPOINT_ID,
                                                  ZIGPC_ZCL_CLUSTER_ON_OFF,
                                                  true,
                                                  &frame);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void test_receive_attribute_frame_should_skip_failed_read_resp_records_onoff(
  void)
{
  // ARRANGE
  sl_status_t status;
  zcl_frame_t frame;

  zcl_attribute_id_t attr_id_1      = ZIGPC_ZCL_CLUSTER_ON_OFF_ATTR_ON_OFF;
  zcl_attribute_id_t attr_id_2      = ZIGPC_ZCL_CLUSTER_ON_OFF_ATTR_ON_TIME;
  zigpc_zcl_data_type_t attr_type_2 = ZIGPC_ZCL_DATA_TYPE_UINT16;
  uint16_t attr_data_2              = 0x1234;

  frame.buffer[0] = attr_id_1;
  frame.buffer[1] = attr_id_1 >> 8;
  frame.buffer[2] = ZIGPC_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;

  frame.buffer[3] = attr_id_2;
  frame.buffer[4] = attr_id_2 >> 8;
  frame.buffer[5] = ZIGPC_ZCL_STATUS_SUCCESS;
  frame.buffer[6] = attr_type_2;
  frame.buffer[7] = attr_data_2;
  frame.buffer[8] = attr_data_2 >> 8;
  frame.size      = 9;

  helper_expect_unid_and_type(attr_type_2, 2);

  uic_mqtt_dotdot_on_off_on_time_publish_ExpectAndReturn(
    BASE_TOPIC.c_str(),
    attr_data_2,
    UCL_MQTT_PUBLISH_TYPE_REPORTED,
    SL_STATUS_OK);

  // ACT
  status = zigpc_attrmgmt_receive_attribute_frame(TEST_EUI64,
                                                  TEST_ENDPOINT_ID,
                                                  ZIGPC_ZCL_CLUSTER_ON_OFF,
                                                  true,
                                                  &frame);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

}  // extern "C"
