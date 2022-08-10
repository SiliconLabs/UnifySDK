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
#include "attribute_timeouts.h"

// Unify includes
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"

// Mock / test helpers
#include "contiki_test_helper.h"

// Static variables
static attribute_store_node_t node_2 = ATTRIBUTE_STORE_INVALID_NODE;
static attribute_store_node_t node_3 = ATTRIBUTE_STORE_INVALID_NODE;
static attribute_store_node_t node_4 = ATTRIBUTE_STORE_INVALID_NODE;
static uint32_t value                = 0;

static void create_test_network()
{
  value  = 2;
  node_2 = attribute_store_add_node(2, attribute_store_get_root());
  attribute_store_set_reported(node_2, &value, sizeof(value));
  attribute_store_set_desired(node_2, &value, sizeof(value));

  value  = 3;
  node_3 = attribute_store_add_node(3, attribute_store_get_root());
  attribute_store_set_reported(node_3, &value, sizeof(value));
  attribute_store_set_desired(node_3, &value, sizeof(value));

  value  = 4;
  node_4 = attribute_store_add_node(4, attribute_store_get_root());
  attribute_store_set_reported(node_4, &value, sizeof(value));
  attribute_store_set_desired(node_4, &value, sizeof(value));
}

static void update_node_reported_value(attribute_store_node_t node)
{
  attribute_store_set_reported(node, &value, sizeof(value));
}

static void update_node_desired_value(attribute_store_node_t node)
{
  attribute_store_set_desired(node, &value, sizeof(value));
}

static void undefine_and_requeue(attribute_store_node_t node)
{
  attribute_store_undefine_reported(node);
  attribute_timeout_set_callback(node, 1000, &undefine_and_requeue);
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
  attribute_timeouts_teardown();
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  node_2 = ATTRIBUTE_STORE_INVALID_NODE;
  node_3 = ATTRIBUTE_STORE_INVALID_NODE;
  node_4 = ATTRIBUTE_STORE_INVALID_NODE;
  create_test_network();
  contiki_test_helper_init();
  attribute_timeouts_init();
}

void test_attribute_timeouts_test()
{
  value = 20;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_timeout_set_callback(node_2, 1000, &update_node_reported_value));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_timeout_set_callback(node_2, 3000, &update_node_desired_value));

  // Get the timer running
  contiki_test_helper_run(0);

  uint32_t read_value = 0;
  attribute_store_get_desired(node_2, &read_value, sizeof(read_value));
  TEST_ASSERT_EQUAL(2, read_value);
  attribute_store_get_reported(node_2, &read_value, sizeof(read_value));
  TEST_ASSERT_EQUAL(2, read_value);

  // Still nothing after 990 ms
  contiki_test_helper_run(990);
  attribute_store_get_desired(node_2, &read_value, sizeof(read_value));
  TEST_ASSERT_EQUAL(2, read_value);
  attribute_store_get_reported(node_2, &read_value, sizeof(read_value));
  TEST_ASSERT_EQUAL(2, read_value);

  // 1 more second, only the reported has been updated
  contiki_test_helper_run(1000);
  attribute_store_get_desired(node_2, &read_value, sizeof(read_value));
  TEST_ASSERT_EQUAL(2, read_value);
  attribute_store_get_reported(node_2, &read_value, sizeof(read_value));
  TEST_ASSERT_EQUAL(value, read_value);

  // 2 more seconds, both value updated
  contiki_test_helper_run(2000);
  attribute_store_get_desired(node_2, &read_value, sizeof(read_value));
  TEST_ASSERT_EQUAL(value, read_value);
  attribute_store_get_reported(node_2, &read_value, sizeof(read_value));
  TEST_ASSERT_EQUAL(value, read_value);
}

void test_attribute_timeouts_test_delete_nodes()
{
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_timeout_set_callback(node_2, 1000, &update_node_reported_value));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_timeout_set_callback(node_2, 3000, &update_node_desired_value));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_timeout_set_callback(node_3,
                                   2000,
                                   &attribute_store_undefine_reported));

  // Get the timer running
  contiki_test_helper_run(1);

  attribute_store_delete_node(node_2);

  // Here the timer should get adapted to the new node list
  contiki_test_helper_run(1);

  // Nothing should happen here, (node 2 timeout):
  contiki_test_helper_run(1000);
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(node_3, REPORTED_ATTRIBUTE));

  // Node 3 gets undefined after its timeout.
  contiki_test_helper_run(1000);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(node_3, REPORTED_ATTRIBUTE));
}

void test_attribute_timeouts_test_cancel_callback()
{
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_timeout_set_callback(node_2, 1000, &update_node_reported_value));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_timeout_set_callback(node_2, 3000, &update_node_desired_value));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_timeout_set_callback(node_3,
                                   2000,
                                   &attribute_store_undefine_reported));

  // Get the timer running
  contiki_test_helper_run(1);

  // Cancel updating the desired value
  TEST_ASSERT_TRUE(attribute_store_is_value_matched(node_2));
  attribute_timeout_cancel_callback(node_2, &update_node_desired_value);

  // Here the timer should get adapted to the new node list
  contiki_test_helper_run(1);
  TEST_ASSERT_TRUE(attribute_store_is_value_matched(node_2));

  // node 2 timeout, value is mismatched now:
  contiki_test_helper_run(1000);
  TEST_ASSERT_FALSE(attribute_store_is_value_matched(node_2));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(node_3, REPORTED_ATTRIBUTE));

  // Node 3 gets undefined after its timeout.
  contiki_test_helper_run(1000);
  TEST_ASSERT_FALSE(attribute_store_is_value_matched(node_2));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(node_3, REPORTED_ATTRIBUTE));
}

void test_attribute_timeouts_requeues_itself()
{
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_timeout_set_callback(node_2, 1000, &undefine_and_requeue));

  // Get the timer running
  contiki_test_helper_run(1);
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(node_2, REPORTED_ATTRIBUTE));

  contiki_test_helper_run(1000);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(node_2, REPORTED_ATTRIBUTE));

  // Redefine it
  attribute_store_set_reported(node_2, &value, sizeof(value));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(node_2, REPORTED_ATTRIBUTE));
  contiki_test_helper_run(1000);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(node_2, REPORTED_ATTRIBUTE));
}

void test_attribute_timeouts_new_queue_while_timer_running()
{
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_timeout_set_callback(node_2,
                                   1000,
                                   &attribute_store_undefine_reported));

  // Get the timer running
  contiki_test_helper_run(1);
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(node_2, REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(node_4, REPORTED_ATTRIBUTE));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_timeout_set_callback(node_4,
                                   100,
                                   &attribute_store_undefine_reported));

  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(node_2, REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(node_4, REPORTED_ATTRIBUTE));

  contiki_test_helper_run(99);
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(node_2, REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(node_4, REPORTED_ATTRIBUTE));

  contiki_test_helper_run(3);
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(node_2, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(node_4, REPORTED_ATTRIBUTE));

  contiki_test_helper_run(880);
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(node_2, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(node_4, REPORTED_ATTRIBUTE));

  contiki_test_helper_run(20);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(node_2, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(node_4, REPORTED_ATTRIBUTE));
}

void test_attribute_timeouts_register_null_pointer()
{
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    attribute_timeout_set_callback(node_2, 1000, NULL));
}

void test_attribute_timeouts_nothing_works_after_teardown()
{
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_timeout_set_callback(node_2,
                                   1000,
                                   &attribute_store_undefine_reported));

  // Get the timer running
  contiki_test_helper_run(1);
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(node_2, REPORTED_ATTRIBUTE));

  // Teardown and see if nothing bad happens.
  attribute_timeouts_teardown();

  contiki_test_helper_run(1000);
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(node_2, REPORTED_ATTRIBUTE));
}

void test_attribute_timeouts_no_duration()
{
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_timeout_set_callback(node_2,
                                   0,
                                   &attribute_store_undefine_reported));

  // Should have immediate effect
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(node_2, REPORTED_ATTRIBUTE));

  // Get the timer running
  contiki_test_helper_run(20);

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(node_2, REPORTED_ATTRIBUTE));
}