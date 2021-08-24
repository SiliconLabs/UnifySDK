/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

// Includes from other components
#include "sl_status.h"
#include "sl_log.h"
#include "config.h"
#include "datastore_fixt.h"

// Generic includes for random numbers
#include <time.h>
#include <stdlib.h>

// Test includes
#include "unity.h"

// Log tag
#define LOG_TAG "attribute_store_benchmark_test"

// Define for the test
#define BENCHMARK_NUMBER_OF_NODES           10000
#define BENCHMARK_NUMBER_OF_NODES_TO_ACCESS 2000
#define MAXIMUM_ATTRIBUTE_STORE_DEPTH       10
#define READ_PASS_CRITERIA                  0.05
#define WRITE_PASS_CRITERIA                 4.0

#define ATTRIBUTE_HOME_ID 101
// Static test variables
// Create a network with 10000 nodes.
static attribute_store_node_t nodes[BENCHMARK_NUMBER_OF_NODES]
  = {ATTRIBUTE_STORE_INVALID_NODE};
static attribute_store_node_t nodes_depth[BENCHMARK_NUMBER_OF_NODES] = {0};
static attribute_store_node_t root_node = ATTRIBUTE_STORE_INVALID_NODE;


/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  // Component initialization
  char *argv_inject[3] = {"attribute_store_benchmark_test","--datastore.file", "attribute_store_benchmark.db"};
  config_parse(sizeof(argv_inject)/sizeof(char *), argv_inject, "");
  datastore_fixt_setup();
  attribute_store_init();
  // Initialize the Randon Number Generator
  srand(time(NULL));

  // Ensure we start from scratch before creating our test network.
  attribute_store_delete_node(attribute_store_get_root());

  // Create a network
  root_node      = attribute_store_get_root();
  nodes[0]       = attribute_store_add_node(ATTRIBUTE_HOME_ID, root_node);
  nodes_depth[0] = 1;

  for (uint16_t i = 1; i < BENCHMARK_NUMBER_OF_NODES; i++) {
    // Try to attach the node with a "realistic depth". (MAXIMUM_ATTRIBUTE_STORE_DEPTH).
    attribute_store_node_t parent_node_index = rand() % i;

    if (nodes_depth[parent_node_index] > MAXIMUM_ATTRIBUTE_STORE_DEPTH) {
      // Just put in under node[0]
      parent_node_index = 0;
    }

    nodes[i]       = attribute_store_add_node(rand(), nodes[parent_node_index]);
    nodes_depth[i] = nodes_depth[parent_node_index] + 1;
  }
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_fixt_teardown();
  return num_failures;
}

void test_attribute_store_write_benchmark()
{
  // Start our clock:
  clock_t start_time = clock();

  // Make some batch modification:
  for (uint16_t i = 0; i < BENCHMARK_NUMBER_OF_NODES_TO_ACCESS; i++) {
    uint8_t target_value[] = {rand() % 255};
    attribute_store_set_node_attribute_value(
      nodes[rand() % BENCHMARK_NUMBER_OF_NODES],
      REPORTED_ATTRIBUTE,
      target_value,
      sizeof(target_value));
  }

  // Stop our clock
  double elapsed_time = ((double)(clock() - start_time)) / CLOCKS_PER_SEC;

  // Show off with the results
  sl_log_info(LOG_TAG,
              "Total time taken to modify %d nodes in an attribute store "
              "containing %d nodes: %f seconds\n",
              BENCHMARK_NUMBER_OF_NODES_TO_ACCESS,
              BENCHMARK_NUMBER_OF_NODES,
              elapsed_time);

  // We pass the test if we used less than WRITE_PASS_CRITERIA seconds:
  TEST_ASSERT_TRUE_MESSAGE(elapsed_time < WRITE_PASS_CRITERIA,
                           "The Attribute Store has become too slow! "
                           "Revert your code immediately, or improve it! :-)");
}

void test_attribute_store_read_benchmark()
{
  // Start our clock:
  clock_t start_time = clock();

  // Make all kind of read operations:
  uint8_t received_values[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];
  uint8_t received_values_size;
  for (uint16_t i = 0; i < BENCHMARK_NUMBER_OF_NODES_TO_ACCESS; i++) {
    attribute_store_get_node_attribute_value(
      nodes[rand() % BENCHMARK_NUMBER_OF_NODES],
      REPORTED_ATTRIBUTE,
      received_values,
      &received_values_size);
  }

  // Stop our clock
  double elapsed_time = ((double)(clock() - start_time)) / CLOCKS_PER_SEC;

  // Show off with the results
  sl_log_info(LOG_TAG,
              "Total time taken to read %d nodes in an attribute store "
              "containing %d nodes: %f seconds\n",
              BENCHMARK_NUMBER_OF_NODES_TO_ACCESS,
              BENCHMARK_NUMBER_OF_NODES,
              elapsed_time);

  // We pass the test if we used less than READ_PASS_CRITERIA seconds:
  TEST_ASSERT_TRUE_MESSAGE(elapsed_time < READ_PASS_CRITERIA,
                           "The Attribute Store has become too slow! "
                           "Revert your code immediately, or improve it! :-)");
}
