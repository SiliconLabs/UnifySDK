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
#include "zwave_command_class_scene_controller_configuration_control.h"
#include "zwave_command_classes_utils.h"
#include "unity.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "zpc_attribute_store_network_helper.h"
#include "zpc_attribute_store_type_registration.h"

// Interface includes
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"
#include "zwave_utils.h"
#include "zwave_controller_types.h"
#include "zwave_command_class_local_scenes_types.h"
#include "zwave_command_class_association_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "attribute_resolver_mock.h"
#include "zpc_attribute_resolver_mock.h"
#include "zwave_command_handler_mock.h"
#include "zwave_network_management_mock.h"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_SCENE_##type

// Static variables
static attribute_resolver_function_t scene_controller_configuration_set = NULL;
static attribute_resolver_function_t scene_controller_configuration_get = NULL;
static zwave_command_handler_t handler                                  = {};
static zwave_controller_connection_info_t connection_info               = {};
static zpc_resolver_event_notification_function_t on_configuration_set_complete
  = NULL;

static uint8_t received_frame[255]  = {};
static uint16_t received_frame_size = 0;

// Stub functions
static sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  if (node_type == ATTRIBUTE(CONTROLLER_CONFIGURATION)) {
    TEST_ASSERT_NOT_NULL(get_func);
    TEST_ASSERT_NOT_NULL(set_func);
    scene_controller_configuration_get = get_func;
    scene_controller_configuration_set = set_func;
  } else {
    TEST_FAIL_MESSAGE("Attribute rule registration on a wrong type");
  }
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME,
                    handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SCENE_CONTROLLER_CONF, handler.command_class);
  TEST_ASSERT_EQUAL(1, handler.version);
  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_NULL(handler.support_handler);
  TEST_ASSERT_FALSE(handler.manual_security_validation);

  return SL_STATUS_OK;
}

static sl_status_t register_send_event_handler_stub(
  attribute_store_type_t type,
  zpc_resolver_event_notification_function_t function,
  int cmock_num_calls)
{
  TEST_ASSERT_EQUAL(ATTRIBUTE(CONTROLLER_CONFIGURATION), type);
  on_configuration_set_complete = function;
  return SL_STATUS_OK;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  zpc_attribute_store_register_known_attribute_types();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

static void
  zwave_command_class_scene_controller_configuration_init_verification()
{
  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  // Resolution send data listener registration
  register_send_event_handler_Stub(&register_send_event_handler_stub);

  // Call init
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_command_class_scene_controller_configuration_control_init());
}

/// Called before each and every test
void setUp()
{
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_set_home_id(home_id);
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);

  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size                = 0;
  scene_controller_configuration_set = NULL;
  scene_controller_configuration_get = NULL;
  on_configuration_set_complete      = NULL;
  memset(&handler, 0, sizeof(zwave_command_handler_t));

  zwave_command_class_scene_controller_configuration_init_verification();

  // Prepare a default connection info with the supporting node
  memset(&connection_info, 0, sizeof(zwave_controller_connection_info_t));
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));
}

void test_zwave_command_class_scene_controller_configuration_set()
{
  TEST_ASSERT_NOT_NULL(scene_controller_configuration_set);

  // No attributes created, we will be unhappy.
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    scene_controller_configuration_set(0,
                                                       received_frame,
                                                       &received_frame_size));
  association_group_id_t group_id = 3;
  attribute_store_node_t group_id_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                              &group_id,
                              sizeof(group_id));

  attribute_store_node_t scene_configuration_node
    = attribute_store_add_node(ATTRIBUTE(CONTROLLER_CONFIGURATION),
                               group_id_node);

  // Conf Children do not exist, it will be default values:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    scene_controller_configuration_set(scene_configuration_node,
                                                       received_frame,
                                                       &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_SCENE_CONTROLLER_CONF,
                                    SCENE_CONTROLLER_CONF_SET,
                                    group_id,
                                    1,
                                    0};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);

  // Conf Children do not exist, it will be default values:
  const local_scene_id_t desired_scene = 58;
  attribute_store_set_child_desired(scene_configuration_node,
                                    ATTRIBUTE(CONTROLLER_SETTING_ID),
                                    &desired_scene,
                                    sizeof(desired_scene));
  const local_scene_dimming_duration_t desired_duration = 0xAA;
  attribute_store_set_child_desired(
    scene_configuration_node,
    ATTRIBUTE(CONTROLLER_SETTING_DIMMING_DURATION),
    &desired_duration,
    sizeof(desired_duration));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    scene_controller_configuration_set(scene_configuration_node,
                                                       received_frame,
                                                       &received_frame_size));

  const uint8_t expected_frame_2[] = {COMMAND_CLASS_SCENE_CONTROLLER_CONF,
                                      SCENE_CONTROLLER_CONF_SET,
                                      group_id,
                                      desired_scene,
                                      desired_duration};
  TEST_ASSERT_EQUAL(sizeof(expected_frame_2), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame_2,
                                expected_frame_2,
                                received_frame_size);
}

void test_zwave_command_class_scene_controller_configuration_get()
{
  TEST_ASSERT_NOT_NULL(scene_controller_configuration_get);

  // No attributes created, we will be unhappy.
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    scene_controller_configuration_get(0,
                                                       received_frame,
                                                       &received_frame_size));
  association_group_id_t group_id = 3;
  attribute_store_node_t group_id_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                              &group_id,
                              sizeof(group_id));

  attribute_store_node_t scene_configuration_node
    = attribute_store_add_node(ATTRIBUTE(CONTROLLER_CONFIGURATION),
                               group_id_node);

  // Happy case
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    scene_controller_configuration_get(scene_configuration_node,
                                                       received_frame,
                                                       &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_SCENE_CONTROLLER_CONF,
                                    SCENE_CONTROLLER_CONF_GET,
                                    group_id};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_scene_controller_attribute_creation()
{
  // We need to create a Group ID on a node that supports Scene Activation,
  // then attributes should be created automatically.
  const uint8_t zwave_nif[] = {COMMAND_CLASS_MULTI_CHANNEL_V4,
                               COMMAND_CLASS_SWITCH_MULTILEVEL,
                               COMMAND_CLASS_DOOR_LOCK,
                               COMMAND_CLASS_SECURITY_2,
                               COMMAND_CLASS_SCENE_CONTROLLER_CONF,
                               COMMAND_CLASS_SECURITY};

  attribute_store_emplace(endpoint_id_node,
                          ATTRIBUTE_ZWAVE_NIF,
                          zwave_nif,
                          sizeof(zwave_nif));

  attribute_store_node_t group_id_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                               endpoint_id_node);

  // Group ID has no defined value yet
  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(group_id_node));
  association_group_id_t group_id = 3;
  attribute_store_set_reported(group_id_node, &group_id, sizeof(group_id));
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(group_id_node));

  // Now all attributes are present
  attribute_store_node_t scene_configuration_node
    = attribute_store_get_first_child_by_type(
      group_id_node,
      ATTRIBUTE(CONTROLLER_CONFIGURATION));
  TEST_ASSERT_EQUAL(
    2,
    attribute_store_get_node_child_count(scene_configuration_node));
  attribute_store_node_t scene_id_node
    = attribute_store_get_first_child_by_type(scene_configuration_node,
                                              ATTRIBUTE(CONTROLLER_SETTING_ID));
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, scene_id_node);
  attribute_store_node_t duration_node
    = attribute_store_get_first_child_by_type(
      scene_configuration_node,
      ATTRIBUTE(CONTROLLER_SETTING_DIMMING_DURATION));
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, duration_node);

  // Check that the value of the configuration gets updated automatically
  // when we update the desired value of the children:
  TEST_ASSERT_FALSE(
    attribute_store_is_reported_defined(scene_configuration_node));
  TEST_ASSERT_FALSE(
    attribute_store_is_desired_defined(scene_configuration_node));

  // No reported value, nothing will happen.
  local_scene_id_t scene_id = 4;
  attribute_store_set_desired(scene_id_node, &scene_id, sizeof(scene_id));

  TEST_ASSERT_FALSE(
    attribute_store_is_reported_defined(scene_configuration_node));
  TEST_ASSERT_FALSE(
    attribute_store_is_desired_defined(scene_configuration_node));

  attribute_store_set_reported(scene_id_node, &scene_id, sizeof(scene_id));
  scene_id += 1;
  attribute_store_set_desired(scene_id_node, &scene_id, sizeof(scene_id));

  TEST_ASSERT_TRUE(
    attribute_store_is_reported_defined(scene_configuration_node));
  TEST_ASSERT_TRUE(
    attribute_store_is_desired_defined(scene_configuration_node));
  TEST_ASSERT_FALSE(attribute_store_is_value_matched(scene_configuration_node));
}

void test_zwave_command_class_scene_controller_on_set_conf_send_complete()
{
  // Here we cheat a little and place the Scene Conf attribute directly
  // under the endpoint. The function never navigates up to check the
  // Association Group ID.
  attribute_store_node_t scene_configuration_node
    = attribute_store_add_node(ATTRIBUTE(CONTROLLER_CONFIGURATION),
                               endpoint_id_node);
  attribute_store_node_t scene_id_node
    = attribute_store_add_node(ATTRIBUTE(CONTROLLER_SETTING_ID),
                               scene_configuration_node);
  local_scene_id_t scene_id = 4;
  attribute_store_set_desired(scene_id_node, &scene_id, sizeof(scene_id));
  attribute_store_node_t duration_node
    = attribute_store_add_node(ATTRIBUTE(CONTROLLER_SETTING_DIMMING_DURATION),
                               scene_configuration_node);
  local_scene_dimming_duration_t duration = 5;
  attribute_store_set_desired(duration_node, &duration, sizeof(duration));

  TEST_ASSERT_NOT_NULL(on_configuration_set_complete);
  on_configuration_set_complete(scene_configuration_node,
                                RESOLVER_SET_RULE,
                                FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS);

  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(scene_id_node));
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(duration_node));
  TEST_ASSERT_TRUE(attribute_store_is_value_matched(scene_id_node));
  TEST_ASSERT_TRUE(attribute_store_is_value_matched(duration_node));

  // Reset, try with a Get rule:
  attribute_store_undefine_reported(scene_id_node);
  attribute_store_undefine_reported(duration_node);

  on_configuration_set_complete(scene_configuration_node,
                                RESOLVER_GET_RULE,
                                FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS);

  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(scene_id_node));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(duration_node));
  TEST_ASSERT_FALSE(attribute_store_is_value_matched(scene_id_node));
  TEST_ASSERT_FALSE(attribute_store_is_value_matched(duration_node));

  // Reset, try with a Supervision Working
  attribute_store_undefine_reported(scene_id_node);
  attribute_store_undefine_reported(duration_node);

  on_configuration_set_complete(scene_configuration_node,
                                RESOLVER_SET_RULE,
                                FRAME_SENT_EVENT_OK_SUPERVISION_WORKING);

  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(scene_id_node));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(duration_node));
  TEST_ASSERT_FALSE(attribute_store_is_value_matched(scene_id_node));
  TEST_ASSERT_FALSE(attribute_store_is_value_matched(duration_node));

  // Reset, try with a Supervision Fail
  attribute_store_undefine_reported(scene_id_node);
  attribute_store_undefine_reported(duration_node);

  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(scene_id_node));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(duration_node));
  TEST_ASSERT_TRUE(
    attribute_store_is_reported_defined(scene_configuration_node));

  on_configuration_set_complete(scene_configuration_node,
                                RESOLVER_SET_RULE,
                                FRAME_SENT_EVENT_OK_SUPERVISION_FAIL);

  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(scene_id_node));
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(duration_node));
  TEST_ASSERT_FALSE(
    attribute_store_is_reported_defined(scene_configuration_node));
}

void test_zwave_command_class_parse_incoming_configuration_report_too_short()
{
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_SCENE_CONTROLLER_CONF, SCENE_CONTROLLER_CONF_REPORT};
  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&connection_info,
                                            incoming_frame,
                                            sizeof(incoming_frame)));
}

void test_zwave_command_class_incoming_command_too_short()
{
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_SCENE_CONTROLLER_CONF, SCENE_CONTROLLER_CONF_REPORT};
  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&connection_info,
                                            incoming_frame,
                                            sizeof(incoming_frame)));
}

void test_zwave_command_class_parse_incoming_configuration_report_happy_case()
{
  const association_group_id_t group_id         = 49;
  const local_scene_id_t scene_id               = 4;
  const local_scene_dimming_duration_t duration = 5;

  attribute_store_node_t group_id_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                              &group_id,
                              sizeof(group_id));

  attribute_store_node_t scene_configuration_node
    = attribute_store_add_node(ATTRIBUTE(CONTROLLER_CONFIGURATION),
                               group_id_node);

  attribute_store_node_t scene_id_node
    = attribute_store_add_node(ATTRIBUTE(CONTROLLER_SETTING_ID),
                               scene_configuration_node);
  attribute_store_node_t duration_node
    = attribute_store_add_node(ATTRIBUTE(CONTROLLER_SETTING_DIMMING_DURATION),
                               scene_configuration_node);

  // Process the frame
  const uint8_t incoming_frame[] = {COMMAND_CLASS_SCENE_CONTROLLER_CONF,
                                    SCENE_CONTROLLER_CONF_REPORT,
                                    group_id,
                                    scene_id,
                                    duration};
  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&connection_info,
                                            incoming_frame,
                                            sizeof(incoming_frame)));

  // Check the attribute store.
  local_scene_id_t saved_scene_id = 0;
  attribute_store_get_reported(scene_id_node,
                               &saved_scene_id,
                               sizeof(saved_scene_id));
  TEST_ASSERT_EQUAL(scene_id, saved_scene_id);
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(scene_id_node));

  local_scene_dimming_duration_t saved_duration = 0;
  attribute_store_get_reported(duration_node,
                               &saved_duration,
                               sizeof(saved_duration));
  TEST_ASSERT_EQUAL(duration, saved_duration);
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(duration_node));

  command_status_values_t state = NEEDS_ONE_COMMAND;
  attribute_store_get_reported(scene_configuration_node, &state, sizeof(state));
  TEST_ASSERT_EQUAL(FINAL_STATE, state);
  attribute_store_get_desired(scene_configuration_node, &state, sizeof(state));
  TEST_ASSERT_EQUAL(FINAL_STATE, state);
}

void test_zwave_command_class_unknown_command()
{
  const uint8_t incoming_frame[] = {COMMAND_CLASS_SCENE_CONTROLLER_CONF, 0xFE};
  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&connection_info,
                                            incoming_frame,
                                            sizeof(incoming_frame)));
}