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
#include "zwave_command_class_multi_channel.c"
#include "zwave_command_class_multi_channel.h"

// Generic includes
#include <string.h>

// Test includes
#include "unity.h"

// Includes from other components
#include "ZW_classcmd.h"
#include "attribute_store_defined_attribute_types.h"

// Mocks
#include "zwave_controller_transport_mock.h"
#include "attribute_store_mock.h"
#include "attribute_store_helper_mock.h"
#include "zpc_attribute_store_network_helper_mock.h"
#include "attribute_resolver_mock.h"
#include "zwave_command_handler_mock.h"
#include "zwave_controller_internal_mock.h"

static uint8_t last_received_frame[ZWAVE_MAX_FRAME_SIZE]       = {0};
static uint16_t last_received_frame_length                     = 0;
static zwave_endpoint_id_t test_zwave_endpoint_id              = 0;
static zwave_controller_connection_info_t test_connection_info = {};
static uint8_t number_of_aggregated_ep                         = 0;
static uint8_t number_of_individual_ep                         = 0;
static uint8_t supporting_node_version                         = 0;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Called before each and every test
void setUp()
{
  last_received_frame_length = 0;
  memset(last_received_frame, 0, sizeof(last_received_frame));
  test_zwave_endpoint_id = 0;
  memset(&test_connection_info, 0, sizeof(test_connection_info));
  number_of_aggregated_ep = 0;
  number_of_individual_ep = 0;
  supporting_node_version = 0;
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void test_zwave_command_class_multi_channel_init()
{
  attribute_resolver_register_rule_ExpectAndReturn(
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_IDENTICAL_ENDPOINTS,
    NULL,
    &zwave_command_class_multi_channel_endpoint_get,
    SL_STATUS_OK);

  attribute_resolver_register_rule_ExpectAndReturn(
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_ALL_INDIVIDUAL_ENDPOINTS_FOUND,
    NULL,
    &zwave_command_class_multi_channel_endpoint_find,
    SL_STATUS_OK);

  attribute_resolver_register_rule_ExpectAndReturn(
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_MEMBERS,
    NULL,
    &zwave_command_class_multi_channel_aggregated_members_get,
    SL_STATUS_OK);

  attribute_store_register_callback_by_type_ExpectAndReturn(
    &zwave_command_class_multi_channel_on_version_attribute_update,
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_VERSION,
    SL_STATUS_NOT_FOUND);

  zwave_command_handler_register_handler_IgnoreAndReturn(SL_STATUS_FAIL);

  // Call the function, it should return a OR operation of the SL status
  // that it received
  TEST_ASSERT_EQUAL(SL_STATUS_OK | SL_STATUS_FAIL | SL_STATUS_NOT_FOUND,
                    zwave_command_class_multi_channel_init());
}

void test_zwave_command_class_multi_channel_endpoint_get()
{
  attribute_store_node_t test_node = 01;

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_multi_channel_endpoint_get(
                      test_node,
                      last_received_frame,
                      &last_received_frame_length));

  const uint8_t expected_frame_data[]
    = {COMMAND_CLASS_MULTI_CHANNEL_V4, MULTI_CHANNEL_END_POINT_GET_V4};
  TEST_ASSERT_EQUAL(sizeof(expected_frame_data), last_received_frame_length);
  TEST_ASSERT_EQUAL_INT8_ARRAY(expected_frame_data,
                               last_received_frame,
                               last_received_frame_length);
}

void test_zwave_command_class_multi_channel_capability_get_endpoint_0()
{
  attribute_store_node_t test_node          = 384;
  attribute_store_node_t test_endpoint_node = 0xf7;

  attribute_store_get_first_parent_with_type_ExpectAndReturn(
    test_node,
    ATTRIBUTE_ENDPOINT_ID,
    test_endpoint_node);

  test_zwave_endpoint_id = 0;
  attribute_store_read_value_ExpectAndReturn(test_endpoint_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(zwave_endpoint_id_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &test_zwave_endpoint_id,
    sizeof(test_zwave_endpoint_id));

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    zwave_command_class_multi_channel_capability_get(
                      test_node,
                      last_received_frame,
                      &last_received_frame_length));

  TEST_ASSERT_EQUAL(0, last_received_frame_length);
}

void test_zwave_command_class_multi_channel_capability_get_happy_case()
{
  attribute_store_node_t test_node          = 384;
  attribute_store_node_t test_endpoint_node = 0xf7;

  attribute_store_get_first_parent_with_type_ExpectAndReturn(
    test_node,
    ATTRIBUTE_ENDPOINT_ID,
    test_endpoint_node);

  test_zwave_endpoint_id = 5;
  attribute_store_read_value_ExpectAndReturn(test_endpoint_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(zwave_endpoint_id_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &test_zwave_endpoint_id,
    sizeof(test_zwave_endpoint_id));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_multi_channel_capability_get(
                      test_node,
                      last_received_frame,
                      &last_received_frame_length));

  const uint8_t expected_frame_data[]
    = {COMMAND_CLASS_MULTI_CHANNEL_V4,
       MULTI_CHANNEL_CAPABILITY_GET_V4,
       test_zwave_endpoint_id & ENDPOINT_MASK};
  TEST_ASSERT_EQUAL(sizeof(expected_frame_data), last_received_frame_length);
  TEST_ASSERT_EQUAL_INT8_ARRAY(expected_frame_data,
                               last_received_frame,
                               last_received_frame_length);
}

void test_zwave_command_class_multi_channel_endpoint_find()
{
  attribute_store_node_t test_node = 0x9485;

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_multi_channel_endpoint_find(
                      test_node,
                      last_received_frame,
                      &last_received_frame_length));

  const uint8_t expected_frame_data[] = {COMMAND_CLASS_MULTI_CHANNEL_V4,
                                         MULTI_CHANNEL_END_POINT_FIND_V4,
                                         0xFF,
                                         0xFF};
  TEST_ASSERT_EQUAL(sizeof(expected_frame_data), last_received_frame_length);
  TEST_ASSERT_EQUAL_INT8_ARRAY(expected_frame_data,
                               last_received_frame,
                               last_received_frame_length);
}

void test_zwave_command_class_multi_channel_aggregated_members_get_endpoint_0()
{
  attribute_store_node_t test_node             = 12345;
  attribute_store_node_t test_endpoint_id_node = 43;

  attribute_store_get_first_parent_with_type_ExpectAndReturn(
    test_node,
    ATTRIBUTE_ENDPOINT_ID,
    test_endpoint_id_node);

  test_zwave_endpoint_id = 0;
  attribute_store_read_value_ExpectAndReturn(test_endpoint_id_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(zwave_endpoint_id_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &test_zwave_endpoint_id,
    sizeof(test_zwave_endpoint_id));

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    zwave_command_class_multi_channel_aggregated_members_get(
                      test_node,
                      last_received_frame,
                      &last_received_frame_length));

  TEST_ASSERT_EQUAL(0, last_received_frame_length);
}

void test_zwave_command_class_multi_channel_aggregated_members_get_happy_case()
{
  attribute_store_node_t test_node          = 12345;
  attribute_store_node_t test_endpoint_node = 43;

  attribute_store_get_first_parent_with_type_ExpectAndReturn(
    test_node,
    ATTRIBUTE_ENDPOINT_ID,
    test_endpoint_node);

  test_zwave_endpoint_id = 3;
  attribute_store_read_value_ExpectAndReturn(test_endpoint_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(zwave_endpoint_id_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &test_zwave_endpoint_id,
    sizeof(test_zwave_endpoint_id));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_multi_channel_aggregated_members_get(
                      test_node,
                      last_received_frame,
                      &last_received_frame_length));

  const uint8_t expected_frame_data[]
    = {COMMAND_CLASS_MULTI_CHANNEL_V4,
       MULTI_CHANNEL_AGGREGATED_MEMBERS_GET_V4,
       test_zwave_endpoint_id & ENDPOINT_MASK};
  TEST_ASSERT_EQUAL(sizeof(expected_frame_data), last_received_frame_length);
  TEST_ASSERT_EQUAL_INT8_ARRAY(expected_frame_data,
                               last_received_frame,
                               last_received_frame_length);
}

void test_zwave_command_class_multi_channel_control_handler_frame_too_short()
{
  const uint8_t test_frame_data[] = {COMMAND_CLASS_MULTI_CHANNEL_V4};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    zwave_command_class_multi_channel_control_handler(&test_connection_info,
                                                      test_frame_data,
                                                      sizeof(test_frame_data)));
}

void test_zwave_command_class_multi_channel_control_handler_wrong_cc()
{
  const uint8_t test_frame_data[]
    = {COMMAND_CLASS_VERSION_V3, MULTI_CHANNEL_END_POINT_REPORT_V4};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    zwave_command_class_multi_channel_control_handler(&test_connection_info,
                                                      test_frame_data,
                                                      sizeof(test_frame_data)));
}

void test_zwave_command_class_multi_channel_control_handler_unknown_command()
{
  const uint8_t test_frame_data[] = {COMMAND_CLASS_MULTI_CHANNEL_V4, 0xFE};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    zwave_command_class_multi_channel_control_handler(&test_connection_info,
                                                      test_frame_data,
                                                      sizeof(test_frame_data)));
}

void test_zwave_command_class_multi_channel_control_handler_endpoint_report_too_short()
{
  const uint8_t test_frame_data[]
    = {COMMAND_CLASS_MULTI_CHANNEL_V4, MULTI_CHANNEL_END_POINT_REPORT_V4};

  // In this case we will save zeros in the attribute store

  attribute_store_node_t test_node_id_node = 60;
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_connection_info.remote.node_id,
    test_node_id_node);

  attribute_store_node_t test_endpoint_node = 355074;
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ENDPOINT_ID,
    REPORTED_ATTRIBUTE,
    NULL,
    sizeof(zwave_endpoint_id_t),
    0,
    test_endpoint_node);
  attribute_store_get_node_child_by_value_IgnoreArg_value();

  attribute_store_node_t test_identical_node = 29;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_IDENTICAL_ENDPOINTS,
    0,
    test_identical_node);

  uint8_t expected_value = 0;
  attribute_store_set_reported_ExpectWithArrayAndReturn(test_identical_node,
                                                        &expected_value,
                                                        sizeof(uint8_t),
                                                        sizeof(uint8_t),
                                                        SL_STATUS_OK);

  attribute_store_node_t test_individual_endpoints_node = 20;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_INDIVIDUAL_ENDPOINTS,
    0,
    test_individual_endpoints_node);

  attribute_store_set_reported_ExpectWithArrayAndReturn(
    test_individual_endpoints_node,
    &expected_value,
    sizeof(uint8_t),
    sizeof(uint8_t),
    SL_STATUS_OK);

  attribute_store_node_t test_aggregated_endpoints_node = 20;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_ENDPOINTS,
    0,
    test_aggregated_endpoints_node);

  attribute_store_set_reported_ExpectWithArrayAndReturn(
    test_aggregated_endpoints_node,
    &expected_value,
    sizeof(uint8_t),
    sizeof(uint8_t),
    SL_STATUS_OK);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_command_class_multi_channel_control_handler(&test_connection_info,
                                                      test_frame_data,
                                                      sizeof(test_frame_data)));
}

void test_zwave_command_class_multi_channel_control_handler_endpoint_report()
{
  test_connection_info.remote.node_id = 34;

  const uint8_t test_frame_data[] = {COMMAND_CLASS_MULTI_CHANNEL_V4,
                                     MULTI_CHANNEL_END_POINT_REPORT_V4,
                                     0xFE,
                                     0x49,
                                     0x29};

  attribute_store_node_t test_node_id_node = 493;
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_connection_info.remote.node_id,
    test_node_id_node);

  attribute_store_node_t test_endpoint_node = 355074;
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ENDPOINT_ID,
    REPORTED_ATTRIBUTE,
    NULL,
    sizeof(zwave_endpoint_id_t),
    0,
    test_endpoint_node);
  attribute_store_get_node_child_by_value_IgnoreArg_value();

  attribute_store_node_t test_identical_node = 29;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_IDENTICAL_ENDPOINTS,
    0,
    test_identical_node);

  uint8_t expected_identical_value = test_frame_data[2] & 0x40;
  attribute_store_set_reported_ExpectWithArrayAndReturn(
    test_identical_node,
    &expected_identical_value,
    sizeof(uint8_t),
    sizeof(uint8_t),
    SL_STATUS_OK);

  attribute_store_node_t test_individual_endpoints_node = 20;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_INDIVIDUAL_ENDPOINTS,
    0,
    test_individual_endpoints_node);

  uint8_t expected_individual_endpoint_value
    = test_frame_data[3] & ENDPOINT_MASK;
  attribute_store_set_reported_ExpectWithArrayAndReturn(
    test_individual_endpoints_node,
    &expected_individual_endpoint_value,
    sizeof(uint8_t),
    sizeof(uint8_t),
    SL_STATUS_OK);

  attribute_store_node_t test_aggregated_endpoints_node = 25;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_ENDPOINTS,
    0,
    test_aggregated_endpoints_node);

  uint8_t expected_aggregated_endpoint_value
    = test_frame_data[4] & ENDPOINT_MASK;
  attribute_store_set_reported_ExpectWithArrayAndReturn(
    test_aggregated_endpoints_node,
    &expected_aggregated_endpoint_value,
    sizeof(uint8_t),
    sizeof(uint8_t),
    SL_STATUS_OK);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_command_class_multi_channel_control_handler(&test_connection_info,
                                                      test_frame_data,
                                                      sizeof(test_frame_data)));
}

void test_zwave_command_class_multi_channel_control_handler_capability_report_too_short()
{
  const uint8_t test_frame_data[] = {COMMAND_CLASS_MULTI_CHANNEL_V4,
                                     MULTI_CHANNEL_CAPABILITY_REPORT_V4,
                                     0x1,
                                     0x02,
                                     0x03};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_command_class_multi_channel_control_handler(&test_connection_info,
                                                      test_frame_data,
                                                      sizeof(test_frame_data)));
}

void test_zwave_command_class_multi_channel_control_handler_capability_report()
{
  test_connection_info.remote.node_id = 232;

  const uint8_t test_frame_data[] = {COMMAND_CLASS_MULTI_CHANNEL_V4,
                                     MULTI_CHANNEL_CAPABILITY_REPORT_V4,
                                     0x25,
                                     0x49,
                                     0x00,
                                     0x20,
                                     0x21,
                                     0x25,
                                     0x26,
                                     0x9F};

  attribute_store_node_t test_node_id_node = 999;
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_connection_info.remote.node_id,
    test_node_id_node);

  attribute_store_node_t test_endpoint_node = 33474;
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ENDPOINT_ID,
    REPORTED_ATTRIBUTE,
    NULL,
    sizeof(zwave_endpoint_id_t),
    0,
    test_endpoint_node);
  attribute_store_get_node_child_by_value_IgnoreArg_value();

  attribute_store_node_t test_endpoint_generic_device_class = 29;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_ZWAVE_GENERIC_DEVICE_CLASS,
    0,
    test_endpoint_generic_device_class);

  attribute_store_set_node_attribute_value_ExpectAndReturn(
    test_endpoint_generic_device_class,
    REPORTED_ATTRIBUTE,
    NULL,
    sizeof(uint8_t),
    SL_STATUS_OK);
  attribute_store_set_node_attribute_value_IgnoreArg_value();

  attribute_store_node_t endpoint_specific_device_class = 4;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_ZWAVE_SPECIFIC_DEVICE_CLASS,
    0,
    endpoint_specific_device_class);

  attribute_store_set_node_attribute_value_ExpectAndReturn(
    endpoint_specific_device_class,
    REPORTED_ATTRIBUTE,
    NULL,
    sizeof(uint8_t),
    SL_STATUS_OK);
  attribute_store_set_node_attribute_value_IgnoreArg_value();

  attribute_store_node_t endpoint_nif = 2021;
  attribute_store_get_node_child_by_type_ExpectAndReturn(test_endpoint_node,
                                                         ATTRIBUTE_ZWAVE_NIF,
                                                         0,
                                                         endpoint_nif);

  attribute_store_set_node_attribute_value_ExpectAndReturn(endpoint_nif,
                                                           REPORTED_ATTRIBUTE,
                                                           NULL,
                                                           5 * sizeof(uint8_t),
                                                           SL_STATUS_OK);
  attribute_store_set_node_attribute_value_IgnoreArg_value();

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_command_class_multi_channel_control_handler(&test_connection_info,
                                                      test_frame_data,
                                                      sizeof(test_frame_data)));
}

void test_zwave_command_class_multi_channel_control_handler_aggregated_members_report_too_short()
{
  const uint8_t test_frame_data[] = {COMMAND_CLASS_MULTI_CHANNEL_V4,
                                     MULTI_CHANNEL_AGGREGATED_MEMBERS_REPORT_V4,
                                     0x1};

  attribute_store_node_t test_node_id_node = 999;
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_connection_info.remote.node_id,
    test_node_id_node);

  attribute_store_node_t test_endpoint_node = 33474;
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ENDPOINT_ID,
    REPORTED_ATTRIBUTE,
    NULL,
    sizeof(zwave_endpoint_id_t),
    0,
    test_endpoint_node);
  attribute_store_get_node_child_by_value_IgnoreArg_value();

  attribute_store_node_t test_aggregated_members_node = 4;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_MEMBERS,
    0,
    test_aggregated_members_node);

  attribute_store_delete_node_ExpectAndReturn(test_aggregated_members_node,
                                              SL_STATUS_OK);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_command_class_multi_channel_control_handler(&test_connection_info,
                                                      test_frame_data,
                                                      sizeof(test_frame_data)));
}

void test_zwave_command_class_multi_channel_control_handler_aggregated_member_report_happy()
{
  test_connection_info.remote.node_id = 232;

  const uint8_t test_frame_data[]
    = {COMMAND_CLASS_MULTI_CHANNEL_V4,
       MULTI_CHANNEL_AGGREGATED_MEMBERS_REPORT_V4,
       0x25,
       0x30,  // dangerous length here, only 2 bytes available.
       0x00,
       0x20};

  attribute_store_node_t test_node_id_node = 999;
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_connection_info.remote.node_id,
    test_node_id_node);

  attribute_store_node_t test_endpoint_node = 33474;
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ENDPOINT_ID,
    REPORTED_ATTRIBUTE,
    NULL,
    sizeof(zwave_endpoint_id_t),
    0,
    test_endpoint_node);
  attribute_store_get_node_child_by_value_IgnoreArg_value();

  attribute_store_node_t test_aggregated_members_node = 0;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_MEMBERS,
    0,
    test_aggregated_members_node);

  uint8_t expected_values[] = {test_frame_data[4], test_frame_data[5]};
  attribute_store_set_reported_ExpectWithArrayAndReturn(
    test_aggregated_members_node,
    &expected_values,
    sizeof(expected_values),
    sizeof(expected_values),
    SL_STATUS_OK);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_command_class_multi_channel_control_handler(&test_connection_info,
                                                      test_frame_data,
                                                      sizeof(test_frame_data)));
}

void test_zwave_command_class_multi_channel_control_handler_aggregated_member_report_no_members()
{
  test_connection_info.remote.node_id = 12;

  const uint8_t test_frame_data[] = {COMMAND_CLASS_MULTI_CHANNEL_V4,
                                     MULTI_CHANNEL_AGGREGATED_MEMBERS_REPORT_V4,
                                     0x25,
                                     0x00,  // No members
                                     0x00,
                                     0x20};

  attribute_store_node_t test_node_id_node = 999;
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_connection_info.remote.node_id,
    test_node_id_node);

  attribute_store_node_t test_endpoint_node = 33474;
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_ENDPOINT_ID,
    REPORTED_ATTRIBUTE,
    NULL,
    sizeof(zwave_endpoint_id_t),
    0,
    test_endpoint_node);
  attribute_store_get_node_child_by_value_IgnoreArg_value();

  attribute_store_node_t test_aggregated_members_node = 0;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_MEMBERS,
    0,
    test_aggregated_members_node);

  attribute_store_delete_node_ExpectAndReturn(test_aggregated_members_node,
                                              SL_STATUS_OK);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_command_class_multi_channel_control_handler(&test_connection_info,
                                                      test_frame_data,
                                                      sizeof(test_frame_data)));
}

void test_zwave_command_class_multi_channel_control_handler_find_report_too_short()
{
  test_connection_info.remote.node_id = 255;

  const uint8_t test_frame_data[] = {COMMAND_CLASS_MULTI_CHANNEL_V4,
                                     MULTI_CHANNEL_END_POINT_FIND_REPORT_V4,
                                     0x25,
                                     0x00};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_command_class_multi_channel_control_handler(&test_connection_info,
                                                      test_frame_data,
                                                      sizeof(test_frame_data)));
}

static void
  create_new_endpoint_verification(attribute_store_node_t node_id_node,
                                   zwave_endpoint_id_t endpoint_id,
                                   bool is_aggregated)
{
  attribute_store_node_t test_endpoint_node = 0x3e2f;
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    node_id_node,
    ATTRIBUTE_ENDPOINT_ID,
    REPORTED_ATTRIBUTE,
    NULL,
    sizeof(endpoint_id),
    0,
    ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_get_node_child_by_value_IgnoreArg_value();

  attribute_store_add_node_ExpectAndReturn(ATTRIBUTE_ENDPOINT_ID,
                                           node_id_node,
                                           test_endpoint_node);

  attribute_store_set_node_attribute_value_ExpectAndReturn(test_endpoint_node,
                                                           REPORTED_ATTRIBUTE,
                                                           NULL,
                                                           sizeof(endpoint_id),
                                                           SL_STATUS_NOT_READY);
  attribute_store_set_node_attribute_value_IgnoreArg_value();

  const attribute_store_type_t expected_attributes[]
    = {ATTRIBUTE_ZWAVE_NIF,
       ATTRIBUTE_ZWAVE_GENERIC_DEVICE_CLASS,
       ATTRIBUTE_ZWAVE_SPECIFIC_DEVICE_CLASS};
  attribute_store_add_if_missing_Expect(test_endpoint_node,
                                        NULL,
                                        COUNT_OF(expected_attributes));
  attribute_store_add_if_missing_IgnoreArg_attributes();

  if (is_aggregated == true) {
    const attribute_store_type_t expected_aggregated_members_attribute[]
      = {ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_MEMBERS};
    attribute_store_add_if_missing_Expect(
      test_endpoint_node,
      NULL,
      COUNT_OF(expected_aggregated_members_attribute));
    attribute_store_add_if_missing_IgnoreArg_attributes();
  }
}

static void
  is_endpoint_aggregated_verification(attribute_store_node_t endpoint_node,
                                      bool return_value)
{
  if (return_value == true) {
    attribute_store_get_node_child_by_type_ExpectAndReturn(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_MEMBERS,
      0,
      0x02);
  } else {
    attribute_store_get_node_child_by_type_ExpectAndReturn(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_MEMBERS,
      0,
      ATTRIBUTE_STORE_INVALID_NODE);
  }
}

void test_is_endpoint_aggregated()
{
  attribute_store_node_t test_node = 0x9F;
  is_endpoint_aggregated_verification(test_node, true);
  TEST_ASSERT_EQUAL(true, is_endpoint_aggregated(test_node));

  is_endpoint_aggregated_verification(test_node, false);
  TEST_ASSERT_EQUAL(false, is_endpoint_aggregated(test_node));
}

static void
  create_aggregated_endpoints_verification(attribute_store_node_t node_id_node)
{
  attribute_store_node_t test_root_device_node = 0x01;
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    node_id_node,
    ATTRIBUTE_ENDPOINT_ID,
    REPORTED_ATTRIBUTE,
    NULL,
    sizeof(zwave_endpoint_id_t),
    0,
    test_root_device_node);
  attribute_store_get_node_child_by_value_IgnoreArg_value();

  attribute_store_node_t test_aggregated_eps_node = 100;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_root_device_node,
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_AGGREGATED_ENDPOINTS,
    0,
    test_aggregated_eps_node);

  number_of_aggregated_ep = 1;  // Number of aggregated endpoints
  attribute_store_read_value_ExpectAndReturn(test_aggregated_eps_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(uint8_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &number_of_aggregated_ep,
    sizeof(number_of_aggregated_ep));

  attribute_store_node_t test_current_endpoint_node = 0x01;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    node_id_node,
    ATTRIBUTE_ENDPOINT_ID,
    0,
    test_current_endpoint_node);

  // Entering the while loop. Let's do 3 individual endpoints
  for (uint8_t i = 0; i < 3; i++) {
    is_endpoint_aggregated_verification(test_current_endpoint_node, false);
    test_current_endpoint_node++;
    attribute_store_get_node_child_by_type_ExpectAndReturn(
      node_id_node,
      ATTRIBUTE_ENDPOINT_ID,
      i + 1,
      test_current_endpoint_node);
  }
  is_endpoint_aggregated_verification(test_current_endpoint_node, false);
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    node_id_node,
    ATTRIBUTE_ENDPOINT_ID,
    4,
    ATTRIBUTE_STORE_INVALID_NODE);

  // Check aggregated endpoint creation
  create_new_endpoint_verification(node_id_node, 4, true);
}

void test_create_aggregated_endpoints()
{
  attribute_store_node_t test_node = 0x26;
  create_aggregated_endpoints_verification(test_node);
  create_aggregated_endpoints(test_node);
}

void check_if_all_individual_endpoints_are_found_verification(
  attribute_store_node_t node_id_node)
{
  attribute_store_node_t test_root_device_node = 0x01;
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    node_id_node,
    ATTRIBUTE_ENDPOINT_ID,
    REPORTED_ATTRIBUTE,
    NULL,
    sizeof(zwave_endpoint_id_t),
    0,
    test_root_device_node);

  attribute_store_get_node_child_by_value_IgnoreArg_value();

  attribute_store_node_t test_individual_endpoint_count = 100;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_root_device_node,
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_INDIVIDUAL_ENDPOINTS,
    0,
    test_individual_endpoint_count);

  number_of_individual_ep = 3;
  attribute_store_read_value_ExpectAndReturn(test_individual_endpoint_count,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(number_of_individual_ep),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &number_of_individual_ep,
    sizeof(number_of_individual_ep));

  // Return the root device first. 3 individual endpoinst are returned.
  for (uint8_t i = 0; i < 4; i++) {
    attribute_store_get_node_child_by_type_ExpectAndReturn(
      node_id_node,
      ATTRIBUTE_ENDPOINT_ID,
      i,
      test_root_device_node + i);
    if (i != 0) {
      is_endpoint_aggregated_verification(test_root_device_node + i, false);
    }
  }
  //Exit the while loop:
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    node_id_node,
    ATTRIBUTE_ENDPOINT_ID,
    4,
    ATTRIBUTE_STORE_INVALID_NODE);

  attribute_store_set_uint8_child_by_type_ExpectAndReturn(
    test_root_device_node,
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_ALL_INDIVIDUAL_ENDPOINTS_FOUND,
    REPORTED_ATTRIBUTE,
    1,
    SL_STATUS_OK);

  create_aggregated_endpoints_verification(node_id_node);
}

void test_zwave_command_class_multi_channel_control_handler_find_report()
{
  test_connection_info.remote.node_id = 255;

  const uint8_t test_frame_data[] = {COMMAND_CLASS_MULTI_CHANNEL_V4,
                                     MULTI_CHANNEL_END_POINT_FIND_REPORT_V4,
                                     0x25,
                                     0x01,
                                     0x02,
                                     0x04,  // First endpoint
                                     0x05,
                                     0x09};

  attribute_store_node_t test_node_id_node = 304;
  attribute_store_network_helper_get_zwave_node_id_node_ExpectAndReturn(
    test_connection_info.remote.node_id,
    test_node_id_node);

  for (uint8_t index = 5; index < 8; index++) {
    zwave_endpoint_id_t current_endpoint = test_frame_data[index];
    create_new_endpoint_verification(test_node_id_node,
                                     current_endpoint,
                                     false);
  }
  check_if_all_individual_endpoints_are_found_verification(test_node_id_node);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_command_class_multi_channel_control_handler(&test_connection_info,
                                                      test_frame_data,
                                                      sizeof(test_frame_data)));
}

void test_zwave_command_class_multi_channel_on_version_attribute_update()
{
  attribute_store_node_t test_node = 5;

  // Nothing happens when we delete.
  zwave_command_class_multi_channel_on_version_attribute_update(
    test_node,
    ATTRIBUTE_DELETED);

  // Now we get an a supporting node with version < 3:
  attribute_store_get_node_type_ExpectAndReturn(test_node, VERSION_VERSION);
  attribute_store_read_value_ExpectAndReturn(test_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(uint8_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  supporting_node_version = 2;
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &supporting_node_version,
    sizeof(supporting_node_version));

  zwave_command_class_multi_channel_on_version_attribute_update(
    test_node,
    ATTRIBUTE_CREATED);

  // Now we get an version attribute created for a non-zero endpoint:
  attribute_store_get_node_type_ExpectAndReturn(test_node, VERSION_VERSION);
  attribute_store_read_value_ExpectAndReturn(test_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(uint8_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  supporting_node_version = 3;
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &supporting_node_version,
    sizeof(supporting_node_version));

  attribute_store_node_t endpoint_node = 39;
  attribute_store_get_first_parent_with_type_ExpectAndReturn(
    test_node,
    ATTRIBUTE_ENDPOINT_ID,
    endpoint_node);
  test_zwave_endpoint_id = 2;
  attribute_store_read_value_ExpectAndReturn(endpoint_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(zwave_endpoint_id_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &test_zwave_endpoint_id,
    sizeof(test_zwave_endpoint_id));
  zwave_command_class_multi_channel_on_version_attribute_update(
    test_node,
    ATTRIBUTE_CREATED);

  // Now the happy case with v3
  attribute_store_get_node_type_ExpectAndReturn(
    test_node,
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_VERSION);
  attribute_store_read_value_ExpectAndReturn(test_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(uint8_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  supporting_node_version = 3;
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &supporting_node_version,
    sizeof(supporting_node_version));

  attribute_store_get_first_parent_with_type_ExpectAndReturn(
    test_node,
    ATTRIBUTE_ENDPOINT_ID,
    endpoint_node);

  test_zwave_endpoint_id = 0;
  attribute_store_read_value_ExpectAndReturn(endpoint_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(zwave_endpoint_id_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &test_zwave_endpoint_id,
    sizeof(test_zwave_endpoint_id));

  attribute_store_add_if_missing_Expect(endpoint_node,
                                        v3_attributes,
                                        COUNT_OF(v3_attributes));

  zwave_command_class_multi_channel_on_version_attribute_update(
    test_node,
    ATTRIBUTE_UPDATED);

  // Now the happy case with v5
  attribute_store_get_node_type_ExpectAndReturn(
    test_node,
    ATTRIBUTE_COMMAND_CLASS_MULTI_CHANNEL_VERSION);
  attribute_store_read_value_ExpectAndReturn(test_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(uint8_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  supporting_node_version = 5;
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &supporting_node_version,
    sizeof(supporting_node_version));

  endpoint_node = 25;
  attribute_store_get_first_parent_with_type_ExpectAndReturn(
    test_node,
    ATTRIBUTE_ENDPOINT_ID,
    endpoint_node);
  test_zwave_endpoint_id = 0;
  attribute_store_read_value_ExpectAndReturn(endpoint_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(zwave_endpoint_id_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &test_zwave_endpoint_id,
    sizeof(test_zwave_endpoint_id));

  attribute_store_add_if_missing_Expect(endpoint_node,
                                        v3_attributes,
                                        COUNT_OF(v3_attributes));

  attribute_store_add_if_missing_Expect(endpoint_node,
                                        v4_attributes,
                                        COUNT_OF(v4_attributes));

  zwave_command_class_multi_channel_on_version_attribute_update(
    test_node,
    ATTRIBUTE_UPDATED);
}