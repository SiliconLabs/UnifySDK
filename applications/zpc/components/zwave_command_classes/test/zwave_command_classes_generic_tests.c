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
#include "zwave_command_classes_utils.h"
#include "unity.h"

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "attribute_resolver_mock.h"
#include "zpc_attribute_resolver_mock.h"
#include "zwave_command_handler_mock.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_set_home_id(home_id);
}

/// Called after each and every test
void tearDown()
{
  attribute_store_teardown();
  datastore_teardown();
}

void test_zwave_command_classes_reports_to_follow()
{
  attribute_store_node_t root = attribute_store_get_root();
  TEST_ASSERT_FALSE(has_reports_to_follow(root));
  TEST_ASSERT_EQUAL(0, get_reports_to_follow(root));

  // Create a Report to follow node:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    set_reports_to_follow(ATTRIBUTE_STORE_INVALID_NODE, 4));
  TEST_ASSERT_EQUAL(SL_STATUS_OK, set_reports_to_follow(root, 4));

  // Check if the data is here:
  TEST_ASSERT_TRUE(has_reports_to_follow(root));
  TEST_ASSERT_EQUAL(4, get_reports_to_follow(root));

  // update the value
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    set_reports_to_follow(ATTRIBUTE_STORE_INVALID_NODE, 2));
  TEST_ASSERT_EQUAL(SL_STATUS_OK, set_reports_to_follow(root, 2));

  // Check if the data is here:
  TEST_ASSERT_TRUE(has_reports_to_follow(root));
  TEST_ASSERT_EQUAL(2, get_reports_to_follow(root));
}