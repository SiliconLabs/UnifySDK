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
// Test includes
#include "unity.h"
#include "ucl_mqtt_node_interview.h"

// Mocks
#include "attribute_store_mock.h"
#include "zpc_attribute_store_mock.h"
#include "zpc_attribute_store_network_helper_mock.h"

// Regular includes
#include "attribute_store_defined_attribute_types.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_ucl_mqtt_initiate_node_interview_happy_case()
{
  unid_t test_unid = "qwerty";
  is_zpc_unid_ExpectAndReturn(test_unid, false);
  attribute_store_node_t test_node = 349;
  attribute_store_network_helper_get_node_id_node_ExpectAndReturn(test_unid,
                                                                  test_node);

  attribute_store_node_t test_current_node = 1;
  attribute_store_get_node_child_by_type_ExpectAndReturn(test_node,
                                                         ATTRIBUTE_ENDPOINT_ID,
                                                         0,
                                                         test_current_node);
  attribute_store_delete_node_ExpectAndReturn(test_current_node, SL_STATUS_OK);

  for (test_current_node = 2; test_current_node < 5; test_current_node++) {
    attribute_store_get_node_child_by_type_ExpectAndReturn(
      test_node,
      ATTRIBUTE_ENDPOINT_ID,
      0,
      test_current_node);
    attribute_store_delete_node_ExpectAndReturn(test_current_node,
                                                SL_STATUS_OK);
  }

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node,
    ATTRIBUTE_ENDPOINT_ID,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);

  // Detetion is done. Now creating again
  test_current_node = 1;
  attribute_store_network_helper_create_endpoint_node_ExpectAndReturn(
    test_unid,
    0,
    test_current_node);
  attribute_store_add_node_ExpectAndReturn(ATTRIBUTE_ZWAVE_NIF,
                                           test_current_node,
                                           SL_STATUS_OK);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, ucl_mqtt_initiate_node_interview(test_unid));
}

void test_ucl_mqtt_initiate_node_interview_node_id_error()
{
  unid_t test_unid = "qwerty";
  is_zpc_unid_ExpectAndReturn(test_unid, false);
  attribute_store_network_helper_get_node_id_node_ExpectAndReturn(
    test_unid,
    ATTRIBUTE_STORE_INVALID_NODE);

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    ucl_mqtt_initiate_node_interview(test_unid));
}

void test_ucl_mqtt_initiate_node_interview_delete_error()
{
  unid_t test_unid = "azerty";
  is_zpc_unid_ExpectAndReturn(test_unid, false);
  attribute_store_node_t test_node = 349;
  attribute_store_network_helper_get_node_id_node_ExpectAndReturn(test_unid,
                                                                  test_node);

  attribute_store_node_t test_current_node = 1;
  attribute_store_get_node_child_by_type_ExpectAndReturn(test_node,
                                                         ATTRIBUTE_ENDPOINT_ID,
                                                         0,
                                                         test_current_node);
  attribute_store_delete_node_ExpectAndReturn(test_current_node,
                                              SL_STATUS_NOT_INITIALIZED);

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    ucl_mqtt_initiate_node_interview(test_unid));
}

void test_ucl_mqtt_initiate_node_interview_zpc_unid()
{
  unid_t test_unid = "MyUNID";
  is_zpc_unid_ExpectAndReturn(test_unid, true);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, ucl_mqtt_initiate_node_interview(test_unid));
}

void test_ucl_mqtt_initiate_endpoint_interview_happy_case()
{
  unid_t test_unid = "qwerty";
  is_zpc_unid_ExpectAndReturn(test_unid, false);
  zwave_endpoint_id_t test_endpoint_id = 25;

  attribute_store_node_t test_node = 349;
  attribute_store_network_helper_get_node_id_node_ExpectAndReturn(test_unid,
                                                                  test_node);

  attribute_store_node_t test_endpoint_node = 1;
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    test_node,
    ATTRIBUTE_ENDPOINT_ID,
    REPORTED_ATTRIBUTE,
    NULL,
    sizeof(zwave_endpoint_id_t),
    0,
    test_endpoint_node);
  attribute_store_get_node_child_by_value_IgnoreArg_value();

  attribute_store_delete_node_ExpectAndReturn(test_endpoint_node, SL_STATUS_OK);

  // Detetion is done. Now creating again
  test_endpoint_node = 2;
  attribute_store_network_helper_create_endpoint_node_ExpectAndReturn(
    test_unid,
    test_endpoint_id,
    test_endpoint_node);
  attribute_store_add_node_ExpectAndReturn(ATTRIBUTE_ZWAVE_NIF,
                                           test_endpoint_node,
                                           SL_STATUS_OK);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    ucl_mqtt_initiate_endpoint_interview(test_unid, test_endpoint_id));
}

void test_ucl_mqtt_initiate_endpoint_interview_node_id_error()
{
  unid_t test_unid = "qwerty";
  is_zpc_unid_ExpectAndReturn(test_unid, false);
  zwave_endpoint_id_t test_endpoint_id = 25;

  attribute_store_network_helper_get_node_id_node_ExpectAndReturn(
    test_unid,
    ATTRIBUTE_STORE_INVALID_NODE);

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_FOUND,
    ucl_mqtt_initiate_endpoint_interview(test_unid, test_endpoint_id));
}

void test_ucl_mqtt_initiate_endpoint_interview_endpoint_node_error()
{
  unid_t test_unid = "qwerty";
  is_zpc_unid_ExpectAndReturn(test_unid, false);
  zwave_endpoint_id_t test_endpoint_id = 25;

  attribute_store_node_t test_node = 349;
  attribute_store_network_helper_get_node_id_node_ExpectAndReturn(test_unid,
                                                                  test_node);

  attribute_store_get_node_child_by_value_ExpectAndReturn(
    test_node,
    ATTRIBUTE_ENDPOINT_ID,
    REPORTED_ATTRIBUTE,
    NULL,
    sizeof(zwave_endpoint_id_t),
    0,
    ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_get_node_child_by_value_IgnoreArg_value();

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_FOUND,
    ucl_mqtt_initiate_endpoint_interview(test_unid, test_endpoint_id));
}

void test_ucl_mqtt_initiate_endpoint_interview_delete_error()
{
  unid_t test_unid = "qwerty";
  is_zpc_unid_ExpectAndReturn(test_unid, false);
  zwave_endpoint_id_t test_endpoint_id = 25;

  attribute_store_node_t test_node = 349;
  attribute_store_network_helper_get_node_id_node_ExpectAndReturn(test_unid,
                                                                  test_node);

  attribute_store_node_t test_endpoint_node = 1;
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    test_node,
    ATTRIBUTE_ENDPOINT_ID,
    REPORTED_ATTRIBUTE,
    NULL,
    sizeof(zwave_endpoint_id_t),
    0,
    test_endpoint_node);
  attribute_store_get_node_child_by_value_IgnoreArg_value();

  attribute_store_delete_node_ExpectAndReturn(test_endpoint_node,
                                              SL_STATUS_NOT_INITIALIZED);

  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    ucl_mqtt_initiate_endpoint_interview(test_unid, test_endpoint_id));
}

void test_ucl_mqtt_initiate_endpoint_interview_zpc_unid()
{
  unid_t test_unid = "MyUNID";
  is_zpc_unid_ExpectAndReturn(test_unid, true);
  zwave_endpoint_id_t test_endpoint_id = 4;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    ucl_mqtt_initiate_endpoint_interview(test_unid, test_endpoint_id));
}