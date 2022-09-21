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
#include "attribute_store.h"
#include "attribute_store_fixt.h"
#include "attribute_store_helper.h"
#include "attribute_store_configuration.h"
#include "attribute_store_internal.h"

// Includes from other components
#include "sl_status.h"
#include "sl_log.h"
#include "datastore_fixt.h"

// Generic includes for random numbers
#include <time.h>
#include <stdlib.h>

// Test helpers
#include "clock.h"
#include "contiki_test_helper.h"

// Test includes
#include "unity.h"

// Log tag
#define LOG_TAG "attribute_store_save_to_datastore_test"

// Define for the test
#define NUMBER_OF_NODES               500
#define MAXIMUM_ATTRIBUTE_STORE_DEPTH 6
#define NUMBER_OF_NODES_TO_DELETE     25
#define DATASTORE_FILE                "attribute_store_save_to_datastore_test.db"

// Static test variables
static attribute_store_node_t nodes[NUMBER_OF_NODES]
  = {ATTRIBUTE_STORE_INVALID_NODE};
static attribute_store_node_t nodes_depth[NUMBER_OF_NODES] = {0};
static attribute_store_node_t root_node = ATTRIBUTE_STORE_INVALID_NODE;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  contiki_test_helper_init();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

// Before every test
void setUp()
{
  datastore_fixt_setup(DATASTORE_FILE);
  attribute_store_init();
  contiki_test_helper_run(0);

  // Initialize the Randon Number Generator
  srand(time(NULL));
}

// After every test
void tearDown()
{
  attribute_store_teardown();
  datastore_fixt_teardown();
}

void test_create_attribute_store_and_save_it_to_datastore()
{
  // Configure no runtime save for test speed
  attribute_store_configuration_set_auto_save_cooldown_interval(100);
  attribute_store_configuration_set_auto_save_safety_interval(100);

  // Ensure we start from scratch before creating our test network.
  sl_log_debug(LOG_TAG, "Deleting Attribute Store");
  attribute_store_delete_node(attribute_store_get_root());

  // Create an attribute tree
  sl_log_debug(LOG_TAG,
               "Creating Random tree with size = %d, maximum depth %d",
               NUMBER_OF_NODES,
               MAXIMUM_ATTRIBUTE_STORE_DEPTH);
  root_node      = attribute_store_get_root();
  nodes[0]       = attribute_store_add_node(rand(), root_node);
  nodes_depth[0] = 1;

  for (uint16_t i = 1; i < NUMBER_OF_NODES; i++) {
    // Try to attach the node with a "realistic depth". (MAXIMUM_ATTRIBUTE_STORE_DEPTH).
    attribute_store_node_t parent_node_index = rand() % i;

    if (nodes_depth[parent_node_index] > MAXIMUM_ATTRIBUTE_STORE_DEPTH) {
      // Just put in under node[0]
      parent_node_index = 0;
    }

    nodes[i]       = attribute_store_add_node(rand(), nodes[parent_node_index]);
    nodes_depth[i] = nodes_depth[parent_node_index] + 1;
    // Set a random value for every 49/50 nodes
    if (rand() % 50 != 0) {
      uint8_t value_size = rand() % 255;
      uint8_t value[value_size];  // save uninitialized garbage
      attribute_store_set_reported(nodes[i], value, value_size);
    }
    if (rand() % 50 != 0) {
      uint8_t value_size = rand() % 255;
      uint8_t value[value_size];  // save uninitialized garbage
      attribute_store_set_desired(nodes[i], value, value_size);
    }
  }

  // Randomly delete some nodes
  sl_log_debug(LOG_TAG,
               "Deleting randomly %d nodes",
               NUMBER_OF_NODES_TO_DELETE);
  for (uint16_t i = 1; i < NUMBER_OF_NODES_TO_DELETE; i++) {
    attribute_store_delete_node(nodes[rand() % NUMBER_OF_NODES]);
  }

  // Find the current size of the attribute store.
  int attribute_store_size
    = attribute_store_get_node_total_child_count(root_node);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, attribute_store_save_to_datastore());

  // Teardown and restart the attribute store, it will reload from the datastore.
  attribute_store_teardown();
  attribute_store_init();

  // Check that everything is here.
  TEST_ASSERT_EQUAL(attribute_store_size,
                    attribute_store_get_node_total_child_count(root_node));
}

void test_set_values_to_root_node_and_restore()
{
  // Make sure that everything is saved instantly
  attribute_store_configuration_set_auto_save_cooldown_interval(0);

  attribute_store_delete_node(attribute_store_get_root());
  uint16_t value = 1005;
  attribute_store_set_desired(attribute_store_get_root(),
                              &value,
                              sizeof(value));
  attribute_store_set_reported(attribute_store_get_root(),
                               &value,
                               sizeof(value));

  attribute_store_node_t node_1
    = attribute_store_add_node(1, attribute_store_get_root());
  int8_t u8_value = -3;
  attribute_store_set_desired(node_1, &u8_value, sizeof(u8_value));
  attribute_store_set_reported(node_1, &u8_value, sizeof(u8_value));

  // Now teardown the datastore, so we can't save anymore.
  datastore_fixt_teardown();
  attribute_store_delete_node(attribute_store_get_root());

  // Reload from datastore
  datastore_fixt_setup(DATASTORE_FILE);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, attribute_store_load_from_datastore());

  // Check that the attribute store is as it should be.
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(attribute_store_get_root(),
                                                    REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(attribute_store_get_root(),
                                                    DESIRED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(node_1, REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(node_1, DESIRED_ATTRIBUTE));
}

void test_auto_save_cooldown_interval_not_reached()
{
  // Reset the network
  attribute_store_delete_node(attribute_store_get_root());

  // Configure to save every second
  const unsigned int test_auto_save_interval = 1;
  attribute_store_configuration_set_auto_save_cooldown_interval(
    test_auto_save_interval);

  // Let's create some network
  attribute_store_node_t node_1
    = attribute_store_add_node(1, attribute_store_get_root());
  contiki_test_helper_run(
    0);  // Get the timer running, a modification was done.
  int8_t u8_value = -3;
  attribute_store_set_desired(node_1, &u8_value, sizeof(u8_value));
  attribute_store_set_reported(node_1, &u8_value, sizeof(u8_value));

  // Miss saving before the next update
  contiki_test_helper_run(test_auto_save_interval * CLOCK_SECOND - 1);

  attribute_store_node_t node_2
    = attribute_store_add_node(2, attribute_store_get_root());
  u8_value = -100;

  contiki_test_helper_run(test_auto_save_interval * CLOCK_SECOND - 1);
  attribute_store_set_desired(node_2, &u8_value, sizeof(u8_value));
  u8_value = 99;
  contiki_test_helper_run(test_auto_save_interval * CLOCK_SECOND - 1);
  attribute_store_set_reported(node_2, &u8_value, sizeof(u8_value));

  attribute_store_node_t node_3 = attribute_store_add_node(3, node_1);
  u8_value                      = 127;
  attribute_store_set_reported(node_3, &u8_value, sizeof(u8_value));
  contiki_test_helper_run(test_auto_save_interval * CLOCK_SECOND - 1);

  attribute_store_node_t node_4 = attribute_store_add_node(4, node_1);
  attribute_store_delete_node(node_4);
  contiki_test_helper_run(test_auto_save_interval * CLOCK_SECOND - 1);

  // Now teardown the datastore, so we can't save anymore.
  datastore_fixt_teardown();
  attribute_store_delete_node(attribute_store_get_root());

  // Reload from datastore, there should be nothing
  datastore_fixt_setup(DATASTORE_FILE);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, attribute_store_load_from_datastore());

  // Check that the attribute store is as it should be.
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(attribute_store_get_root(),
                                                     REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(attribute_store_get_root(),
                                                     DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(attribute_store_node_exists(node_1));
  TEST_ASSERT_FALSE(attribute_store_node_exists(node_2));
  TEST_ASSERT_FALSE(attribute_store_node_exists(node_3));
  TEST_ASSERT_FALSE(attribute_store_node_exists(node_4));
}

void test_auto_save_cooldown_interval_reached()
{
  // Reset the network
  attribute_store_delete_node(attribute_store_get_root());

  // Configure to save every 3 seconds
  const unsigned int test_auto_save_interval = 3;
  attribute_store_configuration_set_auto_save_cooldown_interval(
    test_auto_save_interval);

  // Let's create some network
  attribute_store_delete_node(attribute_store_get_root());
  uint16_t value = 1005;
  attribute_store_set_desired(attribute_store_get_root(),
                              &value,
                              sizeof(value));
  attribute_store_set_reported(attribute_store_get_root(),
                               &value,
                               sizeof(value));

  attribute_store_node_t node_1
    = attribute_store_add_node(1, attribute_store_get_root());
  int8_t u8_value = -3;
  attribute_store_set_desired(node_1, &u8_value, sizeof(u8_value));
  attribute_store_set_reported(node_1, &u8_value, sizeof(u8_value));

  attribute_store_node_t node_2
    = attribute_store_add_node(2, attribute_store_get_root());
  u8_value = -100;
  attribute_store_set_desired(node_2, &u8_value, sizeof(u8_value));
  u8_value = 99;
  attribute_store_set_reported(node_2, &u8_value, sizeof(u8_value));

  attribute_store_node_t node_3 = attribute_store_add_node(3, node_1);
  u8_value                      = 127;
  attribute_store_set_reported(node_3, &u8_value, sizeof(u8_value));

  attribute_store_node_t node_4 = attribute_store_add_node(4, node_1);
  attribute_store_delete_node(node_4);

  // Get the auto-save interval to kick and save to the datastore.
  contiki_test_helper_run(test_auto_save_interval * CLOCK_SECOND + 1);

  // Now teardown the datastore, so we can't save anymore.
  datastore_fixt_teardown();
  attribute_store_delete_node(attribute_store_get_root());

  // Reload from datastore, there should be nothing
  datastore_fixt_setup(DATASTORE_FILE);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, attribute_store_load_from_datastore());

  // Check that the attribute store is as it should be.
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(attribute_store_get_root(),
                                                    REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(attribute_store_get_root(),
                                                    DESIRED_ATTRIBUTE));
  TEST_ASSERT_TRUE(attribute_store_node_exists(node_1));
  TEST_ASSERT_TRUE(attribute_store_node_exists(node_2));
  TEST_ASSERT_TRUE(attribute_store_node_exists(node_3));
  // We created and deleted node 4
  TEST_ASSERT_FALSE(attribute_store_node_exists(node_4));

  // Check that the attribute store is as it should be.
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(node_1, REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(node_1, DESIRED_ATTRIBUTE));
  attribute_store_get_reported(node_1, &u8_value, sizeof(u8_value));
  TEST_ASSERT_EQUAL(-3, u8_value);
  u8_value = 0;
  attribute_store_get_desired(node_1, &u8_value, sizeof(u8_value));
  TEST_ASSERT_EQUAL(-3, u8_value);

  attribute_store_get_reported(node_2, &u8_value, sizeof(u8_value));
  TEST_ASSERT_EQUAL(99, u8_value);
  u8_value = 0;
  attribute_store_get_desired(node_2, &u8_value, sizeof(u8_value));
  TEST_ASSERT_EQUAL(-100, u8_value);

  attribute_store_get_reported(node_3, &u8_value, sizeof(u8_value));
  TEST_ASSERT_EQUAL(127, u8_value);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(node_3, DESIRED_ATTRIBUTE));
}

void test_auto_save_after_create_update_and_delete()
{
  // Reset the network
  attribute_store_delete_node(attribute_store_get_root());

  // Configure to save every 3 seconds
  const unsigned int test_auto_save_interval = 6;
  attribute_store_configuration_set_auto_save_cooldown_interval(
    test_auto_save_interval);

  // Let's create some network
  attribute_store_node_t node_1
    = attribute_store_add_node(1, attribute_store_get_root());
  contiki_test_helper_run(1);

  int32_t value = -3;
  attribute_store_set_desired(node_1, &value, sizeof(value));
  contiki_test_helper_run(1);

  value = -100;
  attribute_store_set_reported(node_1, &value, sizeof(value));
  contiki_test_helper_run(1);

  attribute_store_delete_node(node_1);
  contiki_test_helper_run(1);

  // Get the auto-save interval to kick and save to the datastore, nothing should happen.
  contiki_test_helper_run(test_auto_save_interval * CLOCK_SECOND);

  // Reload from datastore, there should be nothing new
  TEST_ASSERT_EQUAL(SL_STATUS_OK, attribute_store_load_from_datastore());
  TEST_ASSERT_FALSE(attribute_store_node_exists(node_1));
}

void test_auto_save_safety_timer_not_reached()
{
  // Reset the network
  attribute_store_delete_node(attribute_store_get_root());

  // Configure to save every 1 second, safety timer at 15 seconds
  attribute_store_configuration_set_auto_save_cooldown_interval(1);
  attribute_store_configuration_set_auto_save_safety_interval(15);
  contiki_test_helper_run(0);  // Get the safety timer running

  // Let's create some network
  attribute_store_node_t node_1
    = attribute_store_add_node(1, attribute_store_get_root());

  int32_t value = -3;
  attribute_store_set_desired(node_1, &value, sizeof(value));

  for (int i = 0; i < 15; i++) {
    // keep changing before reaching the cooldown
    contiki_test_helper_run(999);
    value++;
    attribute_store_set_desired(node_1, &value, sizeof(value));
  }

  // We have reached neither cooldown or safety save
  // Now teardown the datastore, so we can't save anymore.
  datastore_fixt_teardown();
  attribute_store_delete_node(attribute_store_get_root());

  // Reload from datastore, there should be nothing
  datastore_fixt_setup(DATASTORE_FILE);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, attribute_store_load_from_datastore());

  // Check that the attribute store is as it should be.
  TEST_ASSERT_FALSE(attribute_store_node_exists(node_1));
}

void test_auto_save_safety_timer_reached()
{
  // Reset the network
  attribute_store_delete_node(attribute_store_get_root());

  // Configure to save every 1 second, safety timer at 15 seconds
  attribute_store_configuration_set_auto_save_cooldown_interval(1);
  attribute_store_configuration_set_auto_save_safety_interval(15);
  contiki_test_helper_run(0);  // Get the safety timer running

  // Let's create some network
  attribute_store_node_t node_1
    = attribute_store_add_node(1, attribute_store_get_root());

  int32_t value = -3;
  attribute_store_set_reported(node_1, &value, sizeof(value));

  for (int i = 0; i < 15; i++) {
    // keep changing before reaching the cooldown
    contiki_test_helper_run(999);
    value++;
    attribute_store_set_desired(node_1, &value, sizeof(value));
  }

  // Change one more time and reach the safety timer:
  value++;
  attribute_store_set_desired(node_1, &value, sizeof(value));
  contiki_test_helper_run(999);

  // We have reached safety save
  // Now teardown the datastore, so we can't save anymore.
  datastore_fixt_teardown();
  attribute_store_delete_node(attribute_store_get_root());

  // Reload from datastore, there should be nothing
  datastore_fixt_setup(DATASTORE_FILE);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, attribute_store_load_from_datastore());

  // Check that the attribute store is as it should be.
  TEST_ASSERT_TRUE(attribute_store_node_exists(node_1));
  attribute_store_get_reported(node_1, &value, sizeof(value));
  TEST_ASSERT_EQUAL(-3, value);
  attribute_store_get_desired(node_1, &value, sizeof(value));
  TEST_ASSERT_EQUAL(13, value);
}