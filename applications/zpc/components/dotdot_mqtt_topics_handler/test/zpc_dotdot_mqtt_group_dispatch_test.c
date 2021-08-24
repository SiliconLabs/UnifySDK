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
#include "zpc_dotdot_mqtt.h"

// Interfaces
#include "attribute_store_defined_attribute_types.h"

// Mock Includes
#include "dotdot_mqtt_mock.h"
#include "attribute_store_mock.h"
#include "zpc_attribute_store_mock.h"
#include "attribute_store_helper_mock.h"
#include "zpc_attribute_store_network_helper_mock.h"
#include "zcl_group_cluster_server_mock.h"

// Private variables
static group_dispatch_t zpc_dotdot_mqtt_group_dispatch_save = NULL;
static const char *test_message                             = "Test message";
static unid_t test_unid                                     = "unid-4321";
static zwave_endpoint_id_t test_endpoint_id                 = 48;

// Trick: We excluded this function from the library for the test to avoid
// a million calls to uic_dotdot_mqtt at init.
// We make a stub here by redefining the function.
sl_status_t dotdot_mqtt_topics_handler_init()
{
  return SL_STATUS_OK;
}

// Stub functions
void uic_mqtt_dotdot_set_group_dispatch_stub(group_dispatch_t callback,
                                             int cmock_num_calls)
{
  zpc_dotdot_mqtt_group_dispatch_save = callback;
}

// test function
void mqtt_message_callback_test_function(const char *topic,
                                         const char *message,
                                         const size_t message_length)
{
  TEST_ASSERT_EQUAL_STRING(test_message, message);
  TEST_ASSERT_EQUAL(sizeof(test_message), message_length);
  const char expected_topic[]
    = "ucl/by-unid/unid-4321/ep48/MyOwnCluster/Commands/SuperCommand";
  TEST_ASSERT_EQUAL_STRING(expected_topic, topic);
}
/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  zpc_dotdot_mqtt_group_dispatch_save = NULL;
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_zpc_dotdot_mqtt_group_dispatch_test()
{
  uic_mqtt_dotdot_set_group_dispatch_callback_Stub(
    uic_mqtt_dotdot_set_group_dispatch_stub);
  // First init and save the registered callback
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zpc_dodot_mqtt_init());

  TEST_ASSERT_NOT_NULL(zpc_dotdot_mqtt_group_dispatch_save);
  uint16_t test_group_id         = 39;
  const char test_cluster_name[] = "MyOwnCluster";
  const char test_command[]      = "SuperCommand";

  attribute_store_node_t test_home_id_node = 3485;
  get_zpc_network_node_ExpectAndReturn(test_home_id_node);

  attribute_store_node_t test_node_id_1_node = 1;
  attribute_store_get_node_child_by_type_ExpectAndReturn(test_home_id_node,
                                                         ATTRIBUTE_NODE_ID,
                                                         0,
                                                         test_node_id_1_node);

  attribute_store_network_helper_get_unid_from_node_ExpectAndReturn(
    test_node_id_1_node,
    NULL,
    SL_STATUS_FAIL);
  attribute_store_network_helper_get_unid_from_node_IgnoreArg_unid();
  // Here we move on to the next node:
  attribute_store_node_t test_node_id_2_node = 2;
  attribute_store_get_node_child_by_type_ExpectAndReturn(test_home_id_node,
                                                         ATTRIBUTE_NODE_ID,
                                                         1,
                                                         test_node_id_2_node);

  attribute_store_network_helper_get_unid_from_node_ExpectAndReturn(
    test_node_id_2_node,
    NULL,
    SL_STATUS_OK);
  attribute_store_network_helper_get_unid_from_node_IgnoreArg_unid();
  attribute_store_network_helper_get_unid_from_node_ReturnMemThruPtr_unid(
    test_unid,
    sizeof(unid_t));

  // First endpoint for node 2
  attribute_store_node_t test_endpoint_id_1_node = 100;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_2_node,
    ATTRIBUTE_ENDPOINT_ID,
    0,
    test_endpoint_id_1_node);

  attribute_store_read_value_ExpectAndReturn(test_endpoint_id_1_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(zwave_endpoint_id_t),
                                             SL_STATUS_FAIL);
  attribute_store_read_value_IgnoreArg_read_value();

  //Move on to endpoint 2
  attribute_store_node_t test_endpoint_id_2_node = 102;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_2_node,
    ATTRIBUTE_ENDPOINT_ID,
    1,
    test_endpoint_id_2_node);
  attribute_store_read_value_ExpectAndReturn(test_endpoint_id_2_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(zwave_endpoint_id_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &test_endpoint_id,
    sizeof(zwave_endpoint_id_t));

  is_unid_endpoint_in_group_ExpectAndReturn(test_unid,
                                            test_endpoint_id,
                                            test_group_id,
                                            true);
  //Move on to endpoint 3
  attribute_store_node_t test_endpoint_id_3_node = 103;
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_2_node,
    ATTRIBUTE_ENDPOINT_ID,
    2,
    test_endpoint_id_3_node);
  attribute_store_read_value_ExpectAndReturn(test_endpoint_id_3_node,
                                             REPORTED_ATTRIBUTE,
                                             NULL,
                                             sizeof(zwave_endpoint_id_t),
                                             SL_STATUS_OK);
  attribute_store_read_value_IgnoreArg_read_value();
  attribute_store_read_value_ReturnMemThruPtr_read_value(
    &test_endpoint_id,
    sizeof(zwave_endpoint_id_t));

  // Not part of the group
  is_unid_endpoint_in_group_ExpectAndReturn(test_unid,
                                            test_endpoint_id,
                                            test_group_id,
                                            false);

  // No more endpoints
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_2_node,
    ATTRIBUTE_ENDPOINT_ID,
    3,
    ATTRIBUTE_STORE_INVALID_NODE);

  // No more NodeIDs
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_home_id_node,
    ATTRIBUTE_NODE_ID,
    2,
    ATTRIBUTE_STORE_INVALID_NODE);
  zpc_dotdot_mqtt_group_dispatch_save(test_group_id,
                                      test_cluster_name,
                                      test_command,
                                      test_message,
                                      sizeof(test_message),
                                      mqtt_message_callback_test_function);
}