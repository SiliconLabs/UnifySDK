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
#include "zpc_attribute_store.h"
#include "zpc_attribute_store_network_helper.h"
#include "attribute_store_defined_attribute_types.h"

// Test includes
#include "unity.h"

// UIC includes
#include "datastore.h"
#include "attribute_store_fixt.h"
#include "attribute_store.h"

// Mocks
#include "zwave_network_management_mock.h"

// Static variables
static unid_t my_unid_1          = "zw-FC23F52D-0001";
static uint32_t callback_counter = 0;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();

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

/// Called before each and every test
void SetUp() {}

void callback_test(attribute_store_node_t updated_node,
                   attribute_store_change_t change)
{
  if (change == ATTRIBUTE_UPDATED) {
    callback_counter++;
  }
}

void test_zpc_attribute_store_init()
{
  // Setup HOME ID
  zwave_home_id_t home_id;
  zwave_unid_to_home_id(my_unid_1, &home_id);
  zwave_unid_set_home_id(home_id);
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);

  // The more callbacks you register increment this counter
  uint32_t number_of_callbacks = 1;

  attribute_store_node_t created_node_1
    = attribute_store_network_helper_create_home_id_node(my_unid_1);

  // Return HOME ID for network
  attribute_store_register_callback(callback_test);
  attribute_store_add_node(ATTRIBUTE_NODE_ID, created_node_1);
  attribute_store_log();

  // Will initialize on test_network
  sl_status_t state = zpc_attribute_store_init();
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  // Test that test callback was updated
  TEST_ASSERT_TRUE((number_of_callbacks < callback_counter));
}
