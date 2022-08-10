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
#include "binding_cluster_mapper_helper.h"
#include "unity.h"

// Definitions
#include "dotdot_cluster_id_definitions.h"
#include "attribute_store_defined_attribute_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Unify components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"

static unid_t node_unid;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
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
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_from_node_id(node_id, node_unid);
}

static void setup_groups_for_supporting_node()
{
  // Group 1
  association_group_id_t group_id = 1;
  attribute_store_node_t group_id_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                              &group_id,
                              sizeof(group_id));

  attribute_store_node_t command_list_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST,
    group_id_node);
  // clang-format off
  const uint8_t command_list_1[]
    = {0x5a, 0x01, 0x25, 0x03, 0x5a, 0x01, 0x87, 0x03, 0x71, 0x05,0xFE, 0x01,0x01,0x9F, 0x01,0xFE, 0x01};
  // clang-format on
  attribute_store_set_reported(command_list_node,
                               command_list_1,
                               sizeof(command_list_1));

  // Group 2 (Basic Set CC)
  group_id = 2;
  group_id_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                              &group_id,
                              sizeof(group_id));

  command_list_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST,
    group_id_node);
  const uint8_t command_list_2[] = {COMMAND_CLASS_BASIC, BASIC_SET};
  attribute_store_set_reported(command_list_node,
                               command_list_2,
                               sizeof(command_list_2));

  // Group 3 (Binary Switch Set / Get)
  group_id = 3;
  group_id_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                              &group_id,
                              sizeof(group_id));

  command_list_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST,
    group_id_node);
  const uint8_t command_list_3[] = {COMMAND_CLASS_SWITCH_BINARY,
                                    SWITCH_BINARY_GET,
                                    COMMAND_CLASS_SWITCH_BINARY,
                                    SWITCH_BINARY_SET};
  attribute_store_set_reported(command_list_node,
                               command_list_3,
                               sizeof(command_list_3));

  // Group 4 (Door Lock Sets)
  group_id = 4;
  group_id_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                              &group_id,
                              sizeof(group_id));

  command_list_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST,
    group_id_node);
  const uint8_t command_list_4[] = {COMMAND_CLASS_DOOR_LOCK,
                                    DOOR_LOCK_OPERATION_SET,
                                    COMMAND_CLASS_DOOR_LOCK,
                                    DOOR_LOCK_CONFIGURATION_SET};
  attribute_store_set_reported(command_list_node,
                               command_list_4,
                               sizeof(command_list_4));

  // Group 5 (Door Lock Reports)
  group_id = 5;
  group_id_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                              &group_id,
                              sizeof(group_id));

  command_list_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST,
    group_id_node);
  const uint8_t command_list_5[] = {COMMAND_CLASS_DOOR_LOCK,
                                    DOOR_LOCK_OPERATION_REPORT,
                                    COMMAND_CLASS_DOOR_LOCK,
                                    DOOR_LOCK_CONFIGURATION_REPORT};
  attribute_store_set_reported(command_list_node,
                               command_list_5,
                               sizeof(command_list_5));

  // Create an empty group:
  attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                           endpoint_id_node);

  // Group 7 Thermostat and Identify
  group_id = 7;
  group_id_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                              &group_id,
                              sizeof(group_id));

  command_list_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST,
    group_id_node);
  const uint8_t command_list_7[] = {COMMAND_CLASS_DOOR_LOCK,
                                    DOOR_LOCK_OPERATION_REPORT,
                                    COMMAND_CLASS_INDICATOR,
                                    INDICATOR_SET,
                                    COMMAND_CLASS_THERMOSTAT_SETPOINT,
                                    THERMOSTAT_SETPOINT_SET};
  attribute_store_set_reported(command_list_node,
                               command_list_7,
                               sizeof(command_list_7));

  // Group 8 is level level and level
  group_id = 8;
  group_id_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                              &group_id,
                              sizeof(group_id));

  command_list_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST,
    group_id_node);
  const uint8_t command_list_8[] = {
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
                               command_list_8,
                               sizeof(command_list_8));
}

void test_binding_cluster_mapper_helper_get_group_from_cluster_test()
{
  association_group_id_t group_id_list[255] = {};
  uint8_t group_id_list_size                = 255;
  binding_mapper_helper_get_association_group_list_from_cluster(
    node_unid,
    endpoint_id,
    DOTDOT_ON_OFF_CLUSTER_ID,
    group_id_list,
    &group_id_list_size);

  TEST_ASSERT_EQUAL(0, group_id_list_size);

  setup_groups_for_supporting_node();
  binding_mapper_helper_get_association_group_list_from_cluster(
    node_unid,
    endpoint_id,
    DOTDOT_ON_OFF_CLUSTER_ID,
    group_id_list,
    &group_id_list_size);

  TEST_ASSERT_EQUAL(2, group_id_list_size);
  TEST_ASSERT_EQUAL(2, group_id_list[0]);
  TEST_ASSERT_EQUAL(3, group_id_list[1]);

  binding_mapper_helper_get_association_group_list_from_cluster(
    node_unid,
    endpoint_id,
    DOTDOT_DOOR_LOCK_CLUSTER_ID,
    group_id_list,
    &group_id_list_size);

  TEST_ASSERT_EQUAL(1, group_id_list_size);
  TEST_ASSERT_EQUAL(4, group_id_list[0]);

  binding_mapper_helper_get_association_group_list_from_cluster(
    node_unid,
    endpoint_id,
    DOTDOT_LEVEL_CLUSTER_ID,
    group_id_list,
    &group_id_list_size);

  TEST_ASSERT_EQUAL(1, group_id_list_size);
  TEST_ASSERT_EQUAL(8, group_id_list[0]);

  binding_mapper_helper_get_association_group_list_from_cluster(
    node_unid,
    endpoint_id,
    DOTDOT_IDENTIFY_CLUSTER_ID,
    group_id_list,
    &group_id_list_size);

  TEST_ASSERT_EQUAL(1, group_id_list_size);
  TEST_ASSERT_EQUAL(7, group_id_list[0]);

  binding_mapper_helper_get_association_group_list_from_cluster(
    node_unid,
    endpoint_id,
    DOTDOT_THERMOSTAT_CLUSTER_ID,
    group_id_list,
    &group_id_list_size);

  TEST_ASSERT_EQUAL(1, group_id_list_size);
  TEST_ASSERT_EQUAL(7, group_id_list[0]);
}

void test_binding_cluster_mapper_helper_get_cluster_from_group_test()
{
  dotdot_cluster_id_t cluster_list[255] = {};
  uint8_t cluster_list_size             = 255;

  binding_mapper_helper_get_cluster_list_from_association_group(
    node_unid,
    endpoint_id,
    1,
    cluster_list,
    &cluster_list_size);

  TEST_ASSERT_EQUAL(0, cluster_list_size);

  setup_groups_for_supporting_node();

  // Group 1 is nothing really (lifeline stuff):
  binding_mapper_helper_get_cluster_list_from_association_group(
    node_unid,
    endpoint_id,
    1,
    cluster_list,
    &cluster_list_size);
  TEST_ASSERT_EQUAL(0, cluster_list_size);

  // Group 2 is OnOff
  binding_mapper_helper_get_cluster_list_from_association_group(
    node_unid,
    endpoint_id,
    2,
    cluster_list,
    &cluster_list_size);
  TEST_ASSERT_EQUAL(1, cluster_list_size);
  TEST_ASSERT_EQUAL(DOTDOT_ON_OFF_CLUSTER_ID, cluster_list[0]);

  // Group 3 is OnOff too
  cluster_list_size = 0;
  binding_mapper_helper_get_cluster_list_from_association_group(
    node_unid,
    endpoint_id,
    3,
    cluster_list,
    &cluster_list_size);
  TEST_ASSERT_EQUAL(1, cluster_list_size);
  TEST_ASSERT_EQUAL(DOTDOT_ON_OFF_CLUSTER_ID, cluster_list[0]);

  // Group 4 is DoorLock
  cluster_list_size = 0;
  binding_mapper_helper_get_cluster_list_from_association_group(
    node_unid,
    endpoint_id,
    4,
    cluster_list,
    &cluster_list_size);
  TEST_ASSERT_EQUAL(1, cluster_list_size);
  TEST_ASSERT_EQUAL(DOTDOT_DOOR_LOCK_CLUSTER_ID, cluster_list[0]);

  // Group 5 is nothing
  cluster_list_size = 20;
  binding_mapper_helper_get_cluster_list_from_association_group(
    node_unid,
    endpoint_id,
    5,
    cluster_list,
    &cluster_list_size);
  TEST_ASSERT_EQUAL(0, cluster_list_size);

  // Group 6 is nothing
  cluster_list_size = 20;
  binding_mapper_helper_get_cluster_list_from_association_group(
    node_unid,
    endpoint_id,
    6,
    cluster_list,
    &cluster_list_size);
  TEST_ASSERT_EQUAL(0, cluster_list_size);

  // Group 7 is Thermostat and Identify
  cluster_list_size = 0;
  binding_mapper_helper_get_cluster_list_from_association_group(
    node_unid,
    endpoint_id,
    7,
    cluster_list,
    &cluster_list_size);
  TEST_ASSERT_EQUAL(2, cluster_list_size);
  TEST_ASSERT_EQUAL(DOTDOT_IDENTIFY_CLUSTER_ID, cluster_list[0]);
  TEST_ASSERT_EQUAL(DOTDOT_THERMOSTAT_CLUSTER_ID, cluster_list[1]);

  // Group 8 is Level
  cluster_list_size = 0;
  binding_mapper_helper_get_cluster_list_from_association_group(
    node_unid,
    endpoint_id,
    8,
    cluster_list,
    &cluster_list_size);
  TEST_ASSERT_EQUAL(1, cluster_list_size);
  TEST_ASSERT_EQUAL(DOTDOT_LEVEL_CLUSTER_ID, cluster_list[0]);
}

void test_binding_cluster_mapper_helper_get_cluster_from_node_test()
{
  dotdot_cluster_id_t cluster_list[255] = {};
  uint8_t cluster_list_size             = 255;

  binding_mapper_helper_get_bindable_cluster_list_from_node(node_unid,
                                                            endpoint_id,
                                                            cluster_list,
                                                            &cluster_list_size);
  TEST_ASSERT_EQUAL(0, cluster_list_size);

  setup_groups_for_supporting_node();
  binding_mapper_helper_get_bindable_cluster_list_from_node(node_unid,
                                                            endpoint_id,
                                                            cluster_list,
                                                            &cluster_list_size);
  TEST_ASSERT_EQUAL(0, cluster_list_size);

  // Now indicate that we have 3 groups
  association_group_count_t group_count   = 3;
  attribute_store_node_t group_count_node = attribute_store_emplace(
    endpoint_id_node,
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_SUPPORTED_GROUPINGS,
    &group_count,
    sizeof(group_count));

  binding_mapper_helper_get_bindable_cluster_list_from_node(node_unid,
                                                            endpoint_id,
                                                            cluster_list,
                                                            &cluster_list_size);
  TEST_ASSERT_EQUAL(1, cluster_list_size);
  TEST_ASSERT_EQUAL(DOTDOT_ON_OFF_CLUSTER_ID, cluster_list[0]);

  // Now indicate that we have 8 groups
  group_count      = 8;
  group_count_node = attribute_store_set_reported(group_count_node,
                                                  &group_count,
                                                  sizeof(group_count));

  binding_mapper_helper_get_bindable_cluster_list_from_node(node_unid,
                                                            endpoint_id,
                                                            cluster_list,
                                                            &cluster_list_size);
  TEST_ASSERT_EQUAL(5, cluster_list_size);
  TEST_ASSERT_EQUAL(DOTDOT_IDENTIFY_CLUSTER_ID, cluster_list[0]);
  TEST_ASSERT_EQUAL(DOTDOT_ON_OFF_CLUSTER_ID, cluster_list[1]);
  TEST_ASSERT_EQUAL(DOTDOT_LEVEL_CLUSTER_ID, cluster_list[2]);
  TEST_ASSERT_EQUAL(DOTDOT_DOOR_LOCK_CLUSTER_ID, cluster_list[3]);
  TEST_ASSERT_EQUAL(DOTDOT_THERMOSTAT_CLUSTER_ID, cluster_list[4]);
}