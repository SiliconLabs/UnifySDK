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
#include "attribute_mapper.h"
#include "attribute_mapper_process.h"
#include "workaround.hpp"

// Other Unify components
#include "attribute_store_fixt.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "datastore.h"

// Test helpers
#include "contiki_test_helper.h"

#include "unity.h"

extern "C" {

static attribute_store_node_t root_node;
static attribute_store_node_t node_1;
static attribute_store_node_t node_2;

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
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  // Create a small network: root with 2 nodes under
  attribute_store_delete_node(attribute_store_get_root());
  root_node     = attribute_store_get_root();
  int32_t value = 1;
  node_1        = attribute_store_emplace(root_node, 1, &value, sizeof(value));
  value         = 2;
  node_2        = attribute_store_emplace(root_node, 2, &value, sizeof(value));

  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute_store_get_node_type(root_node));
  e.reset();

  // Start the process
  process_start(&unify_attribute_mapper_process, NULL);
}

/// Called after each and every test
void tearDown()
{
  process_exit(&unify_attribute_mapper_process);
}

void test_attribute_mapper_process_simple_update_test()
{
  // Load a rule, create attribute 3 with the same value as attribute 1.
  MapperEngine &e = MapperEngine::get_instance();
  e.add_expression(R"(scope 0 { r'3 = r'1 })");

  on_reported_attribute_update(node_1, ATTRIBUTE_CREATED);
  on_reported_attribute_update(node_1, ATTRIBUTE_UPDATED);
  on_reported_attribute_update(node_1, ATTRIBUTE_UPDATED);
  on_reported_attribute_update(node_1, ATTRIBUTE_UPDATED);

  // Nothing happens until we run contiki events
  attribute_store_node_t node_3
    = attribute_store_get_first_child_by_type(root_node, 3);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, node_3);

  contiki_test_helper_run(0);
  node_3 = attribute_store_get_first_child_by_type(root_node, 3);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, node_3);
}

void test_attribute_mapper_process_deletion_test()
{
  // Load a rule, create attribute 3 with the same value as attribute 1.
  MapperEngine &e = MapperEngine::get_instance();
  e.add_expression(R"(scope 0 { r'3 = r'1 })");

  on_reported_attribute_update(node_1, ATTRIBUTE_CREATED);
  on_reported_attribute_update(node_1, ATTRIBUTE_UPDATED);
  on_reported_attribute_update(node_1, ATTRIBUTE_DELETED);
  attribute_store_delete_node(node_1);

  // Node 3 was created while r'1 was alive and updated.
  // It should keep existing now
  contiki_test_helper_run(0);
  attribute_store_node_t node_3
    = attribute_store_get_first_child_by_type(root_node, 3);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, node_3);
}

void test_attribute_mapper_has_pending_evaluations()
{
  contiki_test_helper_run(0);
  TEST_ASSERT_FALSE(attribute_mapper_has_pending_evaluations());

  // Load a rule, create attribute 3 with the same value as attribute 1.
  MapperEngine &e = MapperEngine::get_instance();
  e.add_expression(R"(scope 0 { r'3 = r'1 })");

  // Creations are processed immediately
  on_reported_attribute_update(node_1, ATTRIBUTE_CREATED);
  TEST_ASSERT_FALSE(attribute_mapper_has_pending_evaluations());

  on_reported_attribute_update(node_1, ATTRIBUTE_UPDATED);
  TEST_ASSERT_TRUE(attribute_mapper_has_pending_evaluations());

  // Running the events will get no more pending updates
  contiki_test_helper_run(0);
  TEST_ASSERT_FALSE(attribute_mapper_has_pending_evaluations());

  // Deletions are processed immediately
  on_reported_attribute_update(node_1, ATTRIBUTE_DELETED);
  TEST_ASSERT_FALSE(attribute_mapper_has_pending_evaluations());
}

void test_attribute_mapper_process_creation_then_deletion_test()
{
  // Load a rule
  MapperEngine &e = MapperEngine::get_instance();
  e.add_expression(R"(scope 0 {
    r'3 = r'1
    r'4 = r'2
  })");

  on_reported_attribute_update(node_1, ATTRIBUTE_CREATED);
  on_reported_attribute_update(node_1, ATTRIBUTE_UPDATED);
  on_reported_attribute_update(node_2, ATTRIBUTE_UPDATED);
  on_desired_attribute_update(node_2, ATTRIBUTE_UPDATED);

  // Delete should cancel out immediately the updated
  contiki_test_helper_run(0);
  attribute_store_node_t node_3
    = attribute_store_get_first_child_by_type(root_node, 3);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, node_3);
  attribute_store_node_t node_4
    = attribute_store_get_first_child_by_type(root_node, 4);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, node_4);

  on_reported_attribute_update(node_1, ATTRIBUTE_DELETED);
  on_reported_attribute_update(node_2, ATTRIBUTE_DELETED);

  // Delete should cancel out immediately the updated, no need for contiki events to run the rules:
  // Mapper engine won't do anything to the reported values when the attributes are deleted.
  TEST_ASSERT_EQUAL(1, attribute_store_get_reported_number(node_3));
  TEST_ASSERT_EQUAL(2, attribute_store_get_reported_number(node_4));
}

void test_attribute_mapper_process_unknown_event_test()
{
  // Nothing much to test here.
  process_post(&unify_attribute_mapper_process, 0xFF, nullptr);
  contiki_test_helper_run(0);
}

void test_attribute_mapper_process_pending_update_on_deleted_node()
{
  // Load a rule
  MapperEngine &e = MapperEngine::get_instance();
  e.add_expression(R"(scope 0 {
    r'3 = r'1
    r'4 = r'2
  })");

  attribute_store_delete_node(node_1);
  contiki_test_helper_run(0);

  on_reported_attribute_update(node_1, ATTRIBUTE_UPDATED);
  on_reported_attribute_update(node_2, ATTRIBUTE_UPDATED);
  contiki_test_helper_run(0);

  // Node 3 should not exist, the Attribute Update notification
  // was processed when node 3 did not exist.
  attribute_store_node_t node_3
    = attribute_store_get_first_child_by_type(root_node, 3);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, node_3);
  attribute_store_node_t node_4
    = attribute_store_get_first_child_by_type(root_node, 4);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, node_4);
  TEST_ASSERT_EQUAL(2, attribute_store_get_reported_number(node_4));
}
}
