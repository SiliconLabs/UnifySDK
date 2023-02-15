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
// Component being tested
#include "attribute_store.h"
#include "attribute_store_fixt.h"
#include "attribute_store_type_registration.h"
#include "attribute_store_configuration.h"
#include "attribute_store_internal.h"

// Other components
#include "datastore_fixt.h"

// Test includes
#include "unity.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_fixt_setup(":memory:");
}

// Before every test, ensure to clean up the datastore file, else it will grow for ever
void setUp()
{
  // This trigger a deletion of everything before each test
  TEST_ASSERT_EQUAL(SL_STATUS_OK, attribute_store_init());
  attribute_store_delete_node(attribute_store_get_root());
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_fixt_teardown();
  return num_failures;
}

void test_uic_attribute_store_configuration_auto_save_teardown_test()
{
  // Start with Auto-Save in the agressive mode (i.e. slow)
  attribute_store_configuration_set_auto_save_cooldown_interval(0);

  // Create a network
  attribute_store_node_t root_node = attribute_store_get_root();
  attribute_store_add_node(2, root_node);
  attribute_store_add_node(3, root_node);
  attribute_store_node_t test_node = attribute_store_add_node(4, root_node);
  uint8_t value                    = 250;
  attribute_store_set_node_attribute_value(test_node,
                                           REPORTED_ATTRIBUTE,
                                           &value,
                                           sizeof(value));

  // Disable auto-save, configure it to save every 20 seconds
  attribute_store_configuration_set_auto_save_cooldown_interval(20);

  value = 199;
  attribute_store_set_node_attribute_value(test_node,
                                           REPORTED_ATTRIBUTE,
                                           &value,
                                           sizeof(value));

  // Reload the attribute store from the datastore
  // the test_node value will be reverted back:
  attribute_store_load_from_datastore();

  uint8_t received_value      = 0;
  uint8_t received_value_size = 0;
  attribute_store_get_node_attribute_value(test_node,
                                           REPORTED_ATTRIBUTE,
                                           &received_value,
                                           &received_value_size);
  TEST_ASSERT_EQUAL(1, received_value_size);
  TEST_ASSERT_EQUAL(250, received_value);

  // Set a new value again:
  value = 122;
  attribute_store_set_node_attribute_value(test_node,
                                           REPORTED_ATTRIBUTE,
                                           &value,
                                           sizeof(value));

  // Teardown and re-init, it will have saved our data in the datastore
  attribute_store_teardown();
  attribute_store_init();

  received_value      = 0;
  received_value_size = 0;
  attribute_store_get_node_attribute_value(test_node,
                                           REPORTED_ATTRIBUTE,
                                           &received_value,
                                           &received_value_size);
  TEST_ASSERT_EQUAL(1, received_value_size);
  TEST_ASSERT_EQUAL(122, received_value);

  attribute_store_delete_node(attribute_store_add_node(349, root_node));
}

void test_attribute_store_mandatory_parent_type_validation()
{
  attribute_store_delete_node(attribute_store_get_root());
  attribute_store_configuration_set_type_validation(true);

  // Register 2 types, type 1 must be under type 2, type 2 can be anywhere.
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_type(1, "type 1", 2, UNKNOWN_STORAGE_TYPE));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_type(2,
                                  "type 2",
                                  ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
                                  UNKNOWN_STORAGE_TYPE));

  // Adding node type 1 under the root should not work.
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                    attribute_store_add_node(1, attribute_store_get_root()));

  attribute_store_node_t node_2
    = attribute_store_add_node(2, attribute_store_get_root());
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, node_2);

  attribute_store_node_t node_2_bis = attribute_store_add_node(2, node_2);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, node_2_bis);

  attribute_store_node_t node_1 = attribute_store_add_node(1, node_2_bis);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, node_1);

  // Disable the type validation, now anything is allowed:
  attribute_store_configuration_set_type_validation(false);
  TEST_ASSERT_NOT_EQUAL(
    ATTRIBUTE_STORE_INVALID_NODE,
    attribute_store_add_node(1, attribute_store_get_root()));
}