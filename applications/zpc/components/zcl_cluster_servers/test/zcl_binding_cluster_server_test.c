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
#include "zcl_binding_cluster_server.h"
#include "unity.h"

// Definitions
#include "dotdot_cluster_id_definitions.h"
#include "attribute_store_defined_attribute_types.h"
#include "zwave_command_class_association_types.h"
#include "ucl_definitions.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Unify components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"

// Mocks
#include "dotdot_mqtt_mock.h"
#include "zwave_network_management_mock.h"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_##type
#define NODE_BASE_TOPIC "ucl/by-unid/zw-CAFECAFE-0004/ep3"

// Callback functions
static uic_mqtt_dotdot_binding_bind_callback_t bind_callback     = NULL;
static uic_mqtt_dotdot_binding_unbind_callback_t unbind_callback = NULL;
static uic_mqtt_dotdot_binding_bind_to_protocol_controller_callback_t
  bind_to_protocol_controller_callback
  = NULL;
static uic_mqtt_dotdot_binding_unbind_from_protocol_controller_callback_t
  unbind_from_protocol_controller_callback
  = NULL;

// Stub functions for intercepting callback registration.
void uic_mqtt_dotdot_binding_bind_callback_set_stub(
  const uic_mqtt_dotdot_binding_bind_callback_t callback, int cmock_num_calls)
{
  bind_callback = callback;
}

void uic_mqtt_dotdot_binding_unbind_callback_set_stub(
  const uic_mqtt_dotdot_binding_unbind_callback_t callback, int cmock_num_calls)
{
  unbind_callback = callback;
}

void uic_mqtt_dotdot_binding_bind_to_protocol_controller_callback_set_stub(
  const uic_mqtt_dotdot_binding_bind_to_protocol_controller_callback_t callback,
  int cmock_num_calls)
{
  bind_to_protocol_controller_callback = callback;
}

void uic_mqtt_dotdot_binding_unbind_from_protocol_controller_callback_set_stub(
  const uic_mqtt_dotdot_binding_unbind_from_protocol_controller_callback_t
    callback,
  int cmock_num_calls)
{
  unbind_from_protocol_controller_callback = callback;
}

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
  zwave_unid_set_home_id(home_id);
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);
  zwave_network_management_get_node_id_IgnoreAndReturn(zpc_node_id);

  // Intercept the dotdot MQTT callbacks
  bind_callback                            = NULL;
  unbind_callback                          = NULL;
  bind_to_protocol_controller_callback     = NULL;
  unbind_from_protocol_controller_callback = NULL;
  uic_mqtt_dotdot_binding_bind_callback_set_Stub(
    &uic_mqtt_dotdot_binding_bind_callback_set_stub);
  uic_mqtt_dotdot_binding_unbind_callback_set_Stub(
    &uic_mqtt_dotdot_binding_unbind_callback_set_stub);
  uic_mqtt_dotdot_binding_bind_to_protocol_controller_callback_set_Stub(
    &uic_mqtt_dotdot_binding_bind_to_protocol_controller_callback_set_stub);
  uic_mqtt_dotdot_binding_unbind_from_protocol_controller_callback_set_Stub(
    &uic_mqtt_dotdot_binding_unbind_from_protocol_controller_callback_set_stub);

  // Run the component init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, binding_cluster_server_init());
}

void test_binding_cluster_server_test_publish_attributes_after_network_status_update()
{
  // Just create association data under the test node id.
  // Group 1
  association_group_id_t group_id = 1;
  attribute_store_node_t group_id_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(GROUP_ID),
                              &group_id,
                              sizeof(group_id));

  attribute_store_node_t command_list_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST,
    group_id_node);
  const uint8_t command_list_1[] = {COMMAND_CLASS_DOOR_LOCK,
                                    DOOR_LOCK_OPERATION_SET,
                                    COMMAND_CLASS_DOOR_LOCK,
                                    DOOR_LOCK_CONFIGURATION_SET};
  attribute_store_set_reported(command_list_node,
                               command_list_1,
                               sizeof(command_list_1));
  association_group_count_t group_count = 1;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(SUPPORTED_GROUPINGS),
                                     &group_count,
                                     sizeof(group_count));

  // Create a network status and set it to online functional, will trigger publications
  uic_mqtt_dotdot_binding_bindable_cluster_list_publish_ExpectAndReturn(
    NODE_BASE_TOPIC,
    1,
    NULL,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_binding_bindable_cluster_list_publish_IgnoreArg_value();
  uic_mqtt_dotdot_binding_publish_cluster_revision_Expect(NODE_BASE_TOPIC, 1);
  uic_mqtt_dotdot_binding_binding_table_publish_ExpectAndReturn(
    NODE_BASE_TOPIC,
    0,
    NULL,
    UCL_MQTT_PUBLISH_TYPE_DESIRED,
    SL_STATUS_OK);
  uic_mqtt_dotdot_binding_binding_table_publish_ExpectAndReturn(
    NODE_BASE_TOPIC,
    0,
    NULL,
    UCL_MQTT_PUBLISH_TYPE_REPORTED,
    SL_STATUS_OK);
  uic_mqtt_dotdot_binding_binding_table_full_publish_ExpectAndReturn(
    NODE_BASE_TOPIC,
    false,
    UCL_MQTT_PUBLISH_TYPE_DESIRED,
    SL_STATUS_OK);
  uic_mqtt_dotdot_binding_binding_table_full_publish_ExpectAndReturn(
    NODE_BASE_TOPIC,
    false,
    UCL_MQTT_PUBLISH_TYPE_REPORTED,
    SL_STATUS_OK);

  node_state_topic_state_t network_status = NODE_STATE_TOPIC_STATE_INCLUDED;
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));
}

void test_binding_cluster_server_test_publish_attributes_after_group_command_list_update()
{
  // Just create association data under the test node id.
  // Group 1
  association_group_id_t group_id = 1;
  attribute_store_node_t group_id_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(GROUP_ID),
                              &group_id,
                              sizeof(group_id));

  association_group_count_t group_count = 1;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(SUPPORTED_GROUPINGS),
                                     &group_count,
                                     sizeof(group_count));

  node_state_topic_state_t network_status
    = NODE_STATE_TOPIC_STATE_INTERVIEW_FAIL;
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));
  // Set the network status to online functional, it will publish
  // the cluster revision, and no bindable cluster
  uic_mqtt_dotdot_binding_bindable_cluster_list_publish_ExpectAndReturn(
    NODE_BASE_TOPIC,
    0,
    NULL,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_binding_bindable_cluster_list_publish_IgnoreArg_value();
  uic_mqtt_dotdot_binding_publish_cluster_revision_Expect(NODE_BASE_TOPIC, 1);
  uic_mqtt_dotdot_binding_binding_table_publish_ExpectAndReturn(
    NODE_BASE_TOPIC,
    0,
    NULL,
    UCL_MQTT_PUBLISH_TYPE_DESIRED,
    SL_STATUS_OK);
  uic_mqtt_dotdot_binding_binding_table_publish_ExpectAndReturn(
    NODE_BASE_TOPIC,
    0,
    NULL,
    UCL_MQTT_PUBLISH_TYPE_REPORTED,
    SL_STATUS_OK);
  uic_mqtt_dotdot_binding_binding_table_full_publish_ExpectAndReturn(
    NODE_BASE_TOPIC,
    true,
    UCL_MQTT_PUBLISH_TYPE_DESIRED,
    SL_STATUS_OK);
  uic_mqtt_dotdot_binding_binding_table_full_publish_ExpectAndReturn(
    NODE_BASE_TOPIC,
    true,
    UCL_MQTT_PUBLISH_TYPE_REPORTED,
    SL_STATUS_OK);

  network_status = NODE_STATE_TOPIC_STATE_INCLUDED;
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));

  // Now update the group command list, it will publish the bindable cluster list (doorLock)
  uic_mqtt_dotdot_binding_bindable_cluster_list_publish_ExpectAndReturn(
    NODE_BASE_TOPIC,
    1,
    NULL,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_binding_bindable_cluster_list_publish_IgnoreArg_value();
  attribute_store_node_t command_list_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST,
    group_id_node);
  const uint8_t command_list_1[] = {COMMAND_CLASS_DOOR_LOCK,
                                    DOOR_LOCK_OPERATION_SET,
                                    COMMAND_CLASS_DOOR_LOCK,
                                    DOOR_LOCK_CONFIGURATION_SET};
  attribute_store_set_reported(command_list_node,
                               command_list_1,
                               sizeof(command_list_1));
}

void test_binding_cluster_server_no_publish_for_zpc()
{
  // Just create association data under the test node id.
  // Group 1
  association_group_id_t group_id = 1;
  attribute_store_node_t group_id_node
    = attribute_store_emplace(zpc_endpoint_id_node,
                              ATTRIBUTE(GROUP_ID),
                              &group_id,
                              sizeof(group_id));

  association_group_count_t group_count = 1;
  attribute_store_set_child_reported(zpc_endpoint_id_node,
                                     ATTRIBUTE(SUPPORTED_GROUPINGS),
                                     &group_count,
                                     sizeof(group_count));

  node_state_topic_state_t network_status = NODE_STATE_TOPIC_STATE_INCLUDED;
  attribute_store_set_child_reported(zpc_node_id_node,
                                     ATTRIBUTE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));

  attribute_store_node_t command_list_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST,
    group_id_node);
  const uint8_t command_list_1[] = {COMMAND_CLASS_DOOR_LOCK,
                                    DOOR_LOCK_OPERATION_SET,
                                    COMMAND_CLASS_DOOR_LOCK,
                                    DOOR_LOCK_CONFIGURATION_SET};
  attribute_store_set_reported(command_list_node,
                               command_list_1,
                               sizeof(command_list_1));
}

void test_binding_cluster_server_no_valid_unid()
{
  // Just create association data under the root node. Nothing should happen.
  attribute_store_node_t root_node = attribute_store_get_root();

  association_group_id_t group_id = 1;
  attribute_store_node_t group_id_node
    = attribute_store_emplace(root_node,
                              ATTRIBUTE(GROUP_ID),
                              &group_id,
                              sizeof(group_id));

  association_group_count_t group_count = 1;
  attribute_store_set_child_reported(root_node,
                                     ATTRIBUTE(SUPPORTED_GROUPINGS),
                                     &group_count,
                                     sizeof(group_count));

  node_state_topic_state_t network_status = NODE_STATE_TOPIC_STATE_INCLUDED;
  attribute_store_set_child_reported(root_node,
                                     ATTRIBUTE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));

  attribute_store_node_t command_list_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST,
    group_id_node);
  const uint8_t command_list_1[] = {COMMAND_CLASS_DOOR_LOCK,
                                    DOOR_LOCK_OPERATION_SET,
                                    COMMAND_CLASS_DOOR_LOCK,
                                    DOOR_LOCK_CONFIGURATION_SET};
  attribute_store_set_reported(command_list_node,
                               command_list_1,
                               sizeof(command_list_1));
}

void test_binding_cluster_server_support_check()
{
  TEST_ASSERT_NOT_NULL(bind_callback);
  TEST_ASSERT_NOT_NULL(unbind_callback);

  unid_t test_unid;
  zwave_unid_from_node_id(node_id, test_unid);
  unid_t zpc_unid;
  zwave_unid_from_node_id(zpc_node_id, zpc_unid);

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    bind_callback(test_unid,
                                  endpoint_id,
                                  UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                  "OnOff",
                                  zpc_unid,
                                  0));

  // Now we add COMMAND_CLASS_ASSOCIATION in the NIF:
  attribute_store_node_t nif_node
    = attribute_store_add_node(ATTRIBUTE_ZWAVE_NIF, endpoint_id_node);
  uint8_t nif_content[] = {COMMAND_CLASS_SECURITY_2, COMMAND_CLASS_ASSOCIATION};
  attribute_store_set_reported(nif_node, nif_content, sizeof(nif_content));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    bind_callback(test_unid,
                                  endpoint_id,
                                  UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                  "OnOff",
                                  zpc_unid,
                                  0));

  // Now we add COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V2 in the NIF:
  uint8_t nif_content_2[]
    = {COMMAND_CLASS_SECURITY_2, COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V2};
  attribute_store_set_reported(nif_node, nif_content_2, sizeof(nif_content_2));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    bind_callback(test_unid,
                                  endpoint_id,
                                  UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                  NULL,
                                  zpc_unid,
                                  0));

  // Should be the same with Unbind:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    unbind_callback(test_unid,
                                    endpoint_id,
                                    UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                    NULL,
                                    zpc_unid,
                                    0));
}

void test_binding_cluster_server_bind_command()
{
  TEST_ASSERT_NOT_NULL(bind_callback);

  unid_t test_unid;
  zwave_unid_from_node_id(node_id, test_unid);
  unid_t zpc_unid;
  zwave_unid_from_node_id(zpc_node_id, zpc_unid);

  // Nothing should happen with no association groups:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    bind_callback(test_unid,
                                  endpoint_id,
                                  UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                  "OnOff",
                                  zpc_unid,
                                  0));

  // Now create a group 1 sending Door Lock commands
  association_group_id_t group_id = 1;
  attribute_store_node_t group_id_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(GROUP_ID),
                              &group_id,
                              sizeof(group_id));

  association_group_capacity_t group_capacity = 10;
  attribute_store_emplace(group_id_node,
                          ATTRIBUTE(MAX_NODES_SUPPORTED),
                          &group_capacity,
                          sizeof(group_capacity));

  attribute_store_node_t command_list_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST,
    group_id_node);
  const uint8_t command_list_1[] = {COMMAND_CLASS_DOOR_LOCK,
                                    DOOR_LOCK_OPERATION_SET,
                                    COMMAND_CLASS_DOOR_LOCK,
                                    DOOR_LOCK_CONFIGURATION_SET};
  attribute_store_set_reported(command_list_node,
                               command_list_1,
                               sizeof(command_list_1));
  association_group_count_t group_count = 1;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(SUPPORTED_GROUPINGS),
                                     &group_count,
                                     sizeof(group_count));
  attribute_store_node_t group_content_node
    = attribute_store_add_node(ATTRIBUTE(GROUP_CONTENT), group_id_node);

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(group_content_node, DESIRED_ATTRIBUTE));

  // Try to bind Onoff, the Group 1 will be untouched:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    bind_callback(test_unid,
                                  endpoint_id,
                                  UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                  "OnOff",
                                  zpc_unid,
                                  0));

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(group_content_node, DESIRED_ATTRIBUTE));

  // Try to bind Lock, the Group 1 will be untouched:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    bind_callback(test_unid,
                                  endpoint_id,
                                  UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                  "DoorLock",
                                  zpc_unid,
                                  0));

  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(group_content_node, DESIRED_ATTRIBUTE));

  // Try with a non-existing UNID, it will consider the association illegal.
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    bind_callback(test_unid,
                                  endpoint_id,
                                  UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                  "DoorLock",
                                  "zw-01234456-00AA",
                                  0));

  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(group_content_node, DESIRED_ATTRIBUTE));
}

void test_binding_cluster_server_unbind_command()
{
  TEST_ASSERT_NOT_NULL(unbind_callback);

  unid_t test_unid;
  zwave_unid_from_node_id(node_id, test_unid);
  unid_t zpc_unid;
  zwave_unid_from_node_id(zpc_node_id, zpc_unid);

  // Nothing should happen with no association groups:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    unbind_callback(test_unid,
                                    endpoint_id,
                                    UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                    "OnOff",
                                    zpc_unid,
                                    0));

  // Try with a non-existing UNID, it will consider the association illegal.
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    unbind_callback(test_unid,
                                    endpoint_id,
                                    UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                    "DoorLock",
                                    "zw-01234456-00AA",
                                    0));

  // Now create a group 1 sending Door Lock commands
  association_group_id_t group_id = 1;
  attribute_store_node_t group_id_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(GROUP_ID),
                              &group_id,
                              sizeof(group_id));

  association_group_capacity_t group_capacity = 10;
  attribute_store_emplace(group_id_node,
                          ATTRIBUTE(MAX_NODES_SUPPORTED),
                          &group_capacity,
                          sizeof(group_capacity));

  attribute_store_node_t command_list_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST,
    group_id_node);
  const uint8_t command_list_1[] = {COMMAND_CLASS_DOOR_LOCK,
                                    DOOR_LOCK_OPERATION_SET,
                                    COMMAND_CLASS_DOOR_LOCK,
                                    DOOR_LOCK_CONFIGURATION_SET};
  attribute_store_set_reported(command_list_node,
                               command_list_1,
                               sizeof(command_list_1));
  association_group_count_t group_count = 1;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(SUPPORTED_GROUPINGS),
                                     &group_count,
                                     sizeof(group_count));
  attribute_store_node_t group_content_node
    = attribute_store_add_node(ATTRIBUTE(GROUP_CONTENT), group_id_node);
  uint8_t group_content[] = {(uint8_t)zpc_node_id};
  attribute_store_set_reported(group_content_node,
                               group_content,
                               sizeof(group_content));

  // Try to unbind Onoff, the Group 1 will be untouched:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    unbind_callback(test_unid,
                                    endpoint_id,
                                    UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                    "OnOff",
                                    zpc_unid,
                                    0));

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(group_content_node, DESIRED_ATTRIBUTE));

  // Try to unbind Lock, the Group 1 will remove the ZPC UNID
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    unbind_callback(test_unid,
                                    endpoint_id,
                                    UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                    "DoorLock",
                                    zpc_unid,
                                    0));

  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(group_content_node, DESIRED_ATTRIBUTE));
}

void test_binding_cluster_server_bind_to_protocol_controller_command()
{
  TEST_ASSERT_NOT_NULL(bind_to_protocol_controller_callback);

  unid_t test_unid;
  zwave_unid_from_node_id(node_id, test_unid);

  // Nothing will happen here, we did not setup any association groups
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    bind_to_protocol_controller_callback(test_unid,
                                         endpoint_id,
                                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                         "OnOff"));

  // Now create a group 1 sending Door Lock commands
  association_group_id_t group_id = 1;
  attribute_store_node_t group_id_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(GROUP_ID),
                              &group_id,
                              sizeof(group_id));

  association_group_capacity_t group_capacity = 10;
  attribute_store_emplace(group_id_node,
                          ATTRIBUTE(MAX_NODES_SUPPORTED),
                          &group_capacity,
                          sizeof(group_capacity));

  attribute_store_node_t command_list_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST,
    group_id_node);
  const uint8_t command_list_1[] = {COMMAND_CLASS_DOOR_LOCK,
                                    DOOR_LOCK_OPERATION_SET,
                                    COMMAND_CLASS_DOOR_LOCK,
                                    DOOR_LOCK_CONFIGURATION_SET};
  attribute_store_set_reported(command_list_node,
                               command_list_1,
                               sizeof(command_list_1));
  association_group_count_t group_count = 1;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(SUPPORTED_GROUPINGS),
                                     &group_count,
                                     sizeof(group_count));
  attribute_store_node_t group_content_node
    = attribute_store_add_node(ATTRIBUTE(GROUP_CONTENT), group_id_node);

  // Try to bind Onoff, the Group 1 will be untouched:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    bind_to_protocol_controller_callback(test_unid,
                                         endpoint_id,
                                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                         "OnOff"));

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(group_content_node, DESIRED_ATTRIBUTE));

  // Try to unbind Lock, the Group 1 will add the ZPC UNID
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    bind_to_protocol_controller_callback(test_unid,
                                         endpoint_id,
                                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                         "DoorLock"));

  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(group_content_node, DESIRED_ATTRIBUTE));
}

void test_binding_cluster_server_unbind_from_protocol_controller_command()
{
  TEST_ASSERT_NOT_NULL(unbind_from_protocol_controller_callback);

  unid_t test_unid;
  zwave_unid_from_node_id(node_id, test_unid);

  // Nothing will happen here, we did not setup any association groups
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    unbind_from_protocol_controller_callback(
                      test_unid,
                      endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      "OnOff"));

  // Now create a group 1 sending Door Lock commands
  association_group_id_t group_id = 1;
  attribute_store_node_t group_id_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(GROUP_ID),
                              &group_id,
                              sizeof(group_id));

  association_group_capacity_t group_capacity = 10;
  attribute_store_emplace(group_id_node,
                          ATTRIBUTE(MAX_NODES_SUPPORTED),
                          &group_capacity,
                          sizeof(group_capacity));

  attribute_store_node_t command_list_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST,
    group_id_node);
  const uint8_t command_list_1[] = {COMMAND_CLASS_DOOR_LOCK,
                                    DOOR_LOCK_OPERATION_SET,
                                    COMMAND_CLASS_DOOR_LOCK,
                                    DOOR_LOCK_CONFIGURATION_SET};
  attribute_store_set_reported(command_list_node,
                               command_list_1,
                               sizeof(command_list_1));
  association_group_count_t group_count = 1;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(SUPPORTED_GROUPINGS),
                                     &group_count,
                                     sizeof(group_count));
  attribute_store_node_t group_content_node
    = attribute_store_add_node(ATTRIBUTE(GROUP_CONTENT), group_id_node);
  uint8_t group_content[] = {(uint8_t)zpc_node_id};
  attribute_store_set_reported(group_content_node,
                               group_content,
                               sizeof(group_content));

  // Try to unbind Onoff, nothing will happen
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    unbind_from_protocol_controller_callback(
                      test_unid,
                      endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      "OnOff"));

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(group_content_node, DESIRED_ATTRIBUTE));

  // Try to unbind Lock, the Group 1 will remove the ZPC UNID
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    unbind_from_protocol_controller_callback(
                      test_unid,
                      endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      "DoorLock"));

  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(group_content_node, DESIRED_ATTRIBUTE));
}

void test_binding_cluster_server_bind_multiple_groups_and_publish()
{
  TEST_ASSERT_NOT_NULL(bind_to_protocol_controller_callback);

  unid_t test_unid;
  zwave_unid_from_node_id(node_id, test_unid);

  // Nothing will happen here, we did not setup any association groups
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    bind_to_protocol_controller_callback(test_unid,
                                         endpoint_id,
                                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                         "OnOff"));

  // Now create a group 1 sending Binary Switch, group 2 sending Basic Set:
  association_group_count_t group_count = 2;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(SUPPORTED_GROUPINGS),
                                     &group_count,
                                     sizeof(group_count));
  // Group 1
  const association_group_id_t group_id_1 = 1;
  attribute_store_node_t group_id_1_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(GROUP_ID),
                              &group_id_1,
                              sizeof(group_id_1));

  association_group_capacity_t group_capacity = 10;
  attribute_store_set_child_reported(group_id_1_node,
                                     ATTRIBUTE(MAX_NODES_SUPPORTED),
                                     &group_capacity,
                                     sizeof(group_capacity));

  attribute_store_node_t command_list_1_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST,
    group_id_1_node);
  const uint8_t command_list_1[]
    = {COMMAND_CLASS_SWITCH_BINARY, SWITCH_BINARY_SET};
  attribute_store_set_reported(command_list_1_node,
                               command_list_1,
                               sizeof(command_list_1));
  attribute_store_node_t group_content_1_node
    = attribute_store_add_node(ATTRIBUTE(GROUP_CONTENT), group_id_1_node);
  //Group 2
  const association_group_id_t group_id_2 = 2;
  attribute_store_node_t group_id_2_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(GROUP_ID),
                              &group_id_2,
                              sizeof(group_id_2));
  attribute_store_set_child_reported(group_id_2_node,
                                     ATTRIBUTE(MAX_NODES_SUPPORTED),
                                     &group_capacity,
                                     sizeof(group_capacity));

  attribute_store_node_t command_list_2_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_COMMAND_LIST,
    group_id_2_node);
  const uint8_t command_list_2[] = {COMMAND_CLASS_BASIC, BASIC_SET};
  attribute_store_set_reported(command_list_2_node,
                               command_list_2,
                               sizeof(command_list_2));
  attribute_store_node_t group_content_2_node
    = attribute_store_add_node(ATTRIBUTE(GROUP_CONTENT), group_id_2_node);

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(group_content_1_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(group_content_2_node, DESIRED_ATTRIBUTE));

  // Try to bind Lock, nothing should happen:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    bind_to_protocol_controller_callback(test_unid,
                                         endpoint_id,
                                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                         "DoorLock"));

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(group_content_1_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(group_content_2_node, DESIRED_ATTRIBUTE));

  // Try to bind OnOff, It will add the ZPC:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    bind_to_protocol_controller_callback(test_unid,
                                         endpoint_id,
                                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                         "OnOff"));

  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(group_content_1_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(group_content_2_node, DESIRED_ATTRIBUTE));

  // Now set the reported as desired, it should trigger a Binding Table Publication:
  attribute_store_set_reported_as_desired(group_content_1_node);
  attribute_store_set_reported_as_desired(group_content_2_node);

  // Set it to online functional to trigger publications:
  uic_mqtt_dotdot_binding_bindable_cluster_list_publish_ExpectAndReturn(
    NODE_BASE_TOPIC,
    1,
    NULL,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_binding_bindable_cluster_list_publish_IgnoreArg_value();
  uic_mqtt_dotdot_binding_publish_cluster_revision_Expect(NODE_BASE_TOPIC, 1);
  uic_mqtt_dotdot_binding_binding_table_publish_ExpectAndReturn(
    NODE_BASE_TOPIC,
    1,  // 1 items in the binding table, ProtocolController, ep0, OnOff
    NULL,
    UCL_MQTT_PUBLISH_TYPE_DESIRED,
    SL_STATUS_OK);
  uic_mqtt_dotdot_binding_binding_table_publish_IgnoreArg_value();
  uic_mqtt_dotdot_binding_binding_table_publish_ExpectAndReturn(
    NODE_BASE_TOPIC,
    1,  // 1 item in the binding table
    NULL,
    UCL_MQTT_PUBLISH_TYPE_REPORTED,
    SL_STATUS_OK);
  uic_mqtt_dotdot_binding_binding_table_publish_IgnoreArg_value();
  uic_mqtt_dotdot_binding_binding_table_full_publish_ExpectAndReturn(
    NODE_BASE_TOPIC,
    false,
    UCL_MQTT_PUBLISH_TYPE_DESIRED,
    SL_STATUS_OK);
  uic_mqtt_dotdot_binding_binding_table_full_publish_ExpectAndReturn(
    NODE_BASE_TOPIC,
    false,
    UCL_MQTT_PUBLISH_TYPE_REPORTED,
    SL_STATUS_OK);

  node_state_topic_state_t network_status = NODE_STATE_TOPIC_STATE_INCLUDED;
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));
}