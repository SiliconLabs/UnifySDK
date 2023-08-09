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
#include "binding_cluster_mapper.h"
#include "unity.h"

// Definitions
#include "dotdot_cluster_id_definitions.h"
#include "attribute_store_defined_attribute_types.h"

#include "zwave_command_class_association_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Unify components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "unify_dotdot_attribute_store_node_state.h"

// Generic includes
#include <string.h>

// Mocks
#include "dotdot_mqtt_supported_generated_commands_mock.h"
#include "zwave_network_management_mock.h"

// Static variables
static unid_t node_unid;
static uic_mqtt_dotdot_on_off_supported_commands_t expected_on_off_commands;
static uic_mqtt_dotdot_level_supported_commands_t expected_level_commands;
static uic_mqtt_dotdot_scenes_supported_commands_t expected_scenes_commands;

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
  memset(&expected_on_off_commands, 0, sizeof(expected_on_off_commands));
  memset(&expected_level_commands, 0, sizeof(expected_level_commands));
  memset(&expected_scenes_commands, 0, sizeof(expected_scenes_commands));

  zpc_attribute_store_test_helper_create_network();
  zwave_unid_from_node_id(node_id, node_unid);
  zwave_network_management_get_node_id_IgnoreAndReturn(zpc_node_id);
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);
  TEST_ASSERT_TRUE(binding_cluster_mapper_init());
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

void test_publish_commands_for_supporting_node()
{
  setup_groups_for_supporting_node();

  attribute_store_node_t network_status_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS, node_id_node);

  // Nothing happens here as long as we are not online functional.
  NodeStateNetworkStatus network_status = ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  // Set to online functional, expect publications:
  expected_on_off_commands.on  = true;
  expected_on_off_commands.off = true;
  uic_mqtt_dotdot_on_off_publish_supported_generated_commands_ExpectWithArray(
    NULL,
    endpoint_id,
    &expected_on_off_commands,
    sizeof(expected_on_off_commands));
  uic_mqtt_dotdot_on_off_publish_supported_generated_commands_IgnoreArg_unid();

  expected_level_commands.move_to_level = true;
  expected_level_commands.move          = true;
  expected_level_commands.stop          = true;
  uic_mqtt_dotdot_level_publish_supported_generated_commands_ExpectWithArray(
    node_unid,
    endpoint_id,
    &expected_level_commands,
    sizeof(expected_level_commands));
  uic_mqtt_dotdot_level_publish_supported_generated_commands_IgnoreArg_unid();

  uic_mqtt_dotdot_scenes_publish_supported_generated_commands_ExpectWithArray(
    NULL,
    endpoint_id,
    &expected_scenes_commands,
    sizeof(expected_scenes_commands));
  uic_mqtt_dotdot_scenes_publish_supported_generated_commands_IgnoreArg_unid();

  network_status = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
}

void test_publish_commands_for_supporting_node_cannot_read_unid()
{
  setup_groups_for_supporting_node();

  attribute_store_node_t network_status_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS, node_id_node);

  // Nothing happens here as long as we are not online functional.
  NodeStateNetworkStatus network_status = ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  // Remove the value of the node_id, it should not try to publish anything
  attribute_store_undefine_reported(node_id_node);

  network_status = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
}

void test_no_publish_commands_for_zpc()
{
  attribute_store_node_t network_status_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS, zpc_node_id_node);

  // Set to online functional, nothing should happen.
  NodeStateNetworkStatus network_status = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
}

void test_publish_central_scene_commands_for_supporting_node()
{
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
    = {COMMAND_CLASS_CENTRAL_SCENE, CENTRAL_SCENE_NOTIFICATION};
  // clang-format on
  attribute_store_set_reported(command_list_node,
                               command_list_1,
                               sizeof(command_list_1));

  attribute_store_node_t network_status_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS, node_id_node);

  // Nothing happens here as long as we are not online functional.
  NodeStateNetworkStatus network_status = ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  // Set to online functional, expect publications:
  uic_mqtt_dotdot_on_off_publish_supported_generated_commands_ExpectWithArray(
    NULL,
    endpoint_id,
    &expected_on_off_commands,
    sizeof(expected_on_off_commands));
  uic_mqtt_dotdot_on_off_publish_supported_generated_commands_IgnoreArg_unid();

  uic_mqtt_dotdot_level_publish_supported_generated_commands_ExpectWithArray(
    node_unid,
    endpoint_id,
    &expected_level_commands,
    sizeof(expected_level_commands));
  uic_mqtt_dotdot_level_publish_supported_generated_commands_IgnoreArg_unid();

  expected_scenes_commands.recall_scene = true;
  uic_mqtt_dotdot_scenes_publish_supported_generated_commands_ExpectWithArray(
    NULL,
    endpoint_id,
    &expected_scenes_commands,
    sizeof(expected_scenes_commands));
  uic_mqtt_dotdot_scenes_publish_supported_generated_commands_IgnoreArg_unid();

  network_status = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
}

void test_publish_on_off_commands_for_supporting_node()
{
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
    = {COMMAND_CLASS_BASIC,BASIC_SET};
  // clang-format on
  attribute_store_set_reported(command_list_node,
                               command_list_1,
                               sizeof(command_list_1));

  attribute_store_node_t network_status_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS, node_id_node);

  // Nothing happens here as long as we are not online functional.
  NodeStateNetworkStatus network_status = ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  // Set to online functional, expect publications:
  expected_on_off_commands.on  = true;
  expected_on_off_commands.off = true;
  uic_mqtt_dotdot_on_off_publish_supported_generated_commands_ExpectWithArray(
    NULL,
    endpoint_id,
    &expected_on_off_commands,
    sizeof(expected_on_off_commands));
  uic_mqtt_dotdot_on_off_publish_supported_generated_commands_IgnoreArg_unid();

  uic_mqtt_dotdot_level_publish_supported_generated_commands_ExpectWithArray(
    node_unid,
    endpoint_id,
    &expected_level_commands,
    sizeof(expected_level_commands));
  uic_mqtt_dotdot_level_publish_supported_generated_commands_IgnoreArg_unid();

  uic_mqtt_dotdot_scenes_publish_supported_generated_commands_ExpectWithArray(
    NULL,
    endpoint_id,
    &expected_scenes_commands,
    sizeof(expected_scenes_commands));
  uic_mqtt_dotdot_scenes_publish_supported_generated_commands_IgnoreArg_unid();

  network_status = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
}
