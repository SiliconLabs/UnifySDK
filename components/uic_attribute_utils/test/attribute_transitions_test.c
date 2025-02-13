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




void test_attribute_transitions_fixed_transition()
{
  // Start with a transition on undefined values:
  value_node    = attribute_store_add_node(0x02, attribute_store_get_root());
  int32_t value = 100;
  attribute_store_set_reported(value_node, &value, sizeof(value));

  //Check that we can go down and hit the target
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_start_fixed_transition(value_node,REPORTED_ATTRIBUTE,70,-10,100));  
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(90.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(80.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(70.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(70.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));

  //Check that we can go down and hit the target
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_start_fixed_transition(value_node,REPORTED_ATTRIBUTE,100,10,100));  
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(80.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(90.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(100.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(100.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));

  //Check if we are clamping to the target if the step in not an multiple of the diffrence
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_start_fixed_transition(value_node,REPORTED_ATTRIBUTE,90,-8.0,100));  
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(92.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(90.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));

  //Ensure that we always end up on the target
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_start_fixed_transition(value_node,REPORTED_ATTRIBUTE,50, 8.0,100));  
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(50.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));

}


void test_attribute_transitions_fixed_cyclic_transition()
{
    // Start with a transition on undefined values:
  value_node    = attribute_store_add_node(0x03, attribute_store_get_root());
  int32_t value = 100;
  attribute_store_set_reported(value_node, &value, sizeof(value));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_start_fixed_cyclic_transition(value_node,REPORTED_ATTRIBUTE,5,10,0,150,100)); 
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(110.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(120.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(130.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(140.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(150.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(5.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_start_fixed_cyclic_transition(value_node,REPORTED_ATTRIBUTE,0,-10,0,150,100));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(0.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  
  value = 10;
  attribute_store_set_reported(value_node, &value, sizeof(value));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_start_fixed_cyclic_transition(value_node,REPORTED_ATTRIBUTE,135,-7,0,150,100));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(3.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(146.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(139.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(135.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_start_fixed_cyclic_transition(value_node,DESIRED_ATTRIBUTE,0,-7,0,150,100));
  contiki_test_helper_run(100);
  TEST_ASSERT(!is_attribute_transition_ongoing(value_node));
}

void test_attribute_transitions_cyclic_transition()
{
    // Start with a transition on undefined values:
  value_node    = attribute_store_add_node(0x03, attribute_store_get_root());
  int32_t value = 10;
  attribute_store_set_reported(value_node, &value, sizeof(value));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_start_cyclic_transition(value_node,REPORTED_ATTRIBUTE,4,0,15,100)); 
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(14.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(3.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(7.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(11.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(15.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(4.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  attribute_stop_transition(value_node);
  contiki_test_helper_run(100);
  TEST_ASSERT_EQUAL(4.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_start_cyclic_transition(value_node,REPORTED_ATTRIBUTE,-3,0,15,100));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(1.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);  
  TEST_ASSERT_EQUAL(13.0, attribute_store_get_number(value_node, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(100);
  TEST_ASSERT(is_attribute_transition_ongoing(value_node));
}