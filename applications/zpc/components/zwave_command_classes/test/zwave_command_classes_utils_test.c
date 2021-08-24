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
#include "zwave_controller_utils_mock.h"
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
  uint8_t role_type                     = 0x01;
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
                                               sizeof(uint8_t),
                                               SL_STATUS_OK);
  attribute_store_get_reported_IgnoreArg_value();
  attribute_store_get_reported_ReturnThruPtr_value(&role_type);
  TEST_ASSERT_EQUAL_INT8(role_type, get_zwave_node_role_type(test_node_id));
}