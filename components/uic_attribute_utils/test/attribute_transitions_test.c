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
#include "attribute_transitions.h"

// Unify includes
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"

// Mock / test helpers
#include "contiki_test_helper.h"

// Static variables
static attribute_store_node_t value_node = 0;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  contiki_test_helper_init();
  attribute_transitions_init();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_transitions_teardown();
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_attribute_transitions_test()
{
  // Start with a transition on undefined values:
  value_node = attribute_store_add_node(0x23485, attribute_store_get_root());
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    attribute_start_transition(value_node, 1000));

  int32_t value = 0;
  attribute_store_set_reported(value_node, &value, sizeof(value));

  // Desired is still undefined, it won't work.
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    attribute_start_transition(value_node, 1000));

  // Now set the desired correctly.
  value = 100;
  attribute_store_set_desired(value_node, &value, sizeof(value));

  // It should accept a transition 0->100 in 10000 ms
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_start_transition(value_node, 10000));
  contiki_test_helper_run(0);  // Let it start the refresh timer

  // Wait for 2 seconds. We expect to have the value at 20 now.
  contiki_test_helper_run(2000);
  attribute_store_get_reported(value_node, &value, sizeof(value));
  TEST_ASSERT_EQUAL(20, value);

  // Wait for 0.5 seconds. We expect to have the value still at 20 due to the
  // refresh step being larger.
  contiki_test_helper_run(500);
  attribute_store_get_reported(value_node, &value, sizeof(value));
  TEST_ASSERT_EQUAL(20, value);

  // Now restart the transition. 20 -> 100 in 4 seconds
  TEST_ASSERT_EQUAL(SL_STATUS_OK, attribute_start_transition(value_node, 4000));
  contiki_test_helper_run(0);  // Let it start the refresh timerx

  // Wait for 2 seconds. We expect to have the value at 60 now.
  contiki_test_helper_run(2000);
  attribute_store_get_reported(value_node, &value, sizeof(value));
  TEST_ASSERT_EQUAL(60, value);

  // Test the is_transition_ongoing part:
  TEST_ASSERT_TRUE(is_attribute_transition_ongoing(value_node));
  TEST_ASSERT_FALSE(is_attribute_transition_ongoing(0x999));

  // Test the remaining duration, we should have 2 seconds left on the 4 seconds:
  TEST_ASSERT_EQUAL(2000,
                    attribute_transition_get_remaining_duration(value_node));
  contiki_test_helper_run(50);
  TEST_ASSERT_EQUAL(1950,
                    attribute_transition_get_remaining_duration(value_node));

  // Pass the rest of the transition
  contiki_test_helper_run(5000);
  attribute_store_get_reported(value_node, &value, sizeof(value));
  TEST_ASSERT_EQUAL(100, value);
  TEST_ASSERT_EQUAL(0, attribute_transition_get_remaining_duration(value_node));
  TEST_ASSERT_FALSE(is_attribute_transition_ongoing(value_node));
}

void test_attribute_transitions_teardown_during_transition()
{
  // Start with a transition on undefined values:
  value_node    = attribute_store_add_node(0x02, attribute_store_get_root());
  int32_t value = 0;
  attribute_store_set_reported(value_node, &value, sizeof(value));
  value = 100;
  attribute_store_set_desired(value_node, &value, sizeof(value));

  // It should accept a transition 0->100 in 10000 ms
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_start_transition(value_node, 10000));
  contiki_test_helper_run(0);  // Let it start the refresh timer

  // Now teardown the whole thing
  attribute_transitions_teardown();

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(value_node, REPORTED_ATTRIBUTE));

  // No more update should happen.
  contiki_test_helper_run(5000);

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(value_node, REPORTED_ATTRIBUTE));
}

void test_attribute_transitions_node_deleted_during_transition()
{
  // Start with a transition on undefined values:
  value_node    = attribute_store_add_node(0x02, attribute_store_get_root());
  int32_t value = 0;
  attribute_store_set_reported(value_node, &value, sizeof(value));
  value = 100;
  attribute_store_set_desired(value_node, &value, sizeof(value));

  // It should accept a transition 0->100 in 10000 ms
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_start_transition(value_node, 10000));
  contiki_test_helper_run(0);  // Let it start the refresh timer

  // Now delete:
  attribute_store_delete_node(value_node);

  TEST_ASSERT_FALSE(is_attribute_transition_ongoing(value_node));

  // No more update should happen.
  contiki_test_helper_run(5000);

  TEST_ASSERT_FALSE(is_attribute_transition_ongoing(value_node));
}
