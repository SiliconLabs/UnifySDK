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
// Includes from this component
#include "attribute_store.h"
#include "attribute_store_fixt.h"

// Includes from other components
#include "sl_status.h"
#include "sl_log.h"
#include "datastore_fixt.h"

// Test includes
#include "unity.h"

// Static variables for our test
static int update_callback_1_counter          = 0;
static int update_callback_1_creation_counter = 0;
static int update_callback_1_update_counter   = 0;
static int update_callback_1_deletion_counter = 0;
static int update_callback_delete_node_counter = 0;
static int touch_callback_counter              = 0;
static attribute_store_node_t received_callback_1_node
  = ATTRIBUTE_STORE_INVALID_NODE;

static int update_callback_2_counter = 0;
static attribute_store_node_t received_callback_2_node
  = ATTRIBUTE_STORE_INVALID_NODE;
static attribute_store_node_t received_touched_node
  = ATTRIBUTE_STORE_INVALID_NODE;

static int update_callback_read_deleted_value_counter = 0;
static int update_callback_add_child_value_counter    = 0;

#define ATTRIBUTE_HOME_ID                            101
#define ATTRIBUTE_NODE_ID                            102
#define ATTRIBUTE_ENDPOINT_ID                        103
#define ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID 104

void update_callback_1_event(attribute_changed_event_t * event)
{
  received_callback_1_node = event->updated_node;
  update_callback_1_counter++;
  if (event->change == ATTRIBUTE_CREATED) {
    update_callback_1_creation_counter++;
  } else if (event->change == ATTRIBUTE_UPDATED) {
    update_callback_1_update_counter++;
  } else if (event->change == ATTRIBUTE_DELETED) {
    update_callback_1_deletion_counter++;
  }
}

void update_callback_1(attribute_store_node_t test,
                       attribute_store_change_t change)
{
  received_callback_1_node = test;
  update_callback_1_counter++;
  if (change == ATTRIBUTE_CREATED) {
    update_callback_1_creation_counter++;
  } else if (change == ATTRIBUTE_UPDATED) {
    update_callback_1_update_counter++;
  } else if (change == ATTRIBUTE_DELETED) {
    update_callback_1_deletion_counter++;
  }
}

void update_callback_2(attribute_store_node_t test,
                       attribute_store_change_t change)
{
  received_callback_2_node = test;
  update_callback_2_counter++;
}

void touch_callback(attribute_store_node_t touched_node)
{
  received_touched_node = touched_node;
  touch_callback_counter++;
}

void update_callback_read_deleted_value(attribute_store_node_t node,
                                        attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    update_callback_read_deleted_value_counter++;

    uint8_t value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
    uint8_t value_size                                  = 2;

    // Try to write the value: Should not be possible
    TEST_ASSERT_EQUAL(
      SL_STATUS_FAIL,
      attribute_store_set_node_attribute_value(node,
                                               REPORTED_ATTRIBUTE,
                                               value,
                                               value_size));

    // We expect Reported = 1 / Desired = 2 for this attribute.
    TEST_ASSERT_EQUAL(
      SL_STATUS_OK,
      attribute_store_get_node_attribute_value(node,
                                               REPORTED_ATTRIBUTE,
                                               value,
                                               &value_size));
    TEST_ASSERT_EQUAL(1, value_size);
    TEST_ASSERT_EQUAL(1, value[0]);
    TEST_ASSERT_EQUAL(
      SL_STATUS_OK,
      attribute_store_get_node_attribute_value(node,
                                               DESIRED_ATTRIBUTE,
                                               value,
                                               &value_size));
    TEST_ASSERT_EQUAL(1, value_size);
    TEST_ASSERT_EQUAL(2, value[0]);
  }
}

void update_callback_add_child(attribute_store_node_t node,
                               attribute_store_change_t change)
{
  if (change == ATTRIBUTE_DELETED) {
    update_callback_add_child_value_counter++;

    // Try to add a child to the node: Should not be possible
    TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                      attribute_store_add_node(node, 0x4321));
  }
}

void update_callback_delete_node(attribute_store_node_t node,
                                 attribute_store_change_t change)
{
  // I am an evil callback that deletes stuffs in the attribute store
  // Also if the node is already under deletion
  attribute_store_delete_node(node);
  update_callback_delete_node_counter++;
}

static void set_counters_to_zero()
{
  update_callback_1_counter                  = 0;
  update_callback_2_counter                  = 0;
  received_callback_1_node                   = 0;
  received_callback_2_node                   = 0;
  update_callback_1_creation_counter         = 0;
  update_callback_1_update_counter           = 0;
  update_callback_1_deletion_counter         = 0;
  update_callback_delete_node_counter        = 0;
  update_callback_read_deleted_value_counter = 0;
  update_callback_add_child_value_counter    = 0;
  touch_callback_counter                     = 0;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_fixt_setup("attribute_store.db");
}

// Before every test, ensure to clean up the datastore file, else it will grow for ever
void setUp()
{
  // This trigger a deletion of everything before each test
  TEST_ASSERT_EQUAL(SL_STATUS_OK, attribute_store_init());
  set_counters_to_zero();
  attribute_store_delete_node(attribute_store_get_root());
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_fixt_teardown();
  return num_failures;
}

void test_attribute_store_add_node_error_cases()
{
  // Cannot add a node under a non-existing node:
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                    attribute_store_add_node(0x0101, 0x1010));

  // Cannot add a node under the "invalid node"
  TEST_ASSERT_EQUAL(
    ATTRIBUTE_STORE_INVALID_NODE,
    attribute_store_add_node(0x0101, ATTRIBUTE_STORE_INVALID_NODE));

  // Cannot add a node of invalid type
  TEST_ASSERT_EQUAL(
    ATTRIBUTE_STORE_INVALID_NODE,
    attribute_store_add_node(ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
                             attribute_store_get_root()));
}
void test_attribute_store()
{
  uint8_t test_values[] = {0x01, 0x02, 0x03, 0x04};
  /////////////////////////////////////////////
  // Create a network
  /////////////////////////////////////////////
  attribute_store_node_t root_node;
  root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t new_node_1;
  new_node_1 = attribute_store_add_node(ATTRIBUTE_HOME_ID, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, new_node_1);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_node_attribute_value(new_node_1,
                                             REPORTED_ATTRIBUTE,
                                             test_values,
                                             sizeof(test_values)));

  attribute_store_node_t new_node_2;
  new_node_2 = attribute_store_add_node(ATTRIBUTE_NODE_ID, new_node_1);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, new_node_2);

  test_values[0] = 0xAA;  // This will be NodeID AA
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_node_attribute_value(new_node_2,
                                                             REPORTED_ATTRIBUTE,
                                                             test_values,
                                                             1));

  test_values[0] = 0x03;  // We also set a desired value for this guy
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_node_attribute_value(
                      new_node_2,
                      DESIRED_ATTRIBUTE,
                      test_values,
                      3));  // 3 bytes, so [0x03, 0x02, 0x03]

  attribute_store_node_t new_node_3;
  new_node_3 = attribute_store_add_node(ATTRIBUTE_NODE_ID, new_node_1);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, new_node_3);

  test_values[0] = 0x02;  // This will be NodeID 02
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_node_attribute_value(new_node_3,
                                                             REPORTED_ATTRIBUTE,
                                                             test_values,
                                                             1));

  attribute_store_node_t new_node_4;
  new_node_4 = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, new_node_2);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, new_node_4);

  test_values[0] = 0x05;  // This will be endpoint 05
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_node_attribute_value(new_node_4,
                                                             REPORTED_ATTRIBUTE,
                                                             test_values,
                                                             1));

  attribute_store_node_t new_node_5;
  // Invalid value test, it will not be accepted, as only the root has no parent.
  new_node_5
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                               ATTRIBUTE_STORE_INVALID_NODE);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, new_node_5);

  new_node_5
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                               new_node_4);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, new_node_5);

  test_values[0] = 0x01;  // This will be association group ID 01
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_node_attribute_value(new_node_5,
                                                             REPORTED_ATTRIBUTE,
                                                             test_values,
                                                             1));

  // Print out our network
  attribute_store_log_node(root_node, true);  // Increase test coverage

  /////////////////////////////////////////////
  // Test the get_node_parent() functionality
  /////////////////////////////////////////////
  // Now we expect that the parent of new_node_3 is new_node_1
  attribute_store_node_t relative_node_id;
  relative_node_id = attribute_store_get_node_parent(new_node_3);
  TEST_ASSERT_EQUAL(new_node_1, relative_node_id);

  // If we ask for the parent of the root, it should return ATTRIBUTE_STORE_INVALID_NODE
  relative_node_id = attribute_store_get_node_parent(root_node);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, relative_node_id);

  // If we ask for a invalid partent node, it should return invalid node
  relative_node_id
    = attribute_store_get_node_parent(ATTRIBUTE_STORE_INVALID_NODE);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, relative_node_id);

  /////////////////////////////////////////////
  // Test the get_node_child() functionality
  /////////////////////////////////////////////
  // Now we expect that the 1st child of new_node_1 is new_node_2
  relative_node_id
    = attribute_store_get_node_child(new_node_1,
                                     0);  // index 0 = first child

  TEST_ASSERT_EQUAL(new_node_2, relative_node_id);

  // Now we expect that the 2nd child of new_node_1 is new_node_3
  relative_node_id
    = attribute_store_get_node_child(new_node_1,
                                     1);  // index 1 = second child
  TEST_ASSERT_EQUAL(new_node_3, relative_node_id);

  // We expect no more children:
  relative_node_id
    = attribute_store_get_node_child(new_node_1,
                                     2);  // index 2 = third child
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, relative_node_id);

  // Invalid node test
  relative_node_id
    = attribute_store_get_node_child(ATTRIBUTE_STORE_INVALID_NODE, 0);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, relative_node_id);

  ///////////////////////////////////////////////////
  // Test the get_node_child_by_type() functionality
  ///////////////////////////////////////////////////
  // Now we expect that the 1st child of type ATTRIBUTE_NODE_ID under
  // new_node_1 is new_node_2
  relative_node_id
    = attribute_store_get_node_child_by_type(new_node_1,
                                             ATTRIBUTE_NODE_ID,
                                             0);  // index 0 = first child
  TEST_ASSERT_EQUAL(new_node_2, relative_node_id);

  // Now we expect that the 2nd child of type ATTRIBUTE_NODE_ID under
  // new_node_1 is new_node_3
  relative_node_id
    = attribute_store_get_node_child_by_type(new_node_1,
                                             ATTRIBUTE_NODE_ID,
                                             1);  // index 1 = second child
  TEST_ASSERT_EQUAL(new_node_3, relative_node_id);

  // Now we expect that the 3rd child of type ATTRIBUTE_NODE_ID under
  // new_node_1 does not exist
  relative_node_id
    = attribute_store_get_node_child_by_type(new_node_1,
                                             ATTRIBUTE_NODE_ID,
                                             2);  // index 2 = third child
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, relative_node_id);

  // Now we expect that the 1st child of type ATTRIBUTE_ENDPOINT_ID under
  // new_node_1 does not exist
  relative_node_id
    = attribute_store_get_node_child_by_type(new_node_1,
                                             ATTRIBUTE_ENDPOINT_ID,
                                             0);  // index 0 = first child
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, relative_node_id);

  // Invalid node test
  relative_node_id
    = attribute_store_get_node_child_by_type(ATTRIBUTE_STORE_INVALID_NODE,
                                             ATTRIBUTE_ENDPOINT_ID,
                                             0);  // index 0 = first child
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, relative_node_id);

  ///////////////////////////////////////////////////
  // Test the get_node_child_by_value() functionality
  ///////////////////////////////////////////////////
  // Now we expect that the 1st child of type ATTRIBUTE_NODE_ID under
  // new_node_1 with REPORTED = 0xAA is new_node_2
  test_values[0] = 0xAA;
  relative_node_id
    = attribute_store_get_node_child_by_value(new_node_1,
                                              ATTRIBUTE_NODE_ID,
                                              REPORTED_ATTRIBUTE,
                                              test_values,
                                              1,   // 1 byte from test_values
                                              0);  // index 0 = first child
  TEST_ASSERT_EQUAL(new_node_2, relative_node_id);

  // Increasing the index won't find any more child with the same value:
  relative_node_id
    = attribute_store_get_node_child_by_value(new_node_1,
                                              ATTRIBUTE_NODE_ID,
                                              REPORTED_ATTRIBUTE,
                                              test_values,
                                              1,   // 1 byte from test_values
                                              1);  // index 1 = second child
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, relative_node_id);

  // Find the first node with NodeID = 2, but non-matching size
  // (else it would match new_node_3)
  test_values[0] = 0x02;
  relative_node_id
    = attribute_store_get_node_child_by_value(new_node_1,
                                              ATTRIBUTE_NODE_ID,
                                              REPORTED_ATTRIBUTE,
                                              test_values,
                                              20,  // 20 byte from test_values
                                              0);  // index 0 = first child

  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, relative_node_id);

  // Find the first node with NodeID = 2, it should be new_node_3
  test_values[0] = 0x02;
  relative_node_id
    = attribute_store_get_node_child_by_value(new_node_1,
                                              ATTRIBUTE_NODE_ID,
                                              REPORTED_ATTRIBUTE,
                                              test_values,
                                              1,   // 1 byte from test_values
                                              0);  // index 0 = first child
  TEST_ASSERT_EQUAL(new_node_3, relative_node_id);

  // Find the first node with desired NodeID = 0x03, size 1
  // it should be not work due to non-matching sizes.
  test_values[0] = 0x03;
  relative_node_id
    = attribute_store_get_node_child_by_value(new_node_1,
                                              ATTRIBUTE_NODE_ID,
                                              DESIRED_ATTRIBUTE,
                                              test_values,
                                              1,   // 1 byte from test_values
                                              0);  // index 0 = first child
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, relative_node_id);

  // Find the first node with desired NodeID = 0x03 0x02 0x03, size 3
  // it should be new_node_2
  relative_node_id
    = attribute_store_get_node_child_by_value(new_node_1,
                                              ATTRIBUTE_NODE_ID,
                                              DESIRED_ATTRIBUTE,
                                              test_values,
                                              3,   // 3 bytes from test_values
                                              0);  // index 0 = first child
  TEST_ASSERT_EQUAL(new_node_2, relative_node_id);

  // Find the 3rd node with desired NodeID = 0x03 0x02 0x03, size 3
  // there is only 1, so it should results in not found
  relative_node_id
    = attribute_store_get_node_child_by_value(new_node_1,
                                              ATTRIBUTE_NODE_ID,
                                              DESIRED_ATTRIBUTE,
                                              test_values,
                                              3,   // 3 bytes from test_values
                                              2);  // index 2 = third child
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, relative_node_id);

  // Invalid node test
  relative_node_id
    = attribute_store_get_node_child_by_value(ATTRIBUTE_STORE_INVALID_NODE,
                                              ATTRIBUTE_NODE_ID,
                                              DESIRED_ATTRIBUTE,
                                              test_values,
                                              3,   // 3 bytes from test_values
                                              0);  // index 2 = third child
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, relative_node_id);

  ///////////////////////////////////////////////////
  // Test the get_node_type() functionality
  ///////////////////////////////////////////////////
  attribute_store_type_t node_type;
  node_type = attribute_store_get_node_type(new_node_1);
  TEST_ASSERT_EQUAL(ATTRIBUTE_HOME_ID, node_type);

  node_type = attribute_store_get_node_type(new_node_2);
  TEST_ASSERT_EQUAL(ATTRIBUTE_NODE_ID, node_type);

  node_type = attribute_store_get_node_type(new_node_5);
  TEST_ASSERT_EQUAL(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID, node_type);

  // Invalid node test:
  node_type = attribute_store_get_node_type(ATTRIBUTE_STORE_INVALID_NODE);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE, node_type);

  ///////////////////////////////////////////////////
  // Test the attribute_store_get_first_parent_with_type() function
  ///////////////////////////////////////////////////
  TEST_ASSERT_EQUAL(
    ATTRIBUTE_STORE_INVALID_NODE,
    attribute_store_get_first_parent_with_type(ATTRIBUTE_STORE_INVALID_NODE,
                                               ATTRIBUTE_HOME_ID));
  TEST_ASSERT_EQUAL(
    root_node,
    attribute_store_get_first_parent_with_type(new_node_5,
                                               ATTRIBUTE_TREE_ROOT));
  TEST_ASSERT_EQUAL(
    root_node,
    attribute_store_get_first_parent_with_type(new_node_2,
                                               ATTRIBUTE_TREE_ROOT));
  TEST_ASSERT_EQUAL(
    root_node,
    attribute_store_get_first_parent_with_type(new_node_1,
                                               ATTRIBUTE_TREE_ROOT));
  TEST_ASSERT_EQUAL(
    new_node_2,
    attribute_store_get_first_parent_with_type(new_node_5, ATTRIBUTE_NODE_ID));

  ///////////////////////////////////////////////////
  // Test the get_node_attribute_value() functionality
  ///////////////////////////////////////////////////
  uint8_t received_values[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];
  uint8_t received_values_size;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_get_node_attribute_value(new_node_1,
                                             REPORTED_ATTRIBUTE,
                                             received_values,
                                             &received_values_size));
  test_values[0] = 1;
  test_values[1] = 2;
  test_values[2] = 3;
  test_values[3] = 4;
  TEST_ASSERT_EQUAL(4, received_values_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(test_values,
                                received_values,
                                received_values_size);

  // new_node_2 desired value is 0x03, 0x02, 0x03
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_get_node_attribute_value(new_node_2,
                                             DESIRED_ATTRIBUTE,
                                             received_values,
                                             &received_values_size));
  test_values[0] = 0x03;
  TEST_ASSERT_EQUAL(3, received_values_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(test_values,
                                received_values,
                                received_values_size);

  // Try with with a non-existing node
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_get_node_attribute_value(ATTRIBUTE_STORE_INVALID_NODE,
                                             REPORTED_ATTRIBUTE,
                                             received_values,
                                             &received_values_size));
  TEST_ASSERT_EQUAL(0, received_values_size);

  // new_node_2 reported value is 0xAA
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_get_node_attribute_value(new_node_2,
                                             REPORTED_ATTRIBUTE,
                                             received_values,
                                             &received_values_size));
  test_values[0] = 0xAA;
  TEST_ASSERT_EQUAL(1, received_values_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(test_values,
                                received_values,
                                received_values_size);

  // Try with with a non-existing value state
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_get_node_attribute_value(new_node_2,
                                             300,
                                             received_values,
                                             &received_values_size));
  TEST_ASSERT_EQUAL(0, received_values_size);

  // Invalid node test:
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_get_node_attribute_value(ATTRIBUTE_STORE_INVALID_NODE,
                                             REPORTED_ATTRIBUTE,
                                             received_values,
                                             &received_values_size));
  TEST_ASSERT_EQUAL(0, received_values_size);

  ///////////////////////////////////////////////////
  // Test the register_callback_by_type()
  // / by_type_and_state() functionality
  ///////////////////////////////////////////////////
  update_callback_1_counter = 0;
  update_callback_2_counter = 0;
  // Register callback 1:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_callback_by_type(update_callback_1,
                                              ATTRIBUTE_NODE_ID));

  // Register the same twice, it will work only once:
  // ATTRIBUTE_NODE_ID will match new_node_2 and new_node_3
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_callback_by_type(update_callback_1,
                                              ATTRIBUTE_NODE_ID));

  // Update callback 2 will only hear about DESIRED_ATTRIBUTE changes
  // for ATTRIBUTE_NODE_ID
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_callback_by_type_and_state(update_callback_2,
                                                        ATTRIBUTE_NODE_ID,
                                                        DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_node_attribute_value(new_node_3,
                                                             DESIRED_ATTRIBUTE,
                                                             test_values,
                                                             1));

  TEST_ASSERT_EQUAL(1, update_callback_1_counter);
  TEST_ASSERT_EQUAL(0, update_callback_1_creation_counter);
  TEST_ASSERT_EQUAL(1, update_callback_1_update_counter);
  TEST_ASSERT_EQUAL(0, update_callback_1_deletion_counter);
  TEST_ASSERT_EQUAL(new_node_3, received_callback_1_node);
  TEST_ASSERT_EQUAL(1, update_callback_2_counter);
  TEST_ASSERT_EQUAL(new_node_3, received_callback_2_node);

  // Try to set to the same value again, there will be no additional callback
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_node_attribute_value(new_node_3,
                                                             DESIRED_ATTRIBUTE,
                                                             test_values,
                                                             1));
  TEST_ASSERT_EQUAL(1, update_callback_1_counter);
  TEST_ASSERT_EQUAL(0, update_callback_1_creation_counter);
  TEST_ASSERT_EQUAL(1, update_callback_1_update_counter);
  TEST_ASSERT_EQUAL(0, update_callback_1_deletion_counter);
  TEST_ASSERT_EQUAL(new_node_3, received_callback_1_node);
  TEST_ASSERT_EQUAL(1, update_callback_2_counter);
  TEST_ASSERT_EQUAL(new_node_3, received_callback_2_node);

  // Do the same, but try changing the REPORTED attribute.
  test_values[0] = 0xFE;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_node_attribute_value(new_node_2,
                                                             REPORTED_ATTRIBUTE,
                                                             test_values,
                                                             1));

  TEST_ASSERT_EQUAL(2, update_callback_1_counter);
  TEST_ASSERT_EQUAL(0, update_callback_1_creation_counter);
  TEST_ASSERT_EQUAL(2, update_callback_1_update_counter);
  TEST_ASSERT_EQUAL(0, update_callback_1_deletion_counter);
  TEST_ASSERT_EQUAL(new_node_2, received_callback_1_node);
  TEST_ASSERT_EQUAL(1, update_callback_2_counter);
  TEST_ASSERT_EQUAL(new_node_3, received_callback_2_node);

  // Once again, changing the reported value to an identical value
  // No additional callback should be made
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_node_attribute_value(new_node_2,
                                                             REPORTED_ATTRIBUTE,
                                                             test_values,
                                                             1));

  TEST_ASSERT_EQUAL(2, update_callback_1_counter);
  TEST_ASSERT_EQUAL(0, update_callback_1_creation_counter);
  TEST_ASSERT_EQUAL(2, update_callback_1_update_counter);
  TEST_ASSERT_EQUAL(0, update_callback_1_deletion_counter);
  TEST_ASSERT_EQUAL(new_node_2, received_callback_1_node);
  TEST_ASSERT_EQUAL(1, update_callback_2_counter);
  TEST_ASSERT_EQUAL(new_node_3, received_callback_2_node);

  // Add a new node with a matching type:
  attribute_store_node_t new_node_6;
  new_node_6 = attribute_store_add_node(ATTRIBUTE_NODE_ID, new_node_1);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, new_node_6);

  // Node creation triggered a callback for both these guys:
  TEST_ASSERT_EQUAL(3, update_callback_1_counter);
  TEST_ASSERT_EQUAL(1, update_callback_1_creation_counter);
  TEST_ASSERT_EQUAL(2, update_callback_1_update_counter);
  TEST_ASSERT_EQUAL(0, update_callback_1_deletion_counter);
  TEST_ASSERT_EQUAL(new_node_6, received_callback_1_node);
  TEST_ASSERT_EQUAL(1, update_callback_2_counter);
  TEST_ASSERT_EQUAL(new_node_3, received_callback_2_node);

  // Try deleting the node, see if the callback triggers:
  TEST_ASSERT_EQUAL(attribute_store_delete_node(new_node_6), SL_STATUS_OK);

  TEST_ASSERT_EQUAL(4, update_callback_1_counter);
  TEST_ASSERT_EQUAL(1, update_callback_1_creation_counter);
  TEST_ASSERT_EQUAL(2, update_callback_1_update_counter);
  TEST_ASSERT_EQUAL(1, update_callback_1_deletion_counter);

  // Set counters back to 0
  set_counters_to_zero();

  ///////////////////////////////////////////////////
  // Test the register_callback() functionality
  ///////////////////////////////////////////////////
  attribute_store_teardown();  // that's the only way to unregister callbacks
  TEST_ASSERT_EQUAL(SL_STATUS_OK, attribute_store_init());

  // Register callback 1 for all updates:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_register_callback(update_callback_1_event));

  // Modify a value, any value
  test_values[0] = 0x54;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_node_attribute_value(new_node_3,
                                                             DESIRED_ATTRIBUTE,
                                                             test_values,
                                                             1));

  TEST_ASSERT_EQUAL(1, update_callback_1_counter);
  TEST_ASSERT_EQUAL(0, update_callback_1_creation_counter);
  TEST_ASSERT_EQUAL(1, update_callback_1_update_counter);
  TEST_ASSERT_EQUAL(0, update_callback_1_deletion_counter);

  // Create new_node_6 again
  new_node_6 = attribute_store_add_node(ATTRIBUTE_NODE_ID, new_node_1);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, new_node_6);

  TEST_ASSERT_EQUAL(new_node_6, received_callback_1_node);

  TEST_ASSERT_EQUAL(2, update_callback_1_counter);
  TEST_ASSERT_EQUAL(1, update_callback_1_creation_counter);
  TEST_ASSERT_EQUAL(1, update_callback_1_update_counter);
  TEST_ASSERT_EQUAL(0, update_callback_1_deletion_counter);

  // Modify new_node 3 value again:
  test_values[0] = 0x55;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_node_attribute_value(new_node_3,
                                                             REPORTED_ATTRIBUTE,
                                                             test_values,
                                                             1));
  TEST_ASSERT_EQUAL(new_node_3, received_callback_1_node);

  TEST_ASSERT_EQUAL(3, update_callback_1_counter);
  TEST_ASSERT_EQUAL(1, update_callback_1_creation_counter);
  TEST_ASSERT_EQUAL(2, update_callback_1_update_counter);
  TEST_ASSERT_EQUAL(0, update_callback_1_deletion_counter);

  // Try deleting the node 6, see if the callback triggers:
  TEST_ASSERT_EQUAL(attribute_store_delete_node(new_node_6), SL_STATUS_OK);

  TEST_ASSERT_EQUAL(new_node_6, received_callback_1_node);
  TEST_ASSERT_EQUAL(4, update_callback_1_counter);
  TEST_ASSERT_EQUAL(1, update_callback_1_creation_counter);
  TEST_ASSERT_EQUAL(2, update_callback_1_update_counter);
  TEST_ASSERT_EQUAL(1, update_callback_1_deletion_counter);

  // Set counters back to 0
  set_counters_to_zero();

  ///////////////////////////////////////////////////
  // Test the attribute_store_node_exists() function
  ///////////////////////////////////////////////////
  TEST_ASSERT_TRUE(attribute_store_node_exists(new_node_1));
  TEST_ASSERT_TRUE(attribute_store_node_exists(new_node_3));
  TEST_ASSERT_FALSE(attribute_store_node_exists(ATTRIBUTE_STORE_INVALID_NODE));
  TEST_ASSERT_FALSE(attribute_store_node_exists(1648));

  ///////////////////////////////////////////////////
  // Test the delete_node() functionality
  ///////////////////////////////////////////////////
  // Delete on non-existing node / invalid node
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_delete_node(ATTRIBUTE_STORE_INVALID_NODE));

  // Delete node 2, which has 4 as child, which as 5 as child
  TEST_ASSERT_EQUAL(SL_STATUS_OK, attribute_store_delete_node(new_node_2));

  // Now we expect nodes 2, 4 and 5 to be gone:
  // The 1st child of new_node_1 is new_node_3 instead of 2
  relative_node_id
    = attribute_store_get_node_child(new_node_1,
                                     0);  // index 0 = first child

  TEST_ASSERT_EQUAL(new_node_3, relative_node_id);

  // Try to delete the root, everything will be gone but the root.
  TEST_ASSERT_EQUAL(SL_STATUS_OK, attribute_store_delete_node(root_node));

  // The root should be sitting alone in the tree
  relative_node_id
    = attribute_store_get_node_child(root_node,
                                     0);  // index 0 = first child

  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, relative_node_id);
}

void test_attribute_store_persistence()
{
  // Create a new network.
  attribute_store_delete_node(attribute_store_get_root());
  uint8_t test_values[] = {0x01, 0x02, 0x03, 0x04};
  attribute_store_node_t root_node;
  root_node = attribute_store_get_root();

  attribute_store_node_t home_id;
  home_id = attribute_store_add_node(ATTRIBUTE_HOME_ID, root_node);

  attribute_store_set_node_attribute_value(home_id,
                                           REPORTED_ATTRIBUTE,
                                           test_values,
                                           sizeof(test_values));

  attribute_store_node_t node_1;
  node_1 = attribute_store_add_node(ATTRIBUTE_NODE_ID, home_id);

  // Node 1 with identical REPORTED / DESIRED values
  test_values[0] = 0x45;
  attribute_store_set_node_attribute_value(node_1,
                                           REPORTED_ATTRIBUTE,
                                           test_values,
                                           1);
  attribute_store_set_node_attribute_value(node_1,
                                           DESIRED_ATTRIBUTE,
                                           test_values,
                                           1);

  // Node 2 with non-identical REPORTED / DESIRED values
  attribute_store_node_t node_2;
  node_2         = attribute_store_add_node(ATTRIBUTE_NODE_ID, home_id);
  test_values[0] = 0x02;
  attribute_store_set_node_attribute_value(node_2,
                                           REPORTED_ATTRIBUTE,
                                           test_values,
                                           1);

  attribute_store_set_node_attribute_value(node_2,
                                           DESIRED_ATTRIBUTE,
                                           test_values,
                                           3);

  attribute_store_log();

  // Now tear the thing down, so everything is wiped out from the in-memory
  attribute_store_teardown();

  // Call init again, it will load up from the datastore
  TEST_ASSERT_EQUAL(SL_STATUS_OK, attribute_store_init());

  // Now our attribute store has been restored.
  // If we traverse the tree we should find IDs and values identical
  attribute_store_log_node(root_node, false);  // Just for test coverage
  attribute_store_log();

  // Fetch the root:
  attribute_store_node_t new_root_node = attribute_store_get_root();
  TEST_ASSERT_EQUAL(root_node, new_root_node);
  // root should have no defined values:
  uint8_t received_values[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {0};
  uint8_t received_values_size;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_get_node_attribute_value(new_root_node,
                                             REPORTED_ATTRIBUTE,
                                             received_values,
                                             &received_values_size));
  TEST_ASSERT_EQUAL(0, received_values_size);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_get_node_attribute_value(new_root_node,
                                             DESIRED_ATTRIBUTE,
                                             received_values,
                                             &received_values_size));
  TEST_ASSERT_EQUAL(0, received_values_size);

  // Fetch the HomeID (root child)
  attribute_store_node_t new_home_id
    = attribute_store_get_node_child(new_root_node, 0);
  TEST_ASSERT_EQUAL(home_id, new_home_id);

  // HomeID must have only the REPORTED set to its homeID:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_get_node_attribute_value(new_home_id,
                                             REPORTED_ATTRIBUTE,
                                             received_values,
                                             &received_values_size));
  test_values[0] = 0x01;
  test_values[1] = 0x02;
  test_values[2] = 0x03;
  test_values[3] = 0x04;
  TEST_ASSERT_EQUAL(4, received_values_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(test_values,
                                received_values,
                                received_values_size);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_get_node_attribute_value(new_home_id,
                                             DESIRED_ATTRIBUTE,
                                             received_values,
                                             &received_values_size));
  TEST_ASSERT_EQUAL(0, received_values_size);

  // Then we should find NodeID 45 under HomeID:
  attribute_store_node_t new_node_id_1
    = attribute_store_get_node_child(new_home_id, 0);
  TEST_ASSERT_EQUAL(node_1, new_node_id_1);

  // NodeID 45 had both its desired and reported saved in the test tree
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_get_node_attribute_value(new_node_id_1,
                                             REPORTED_ATTRIBUTE,
                                             received_values,
                                             &received_values_size));
  TEST_ASSERT_EQUAL(1, received_values_size);
  test_values[0] = 0x45;
  TEST_ASSERT_EQUAL_UINT8_ARRAY(test_values,
                                received_values,
                                received_values_size);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_get_node_attribute_value(new_node_id_1,
                                             DESIRED_ATTRIBUTE,
                                             received_values,
                                             &received_values_size));
  TEST_ASSERT_EQUAL(1, received_values_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(test_values,
                                received_values,
                                received_values_size);

  // Then we should find NodeID 02 under HomeID:
  attribute_store_node_t new_node_id_2
    = attribute_store_get_node_child(new_home_id, 1);

  TEST_ASSERT_EQUAL(node_2, new_node_id_2);

  // NodeID 02 had both its desired and reported saved in the test tree
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_get_node_attribute_value(new_node_id_2,
                                             REPORTED_ATTRIBUTE,
                                             received_values,
                                             &received_values_size));
  TEST_ASSERT_EQUAL(1, received_values_size);
  test_values[0] = 0x02;
  TEST_ASSERT_EQUAL_UINT8_ARRAY(test_values,
                                received_values,
                                received_values_size);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_get_node_attribute_value(new_node_id_2,
                                             DESIRED_ATTRIBUTE,
                                             received_values,
                                             &received_values_size));
  TEST_ASSERT_EQUAL(3, received_values_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(test_values,
                                received_values,
                                received_values_size);
}

void test_attribute_store_set_node_value()
{
  // Create a couple of nodes, it's enough
  uint8_t test_values[] = {0x01, 0x02, 0x03, 0x04};
  attribute_store_node_t root_node;
  root_node = attribute_store_get_root();

  attribute_store_node_t home_id;
  home_id = attribute_store_add_node(ATTRIBUTE_HOME_ID, root_node);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_set_node_attribute_value(home_id,
                                             0x48,  // Not defined
                                             test_values,
                                             sizeof(test_values)));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_node_attribute_value(home_id,
                                             DESIRED_ATTRIBUTE,
                                             test_values,
                                             sizeof(test_values)));

  // Set to the same for test coverage
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_node_attribute_value(home_id,
                                             DESIRED_ATTRIBUTE,
                                             test_values,
                                             sizeof(test_values)));

  // Same thing with REPORTED, set x2 the same value:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_node_attribute_value(home_id,
                                             REPORTED_ATTRIBUTE,
                                             test_values,
                                             sizeof(test_values)));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_node_attribute_value(home_id,
                                             REPORTED_ATTRIBUTE,
                                             test_values,
                                             sizeof(test_values)));
}

void test_refreshing_node_and_children_callbacks()
{
  // Setting up attribute store
  update_callback_1_update_counter   = 0;
  update_callback_2_counter          = 0;
  uint32_t expecting_counter_to_be   = 4;
  uint32_t expecting_counter_2_to_be = 1;
  attribute_store_node_t root_node;
  attribute_store_node_t home_id;
  attribute_store_node_t test_id;
  attribute_store_node_t non_existent_node = 10000;

  // Adding 4 nodes and one endpoint and assigning callback to only attribute nodes
  root_node = attribute_store_get_root();
  home_id   = attribute_store_add_node(ATTRIBUTE_HOME_ID, root_node);
  attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                           home_id);
  attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                           home_id);
  test_id
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                               home_id);
  attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, test_id);
  attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                           test_id);

  attribute_store_register_callback_by_type(
    update_callback_1,
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID);

  // Only callback for REPORTED_ATTRIBUTE should be triggered from refresh function
  attribute_store_register_callback_by_type_and_state(update_callback_2,
                                                      ATTRIBUTE_ENDPOINT_ID,
                                                      DESIRED_ATTRIBUTE);
  attribute_store_register_callback_by_type_and_state(update_callback_2,
                                                      ATTRIBUTE_ENDPOINT_ID,
                                                      REPORTED_ATTRIBUTE);
  attribute_store_register_callback_by_type_and_state(
    update_callback_2,
    ATTRIBUTE_ENDPOINT_ID,
    DESIRED_OR_REPORTED_ATTRIBUTE);

  // Acting on the attribute store refreshing all callbacks for all nodes under home_id
  sl_status_t state;
  state = attribute_store_refresh_node_and_children_callbacks(home_id);

  // Checking that the refreshing worked
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
  TEST_ASSERT_EQUAL(expecting_counter_to_be, update_callback_1_update_counter);
  TEST_ASSERT_EQUAL(expecting_counter_2_to_be, update_callback_2_counter);

  // Checking fault safety for none existent node
  state
    = attribute_store_refresh_node_and_children_callbacks(non_existent_node);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  // Checking fault safety for non initialized attribute store
  attribute_store_teardown();
  state = attribute_store_refresh_node_and_children_callbacks(home_id);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);
}

void test_uninitialized_attribute_store()
{
  // Teardown the attribute store manually.
  attribute_store_teardown();

  // Then try all the functions, they should not break.
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, attribute_store_get_root());

  attribute_store_node_t test_node = 3;
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                    attribute_store_add_node(ATTRIBUTE_HOME_ID, test_node));

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_INITIALIZED,
                    attribute_store_delete_node(test_node));

  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                    attribute_store_get_node_parent(test_node));

  TEST_ASSERT_EQUAL(
    ATTRIBUTE_STORE_INVALID_NODE,
    attribute_store_get_first_parent_with_type(test_node, ATTRIBUTE_HOME_ID));

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_INITIALIZED,
                    attribute_store_set_node_attribute_value(test_node,
                                                             REPORTED_ATTRIBUTE,
                                                             NULL,
                                                             3));

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_INITIALIZED,
                    attribute_store_get_node_attribute_value(test_node,
                                                             REPORTED_ATTRIBUTE,
                                                             NULL,
                                                             NULL));

  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
                    attribute_store_get_node_type(test_node));

  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                    attribute_store_get_node_child(test_node, 0));

  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(test_node));

  TEST_ASSERT_EQUAL(
    ATTRIBUTE_STORE_INVALID_NODE,
    attribute_store_get_node_child_by_type(test_node, ATTRIBUTE_HOME_ID, 0));

  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                    attribute_store_get_node_child_by_value(test_node,
                                                            ATTRIBUTE_HOME_ID,
                                                            REPORTED_ATTRIBUTE,
                                                            NULL,
                                                            0,
                                                            0));
  TEST_ASSERT_EQUAL(false, attribute_store_node_exists(test_node));
}

void test_attribute_store_delete_nodes_and_read_their_value()
{
  attribute_store_node_t node_1
    = attribute_store_add_node(0x01, attribute_store_get_root());
  attribute_store_node_t node_2 = attribute_store_add_node(0x02, node_1);

  uint8_t test_value = 0x01;
  attribute_store_set_node_attribute_value(node_2,
                                           REPORTED_ATTRIBUTE,
                                           &test_value,
                                           sizeof(test_value));
  test_value = 0x02;
  attribute_store_set_node_attribute_value(node_2,
                                           DESIRED_ATTRIBUTE,
                                           &test_value,
                                           sizeof(test_value));

  // register a callback for deletion of node 2
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_register_callback_by_type(
                      update_callback_read_deleted_value,
                      0x02));

  // Delete node 1
  TEST_ASSERT_EQUAL(SL_STATUS_OK, attribute_store_delete_node(node_1));
  TEST_ASSERT_EQUAL(1, update_callback_read_deleted_value_counter);
}

void test_attribute_store_delete_nodes_and_try_to_add_a_child()
{
  attribute_store_node_t node_1
    = attribute_store_add_node(0x55, attribute_store_get_root());
  attribute_store_add_node(0x57, node_1);

  // register a callback for deletion of node 2
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_callback_by_type(update_callback_add_child, 0x57));

  // Delete node 1
  attribute_store_delete_node(node_1);
  TEST_ASSERT_EQUAL(1, update_callback_add_child_value_counter);
}

void test_attribute_store_register_nullptr_callbacks()
{
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, attribute_store_register_callback(NULL));
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    attribute_store_register_callback_by_type(NULL, 1));
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_register_callback_by_type_and_state(NULL,
                                                        1,
                                                        REPORTED_ATTRIBUTE));

  // Verify that nothing crashes
  attribute_store_node_t node_1
    = attribute_store_add_node(1, attribute_store_get_root());
  uint8_t test_value = 0x01;
  attribute_store_set_node_attribute_value(node_1,
                                           REPORTED_ATTRIBUTE,
                                           &test_value,
                                           sizeof(test_value));
}

void test_attribute_store_double_delete()
{
  attribute_store_delete_node(attribute_store_get_root());
  attribute_store_node_t node_1
    = attribute_store_add_node(1, attribute_store_get_root());
  attribute_store_node_t node_2 = attribute_store_add_node(2, node_1);
  attribute_store_add_node(3, node_1);
  attribute_store_add_node(4, node_2);

  // Register callbacks.
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_callback_by_type(&update_callback_delete_node, 2));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_register_callback(&update_callback_1_event));

  // Trigger a full refresh, node 2 gets clipped as part of the callback.
  attribute_store_refresh_node_and_children_callbacks(node_1);

  // Node 1,2 triggered a callback. 3 does not get detected due to tree change.
  TEST_ASSERT_EQUAL(2, update_callback_1_update_counter);
  // Node 2 update + delete triggered callbacks to the update_callback_delete_node
  TEST_ASSERT_EQUAL(2, update_callback_delete_node_counter);
  // Node 2 deletion made 2 callbacks, deletion of node 2 and node 4.
  TEST_ASSERT_EQUAL(2, update_callback_1_deletion_counter);
}

void test_attribute_store_touch_generic_callbacks()
{
  attribute_store_delete_node(attribute_store_get_root());
  attribute_store_node_t node_1
    = attribute_store_add_node(1, attribute_store_get_root());
  attribute_store_node_t node_2 = attribute_store_add_node(2, node_1);
  attribute_store_node_t node_3 = attribute_store_add_node(3, node_1);
  attribute_store_node_t node_4 = attribute_store_add_node(4, node_2);

  // Register callbacks.
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    attribute_store_register_touch_notification_callback(NULL));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_touch_notification_callback(&touch_callback));

  TEST_ASSERT_EQUAL(0, touch_callback_counter);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, received_touched_node);

  // Trigger a full refresh, no touch notifications
  attribute_store_refresh_node_and_children_callbacks(node_1);
  TEST_ASSERT_EQUAL(0, touch_callback_counter);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, received_touched_node);

  // Set node 2 to nothing, same value as before.
  attribute_store_set_node_attribute_value(node_2, DESIRED_ATTRIBUTE, NULL, 0);
  TEST_ASSERT_EQUAL(1, touch_callback_counter);
  TEST_ASSERT_EQUAL(node_2, received_touched_node);

  // Set node 3 to nothing, same value as before.
  attribute_store_set_node_attribute_value(node_3, REPORTED_ATTRIBUTE, NULL, 0);
  TEST_ASSERT_EQUAL(2, touch_callback_counter);
  TEST_ASSERT_EQUAL(node_3, received_touched_node);

  // Set node 4 to something, expect no touch callback
  uint8_t value = 23;
  attribute_store_set_node_attribute_value(node_4,
                                           REPORTED_ATTRIBUTE,
                                           &value,
                                           sizeof(value));
  TEST_ASSERT_EQUAL(2, touch_callback_counter);
  TEST_ASSERT_EQUAL(node_3, received_touched_node);

  // Set node 4 to the same again, now we get a touch notification
  attribute_store_set_node_attribute_value(node_4,
                                           REPORTED_ATTRIBUTE,
                                           &value,
                                           sizeof(value));
  TEST_ASSERT_EQUAL(3, touch_callback_counter);
  TEST_ASSERT_EQUAL(node_4, received_touched_node);

  // Delete node 3
  attribute_store_delete_node(node_3);
  TEST_ASSERT_EQUAL(3, touch_callback_counter);
  TEST_ASSERT_EQUAL(node_4, received_touched_node);
}