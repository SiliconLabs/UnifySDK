/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "aoxpc_attribute_store_network_helper.h"
#include "aoxpc_attribute_store_defined_attribute_types.h"
#include "aoxpc_attribute_store.h"

// Includes from other components
#include "attribute_store_fixt.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "datastore.h"

#include "sl_status.h"
#include "sl_log.h"

// Test includes
#include "unity.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
}

/// Called before each and every test
void setUp()
{
  // Ensure we start from scratch before creating our test network.
  attribute_store_delete_node(attribute_store_get_root());
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

void test_aoxpc_attribute_store_network_helper_create_unid_endpoint_node()
{
  const aoa_id_t node_unid = "ble-TestUnid";

  attribute_store_node_t unid_node
    = aoxpc_attribute_store_network_helper_create_unid_node(node_unid);

  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, unid_node);

  // Verification
  aoa_id_t found_node_unid;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_get_reported_string(unid_node,
                                        (char *)&found_node_unid,
                                        sizeof(aoa_id_t)));

  TEST_ASSERT_EQUAL_STRING(node_unid, found_node_unid);

  dotdot_endpoint_id_t test_endpoint_id = 34;
  attribute_store_node_t endpoint_node
    = aoxpc_attribute_store_network_helper_create_endpoint_node(
      node_unid,
      test_endpoint_id);

  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, endpoint_node);
  TEST_ASSERT_EQUAL(unid_node, attribute_store_get_node_parent(endpoint_node));

  dotdot_endpoint_id_t found_endpoint_id = 0;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_get_reported(endpoint_node,
                                                 &found_endpoint_id,
                                                 sizeof(found_endpoint_id)));
  TEST_ASSERT_EQUAL(test_endpoint_id, found_endpoint_id);

  // Try again will return the same node:
  attribute_store_node_t endpoint_node_again
    = aoxpc_attribute_store_network_helper_create_endpoint_node(
      node_unid,
      test_endpoint_id);
  TEST_ASSERT_EQUAL(endpoint_node, endpoint_node_again);
}

void test_aoxpc_attribute_store_network_helper_get_unid_node()
{
  const aoa_id_t node_unid = "ble-TestUnid";

  attribute_store_node_t unid_node
    = aoxpc_attribute_store_network_helper_get_unid_node(node_unid);

  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, unid_node);

  attribute_store_node_t expected_unid_node
    = aoxpc_attribute_store_network_helper_create_unid_node(node_unid);

  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, expected_unid_node);

  unid_node = aoxpc_attribute_store_network_helper_get_unid_node(node_unid);
  TEST_ASSERT_EQUAL(expected_unid_node, unid_node);
}

void test_aoxpc_attribute_store_network_helper_get_endpoint_node()
{
  const aoa_id_t node_unid              = "ble-TestUnid";
  dotdot_endpoint_id_t test_endpoint_id = 34;

  attribute_store_node_t endpoint_node
    = aoxpc_attribute_store_network_helper_get_endpoint_node(node_unid,
                                                             test_endpoint_id);

  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, endpoint_node);

  attribute_store_node_t expected_endpoint_node
    = aoxpc_attribute_store_network_helper_create_endpoint_node(
      node_unid,
      test_endpoint_id);

  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, expected_endpoint_node);

  endpoint_node
    = aoxpc_attribute_store_network_helper_get_endpoint_node(node_unid,
                                                             test_endpoint_id);
  TEST_ASSERT_EQUAL(expected_endpoint_node, endpoint_node);
}
