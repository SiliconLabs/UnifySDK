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
#include "attribute.hpp"

#include "unity.h"
#include "attribute_resolver_rule.h"
#include "attribute_resolver_rule_internal.h"
#include "attribute_resolver_rule_internal.hpp"
#include "attribute_resolver.h"
#include "attribute_resolver_internal.h"

// Test helpers
#include "contiki_test_helper.h"

// Includes from other components
#include "attribute_store_fixt.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "datastore.h"
#include "sl_log.h"

extern "C" {

// List of attribute store nodes. node_2 has type 2, node_3 has type 3, etc.
static attribute_store_node_t node_2;
static attribute_store_node_t node_3;
static attribute_store_node_t node_4;
static attribute_store_node_t node_5;
static attribute_store_node_t node_6;
static attribute_store_node_t node_7;
static attribute_store_node_t node_8;
static attribute_store_node_t node_9;
static attribute_store_node_t node_10;

static uint32_t value          = 0;
static uint32_t expected_value = 0;

#define TEST_GET_RETRY_TIMEOUT 23489
#define TEST_GET_RETRY_COUNT   4
#define TEST_TRANSMISSION_TIME 23

static int test_send_init_function_call_count            = 0;
static int test_send_function_call_count                 = 0;
static int test_abort_function_call_count                = 0;
static int test_set_function_call_count                  = 0;
static int test_get_function_call_count                  = 0;
static int test_resolution_listener_function_call_count  = 0;
static int test_set_rule_listener_function_call_count    = 0;
static int test_get_give_up_listener_function_call_count = 0;

static attribute_store_node_t expected_node_for_resolution               = 0;
static attribute_store_node_t expected_node_for_listening_notification   = 0;
static attribute_store_node_t expected_node_for_abort                    = 0;
static attribute_store_type_t expected_type_for_listening_notification   = 0;
static attribute_store_node_t expected_node_for_get_give_up_notification = 0;
static sl_status_t send_function_return_code = SL_STATUS_OK;
static sl_status_t set_function_return_code  = SL_STATUS_OK;
static sl_status_t get_function_return_code  = SL_STATUS_OK;

// Functions registered to init
static void test_send_init_function()
{
  //sl_log_debug("TEST", "send init called\n");
  test_send_init_function_call_count += 1;
}

static sl_status_t test_send_function(attribute_store_node_t node,
                                      const uint8_t *frame_data,
                                      uint16_t frame_data_len,
                                      bool is_set)
{
  //sl_log_debug("TEST", "Send function called %d", node);
  if (expected_node_for_resolution != ATTRIBUTE_STORE_INVALID_NODE) {
    TEST_ASSERT_EQUAL(expected_node_for_resolution, node);
  }
  test_send_function_call_count += 1;
  return send_function_return_code;
}

static sl_status_t test_abort_function(attribute_store_node_t node)
{
  //sl_log_debug("TEST", "Abort called %d", node);
  if (expected_node_for_abort != ATTRIBUTE_STORE_INVALID_NODE) {
    TEST_ASSERT_EQUAL(expected_node_for_abort, node);
  }
  test_abort_function_call_count += 1;
  return SL_STATUS_OK;
}

static sl_status_t test_set_resolution_function(attribute_store_node_t node,
                                                uint8_t *frame,
                                                uint16_t *frame_len)
{
  //sl_log_debug("TEST", "Set resolution function called %d", node);
  *frame_len = 0;
  test_set_function_call_count += 1;
  return set_function_return_code;
}

static sl_status_t test_get_resolution_function(attribute_store_node_t node,
                                                uint8_t *frame,
                                                uint16_t *frame_len)
{
  //sl_log_debug("TEST", "Get resolution function called %d", node);
  *frame_len = 0;
  test_get_function_call_count += 1;
  return get_function_return_code;
}

static void test_resolution_listener_function(attribute_store_node_t node)
{
  //sl_log_debug("TEST", "Resolution listener function called %d", node);
  if (expected_node_for_listening_notification
      != ATTRIBUTE_STORE_INVALID_NODE) {
    TEST_ASSERT_EQUAL(expected_node_for_listening_notification, node);
  }
  test_resolution_listener_function_call_count += 1;
}

static void
  test_set_rule_registration_listener_function(attribute_store_type_t type)
{
  //sl_log_debug("TEST", "Set Rule registration listener function called, type: %d", type);
  if (expected_type_for_listening_notification
      != ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE) {
    TEST_ASSERT_EQUAL(expected_type_for_listening_notification, type);
  }
  test_set_rule_listener_function_call_count += 1;
}

static void test_get_give_up_listener_function(attribute_store_type_t node)
{
  if (expected_node_for_get_give_up_notification
      != ATTRIBUTE_STORE_INVALID_NODE) {
    TEST_ASSERT_EQUAL(expected_node_for_get_give_up_notification, node);
  }
  test_get_give_up_listener_function_call_count += 1;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  contiki_test_helper_init();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_resolver_teardown();
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

void create_test_attribute_store_network()
{
  // root ------
  //  |         |
  //  1-----    8
  //  |     |
  //  2--   3
  //  |  |
  //  4  5
  //  |
  //  6---
  //  |  |
  //  7  9
  // Make sure to start from scratch
  attribute_store_delete_node(attribute_store_get_root());
  // Node 1, under root
  value   = 10;
  node_10 = attribute_store_add_node(10, attribute_store_get_root());
  attribute_store_set_reported(node_10, &value, sizeof(value));
  attribute_store_set_desired(node_10, &value, sizeof(value));

  // Node 2, under node 1
  value  = 2;
  node_2 = attribute_store_add_node(2, node_10);
  attribute_store_set_reported(node_2, &value, sizeof(value));
  attribute_store_set_desired(node_2, &value, sizeof(value));

  // Node 3, under node 1
  value  = 3;
  node_3 = attribute_store_add_node(3, node_10);
  attribute_store_set_reported(node_3, &value, sizeof(value));
  attribute_store_set_desired(node_3, &value, sizeof(value));

  // Node 4, under node 2
  value  = 4;
  node_4 = attribute_store_add_node(4, node_2);
  attribute_store_set_reported(node_4, &value, sizeof(value));
  attribute_store_set_desired(node_4, &value, sizeof(value));

  // Node 5, under node 2
  value  = 5;
  node_5 = attribute_store_add_node(5, node_2);
  attribute_store_set_reported(node_5, &value, sizeof(value));
  attribute_store_set_desired(node_5, &value, sizeof(value));

  // Node 6, under node 4
  value  = 6;
  node_6 = attribute_store_add_node(6, node_4);
  attribute_store_set_reported(node_6, &value, sizeof(value));
  attribute_store_set_desired(node_6, &value, sizeof(value));

  // Node 7, under node 6
  value  = 7;
  node_7 = attribute_store_add_node(7, node_6);
  attribute_store_set_reported(node_7, &value, sizeof(value));
  attribute_store_set_desired(node_7, &value, sizeof(value));

  // Node 8, under the root
  value  = 8;
  node_8 = attribute_store_add_node(8, attribute_store_get_root());
  attribute_store_set_reported(node_8, &value, sizeof(value));
  attribute_store_set_desired(node_8, &value, sizeof(value));

  // Node 9, under node 6
  value  = 9;
  node_9 = attribute_store_add_node(9, node_6);
  attribute_store_set_reported(node_9, &value, sizeof(value));
  attribute_store_set_desired(node_9, &value, sizeof(value));
}

/// Called before each and every test
void setUp()
{
  // Ensure we start from scratch before creating our test network.
  create_test_attribute_store_network();

  test_send_init_function_call_count            = 0;
  test_send_function_call_count                 = 0;
  test_abort_function_call_count                = 0;
  test_set_rule_listener_function_call_count    = 0;
  test_get_give_up_listener_function_call_count = 0;
  expected_node_for_resolution                  = ATTRIBUTE_STORE_INVALID_NODE;
  expected_node_for_listening_notification      = ATTRIBUTE_STORE_INVALID_NODE;
  expected_node_for_abort                       = ATTRIBUTE_STORE_INVALID_NODE;
  expected_node_for_get_give_up_notification    = ATTRIBUTE_STORE_INVALID_NODE;
  send_function_return_code                     = SL_STATUS_OK;
  set_function_return_code                      = SL_STATUS_OK;
  get_function_return_code                      = SL_STATUS_OK;
  test_set_function_call_count                  = 0;
  test_get_function_call_count                  = 0;
  test_resolution_listener_function_call_count  = 0;
  value                                         = 0;
  expected_value                                = 0;
  expected_type_for_listening_notification
    = ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE;

  // Stop the process from the previous test, so we start with fresh state
  attribute_resolver_teardown();

  // Resolver init
  attribute_resolver_config_t config = {};
  config.send_init                   = &test_send_init_function;
  config.send                        = &test_send_function;
  config.abort                       = &test_abort_function;
  config.get_retry_timeout           = TEST_GET_RETRY_TIMEOUT;
  config.get_retry_count             = TEST_GET_RETRY_COUNT;
  attribute_resolver_init(config);

  TEST_ASSERT_EQUAL(1, test_send_init_function_call_count);
  TEST_ASSERT_EQUAL(0, test_send_function_call_count);
  TEST_ASSERT_EQUAL(0, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);
  TEST_ASSERT_EQUAL(0, test_set_rule_listener_function_call_count);
  TEST_ASSERT_EQUAL(0, test_resolution_listener_function_call_count);
  TEST_ASSERT_EQUAL(0, test_abort_function_call_count);

  contiki_test_helper_run(0);
}

void test_attribute_resolver_common_set_rule()
{
  // Register a rule for type 10
  attribute_resolver_register_rule(attribute_store_get_node_type(node_10),
                                   &test_set_resolution_function,
                                   NULL);

  // Regular set resolution.
  value          = 34;
  expected_value = value;
  attribute_store_set_desired(node_10, &value, sizeof(value));

  // Expect a Set resolution
  send_function_return_code    = SL_STATUS_OK;
  set_function_return_code     = SL_STATUS_OK;
  expected_node_for_resolution = node_10;
  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);

  // Simulate a send data complete
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK_EXECUTION_VERIFIED,
                                 0,
                                 node_10,
                                 RESOLVER_SET_RULE);

  attribute_store_get_reported(node_10, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);

  // nothing should happen here
  contiki_test_helper_run(0);
}

void test_attribute_resolver_common_get_rule()
{
  // Register a rule for type 10
  attribute_resolver_register_rule(attribute_store_get_node_type(node_10),
                                   NULL,
                                   &test_get_resolution_function);

  // Regular get resolution.
  attribute_store_undefine_reported(node_10);

  // Expect a get resolution
  send_function_return_code    = SL_STATUS_OK;
  get_function_return_code     = SL_STATUS_OK;
  expected_node_for_resolution = node_10;
  contiki_test_helper_run(1);

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(0, test_set_function_call_count);
  TEST_ASSERT_EQUAL(1, test_get_function_call_count);

  // Simulate a send data complete
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK,
                                 TEST_TRANSMISSION_TIME,
                                 node_10,
                                 RESOLVER_GET_RULE);
  contiki_test_helper_run(1);  // start the timer

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(node_10, REPORTED_ATTRIBUTE));

  // Get was just executed, nothing else will happen.

  // Now pass the retry timer:
  contiki_test_helper_run(TEST_GET_RETRY_TIMEOUT);
  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(0, test_set_function_call_count);
  TEST_ASSERT_EQUAL(1, test_get_function_call_count);
  contiki_test_helper_run(TEST_TRANSMISSION_TIME);
  TEST_ASSERT_EQUAL(2, test_send_function_call_count);
  TEST_ASSERT_EQUAL(0, test_set_function_call_count);
  TEST_ASSERT_EQUAL(2, test_get_function_call_count);

  // Set it back. resolver will remove the pending get from its list.
  value = 10;
  attribute_store_set_reported(node_10, &value, sizeof(value));
  contiki_test_helper_run(1);
}

void test_attribute_resolver_common_get_max_retries()
{
  // Regular get resolution, undefine this before rule registration
  attribute_store_undefine_reported(node_2);

  // I want to get notified if we give up on a get.
  attribute_resolver_set_resolution_give_up_listener(
    2,
    &test_get_give_up_listener_function);

  // Register a rule for type 10
  attribute_resolver_register_rule(attribute_store_get_node_type(node_2),
                                   NULL,
                                   &test_get_resolution_function);

  // Attach a resolution listener on the parent, it should be called when we
  // give up on resolving the get
  expected_node_for_listening_notification = node_10;
  attribute_resolver_set_resolution_listener(
    expected_node_for_listening_notification,
    &test_resolution_listener_function);

  // Expect a get resolution
  send_function_return_code                  = SL_STATUS_OK;
  get_function_return_code                   = SL_STATUS_OK;
  expected_node_for_resolution               = node_2;
  expected_node_for_get_give_up_notification = node_2;

  for (uint8_t attempt = 1; attempt <= TEST_GET_RETRY_COUNT; attempt++) {
    contiki_test_helper_run(1);  // execute the get
    // Simulate a send data complete
    on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK,
                                   TEST_TRANSMISSION_TIME,
                                   expected_node_for_resolution,
                                   RESOLVER_GET_RULE);
    contiki_test_helper_run(1);  // start the timer

    TEST_ASSERT_EQUAL(attempt, test_send_function_call_count);
    TEST_ASSERT_EQUAL(0, test_set_function_call_count);
    TEST_ASSERT_EQUAL(attempt, test_get_function_call_count);
    if (attempt < TEST_GET_RETRY_COUNT) {
      TEST_ASSERT_EQUAL(0, test_resolution_listener_function_call_count);
      TEST_ASSERT_EQUAL(0, test_get_give_up_listener_function_call_count);
    } else {
      TEST_ASSERT_EQUAL(1, test_resolution_listener_function_call_count);
      TEST_ASSERT_EQUAL(1, test_get_give_up_listener_function_call_count);
    }

    TEST_ASSERT_FALSE(
      attribute_store_is_value_defined(expected_node_for_resolution,
                                       REPORTED_ATTRIBUTE));

    // Get was just executed, nothing else will happen.
    contiki_test_helper_run(TEST_GET_RETRY_TIMEOUT);
    contiki_test_helper_run(TEST_TRANSMISSION_TIME);
  }

  // At that point, the resolver should give up, no more tries
  contiki_test_helper_run(1);  // execute the get
  // Simulate a send data complete, which makes no sense.
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK,
                                 TEST_TRANSMISSION_TIME,
                                 expected_node_for_resolution,
                                 RESOLVER_GET_RULE);
  contiki_test_helper_run(1);  // start the timer

  // Listener should have been called:
  TEST_ASSERT_EQUAL(TEST_GET_RETRY_COUNT, test_send_function_call_count);
  TEST_ASSERT_EQUAL(0, test_set_function_call_count);
  TEST_ASSERT_EQUAL(TEST_GET_RETRY_COUNT, test_get_function_call_count);
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(1, test_resolution_listener_function_call_count);
}

void test_attribute_resolver_common_get_node_groups()
{
  // Node 7 and 9 are "siblings", test that they get grouped in a
  // single resolution
  attribute_resolver_set_attribute_depth(attribute_store_get_node_type(node_7),
                                         1);

  // Register a rule for type 7
  attribute_resolver_register_rule(attribute_store_get_node_type(node_7),
                                   NULL,
                                   &test_get_resolution_function);
  // Register the same rule for type 9
  attribute_resolver_register_rule(attribute_store_get_node_type(node_9),
                                   NULL,
                                   &test_get_resolution_function);

  // Get the resolver to resolve:
  attribute_store_undefine_reported(node_7);
  attribute_store_undefine_reported(node_9);

  // Expect a get resolution
  send_function_return_code    = SL_STATUS_OK;
  get_function_return_code     = SL_STATUS_OK;
  expected_node_for_resolution = node_7;

  contiki_test_helper_run(1);  // execute the get
  // Simulate a send data complete
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK,
                                 TEST_TRANSMISSION_TIME,
                                 expected_node_for_resolution,
                                 RESOLVER_GET_RULE);
  contiki_test_helper_run(1);  // start the timer

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(0, test_set_function_call_count);
  TEST_ASSERT_EQUAL(1, test_get_function_call_count);

  value = 7;
  attribute_store_set_reported(node_7, &value, sizeof(value));
  value = 9;
  attribute_store_set_reported(node_9, &value, sizeof(value));
}

void test_attribute_resolver_common_set_node_groups_ok_execution_verified()
{
  // Node 7 and 9 are "siblings", test that they get grouped in a
  // single resolution
  attribute_resolver_set_attribute_depth(attribute_store_get_node_type(node_7),
                                         1);

  // Register a rule for type 7
  attribute_resolver_register_rule(attribute_store_get_node_type(node_7),
                                   &test_set_resolution_function,
                                   NULL);
  // Register the same rule for type 9
  attribute_resolver_register_rule(attribute_store_get_node_type(node_9),
                                   &test_set_resolution_function,
                                   NULL);

  // Get the resolver to resolve:
  value = 1;
  attribute_store_set_desired(node_7, &value, sizeof(value));
  attribute_store_set_desired(node_9, &value, sizeof(value));

  // Expect a get resolution
  send_function_return_code    = SL_STATUS_OK;
  get_function_return_code     = SL_STATUS_OK;
  expected_node_for_resolution = node_7;

  contiki_test_helper_run(1);  // execute the set
  // Simulate a send data complete
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK_EXECUTION_VERIFIED,
                                 TEST_TRANSMISSION_TIME,
                                 expected_node_for_resolution,
                                 RESOLVER_SET_RULE);

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);

  expected_value = 1;
  attribute_store_get_reported(node_7, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);
  attribute_store_get_reported(node_9, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);
}

void test_attribute_resolver_common_set_node_groups_ok_execution_failed()
{
  // Node 7 and 9 are "siblings", test that they get grouped in a
  // single resolution
  attribute_resolver_set_attribute_depth(attribute_store_get_node_type(node_7),
                                         1);

  // Register a rule for type 7
  attribute_resolver_register_rule(attribute_store_get_node_type(node_7),
                                   &test_set_resolution_function,
                                   NULL);
  // Register the same rule for type 9
  attribute_resolver_register_rule(attribute_store_get_node_type(node_9),
                                   &test_set_resolution_function,
                                   NULL);

  // Get the resolver to resolve:
  value = 1;
  attribute_store_set_desired(node_7, &value, sizeof(value));
  attribute_store_set_desired(node_9, &value, sizeof(value));

  // Expect a get resolution
  send_function_return_code    = SL_STATUS_OK;
  get_function_return_code     = SL_STATUS_OK;
  expected_node_for_resolution = node_7;

  contiki_test_helper_run(1);  // execute the set
  // Simulate a send data complete
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK_EXECUTION_FAILED,
                                 TEST_TRANSMISSION_TIME,
                                 expected_node_for_resolution,
                                 RESOLVER_SET_RULE);

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(node_7, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(node_7, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(node_9, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(node_9, REPORTED_ATTRIBUTE));
}

void test_attribute_resolver_multi_listeners()
{
  // Register a rule for type 7
  attribute_resolver_register_rule(attribute_store_get_node_type(node_7),
                                   &test_set_resolution_function,
                                   NULL);
  // Register the another rule for type 9
  attribute_resolver_register_rule(attribute_store_get_node_type(node_9),
                                   NULL,
                                   &test_get_resolution_function);

  attribute_resolver_set_resolution_listener(
    node_2,
    &test_resolution_listener_function);

  attribute_resolver_set_resolution_listener(
    node_10,
    &test_resolution_listener_function);

  // Get the resolver to resolve:
  value = 1;
  attribute_store_set_desired(node_7, &value, sizeof(value));
  attribute_store_undefine_reported(node_9);

  // Expect the set resolution first (node 7)
  send_function_return_code    = SL_STATUS_OK;
  get_function_return_code     = SL_STATUS_OK;
  expected_node_for_resolution = node_7;

  contiki_test_helper_run(1);  // execute the set
  // Simulate a send data complete
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK_EXECUTION_PENDING,
                                 TEST_TRANSMISSION_TIME,
                                 expected_node_for_resolution,
                                 RESOLVER_SET_RULE);

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);
  TEST_ASSERT_EQUAL(0, test_resolution_listener_function_call_count);

  // While 7 is working, resolver will take a stab at resolving node 9
  send_function_return_code    = SL_STATUS_OK;
  get_function_return_code     = SL_STATUS_OK;
  expected_node_for_resolution = node_9;
  contiki_test_helper_run(1);  // execute the get
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK,
                                 TEST_TRANSMISSION_TIME,
                                 expected_node_for_resolution,
                                 RESOLVER_GET_RULE);

  TEST_ASSERT_EQUAL(2, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(1, test_get_function_call_count);

  contiki_test_helper_run(1);  // get to look at the stack again

  TEST_ASSERT_EQUAL(0, test_resolution_listener_function_call_count);

  // Simulate the end of the WORKING state
  // Listeners are told of resolution when get have been tried once, but set
  // have completed their transition.

  // Listener will call Node 2, then Node 10 (lower in the tree first)
  // Simulate a send data complete
  expected_node_for_resolution = node_7;
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK_EXECUTION_VERIFIED,
                                 TEST_TRANSMISSION_TIME,
                                 expected_node_for_resolution,
                                 RESOLVER_SET_RULE);
  expected_node_for_resolution = 0;

  contiki_test_helper_run(1);  // get to look at the stack again

  // Node 9 is still missing a reported value
  TEST_ASSERT_EQUAL(0, test_resolution_listener_function_call_count);

  value = 9;
  attribute_store_set_reported(node_9, &value, sizeof(value));
  contiki_test_helper_run(1);  // get to look at the stack again

  TEST_ASSERT_EQUAL(2, test_resolution_listener_function_call_count);
}

void test_attribute_resolver_pause_nodes()
{
  // Register a rule for type 7
  attribute_resolver_register_rule(attribute_store_get_node_type(node_7),
                                   &test_set_resolution_function,
                                   NULL);
  // Register another rule for type 9
  attribute_resolver_register_rule(attribute_store_get_node_type(node_9),
                                   NULL,
                                   &test_get_resolution_function);

  attribute_resolver_set_resolution_listener(
    node_2,
    &test_resolution_listener_function);

  attribute_resolver_set_resolution_listener(
    node_10,
    &test_resolution_listener_function);

  // Get the resolver to resolve:
  value = 1;
  attribute_store_set_desired(node_7, &value, sizeof(value));
  attribute_store_undefine_reported(node_9);

  // Expect the set resolution first (node 9)
  send_function_return_code    = SL_STATUS_OK;
  get_function_return_code     = SL_STATUS_OK;
  expected_node_for_resolution = node_7;

  contiki_test_helper_run(1);  // execute the set
  // Simulate a send data complete
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK_EXECUTION_PENDING,
                                 TEST_TRANSMISSION_TIME,
                                 expected_node_for_resolution,
                                 RESOLVER_SET_RULE);

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);
  TEST_ASSERT_EQUAL(0, test_resolution_listener_function_call_count);

  // Now pause the resolution of the root node and node 10
  attribute_resolver_pause_node_resolution(attribute_store_get_root());
  attribute_resolver_pause_node_resolution(node_10);

  // nothing should happen when looking at the stack here.
  contiki_test_helper_run(1);
  contiki_test_helper_run(1);

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);
  TEST_ASSERT_EQUAL(0, test_resolution_listener_function_call_count);

  // Attributes get resolved while root node is paused (why not?)
  attribute_store_set_desired_as_reported(node_7);
  value = 9;
  attribute_store_set_reported(node_9, &value, sizeof(value));

  // Paused nodes, so even if resolved, no notification should come
  contiki_test_helper_run(1);  // get to look at the stack again

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);
  TEST_ASSERT_EQUAL(0, test_resolution_listener_function_call_count);

  // Unpause Node 10. It's a trick, the root above is still paused so
  // nothing should happen
  attribute_resolver_resume_node_resolution(node_10);
  contiki_test_helper_run(1);  // get to look at the stack again

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);
  TEST_ASSERT_EQUAL(0, test_resolution_listener_function_call_count);

  // Finally, unpause the root, we should get notified that our nodes are resolved.
  attribute_resolver_resume_node_resolution(attribute_store_get_root());
  contiki_test_helper_run(1);  // get to look at the stack again

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);
  TEST_ASSERT_EQUAL(2, test_resolution_listener_function_call_count);
}

void test_attribute_resolver_pause_nodes_with_single_listener()
{
  // Register a rule for type 7
  attribute_resolver_register_rule(attribute_store_get_node_type(node_7),
                                   &test_set_resolution_function,
                                   NULL);
  // Register the another rule for type 9
  attribute_resolver_register_rule(attribute_store_get_node_type(node_9),
                                   NULL,
                                   &test_get_resolution_function);

  expected_node_for_listening_notification = node_4;
  attribute_resolver_set_resolution_listener(
    expected_node_for_listening_notification,
    &test_resolution_listener_function);

  // Get the resolver to resolve:
  value = 1;
  attribute_store_set_desired(node_7, &value, sizeof(value));
  attribute_store_undefine_reported(node_9);

  // Expect the set resolution first (node 9)
  send_function_return_code    = SL_STATUS_OK;
  get_function_return_code     = SL_STATUS_OK;
  expected_node_for_resolution = node_7;

  contiki_test_helper_run(1);  // execute the set
  // Simulate a send data complete
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_FAIL,
                                 TEST_TRANSMISSION_TIME,
                                 expected_node_for_resolution,
                                 RESOLVER_SET_RULE);

  // Failure will just roll-back the desired.
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(expected_node_for_resolution,
                                     DESIRED_ATTRIBUTE));

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);
  TEST_ASSERT_EQUAL(0, test_resolution_listener_function_call_count);

  // Look at the stack again, now node_9 is up for a get:
  expected_node_for_resolution = node_9;
  contiki_test_helper_run(1);

  TEST_ASSERT_EQUAL(2, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(1, test_get_function_call_count);
  TEST_ASSERT_EQUAL(0, test_resolution_listener_function_call_count);

  // Simulate a send data complete
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK,
                                 TEST_TRANSMISSION_TIME,
                                 expected_node_for_resolution,
                                 RESOLVER_GET_RULE);

  // nothing should happen when looking at the stack here.
  attribute_resolver_state_log();  // unit test coverage
  contiki_test_helper_run(1);

  TEST_ASSERT_EQUAL(2, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(1, test_get_function_call_count);
  TEST_ASSERT_EQUAL(0, test_resolution_listener_function_call_count);

  // Pausing / unpausing should trigger the listeners to get a notification
  // if fully resolved.
  attribute_resolver_pause_node_resolution(node_10);
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(0, test_resolution_listener_function_call_count);

  attribute_resolver_resume_node_resolution(node_10);
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(0, test_resolution_listener_function_call_count);

  // Now simulate an incoming report with the get resolution:
  value = 9;
  attribute_store_set_reported(node_9, &value, sizeof(value));
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(1, test_resolution_listener_function_call_count);

  // Pausing / unpausing should trigger the listeners to get a notification
  // if fully resolved.
  attribute_resolver_pause_node_resolution(node_10);
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(1, test_resolution_listener_function_call_count);

  attribute_resolver_resume_node_resolution(node_10);
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(2, test_resolution_listener_function_call_count);

  // Now toggle the value real quick, it should also say that it got resolved
  attribute_store_undefine_reported(node_9);
  attribute_store_set_reported(node_9, &value, sizeof(value));
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(3, test_resolution_listener_function_call_count);

  // Clear the resolution listener, no more calls:
  attribute_resolver_clear_resolution_listener(
    expected_node_for_listening_notification,
    &test_resolution_listener_function);
  attribute_resolver_pause_node_resolution(node_10);
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(3, test_resolution_listener_function_call_count);

  attribute_resolver_resume_node_resolution(node_10);
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(3, test_resolution_listener_function_call_count);
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(3, test_resolution_listener_function_call_count);
}

void test_attribute_resolver_common_abort_and_restart_set_rule()
{
  // Register a rule for type 10
  attribute_resolver_register_rule(attribute_store_get_node_type(node_10),
                                   &test_set_resolution_function,
                                   NULL);

  // Regular set resolution.
  value          = 34;
  expected_value = value;
  attribute_store_set_desired(node_10, &value, sizeof(value));

  // Expect a Set resolution
  send_function_return_code    = SL_STATUS_OK;
  set_function_return_code     = SL_STATUS_OK;
  expected_node_for_resolution = node_10;
  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);

  // Simulate a send data complete
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK_EXECUTION_PENDING,
                                 0,
                                 node_10,
                                 RESOLVER_SET_RULE);

  // nothing should happen here, we are waiting for the set resolution.
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(0, test_abort_function_call_count);

  TEST_ASSERT_TRUE(is_node_pending_set_resolution(node_10));
  expected_node_for_abort = node_10;
  attribute_resolver_restart_set_resolution(node_10);
  TEST_ASSERT_EQUAL(1, test_abort_function_call_count);

  // Now it will try to resolve again
  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(2, test_send_function_call_count);
  TEST_ASSERT_EQUAL(2, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);
}

void test_attribute_resolver_common_abort_and_restart_get_rule()
{
  // Register a rule for type 10
  attribute_resolver_register_rule(attribute_store_get_node_type(node_10),
                                   NULL,
                                   &test_get_resolution_function);

  // Regular get resolution.
  attribute_store_undefine_reported(node_10);

  // Expect a Set resolution
  send_function_return_code    = SL_STATUS_OK;
  get_function_return_code     = SL_STATUS_OK;
  expected_node_for_resolution = node_10;
  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(0, test_set_function_call_count);
  TEST_ASSERT_EQUAL(1, test_get_function_call_count);

  // Simulate a send data complete
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK,
                                 0,
                                 node_10,
                                 RESOLVER_GET_RULE);

  // nothing should happen here, we are waiting for the set resolution.
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(0, test_abort_function_call_count);

  // Now it will try to resolve again
  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(0, test_set_function_call_count);
  TEST_ASSERT_EQUAL(1, test_get_function_call_count);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_resolver_restart_get_resolution(node_10));

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(0, test_set_function_call_count);
  TEST_ASSERT_EQUAL(1, test_get_function_call_count);

  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(2, test_send_function_call_count);
  TEST_ASSERT_EQUAL(0, test_set_function_call_count);
  TEST_ASSERT_EQUAL(2, test_get_function_call_count);

  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK,
                                 0,
                                 node_10,
                                 RESOLVER_GET_RULE);
  uint8_t value = 1;
  attribute_store_set_reported(node_10, &value, sizeof(value));
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    attribute_resolver_restart_get_resolution(node_10));
}

void test_attribute_resolver_common_set_node_groups_with_restart()
{
  // Node 7 and 9 are "siblings", test that they get grouped in a
  // single resolution
  attribute_resolver_set_attribute_depth(attribute_store_get_node_type(node_7),
                                         1);

  // Register a rule for type 7
  attribute_resolver_register_rule(attribute_store_get_node_type(node_7),
                                   &test_set_resolution_function,
                                   NULL);
  // Register the same rule for type 9
  attribute_resolver_register_rule(attribute_store_get_node_type(node_9),
                                   &test_set_resolution_function,
                                   NULL);

  // Get the resolver to resolve:
  value = 1;
  attribute_store_set_desired(node_7, &value, sizeof(value));
  attribute_store_set_desired(node_9, &value, sizeof(value));

  // Expect a get resolution
  send_function_return_code    = SL_STATUS_OK;
  get_function_return_code     = SL_STATUS_OK;
  expected_node_for_resolution = node_7;

  contiki_test_helper_run(1);  // execute the set
  // Simulate a send data complete
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK_EXECUTION_PENDING,
                                 TEST_TRANSMISSION_TIME,
                                 expected_node_for_resolution,
                                 RESOLVER_SET_RULE);
  contiki_test_helper_run(1);  // look at the stack again

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);
  TEST_ASSERT_EQUAL(0, test_abort_function_call_count);

  // Now we want to restart resolution on node_7 or node_9
  TEST_ASSERT_TRUE(is_node_pending_set_resolution(node_7));
  TEST_ASSERT_FALSE(is_node_pending_set_resolution(node_9));
  expected_node_for_abort = node_7;
  attribute_resolver_restart_set_resolution(node_7);
  TEST_ASSERT_EQUAL(1, test_abort_function_call_count);

  contiki_test_helper_run(1);  // execute the set
  TEST_ASSERT_EQUAL(2, test_send_function_call_count);
  TEST_ASSERT_EQUAL(2, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);
  TEST_ASSERT_EQUAL(1, test_abort_function_call_count);
}

void test_attribute_resolver_custom_supervision_handler()
{
  // Register a rule for type 10
  attribute_resolver_register_rule(attribute_store_get_node_type(node_10),
                                   &test_set_resolution_function,
                                   NULL);

  // Regular set resolution.
  value          = 34;
  expected_value = value;
  attribute_store_set_desired(node_10, &value, sizeof(value));

  // Expect a Set resolution
  send_function_return_code    = SL_STATUS_OK;
  set_function_return_code     = SL_STATUS_OK;
  expected_node_for_resolution = node_10;
  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);

  // Simulate a send data complete
  TEST_ASSERT_TRUE(attribute_resolver_rule_busy());
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_ALREADY_HANDLED,
                                 0,
                                 node_10,
                                 RESOLVER_SET_RULE);
  TEST_ASSERT_FALSE(attribute_resolver_rule_busy());

  // Values haven't changed for node 10
  attribute_store_get_desired(node_10, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);
  expected_value = 10;
  attribute_store_get_reported(node_10, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);

  // Abort or already handled will not trigger a re-resolution on their own
  //(attribute_resolver_restart_set_resolution is needed):
  contiki_test_helper_run(1);
  contiki_test_helper_run(1);
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);
}

void test_attribute_resolver_custom_aborted()
{
  // Register a rule for type 10
  attribute_resolver_register_rule(attribute_store_get_node_type(node_10),
                                   &test_set_resolution_function,
                                   NULL);

  // Regular set resolution.
  value          = 34;
  expected_value = value;
  attribute_store_set_desired(node_10, &value, sizeof(value));

  // Expect a Set resolution
  send_function_return_code    = SL_STATUS_OK;
  set_function_return_code     = SL_STATUS_OK;
  expected_node_for_resolution = node_10;
  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);

  // Simulate a send data complete
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_ABORTED,
                                 0,
                                 node_10,
                                 RESOLVER_SET_RULE);

  // Values haven't changed for node 10
  attribute_store_get_desired(node_10, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);
  expected_value = 10;
  attribute_store_get_reported(node_10, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);

  // Abort or already handled will not trigger a re-resolution on their own
  //(attribute_resolver_restart_set_resolution is needed):
  contiki_test_helper_run(1);
  contiki_test_helper_run(1);
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);
}

void test_attribute_resolver_multi_frame_resolution()
{
  // Register a rule for type 10
  attribute_resolver_register_rule(attribute_store_get_node_type(node_10),
                                   &test_set_resolution_function,
                                   NULL);

  // Regular set resolution.
  value          = 34;
  expected_value = value;
  attribute_store_set_desired(node_10, &value, sizeof(value));

  // Expect a Set resolution
  send_function_return_code    = SL_STATUS_OK;
  set_function_return_code     = SL_STATUS_IN_PROGRESS;
  expected_node_for_resolution = node_10;
  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);

  // Simulate a send data complete
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK_EXECUTION_VERIFIED,
                                 TEST_TRANSMISSION_TIME,
                                 node_10,
                                 RESOLVER_SET_RULE);

  // No value alignment due to multi-frame
  attribute_store_get_desired(node_10, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);
  expected_value = 10;
  attribute_store_get_reported(node_10, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);

  // In progress / multi frame will trigger that we try to resolve again immediately
  contiki_test_helper_run(1);

  TEST_ASSERT_EQUAL(2, test_send_function_call_count);
  TEST_ASSERT_EQUAL(2, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);

  contiki_test_helper_run(1);
}

void test_attribute_resolver_multi_frame_no_supervision()
{
  // Register a rule for type 10
  attribute_resolver_register_rule(attribute_store_get_node_type(node_10),
                                   &test_set_resolution_function,
                                   &test_get_resolution_function);

  // Regular set resolution.
  value          = 34;
  expected_value = value;
  attribute_store_set_desired(node_10, &value, sizeof(value));

  // Expect a Set resolution
  send_function_return_code    = SL_STATUS_OK;
  set_function_return_code     = SL_STATUS_IN_PROGRESS;
  expected_node_for_resolution = node_10;
  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);

  // Simulate a send data complete
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK,
                                 0,
                                 node_10,
                                 RESOLVER_SET_RULE);

  // Execution OK, we undefine the reported no matter what. (multi frame or not)
  attribute_store_get_desired(node_10, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(node_10, REPORTED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(expected_value, value);

  // In progress / multi frame will trigger that we try to resolve again immediately
  // Next in line will be the Get rule (undefined reported)
  contiki_test_helper_run(1);

  TEST_ASSERT_EQUAL(2, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(1, test_get_function_call_count);

  contiki_test_helper_run(1);
}

void test_attribute_resolver_send_data_cannot_send()
{
  // Register a rule for type 10
  attribute_resolver_register_rule(attribute_store_get_node_type(node_10),
                                   &test_set_resolution_function,
                                   &test_get_resolution_function);

  // Regular set resolution.
  value          = 34;
  expected_value = value;
  attribute_store_set_desired(node_10, &value, sizeof(value));

  // Expect a Set resolution
  send_function_return_code    = SL_STATUS_TRANSMIT_BUSY;
  set_function_return_code     = SL_STATUS_OK;
  expected_node_for_resolution = node_10;
  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);

  // Now the resolution stalls.
  // Should we try again spontaneously when a frame could not be prepared ?

  // Values of the attribute should be unchanged.
  attribute_store_get_desired(node_10, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);
  expected_value = 10;
  attribute_store_get_reported(node_10, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);

  // Nothing should happen if we ask the resolver to look at the stack:
  contiki_test_helper_run(1);

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);

  contiki_test_helper_run(1);
}

void test_attribute_resolver_set_rule_status_fails_to_prepare_frame()
{
  // Register a rule for type 10
  attribute_resolver_register_rule(attribute_store_get_node_type(node_10),
                                   &test_set_resolution_function,
                                   &test_get_resolution_function);

  // Regular set resolution.
  value          = 34;
  expected_value = value;
  attribute_store_set_desired(node_10, &value, sizeof(value));

  // Expect a Set resolution
  send_function_return_code    = SL_STATUS_OK;
  set_function_return_code     = SL_STATUS_FAIL;
  expected_node_for_resolution = node_10;
  contiki_test_helper_run(0);

  // Send won't be called due to the lack of ability to create a frame.
  TEST_ASSERT_EQUAL(0, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);

  // Now the resolution stalls.
  // Should we try again spontaneously when a frame could not be prepared ?

  // Values of the attribute should be unchanged.
  attribute_store_get_desired(node_10, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);
  expected_value = 10;
  attribute_store_get_reported(node_10, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);

  // Nothing should happen if we ask the resolver to look at the stack:
  contiki_test_helper_run(1);

  TEST_ASSERT_EQUAL(0, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);

  contiki_test_helper_run(1);
}

void test_attribute_resolver_no_frame_resolution()
{
  // Register a rule for type 10
  attribute_resolver_register_rule(attribute_store_get_node_type(node_10),
                                   &test_set_resolution_function,
                                   &test_get_resolution_function);

  // Regular set resolution.
  value          = 34;
  expected_value = value;
  attribute_store_set_desired(node_10, &value, sizeof(value));

  // Expect a Set resolution
  send_function_return_code    = SL_STATUS_OK;
  set_function_return_code     = SL_STATUS_ALREADY_EXISTS;
  expected_node_for_resolution = node_10;
  contiki_test_helper_run(0);

  // Send won't be called due to this "I don't need a frame to resolve" status
  TEST_ASSERT_EQUAL(0, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);

  // No need for send data callback here.

  // Values of the attribute should be resolved automatically. (in normal use)
  // The resolver won't try to solve it again unless there are attribute updates
  contiki_test_helper_run(1);

  // Expect a re-resolution, if that's not the case.
  attribute_store_refresh_node_and_children_callbacks(node_10);
  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(0, test_send_function_call_count);
  TEST_ASSERT_EQUAL(2, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);

  contiki_test_helper_run(1);

  TEST_ASSERT_EQUAL(0, test_send_function_call_count);
  TEST_ASSERT_EQUAL(2, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);
}

void test_attribute_resolver_node_or_child_need_resolution()
{
  TEST_ASSERT_FALSE(attribute_resolver_node_or_child_needs_resolution(node_10));

  // Still false, value mistmatched but no rule for it.
  value = 34;
  attribute_store_set_desired(node_10, &value, sizeof(value));
  TEST_ASSERT_FALSE(attribute_resolver_node_or_child_needs_resolution(node_10));

  // Register a rule for type 10
  value = 10;
  attribute_store_set_desired(node_10, &value, sizeof(value));
  attribute_resolver_register_rule(attribute_store_get_node_type(node_10),
                                   &test_set_resolution_function,
                                   &test_get_resolution_function);
  TEST_ASSERT_FALSE(attribute_resolver_node_or_child_needs_resolution(node_10));

  // Now we both have a rule and a mismatched value.
  value = 34;
  attribute_store_set_desired(node_10, &value, sizeof(value));
  TEST_ASSERT_TRUE(attribute_resolver_node_or_child_needs_resolution(node_10));
}

void test_attribute_resolver_set_rule_notifications()
{
  // Register a rule before the listener
  attribute_resolver_register_rule(attribute_store_get_node_type(node_10),
                                   &test_set_resolution_function,
                                   &test_get_resolution_function);

  expected_type_for_listening_notification = 10;
  TEST_ASSERT_EQUAL(0, test_set_rule_listener_function_call_count);

  // Register the listener, it should get notified of existing rules.
  attribute_resolver_register_set_rule_listener(
    &test_set_rule_registration_listener_function);
  TEST_ASSERT_EQUAL(1, test_set_rule_listener_function_call_count);

  // Register another rule.
  expected_type_for_listening_notification = 6;
  attribute_resolver_register_rule(attribute_store_get_node_type(node_6),
                                   &test_set_resolution_function,
                                   NULL);
  TEST_ASSERT_EQUAL(2, test_set_rule_listener_function_call_count);

  attribute_resolver_register_rule(attribute_store_get_node_type(node_6),
                                   NULL,
                                   NULL);
  TEST_ASSERT_EQUAL(2, test_set_rule_listener_function_call_count);
}

void test_attribute_resolver_common_unknown_data_in_callback()
{
  // Register a rule for type 10
  attribute_resolver_register_rule(attribute_store_get_node_type(node_10),
                                   &test_set_resolution_function,
                                   &test_get_resolution_function);

  // Regular set resolution.
  value          = 34;
  expected_value = value;
  attribute_store_set_desired(node_10, &value, sizeof(value));

  // Expect a Set resolution
  send_function_return_code    = SL_STATUS_OK;
  set_function_return_code     = SL_STATUS_OK;
  expected_node_for_resolution = node_10;
  contiki_test_helper_run(0);

  // Simulate a weird send data complete
  on_resolver_send_data_complete(
    static_cast<resolver_send_status_t>(0xFF),  // Unknown status
    0,
    node_10,
    static_cast<resolver_rule_type_t>(RESOLVER_SET_RULE));

  // No value change
  expected_value = 10;
  attribute_store_get_reported(node_10, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);
  expected_value = 34;
  attribute_store_get_desired(node_10, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);

  // nothing should happen here
  contiki_test_helper_run(0);

  // Simulate a weird send data complete
  on_resolver_send_data_complete(
    static_cast<resolver_send_status_t>(
      RESOLVER_SEND_STATUS_OK_EXECUTION_VERIFIED),
    0,
    node_10,
    static_cast<resolver_rule_type_t>(0xFF));  // Unknown rule type

  // No value change
  expected_value = 10;
  attribute_store_get_reported(node_10, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);
  expected_value = 34;
  attribute_store_get_desired(node_10, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);

  // Simulate yet another weird send data complete
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK_EXECUTION_VERIFIED,
                                 0,
                                 ATTRIBUTE_STORE_INVALID_NODE,  // Invalid node
                                 RESOLVER_SET_RULE);

  // nothing should happen here
  contiki_test_helper_run(0);
}

void test_attribute_resolver_multi_frame_resolution_with_different_callbacks()
{
  // Register a rule for type 7
  attribute_resolver_register_rule(attribute_store_get_node_type(node_7),
                                   &test_set_resolution_function,
                                   NULL);
  // Register the same rule for type 6
  attribute_resolver_register_rule(attribute_store_get_node_type(node_6),
                                   &test_set_resolution_function,
                                   NULL);

  // Regular set resolution.
  value = 34;
  attribute_store_set_desired(node_7, &value, sizeof(value));
  attribute_store_set_desired(node_6, &value, sizeof(value));

  // Expect a Set resolution, make it multi frame for node 7
  send_function_return_code    = SL_STATUS_OK;
  set_function_return_code     = SL_STATUS_IN_PROGRESS;
  expected_node_for_resolution = node_7;
  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);

  // Actually the resolution send data complete will also align the values of the node
  // if we never got around to send it, so we can call send_data_complete for node_6

  // Simulate a send data complete for node_6. its values should get aligned,
  // because "needs_more_frames" is for node_7
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK_EXECUTION_VERIFIED,
                                 0,
                                 node_6,
                                 RESOLVER_SET_RULE);

  expected_value = 34;
  attribute_store_get_reported(node_6, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(node_6, DESIRED_ATTRIBUTE));

  // Simulate a send data complete for node_7, multi frame:
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK_EXECUTION_VERIFIED,
                                 0,
                                 node_7,
                                 RESOLVER_SET_RULE);

  // No automatic value change for multi frames
  expected_value = 7;
  attribute_store_get_reported(node_7, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);
  expected_value = 34;
  attribute_store_get_desired(node_7, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);

  contiki_test_helper_run(1);
}

void test_attribute_resolver_delete_attribute_while_waiting_for_callback()
{
  // Register a rule for type 7
  attribute_resolver_register_rule(attribute_store_get_node_type(node_7),
                                   &test_set_resolution_function,
                                   NULL);
  // Register the same rule for type 6
  attribute_resolver_register_rule(attribute_store_get_node_type(node_6),
                                   &test_set_resolution_function,
                                   NULL);

  // Regular set resolution.
  value          = 34;
  expected_value = value;
  attribute_store_set_desired(node_7, &value, sizeof(value));
  attribute_store_set_desired(node_6, &value, sizeof(value));

  // Expect a Set resolution, for node 7
  send_function_return_code    = SL_STATUS_OK;
  set_function_return_code     = SL_STATUS_OK;
  expected_node_for_resolution = node_7;
  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(1, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);

  // Now if Node 7 get erased from the attribute store, we should move on
  // without waiting for a callabck!
  attribute_store_delete_node(node_7);

  // Expect a Set resolution, for node 6
  send_function_return_code    = SL_STATUS_OK;
  set_function_return_code     = SL_STATUS_OK;
  expected_node_for_resolution = node_6;
  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(2, test_send_function_call_count);
  TEST_ASSERT_EQUAL(2, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);

  // Simulate a send data complete for node_6:
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK_EXECUTION_VERIFIED,
                                 0,
                                 node_6,
                                 RESOLVER_SET_RULE);

  // Check the automatic value change for node 6:
  attribute_store_get_reported(node_6, &value, sizeof(value));
  TEST_ASSERT_EQUAL(expected_value, value);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(node_6, DESIRED_ATTRIBUTE));

  contiki_test_helper_run(1);

  TEST_ASSERT_EQUAL(2, test_send_function_call_count);
  TEST_ASSERT_EQUAL(2, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);
}

void test_attribute_resolver_common_is_waiting_rule_status()
{
  // Register a rule for type 7
  attribute_resolver_register_rule(attribute_store_get_node_type(node_7),
                                   &test_set_resolution_function,
                                   NULL);
  // Register the same rule for type 6
  attribute_resolver_register_rule(attribute_store_get_node_type(node_6),
                                   &test_set_resolution_function,
                                   NULL);

  // Regular set resolution.
  value = 34;
  attribute_store_set_desired(node_7, &value, sizeof(value));
  attribute_store_set_desired(node_6, &value, sizeof(value));

  // Expect a Set resolution, for node 7. If IS_WAITING, we move on immediately.
  send_function_return_code = SL_STATUS_OK;
  set_function_return_code  = SL_STATUS_IS_WAITING;
  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(0, test_send_function_call_count);
  TEST_ASSERT_EQUAL(2, test_set_function_call_count);
  TEST_ASSERT_EQUAL(0, test_get_function_call_count);
}

void test_attribute_resolver_resolution_notification_on_deletion_while_waiting_for_get()
{
  // Register a rule for type 7
  attribute_resolver_register_rule(attribute_store_get_node_type(node_7),
                                   NULL,
                                   &test_get_resolution_function);

  // Let node 7 need a get resolution:
  attribute_store_undefine_reported(node_7);

  // We have 2 -> 4 -> 6 -> 7
  // Listen for resolution of Node 2
  attribute_resolver_set_resolution_listener(
    node_2,
    &test_resolution_listener_function);
  expected_node_for_listening_notification = node_2;
  TEST_ASSERT_EQUAL(0, test_resolution_listener_function_call_count);

  // Expect a get resolution
  send_function_return_code    = SL_STATUS_OK;
  get_function_return_code     = SL_STATUS_OK;
  expected_node_for_resolution = node_7;
  contiki_test_helper_run(1);

  TEST_ASSERT_EQUAL(1, test_send_function_call_count);
  TEST_ASSERT_EQUAL(0, test_set_function_call_count);
  TEST_ASSERT_EQUAL(1, test_get_function_call_count);

  // Simulate a send data complete
  on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK,
                                 TEST_TRANSMISSION_TIME,
                                 node_7,
                                 RESOLVER_GET_RULE);
  contiki_test_helper_run(1);  // start the timer

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(node_7, REPORTED_ATTRIBUTE));

  // Get was just executed, nothing else will happen.

  // Delete Node 7. Node 2 is resolved now
  attribute_store_delete_node(node_7);

  contiki_test_helper_run(1);
  TEST_ASSERT_EQUAL(1, test_resolution_listener_function_call_count);

  contiki_test_helper_run(0);
}

void test_attribute_resolver_give_up_on_group_get_rule()
{
  // Group Node 7 and 9 as "siblings"
  attribute_resolver_set_attribute_depth(attribute_store_get_node_type(node_7),
                                         1);

  // Register a rule for type 7
  attribute_resolver_register_rule(attribute_store_get_node_type(node_7),
                                   NULL,
                                   &test_get_resolution_function);
  // Register the same rule for type 9
  attribute_resolver_register_rule(attribute_store_get_node_type(node_9),
                                   NULL,
                                   &test_get_resolution_function);

  expected_node_for_listening_notification = node_2;
  attribute_resolver_set_resolution_listener(
    expected_node_for_listening_notification,
    &test_resolution_listener_function);

  // Regular get resolution.
  attribute_store_undefine_reported(node_7);
  attribute_store_undefine_reported(node_9);

  // Expect a get resolution
  send_function_return_code    = SL_STATUS_OK;
  get_function_return_code     = SL_STATUS_OK;
  expected_node_for_resolution = node_7;

  for (uint8_t i = 0; i < TEST_GET_RETRY_COUNT; i++) {
    contiki_test_helper_run(1);

    TEST_ASSERT_EQUAL(i + 1, test_send_function_call_count);
    TEST_ASSERT_EQUAL(0, test_set_function_call_count);
    TEST_ASSERT_EQUAL(i + 1, test_get_function_call_count);

    // Simulate a send data complete for attempt i
    TEST_ASSERT_EQUAL(0, test_resolution_listener_function_call_count);
    on_resolver_send_data_complete(RESOLVER_SEND_STATUS_OK,
                                   TEST_TRANSMISSION_TIME,
                                   expected_node_for_resolution,
                                   RESOLVER_GET_RULE);
    contiki_test_helper_run(1);  // start the timer

    // Now pass the retry timer:
    contiki_test_helper_run(TEST_GET_RETRY_TIMEOUT);
    TEST_ASSERT_EQUAL(i + 1, test_send_function_call_count);
    TEST_ASSERT_EQUAL(0, test_set_function_call_count);
    TEST_ASSERT_EQUAL(i + 1, test_get_function_call_count);
    contiki_test_helper_run(TEST_TRANSMISSION_TIME);
  }

  contiki_test_helper_run(TEST_GET_RETRY_TIMEOUT + TEST_TRANSMISSION_TIME);
  TEST_ASSERT_EQUAL(TEST_GET_RETRY_COUNT, test_send_function_call_count);
  TEST_ASSERT_EQUAL(0, test_set_function_call_count);
  TEST_ASSERT_EQUAL(TEST_GET_RETRY_COUNT, test_get_function_call_count);

  // Now we should have given up on both Node 7 and Node 9
  TEST_ASSERT_EQUAL(1, test_resolution_listener_function_call_count);
}

void test_attribute_resolver_rule_get_group_node()
{
  using namespace attribute_store;
  /* Lets construct the tree below
   *         A ---
   *       /   \   \
   *      B1   B2   D1
   *     / \   / \
   *    C   C C   C
   */
  attribute::root().delete_node();

  auto a = attribute::root().emplace_node(0xa, 0xa);

  auto b1 = a.emplace_node(0xb, 1);
  auto b2 = a.emplace_node(0xb, 2);
  auto d  = a.emplace_node(0xd, 1);

  auto b1c1 = b1.emplace_node(0xc, 1);
  auto b1c2 = b1.emplace_node(0xc, 2);
  auto b2c1 = b2.emplace_node(0xc, 1);
  auto b2c2 = b2.emplace_node(0xc, 2);

  attribute_resolver_rule_init(0);
  // Let all node types B,C,D be resolved by the same frame

  attribute_resolver_rule_register(0xb,
                                   &test_set_resolution_function,
                                   &test_get_resolution_function);
  attribute_resolver_rule_register(0xc,
                                   &test_set_resolution_function,
                                   &test_get_resolution_function);
  attribute_resolver_rule_register(0xd,
                                   &test_set_resolution_function,
                                   &test_get_resolution_function);

  // Depth 0, means the node sits alone in its group
  attribute_resolver_set_attribute_depth(0xb, 0);
  std::set<attribute_store_node_t> g0
    = attribute_resolver_rule_get_group_nodes(RESOLVER_GET_RULE, b1);
  TEST_ASSERT_EQUAL(1, g0.count(b1));
  TEST_ASSERT_EQUAL(1, g0.size());

  // Now which nodes are also resolved by the set rule of b1 at depth 1?
  // It should be b1,b2 and d1.
  attribute_resolver_set_attribute_depth(0xb, 1);
  std::set<attribute_store_node_t> g1
    = attribute_resolver_rule_get_group_nodes(RESOLVER_GET_RULE, b1);

  TEST_ASSERT_EQUAL(1, g1.count(b1));
  TEST_ASSERT_EQUAL(1, g1.count(b2));
  TEST_ASSERT_EQUAL(1, g1.count(d));
  TEST_ASSERT_EQUAL(3, g1.size());

  attribute_resolver_set_attribute_depth(0xc, 2);

  std::set<attribute_store_node_t> g2
    = attribute_resolver_rule_get_group_nodes(RESOLVER_GET_RULE, b1c1);

  TEST_ASSERT_EQUAL(1, g2.count(b1c1));
  TEST_ASSERT_EQUAL(1, g2.count(b1c2));
  TEST_ASSERT_EQUAL(1, g2.count(b2c1));
  TEST_ASSERT_EQUAL(1, g2.count(b2c2));
  TEST_ASSERT_EQUAL(4, g2.size());
}
}
