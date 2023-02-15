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
#include "zwave_command_class_scene_actuator_configuration_control.h"
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
#include "dotdot_mqtt_mock.h"
#include "attribute_resolver_mock.h"
#include "zpc_attribute_resolver_mock.h"
#include "zwave_command_handler_mock.h"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_SCENE_ACTUATOR_##type

// Static variables
static attribute_resolver_function_t scene_actuator_configuration_set = NULL;
static attribute_resolver_function_t scene_actuator_configuration_get = NULL;
static zwave_command_handler_t handler                                = {};
static zwave_controller_connection_info_t connection_info             = {};
static zpc_resolver_event_notification_function_t on_configuration_set_complete
  = NULL;

static uic_mqtt_dotdot_scenes_view_scene_callback_t view_scene_command   = NULL;
static uic_mqtt_dotdot_scenes_store_scene_callback_t store_scene_command = NULL;

static uint8_t received_frame[255]  = {};
static uint16_t received_frame_size = 0;

// Stub functions
static sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  if (node_type == ATTRIBUTE(SETTING_DIMMING_DURATION)
      || node_type == ATTRIBUTE(SETTING_LEVEL)) {
    TEST_ASSERT_NOT_NULL(get_func);
    TEST_ASSERT_NULL(set_func);
    scene_actuator_configuration_get = get_func;
  } else if (node_type == ATTRIBUTE(ID)) {
    TEST_ASSERT_NOT_NULL(set_func);
    TEST_ASSERT_NULL(get_func);
    scene_actuator_configuration_set = set_func;
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
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SCENE_ACTUATOR_CONF, handler.command_class);
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
  TEST_ASSERT_EQUAL(ATTRIBUTE(ID), type);
  on_configuration_set_complete = function;
  return SL_STATUS_OK;
}

static void uic_mqtt_dotdot_scenes_view_scene_callback_set_stub(
  const uic_mqtt_dotdot_scenes_view_scene_callback_t callback,
  int cmock_num_calls)
{
  view_scene_command = callback;
}

void uic_mqtt_dotdot_scenes_store_scene_callback_set_stub(
  const uic_mqtt_dotdot_scenes_store_scene_callback_t callback,
  int cmock_num_calls)
{
  store_scene_command = callback;
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

static void zwave_command_class_scene_actuator_configuration_init_verification()
{
  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  // Resolution send data listener registration
  register_send_event_handler_Stub(&register_send_event_handler_stub);

  uic_mqtt_dotdot_scenes_view_scene_callback_set_Stub(
    &uic_mqtt_dotdot_scenes_view_scene_callback_set_stub);

  uic_mqtt_dotdot_scenes_store_scene_callback_set_Stub(
    &uic_mqtt_dotdot_scenes_store_scene_callback_set_stub);

  // Call init
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_command_class_scene_actuator_configuration_control_init());
}

/// Called before each and every test
void setUp()
{
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_set_home_id(home_id);

  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size              = 0;
  scene_actuator_configuration_set = NULL;
  scene_actuator_configuration_get = NULL;
  on_configuration_set_complete    = NULL;
  memset(&handler, 0, sizeof(zwave_command_handler_t));

  zwave_command_class_scene_actuator_configuration_init_verification();

  // Prepare a default connection info with the supporting node
  memset(&connection_info, 0, sizeof(zwave_controller_connection_info_t));
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));
}

void test_zwave_command_class_scene_actuator_configuration_set()
{
  TEST_ASSERT_NOT_NULL(scene_actuator_configuration_set);

  // No attributes created, do not provide a frame:
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    scene_actuator_configuration_set(0, received_frame, &received_frame_size));

  attribute_store_node_t local_scenes_node
    = attribute_store_create_child_if_missing(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_SCENE_LOCAL_SCENES);
  attribute_store_node_t scene_configuration_node
    = attribute_store_create_child_if_missing(local_scenes_node,
                                              ATTRIBUTE(CONFIGURATION));

  local_scene_id_t scene_id = 4;
  attribute_store_node_t scene_id_node
    = attribute_store_emplace(scene_configuration_node,
                              ATTRIBUTE(ID),
                              &scene_id,
                              sizeof(scene_id));
  local_scene_dimming_duration_t duration = 5;
  attribute_store_set_child_desired(scene_id_node,
                                    ATTRIBUTE(SETTING_DIMMING_DURATION),
                                    &duration,
                                    sizeof(duration));

  local_scene_level_t level = 45;
  attribute_store_set_child_desired(scene_id_node,
                                    ATTRIBUTE(SETTING_LEVEL),
                                    &level,
                                    sizeof(level));

  local_scene_override_t override = 1;
  attribute_store_set_child_desired(scene_id_node,
                                    ATTRIBUTE(SETTING_OVERRIDE),
                                    &override,
                                    sizeof(override));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    scene_actuator_configuration_set(scene_id_node,
                                                     received_frame,
                                                     &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_SCENE_ACTUATOR_CONF,
                                    SCENE_ACTUATOR_CONF_SET,
                                    scene_id,
                                    duration,
                                    override ? 0x80 : 0,
                                    level};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_scene_actuator_configuration_get()
{
  TEST_ASSERT_NOT_NULL(scene_actuator_configuration_get);

  // No attributes created, we will be unhappy.
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    scene_actuator_configuration_get(0, received_frame, &received_frame_size));

  attribute_store_node_t local_scenes_node
    = attribute_store_create_child_if_missing(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_SCENE_LOCAL_SCENES);
  attribute_store_node_t scene_configuration_node
    = attribute_store_create_child_if_missing(local_scenes_node,
                                              ATTRIBUTE(CONFIGURATION));

  local_scene_id_t scene_id = 4;
  attribute_store_node_t scene_id_node
    = attribute_store_emplace(scene_configuration_node,
                              ATTRIBUTE(ID),
                              &scene_id,
                              sizeof(scene_id));

  attribute_store_node_t level_node
    = attribute_store_add_node(ATTRIBUTE(SETTING_LEVEL), scene_id_node);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    scene_actuator_configuration_get(level_node,
                                                     received_frame,
                                                     &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_SCENE_ACTUATOR_CONF, SCENE_ACTUATOR_CONF_GET, scene_id};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_parse_incoming_configuration_report_happy_case()
{
  const local_scene_id_t scene_id               = 4;
  const local_scene_dimming_duration_t duration = 5;
  const local_scene_level_t level               = 102;

  // Process the frame
  const uint8_t incoming_frame[] = {COMMAND_CLASS_SCENE_ACTUATOR_CONF,
                                    SCENE_ACTUATOR_CONF_REPORT,
                                    scene_id,
                                    level,
                                    duration};
  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&connection_info,
                                            incoming_frame,
                                            sizeof(incoming_frame)));

  // Check the attribute store.
  attribute_store_node_t local_scenes_node
    = attribute_store_get_first_child_by_type(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_SCENE_LOCAL_SCENES);
  attribute_store_node_t actuator_configuration_node
    = attribute_store_get_first_child_by_type(local_scenes_node,
                                              ATTRIBUTE(CONFIGURATION));
  attribute_store_node_t scene_id_node
    = attribute_store_get_node_child_by_value(actuator_configuration_node,
                                              ATTRIBUTE(ID),
                                              REPORTED_ATTRIBUTE,
                                              &scene_id,
                                              sizeof(scene_id),
                                              0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, scene_id_node);

  attribute_store_node_t duration_node
    = attribute_store_get_first_child_by_type(
      scene_id_node,
      ATTRIBUTE(SETTING_DIMMING_DURATION));
  local_scene_dimming_duration_t saved_duration = 0;
  attribute_store_get_reported(duration_node,
                               &saved_duration,
                               sizeof(saved_duration));
  TEST_ASSERT_EQUAL(duration, saved_duration);
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(duration_node));

  attribute_store_node_t level_node
    = attribute_store_get_first_child_by_type(scene_id_node,
                                              ATTRIBUTE(SETTING_LEVEL));
  local_scene_level_t saved_level = 0;
  attribute_store_get_reported(level_node, &saved_level, sizeof(saved_level));
  TEST_ASSERT_EQUAL(level, saved_level);
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(level_node));

  attribute_store_node_t override_node
    = attribute_store_get_first_child_by_type(scene_id_node,
                                              ATTRIBUTE(SETTING_OVERRIDE));
  local_scene_override_t saved_override = 1;
  attribute_store_get_reported(override_node,
                               &saved_override,
                               sizeof(saved_override));
  TEST_ASSERT_EQUAL(0, saved_override);
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(override_node));
}

void test_zwave_command_class_parse_incoming_configuration_report_default_settings()
{
  const local_scene_id_t scene_id = 150;

  // Process the frame
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_SCENE_ACTUATOR_CONF, SCENE_ACTUATOR_CONF_REPORT, scene_id};
  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&connection_info,
                                            incoming_frame,
                                            sizeof(incoming_frame)));

  // Check the attribute store.
  attribute_store_node_t local_scenes_node
    = attribute_store_get_first_child_by_type(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_SCENE_LOCAL_SCENES);
  attribute_store_node_t actuator_configuration_node
    = attribute_store_get_first_child_by_type(local_scenes_node,
                                              ATTRIBUTE(CONFIGURATION));
  attribute_store_node_t scene_id_node
    = attribute_store_get_node_child_by_value(actuator_configuration_node,
                                              ATTRIBUTE(ID),
                                              REPORTED_ATTRIBUTE,
                                              &scene_id,
                                              sizeof(scene_id),
                                              0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, scene_id_node);

  attribute_store_node_t duration_node
    = attribute_store_get_first_child_by_type(
      scene_id_node,
      ATTRIBUTE(SETTING_DIMMING_DURATION));
  local_scene_dimming_duration_t saved_duration = 0;
  attribute_store_get_reported(duration_node,
                               &saved_duration,
                               sizeof(saved_duration));
  TEST_ASSERT_EQUAL(0, saved_duration);
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(duration_node));

  attribute_store_node_t level_node
    = attribute_store_get_first_child_by_type(scene_id_node,
                                              ATTRIBUTE(SETTING_LEVEL));
  local_scene_level_t saved_level = 0;
  attribute_store_get_reported(level_node, &saved_level, sizeof(saved_level));
  TEST_ASSERT_EQUAL(0, saved_level);
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(level_node));

  attribute_store_node_t override_node
    = attribute_store_get_first_child_by_type(scene_id_node,
                                              ATTRIBUTE(SETTING_OVERRIDE));
  local_scene_override_t saved_override = 1;
  attribute_store_get_reported(override_node,
                               &saved_override,
                               sizeof(saved_override));
  TEST_ASSERT_EQUAL(0, saved_override);
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(override_node));
}

void test_zwave_command_class_parse_incoming_configuration_report_invalid_scene_id()
{
  const local_scene_dimming_duration_t duration = 5;
  const local_scene_level_t level               = 102;

  // Process the frame
  const uint8_t incoming_frame[] = {COMMAND_CLASS_SCENE_ACTUATOR_CONF,
                                    SCENE_ACTUATOR_CONF_REPORT,
                                    0,
                                    level,
                                    duration};
  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&connection_info,
                                            incoming_frame,
                                            sizeof(incoming_frame)));

  // Check the attribute store.
  attribute_store_node_t local_scenes_node
    = attribute_store_get_first_child_by_type(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_SCENE_LOCAL_SCENES);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, local_scenes_node);
}

void test_on_version_attribute_update()
{
  // Create a version node under the endpoint
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE(CONF_VERSION), endpoint_id_node);

  // Nothing should happen yet
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 3;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  attribute_store_node_t local_scenes_node
    = attribute_store_get_first_child_by_type(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_SCENE_LOCAL_SCENES);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, local_scenes_node);
  attribute_store_node_t actuator_configuration_node
    = attribute_store_get_first_child_by_type(local_scenes_node,
                                              ATTRIBUTE(CONFIGURATION));
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                        actuator_configuration_node);
}

void test_on_set_send_data_complete()
{
  TEST_ASSERT_NOT_NULL(on_configuration_set_complete);

  // Create the necessary attributes
  attribute_store_node_t local_scenes_node
    = attribute_store_create_child_if_missing(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_SCENE_LOCAL_SCENES);

  attribute_store_node_t actuator_configuration_node
    = attribute_store_create_child_if_missing(local_scenes_node,
                                              ATTRIBUTE(CONFIGURATION));
  local_scene_id_t scene_id = 34;
  attribute_store_node_t scene_id_node
    = attribute_store_emplace(actuator_configuration_node,
                              ATTRIBUTE(ID),
                              &scene_id,
                              sizeof(scene_id));
  scene_id = 0;
  attribute_store_set_desired(scene_id_node, &scene_id, sizeof(scene_id));

  // Just an error message here when we invoke for a Get Rule:
  on_configuration_set_complete(scene_id_node,
                                RESOLVER_GET_RULE,
                                FRAME_SENT_EVENT_OK_SUPERVISION_WORKING);

  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(scene_id_node));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(scene_id_node));
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(scene_id_node));

  // Set rule with Working, it will create the children but
  // don't do much with them.
  on_configuration_set_complete(scene_id_node,
                                RESOLVER_SET_RULE,
                                FRAME_SENT_EVENT_OK_SUPERVISION_WORKING);

  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(scene_id_node));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(scene_id_node));
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(scene_id_node));

  // Set some mismatched values
  attribute_store_node_t level_node
    = attribute_store_get_first_child_by_type(scene_id_node,
                                              ATTRIBUTE(SETTING_LEVEL));
  local_scene_level_t level = 102;
  attribute_store_set_reported(level_node, &level, sizeof(level));
  level = 150;
  attribute_store_set_desired(level_node, &level, sizeof(level));

  attribute_store_node_t duration_node
    = attribute_store_get_first_child_by_type(
      scene_id_node,
      ATTRIBUTE(SETTING_DIMMING_DURATION));
  local_scene_dimming_duration_t duration = 20;
  attribute_store_set_reported(duration_node, &duration, sizeof(duration));
  duration = 21;
  attribute_store_set_desired(duration_node, &duration, sizeof(duration));

  attribute_store_node_t override_node
    = attribute_store_get_first_child_by_type(scene_id_node,
                                              ATTRIBUTE(SETTING_OVERRIDE));
  local_scene_override_t override = 1;
  attribute_store_set_reported(override_node, &override, sizeof(override));

  // Set rule with Success, it will align all reported/desired
  on_configuration_set_complete(scene_id_node,
                                RESOLVER_SET_RULE,
                                FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS);

  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(scene_id_node));
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(scene_id_node));
  TEST_ASSERT_TRUE(attribute_store_is_value_matched(level_node));
  TEST_ASSERT_TRUE(attribute_store_is_value_matched(duration_node));
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(override_node));

  // Set rule with other than Success, we undefine everything
  on_configuration_set_complete(scene_id_node,
                                RESOLVER_SET_RULE,
                                FRAME_SENT_EVENT_OK_SUPERVISION_NO_SUPPORT);

  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(scene_id_node));
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(scene_id_node));
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(level_node));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(level_node));
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(duration_node));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(duration_node));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(override_node));

  // Try again, when the override desired was set to 0
  override = 0;
  attribute_store_set_desired(override_node, &override, sizeof(override));
  level = 1;
  attribute_store_set_reported(level_node, &level, sizeof(level));
  level = 2;
  attribute_store_set_desired(level_node, &level, sizeof(level));
  duration = 2;
  attribute_store_set_reported(duration_node, &duration, sizeof(duration));
  duration = 3;
  attribute_store_set_desired(duration_node, &duration, sizeof(duration));

  // Set rule with Success, everything will get undefined due to override desired = 1.
  on_configuration_set_complete(scene_id_node,
                                RESOLVER_SET_RULE,
                                FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS);

  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(scene_id_node));
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(scene_id_node));
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(level_node));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(level_node));
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(duration_node));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(duration_node));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(override_node));
}

void test_zwave_command_class_incoming_no_command()
{
  const uint8_t incoming_frame[] = {COMMAND_CLASS_SCENE_ACTUATOR_CONF};
  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&connection_info,
                                            incoming_frame,
                                            sizeof(incoming_frame)));
}

void test_zwave_command_class_incoming_command_too_short()
{
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_SCENE_ACTUATOR_CONF, SCENE_ACTUATOR_CONF_REPORT};
  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&connection_info,
                                            incoming_frame,
                                            sizeof(incoming_frame)));
}

void test_zwave_command_class_unknown_command()
{
  const uint8_t incoming_frame[] = {COMMAND_CLASS_SCENE_ACTUATOR_CONF, 0xFE};
  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&connection_info,
                                            incoming_frame,
                                            sizeof(incoming_frame)));
}

void test_view_scene_command()
{
  TEST_ASSERT_NOT_NULL(view_scene_command);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    view_scene_command(supporting_node_unid,
                       endpoint_id,
                       UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                       0,
                       0));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    view_scene_command(supporting_node_unid,
                                       endpoint_id,
                                       UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                       0,
                                       0));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    view_scene_command(supporting_node_unid,
                                       endpoint_id,
                                       UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                       255,
                                       0));
}

void test_store_scene_command()
{
  TEST_ASSERT_NOT_NULL(view_scene_command);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    store_scene_command(supporting_node_unid,
                        endpoint_id,
                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                        0,
                        0));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    store_scene_command(supporting_node_unid,
                                        endpoint_id,
                                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                        0,
                                        0));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    store_scene_command(supporting_node_unid,
                                        endpoint_id,
                                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                        255,
                                        0));
}