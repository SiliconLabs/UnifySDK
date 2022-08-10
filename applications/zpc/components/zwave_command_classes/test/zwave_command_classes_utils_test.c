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
#include "zwave_command_classes_utils.h"

// Interface includes
#include "ZW_classcmd.h"
#include "attribute_store_defined_attribute_types.h"

// Mock includes
#include "zwave_utils_mock.h"
#include "zpc_attribute_store_network_helper_mock.h"
#include "attribute_store_mock.h"
#include "attribute_store_helper_mock.h"
#include "zwave_unid_mock.h"

// Test variables
static zwave_endpoint_id_t endpoint_id = 0;
static zwave_node_id_t node_id         = 0;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_zwave_command_classes_utils_time_to_duration()
{
  TEST_ASSERT_EQUAL(0, time_to_zwave_duration(0));
  TEST_ASSERT_EQUAL(1, time_to_zwave_duration(CLOCK_SECOND));
  TEST_ASSERT_EQUAL(10, time_to_zwave_duration(10000));
  TEST_ASSERT_EQUAL(126, time_to_zwave_duration(126000));
  TEST_ASSERT_EQUAL(127, time_to_zwave_duration(127000));
  TEST_ASSERT_EQUAL(127 + 2, time_to_zwave_duration(127001));
  TEST_ASSERT_EQUAL(127 + 3, time_to_zwave_duration(3 * 60 * CLOCK_SECOND));
  TEST_ASSERT_EQUAL(127 + 126, time_to_zwave_duration(126 * 60 * CLOCK_SECOND));
  TEST_ASSERT_EQUAL(0xFD, time_to_zwave_duration(126 * 60 * CLOCK_SECOND + 1));
}

void test_zwave_command_classes_utils_duration_to_time()
{
  TEST_ASSERT_EQUAL(0, zwave_duration_to_time(0));
  TEST_ASSERT_EQUAL(CLOCK_SECOND, zwave_duration_to_time(1));
  TEST_ASSERT_EQUAL(10 * CLOCK_SECOND, zwave_duration_to_time(10));
  TEST_ASSERT_EQUAL(100 * CLOCK_SECOND, zwave_duration_to_time(100));
  TEST_ASSERT_EQUAL(127 * CLOCK_SECOND, zwave_duration_to_time(127));
  TEST_ASSERT_EQUAL(60 * CLOCK_SECOND, zwave_duration_to_time(128));
  TEST_ASSERT_EQUAL(2 * 60 * CLOCK_SECOND, zwave_duration_to_time(127 + 2));
  TEST_ASSERT_EQUAL(126 * 60 * CLOCK_SECOND, zwave_duration_to_time(127 + 126));
  TEST_ASSERT_EQUAL(0, zwave_duration_to_time(127 + 127));
}

void test_zwave_command_classes_utils_is_actuator_command_class()
{
  TEST_ASSERT_TRUE(is_actuator_command_class(COMMAND_CLASS_BARRIER_OPERATOR));
  TEST_ASSERT_TRUE(is_actuator_command_class(COMMAND_CLASS_BASIC));
  TEST_ASSERT_TRUE(is_actuator_command_class(COMMAND_CLASS_SWITCH_BINARY));
  TEST_ASSERT_TRUE(is_actuator_command_class(COMMAND_CLASS_SWITCH_COLOR));
  TEST_ASSERT_TRUE(is_actuator_command_class(COMMAND_CLASS_DOOR_LOCK));
  TEST_ASSERT_TRUE(is_actuator_command_class(COMMAND_CLASS_SWITCH_MULTILEVEL));
  TEST_ASSERT_TRUE(is_actuator_command_class(COMMAND_CLASS_SIMPLE_AV_CONTROL));
  TEST_ASSERT_TRUE(is_actuator_command_class(COMMAND_CLASS_SOUND_SWITCH));
  TEST_ASSERT_TRUE(
    is_actuator_command_class(COMMAND_CLASS_THERMOSTAT_SETPOINT));
  TEST_ASSERT_TRUE(is_actuator_command_class(COMMAND_CLASS_THERMOSTAT_MODE));
  TEST_ASSERT_TRUE(is_actuator_command_class(COMMAND_CLASS_WINDOW_COVERING));

  TEST_ASSERT_FALSE(is_actuator_command_class(0x01));
  TEST_ASSERT_FALSE(is_actuator_command_class(COMMAND_CLASS_SECURITY_2));
  TEST_ASSERT_FALSE(is_actuator_command_class(COMMAND_CLASS_SECURITY));
  TEST_ASSERT_FALSE(is_actuator_command_class(COMMAND_CLASS_NOTIFICATION_V3));
  TEST_ASSERT_FALSE(is_actuator_command_class(COMMAND_CLASS_USER_CODE));
  TEST_ASSERT_FALSE(is_actuator_command_class(COMMAND_CLASS_MULTI_CHANNEL_V4));
  TEST_ASSERT_FALSE(is_actuator_command_class(COMMAND_CLASS_ASSOCIATION));
  TEST_ASSERT_FALSE(
    is_actuator_command_class(COMMAND_CLASS_ASSOCIATION_GRP_INFO));
  TEST_ASSERT_FALSE(is_actuator_command_class(COMMAND_CLASS_CENTRAL_SCENE));
  TEST_ASSERT_FALSE(is_actuator_command_class(COMMAND_CLASS_CLOCK));
  TEST_ASSERT_FALSE(is_actuator_command_class(COMMAND_CLASS_WAKE_UP));
  TEST_ASSERT_FALSE(is_actuator_command_class(COMMAND_CLASS_SENSOR_MULTILEVEL));
}

void test_is_zwave_command_class_filtered_for_root_device_ep1()
{
  attribute_store_node_t updated_node = 1298;
  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    updated_node,
    NULL,
    NULL,
    SL_STATUS_OK);

  endpoint_id = 1;
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
    &endpoint_id);

  TEST_ASSERT_EQUAL(false,
                    is_zwave_command_class_filtered_for_root_device(
                      COMMAND_CLASS_SWITCH_BINARY_V2,
                      updated_node));
}

void test_is_zwave_command_class_filtered_for_root_device_ep0_no_multi_channel()
{
  attribute_store_node_t updated_node = 1298;
  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    updated_node,
    NULL,
    NULL,
    SL_STATUS_OK);

  endpoint_id = 0;
  node_id     = 5;
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
    &endpoint_id);
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_node_id(
    &node_id);

  zwave_node_supports_command_class_ExpectAndReturn(
    COMMAND_CLASS_MULTI_CHANNEL_V4,
    node_id,
    endpoint_id,
    false);

  TEST_ASSERT_EQUAL(false,
                    is_zwave_command_class_filtered_for_root_device(
                      COMMAND_CLASS_SIMPLE_AV_CONTROL,
                      updated_node));
}

void test_is_zwave_command_class_filtered_for_root_device_ep0_with_multi_channel()
{
  attribute_store_node_t updated_node = 1298;
  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    updated_node,
    NULL,
    NULL,
    SL_STATUS_OK);

  endpoint_id = 0;
  node_id     = 6;
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
    &endpoint_id);
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_node_id(
    &node_id);

  zwave_node_supports_command_class_ExpectAndReturn(
    COMMAND_CLASS_MULTI_CHANNEL_V4,
    node_id,
    endpoint_id,
    true);

  attribute_store_node_t test_node = 98;
  attribute_store_get_first_parent_with_type_ExpectAndReturn(updated_node,
                                                             ATTRIBUTE_NODE_ID,
                                                             test_node);
  attribute_store_node_t attribute_cc_version_found = 198;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node,
    ATTRIBUTE_CC_VERSION_FOUND,
    0,
    attribute_cc_version_found);

  zwave_node_get_command_class_version_ExpectAndReturn(
    COMMAND_CLASS_MULTI_CHANNEL_V4,
    node_id,
    endpoint_id,
    6);

  TEST_ASSERT_EQUAL(true,
                    is_zwave_command_class_filtered_for_root_device(
                      COMMAND_CLASS_THERMOSTAT_MODE,
                      updated_node));
}

void test_is_zwave_command_class_filtered_for_root_device_ep0_version_too_low()
{
  attribute_store_node_t updated_node = 1298;
  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    updated_node,
    NULL,
    NULL,
    SL_STATUS_OK);

  endpoint_id = 0;
  node_id     = 7;
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
    &endpoint_id);
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_node_id(
    &node_id);

  zwave_node_supports_command_class_ExpectAndReturn(
    COMMAND_CLASS_MULTI_CHANNEL_V4,
    node_id,
    endpoint_id,
    true);

  attribute_store_node_t test_node = 99;
  attribute_store_get_first_parent_with_type_ExpectAndReturn(updated_node,
                                                             ATTRIBUTE_NODE_ID,
                                                             test_node);
  attribute_store_node_t attribute_cc_version_found = 199;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node,
    ATTRIBUTE_CC_VERSION_FOUND,
    0,
    attribute_cc_version_found);

  zwave_node_get_command_class_version_ExpectAndReturn(
    COMMAND_CLASS_MULTI_CHANNEL_V4,
    node_id,
    endpoint_id,
    2);

  TEST_ASSERT_EQUAL(false,
                    is_zwave_command_class_filtered_for_root_device(
                      COMMAND_CLASS_WINDOW_COVERING,
                      updated_node));
}

void test_is_zwave_command_class_filtered_for_root_device_ep0_with_multi_channel_version_unknown()
{
  attribute_store_node_t updated_node = 1298;
  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    updated_node,
    NULL,
    NULL,
    SL_STATUS_OK);

  endpoint_id = 0;
  node_id     = 8;
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
    &endpoint_id);
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_node_id(
    &node_id);

  zwave_node_supports_command_class_ExpectAndReturn(
    COMMAND_CLASS_MULTI_CHANNEL_V4,
    node_id,
    endpoint_id,
    true);

  attribute_store_node_t test_node = 100;
  attribute_store_get_first_parent_with_type_ExpectAndReturn(updated_node,
                                                             ATTRIBUTE_NODE_ID,
                                                             test_node);
  attribute_store_node_t attribute_cc_version_found
    = ATTRIBUTE_STORE_INVALID_NODE;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node,
    ATTRIBUTE_CC_VERSION_FOUND,
    0,
    attribute_cc_version_found);

  TEST_ASSERT_EQUAL(true,
                    is_zwave_command_class_filtered_for_root_device(
                      COMMAND_CLASS_BARRIER_OPERATOR,
                      updated_node));
}

void test_is_zwave_command_class_filtered_for_root_device_non_actuator()
{
  attribute_store_node_t updated_node = 9;

  TEST_ASSERT_EQUAL(
    false,
    is_zwave_command_class_filtered_for_root_device(COMMAND_CLASS_SECURITY_2,
                                                    updated_node));
}

void test_get_zwave_node_role_type()
{
  zwave_node_id_t test_node_id          = 0x01;
  unid_t test_unid                      = "zw000001";
  zwave_role_type_t role_type           = 0x01;
  attribute_store_node_t node_endpoint  = 0x88;
  attribute_store_node_t role_type_node = 0x98;
  zwave_unid_from_node_id_Expect(test_node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));
  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    0,
    node_endpoint);
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    node_endpoint,
    ATTRIBUTE_ZWAVE_ROLE_TYPE,
    0,
    role_type_node);
  attribute_store_get_reported_ExpectAndReturn(role_type_node,
                                               NULL,
                                               sizeof(zwave_role_type_t),
                                               SL_STATUS_OK);
  attribute_store_get_reported_IgnoreArg_value();
  attribute_store_get_reported_ReturnThruPtr_value(&role_type);
  TEST_ASSERT_EQUAL(role_type, get_zwave_node_role_type(test_node_id));
}

void test_is_portable_end_node()
{
  zwave_node_id_t test_node_id          = 0x01;
  unid_t test_unid                      = "zw000001";
  zwave_role_type_t role_type           = ROLE_TYPE_PEN;
  attribute_store_node_t node_id_node   = 0x78;
  attribute_store_node_t node_endpoint  = 0x88;
  attribute_store_node_t role_type_node = 0x98;
  attribute_store_get_first_parent_with_type_ExpectAndReturn(node_id_node,
                                                             ATTRIBUTE_NODE_ID,
                                                             node_id_node);
  attribute_store_get_reported_ExpectAndReturn(node_id_node,
                                               NULL,
                                               sizeof(zwave_node_id_t),
                                               SL_STATUS_OK);
  attribute_store_get_reported_IgnoreArg_value();
  attribute_store_get_reported_ReturnThruPtr_value(&test_node_id);

  zwave_unid_from_node_id_Expect(test_node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));
  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    0,
    node_endpoint);
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    node_endpoint,
    ATTRIBUTE_ZWAVE_ROLE_TYPE,
    0,
    role_type_node);
  attribute_store_get_reported_ExpectAndReturn(role_type_node,
                                               NULL,
                                               sizeof(zwave_role_type_t),
                                               SL_STATUS_OK);
  attribute_store_get_reported_IgnoreArg_value();
  attribute_store_get_reported_ReturnThruPtr_value(&role_type);
  TEST_ASSERT_TRUE(is_portable_end_node(node_id_node));

  // Not a PEN
  role_type = ROLE_TYPE_CSC;
  attribute_store_get_first_parent_with_type_ExpectAndReturn(node_id_node,
                                                             ATTRIBUTE_NODE_ID,
                                                             node_id_node);
  attribute_store_get_reported_ExpectAndReturn(node_id_node,
                                               NULL,
                                               sizeof(zwave_node_id_t),
                                               SL_STATUS_OK);
  attribute_store_get_reported_IgnoreArg_value();
  attribute_store_get_reported_ReturnThruPtr_value(&test_node_id);
  zwave_unid_from_node_id_Expect(test_node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));
  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    0,
    node_endpoint);
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    node_endpoint,
    ATTRIBUTE_ZWAVE_ROLE_TYPE,
    0,
    role_type_node);
  attribute_store_get_reported_ExpectAndReturn(role_type_node,
                                               NULL,
                                               sizeof(zwave_role_type_t),
                                               SL_STATUS_OK);
  attribute_store_get_reported_IgnoreArg_value();
  attribute_store_get_reported_ReturnThruPtr_value(&role_type);
  TEST_ASSERT_FALSE(is_portable_end_node(node_id_node));
}

void test_signed_value_extraction()
{
  int32_t test_signed_value_4_bytes = 12345677;
  uint8_t frame[4]                  = {};
  // Pack in the frame, MSB first
  frame[0] = (uint8_t)((test_signed_value_4_bytes >> 24) & 0xFF);
  frame[1] = (uint8_t)((test_signed_value_4_bytes >> 16) & 0xFF);
  frame[2] = (uint8_t)((test_signed_value_4_bytes >> 8) & 0xFF);
  frame[3] = (uint8_t)((test_signed_value_4_bytes >> 0) & 0xFF);

  int32_t extracted_value = get_signed_value_from_frame_and_size(frame, 4);
  TEST_ASSERT_EQUAL_INT32(test_signed_value_4_bytes, extracted_value);

  test_signed_value_4_bytes = -1;
  frame[0] = (uint8_t)((test_signed_value_4_bytes >> 24) & 0xFF);
  frame[1] = (uint8_t)((test_signed_value_4_bytes >> 16) & 0xFF);
  frame[2] = (uint8_t)((test_signed_value_4_bytes >> 8) & 0xFF);
  frame[3] = (uint8_t)((test_signed_value_4_bytes >> 0) & 0xFF);

  extracted_value = get_signed_value_from_frame_and_size(frame, 4);
  TEST_ASSERT_EQUAL_INT32(test_signed_value_4_bytes, extracted_value);

  test_signed_value_4_bytes = INT32_MAX;
  frame[0] = (uint8_t)((test_signed_value_4_bytes >> 24) & 0xFF);
  frame[1] = (uint8_t)((test_signed_value_4_bytes >> 16) & 0xFF);
  frame[2] = (uint8_t)((test_signed_value_4_bytes >> 8) & 0xFF);
  frame[3] = (uint8_t)((test_signed_value_4_bytes >> 0) & 0xFF);

  extracted_value = get_signed_value_from_frame_and_size(frame, 4);
  TEST_ASSERT_EQUAL_INT32(test_signed_value_4_bytes, extracted_value);

  test_signed_value_4_bytes = INT32_MIN;
  frame[0] = (uint8_t)((test_signed_value_4_bytes >> 24) & 0xFF);
  frame[1] = (uint8_t)((test_signed_value_4_bytes >> 16) & 0xFF);
  frame[2] = (uint8_t)((test_signed_value_4_bytes >> 8) & 0xFF);
  frame[3] = (uint8_t)((test_signed_value_4_bytes >> 0) & 0xFF);

  extracted_value = get_signed_value_from_frame_and_size(frame, 4);
  TEST_ASSERT_EQUAL_INT32(test_signed_value_4_bytes, extracted_value);

  // Test with 0xfffe = -2, size 2 bytes.
  frame[0] = 0xff;
  frame[1] = 0xfe;

  extracted_value = get_signed_value_from_frame_and_size(frame, 2);
  TEST_ASSERT_EQUAL_INT32(-2, extracted_value);

  // Test with 0xfed4 = -300, size 2 bytes.
  frame[0] = 0xfe;
  frame[1] = 0xd4;

  extracted_value = get_signed_value_from_frame_and_size(frame, 2);
  TEST_ASSERT_EQUAL_INT32(-300, extracted_value);

  //  Try with 1 byte signed value:
  int8_t test_signed_value_1_byte = -37;
  // Pack in the frame, MSB first
  frame[0] = (uint8_t)test_signed_value_1_byte;

  extracted_value = get_signed_value_from_frame_and_size(frame, 1);
  TEST_ASSERT_EQUAL_INT32(test_signed_value_1_byte, extracted_value);

  // Value 37
  test_signed_value_1_byte = 37;
  // Pack in the frame, MSB first
  frame[0] = (uint8_t)test_signed_value_1_byte;

  extracted_value = get_signed_value_from_frame_and_size(frame, 1);
  TEST_ASSERT_EQUAL_INT32(test_signed_value_1_byte, extracted_value);

  // Value 127
  test_signed_value_1_byte = 127;
  // Pack in the frame, MSB first
  frame[0] = (uint8_t)test_signed_value_1_byte;

  extracted_value = get_signed_value_from_frame_and_size(frame, 1);
  TEST_ASSERT_EQUAL_INT32(test_signed_value_1_byte, extracted_value);

  // Value -127
  test_signed_value_1_byte = -127;
  // Pack in the frame, MSB first
  frame[0] = (uint8_t)test_signed_value_1_byte;

  extracted_value = get_signed_value_from_frame_and_size(frame, 1);
  TEST_ASSERT_EQUAL_INT32(test_signed_value_1_byte, extracted_value);

  // Try with 2 bytes signed value:
  int16_t test_signed_value_2_bytes = -15000;
  // Pack in the frame, MSB first
  frame[0] = (uint8_t)((test_signed_value_2_bytes >> 8) & 0xFF);
  frame[1] = (uint8_t)((test_signed_value_2_bytes >> 0) & 0xFF);

  extracted_value = get_signed_value_from_frame_and_size(frame, 2);
  TEST_ASSERT_EQUAL_INT32(test_signed_value_2_bytes, extracted_value);

  // Try with 2 bytes signed value:
  test_signed_value_2_bytes = 15000;
  // Pack in the frame, MSB first
  frame[0] = (uint8_t)((test_signed_value_2_bytes >> 8) & 0xFF);
  frame[1] = (uint8_t)((test_signed_value_2_bytes >> 0) & 0xFF);

  extracted_value = get_signed_value_from_frame_and_size(frame, 2);
  TEST_ASSERT_EQUAL_INT32(test_signed_value_2_bytes, extracted_value);

  // Try with 2 bytes signed value:
  test_signed_value_2_bytes = INT16_MAX;
  // Pack in the frame, MSB first
  frame[0] = (uint8_t)((test_signed_value_2_bytes >> 8) & 0xFF);
  frame[1] = (uint8_t)((test_signed_value_2_bytes >> 0) & 0xFF);

  extracted_value = get_signed_value_from_frame_and_size(frame, 2);
  TEST_ASSERT_EQUAL_INT32(test_signed_value_2_bytes, extracted_value);

  // Try with 2 bytes signed value:
  test_signed_value_2_bytes = INT16_MIN;
  // Pack in the frame, MSB first
  frame[0] = (uint8_t)((test_signed_value_2_bytes >> 8) & 0xFF);
  frame[1] = (uint8_t)((test_signed_value_2_bytes >> 0) & 0xFF);

  extracted_value = get_signed_value_from_frame_and_size(frame, 2);
  TEST_ASSERT_EQUAL_INT32(test_signed_value_2_bytes, extracted_value);
}

void test_unsigned_value_extraction()
{
  const uint32_t test_unsigned_value = 0x1265324;
  uint8_t frame[4]                   = {};
  // Pack in the frame, MSB first
  frame[0] = (uint8_t)((test_unsigned_value >> 24) & 0xFF);
  frame[1] = (uint8_t)((test_unsigned_value >> 16) & 0xFF);
  frame[2] = (uint8_t)((test_unsigned_value >> 8) & 0xFF);
  frame[3] = (uint8_t)((test_unsigned_value >> 0) & 0xFF);

  uint32_t extracted_value = get_unsigned_value_from_frame_and_size(frame, 4);
  TEST_ASSERT_EQUAL_UINT32(test_unsigned_value, extracted_value);

  // Test with 0xfffe = 65534, size 2 bytes.
  frame[0] = 0xff;
  frame[1] = 0xfe;

  extracted_value = get_unsigned_value_from_frame_and_size(frame, 2);
  TEST_ASSERT_EQUAL_UINT32(65534, extracted_value);

  // Test with 0xfed4 = 65236, size 2 bytes.
  frame[0] = 0xfe;
  frame[1] = 0xd4;

  extracted_value = get_unsigned_value_from_frame_and_size(frame, 2);
  TEST_ASSERT_EQUAL_UINT32(65236, extracted_value);

  //  Try with 1 byte signed value:
  const uint8_t test_unsigned_value_1 = 37;
  // Pack in the frame, MSB first
  frame[0] = (uint8_t)test_unsigned_value_1;

  extracted_value = get_unsigned_value_from_frame_and_size(frame, 1);
  TEST_ASSERT_EQUAL_UINT32(test_unsigned_value_1, extracted_value);

  //  Try with 2 bytes signed value:
  const uint16_t test_unsigned_value_2 = 15000;
  // Pack in the frame, MSB first
  frame[0] = (uint8_t)((test_unsigned_value_2 >> 8) & 0xFF);
  frame[1] = (uint8_t)((test_unsigned_value_2 >> 0) & 0xFF);

  extracted_value = get_unsigned_value_from_frame_and_size(frame, 2);
  TEST_ASSERT_EQUAL_UINT32(test_unsigned_value_2, extracted_value);
}

void test_value_size_precision_extraction()
{
  const int32_t test_signed_value = 3560;
  uint8_t frame[4]                = {};
  // Pack in the frame, MSB first
  frame[0] = (uint8_t)((test_signed_value >> 24) & 0xFF);
  frame[1] = (uint8_t)((test_signed_value >> 16) & 0xFF);
  frame[2] = (uint8_t)((test_signed_value >> 8) & 0xFF);
  frame[3] = (uint8_t)((test_signed_value >> 0) & 0xFF);

  uint32_t extracted_value = command_class_get_int32_value(4, 3, frame);
  TEST_ASSERT_EQUAL_UINT32(test_signed_value, extracted_value);

  extracted_value = command_class_get_int32_value(4, 0, frame);
  TEST_ASSERT_EQUAL_UINT32(test_signed_value * 1000, extracted_value);

  extracted_value = command_class_get_int32_value(4, 1, frame);
  TEST_ASSERT_EQUAL_UINT32(test_signed_value * 100, extracted_value);

  extracted_value = command_class_get_int32_value(4, 2, frame);
  TEST_ASSERT_EQUAL_UINT32(test_signed_value * 10, extracted_value);
}