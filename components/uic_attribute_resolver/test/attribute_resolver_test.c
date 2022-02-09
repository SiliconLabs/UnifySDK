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
#include "contiki_test_helper.h"
#include "process.h"
#include "attribute_store_fixt.h"
#include "attribute_resolver.h"
#include "attribute_resolver_rule_mock.h"
#include "attribute_resolver_rule_internal_mock.h"
#include "sl_log.h"
#include "sl_status.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "datastore_fixt.h"
#include <time.h>

#define LOG_TAG               "attribute_resolver_test"
#define ATTRIBUTE_HOME_ID     2
#define ATTRIBUTE_ENDPOINT_ID 4
extern struct process attribute_resolver_process;

// Static test variables.
static attribute_store_node_t received_node;
static uint8_t node_listener_calls;
static uint8_t node_listener2_calls;
static uint8_t abort_calls;
static attribute_rule_complete_t rule_compl_func;

typedef enum {
  RESOLVER_NEXT_EVENT,
  RESOLVER_PAUSE_EVENT,
  RESOLVER_RESUME_EVENT
} attribute_resolver_worker_event_t;

// Test callback for node listener
void node_listener_function(attribute_store_node_t node)
{
  received_node = node;
  node_listener_calls++;
}
void node_listener2_function(attribute_store_node_t node)
{
  node_listener2_calls++;
}
sl_status_t attribute_resolver_send(attribute_store_node_t node,
                                    const uint8_t *frame_data,
                                    uint16_t frame_data_len,
                                    bool is_set)
{
  return SL_STATUS_OK;
}
sl_status_t attribute_resolver_abort(attribute_store_node_t node)
{
  abort_calls++;
  return SL_STATUS_OK;
}
attribute_resolver_config_t config = {.send_init = NULL,
                                      .send      = attribute_resolver_send,
                                      .abort     = attribute_resolver_abort,
                                      .get_retry_timeout = 20000,
                                      .get_retry_count   = 3};
void suiteSetUp() {}

int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp()
{
  received_node       = ATTRIBUTE_STORE_INVALID_NODE;
  node_listener_calls = 0;
  abort_calls         = 0;
  datastore_fixt_setup(":memory:");
  attribute_store_init();
  attribute_resolver_rule_busy_IgnoreAndReturn(false);
}

void tearDown()
{
  attribute_resolver_teardown();
  attribute_store_teardown();
}

void resolver_rule_init_stub(attribute_rule_complete_t __compl_func, int n)
{
  rule_compl_func = __compl_func;
}

void test_attribute_resolver_get_func()
{
  // Init process
  attribute_resolver_rule_init_Stub(resolver_rule_init_stub);

  contiki_test_helper_init();
  attribute_resolver_init(config);
  uint32_t home_id = 0x11223344;
  // Add a node with undefined home ID node
  attribute_store_node_t home_id_node;
  attribute_store_node_t root_node = attribute_store_get_root();

  attribute_resolver_has_get_rule_IgnoreAndReturn(true);
  attribute_resolver_has_set_rule_IgnoreAndReturn(false);
  home_id_node = attribute_store_add_node(ATTRIBUTE_HOME_ID, root_node);

  // get func is expected to be called
  attribute_resolver_rule_execute_ExpectAndReturn(home_id_node,
                                                  false,
                                                  SL_STATUS_OK);
  contiki_test_helper_run(0);

  // Set the reported value to mimic the get command sent successfully
  attribute_store_set_reported(home_id_node, &home_id, sizeof(home_id));

  // Expect only the root node
  attribute_resolver_rule_execute_ExpectAndReturn(root_node,
                                                  false,
                                                  SL_STATUS_NOT_FOUND);
  process_post(&attribute_resolver_process, RESOLVER_NEXT_EVENT, NULL);
  contiki_test_helper_run(0);
}

void test_attribute_resolver_get_retransmission_func()
{
  // Init process
  attribute_resolver_rule_init_Stub(resolver_rule_init_stub);

  contiki_test_helper_init();
  attribute_resolver_init(config);
  // Add a node with undefined home ID node
  attribute_store_node_t home_id_node;
  attribute_store_node_t root_node = attribute_store_get_root();
  attribute_resolver_has_get_rule_IgnoreAndReturn(true);
  attribute_resolver_has_set_rule_IgnoreAndReturn(false);
  home_id_node = attribute_store_add_node(ATTRIBUTE_HOME_ID, root_node);

  for (int i = 0; i < config.get_retry_count; i++) {
    // get func is expected to be called
    attribute_resolver_rule_execute_ExpectAndReturn(home_id_node,
                                                    false,
                                                    SL_STATUS_OK);
    rule_compl_func(home_id_node, 100);
    contiki_test_helper_run(100000);

    if (i == 0) {
      // Root will get attempted for resolution only once
      attribute_resolver_rule_execute_ExpectAndReturn(root_node,
                                                      false,
                                                      SL_STATUS_ALREADY_EXISTS);
    }
    //Let time pass
    contiki_test_helper_run(100000);
  }

  contiki_test_helper_run(100000);
}

void test_attribute_resolver_set_func()
{
  attribute_resolver_rule_init_Ignore();
  // Init process
  contiki_test_helper_init();
  attribute_resolver_init(config);

  uint32_t home_id         = 0x11223344;
  uint32_t desired_home_id = 0x55667788;
  // Add a node with undefined home ID node
  attribute_store_node_t home_id_node;
  attribute_store_node_t root_node = attribute_store_get_root();

  attribute_resolver_has_get_rule_IgnoreAndReturn(false);
  attribute_resolver_has_set_rule_IgnoreAndReturn(true);
  home_id_node = attribute_store_add_node(ATTRIBUTE_HOME_ID, root_node);

  attribute_store_set_node_attribute_value(home_id_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&home_id,
                                           sizeof(home_id));
  attribute_store_set_node_attribute_value(home_id_node,
                                           DESIRED_ATTRIBUTE,
                                           (uint8_t *)&desired_home_id,
                                           sizeof(desired_home_id));

  attribute_resolver_rule_execute_ExpectAndReturn(home_id_node,
                                                  true,
                                                  SL_STATUS_OK);
  contiki_test_helper_run(0);
}

void test_attribute_resolver_pause_resume()
{
  attribute_resolver_rule_init_Stub(resolver_rule_init_stub);
  // Init process
  contiki_test_helper_init();
  attribute_resolver_init(config);

  // Add a node with undefined home ID node
  attribute_store_node_t home_id_node  = 0;
  attribute_store_node_t endpoint_node = 0;
  attribute_store_node_t root_node     = attribute_store_get_root();

  //At init we will just scan the root. We only do get rules in this test
  attribute_resolver_has_get_rule_IgnoreAndReturn(true);
  attribute_resolver_has_set_rule_IgnoreAndReturn(false);
  attribute_resolver_rule_execute_IgnoreAndReturn(SL_STATUS_NOT_FOUND);

  //Add homeid node. Nothing should be resolved as the root is paused
  home_id_node  = attribute_store_add_node(ATTRIBUTE_HOME_ID, root_node);
  endpoint_node = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, home_id_node);

  uint8_t value = 23;
  attribute_store_set_node_attribute_value(root_node,
                                           REPORTED_ATTRIBUTE,
                                           &value,
                                           sizeof(value));
  attribute_store_set_node_attribute_value(home_id_node,
                                           REPORTED_ATTRIBUTE,
                                           &value,
                                           sizeof(value));
  attribute_store_set_node_attribute_value(endpoint_node,
                                           REPORTED_ATTRIBUTE,
                                           &value,
                                           sizeof(value));
  contiki_test_helper_run(0);
  rule_compl_func(endpoint_node, 0);
  contiki_test_helper_run(0);
  rule_compl_func(home_id_node, 0);
  contiki_test_helper_run(0);
  rule_compl_func(root_node, 0);

  // Pause the resolution of the home_id_node and children
  attribute_resolver_pause_node_resolution(home_id_node);

  // Increase test coverage by calling the log function:
  attribute_resolver_state_log();

  // Nothing should happen
  contiki_test_helper_run(10);

  // Now unpause and expect resolution of the 2 new nodes
  attribute_resolver_rule_execute_StopIgnore();

  // Try to modify the values and see if resolution is triggered
  attribute_store_set_node_attribute_value(endpoint_node,
                                           REPORTED_ATTRIBUTE,
                                           NULL,
                                           0);

  // Nothing should happen, nodes are paused
  contiki_test_helper_run(10);

  attribute_resolver_resume_node_resolution(home_id_node);

  attribute_resolver_rule_execute_ExpectAndReturn(endpoint_node,
                                                  false,
                                                  SL_STATUS_OK);

  contiki_test_helper_run(0);
  rule_compl_func(endpoint_node, 0);
  contiki_test_helper_run(0);
}

void test_rule_execution_return_busy()
{
  attribute_resolver_rule_init_Ignore();
  contiki_test_helper_init();
  attribute_resolver_init(config);
  // Add a node with undefined home ID node
  attribute_store_node_t home_id_node;
  attribute_store_node_t root_node = attribute_store_get_root();

  attribute_resolver_has_get_rule_IgnoreAndReturn(true);
  attribute_resolver_has_set_rule_IgnoreAndReturn(true);
  home_id_node = attribute_store_add_node(ATTRIBUTE_HOME_ID, root_node);

  // get func is expected to be called
  attribute_resolver_rule_execute_ExpectAndReturn(home_id_node,
                                                  false,
                                                  SL_STATUS_BUSY);
  attribute_resolver_rule_execute_ExpectAndReturn(root_node,
                                                  false,
                                                  SL_STATUS_NOT_FOUND);
  contiki_test_helper_run(0);
}

void test_set_listener()
{
  attribute_resolver_rule_init_Ignore();
  contiki_test_helper_init();
  attribute_resolver_init(config);

  // Add a node with undefined home ID node
  attribute_store_node_t home_id_node;
  attribute_store_node_t root_node = attribute_store_get_root();

  attribute_resolver_has_get_rule_IgnoreAndReturn(true);
  attribute_resolver_has_set_rule_IgnoreAndReturn(true);
  home_id_node = attribute_store_add_node(ATTRIBUTE_HOME_ID, root_node);

  // Somebody want to get notified about the resolved HomeID:
  attribute_resolver_set_resolution_listener(home_id_node,
                                             node_listener_function);
  attribute_resolver_set_resolution_listener(home_id_node,
                                             node_listener2_function);

  // get func is expected to be called
  attribute_resolver_rule_execute_ExpectAndReturn(home_id_node,
                                                  false,
                                                  SL_STATUS_ALREADY_EXISTS);
  attribute_resolver_rule_execute_ExpectAndReturn(root_node,
                                                  false,
                                                  SL_STATUS_ALREADY_EXISTS);

  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(0, node_listener_calls);
  TEST_ASSERT_EQUAL(0, node_listener2_calls);

  // As long as the values are still undefined, no listener call is made.
  uint8_t value = 23;
  attribute_store_set_node_attribute_value(home_id_node,
                                           REPORTED_ATTRIBUTE,
                                           &value,
                                           sizeof(value));

  attribute_store_set_node_attribute_value(root_node,
                                           REPORTED_ATTRIBUTE,
                                           &value,
                                           sizeof(value));

  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(1, node_listener_calls);
  TEST_ASSERT_EQUAL(1, node_listener2_calls);
  TEST_ASSERT_EQUAL(home_id_node, received_node);
}

void test_clear_listener()
{
  attribute_resolver_rule_init_Ignore();
  contiki_test_helper_init();
  attribute_resolver_init(config);

  // Add a node with undefined home ID node
  attribute_store_node_t home_id_node;
  attribute_store_node_t root_node = attribute_store_get_root();

  attribute_resolver_has_get_rule_IgnoreAndReturn(true);
  attribute_resolver_has_set_rule_IgnoreAndReturn(true);
  home_id_node = attribute_store_add_node(ATTRIBUTE_HOME_ID, root_node);

  // Somebody want to get notified about the resolved HomeID:
  attribute_resolver_set_resolution_listener(home_id_node,
                                             node_listener_function);
  // Add a second listener on the same HomeID
  attribute_resolver_set_resolution_listener(home_id_node,
                                             node_listener2_function);
  // Remove first callback on the resolved HomeID:
  attribute_resolver_clear_resolution_listener(home_id_node,
                                               node_listener_function);

  // get func is expected to be called
  attribute_resolver_rule_execute_ExpectAndReturn(home_id_node,
                                                  false,
                                                  SL_STATUS_NOT_SUPPORTED);
  attribute_resolver_rule_execute_ExpectAndReturn(root_node,
                                                  false,
                                                  SL_STATUS_NOT_SUPPORTED);
  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(0, node_listener_calls);
  TEST_ASSERT_EQUAL(1, node_listener2_calls);
}

void test_attribute_resolver_on_node_update_test()
{
  // Init process
  attribute_resolver_rule_init_Ignore();
  contiki_test_helper_init();
  attribute_resolver_init(config);

  attribute_resolver_rule_register_Ignore();
  attribute_resolver_register_rule(ATTRIBUTE_ENDPOINT_ID, NULL, NULL);

  // Built attribute tree contain root node and home id
  attribute_store_node_t home_id_node;
  attribute_store_node_t root_node = attribute_store_get_root();

  attribute_resolver_has_get_rule_IgnoreAndReturn(true);
  attribute_resolver_has_set_rule_IgnoreAndReturn(true);
  home_id_node = attribute_store_add_node(ATTRIBUTE_HOME_ID, root_node);

  attribute_resolver_rule_execute_ExpectAndReturn(home_id_node,
                                                  false,
                                                  SL_STATUS_NOT_INITIALIZED);

  attribute_resolver_rule_execute_ExpectAndReturn(root_node,
                                                  false,
                                                  SL_STATUS_NOT_INITIALIZED);
  contiki_test_helper_run(0);

  // Add end point attribute to trigger on_resolver_node_update and scan the three again,
  // the scan shall only be scan only root and end point attributes
  attribute_store_node_t end_id_node;
  end_id_node = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, home_id_node);

  uint32_t home_id = 0x11223344;
  attribute_store_set_node_attribute_value(home_id_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&home_id,
                                           sizeof(home_id));
  attribute_store_set_node_attribute_value(root_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&home_id,
                                           sizeof(home_id));

  attribute_resolver_rule_execute_ExpectAndReturn(end_id_node,
                                                  false,
                                                  SL_STATUS_ALREADY_EXISTS);

  // to get notified about the resolved ENDPOINT
  attribute_resolver_set_resolution_listener(end_id_node,
                                             node_listener_function);

  contiki_test_helper_run(0);
  // The node_listener_function shall only be called when end point attribute is resolved
  // Endpoint ID is still undefined.
  TEST_ASSERT_EQUAL(0, node_listener_calls);

  // As long as the values are still undefined, no listener call is made.
  uint8_t value = 23;
  attribute_store_set_node_attribute_value(end_id_node,
                                           REPORTED_ATTRIBUTE,
                                           &value,
                                           sizeof(value));

  contiki_test_helper_run(0);

  // The node_listener_function shall only be called when end point attribute is resolved
  // Endpoint ID is still undefined.
  TEST_ASSERT_EQUAL(1, node_listener_calls);
}

void test_attribute_resolver_on_node_deletion_test()
{
  // Init process
  attribute_resolver_rule_init_Ignore();
  contiki_test_helper_init();
  attribute_resolver_init(config);

  attribute_resolver_rule_register_Ignore();
  attribute_resolver_register_rule(ATTRIBUTE_ENDPOINT_ID, NULL, NULL);

  attribute_store_node_t home_id_node;
  attribute_store_node_t root_node = attribute_store_get_root();

  attribute_resolver_has_get_rule_IgnoreAndReturn(true);
  attribute_resolver_has_set_rule_IgnoreAndReturn(true);
  home_id_node = attribute_store_add_node(ATTRIBUTE_HOME_ID, root_node);
  attribute_store_node_t endpoint_id_node
    = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, home_id_node);

  attribute_resolver_set_resolution_listener(endpoint_id_node,
                                             node_listener_function);

  attribute_resolver_rule_execute_ExpectAndReturn(endpoint_id_node,
                                                  false,
                                                  SL_STATUS_NOT_INITIALIZED);

  attribute_resolver_rule_execute_ExpectAndReturn(home_id_node,
                                                  false,
                                                  SL_STATUS_NOT_INITIALIZED);

  attribute_resolver_rule_execute_ExpectAndReturn(root_node,
                                                  false,
                                                  SL_STATUS_NOT_INITIALIZED);
  contiki_test_helper_run(0);

  // The node_listener_function shall only be called when the endpoint
  // attribute is fully resolved. Endpoint ID value is still undefined.
  TEST_ASSERT_EQUAL(0, node_listener_calls);

  // Now we just delete the attribute store.
  attribute_resolver_rule_abort_Expect(endpoint_id_node);
  attribute_resolver_rule_abort_Expect(home_id_node);
  attribute_store_delete_node(root_node);
  contiki_test_helper_run(0);

  // The node_listener_function shall only be called when end point attribute is resolved
  // Endpoint ID is still undefined.
  TEST_ASSERT_EQUAL(0, node_listener_calls);
}

void test_attribute_resolver_node_or_child_needs_resolution()
{
  attribute_resolver_rule_init_Stub(resolver_rule_init_stub);
  attribute_resolver_init(config);

  // Add a node with undefined home ID node
  attribute_store_node_t home_id_node  = 0;
  attribute_store_node_t endpoint_node = 0;
  attribute_store_node_t root_node     = attribute_store_get_root();
  attribute_resolver_has_get_rule_IgnoreAndReturn(false);
  attribute_resolver_has_set_rule_IgnoreAndReturn(false);
  home_id_node  = attribute_store_add_node(ATTRIBUTE_HOME_ID, root_node);
  endpoint_node = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, home_id_node);
  attribute_store_type_t root_node_type
    = attribute_store_get_node_type(root_node);
  attribute_store_type_t home_id_node_type
    = attribute_store_get_node_type(home_id_node);
  attribute_store_type_t endpoint_node_type
    = attribute_store_get_node_type(endpoint_node);
  attribute_resolver_has_get_rule_StopIgnore();
  attribute_resolver_has_set_rule_StopIgnore();

  // First no rule for anybody:
  attribute_resolver_has_get_rule_ExpectAndReturn(root_node_type, false);
  attribute_resolver_has_set_rule_ExpectAndReturn(root_node_type, false);
  attribute_resolver_has_get_rule_ExpectAndReturn(home_id_node_type, false);
  attribute_resolver_has_set_rule_ExpectAndReturn(home_id_node_type, false);
  attribute_resolver_has_get_rule_ExpectAndReturn(endpoint_node_type, false);
  attribute_resolver_has_set_rule_ExpectAndReturn(endpoint_node_type, false);

  TEST_ASSERT_EQUAL(
    false,
    attribute_resolver_node_or_child_needs_resolution(root_node));

  // Get rule is present for the HomeID:
  attribute_resolver_has_get_rule_ExpectAndReturn(root_node_type, false);
  attribute_resolver_has_set_rule_ExpectAndReturn(root_node_type, false);
  attribute_resolver_has_get_rule_ExpectAndReturn(home_id_node_type, true);

  TEST_ASSERT_EQUAL(
    true,
    attribute_resolver_node_or_child_needs_resolution(root_node));

  uint8_t value = 23;
  attribute_resolver_has_get_rule_IgnoreAndReturn(false);
  attribute_resolver_has_set_rule_IgnoreAndReturn(false);
  attribute_store_set_node_attribute_value(home_id_node,
                                           REPORTED_ATTRIBUTE,
                                           &value,
                                           sizeof(value));
  attribute_resolver_has_get_rule_StopIgnore();
  attribute_resolver_has_set_rule_StopIgnore();
  // HomeID does not need a get anymore. It should return false, even if we
  // have a rule for it
  attribute_resolver_has_get_rule_ExpectAndReturn(root_node_type, false);
  attribute_resolver_has_set_rule_ExpectAndReturn(root_node_type, false);
  attribute_resolver_has_get_rule_ExpectAndReturn(home_id_node_type, true);
  attribute_resolver_has_set_rule_ExpectAndReturn(home_id_node_type, false);
  attribute_resolver_has_get_rule_ExpectAndReturn(endpoint_node_type, false);
  attribute_resolver_has_set_rule_ExpectAndReturn(endpoint_node_type, false);

  TEST_ASSERT_EQUAL(
    false,
    attribute_resolver_node_or_child_needs_resolution(root_node));

  // Unmatch the value for the endpoint:
  attribute_resolver_has_get_rule_IgnoreAndReturn(false);
  attribute_resolver_has_set_rule_IgnoreAndReturn(false);
  attribute_store_set_node_attribute_value(endpoint_node,
                                           REPORTED_ATTRIBUTE,
                                           &value,
                                           sizeof(value));
  value = 25;
  attribute_store_set_node_attribute_value(endpoint_node,
                                           DESIRED_ATTRIBUTE,
                                           &value,
                                           sizeof(value));
  attribute_resolver_has_get_rule_StopIgnore();
  attribute_resolver_has_set_rule_StopIgnore();

  // and pretend we have a set rule:
  attribute_resolver_has_get_rule_ExpectAndReturn(root_node_type, false);
  attribute_resolver_has_set_rule_ExpectAndReturn(root_node_type, false);
  attribute_resolver_has_get_rule_ExpectAndReturn(home_id_node_type, false);
  attribute_resolver_has_set_rule_ExpectAndReturn(home_id_node_type, false);
  attribute_resolver_has_get_rule_ExpectAndReturn(endpoint_node_type, true);
  attribute_resolver_has_set_rule_ExpectAndReturn(endpoint_node_type, true);

  TEST_ASSERT_EQUAL(
    true,
    attribute_resolver_node_or_child_needs_resolution(root_node));
}

void test_attribute_resolver_restart_set_func()
{
  attribute_resolver_rule_init_Ignore();
  // Init process
  contiki_test_helper_init();
  attribute_resolver_init(config);

  uint32_t home_id         = 0x11223344;
  uint32_t desired_home_id = 0x55667788;
  // Add a node with undefined home ID node
  attribute_store_node_t home_id_node;
  attribute_store_node_t root_node = attribute_store_get_root();

  attribute_resolver_has_get_rule_IgnoreAndReturn(false);
  attribute_resolver_has_set_rule_IgnoreAndReturn(true);
  home_id_node = attribute_store_add_node(ATTRIBUTE_HOME_ID, root_node);

  attribute_store_set_node_attribute_value(home_id_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&home_id,
                                           sizeof(home_id));
  attribute_store_set_node_attribute_value(home_id_node,
                                           DESIRED_ATTRIBUTE,
                                           (uint8_t *)&desired_home_id,
                                           sizeof(desired_home_id));

  attribute_resolver_rule_execute_ExpectAndReturn(home_id_node,
                                                  true,
                                                  SL_STATUS_OK);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(0, abort_calls);
  TEST_ASSERT_TRUE(is_node_pending_set_resolution(home_id_node));
  TEST_ASSERT_FALSE(is_node_pending_set_resolution(root_node));
  attribute_resolver_state_log();
  attribute_resolver_restart_set_resolution(home_id_node);

  TEST_ASSERT_EQUAL(1, abort_calls);
  contiki_test_helper_run(0);
}
