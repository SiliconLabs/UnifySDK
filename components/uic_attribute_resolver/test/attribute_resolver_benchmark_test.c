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

#include "unity.h"
#include "contiki_test_helper.h"
#include "process.h"
#include "attribute_store_fixt.h"
#include "datastore_fixt.h"
#include "attribute_resolver.h"
#include "attribute_resolver_rule_internal_mock.h"
#include "sl_log.h"
#include "attribute_store.h"
#include "datastore.h"
#include "config.h"
#include <time.h>
#include <string.h>

#define LOG_TAG "attribute_resolver_benchmark_test"
#define DB_FILENAME "attribute_resolver_benchmark_test.db"
#define ATTRIBUTE_HOME_ID 2

typedef enum {
  RESOLVER_NEXT_EVENT,
  RESOLVER_PAUSE_EVENT,
  RESOLVER_RESUME_EVENT,
  RESOLVER_WATCH_EVENT,
  RESOLVER_TIMER_SET_EVENT,
} attribute_resolver_worker_event_t;

extern struct process attribute_resolver_process;
static int total_nodes = 1;

sl_status_t attribute_resolver_send(attribute_store_node_t node,
                                    const uint8_t *frame_data,
                                    uint16_t frame_data_len,
                                    bool is_set)
{
  return SL_STATUS_OK;
}

attribute_resolver_config_t attribute_resolver_config = {
    .send_init = NULL,
    .send      = attribute_resolver_send,
    .get_retry_timeout = 20000,
    .get_retry_count = 3
};

void suiteSetUp()
{
  char *argv_inject[3]
    = {"attribute_resolver_benchmark_test", "--datastore.file", DB_FILENAME};
  config_parse(sizeof(argv_inject) / sizeof(char *), argv_inject, "");
  datastore_fixt_setup();
  attribute_store_init();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_resolver_teardown();
  attribute_store_teardown();
  datastore_fixt_teardown();
  remove(DB_FILENAME);
  return num_failures;
}

void generate_tree(int depth, int branches, attribute_store_node_t node)
{
  if (depth == 0) {
    return;
  }
  for (int i = 0; i < branches; i++) {
    attribute_store_node_t endpoint
      = attribute_store_add_node(ATTRIBUTE_HOME_ID, node);
    total_nodes++;
    generate_tree(depth - 1, branches, endpoint);
  }
}

void create_test_network(int depth,
                         int number_of_nodes,
                         attribute_store_node_t node)
{
  // Add home ID node
  attribute_store_node_t home_id_node;
  attribute_store_node_t root_node = attribute_store_get_root();
  home_id_node = attribute_store_add_node(ATTRIBUTE_HOME_ID, root_node);
  total_nodes++;

  for (int i = 1; i <= number_of_nodes; i++) {
    attribute_store_node_t node
      = attribute_store_add_node(ATTRIBUTE_HOME_ID, home_id_node);
    total_nodes++;
    generate_tree(depth - 2, 4, node);
  }
}

void setUp()
{
  attribute_store_delete_node(attribute_store_get_root());
}


void test_attribute_resolver_benchmark()
{
  // This has approximately 87000 attribute nodes
  create_test_network(4, 4, attribute_store_get_root());
  attribute_resolver_rule_init_Expect(NULL);
  attribute_resolver_rule_init_IgnoreArg_compl_func();
  attribute_resolver_rule_execute_IgnoreAndReturn(SL_STATUS_OK);
  attribute_resolver_rule_busy_IgnoreAndReturn(false);
  attribute_resolver_has_get_rule_IgnoreAndReturn(true);
  attribute_resolver_has_set_rule_IgnoreAndReturn(false);
  contiki_test_helper_init();
  clock_t start_time = clock();

  // Init attribute resolver
  attribute_resolver_init(attribute_resolver_config);
  contiki_test_helper_run(0);
  total_nodes = total_nodes - 1;

  // continue resolving the rest
  for (int i = 0;i < total_nodes;i++) {
    attribute_resolver_rule_execute_IgnoreAndReturn(SL_STATUS_OK);
    attribute_resolver_rule_busy_IgnoreAndReturn(false);
    attribute_resolver_has_get_rule_IgnoreAndReturn(true);
    attribute_resolver_has_set_rule_IgnoreAndReturn(false);
    process_post(&attribute_resolver_process, RESOLVER_NEXT_EVENT, NULL);
    contiki_test_helper_run(0);
  }
  double elapsed_time = ((double)(clock() - start_time)) / CLOCKS_PER_SEC;
  sl_log_debug(LOG_TAG, "Elapsed time %f", elapsed_time);
}