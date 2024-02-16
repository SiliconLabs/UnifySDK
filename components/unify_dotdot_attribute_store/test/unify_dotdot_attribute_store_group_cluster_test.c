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
#include "unify_dotdot_attribute_store.h"
#include "unify_dotdot_attribute_store_node_state.h"
#include "unify_dotdot_defined_attribute_types.h"
#include "unify_dotdot_attribute_store_group_cluster.h"
#include "unity.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "sl_log.h"

// Mock includes
#include "dotdot_mqtt_mock.h"
#include "uic_mqtt_mock.h"
#include "contiki_test_helper.h"

#define TEST_ENDPOINT_TYPE 3
#define TEST_NODE_TYPE     2

#define STATE(attribute)  DOTDOT_ATTRIBUTE_ID_STATE_##attribute
#define GROUPS(attribute) DOTDOT_ATTRIBUTE_ID_GROUPS_##attribute

static mqtt_message_callback_t by_group_command_dispatch = NULL;
static mqtt_message_callback_t by_unid_command_dispatch  = NULL;

static void test_group_dispatch(uint16_t group_id,
                                const char *cluster_name,
                                const char *command,
                                const char *message,
                                size_t message_length,
                                mqtt_message_callback_t callback)
{
  TEST_ASSERT_EQUAL(3, group_id);
  TEST_ASSERT_EQUAL_STRING("Groups", cluster_name);
  TEST_ASSERT_EQUAL_STRING("AddGroup", command);
  TEST_ASSERT_EQUAL_STRING("{\"GroupId\": 6, \"GroupName\":\"TestGroup\"}",
                           message);
  TEST_ASSERT_EQUAL(39, message_length);
  TEST_ASSERT_EQUAL_PTR(callback, by_unid_command_dispatch);
}

// Test functions used for the configuration the Unify Dotdot Attribute Store Component.
static attribute_store_node_t
  get_endpoint_node(const dotdot_unid_t unid, dotdot_endpoint_id_t endpoint_id)
{
  attribute_store_node_t node = attribute_store_get_root();

  node = attribute_store_get_node_child_by_value(node,
                                                 TEST_NODE_TYPE,
                                                 REPORTED_ATTRIBUTE,
                                                 (const uint8_t *)unid,
                                                 strlen(unid) + 1,
                                                 0);

  node = attribute_store_get_node_child_by_value(node,
                                                 TEST_ENDPOINT_TYPE,
                                                 REPORTED_ATTRIBUTE,
                                                 &endpoint_id,
                                                 sizeof(endpoint_id),
                                                 0);

  return node;
}

static sl_status_t get_unid(attribute_store_node_t node, char *unid)
{
  if (attribute_store_get_node_type(node) != TEST_NODE_TYPE) {
    node = attribute_store_get_first_parent_with_type(node, TEST_NODE_TYPE);
  }

  return attribute_store_get_reported_string(node, unid, MAXIMUM_UNID_SIZE);
}

static sl_status_t get_unid_endpoint(attribute_store_node_t node,
                                     char *unid,
                                     dotdot_endpoint_id_t *endpoint_id)
{
  if (attribute_store_get_node_type(node) != TEST_ENDPOINT_TYPE) {
    node = attribute_store_get_first_parent_with_type(node, TEST_ENDPOINT_TYPE);
  }
  if (SL_STATUS_OK
      != attribute_store_get_reported(node,
                                      endpoint_id,
                                      sizeof(dotdot_endpoint_id_t))) {
    return SL_STATUS_FAIL;
  }
  return get_unid(node, unid);
}

const unify_dotdot_attribute_store_configuration_t test_configuration = {
  .get_endpoint_node_function                  = &get_endpoint_node,
  .get_unid_endpoint_function                  = &get_unid_endpoint,
  .get_unid_function                           = &get_unid,
  .update_attribute_desired_values_on_commands = false,
  .clear_reported_on_desired_updates           = false,
  .automatic_deduction_of_supported_commands   = true,
  .force_read_attributes_enabled               = false,
  .write_attributes_enabled                    = false,
  .publish_desired_attribute_values_to_mqtt    = true,
  .publish_reported_attribute_values_to_mqtt   = true,
  .node_type                                   = TEST_NODE_TYPE,
  .endpoint_type                               = TEST_ENDPOINT_TYPE,
};

// Stub functions
static void uic_mqtt_subscribe_stub(const char *topic,
                                    mqtt_message_callback_t callback,
                                    int cmock_num_calls)
{
  if (strcmp("ucl/by-unid/unid-42/ep4/Groups/Commands/+", topic) == 0) {
    by_unid_command_dispatch = callback;
  } else {
    by_group_command_dispatch = callback;
  }
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
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  by_unid_command_dispatch  = NULL;
  by_group_command_dispatch = NULL;
  uic_mqtt_subscribe_Stub(&uic_mqtt_subscribe_stub);
  attribute_store_delete_node(attribute_store_get_root());
  unify_dotdot_attribute_store_set_configuration(&test_configuration);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    unify_dotdot_attribute_store_group_cluster_init());
}

/// Called after each and every test
void tearDown() {}

/**
 * @brief Helper function that create a UNID/EP node in the attribute store
*/
static attribute_store_node_t create_node_ep(char *unid,
                                             dotdot_endpoint_id_t endpoint_id)
{
  attribute_store_node_t unid_node
    = attribute_store_add_node(TEST_NODE_TYPE, attribute_store_get_root());
  attribute_store_node_t endpoint_node
    = attribute_store_add_node(TEST_ENDPOINT_TYPE, unid_node);

  attribute_store_set_reported_string(unid_node, unid);
  attribute_store_set_reported(endpoint_node,
                               &endpoint_id,
                               sizeof(endpoint_id));

  return endpoint_node;
}

void test_publish_node_groups_happy_case()
{
  // Create an unid/endpoint:
  char unid_value[]                = "unid-42";
  dotdot_endpoint_id_t endpoint_id = 4;
  attribute_store_node_t endpoint_node
    = create_node_ep(unid_value, endpoint_id);

  // Check what's its network status: (should be nothing yet)
  TEST_ASSERT_EQUAL(ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE,
                    unify_attribute_store_node_state_get_status(endpoint_node));

  uint16_t group_id = 3;
  attribute_store_emplace(endpoint_node,
                          GROUPS(GROUP_ID),
                          &group_id,
                          sizeof(group_id));

  uic_mqtt_publish_Expect(
    "ucl/by-unid/unid-42/ep4/Groups/Attributes/NameSupport/Desired",
    "{\"value\":{\"Supported\":true}}",
    28,
    true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/unid-42/ep4/Groups/Attributes/NameSupport/Reported",
    "{\"value\":{\"Supported\":true}}",
    28,
    true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/unid-42/ep4/Groups/Attributes/GroupList/Desired",
    "{\"value\":[3]}",
    13,
    true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/unid-42/ep4/Groups/Attributes/GroupList/Reported",
    "{\"value\":[3]}",
    13,
    true);
  uic_mqtt_publish_Expect("ucl/by-unid/unid-42/ep4/Groups/SupportedCommands",
                          "{\"value\": [\"AddGroup\", \"RemoveGroup\", "
                          "\"RemoveAllGroups\", \"AddGroupIfIdentifying\"]}",
                          82,
                          true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/unid-42/ep4/Groups/Attributes/ClusterRevision/Desired",
    "{\"value\": 1}",
    12,
    true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/unid-42/ep4/Groups/Attributes/ClusterRevision/Reported",
    "{\"value\": 1}",
    12,
    true);

  // Now get online functional
  NodeStateNetworkStatus network_status
    = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL;
  attribute_store_set_child_reported(
    attribute_store_get_node_parent(endpoint_node),
    STATE(NETWORK_STATUS),
    &network_status,
    sizeof(network_status));

  // Add a group
  TEST_ASSERT_NOT_NULL(by_unid_command_dispatch);
  by_unid_command_dispatch("ucl/by-unid/unid-42/ep4/Groups/Commands/AddGroup",
                           "{\"GroupId\": 5, \"GroupName\":\"TestGroup\"}",
                           47);

  uic_mqtt_publish_Expect(
    "ucl/by-unid/unid-42/ep4/Groups/Attributes/GroupList/Desired",
    "{\"value\":[3, 5]}",
    16,
    true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/unid-42/ep4/Groups/Attributes/GroupList/Reported",
    "{\"value\":[3, 5]}",
    16,
    true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/unid-42/ep4/Groups/Attributes/5/Name/Desired",
    "{\"value\": \"TestGroup\"}",
    22,
    true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/unid-42/ep4/Groups/Attributes/5/Name/Reported",
    "{\"value\": \"TestGroup\"}",
    22,
    true);
  contiki_test_helper_run(1);

  // Remove a group
  uic_mqtt_unretain_Expect("ucl/by-unid/unid-42/ep4/Groups/Attributes/5/Name");
  by_unid_command_dispatch(
    "ucl/by-unid/unid-42/ep4/Groups/Commands/RemoveGroup",
    "{\"GroupId\": 5}",
    18);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/unid-42/ep4/Groups/Attributes/GroupList/Desired",
    "{\"value\":[3]}",
    13,
    true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/unid-42/ep4/Groups/Attributes/GroupList/Reported",
    "{\"value\":[3]}",
    13,
    true);
  contiki_test_helper_run(1);

  // Add a group for nodes part of group 3:
  TEST_ASSERT_NOT_NULL(by_group_command_dispatch);
  uic_mqtt_dotdot_get_group_dispatch_callback_ExpectAndReturn(
    &test_group_dispatch);
  by_group_command_dispatch("ucl/by-group/3/Groups/Commands/AddGroup",
                            "{\"GroupId\": 6, \"GroupName\":\"TestGroup\"}",
                            39);
  contiki_test_helper_run(1);

  // Add if Identifying. Nothing will happen here.
  by_unid_command_dispatch(
    "ucl/by-unid/unid-42/ep4/Groups/Commands/AddGroupIfIdentifying",
    "{\"GroupId\": 5}",
    18);
  contiki_test_helper_run(1);

  //Remove all groups
  uic_mqtt_unretain_Expect("ucl/by-unid/unid-42/ep4/Groups/Attributes/3/Name");
  by_unid_command_dispatch(
    "ucl/by-unid/unid-42/ep4/Groups/Commands/RemoveAllGroups",
    "{}",
    2);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/unid-42/ep4/Groups/Attributes/GroupList/Desired",
    "{\"value\":[]}",
    12,
    true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/unid-42/ep4/Groups/Attributes/GroupList/Reported",
    "{\"value\":[]}",
    12,
    true);
  contiki_test_helper_run(1);

  // Now delete everything, check that things are unretained:
  uic_mqtt_unretain_Expect("ucl/by-unid/unid-42/ep4/Groups");
  attribute_store_delete_node(endpoint_node);
}
