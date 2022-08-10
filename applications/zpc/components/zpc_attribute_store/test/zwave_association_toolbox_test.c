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
#include "zwave_command_class_association_helper.h"
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

  // Group is now full. Try to force add, it will remove the existing
  // in favor or a new one.
  test_association.node_id = 3;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
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

void test_is_association_in_group()
{
  // Create some group data under our supporting node
  setup_grouping_number(endpoint_id_node, 1);
  attribute_store_node_t group_content_node_1
    = setup_group(endpoint_id_node, 1, 10);

  association_t test_association;
  test_association.node_id     = zpc_node_id;
  test_association.endpoint_id = 2;
  test_association.type        = ENDPOINT;

  TEST_ASSERT_FALSE(
    is_association_in_group(node_id, endpoint_id, 1, test_association));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_desired_association(node_id, endpoint_id, 1, test_association));

  TEST_ASSERT_TRUE(
    is_association_in_group(node_id, endpoint_id, 1, test_association));

  TEST_ASSERT_FALSE(
    is_association_in_group(node_id + 1, endpoint_id + 2, 1, test_association));
  TEST_ASSERT_FALSE(
    is_association_in_group(node_id, endpoint_id, 2, test_association));

  attribute_store_undefine_desired(group_content_node_1);
  TEST_ASSERT_FALSE(
    is_association_in_group(node_id, endpoint_id, 1, test_association));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_reported_association(node_id, endpoint_id, 1, test_association));

  TEST_ASSERT_TRUE(
    is_association_in_group(node_id, endpoint_id, 1, test_association));

  test_association.type = NODE_ID;
  TEST_ASSERT_FALSE(
    is_association_in_group(node_id, endpoint_id, 1, test_association));
}

void test_is_association_legal()
{
  association_t test_association = {};
  test_association.node_id       = zpc_node_id;
  test_association.endpoint_id   = 0;
  test_association.type          = NODE_ID;

  // Towards ZPC is always valid:
  TEST_ASSERT_TRUE(
    is_association_legal(node_id, endpoint_id, 1, test_association));

  test_association.endpoint_id = 23;
  test_association.type        = ENDPOINT;
  TEST_ASSERT_TRUE(
    is_association_legal(node_id, endpoint_id, 1, test_association));

  // Association towards the node itself is not okay:
  test_association.node_id = node_id;
  TEST_ASSERT_FALSE(
    is_association_legal(node_id, endpoint_id, 1, test_association));

  // Create a NodeID 7:
  const zwave_node_id_t node_id_7 = 7;
  attribute_store_node_t node_id_7_node
    = attribute_store_emplace(home_id_node,
                              ATTRIBUTE_NODE_ID,
                              &node_id_7,
                              sizeof(node_id_7));

  const zwave_endpoint_id_t endpoint_id_2 = 2;
  attribute_store_node_t endpoint_2_node
    = attribute_store_emplace(node_id_7_node,
                              ATTRIBUTE_ENDPOINT_ID,
                              &endpoint_id_2,
                              sizeof(endpoint_id_2));

  // Make NodeID 7 an NL Node:
  uint8_t listening_protocol = 0;
  attribute_store_set_child_reported(node_id_7_node,
                                     ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
                                     &listening_protocol,
                                     sizeof(listening_protocol));
  attribute_store_set_child_reported(node_id_7_node,
                                     ATTRIBUTE_ZWAVE_OPTIONAL_PROTOCOL,
                                     &listening_protocol,
                                     sizeof(listening_protocol));

  // Try to associate towards NodeID 7, it should not be happy for NL nodes.
  test_association.node_id = node_id_7;
  TEST_ASSERT_FALSE(
    is_association_legal(node_id, endpoint_id, 1, test_association));

  // Now make the NodeID 7 AL, but unkonwn protocol will prevent the association
  // to be valid
  listening_protocol = ZWAVE_NODE_INFO_LISTENING_PROTOCOL_LISTENING_MASK;
  attribute_store_set_child_reported(node_id_7_node,
                                     ATTRIBUTE_ZWAVE_PROTOCOL_LISTENING,
                                     &listening_protocol,
                                     sizeof(listening_protocol));

  TEST_ASSERT_FALSE(
    is_association_legal(node_id, endpoint_id, 1, test_association));

  // Get NodeID 7 to operate on Z-Wave, still not valid
  zwave_protocol_t protocol = PROTOCOL_ZWAVE;
  attribute_store_set_child_reported(node_id_7_node,
                                     ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                                     &protocol,
                                     sizeof(protocol));
  TEST_ASSERT_FALSE(
    is_association_legal(node_id, endpoint_id, 1, test_association));

  // Now they are both running Z-Wave:
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_ZWAVE_INCLUSION_PROTOCOL,
                                     &protocol,
                                     sizeof(protocol));

  // Next stop, granted keys:
  zwave_keyset_t granted_keys = 0;
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_GRANTED_SECURITY_KEYS,
                                     &granted_keys,
                                     sizeof(granted_keys));
  granted_keys = 0x83;
  attribute_store_set_child_reported(node_id_7_node,
                                     ATTRIBUTE_GRANTED_SECURITY_KEYS,
                                     &granted_keys,
                                     sizeof(granted_keys));

  TEST_ASSERT_FALSE(
    is_association_legal(node_id, endpoint_id, 1, test_association));

  granted_keys = 0x03;  // Same highest as 0x83
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_GRANTED_SECURITY_KEYS,
                                     &granted_keys,
                                     sizeof(granted_keys));

  // Multi channel capabilities, with no support, it will be not accepted:
  test_association.type        = ENDPOINT;
  test_association.endpoint_id = endpoint_id_2;
  TEST_ASSERT_FALSE(
    is_association_legal(node_id, endpoint_id, 1, test_association));

  // If not Endpoint Type, then we need at least 1 mapped command:
  test_association.type        = NODE_ID;
  test_association.endpoint_id = endpoint_id_2;
  TEST_ASSERT_FALSE(
    is_association_legal(node_id, endpoint_id, 1, test_association));

  // Setup a group 1 under our source NodeID
  setup_grouping_number(endpoint_id_node, 1);
  const association_group_id_t group_id_1 = 1;
  attribute_store_node_t group_id_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(GROUP_ID),
                              &group_id_1,
                              sizeof(group_id_1));
  attribute_store_node_t command_list_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST,
    group_id_node);
  const uint8_t command_list[] = {COMMAND_CLASS_DOOR_LOCK,
                                  DOOR_LOCK_OPERATION_REPORT,
                                  COMMAND_CLASS_DOOR_LOCK,
                                  DOOR_LOCK_CONFIGURATION_REPORT};
  attribute_store_set_reported(command_list_node,
                               command_list,
                               sizeof(command_list));

  // The command list is all supporting, nothing to be mapped.
  TEST_ASSERT_FALSE(
    is_association_legal(node_id, endpoint_id, 1, test_association));

  const uint8_t command_list_controlling[] = {
    COMMAND_CLASS_SWITCH_MULTILEVEL,
    SWITCH_MULTILEVEL_SET,
    COMMAND_CLASS_SWITCH_MULTILEVEL,
    SWITCH_MULTILEVEL_START_LEVEL_CHANGE,
    COMMAND_CLASS_SWITCH_MULTILEVEL,
    SWITCH_MULTILEVEL_STOP_LEVEL_CHANGE,
    COMMAND_CLASS_SWITCH_MULTILEVEL,
    SWITCH_MULTILEVEL_START_LEVEL_CHANGE,
  };
  attribute_store_set_reported(command_list_node,
                               command_list_controlling,
                               sizeof(command_list_controlling));

  // The command list is controlling, mapped, but not supported by the destination
  TEST_ASSERT_FALSE(
    is_association_legal(node_id, endpoint_id, 1, test_association));

  // Add Multilevel Switch support to the destination.
  const uint8_t nif[] = {0xFE,
                         0x25,
                         COMMAND_CLASS_MULTI_CHANNEL_V4,
                         COMMAND_CLASS_SWITCH_MULTILEVEL,
                         COMMAND_CLASS_DOOR_LOCK,
                         COMMAND_CLASS_SECURITY_2};
  attribute_store_set_child_reported(endpoint_2_node,
                                     ATTRIBUTE_ZWAVE_NIF,
                                     nif,
                                     sizeof(nif));

  // Finally, we made it !
  // The command list is controlling, mapped, and supported by the destination
  TEST_ASSERT_TRUE(
    is_association_legal(node_id, endpoint_id, 1, test_association));

  // Add multi channel capabilities to both nodes
  test_association.type = ENDPOINT;
  const uint8_t nif_multi_channel[]
    = {COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V3,
       COMMAND_CLASS_MULTI_CHANNEL_V4};
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE_ZWAVE_NIF,
                                     nif_multi_channel,
                                     sizeof(nif_multi_channel));
  const zwave_endpoint_id_t endpoint_id_0 = 0;
  attribute_store_node_t node_7_endpoint_0_node
    = attribute_store_emplace(node_id_7_node,
                              ATTRIBUTE_ENDPOINT_ID,
                              &endpoint_id_0,
                              sizeof(endpoint_id_0));
  attribute_store_set_child_reported(node_7_endpoint_0_node,
                                     ATTRIBUTE_ZWAVE_NIF,
                                     nif_multi_channel,
                                     sizeof(nif_multi_channel));

  // Should still be true:
  TEST_ASSERT_TRUE(
    is_association_legal(node_id, endpoint_id, 1, test_association));

  // Association with Basic CC should be legal:
  const uint8_t basic_command_list[] = {COMMAND_CLASS_BASIC, BASIC_SET};
  attribute_store_set_reported(command_list_node,
                               basic_command_list,
                               sizeof(basic_command_list));
  TEST_ASSERT_TRUE(
    is_association_legal(node_id, endpoint_id, 1, test_association));

  // Try with a non-existing endpoint:
  test_association.endpoint_id = 23;
  TEST_ASSERT_FALSE(
    is_association_legal(node_id, endpoint_id, 1, test_association));
}

void test_is_association_valid()
{
  association_t test_association = {};
  test_association.node_id       = zpc_node_id;
  test_association.endpoint_id   = 0;
  test_association.type          = NODE_ID;

  // is_association_legal returns the result of is_association_valid when the destination is us
  TEST_ASSERT_TRUE(
    is_association_legal(node_id, endpoint_id, 1, test_association));

  test_association.endpoint_id = ZWAVE_MAXIMUM_ENDPOINT_ID + 1;
  TEST_ASSERT_TRUE(
    is_association_legal(node_id, endpoint_id, 1, test_association));

  test_association.type = ENDPOINT;
  TEST_ASSERT_FALSE(
    is_association_legal(node_id, endpoint_id, 1, test_association));

  test_association.type        = ENDPOINT + 25;
  test_association.endpoint_id = ZWAVE_MAXIMUM_ENDPOINT_ID;
  TEST_ASSERT_FALSE(
    is_association_legal(node_id, endpoint_id, 1, test_association));

  test_association.type        = ENDPOINT;
  test_association.endpoint_id = ZWAVE_MAXIMUM_ENDPOINT_ID;
  TEST_ASSERT_TRUE(
    is_association_legal(node_id, endpoint_id, 1, test_association));

  test_association.node_id     = ZW_MAX_NODES;
  test_association.endpoint_id = 0;
  test_association.type        = NODE_ID;
  zwave_network_management_get_node_id_StopIgnore();
  zwave_network_management_get_node_id_ExpectAndReturn(
    test_association.node_id);

  TEST_ASSERT_TRUE(
    is_association_legal(node_id, endpoint_id, 1, test_association));

  test_association.node_id     = ZW_MAX_NODES + 1;
  test_association.endpoint_id = 0;
  test_association.type        = NODE_ID;
  zwave_network_management_get_node_id_StopIgnore();
  zwave_network_management_get_node_id_ExpectAndReturn(
    test_association.node_id);

  TEST_ASSERT_FALSE(
    is_association_legal(node_id, endpoint_id, 1, test_association));
}
