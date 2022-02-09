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
#include "aoxpc_attribute_store.h"
#include "aoxpc_attribute_store_network_helper.h"
#include "aoxpc_attribute_store_defined_attribute_types.h"

// Test includes
#include "unity.h"

// UIC includes
#include "datastore.h"
#include "attribute_store_fixt.h"
#include "attribute_store.h"

// Mocks
#include "aoxpc_unid_mock.h"

// Test constants
#define AOXPC_UNID "AoXPCUNID_TeST"

// Static variables
static sl_status_t get_aoxpc_unid_return_value = SL_STATUS_OK;

// Stubs
sl_status_t get_aoxpc_unid_stub(aoa_id_t aoxpc_unid, int cmock_num_calls)
{
  snprintf(aoxpc_unid, sizeof(aoa_id_t), AOXPC_UNID);
  return get_aoxpc_unid_return_value;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

/// Called before each and every test
void SetUp()
{
  // Ensure we start from scratch before creating our test network.
  attribute_store_delete_node(attribute_store_get_root());
}

void test_aoxpc_attribute_store_get_aoxpc_unid_node()
{
  get_aoxpc_unid_Stub(&get_aoxpc_unid_stub);
  get_aoxpc_unid_return_value = SL_STATUS_FAIL;

  // Abort when we can't get the UNID
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, get_aoxpc_unid_node());

  // Nothing happens with an empty attribute store:
  get_aoxpc_unid_return_value = SL_STATUS_OK;
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, get_aoxpc_unid_node());

  // Now create a UNID, but not matching
  const aoa_id_t test_node_unid = "not_matching";
  aoxpc_attribute_store_network_helper_create_unid_node(test_node_unid);

  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, get_aoxpc_unid_node());

  attribute_store_node_t expected_node
    = aoxpc_attribute_store_network_helper_create_unid_node(AOXPC_UNID);
  TEST_ASSERT_EQUAL(expected_node, get_aoxpc_unid_node());
}

void test_aoxpc_attribute_store_get_aoxpc_unid_endpoint()
{
  dotdot_endpoint_id_t test_endpoint_id = 23;
  get_aoxpc_unid_Stub(&get_aoxpc_unid_stub);
  get_aoxpc_unid_return_value = SL_STATUS_FAIL;

  // Abort when we can't get the UNID
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                    get_aoxpc_endpoint_id_node(test_endpoint_id));

  // Nothing happens with an empty attribute store:
  get_aoxpc_unid_return_value = SL_STATUS_OK;
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                    get_aoxpc_endpoint_id_node(test_endpoint_id));

  // Now create a UNID, but no endpoint
  aoxpc_attribute_store_network_helper_create_unid_node(AOXPC_UNID);

  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                    get_aoxpc_endpoint_id_node(test_endpoint_id));

  // Now create an Endpoint, but not matching
  aoxpc_attribute_store_network_helper_create_endpoint_node(AOXPC_UNID,
                                                            test_endpoint_id
                                                              + 2);

  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                    get_aoxpc_endpoint_id_node(test_endpoint_id));

  attribute_store_node_t expected_node
    = aoxpc_attribute_store_network_helper_create_endpoint_node(
      AOXPC_UNID,
      test_endpoint_id);

  TEST_ASSERT_EQUAL(expected_node,
                    get_aoxpc_endpoint_id_node(test_endpoint_id));
}
