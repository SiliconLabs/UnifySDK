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
#include "zwave_command_class_association.h"
#include "zwave_command_class_multi_channel_association.h"
#include "zwave_command_class_association_internals.h"
#include "zwave_command_classes_utils.h"

// Includes from other components
#include "unity.h"
#include "sl_log.h"

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"

// Interface includes
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"
#include "zwave_utils.h"
#include "zwave_controller_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "zwave_network_management_mock.h"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_##type

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  zwave_unid_set_home_id(home_id);
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
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);
  zwave_network_management_get_node_id_IgnoreAndReturn(zpc_node_id);
  zpc_attribute_store_test_helper_create_network();
}

/**
 * @brief Create group data under the endpoint and returns the Group Content node.
 *
 * @param endpoint_node       Attribute Store node for the endpoint
 * @param group_id            Group ID to configure
 * @param group_capacity      Total number of associations for the group
 *
 * @returns group_content_node.
 */
static attribute_store_node_t
  setup_group(attribute_store_node_t endpoint_node,
              association_group_id_t group_id,
              association_group_capacity_t group_capacity)
{
  attribute_store_node_t group_id_node
    = attribute_store_add_node(ATTRIBUTE(GROUP_ID), endpoint_id_node);
  attribute_store_set_reported(group_id_node, &group_id, sizeof(group_id));

  attribute_store_node_t group_capacity_node_1
    = attribute_store_add_node(ATTRIBUTE(MAX_NODES_SUPPORTED), group_id_node);
  attribute_store_set_reported(group_capacity_node_1,
                               &group_capacity,
                               sizeof(group_capacity));

  return attribute_store_add_node(ATTRIBUTE(GROUP_CONTENT), group_id_node);
}

/**
 * @brief Create max number of group data under the endpoint and returns
 * the "Number of groups" node
 *
 * @param endpoint_node       Attribute Store node for the endpoint
 * @param group_id            Group ID to configure
 * @param group_capacity      Total number of associations for the group
 *
 * @returns group_content_node.
 */
static attribute_store_node_t
  setup_grouping_number(attribute_store_node_t endpoint_node,
                        association_group_count_t number_of_groups)
{
  attribute_store_node_t supported_groupings_node
    = attribute_store_add_node(ATTRIBUTE(SUPPORTED_GROUPINGS),
                               endpoint_id_node);

  attribute_store_set_reported(supported_groupings_node,
                               &number_of_groups,
                               sizeof(number_of_groups));

  return supported_groupings_node;
}

void test_add_desired_in_all_groups()
{
  // 3 groups
  setup_grouping_number(endpoint_id_node, 3);
  setup_group(endpoint_id_node, 2, 0);
  setup_group(endpoint_id_node, 3, 0);

  attribute_store_node_t group_content_node_1
    = setup_group(endpoint_id_node, 1, 50);

  // Group content : node 2, node 1, node 1:2, node 2:1
  uint8_t group_1_content[] = {0x02, 0x01, 0x00, 0x01, 0x02, 0x02, 0x01};
  attribute_store_set_reported(group_content_node_1,
                               &group_1_content,
                               sizeof(group_1_content));

  // Run the test
  association_t new_association = {};
  new_association.node_id       = 4;
  new_association.endpoint_id   = 3;
  new_association.type          = NODE_ID;
  add_desired_association_in_all_groups(node_id, endpoint_id, new_association);

  // Association sets have the side effect to re-order associations with ascending
  // NodeIDs.
  const uint8_t expected_new_group_content[]
    = {0x01, 0x02, 0x04, 0x00, 0x01, 0x02, 0x02, 0x01};
  attribute_store_get_desired(group_content_node_1,
                              group_1_content,
                              sizeof(expected_new_group_content));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_new_group_content,
                                group_1_content,
                                sizeof(expected_new_group_content));
}

void test_add_desired_in_all_groups_endpoint_association()
{
  // Create some group data under our supporting node
  setup_grouping_number(endpoint_id_node, 3);
  setup_group(endpoint_id_node, 2, 0);
  setup_group(endpoint_id_node, 3, 0);

  attribute_store_node_t group_content_node_1
    = setup_group(endpoint_id_node, 1, 50);

  // Group content : node 2, node 1, node 1:2, node 2:1
  uint8_t group_1_content[] = {0x02, 0x01, 0x00, 0x01, 0x02, 0x02, 0x01};
  attribute_store_set_reported(group_content_node_1,
                               &group_1_content,
                               sizeof(group_1_content));

  // Run the test
  association_t new_association = {};
  new_association.node_id       = 1;
  new_association.endpoint_id   = 2;
  new_association.type          = ENDPOINT;
  add_desired_association_in_all_groups(node_id, endpoint_id, new_association);

  // Association sets have the side effect to re-order associations with ascending
  // NodeIDs.
  const uint8_t expected_new_group_content[]
    = {0x01, 0x02, 0x00, 0x01, 0x02, 0x02, 0x01};
  attribute_store_get_desired(group_content_node_1,
                              group_1_content,
                              sizeof(expected_new_group_content));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_new_group_content,
                                group_1_content,
                                sizeof(expected_new_group_content));
}

void test_remove_desired_node_id_from_all_associations()
{
  // Create some group data under our supporting node
  setup_grouping_number(endpoint_id_node, 3);

  attribute_store_node_t group_content_node_1
    = setup_group(endpoint_id_node, 1, 50);
  attribute_store_node_t group_content_node_2
    = setup_group(endpoint_id_node, 2, 2);
  attribute_store_node_t group_content_node_3
    = setup_group(endpoint_id_node, 3, 3);

  // Group content : node 2, node 1, node 1:2, node 2:1
  uint8_t group_1_content[] = {0x02, 0x01, 0x00, 0x01, 0x02, 0x02, 0x01};
  attribute_store_set_reported(group_content_node_1,
                               &group_1_content,
                               sizeof(group_1_content));

  // Group content : node 5, node 3:2, node 23:1
  uint8_t group_2_content[] = {5, 0x00, 3, 2, 23, 1};
  attribute_store_set_reported(group_content_node_2,
                               &group_2_content,
                               sizeof(group_2_content));

  // Group content : node 5, node 3:2, node 23:1
  uint8_t group_3_content[] = {1, 2, 3, 4, 5, 6};
  attribute_store_set_reported(group_content_node_3,
                               &group_3_content,
                               sizeof(group_3_content));

  // Run the test, remove NodeID 2 from everywhere
  remove_desired_node_id_from_all_associations(2, node_id, endpoint_id);

  // Association sets have the side effect to re-order associations with ascending
  // NodeIDs.
  const uint8_t expected_new_group_1_content[] = {0x01, 0x00, 0x01, 0x02};
  attribute_store_get_desired(group_content_node_1,
                              group_1_content,
                              sizeof(expected_new_group_1_content));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_new_group_1_content,
                                group_1_content,
                                sizeof(expected_new_group_1_content));

  const uint8_t expected_new_group_2_content[] = {5, 0x00, 3, 2, 23, 1};
  attribute_store_get_desired(group_content_node_2,
                              group_2_content,
                              sizeof(expected_new_group_2_content));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_new_group_2_content,
                                group_2_content,
                                sizeof(expected_new_group_2_content));

  const uint8_t expected_new_group_3_content[] = {1, 3, 4, 5, 6};
  attribute_store_get_desired(group_content_node_3,
                              group_3_content,
                              sizeof(expected_new_group_3_content));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_new_group_3_content,
                                group_3_content,
                                sizeof(expected_new_group_3_content));
}

void test_remove_desired_node_id_from_all_associations_in_network()
{
  // Create some group data under our supporting node
  setup_grouping_number(endpoint_id_node, 3);

  attribute_store_node_t group_content_node_1
    = setup_group(endpoint_id_node, 1, 50);
  attribute_store_node_t group_content_node_2
    = setup_group(endpoint_id_node, 2, 2);
  attribute_store_node_t group_content_node_3
    = setup_group(endpoint_id_node, 3, 3);

  // Group content : node 2, node 1, node 1:2, node 2:1
  uint8_t group_1_content[] = {0x02, 0x01, 0x00, 0x01, 0x02, 0x02, 0x01};
  attribute_store_set_reported(group_content_node_1,
                               &group_1_content,
                               sizeof(group_1_content));

  // Group content : node 5, node 3:2, node 23:1
  uint8_t group_2_content[] = {5, 0x00, 3, 2, 23, 1};
  attribute_store_set_reported(group_content_node_2,
                               &group_2_content,
                               sizeof(group_2_content));

  // Group content : node 5, node 3:2, node 23:1
  uint8_t group_3_content[] = {1, 2, 3, 4, 5, 6};
  attribute_store_set_reported(group_content_node_3,
                               &group_3_content,
                               sizeof(group_3_content));

  // Run the test, remove NodeID 2 from everywhere
  remove_desired_node_id_from_all_associations_in_network(2);

  // Association sets have the side effect to re-order associations with ascending
  // NodeIDs.
  const uint8_t expected_new_group_1_content[] = {0x01, 0x00, 0x01, 0x02};
  attribute_store_get_desired(group_content_node_1,
                              group_1_content,
                              sizeof(expected_new_group_1_content));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_new_group_1_content,
                                group_1_content,
                                sizeof(expected_new_group_1_content));

  const uint8_t expected_new_group_2_content[] = {5, 0x00, 3, 2, 23, 1};
  attribute_store_get_desired(group_content_node_2,
                              group_2_content,
                              sizeof(expected_new_group_2_content));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_new_group_2_content,
                                group_2_content,
                                sizeof(expected_new_group_2_content));

  const uint8_t expected_new_group_3_content[] = {1, 3, 4, 5, 6};
  attribute_store_get_desired(group_content_node_3,
                              group_3_content,
                              sizeof(expected_new_group_3_content));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_new_group_3_content,
                                group_3_content,
                                sizeof(expected_new_group_3_content));
}

void test_is_association_valid_and_remove_all_reported_associations()
{
  // Create some group data under our supporting node
  setup_grouping_number(endpoint_id_node, 1);

  attribute_store_node_t group_content_node_1
    = setup_group(endpoint_id_node, 1, 50);

  association_t test_association;
  test_association.node_id     = 0;
  test_association.endpoint_id = 0;
  test_association.type        = ENDPOINT;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_reported_association(node_id, endpoint_id, 1, test_association));

  test_association.node_id = 233;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_reported_association(node_id, endpoint_id, 1, test_association));

  test_association.node_id = 232;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_reported_association(node_id, endpoint_id, 1, test_association));

  test_association.node_id = 1;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_reported_association(node_id, endpoint_id, 1, test_association));

  test_association.type = NODE_ID;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_reported_association(node_id, endpoint_id, 1, test_association));

  test_association.type = ENDPOINT + 1;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_reported_association(node_id, endpoint_id, 1, test_association));

  const uint8_t expected_new_group_1_content[] = {1, 0x00, 1, 0, 232, 0};
  uint8_t group_1_content[50];
  attribute_store_get_reported(group_content_node_1,
                               group_1_content,
                               sizeof(expected_new_group_1_content));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_new_group_1_content,
                                group_1_content,
                                sizeof(expected_new_group_1_content));

  // Now remove all associations
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    remove_all_reported_associations(node_id, endpoint_id, 1));
  const uint8_t expected_new_group_1_content_2[] = {0x00};
  attribute_store_get_reported(group_content_node_1,
                               group_1_content,
                               sizeof(expected_new_group_1_content_2));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_new_group_1_content_2,
                                group_1_content,
                                sizeof(expected_new_group_1_content_2));
}

void test_remove_all_desired_associations()
{
  // Create some group data under our supporting node
  setup_grouping_number(endpoint_id_node, 1);

  attribute_store_node_t group_content_node_1
    = setup_group(endpoint_id_node, 1, 50);

  association_t test_association;
  test_association.node_id     = 2;
  test_association.endpoint_id = 2;
  test_association.type        = ENDPOINT;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_desired_association(node_id, endpoint_id, 1, test_association));

  test_association.node_id     = 3;
  test_association.endpoint_id = 3;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_desired_association(node_id, endpoint_id, 1, test_association));

  test_association.node_id     = 4;
  test_association.endpoint_id = 4;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_desired_association(node_id, endpoint_id, 1, test_association));

  test_association.node_id     = 5;
  test_association.endpoint_id = 5;
  test_association.type        = NODE_ID;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_desired_association(node_id, endpoint_id, 1, test_association));

  const uint8_t expected_new_group_1_content[] = {5, 0x00, 2, 2, 3, 3, 4, 4};
  uint8_t group_1_content[50];
  attribute_store_get_desired(group_content_node_1,
                              group_1_content,
                              sizeof(expected_new_group_1_content));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_new_group_1_content,
                                group_1_content,
                                sizeof(expected_new_group_1_content));

  // Now remove all associations
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    remove_all_desired_associations(node_id, endpoint_id, 1));
  const uint8_t expected_new_group_1_content_2[] = {0x00};
  attribute_store_get_desired(group_content_node_1,
                              group_1_content,
                              sizeof(expected_new_group_1_content_2));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_new_group_1_content_2,
                                group_1_content,
                                sizeof(expected_new_group_1_content_2));
}

void test_remove_all_reported_node_id_associations()
{
  // Create some group data under our supporting node
  setup_grouping_number(endpoint_id_node, 1);

  attribute_store_node_t group_content_node_1
    = setup_group(endpoint_id_node, 1, 50);

  association_t test_association;
  test_association.node_id     = 2;
  test_association.endpoint_id = 2;
  test_association.type        = ENDPOINT;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_reported_association(node_id, endpoint_id, 1, test_association));

  test_association.node_id     = 3;
  test_association.endpoint_id = 3;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_reported_association(node_id, endpoint_id, 1, test_association));

  test_association.node_id     = 4;
  test_association.endpoint_id = 4;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_reported_association(node_id, endpoint_id, 1, test_association));

  test_association.node_id     = 5;
  test_association.endpoint_id = 5;
  test_association.type        = NODE_ID;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_reported_association(node_id, endpoint_id, 1, test_association));

  test_association.node_id = 6;
  test_association.type    = NODE_ID;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_reported_association(node_id, endpoint_id, 1, test_association));

  test_association.node_id = 7;
  test_association.type    = NODE_ID;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_reported_association(node_id, endpoint_id, 1, test_association));

  const uint8_t expected_new_group_1_content[]
    = {5, 6, 7, 0x00, 2, 2, 3, 3, 4, 4};
  uint8_t group_1_content[50];
  attribute_store_get_reported(group_content_node_1,
                               group_1_content,
                               sizeof(expected_new_group_1_content));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_new_group_1_content,
                                group_1_content,
                                sizeof(expected_new_group_1_content));

  // Now remove all associations
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    remove_all_reported_node_id_associations(node_id, endpoint_id, 1));
  const uint8_t expected_new_group_1_content_2[] = {0x00, 2, 2, 3, 3, 4, 4};

  attribute_store_get_reported(group_content_node_1,
                               group_1_content,
                               sizeof(expected_new_group_1_content_2));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_new_group_1_content_2,
                                group_1_content,
                                sizeof(expected_new_group_1_content_2));
}

void test_is_reported_group_full()
{
  // Create some group data under our supporting node
  setup_grouping_number(endpoint_id_node, 1);
  setup_group(endpoint_id_node, 1, 2);

  association_t test_association;
  test_association.node_id     = 2;
  test_association.endpoint_id = 2;
  test_association.type        = ENDPOINT;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_reported_association(node_id, endpoint_id, 1, test_association));

  TEST_ASSERT_FALSE(is_reported_group_full(node_id, endpoint_id, 1));

  test_association.node_id = 3;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_reported_association(node_id, endpoint_id, 1, test_association));

  TEST_ASSERT_TRUE(is_reported_group_full(node_id, endpoint_id, 1));
}

void test_is_desired_group_full()
{
  // Create some group data under our supporting node
  setup_grouping_number(endpoint_id_node, 1);
  setup_group(endpoint_id_node, 1, 2);

  association_t test_association;
  test_association.node_id     = 2;
  test_association.endpoint_id = 2;
  test_association.type        = ENDPOINT;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_desired_association(node_id, endpoint_id, 1, test_association));

  TEST_ASSERT_FALSE(is_desired_group_full(node_id, endpoint_id, 1));

  test_association.node_id = 3;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_desired_association(node_id, endpoint_id, 1, test_association));

  TEST_ASSERT_TRUE(is_desired_group_full(node_id, endpoint_id, 1));
}

void test_force_add_desired_association()
{
  // Create some group data under our supporting node
  setup_grouping_number(endpoint_id_node, 1);
  attribute_store_node_t group_content_node_1
    = setup_group(endpoint_id_node, 1, 1);

  association_t test_association;
  test_association.node_id     = zpc_node_id;
  test_association.endpoint_id = 2;
  test_association.type        = ENDPOINT;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    force_add_desired_association(node_id, endpoint_id, 1, test_association));

  // Group is now full. Try to force add, it won't remove us for somebody else
  test_association.node_id = 3;
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    force_add_desired_association(node_id, endpoint_id, 1, test_association));

  // Clear up the group
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    remove_all_desired_associations(node_id, endpoint_id, 1));

  test_association.node_id = 3;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    force_add_desired_association(node_id, endpoint_id, 1, test_association));

  // Now we will bump NodeID 3 out for ourselves
  test_association.node_id = zpc_node_id;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    force_add_desired_association(node_id, endpoint_id, 1, test_association));

  const uint8_t expected_new_group_1_content[] = {0x00, 0x01, 0x02};
  uint8_t group_1_content[3]                   = {};
  attribute_store_get_desired(group_content_node_1,
                              group_1_content,
                              sizeof(expected_new_group_1_content));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_new_group_1_content,
                                group_1_content,
                                sizeof(expected_new_group_1_content));
}

void test_remove_desired_association_in_all_groups()
{
  // Create some group data under our supporting node
  setup_grouping_number(endpoint_id_node, 3);

  attribute_store_node_t group_content_node_1
    = setup_group(endpoint_id_node, 1, 50);
  attribute_store_node_t group_content_node_2
    = setup_group(endpoint_id_node, 2, 2);
  attribute_store_node_t group_content_node_3
    = setup_group(endpoint_id_node, 3, 3);

  // Group content : node 2, node 1, node 1:2, node 2:1
  uint8_t group_1_content[] = {0x02, 0x01, 0x00, 0x01, 0x02, 0x02, 0x01};
  attribute_store_set_reported(group_content_node_1,
                               &group_1_content,
                               sizeof(group_1_content));

  // Group content : node 5, node 3:2, node 23:1
  uint8_t group_2_content[] = {5, 0x00, 3, 2, 23, 1};
  attribute_store_set_reported(group_content_node_2,
                               &group_2_content,
                               sizeof(group_2_content));

  // Group content : node 5, node 3:2, node 23:1
  uint8_t group_3_content[] = {1, 2, 3, 4, 5, 6};
  attribute_store_set_reported(group_content_node_3,
                               &group_3_content,
                               sizeof(group_3_content));

  // Run the test, remove NodeID 2 from everywhere
  association_t association_to_remove = {};
  association_to_remove.node_id       = 2;
  association_to_remove.type          = NODE_ID;
  remove_desired_association_in_all_groups(node_id,
                                           endpoint_id,
                                           association_to_remove);

  // Association sets have the side effect to re-order associations with ascending
  // NodeIDs.
  const uint8_t expected_new_group_1_content[]
    = {0x01, 0x00, 0x01, 0x02, 0x02, 0x01};
  attribute_store_get_desired(group_content_node_1,
                              group_1_content,
                              sizeof(expected_new_group_1_content));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_new_group_1_content,
                                group_1_content,
                                sizeof(expected_new_group_1_content));

  const uint8_t expected_new_group_2_content[] = {5, 0x00, 3, 2, 23, 1};
  attribute_store_get_desired(group_content_node_2,
                              group_2_content,
                              sizeof(expected_new_group_2_content));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_new_group_2_content,
                                group_2_content,
                                sizeof(expected_new_group_2_content));

  const uint8_t expected_new_group_3_content[] = {1, 3, 4, 5, 6};
  attribute_store_get_desired(group_content_node_3,
                              group_3_content,
                              sizeof(expected_new_group_3_content));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_new_group_3_content,
                                group_3_content,
                                sizeof(expected_new_group_3_content));
}