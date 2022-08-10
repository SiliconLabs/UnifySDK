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
#include "zcl_group_cluster_server.h"
#include "unity.h"

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "attribute_store_defined_attribute_types.h"
#include "zpc_attribute_store_network_helper.h"
#include "dotdot_attributes.h"
#include "ucl_node_state.h"

// Interfaces
#include "sl_status.h"
#include "zwave_controller_types.h"
#include "zwave_utils.h"

// Mock includes
#include "uic_mqtt_mock.h"
#include "zpc_dotdot_mqtt_group_dispatch_mock.h"
#include "zwave_network_management_mock.h"

// Generic includes
#include <string.h>

// Static variables
// We do not bother initializing the zwave_unid.h home_id
const zwave_home_id_t home_id          = 0x00000000;
static zwave_node_id_t node_id         = 0;
static zwave_endpoint_id_t endpoint_id = 0;

static attribute_store_node_t home_id_node     = ATTRIBUTE_STORE_INVALID_NODE;
static attribute_store_node_t node_id_node     = ATTRIBUTE_STORE_INVALID_NODE;
static attribute_store_node_t my_node_id_node  = ATTRIBUTE_STORE_INVALID_NODE;
static attribute_store_node_t endpoint_id_node = ATTRIBUTE_STORE_INVALID_NODE;

static mqtt_message_callback_t incoming_publication_callback_save       = NULL;
static mqtt_message_callback_t incoming_group_publication_callback_save = NULL;

// Expected publications
const char name_support_message[]     = "{\"value\":{\"Supported\":true}}";
const char empty_group_list_message[] = "{\"value\":[]}";
const char supported_commands_message[]
  = "{\"value\": [\"AddGroup\", \"RemoveGroup\", "
    "\"RemoveAllGroups\", \"AddGroupIfIdentifying\"]}";
const char revision_message[] = "{\"value\": 1}";

static void create_test_network()
{
  // It will be NodeID 1 - EP 0 and 1 (This is us)
  //            NodeID 2 - EP 1 and 2. EP 1 is identifying
  //            NodeID 4 - EP 0. It has a GroupID attribute with undefined value []
  //            NodeID 5 - EP [] with undefined value

  home_id_node
    = attribute_store_add_node(ATTRIBUTE_HOME_ID, attribute_store_get_root());

  attribute_store_set_node_attribute_value(home_id_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&home_id,
                                           sizeof(zwave_home_id_t));

  // NodeID 1
  node_id_node    = attribute_store_add_node(ATTRIBUTE_NODE_ID, home_id_node);
  my_node_id_node = node_id_node;

  node_id = 1;
  attribute_store_set_reported(node_id_node, &node_id, sizeof(zwave_node_id_t));

  endpoint_id_node
    = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, node_id_node);
  endpoint_id = 0;
  attribute_store_set_reported(endpoint_id_node,
                               &endpoint_id,
                               sizeof(zwave_endpoint_id_t));

  endpoint_id_node
    = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, node_id_node);
  endpoint_id = 1;
  attribute_store_set_reported(endpoint_id_node,
                               &endpoint_id,
                               sizeof(zwave_endpoint_id_t));

  attribute_store_node_t network_status_node
    = attribute_store_add_node(ATTRIBUTE_NETWORK_STATUS, node_id_node);
  node_state_topic_state_t network_status = NODE_STATE_TOPIC_STATE_INCLUDED;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  // NodeID 2
  node_id_node = attribute_store_add_node(ATTRIBUTE_NODE_ID, home_id_node);

  node_id = 2;
  attribute_store_set_reported(node_id_node, &node_id, sizeof(zwave_node_id_t));

  endpoint_id_node
    = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, node_id_node);
  endpoint_id = 1;
  attribute_store_set_reported(endpoint_id_node,
                               &endpoint_id,
                               sizeof(zwave_endpoint_id_t));

  attribute_store_node_t identify_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_IDENTIFY_IDENTIFY_TIME,
                               endpoint_id_node);
  uint16_t identify_time = 3;
  attribute_store_set_reported(identify_node,
                               &identify_time,
                               sizeof(identify_time));

  endpoint_id_node
    = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, node_id_node);
  endpoint_id = 2;
  attribute_store_set_reported(endpoint_id_node,
                               &endpoint_id,
                               sizeof(zwave_endpoint_id_t));

  network_status_node
    = attribute_store_add_node(ATTRIBUTE_NETWORK_STATUS, node_id_node);
  network_status = NODE_STATE_TOPIC_STATE_INCLUDED;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  // NodeID 4
  node_id_node = attribute_store_add_node(ATTRIBUTE_NODE_ID, home_id_node);

  node_id = 4;
  attribute_store_set_reported(node_id_node, &node_id, sizeof(zwave_node_id_t));

  endpoint_id_node
    = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, node_id_node);
  endpoint_id = 0;
  attribute_store_set_reported(endpoint_id_node,
                               &endpoint_id,
                               sizeof(zwave_endpoint_id_t));
  attribute_store_add_node(0x00040001, node_id_node);

  network_status_node
    = attribute_store_add_node(ATTRIBUTE_NETWORK_STATUS, node_id_node);
  network_status = NODE_STATE_TOPIC_STATE_INCLUDED;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));

  // NodeID 5, with undefined EP
  node_id_node = attribute_store_add_node(ATTRIBUTE_NODE_ID, home_id_node);

  node_id = 5;
  attribute_store_set_reported(node_id_node, &node_id, sizeof(zwave_node_id_t));

  endpoint_id_node
    = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, node_id_node);

  network_status_node
    = attribute_store_add_node(ATTRIBUTE_NETWORK_STATUS, node_id_node);
  network_status = NODE_STATE_TOPIC_STATE_INCLUDED;
  attribute_store_set_reported(network_status_node,
                               &network_status,
                               sizeof(network_status));
}

void uic_mqtt_subscribe_callback(const char *topic,
                                 mqtt_message_callback_t callback,
                                 int cmock_num_calls)
{
  // Save the on_zcl_group_cluster_server_command_received function address
  if (strcmp(topic, "ucl/by-group/+/Groups/Commands/+") == 0) {
    incoming_group_publication_callback_save = callback;
  } else {
    incoming_publication_callback_save = callback;
  }
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  create_test_network();
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
  // Set the homeID / NodeID to 0, so our UNID is zw-00000000-0001
  zwave_unid_set_home_id(home_id);
  zwave_network_management_get_node_id_IgnoreAndReturn(1);
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);

  uic_mqtt_subscribe_AddCallback(uic_mqtt_subscribe_callback);
}

void test_zcl_group_cluster_server_init_test()
{
  // Empty attribute store, expect the subscription by group after init
  uic_mqtt_subscribe_Expect("ucl/by-group/+/Groups/Commands/+", NULL);
  uic_mqtt_subscribe_IgnoreArg_callback();

  // Now trigger the init.
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zcl_group_cluster_server_init());

  // When we refresh our attribute store callbacks, it will trigger a bunch
  // subscriptions
  // First let the group server cluster init itself
  // Name support for Node 2, ep 1
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/NameSupport/Desired",
    name_support_message,
    sizeof(name_support_message) - 1,
    true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/NameSupport/Reported",
    name_support_message,
    sizeof(name_support_message) - 1,
    true);

  // GroupList for Node 2, ep 1
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/GroupList/Desired",
    empty_group_list_message,
    sizeof(empty_group_list_message) - 1,
    true);

  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/GroupList/Reported",
    empty_group_list_message,
    sizeof(empty_group_list_message) - 1,
    true);

  // Supported Commands for Node 2, ep 1
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/SupportedCommands",
    supported_commands_message,
    sizeof(supported_commands_message) - 1,
    true);

  // Group Cluster Revision for Node 2, ep 1
  uic_mqtt_publish_Expect("ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/"
                          "ClusterRevision/Desired",
                          revision_message,
                          sizeof(revision_message) - 1,
                          true);
  uic_mqtt_publish_Expect("ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/"
                          "ClusterRevision/Reported",
                          revision_message,
                          sizeof(revision_message) - 1,
                          true);

  // subscribe to incoming commands for NodeID 2, ep 1
  uic_mqtt_subscribe_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Commands/+",
    NULL);
  uic_mqtt_subscribe_IgnoreArg_callback();

  // Name support for Node 2, ep 2
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep2/Groups/Attributes/NameSupport/Desired",
    name_support_message,
    sizeof(name_support_message) - 1,
    true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep2/Groups/Attributes/NameSupport/Reported",
    name_support_message,
    sizeof(name_support_message) - 1,
    true);

  // GroupList for Node 2, ep 2
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep2/Groups/Attributes/GroupList/Desired",
    empty_group_list_message,
    sizeof(empty_group_list_message) - 1,
    true);

  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep2/Groups/Attributes/GroupList/Reported",
    empty_group_list_message,
    sizeof(empty_group_list_message) - 1,
    true);

  // Supported Commands for Node 2, ep2
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep2/Groups/SupportedCommands",
    supported_commands_message,
    sizeof(supported_commands_message) - 1,
    true);

  // Group Cluster Revision for Node 2, ep 2
  uic_mqtt_publish_Expect("ucl/by-unid/zw-00000000-0002/ep2/Groups/Attributes/"
                          "ClusterRevision/Desired",
                          revision_message,
                          sizeof(revision_message) - 1,
                          true);
  uic_mqtt_publish_Expect("ucl/by-unid/zw-00000000-0002/ep2/Groups/Attributes/"
                          "ClusterRevision/Reported",
                          revision_message,
                          sizeof(revision_message) - 1,
                          true);

  // subscribe to incoming commands for NodeID 2, ep 1
  uic_mqtt_subscribe_Expect(
    "ucl/by-unid/zw-00000000-0002/ep2/Groups/Commands/+",
    NULL);
  uic_mqtt_subscribe_IgnoreArg_callback();

  // Name support for Node 4, ep 0
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0004/ep0/Groups/Attributes/NameSupport/Desired",
    name_support_message,
    sizeof(name_support_message) - 1,
    true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0004/ep0/Groups/Attributes/NameSupport/Reported",
    name_support_message,
    sizeof(name_support_message) - 1,
    true);

  // GroupList for Node 4, ep 0
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0004/ep0/Groups/Attributes/GroupList/Desired",
    empty_group_list_message,
    sizeof(empty_group_list_message) - 1,
    true);

  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0004/ep0/Groups/Attributes/GroupList/Reported",
    empty_group_list_message,
    sizeof(empty_group_list_message) - 1,
    true);

  // Supported Commands for Node 4, ep0
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0004/ep0/Groups/SupportedCommands",
    supported_commands_message,
    sizeof(supported_commands_message) - 1,
    true);

  // Group Cluster Revision for Node 4, ep 0
  uic_mqtt_publish_Expect("ucl/by-unid/zw-00000000-0004/ep0/Groups/Attributes/"
                          "ClusterRevision/Desired",
                          revision_message,
                          sizeof(revision_message) - 1,
                          true);
  uic_mqtt_publish_Expect("ucl/by-unid/zw-00000000-0004/ep0/Groups/Attributes/"
                          "ClusterRevision/Reported",
                          revision_message,
                          sizeof(revision_message) - 1,
                          true);

  // subscribe to incoming commands for NodeID 4, ep 0
  uic_mqtt_subscribe_Expect(
    "ucl/by-unid/zw-00000000-0004/ep0/Groups/Commands/+",
    NULL);
  uic_mqtt_subscribe_IgnoreArg_callback();

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_refresh_node_and_children_callbacks(home_id_node));
}

void test_zcl_group_cluster_server_add_group_command_test()
{
  TEST_ASSERT_NOT_NULL(incoming_publication_callback_save);

  // Add Group with invalid parameters
  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0004/ep0/Groups/Commands/AddGroup",
    "{}",
    sizeof("{}") - 1);

  // Add Group with non-valid JSON
  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0004/ep0/Groups/Commands/AddGroup",
    "{ewrewqr]37\"{",
    sizeof("{ewrewqr]37\"{") - 1);

  // Add group without Group ID
  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Commands/AddGroup",
    "{\"GroupName\":\"\"}",
    sizeof("{\"GroupName\":\"\"}") - 1);

  // Add group without a group name
  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Commands/AddGroup",
    "{\"GroupId\":1}",
    sizeof("{\"GroupId\":1}") - 1);

  // Try to add Group ID 0
  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Commands/AddGroup",
    "{\"GroupId\":0,\"GroupName\":\"Test Group\"}",
    sizeof("{\"GroupId\":0,\"GroupName\":\"Test Group\"}") - 1);

  // Add Group 1 with no name on NodeID 2, Ep 1
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/GroupList/Desired",
    "{\"value\":[1]}",
    sizeof("{\"value\":[1]}") - 1,
    true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/GroupList/Reported",
    "{\"value\":[1]}",
    sizeof("{\"value\":[1]}") - 1,
    true);

  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/1/Name/Desired",
    "{\"value\": \"\"}",
    sizeof("{\"value\": \"\"}") - 1,
    true);

  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/1/Name/Reported",
    "{\"value\": \"\"}",
    sizeof("{\"value\": \"\"}") - 1,
    true);

  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Commands/AddGroup",
    "{\"GroupId\":1,\"GroupName\":\"\"}",
    sizeof("{\"GroupId\":1,\"GroupName\":\"\"}") - 1);

  // Add Group 2 with name on NodeID 2, Ep 1
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/GroupList/Desired",
    "{\"value\":[1, 2]}",
    sizeof("{\"value\":[1, 2]}") - 1,
    true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/GroupList/Reported",
    "{\"value\":[1, 2]}",
    sizeof("{\"value\":[1, 2]}") - 1,
    true);

  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/2/Name/Desired",
    "{\"value\": \"Test Group\"}",
    sizeof("{\"value\": \"Test Group\"}") - 1,
    true);

  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/2/Name/Reported",
    "{\"value\": \"Test Group\"}",
    sizeof("{\"value\": \"Test Group\"}") - 1,
    true);

  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Commands/AddGroup",
    "{\"GroupId\":2,\"GroupName\":\"Test Group\"}",
    sizeof("{\"GroupId\":2,\"GroupName\":\"Test Group\"}") - 1);

  // Add group to non-existing endpoint
  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0002/ep32/Groups/Commands/AddGroup",
    "{\"GroupId\":3,\"GroupName\":\"Test Group invalid endpoint\"}",
    sizeof("{\"GroupId\":3,\"GroupName\":\"Test Group invalid endpoint\"}")
      - 1);

  // Add group to non-existing NodeID
  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0034/ep0/Groups/Commands/AddGroup",
    "{\"GroupId\":3,\"GroupName\":\"Test Group invalid NodeID\"}",
    sizeof("{\"GroupId\":3,\"GroupName\":\"Test Group invalid NodeID\"}") - 1);
}

void test_zcl_group_cluster_server_remove_group_command_test()
{
  TEST_ASSERT_NOT_NULL(incoming_publication_callback_save);

  // Remove Group with invalid parameters
  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Commands/RemoveGroup",
    "{}",
    sizeof("{}") - 1);

  // Remove Group with non-valid JSON
  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0004/ep0/Groups/Commands/RemoveGroup",
    "{ewrewqr]37\"{",
    sizeof("{ewrewqr]37\"{") - 1);

  // Remove group to non-existing NodeID
  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0034/ep0/Groups/Commands/RemoveGroup",
    "{\"GroupId\":3}",
    sizeof("{\"GroupId\":3}") - 1);

  // Remove Group 2 with name on NodeID 2, Ep 1
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/GroupList/Desired",
    "{\"value\":[1]}",
    sizeof("{\"value\":[1]}") - 1,
    true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/GroupList/Reported",
    "{\"value\":[1]}",
    sizeof("{\"value\":[1]}") - 1,
    true);

  uic_mqtt_unretain_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/2/Name");

  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Commands/RemoveGroup",
    "{\"GroupId\":2}",
    sizeof("{\"GroupId\":2}") - 1);

  // Try to non-existing group 2 again on NodeID 2, Ep1. Nothing bad should happen.
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/GroupList/Desired",
    "{\"value\":[1]}",
    sizeof("{\"value\":[1]}") - 1,
    true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/GroupList/Reported",
    "{\"value\":[1]}",
    sizeof("{\"value\":[1]}") - 1,
    true);

  uic_mqtt_unretain_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/2/Name");

  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Commands/RemoveGroup",
    "{\"GroupId\":2}",
    sizeof("{\"GroupId\":2}") - 1);
}

void test_zcl_group_cluster_server_commands_with_no_effect_test()
{
  TEST_ASSERT_NOT_NULL(incoming_publication_callback_save);

  // None of these commands should trigger anything.
  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Commands/ViewGroup",
    "{}",
    sizeof("{}") - 1);

  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Commands/GetGroupMembership",
    "{}",
    sizeof("{}") - 1);

  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Commands/ThisIsNotAValidCommand",
    "{}",
    sizeof("{}") - 1);
}

void test_zcl_group_cluster_server_add_if_identifying_command_test()
{
  TEST_ASSERT_NOT_NULL(incoming_publication_callback_save);

  // Add Group 3 with name on NodeID 2, Ep 1, if identifying
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/GroupList/Desired",
    "{\"value\":[1, 3]}",
    sizeof("{\"value\":[1, 3]}") - 1,
    true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/GroupList/Reported",
    "{\"value\":[1, 3]}",
    sizeof("{\"value\":[1, 3]}") - 1,
    true);

  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/3/Name/Desired",
    "{\"value\": \"Test Idenfifying Group\"}",
    sizeof("{\"value\": \"Test Idenfifying Group\"}") - 1,
    true);

  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/3/Name/Reported",
    "{\"value\": \"Test Idenfifying Group\"}",
    sizeof("{\"value\": \"Test Idenfifying Group\"}") - 1,
    true);

  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Commands/AddGroupIfIdentifying",
    "{\"GroupId\":3,\"GroupName\":\"Test Idenfifying Group\"}",
    sizeof("{\"GroupId\":3,\"GroupName\":\"Test Idenfifying Group\"}") - 1);

  // Add group to non-existing endpoint
  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0002/ep8/Groups/Commands/AddGroupIfIdentifying",
    "{\"GroupId\":4,\"GroupName\":\"Test Invalid Group\"}",
    sizeof("{\"GroupId\":4,\"GroupName\":\"Test Invalid Group\"}") - 1);

  // Add group to non-existing NodeID
  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-00AA/ep0/Groups/Commands/AddGroupIfIdentifying",
    "{\"GroupId\":4,\"GroupName\":\"Test Invalid Group\"}",
    sizeof("{\"GroupId\":4,\"GroupName\":\"Test Invalid Group\"}") - 1);

  // Add group on a non-identifying endpoint
  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0002/ep2/Groups/Commands/AddGroupIfIdentifying",
    "{\"GroupId\":4,\"GroupName\":\"Test non-identifying Group\"}",
    sizeof("{\"GroupId\":4,\"GroupName\":\"Test non-identifying Group\"}") - 1);

  // Missing Group ID
  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Commands/AddGroupIfIdentifying",
    "{\"GroupName\":\"Test Idenfifying Group\"}",
    sizeof("{\"GroupName\":\"Test Idenfifying Group\"}") - 1);

  // Missing Group Name
  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Commands/AddGroupIfIdentifying",
    "{\"GroupId\":6}",
    sizeof("{\"GroupId\":6}") - 1);

  // With non-valid JSON
  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Commands/AddGroupIfIdentifying",
    "{ewrewqr]37\"{",
    sizeof("{ewrewqr]37\"{") - 1);
}

void test_zcl_group_cluster_server_is_unid_endpoint_in_group_test()
{
  // At that point, NodeID 2/Ep 1 is part of group 1 and 3
  TEST_ASSERT_TRUE(is_unid_endpoint_in_group("zw-00000000-0002", 1, 1));
  TEST_ASSERT_FALSE(is_unid_endpoint_in_group("zw-00000000-0002", 1, 2));
  TEST_ASSERT_TRUE(is_unid_endpoint_in_group("zw-00000000-0002", 1, 3));
  TEST_ASSERT_FALSE(is_unid_endpoint_in_group("zw-00000000-0002", 2, 1));

  TEST_ASSERT_FALSE(is_unid_endpoint_in_group("zw-00000000-0004", 0, 1));
  TEST_ASSERT_FALSE(is_unid_endpoint_in_group("zw-00000000-0004", 0, 200));
  TEST_ASSERT_FALSE(is_unid_endpoint_in_group("zw-00000000-0005", 1, 1));
}

void test_zcl_group_cluster_server_by_group_command_dispatch()
{
  TEST_ASSERT_NOT_NULL(incoming_publication_callback_save);
  TEST_ASSERT_NOT_NULL(incoming_group_publication_callback_save);

  // Nothing should happen on error:
  incoming_group_publication_callback_save(
    "ucl/by-group/zxmcnv/Groups/Commands/AddfdsfGroup",
    "{}",
    sizeof("{}") - 1);

  // This should go through to the dispatch component
  zpc_dotdot_mqtt_group_dispatch_Expect(12,
                                        "Groups",
                                        "TestCommand",
                                        "TestMessage",
                                        sizeof("TestMessage") - 1,
                                        incoming_publication_callback_save);

  incoming_group_publication_callback_save(
    "ucl/by-group/12/Groups/Commands/TestCommand",
    "TestMessage",
    sizeof("TestMessage") - 1);
}

void test_zcl_group_cluster_server_remove_all_groups_command_test()
{
  TEST_ASSERT_NOT_NULL(incoming_publication_callback_save);

  // At that point, NodeID 2 Ep 1 is part of group 1 and 3
  uic_mqtt_unretain_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/1/Name");
  uic_mqtt_unretain_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/3/Name");

  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/GroupList/Desired",
    "{\"value\":[]}",
    sizeof("{\"value\":[]}") - 1,
    true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Attributes/GroupList/Reported",
    "{\"value\":[]}",
    sizeof("{\"value\":[]}") - 1,
    true);

  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0002/ep1/Groups/Commands/RemoveAllGroups",
    "{}",
    sizeof("{}") - 1);

  // Call it on NodeID 4, ep 0, No group deleted, but publication will happen
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0004/ep0/Groups/Attributes/GroupList/Desired",
    "{\"value\":[]}",
    sizeof("{\"value\":[]}") - 1,
    true);
  uic_mqtt_publish_Expect(
    "ucl/by-unid/zw-00000000-0004/ep0/Groups/Attributes/GroupList/Reported",
    "{\"value\":[]}",
    sizeof("{\"value\":[]}") - 1,
    true);
  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-0004/ep0/Groups/Commands/RemoveAllGroups",
    "{dummy payload}",
    sizeof("{dummy payload}") - 1);

  // Call it on NodeID 0xAA, ep0, Nothing should happen since the node does not exist
  incoming_publication_callback_save(
    "ucl/by-unid/zw-00000000-00AA/ep0/Groups/Commands/RemoveAllGroups",
    "{}",
    sizeof("{}") - 1);
}

void test_zcl_group_cluster_server_delete_network_status_test()
{
  // Delete a network status. Nothing should happen.
  zwave_node_id_t node_id_2 = 2;
  attribute_store_node_t node_id_2_node
    = attribute_store_get_node_child_by_value(home_id_node,
                                              ATTRIBUTE_NODE_ID,
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&node_id_2,
                                              sizeof(node_id_2),
                                              0);

  attribute_store_node_t network_status_2
    = attribute_store_get_node_child_by_type(node_id_2_node,
                                             ATTRIBUTE_NETWORK_STATUS,
                                             0);
  attribute_store_delete_node(network_status_2);
}

void test_zcl_group_cluster_server_delete_endpoint_test()
{
  // Delete an endpoint. We will clean up the publications
  zwave_node_id_t node_id_2 = 2;
  attribute_store_node_t node_id_2_node
    = attribute_store_get_node_child_by_value(home_id_node,
                                              ATTRIBUTE_NODE_ID,
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&node_id_2,
                                              sizeof(node_id_2),
                                              0);

  zwave_endpoint_id_t endpoint_2 = 2;
  attribute_store_node_t endpoint_2_node
    = attribute_store_get_node_child_by_value(node_id_2_node,
                                              ATTRIBUTE_ENDPOINT_ID,
                                              REPORTED_ATTRIBUTE,
                                              (uint8_t *)&endpoint_2,
                                              sizeof(endpoint_2),
                                              0);
  uic_mqtt_unretain_Expect("ucl/by-unid/zw-00000000-0002/ep2/Groups");
  attribute_store_delete_node(endpoint_2_node);
}

void test_zcl_group_cluster_server_teardown_test()
{
  // Teardown the Group Server cluster, nothing should happen
  zcl_group_cluster_server_teardown();
}
