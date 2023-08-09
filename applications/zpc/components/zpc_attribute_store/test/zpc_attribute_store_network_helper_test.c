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

// Includes from this component
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"

#include "zpc_attribute_store.h"

// Includes from other components
#include "attribute_store_fixt.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "datastore.h"

#include "sl_status.h"
#include "zwave_unid.h"
#include "zwave_controller_types.h"
#include "sl_log.h"

// Mock includes
#include "zwave_network_management_mock.h"

// Test includes
#include "unity.h"

// Test constants
static const zwave_home_id_t test_network_home_id = 0x01020304;
static unid_t my_unid_1                           = "zw-FC23F52D-0001";
static unid_t my_unid_2                           = "zw-FC23F52D-0031";
static unid_t my_unid_3                           = "zw-AAAAAAAA-0043";
static unid_t my_unid_test_network_expected       = "zw-01020304-00AA";
static attribute_store_node_t home_id_expected;
static attribute_store_node_t node_id_expected;
static attribute_store_node_t endpoint_expected;
static attribute_store_node_t binary_switch_value_node;
static attribute_store_node_t association_group_node;

// private function prototype
static void create_test_network();

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

void setUp()
{
  datastore_init(":memory:");
  attribute_store_init();

  // Ensure we start from scratch before creating our test network.
  attribute_store_delete_node(attribute_store_get_root());

  create_test_network();
  zwave_network_management_get_home_id_IgnoreAndReturn(test_network_home_id);
}

void test_attribute_store_create_home_id_helper()
{
  // Set our HomeID properly, based on my_unid_1
  zwave_home_id_t home_id;
  zwave_unid_to_home_id(my_unid_1, &home_id);
  zwave_unid_set_home_id(home_id);

  attribute_store_node_t created_node_1
    = attribute_store_network_helper_create_home_id_node(my_unid_1);

  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, created_node_1);

  attribute_store_node_t created_node_2
    = attribute_store_network_helper_create_home_id_node(my_unid_2);

  TEST_ASSERT_EQUAL_PTR(created_node_1, created_node_2);

  created_node_2
    = attribute_store_network_helper_create_home_id_node(my_unid_3);
  TEST_ASSERT_FALSE(created_node_1 == created_node_2);

  // Now switch back to the test network HomeID
  zwave_unid_to_home_id(my_unid_test_network_expected, &home_id);
  zwave_unid_set_home_id(home_id);

  // Now pass on an HomeID that should already exist: (from the test network)
  created_node_2 = attribute_store_network_helper_create_home_id_node(
    my_unid_test_network_expected);
  TEST_ASSERT_EQUAL_PTR(created_node_2, home_id_expected);
}

void test_attribute_store_create_node_id_helper()
{
  // Set our HomeID properly, based on my_unid_1
  zwave_home_id_t home_id;
  zwave_unid_to_home_id(my_unid_1, &home_id);
  zwave_unid_set_home_id(home_id);

  attribute_store_node_t created_node_1
    = attribute_store_network_helper_create_node_id_node(my_unid_1);

  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, created_node_1);

  attribute_store_node_t created_node_2
    = attribute_store_network_helper_create_node_id_node(my_unid_2);

  TEST_ASSERT_FALSE(created_node_1 == created_node_2);

  created_node_2
    = attribute_store_network_helper_create_node_id_node(my_unid_1);
  TEST_ASSERT_EQUAL_PTR(created_node_1, created_node_2);

  // Now switch back to the test network HomeID
  zwave_unid_to_home_id(my_unid_test_network_expected, &home_id);
  zwave_unid_set_home_id(home_id);

  // Now pass on an HomeID/nodeID that should already exist: (from the test network)
  created_node_2 = attribute_store_network_helper_create_node_id_node(
    my_unid_test_network_expected);

  TEST_ASSERT_EQUAL_PTR(created_node_2, node_id_expected);
}

void test_attribute_store_create_endpoint_helper()
{
  // Set our HomeID properly, based on my_unid_1
  zwave_home_id_t home_id;
  zwave_unid_to_home_id(my_unid_1, &home_id);
  zwave_unid_set_home_id(home_id);

  attribute_store_node_t created_node_1
    = attribute_store_network_helper_create_endpoint_node(my_unid_1, 0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, created_node_1);

  attribute_store_node_t created_node_2
    = attribute_store_network_helper_create_endpoint_node(my_unid_1, 0);
  TEST_ASSERT_EQUAL_PTR(created_node_1, created_node_2);

  created_node_2
    = attribute_store_network_helper_create_endpoint_node(my_unid_2, 0);
  TEST_ASSERT_FALSE(created_node_1 == created_node_2);

  created_node_1
    = attribute_store_network_helper_create_endpoint_node(my_unid_2, 1);
  TEST_ASSERT_FALSE(created_node_1 == created_node_2);

  // Now switch back to the test network HomeID
  zwave_unid_to_home_id(my_unid_test_network_expected, &home_id);
  zwave_unid_set_home_id(home_id);

  // Now pass on an HomeID/nodeID endpoint that should already exist: (from the test network)
  created_node_2 = attribute_store_network_helper_create_endpoint_node(
    my_unid_test_network_expected,
    5);

  TEST_ASSERT_EQUAL_PTR(created_node_2, endpoint_expected);

  // Try again with a different endpoint, it should create a new node now
  created_node_2 = attribute_store_network_helper_create_endpoint_node(
    my_unid_test_network_expected,
    3);
  TEST_ASSERT_FALSE(created_node_2 == endpoint_expected);
}

void test_attribute_store_get_home_id_helper()
{
  // Set our HomeID properly, based on my_unid_1
  zwave_home_id_t home_id;
  zwave_unid_to_home_id(my_unid_1, &home_id);
  zwave_unid_set_home_id(home_id);

  // Create a HomeID node
  attribute_store_node_t created_node_1
    = attribute_store_network_helper_create_home_id_node(my_unid_1);

  // Try to get a non-existing HomeID:
  unid_t non_existing_home_id = "zwBBAABBAA-0123";
  attribute_store_node_t created_node_2
    = attribute_store_network_helper_get_home_id_node(non_existing_home_id);

  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, created_node_2);

  // Try to get an existing HomeID:
  attribute_store_node_t created_node_3
    = attribute_store_network_helper_get_home_id_node(my_unid_1);

  TEST_ASSERT_EQUAL_PTR(created_node_1, created_node_3);
}

void test_attribute_store_get_node_id_helper()
{
  // Set our HomeID properly, based on my_unid_1
  zwave_home_id_t home_id;
  zwave_unid_to_home_id(my_unid_1, &home_id);
  zwave_unid_set_home_id(home_id);

  // Create a NodeID node
  attribute_store_node_t created_node_1
    = attribute_store_network_helper_create_node_id_node(my_unid_1);

  // Try to get a non-existing HomeID:
  unid_t non_existing_node_id = "zwFC23F52D-0123";
  attribute_store_node_t created_node_2
    = attribute_store_network_helper_get_node_id_node(non_existing_node_id);

  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, created_node_2);

  // Try to get an existing NodeID:
  attribute_store_node_t received_node
    = attribute_store_network_helper_get_node_id_node(my_unid_1);

  TEST_ASSERT_EQUAL_PTR(created_node_1, received_node);
}

void test_attribute_store_get_endpoint_id_helper()
{
  // Set our HomeID properly, based on my_unid_1
  zwave_home_id_t home_id;
  zwave_unid_to_home_id(my_unid_1, &home_id);
  zwave_unid_set_home_id(home_id);

  // Create an endpoint node, if does not exist
  attribute_store_node_t created_node_1
    = attribute_store_network_helper_create_endpoint_node(my_unid_1, 43);

  // Try to get a non-existing NodeID / endpoint:
  attribute_store_node_t created_node_2
    = attribute_store_network_helper_get_endpoint_node(my_unid_1, 44);

  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, created_node_2);

  // Try to get an existing NodeID:
  attribute_store_node_t received_node
    = attribute_store_network_helper_get_endpoint_node(my_unid_1, 43);

  TEST_ASSERT_EQUAL_PTR(created_node_1, received_node);
}

void test_attribute_store_get_unid_endpoint_from_node_helper()
{
  // Set our HomeID properly, based on my_unid_test_network_expected
  zwave_home_id_t home_id;
  zwave_unid_to_home_id(my_unid_test_network_expected, &home_id);
  zwave_unid_set_home_id(home_id);

  unid_t unid;
  zwave_endpoint_id_t endpoint_id;

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_network_helper_get_unid_endpoint_from_node(
                      association_group_node,
                      unid,
                      &endpoint_id));

  // Going up, it should have found Node ID AA, endpoint 5
  TEST_ASSERT_EQUAL_STRING(unid, my_unid_test_network_expected);
  TEST_ASSERT_EQUAL(5, endpoint_id);

  // Make the nodeID value undefined:
  attribute_store_set_node_attribute_value(node_id_expected,
                                           REPORTED_ATTRIBUTE,
                                           NULL,
                                           0);

  // Try again, now it will be unhappy about the NodeID:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    attribute_store_network_helper_get_unid_endpoint_from_node(
                      association_group_node,
                      unid,
                      &endpoint_id));
  // Values should not have been assigned (unchanged)
  TEST_ASSERT_EQUAL_STRING(unid, my_unid_test_network_expected);
  TEST_ASSERT_EQUAL(5, endpoint_id);

  // Restore the network: (with the nodeID AA)
  zwave_node_id_t node_id = 0xAA;
  attribute_store_set_node_attribute_value(node_id_expected,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&node_id,
                                           sizeof(node_id));

  // Try with a invalid node:
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    attribute_store_network_helper_get_unid_endpoint_from_node(
                      ATTRIBUTE_STORE_INVALID_NODE,
                      unid,
                      &endpoint_id));

  // Try with the wrong HomeID, it should accept:
  home_id = home_id + 3;
  attribute_store_set_reported(home_id_expected, &home_id, sizeof(home_id));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_network_helper_get_unid_endpoint_from_node(
                      association_group_node,
                      unid,
                      &endpoint_id));
}

void test_attribute_store_get_unid_from_node_helper()
{
  // Set our HomeID properly, based on my_unid_test_network_expected
  zwave_home_id_t home_id;
  zwave_unid_to_home_id(my_unid_test_network_expected, &home_id);
  zwave_unid_set_home_id(home_id);

  unid_t unid;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_network_helper_get_unid_from_node(association_group_node,
                                                      unid));

  // Going up, it should have found Node ID AA, endpoint 5
  TEST_ASSERT_EQUAL_STRING(unid, my_unid_test_network_expected);

  // Should also work for NodeID node
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_network_helper_get_unid_from_node(node_id_expected, unid));
  TEST_ASSERT_EQUAL_STRING(unid, my_unid_test_network_expected);

  // Same thing for the endpoint node
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_network_helper_get_unid_from_node(endpoint_expected, unid));
  TEST_ASSERT_EQUAL_STRING(unid, my_unid_test_network_expected);

  // Make the nodeID value undefined:
  attribute_store_set_node_attribute_value(node_id_expected,
                                           REPORTED_ATTRIBUTE,
                                           NULL,
                                           0);

  // Try again, now it will be unhappy about the NodeID:
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_network_helper_get_unid_from_node(association_group_node,
                                                      unid));
  // Values should not have been assigned (unchanged)
  TEST_ASSERT_EQUAL_STRING(unid, my_unid_test_network_expected);

  // Restore the network: (with the nodeID AA)
  zwave_node_id_t node_id = 0xAA;
  attribute_store_set_node_attribute_value(node_id_expected,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&node_id,
                                           sizeof(node_id));
  // Try with a invalid node:
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    attribute_store_network_helper_get_unid_from_node(
                      ATTRIBUTE_STORE_INVALID_NODE,
                      unid));

  // Try with the wrong HomeID, it should accept
  home_id = home_id + 3;
  attribute_store_set_reported(home_id_expected, &home_id, sizeof(home_id));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_network_helper_get_unid_from_node(association_group_node,
                                                      unid));
}

void test_attribute_store_network_helper_get_node_id_from_node()
{
  // Set our HomeID properly, based on my_unid_test_network_expected
  zwave_home_id_t home_id;
  zwave_unid_to_home_id(my_unid_test_network_expected, &home_id);
  zwave_unid_set_home_id(home_id);

  zwave_node_id_t received_node_id = 0;
  zwave_node_id_t expected_node_id = 0xFF;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_network_helper_get_node_id_from_node(association_group_node,
                                                         &received_node_id));
  attribute_store_read_value(node_id_expected,
                             REPORTED_ATTRIBUTE,
                             &expected_node_id,
                             sizeof(zwave_node_id_t));

  TEST_ASSERT_EQUAL(expected_node_id, received_node_id);

  // Try again with a node that has no NodeID above itself.
  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_FOUND,
    attribute_store_network_helper_get_node_id_from_node(home_id_expected,
                                                         &received_node_id));

  // Try with the NodeID, that has no value
  attribute_store_set_node_attribute_value(node_id_expected,
                                           REPORTED_ATTRIBUTE,
                                           NULL,
                                           0);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    attribute_store_network_helper_get_node_id_from_node(
                      binary_switch_value_node,
                      &received_node_id));
  // Restore the NodeID value of our test network
  attribute_store_set_node_attribute_value(node_id_expected,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&expected_node_id,
                                           sizeof(zwave_node_id_t));
}

void test_attribute_store_network_helper_get_endpoint_id_from_node()
{
  // Set our HomeID properly, based on my_unid_test_network_expected
  zwave_home_id_t home_id;
  zwave_unid_to_home_id(my_unid_test_network_expected, &home_id);
  zwave_unid_set_home_id(home_id);

  zwave_endpoint_id_t received_endpoint_id = 0;
  zwave_endpoint_id_t expected_endpoint_id = 0xFF;

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_network_helper_get_endpoint_id_from_node(
                      association_group_node,
                      &received_endpoint_id));
  attribute_store_read_value(endpoint_expected,
                             REPORTED_ATTRIBUTE,
                             &expected_endpoint_id,
                             sizeof(zwave_endpoint_id_t));

  TEST_ASSERT_EQUAL(expected_endpoint_id, received_endpoint_id);

  // Try again with a node that has no endpoint above itself.
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    attribute_store_network_helper_get_endpoint_id_from_node(
                      home_id_expected,
                      &received_endpoint_id));

  // Try with the endpoint reported value not matching
  attribute_store_set_node_attribute_value(endpoint_expected,
                                           REPORTED_ATTRIBUTE,
                                           NULL,
                                           0);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    attribute_store_network_helper_get_endpoint_id_from_node(
                      binary_switch_value_node,
                      &received_endpoint_id));
  // restore the value
  attribute_store_set_node_attribute_value(endpoint_expected,
                                           REPORTED_ATTRIBUTE,
                                           &expected_endpoint_id,
                                           sizeof(zwave_endpoint_id_t));
}

void test_attribute_store_network_helper_get_zwave_ids_from_node()
{
  // Set our HomeID properly, based on my_unid_test_network_expected
  zwave_home_id_t home_id;
  zwave_unid_to_home_id(my_unid_test_network_expected, &home_id);
  zwave_unid_set_home_id(home_id);

  zwave_endpoint_id_t received_endpoint_id = 0;
  zwave_node_id_t received_node_id         = 0;
  zwave_endpoint_id_t expected_endpoint_id = 0xFF;
  zwave_node_id_t expected_node_id         = 0xFF;

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_network_helper_get_zwave_ids_from_node(
                      association_group_node,
                      &received_node_id,
                      &received_endpoint_id));

  // Read the expected values from our test network
  attribute_store_read_value(node_id_expected,
                             REPORTED_ATTRIBUTE,
                             &expected_node_id,
                             sizeof(zwave_node_id_t));
  attribute_store_read_value(endpoint_expected,
                             REPORTED_ATTRIBUTE,
                             &expected_endpoint_id,
                             sizeof(zwave_endpoint_id_t));

  TEST_ASSERT_EQUAL(expected_endpoint_id, received_endpoint_id);
  TEST_ASSERT_EQUAL(expected_node_id, received_node_id);

  // Try again with a node that has no Endpoint/NodeID above itself.
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    attribute_store_network_helper_get_zwave_ids_from_node(
                      home_id_expected,
                      &received_node_id,
                      &received_endpoint_id));

  // Try with an undefined endpoint value
  attribute_store_set_node_attribute_value(endpoint_expected,
                                           REPORTED_ATTRIBUTE,
                                           NULL,
                                           0);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    attribute_store_network_helper_get_zwave_ids_from_node(
                      binary_switch_value_node,
                      &received_node_id,
                      &received_endpoint_id));
  attribute_store_set_node_attribute_value(endpoint_expected,
                                           REPORTED_ATTRIBUTE,
                                           &expected_endpoint_id,
                                           sizeof(zwave_endpoint_id_t));

  // Try with an undefined NodeID value
  attribute_store_set_node_attribute_value(node_id_expected,
                                           REPORTED_ATTRIBUTE,
                                           NULL,
                                           0);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    attribute_store_network_helper_get_zwave_ids_from_node(
                      binary_switch_value_node,
                      &received_node_id,
                      &received_endpoint_id));
  // Restore the NodeID value of our test network
  attribute_store_set_node_attribute_value(node_id_expected,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&expected_node_id,
                                           sizeof(zwave_node_id_t));
}

static void create_test_network()
{
  zwave_node_id_t node_id         = 1;
  zwave_endpoint_id_t endpoint_id = 1;
  uint8_t test_values[]           = {0x01, 0x02, 0x03, 0x04};
  /////////////////////////////////////////////
  // Create a network, all tests rely on this network
  /////////////////////////////////////////////
  attribute_store_node_t root_node;
  root_node = attribute_store_get_root();

  home_id_expected = attribute_store_add_node(ATTRIBUTE_HOME_ID, root_node);

  attribute_store_set_node_attribute_value(home_id_expected,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&test_network_home_id,
                                           sizeof(zwave_home_id_t));

  node_id_expected
    = attribute_store_add_node(ATTRIBUTE_NODE_ID, home_id_expected);

  node_id = 0xAA;  // This will be NodeID AA
  attribute_store_set_node_attribute_value(node_id_expected,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&node_id,
                                           sizeof(zwave_node_id_t));

  test_values[0] = 0x03;  // We also set a desired value for this guy
  attribute_store_set_node_attribute_value(
    node_id_expected,
    DESIRED_ATTRIBUTE,
    (uint8_t *)&node_id,
    sizeof(zwave_node_id_t));  // 3 bytes, so [0x03, 0x02, 0x03]

  attribute_store_node_t new_node_3;
  new_node_3 = attribute_store_add_node(ATTRIBUTE_NODE_ID, home_id_expected);

  test_values[0] = 0x02;  // This will be NodeID 02
  attribute_store_set_node_attribute_value(new_node_3,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&node_id,
                                           sizeof(zwave_node_id_t));

  endpoint_expected
    = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, node_id_expected);

  // set the network status of
  attribute_store_node_t network_status_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS, node_id_expected);
  uint8_t test_online_network_status = 0x01;
  attribute_store_set_node_attribute_value(network_status_node,
                                           REPORTED_ATTRIBUTE,
                                           &test_online_network_status,
                                           sizeof(test_online_network_status));

  endpoint_id = 0x05;  // This will be endpoint 05
  attribute_store_set_node_attribute_value(endpoint_expected,
                                           REPORTED_ATTRIBUTE,
                                           &endpoint_id,
                                           sizeof(zwave_endpoint_id_t));

  association_group_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                               endpoint_expected);

  test_values[0] = 0x01;  // This will be association group ID 01
  attribute_store_set_node_attribute_value(association_group_node,
                                           REPORTED_ATTRIBUTE,
                                           test_values,
                                           1);

  // Add a binary switch value here.
  binary_switch_value_node = attribute_store_add_node(1111, endpoint_expected);
  test_values[0] = 0x00;  // Binary switch will be desired 0xFF, reported 0x00
  attribute_store_set_node_attribute_value(binary_switch_value_node,
                                           REPORTED_ATTRIBUTE,
                                           test_values,
                                           1);
  test_values[0] = 0xFF;
  attribute_store_set_node_attribute_value(binary_switch_value_node,
                                           DESIRED_ATTRIBUTE,
                                           test_values,
                                           1);
}

void test_attribute_store_network_helper_get_endpoint_0_node()
{
  attribute_store_node_t home_id_node
    = attribute_store_network_helper_get_home_id_node(my_unid_1);

  attribute_store_node_t node_id_node
    = attribute_store_add_node(ATTRIBUTE_NODE_ID, home_id_node);

  // Endpoint 0 does not exist
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                    attribute_store_get_endpoint_0_node(node_id_node));

  attribute_store_node_t endpoint_id_node
    = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, node_id_node);

  // Still not, no reported value.
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                    attribute_store_get_endpoint_0_node(node_id_node));

  const zwave_endpoint_id_t endpoint_0 = 0;
  attribute_store_set_reported(endpoint_id_node,
                               &endpoint_0,
                               sizeof(endpoint_0));
  TEST_ASSERT_EQUAL(endpoint_id_node,
                    attribute_store_get_endpoint_0_node(node_id_node));
}
