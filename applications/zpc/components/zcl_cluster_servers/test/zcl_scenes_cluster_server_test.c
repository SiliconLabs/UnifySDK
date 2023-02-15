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
#include "zcl_scenes_cluster_server.h"
#include "unity.h"
#include "unity_helpers.h"

// Unify components
#include "datastore.h"
#include "attribute_store_fixt.h"
#include "attribute_store_helper.h"
#include "attribute_timeouts.h"
#include "dotdot_attribute_id_definitions.h"
#include "unify_dotdot_defined_attribute_types.h"
#include "sl_log.h"

// Interfaces/definitions
#include "ucl_definitions.h"

// ZPC Components
#include "zwave_unid.h"
#include "attribute_store_defined_attribute_types.h"
#include "zpc_attribute_store_type_registration.h"

// Mock includes
#include "dotdot_mqtt_mock.h"
#include "zwave_network_management_mock.h"
#include "zpc_dotdot_mqtt_group_dispatch_mock.h"

// Test helpers
#include "contiki_test_helper.h"
#include "mqtt_test_helper.h"
#include "zpc_attribute_store_test_helper.h"

// Generic includes
#include <string.h>

// Constants
static const char SCENE_TABLE_REPORTED_TOPIC[]
  = "ucl/by-unid/zw-CAFECAFE-0004/ep3/Scenes/Attributes/SceneTable/Reported";
static const char SCENE_TABLE_DESIRED_TOPIC[]
  = "ucl/by-unid/zw-CAFECAFE-0004/ep3/Scenes/Attributes/SceneTable/Reported";
static const char ADD_SCENE_TOPIC[]
  = "ucl/by-unid/zw-CAFECAFE-0004/ep3/Scenes/Commands/AddScene";
static const char ADD_SCENE_GROUP_TOPIC[]
  = "ucl/by-group/4/Scenes/Commands/AddScene";

// Private variables
static uic_mqtt_dotdot_scenes_add_scene_callback_t add_scene_command   = NULL;
static uic_mqtt_dotdot_scenes_view_scene_callback_t view_scene_command = NULL;
static uic_mqtt_dotdot_scenes_remove_scene_callback_t remove_scene_command
  = NULL;
static uic_mqtt_dotdot_scenes_remove_all_scenes_callback_t
  remove_all_scenes_command
  = NULL;
static uic_mqtt_dotdot_scenes_store_scene_callback_t store_scene_command = NULL;
static uic_mqtt_dotdot_scenes_recall_scene_callback_t recall_scene_command
  = NULL;
static uic_mqtt_dotdot_scenes_get_scene_membership_callback_t
  get_scene_membership_command
  = NULL;

// Stub functions
void uic_mqtt_dotdot_scenes_add_scene_callback_set_stub(
  const uic_mqtt_dotdot_scenes_add_scene_callback_t callback,
  int cmock_num_calls)
{
  add_scene_command = callback;
}

void uic_mqtt_dotdot_scenes_view_scene_callback_set_stub(
  const uic_mqtt_dotdot_scenes_view_scene_callback_t callback,
  int cmock_num_calls)
{
  view_scene_command = callback;
}

void uic_mqtt_dotdot_scenes_remove_scene_callback_set_stub(
  const uic_mqtt_dotdot_scenes_remove_scene_callback_t callback,
  int cmock_num_calls)
{
  remove_scene_command = callback;
}

void uic_mqtt_dotdot_scenes_remove_all_scenes_callback_set_stub(
  const uic_mqtt_dotdot_scenes_remove_all_scenes_callback_t callback,
  int cmock_num_calls)
{
  remove_all_scenes_command = callback;
}

void uic_mqtt_dotdot_scenes_store_scene_callback_set_stub(
  const uic_mqtt_dotdot_scenes_store_scene_callback_t callback,
  int cmock_num_calls)
{
  store_scene_command = callback;
}

void uic_mqtt_dotdot_scenes_recall_scene_callback_set_stub(
  const uic_mqtt_dotdot_scenes_recall_scene_callback_t callback,
  int cmock_num_calls)
{
  recall_scene_command = callback;
}

void uic_mqtt_dotdot_scenes_get_scene_membership_callback_set_stub(
  const uic_mqtt_dotdot_scenes_get_scene_membership_callback_t callback,
  int cmock_num_calls)
{
  get_scene_membership_command = callback;
}
/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  contiki_test_helper_init();
  attribute_timeouts_init();
  mqtt_test_helper_init();

  // Create our test network.
  zpc_attribute_store_register_known_attribute_types();
  zwave_unid_set_home_id(home_id);
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  zcl_scenes_cluster_server_teardown();
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  attribute_store_delete_node(attribute_store_get_root());
  zpc_attribute_store_test_helper_create_network();

  add_scene_command            = NULL;
  view_scene_command           = NULL;
  remove_scene_command         = NULL;
  remove_all_scenes_command    = NULL;
  store_scene_command          = NULL;
  recall_scene_command         = NULL;
  get_scene_membership_command = NULL;

  // Set up the stubs
  uic_mqtt_dotdot_scenes_add_scene_callback_set_Stub(
    &uic_mqtt_dotdot_scenes_add_scene_callback_set_stub);

  uic_mqtt_dotdot_scenes_view_scene_callback_set_Stub(
    &uic_mqtt_dotdot_scenes_view_scene_callback_set_stub);

  uic_mqtt_dotdot_scenes_remove_scene_callback_set_Stub(
    &uic_mqtt_dotdot_scenes_remove_scene_callback_set_stub);

  uic_mqtt_dotdot_scenes_remove_all_scenes_callback_set_Stub(
    &uic_mqtt_dotdot_scenes_remove_all_scenes_callback_set_stub);

  uic_mqtt_dotdot_scenes_store_scene_callback_set_Stub(
    &uic_mqtt_dotdot_scenes_store_scene_callback_set_stub);

  uic_mqtt_dotdot_scenes_recall_scene_callback_set_Stub(
    &uic_mqtt_dotdot_scenes_recall_scene_callback_set_stub);

  uic_mqtt_dotdot_scenes_get_scene_membership_callback_set_Stub(
    &uic_mqtt_dotdot_scenes_get_scene_membership_callback_set_stub);

  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);
  zwave_network_management_get_node_id_IgnoreAndReturn(zpc_node_id);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, zcl_scenes_cluster_server_init());

  // Add a OnOff and CurrentLevel to our test supporting node.
  attribute_store_create_child_if_missing(endpoint_id_node,
                                          DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF);
  attribute_store_create_child_if_missing(
    endpoint_id_node,
    DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL);

  // Configure the supporting node to be online functional:
  const node_state_topic_state_t network_status
    = NODE_STATE_TOPIC_STATE_INCLUDED;
  attribute_store_set_child_reported(node_id_node,
                                     ATTRIBUTE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));
}

void test_scenes_cluster_mapper_add_scene()
{
  TEST_ASSERT_NOT_NULL(add_scene_command);

  // Supported by our test node, but not with the wrong endpoint:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    add_scene_command(supporting_node_unid,
                      endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      NULL,
                      0,
                      NULL,
                      0));
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    add_scene_command(supporting_node_unid,
                      endpoint_id + 1,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      NULL,
                      0,
                      NULL,
                      0));

  // Invoking is a no-op, hanlded in another function.
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    add_scene_command(supporting_node_unid,
                                      endpoint_id,
                                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                      0,
                                      0,
                                      0,
                                      NULL,
                                      0,
                                      NULL,
                                      0));
}

void test_scenes_cluster_mapper_view_scene()
{
  TEST_ASSERT_NOT_NULL(view_scene_command);

  // Supported by our test node, but not with the wrong endpoint:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    view_scene_command(supporting_node_unid,
                       endpoint_id,
                       UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                       0,
                       0));
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    view_scene_command(supporting_node_unid,
                       endpoint_id + 1,
                       UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                       0,
                       0));

  // Invoking with invalid parameters
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    view_scene_command(supporting_node_unid,
                                       endpoint_id,
                                       UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                       0,
                                       255));

  // Happy case
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    view_scene_command(supporting_node_unid,
                                       endpoint_id,
                                       UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                       0,
                                       0));

  // The component should ask for publishing the Scene Table again
  contiki_test_helper_run(20);
}

void test_scenes_cluster_mapper_remove_scene()
{
  TEST_ASSERT_NOT_NULL(remove_scene_command);

  // Supported by our test node, but not with the wrong endpoint
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    remove_scene_command(supporting_node_unid,
                         endpoint_id,
                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                         0,
                         0));
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    remove_scene_command(supporting_node_unid,
                         endpoint_id + 1,
                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                         0,
                         0));

  // Normal case
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    remove_scene_command(supporting_node_unid,
                                         endpoint_id,
                                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                         0,
                                         0));
}

void test_scenes_cluster_mapper_remove_all_scenes()
{
  TEST_ASSERT_NOT_NULL(remove_all_scenes_command);

  // Suppported by our test node, but not with the wrong endpoint
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    remove_all_scenes_command(supporting_node_unid,
                              endpoint_id,
                              UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                              0));
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    remove_all_scenes_command(supporting_node_unid,
                              endpoint_id + 1,
                              UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                              0));

  // Normal case
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    remove_all_scenes_command(supporting_node_unid,
                              endpoint_id,
                              UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                              0));
}

void test_scenes_cluster_mapper_store_scene()
{
  TEST_ASSERT_NOT_NULL(store_scene_command);

  // Supported by our test node, but not if we provide a wrong endpoint
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    store_scene_command(supporting_node_unid,
                        endpoint_id,
                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                        0,
                        2));
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    store_scene_command(supporting_node_unid,
                        endpoint_id + 1,
                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                        0,
                        2));

  // normal call on a non-supporting node will also return fail.
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    store_scene_command(supporting_node_unid,
                                        endpoint_id + 1,
                                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                        0,
                                        3));

  // Normal case
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    store_scene_command(supporting_node_unid,
                                        endpoint_id,
                                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                        0,
                                        3));

  // Test providing a GroupID != 0, it will be rejected if the UNID/EP is not part of the group
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    store_scene_command(supporting_node_unid,
                                        endpoint_id,
                                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                        2,
                                        3));

  // The component should ask for publishing the Scene Table again
  contiki_test_helper_run(20);
}

void test_scenes_cluster_mapper_recall_scene()
{
  TEST_ASSERT_NOT_NULL(recall_scene_command);

  // Support check
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    recall_scene_command(supporting_node_unid,
                         endpoint_id,
                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                         0,
                         2,
                         10));
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    recall_scene_command(supporting_node_unid,
                         endpoint_id + 1,
                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                         0,
                         2,
                         10));

  // No Scene Extension fieldset saved in the attribute store will do nothing
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    recall_scene_command(supporting_node_unid,
                                         endpoint_id,
                                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                         0,
                                         3,
                                         10));

  // No Scene Extension fieldset saved in the attribute store will do nothing
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    recall_scene_command(supporting_node_unid,
                                         endpoint_id,
                                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                         0,
                                         3,
                                         10));
}

void test_scenes_cluster_mapper_get_scene_membership()
{
  TEST_ASSERT_NOT_NULL(get_scene_membership_command);

  // Support check, should be supported by our test node
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    get_scene_membership_command(supporting_node_unid,
                                 endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                 0));
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    get_scene_membership_command(supporting_node_unid,
                                 endpoint_id + 1,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                 0));

  // Normal case
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    get_scene_membership_command(supporting_node_unid,
                                 endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                 0));
}

void test_on_add_scene_command_received_with_errors()
{
  // Nothing should happen when publishing for a non-existing node or endpoint
  // Generate an incoming command published on MQTT:
  const char topic_incorrect_endpoint[]
    = "ucl/by-unid/zw-CAFECAFE-0004/ep12/Scenes/Commands/AddScene";

  // Test payload
  const char payload[]
    = "{\"GroupID\": 1, \"SceneID\": 1, \"TransitionTime\": 23, \"SceneName\": "
      "\"Test Scene\",\"ExtensionFieldSets\":{\"OnOff\": {\"OnOff\": "
      "false,\"OnTime\": 23},\"Level\": {\"CurrentLevel\": 99}}}";

  mqtt_test_helper_publish(topic_incorrect_endpoint, payload, strlen(payload));

  // Try with incorrect UNID
  const char topic_incorrect_unid[]
    = "ucl/by-unid/zw-CAFECAFE-0005/ep3/Scenes/Commands/AddScene";

  mqtt_test_helper_publish(topic_incorrect_unid, payload, strlen(payload));

  // Try with empty message / JSON payload
  mqtt_test_helper_publish(ADD_SCENE_TOPIC, NULL, 0);

  // Try with an incoherent topic:
  const char topic_no_possible_parsing[] = "ucl/by-unidScenesCommands/AddScene";
  mqtt_test_helper_publish(topic_no_possible_parsing, payload, strlen(payload));

  // Get the timer going to check for publications
  contiki_test_helper_run(20);
  TEST_ASSERT_GREATER_OR_EQUAL(
    1,
    mqtt_test_helper_get_num_publish(SCENE_TABLE_REPORTED_TOPIC));
  TEST_ASSERT_GREATER_OR_EQUAL(
    1,
    mqtt_test_helper_get_num_publish(SCENE_TABLE_DESIRED_TOPIC));

  char published_message[1000] = {};
  TEST_ASSERT_NOT_NULL(mqtt_test_helper_pop_publish(SCENE_TABLE_REPORTED_TOPIC,
                                                    published_message));

  const char expected_published_message[] = "{\"value\": []}";
  TEST_ASSERT_EQUAL_JSON(expected_published_message, published_message);
}

void test_on_add_scene_command_received_payload_missing_mandatory_fields()
{
  // Nothing should happen with missing fields.
  const char payload_no_group[]
    = "{\"SceneID\": 1, \"TransitionTime\": 23, \"TransitionTime100ms\": 9, "
      "\"SceneName\": "
      "\"Test Scene\",\"ExtensionFieldSets\":{\"OnOff\": {\"OnOff\": "
      "false,\"OnTime\": 23},\"Level\": {\"CurrentLevel\": 99}}}";

  mqtt_test_helper_publish(ADD_SCENE_TOPIC,
                           payload_no_group,
                           strlen(payload_no_group));

  const char payload_no_scene_id[]
    = "{\"GroupID\": 1, \"TransitionTime\": 23, \"SceneName\": "
      "\"Test Scene\",\"ExtensionFieldSets\":{\"OnOff\": {\"OnOff\": "
      "false,\"OnTime\": 23},\"Level\": {\"CurrentLevel\": 99}}, "
      "\"TransitionTime100ms\": 0}";

  mqtt_test_helper_publish(ADD_SCENE_TOPIC,
                           payload_no_scene_id,
                           strlen(payload_no_scene_id));

  const char payload_no_scene_name[]
    = "{\"GroupID\": 1, \"SceneID\": 1, \"TransitionTime\": 23, "
      ",\"ExtensionFieldSets\":{\"OnOff\": {\"OnOff\": "
      "false,\"OnTime\": 23},\"Level\": {\"CurrentLevel\": 99}}, "
      "\"TransitionTime100ms\": 0}";

  mqtt_test_helper_publish(ADD_SCENE_TOPIC,
                           payload_no_scene_name,
                           strlen(payload_no_scene_name));

  const char payload_no_transition_time[]
    = "{\"GroupID\": 1, \"SceneID\": 1, \"SceneName\": "
      "\"Test Scene\",\"ExtensionFieldSets\":{\"OnOff\": {\"OnOff\": "
      "false,\"OnTime\": 23},\"Level\": {\"CurrentLevel\": 99}}, "
      "\"TransitionTime100ms\": 0}";

  mqtt_test_helper_publish(ADD_SCENE_TOPIC,
                           payload_no_transition_time,
                           strlen(payload_no_transition_time));

  const char payload_no_transition_time_100ms[]
    = "{\"GroupID\": 1, \"SceneID\": 1, \"TransitionTime\": 23, \"SceneName\": "
      "\"Test Scene\",\"ExtensionFieldSets\":{\"OnOff\": {\"OnOff\": "
      "false,\"OnTime\": 23},\"Level\": {\"CurrentLevel\": 99}}}";

  mqtt_test_helper_publish(ADD_SCENE_TOPIC,
                           payload_no_transition_time_100ms,
                           strlen(payload_no_transition_time_100ms));

  const char payload_no_extension_fieldset[]
    = "{\"GroupID\": 1, \"SceneID\": 1, \"TransitionTime\": 23, "
      "\"TransitionTime100ms\": 1, \"SceneName\": "
      "\"Test Scene\"}";

  mqtt_test_helper_publish(ADD_SCENE_TOPIC,
                           payload_no_extension_fieldset,
                           strlen(payload_no_extension_fieldset));

  // Get the timer going to check for publications
  contiki_test_helper_run(20);
  TEST_ASSERT_GREATER_OR_EQUAL(
    1,
    mqtt_test_helper_get_num_publish(SCENE_TABLE_REPORTED_TOPIC));
  TEST_ASSERT_GREATER_OR_EQUAL(
    1,
    mqtt_test_helper_get_num_publish(SCENE_TABLE_DESIRED_TOPIC));

  // Verify that the Scene table is empty:
  // Verify that it did its job:
  char published_message[1000] = {};
  TEST_ASSERT_NOT_NULL(mqtt_test_helper_pop_publish(SCENE_TABLE_REPORTED_TOPIC,
                                                    published_message));

  const char expected_published_message[] = "{\"value\": []}";
  TEST_ASSERT_EQUAL_JSON(expected_published_message, published_message);
}

void test_on_add_scene_command_happy_case()
{
  const char payload[]
    = "{\"GroupID\": 1, \"SceneID\": 1, \"TransitionTime\": 23, "
      "\"TransitionTime100ms\": 1, \"SceneName\": "
      "\"Test Scene\",\"ExtensionFieldSets\":{\"OnOff\": {\"OnOff\": "
      "false,\"OnTime\": 23},\"Level\": {\"CurrentLevel\": 99}}}";

  mqtt_test_helper_publish(ADD_SCENE_TOPIC, payload, strlen(payload));

  // Check if the entry is accepted:
  contiki_test_helper_run(20);
  TEST_ASSERT_GREATER_OR_EQUAL(
    1,
    mqtt_test_helper_get_num_publish(SCENE_TABLE_REPORTED_TOPIC));
  TEST_ASSERT_GREATER_OR_EQUAL(
    1,
    mqtt_test_helper_get_num_publish(SCENE_TABLE_DESIRED_TOPIC));

  // Check out the content of the publication
  char published_message[1000] = {};
  while (NULL
         != mqtt_test_helper_pop_publish(SCENE_TABLE_REPORTED_TOPIC,
                                         published_message)) {
    // This empty loop will get us the last published message
  }

  const char expected_published_message[]
    = "{\"value\":[{\"GroupID\":1,\"SceneID\":1,\"SceneName\":\"Test "
      "Scene\",\"SceneTableExtensions\":{\"Level\":{\"CurrentLevel\":99},"
      "\"OnOff\":{\"OnOff\":false}},\"TransitionTime\":23,"
      "\"TransitionTime100ms\":1}]}";
  TEST_ASSERT_EQUAL_JSON(expected_published_message, published_message);

  // Now recall the added scene:
  TEST_ASSERT_NOT_NULL(recall_scene_command);
  // Here we receive a SL_STATUS_FAIL because the mapper is not active and
  // dotdot_set_scenes_current_group / dotdot_set_scenes_current_scene / dotdot_set_scenes_scene_valid
  // will return SL_STATUS_FAIL due to missing attributes. However, the command should activate the scene.
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    recall_scene_command(supporting_node_unid,
                                         endpoint_id,
                                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                         1,
                                         1,
                                         10));

  // Check on the publications
  contiki_test_helper_run(20);
  TEST_ASSERT_GREATER_OR_EQUAL(
    2,
    mqtt_test_helper_get_num_publish(SCENE_TABLE_REPORTED_TOPIC));
  TEST_ASSERT_GREATER_OR_EQUAL(
    2,
    mqtt_test_helper_get_num_publish(SCENE_TABLE_DESIRED_TOPIC));

  // Check if the actuator settings have been updated
  attribute_store_node_t on_off_attribute
    = attribute_store_get_first_child_by_type(
      endpoint_id_node,
      DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF);
  TEST_ASSERT_EQUAL(0, attribute_store_get_desired_number(on_off_attribute));
  attribute_store_node_t level_attribute
    = attribute_store_get_first_child_by_type(
      endpoint_id_node,
      DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL);
  TEST_ASSERT_EQUAL(99, attribute_store_get_desired_number(level_attribute));

  // Now pretend that the scene is fully applied and
  attribute_store_set_reported_as_desired(on_off_attribute);
  attribute_store_set_reported_as_desired(level_attribute);
  // Store the current Scene as Scene 1, group 0
  TEST_ASSERT_NOT_NULL(store_scene_command);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    store_scene_command(supporting_node_unid,
                                        endpoint_id,
                                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                        0,
                                        1));

  // Check out if we have 2 scenes in the table:
  contiki_test_helper_run(20);
  while (NULL
         != mqtt_test_helper_pop_publish(SCENE_TABLE_REPORTED_TOPIC,
                                         published_message)) {
    // This empty loop will get us the last published message
  }

  const char expected_published_message_2_scenes[]
    = "{\"value\":[{\"GroupID\":1,\"SceneID\":1,\"SceneName\":\"Test "
      "Scene\",\"SceneTableExtensions\":{\"Level\":{\"CurrentLevel\":99},"
      "\"OnOff\":{\"OnOff\":false}},\"TransitionTime\":23,"
      "\"TransitionTime100ms\":1},{\"GroupID\":0,\"SceneID\":1,\"SceneName\":"
      "\"\",\"SceneTableExtensions\":{\"Level\":{\"CurrentLevel\":99},"
      "\"OnOff\":{\"OnOff\":false}},\"TransitionTime\":0,"
      "\"TransitionTime100ms\":0}]}";
  TEST_ASSERT_EQUAL_JSON(expected_published_message_2_scenes,
                         published_message);
}

void test_on_group_add_scene_command()
{
  // Try a 0 length payload:
  mqtt_test_helper_publish(ADD_SCENE_GROUP_TOPIC, NULL, 0);

  const char payload[]
    = "{\"GroupID\": 1, \"SceneID\": 1, \"TransitionTime\": 23, "
      "\"TransitionTime100ms\": 1, \"SceneName\": "
      "\"Test Scene\",\"ExtensionFieldSets\":{\"OnOff\": {\"OnOff\": "
      "false,\"OnTime\": 23},\"Level\": {\"CurrentLevel\": 99}}}";

  // Try with a Null pointer topic:
  mqtt_test_helper_publish(NULL, payload, strlen(payload));

  // Try with a wrong Topic:
  const char wrong_group_topic[]
    = "ucl/WrongTopic/ThisShouldBeAGroupID/Scenes/Commands/AddScene";
  mqtt_test_helper_publish(wrong_group_topic, payload, strlen(payload));

  // This should go through to the dispatch component
  zpc_dotdot_mqtt_group_dispatch_Expect(4,
                                        "Scenes",
                                        "AddScene",
                                        payload,
                                        strlen(payload),
                                        NULL);
  zpc_dotdot_mqtt_group_dispatch_IgnoreArg_callback();
  mqtt_test_helper_publish(ADD_SCENE_GROUP_TOPIC, payload, strlen(payload));
}