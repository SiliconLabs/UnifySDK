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
#include "zwave_command_class_scene_activation_control.h"
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

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "attribute_resolver_mock.h"
#include "zpc_attribute_resolver_mock.h"
#include "zwave_command_handler_mock.h"
#include "dotdot_mqtt_mock.h"
#include "dotdot_mqtt_generated_commands_mock.h"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_SCENE_##type

// Static variables
static attribute_resolver_function_t scene_activation_set = NULL;
static zwave_command_handler_t scene_activation_handler   = {};
static zwave_controller_connection_info_t connection_info = {};
static zpc_resolver_event_notification_function_t on_activation_set_complete
  = NULL;
static uic_mqtt_dotdot_scenes_recall_scene_callback_t recall_scene_command
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
  if (node_type == ATTRIBUTE(ACTIVATION_ACTIVE_SCENE)) {
    TEST_ASSERT_NULL(get_func);
    TEST_ASSERT_NOT_NULL(set_func);
    scene_activation_set = set_func;
  } else {
    TEST_FAIL_MESSAGE("Attribute rule registration on a wrong type");
  }
  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  scene_activation_handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME,
                    scene_activation_handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SCENE_ACTIVATION,
                    scene_activation_handler.command_class);
  TEST_ASSERT_EQUAL(1, scene_activation_handler.version);
  TEST_ASSERT_NOT_NULL(scene_activation_handler.control_handler);
  TEST_ASSERT_NULL(scene_activation_handler.support_handler);
  TEST_ASSERT_FALSE(scene_activation_handler.manual_security_validation);

  return SL_STATUS_OK;
}

static sl_status_t register_send_event_handler_stub(
  attribute_store_type_t type,
  zpc_resolver_event_notification_function_t function,
  int cmock_num_calls)
{
  TEST_ASSERT_EQUAL(ATTRIBUTE(ACTIVATION_ACTIVE_SCENE), type);
  on_activation_set_complete = function;
  return SL_STATUS_OK;
}

void uic_mqtt_dotdot_scenes_recall_scene_callback_set_stub(
  const uic_mqtt_dotdot_scenes_recall_scene_callback_t callback,
  int cmock_num_calls)
{
  recall_scene_command = callback;
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

static void zwave_command_class_scene_activation_init_verification()
{
  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  // Resolution send data listener registration
  register_send_event_handler_Stub(&register_send_event_handler_stub);

  uic_mqtt_dotdot_scenes_recall_scene_callback_set_Stub(
    &uic_mqtt_dotdot_scenes_recall_scene_callback_set_stub);

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_scene_activation_control_init());
}

/// Called before each and every test
void setUp()
{
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_set_home_id(home_id);

  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size        = 0;
  scene_activation_set       = NULL;
  on_activation_set_complete = NULL;
  memset(&scene_activation_handler, 0, sizeof(zwave_command_handler_t));

  zwave_command_class_scene_activation_init_verification();

  // Prepare a default connection info with the supporting node
  memset(&connection_info, 0, sizeof(zwave_controller_connection_info_t));
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));
}

void test_zwave_command_class_scene_activation_set_no_attributes()
{
  // Ask for a Activation Set Command, should default since no attribute are created
  TEST_ASSERT_NOT_NULL(scene_activation_set);
  scene_activation_set(0, received_frame, &received_frame_size);
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_SCENE_ACTIVATION, SCENE_ACTIVATION_SET, 1, 0};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_zwave_command_class_scene_activation_set_with_attributes_and_send_data_complete()
{
  // Create a version node under the endpoint
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE(ACTIVATION_VERSION), endpoint_id_node);

  zwave_cc_version_t version = 3;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  attribute_store_node_t local_scenes_node
    = attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(LOCAL_SCENES));
  attribute_store_node_t active_scene_node
    = attribute_store_get_first_child_by_type(
      local_scenes_node,
      ATTRIBUTE(ACTIVATION_ACTIVE_SCENE));
  attribute_store_node_t dimming_duration_node
    = attribute_store_get_first_child_by_type(
      active_scene_node,
      ATTRIBUTE(ACTIVATION_SETTING_DIMMING_DURATION));

  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, local_scenes_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, active_scene_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, dimming_duration_node);

  // Set desired values:
  const local_scene_id_t expected_scene = 24;
  attribute_store_set_desired(active_scene_node,
                              &expected_scene,
                              sizeof(expected_scene));
  const local_scene_dimming_duration_t expected_duration = 34;
  attribute_store_set_desired(dimming_duration_node,
                              &expected_duration,
                              sizeof(expected_duration));

  // Ask for a Activation Set Command
  TEST_ASSERT_NOT_NULL(scene_activation_set);
  scene_activation_set(active_scene_node, received_frame, &received_frame_size);
  const uint8_t expected_frame[] = {COMMAND_CLASS_SCENE_ACTIVATION,
                                    SCENE_ACTIVATION_SET,
                                    expected_scene,
                                    expected_duration};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);

  TEST_ASSERT_FALSE(attribute_store_is_value_matched(active_scene_node));
  TEST_ASSERT_FALSE(attribute_store_is_value_matched(dimming_duration_node));

  // Now pretend that the send data is completed.
  TEST_ASSERT_NOT_NULL(on_activation_set_complete);

  // nothing happens here, Get Rule should not exist
  on_activation_set_complete(active_scene_node, RESOLVER_GET_RULE, 0);

  TEST_ASSERT_FALSE(attribute_store_is_value_matched(active_scene_node));
  TEST_ASSERT_FALSE(attribute_store_is_value_matched(dimming_duration_node));

  // nothing happens on supervision WORKING
  attribute_resolver_resume_node_resolution_Ignore();
  on_activation_set_complete(active_scene_node,
                             RESOLVER_SET_RULE,
                             FRAME_SENT_EVENT_OK_SUPERVISION_WORKING);

  TEST_ASSERT_FALSE(attribute_store_is_value_matched(active_scene_node));
  TEST_ASSERT_FALSE(attribute_store_is_value_matched(dimming_duration_node));

  // success or anythin that does not indicate an error will be assumed to be successful.
  on_activation_set_complete(active_scene_node,
                             RESOLVER_SET_RULE,
                             FRAME_SENT_EVENT_OK_SUPERVISION_SUCCESS);

  TEST_ASSERT_TRUE(attribute_store_is_value_matched(active_scene_node));
  TEST_ASSERT_TRUE(attribute_store_is_value_matched(dimming_duration_node));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(active_scene_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(dimming_duration_node, DESIRED_ATTRIBUTE));

  // When an error happens, we roll-back the desired.
  on_activation_set_complete(active_scene_node,
                             RESOLVER_SET_RULE,
                             FRAME_SENT_EVENT_OK_SUPERVISION_FAIL);
  TEST_ASSERT_FALSE(attribute_store_is_value_matched(active_scene_node));
  TEST_ASSERT_TRUE(attribute_store_is_value_matched(dimming_duration_node));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(active_scene_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(dimming_duration_node, DESIRED_ATTRIBUTE));
}

void test_zwave_command_class_incoming_scene_activation_set_command()
{
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_SCENE_ACTIVATION, SCENE_ACTIVATION_SET, 2, 3};

  uic_mqtt_dotdot_scenes_command_recall_scene_fields_t expected_fields = {};
  expected_fields.sceneid         = incoming_frame[2];
  expected_fields.transition_time = incoming_frame[3];
  uic_mqtt_dotdot_scenes_publish_generated_recall_scene_command_Expect(
    NULL,
    connection_info.remote.endpoint_id,
    &expected_fields);
  uic_mqtt_dotdot_scenes_publish_generated_recall_scene_command_IgnoreArg_unid();

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    scene_activation_handler.control_handler(&connection_info,
                                             incoming_frame,
                                             sizeof(incoming_frame)));
}

void test_zwave_command_class_incoming_scene_activation_set_command_duration_minutes()
{
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_SCENE_ACTIVATION, SCENE_ACTIVATION_SET, 2, 0x81};

  uic_mqtt_dotdot_scenes_command_recall_scene_fields_t expected_fields = {};
  expected_fields.sceneid         = incoming_frame[2];
  expected_fields.transition_time = incoming_frame[3] * 60;
  uic_mqtt_dotdot_scenes_publish_generated_recall_scene_command_Expect(
    NULL,
    connection_info.remote.endpoint_id,
    &expected_fields);
  uic_mqtt_dotdot_scenes_publish_generated_recall_scene_command_IgnoreArg_unid();

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    scene_activation_handler.control_handler(&connection_info,
                                             incoming_frame,
                                             sizeof(incoming_frame)));
}

void test_zwave_command_class_incoming_scene_activation_set_command_duration_ff()
{
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_SCENE_ACTIVATION, SCENE_ACTIVATION_SET, 45, 0xFF};

  uic_mqtt_dotdot_scenes_command_recall_scene_fields_t expected_fields = {};
  expected_fields.sceneid         = incoming_frame[2];
  expected_fields.transition_time = 0;
  uic_mqtt_dotdot_scenes_publish_generated_recall_scene_command_Expect(
    NULL,
    connection_info.remote.endpoint_id,
    &expected_fields);
  uic_mqtt_dotdot_scenes_publish_generated_recall_scene_command_IgnoreArg_unid();

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    scene_activation_handler.control_handler(&connection_info,
                                             incoming_frame,
                                             sizeof(incoming_frame)));
}

void test_zwave_command_class_incoming_scene_activation_set_command_default_fields()
{
  const uint8_t incoming_frame[]
    = {COMMAND_CLASS_SCENE_ACTIVATION, SCENE_ACTIVATION_SET};

  uic_mqtt_dotdot_scenes_command_recall_scene_fields_t expected_fields = {};
  uic_mqtt_dotdot_scenes_publish_generated_recall_scene_command_Expect(
    NULL,
    connection_info.remote.endpoint_id,
    &expected_fields);
  uic_mqtt_dotdot_scenes_publish_generated_recall_scene_command_IgnoreArg_unid();

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    scene_activation_handler.control_handler(&connection_info,
                                             incoming_frame,
                                             sizeof(incoming_frame)));
}

void test_zwave_command_class_incoming_command_too_short()
{
  const uint8_t incoming_frame[] = {COMMAND_CLASS_SCENE_ACTIVATION};
  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    scene_activation_handler.control_handler(&connection_info,
                                             incoming_frame,
                                             sizeof(incoming_frame)));
}

void test_zwave_command_class_unknown_command()
{
  const uint8_t incoming_frame[] = {COMMAND_CLASS_SCENE_ACTIVATION, 0xFE};
  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    scene_activation_handler.control_handler(&connection_info,
                                             incoming_frame,
                                             sizeof(incoming_frame)));
}

void test_recall_scene_command()
{
  TEST_ASSERT_NOT_NULL(recall_scene_command);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    recall_scene_command(supporting_node_unid,
                         endpoint_id,
                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                         0,
                         0,
                         0));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    recall_scene_command(supporting_node_unid,
                                         endpoint_id,
                                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                         0,
                                         255,
                                         0));

  attribute_resolver_pause_node_resolution_Ignore();
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    recall_scene_command(supporting_node_unid,
                                         endpoint_id,
                                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                         0,
                                         0,
                                         0));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    recall_scene_command(supporting_node_unid,
                                         endpoint_id,
                                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                         0,
                                         2,
                                         0x81));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    recall_scene_command(supporting_node_unid,
                                         endpoint_id,
                                         UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                         2,
                                         2,
                                         1));
}