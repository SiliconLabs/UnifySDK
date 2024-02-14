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
#include "zwave_command_class_notification.h"
#include "zwave_command_class_test_helper.h"

// Generic includes
#include <string.h>  // for memset

// Includes from other components
#include "attribute_store_defined_attribute_types.h"
#include "attribute_store_mock.h"
#include "zpc_attribute_store_network_helper_mock.h"
#include "attribute_store_helper_mock.h"
#include "attribute_resolver_mock.h"

#include "zwave_controller_utils_mock.h"
#include "zwave_utils_mock.h"
#include "zwave_command_handler_mock.h"
#include "zwave_unid_mock.h"
#include "zwave_controller_connection_info.h"

#include "ZW_classcmd.h"

// Test includes
#include "unity.h"

// Static variable, testing
static zwave_controller_connection_info_t test_connection_info = {};

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

static attribute_resolver_function_t supported_notification_types_callback;
static attribute_resolver_function_t
  supported_notification_state_or_events_callback;
static attribute_resolver_function_t notification_state_or_event_callback;
static sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  if (ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_SUPPORTED_NOTIFICATION_TYPES
      == node_type) {
    supported_notification_types_callback = get_func;
  } else if (ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_EVENT == node_type) {
    notification_state_or_event_callback = get_func;
  } else if (ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_SUPPORTED_STATES_OR_EVENTS
             == node_type) {
    supported_notification_state_or_events_callback = get_func;
  } else {
    char tmp[100];
    snprintf(tmp,
             sizeof(tmp),
             "Received unexpected attribute_resolver_register_rule %x",
             node_type);
    TEST_ASSERT_TRUE_MESSAGE(false, tmp);
  }
  return SL_STATUS_OK;
}

static attribute_store_node_changed_callback_t notification_version_callback;
static sl_status_t attribute_store_register_callback_by_type_stub(
  attribute_store_node_changed_callback_t callback_function,
  attribute_store_type_t type,
  int cmock_num_calls)
{
  if (ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_VERSION == type) {
    notification_version_callback = callback_function;
  } else {
    TEST_ASSERT_TRUE_MESSAGE(
      false,
      "Received unexpected attribute_store_register_callback_by_type");
  }
  return SL_STATUS_OK;
}
static zwave_command_handler_t command_handler;
static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  command_handler = new_command_class_handler;
  return SL_STATUS_OK;
}
static bool attribute_store_node_exists_stub(attribute_store_node_t node,
                                             int cmock_num_calls)
{
  return node != ATTRIBUTE_STORE_INVALID_NODE;
}

static void init_notification_cc()
{
  attribute_resolver_register_rule_Stub(attribute_resolver_register_rule_stub);
  attribute_store_register_callback_by_type_Stub(
    attribute_store_register_callback_by_type_stub);
  zwave_command_handler_register_handler_Stub(
    zwave_command_handler_register_handler_stub);

  attribute_resolver_register_rule_ExpectAndReturn(
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_SUPPORTED_NOTIFICATION_TYPES,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_resolver_register_rule_IgnoreArg_set_func();

  attribute_resolver_register_rule_ExpectAndReturn(
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_SUPPORTED_STATES_OR_EVENTS,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_resolver_register_rule_IgnoreArg_set_func();

  zwave_command_handler_register_handler_ExpectAndReturn(command_handler,
                                                         SL_STATUS_OK);
  zwave_command_handler_register_handler_IgnoreArg_new_command_class_handler();
  zwave_command_class_notification_init();
}

void setUp()
{
  attribute_store_node_exists_Stub(&attribute_store_node_exists_stub);
  init_notification_cc();
}

// Test notification report handler
void test_zwave_command_class_notification_handler_short_frame()
{
  // Test received frame size
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    command_handler.control_handler(NULL, NULL, 1));

  // Test command wrong command class
  uint8_t test_frame_data[] = {0x72, 0x01};
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    command_handler.control_handler(NULL,
                                                    test_frame_data,
                                                    sizeof(test_frame_data)));

  // Test with not supported command
  test_frame_data[0] = 0x71;
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    command_handler.control_handler(NULL,
                                                    test_frame_data,
                                                    sizeof(test_frame_data)));

  // Test notification report handler
  test_frame_data[0]                      = COMMAND_CLASS_NOTIFICATION_V4;
  test_frame_data[1]                      = NOTIFICATION_REPORT_V4;
  test_connection_info.remote.node_id     = 0x01;
  test_connection_info.remote.endpoint_id = 0x02;

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    command_handler.control_handler(&test_connection_info,
                                                    test_frame_data,
                                                    sizeof(test_frame_data)));
}
void test_zwave_command_class_notification_report()
{
  // Test a report for Home Security notification type
  uint8_t test_frame_data_2[]
    = {0x71, 0x05, 0x00, 0x00, 0x00, 0x00, 0x06, 0x16, 0x01, 0x00, 0x00};
  int32_t notification_event_value = 0x16;  //Door state
  int32_t notification_event_param_value
    = 0x00;  //0x00: Door/Window open in regular position
  unid_t test_unid                                     = "zw000001";
  attribute_store_node_t endpoint_node                 = 0x017;
  attribute_store_node_t notification_status_node      = 0x012;
  attribute_store_node_t notification_state_node       = 0x013;
  attribute_store_node_t notification_event_node       = 0x014;
  attribute_store_node_t notification_event_param_node = 0x015;
  zwave_unid_from_node_id_Expect(test_connection_info.remote.node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    test_connection_info.remote.endpoint_id,
    endpoint_node);

  attribute_store_get_node_child_by_value_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_TYPE,
    REPORTED_ATTRIBUTE,
    &test_frame_data_2[6],
    sizeof(uint8_t),
    0,
    notification_status_node);

  uint8_t state = NOTIFICATION_STATE_ACCESS_CONTROL_DOOR_STATE;
  attribute_store_get_node_child_by_value_ExpectAndReturn(
    notification_status_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_STATE,
    REPORTED_ATTRIBUTE,
    &state,
    sizeof(state),
    0,
    notification_state_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    notification_state_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_EVENT,
    0,
    notification_event_node);

  attribute_store_set_node_attribute_value_ExpectAndReturn(
    notification_event_node,
    REPORTED_ATTRIBUTE,
    (uint8_t *)&notification_event_value,
    sizeof(int32_t),
    SL_STATUS_OK);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    notification_event_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_EVENT_PARAMETERS,
    0,
    notification_event_param_node);

  attribute_store_set_node_attribute_value_ExpectAndReturn(
    notification_event_param_node,
    REPORTED_ATTRIBUTE,
    (uint8_t *)&notification_event_param_value,
    sizeof(int32_t),
    SL_STATUS_OK);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    command_handler.control_handler(&test_connection_info,
                                                    test_frame_data_2,
                                                    sizeof(test_frame_data_2)));
}

void test_zwave_command_class_notification_report_v1_alarm_type()
{
  // Test V1 Alarm Type Notification
  const uint8_t test_frame_v1_alarm[]              = {0x71, 0x05, 0x18, 0x01};
  unid_t test_unid_alarm                           = "zw000001";
  static attribute_store_node_t endpoint_node_test = 0x01;
  static attribute_store_node_t v1_alarm_type_node_test  = 0x02;
  static attribute_store_node_t v1_alarm_level_node_test = 0x03;
  test_connection_info.remote.node_id                    = 0x01;
  test_connection_info.remote.endpoint_id                = 0x00;

  zwave_unid_from_node_id_Expect(test_connection_info.remote.node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid_alarm,
                                                sizeof(unid_t));
  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid_alarm,
    test_connection_info.remote.endpoint_id,
    endpoint_node_test);

  attribute_store_get_node_child_by_value_ExpectAndReturn(
    endpoint_node_test,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_V1_ALARM_TYPE,
    REPORTED_ATTRIBUTE,
    &test_frame_v1_alarm[2],
    sizeof(uint8_t),
    0,
    ATTRIBUTE_STORE_INVALID_NODE);

  attribute_store_add_node_ExpectAndReturn(
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_V1_ALARM_TYPE,
    endpoint_node_test,
    v1_alarm_type_node_test);
  attribute_store_set_node_attribute_value_ExpectAndReturn(
    v1_alarm_type_node_test,
    REPORTED_ATTRIBUTE,
    &test_frame_v1_alarm[2],
    sizeof(uint8_t),
    SL_STATUS_OK);

  attribute_store_add_node_ExpectAndReturn(
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_V1_ALARM_LEVEL,
    v1_alarm_type_node_test,
    v1_alarm_level_node_test);
  attribute_store_set_node_attribute_value_ExpectAndReturn(
    v1_alarm_level_node_test,
    REPORTED_ATTRIBUTE,
    &test_frame_v1_alarm[3],
    sizeof(uint8_t),
    SL_STATUS_OK);

  attribute_store_get_node_child_by_value_ExpectAndReturn(
    endpoint_node_test,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_TYPE,
    REPORTED_ATTRIBUTE,
    NULL,
    sizeof(uint8_t),
    0,
    ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_get_node_child_by_value_IgnoreArg_value();

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    command_handler.control_handler(&test_connection_info,
                                    test_frame_v1_alarm,
                                    sizeof(test_frame_v1_alarm)));
}

void test_zwave_command_class_notification_report_idle_param()
{
  // Test a report for Home Security notification type
  uint8_t test_frame_data_2[]
    = {0x71, 0x05, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x01, 0x16, 0x00};
  int32_t notification_event_value       = 0;
  int32_t notification_event_param_value = 0x16;

  unid_t test_unid                                     = "zw000001";
  attribute_store_node_t endpoint_node                 = 0x017;
  attribute_store_node_t notification_status_node      = 0x012;
  attribute_store_node_t notification_state_node       = 0x013;
  attribute_store_node_t notification_event_node       = 0x014;
  attribute_store_node_t notification_event_param_node = 0x015;
  zwave_unid_from_node_id_Expect(test_connection_info.remote.node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    test_connection_info.remote.endpoint_id,
    endpoint_node);

  attribute_store_get_node_child_by_value_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_TYPE,
    REPORTED_ATTRIBUTE,
    &test_frame_data_2[6],
    sizeof(uint8_t),
    0,
    notification_status_node);

  const uint8_t states[] = {
    NOTIFICATION_STATE_ACCESS_CONTROL_DOOR_STATE,
  };
  for (unsigned int i = 0; i < sizeof(states); i++) {
    attribute_store_get_node_child_by_value_ExpectAndReturn(
      notification_status_node,
      ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_STATE,
      REPORTED_ATTRIBUTE,
      &states[i],
      sizeof(states[0]),
      0,
      notification_state_node);

    attribute_store_get_node_child_by_type_ExpectAndReturn(
      notification_state_node,
      ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_EVENT,
      0,
      notification_event_node);

    attribute_store_set_node_attribute_value_ExpectAndReturn(
      notification_event_node,
      REPORTED_ATTRIBUTE,
      (uint8_t *)&notification_event_value,
      sizeof(int32_t),
      SL_STATUS_OK);

    attribute_store_get_node_child_by_type_ExpectAndReturn(
      notification_event_node,
      ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_EVENT_PARAMETERS,
      0,
      notification_event_param_node);

    attribute_store_set_node_attribute_value_ExpectAndReturn(
      notification_event_param_node,
      REPORTED_ATTRIBUTE,
      (uint8_t *)&notification_event_param_value,
      sizeof(notification_event_param_value),
      SL_STATUS_OK);
  }

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    command_handler.control_handler(&test_connection_info,
                                                    test_frame_data_2,
                                                    sizeof(test_frame_data_2)));
}
void test_zwave_command_class_notification_report_idle_no_param()
{
  // Test a report for Home Security notification type
  uint8_t test_frame_data_2[]
    = {0x71, 0x05, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00};
  int32_t notification_event_value = 0;

  unid_t test_unid                                     = "zw000001";
  attribute_store_node_t endpoint_node                 = 0x017;
  attribute_store_node_t notification_status_node      = 0x012;
  attribute_store_node_t notification_state_node       = 0x013;
  attribute_store_node_t notification_event_node       = 0x014;
  attribute_store_node_t notification_event_param_node = 0x015;
  zwave_unid_from_node_id_Expect(test_connection_info.remote.node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    test_connection_info.remote.endpoint_id,
    endpoint_node);

  attribute_store_get_node_child_by_value_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_TYPE,
    REPORTED_ATTRIBUTE,
    &test_frame_data_2[6],
    sizeof(uint8_t),
    0,
    notification_status_node);

  const uint8_t states[]
    = {NOTIFICATION_STATE_ACCESS_CONTROL_LOCK_STATE,
       NOTIFICATION_STATE_ACCESS_CONTROL_KEYPAD_STATE,
       NOTIFICATION_STATE_ACCESS_CONTROL_DOOR_STATE,
       NOTIFICATION_STATE_ACCESS_CONTROL_DOOR_HANDLE_STATE,
       NOTIFICATION_STATE_ACCESS_CONTROL_BARRIER_UL_DISABLING_STATUS,
       NOTIFICATION_STATE_ACCESS_CONTROL_BARRIER_VACATION_MODE_STATUS,
       NOTIFICATION_STATE_ACCESS_CONTROL_BARRIER_SAFETY_BEARM_OBSTACLE_STATUS,
       NOTIFICATION_STATE_ACCESS_CONTROL_BARRIER_SENSOR_STATUS,
       NOTIFICATION_STATE_ACCESS_CONTROL_BARRIER_BATTERY_STATUS,
       NOTIFICATION_STATE_ACCESS_CONTROL_BARRIER_SHORTCIRCUIT_STATUS,
       NOTIFICATION_STATE_ACCESS_CONTROL_BARRIER_CONTROL_STATUS};
  for (unsigned int i = 0; i < sizeof(states); i++) {
    attribute_store_get_node_child_by_value_ExpectAndReturn(
      notification_status_node,
      ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_STATE,
      REPORTED_ATTRIBUTE,
      &states[i],
      sizeof(states[0]),
      0,
      notification_state_node);

    attribute_store_get_node_child_by_type_ExpectAndReturn(
      notification_state_node,
      ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_EVENT,
      0,
      notification_event_node);

    attribute_store_set_node_attribute_value_ExpectAndReturn(
      notification_event_node,
      REPORTED_ATTRIBUTE,
      (uint8_t *)&notification_event_value,
      sizeof(int32_t),
      SL_STATUS_OK);

    attribute_store_get_node_child_by_type_ExpectAndReturn(
      notification_event_node,
      ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_EVENT_PARAMETERS,
      0,
      notification_event_param_node);

    attribute_store_set_node_attribute_value_ExpectAndReturn(
      notification_event_param_node,
      REPORTED_ATTRIBUTE,
      NULL,
      0,
      SL_STATUS_OK);
  }

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    command_handler.control_handler(&test_connection_info,
                                                    test_frame_data_2,
                                                    sizeof(test_frame_data_2)));
}

// Test the supported notification types report handler
void helper_test_zwave_command_class_supported_notification_types_report(
  const unsigned int version)
{
  uint8_t notification_count  = 0;
  uint8_t test_frame_data_1[] = {
    COMMAND_CLASS_NOTIFICATION_V4,
    NOTIFICATION_SUPPORTED_REPORT_V4,
    0x01,
    0x80,  // NOTIFICATION_REPORT_HOME_SECURITY_V4
  };
  uint8_t test_frame_data_2[] = {
    COMMAND_CLASS_NOTIFICATION_V7,
    NOTIFICATION_SUPPORTED_REPORT_V7,
    0x02,
    0x16,  // NOTIFICATION_REPORT_SMOKE_V8 + NOTIFICATION_REPORT_CO_V8 + NOTIFICATION_REPORT_HEAT_V4
    0x18   // NOTIFICATION_REPORT_CLOCK_V4 + NOTIFICATION_REPORT_APPLIANCE_V4};
  };
  uint8_t test_frame_data_3[] = {
    COMMAND_CLASS_NOTIFICATION_V8,
    NOTIFICATION_SUPPORTED_REPORT_V8,
    0x03,
    0x04,  // NOTIFICATION_REPORT_CO_V4
    0x20,  // NOTIFICATION_REPORT_HOME_HEALTH_V4
    0x7F,  // NOTIFICATION_GET_WEATHER_ALARM_V8 + NOTIFICATION_GET_IRRIGATION_V8 +NOTIFICATION_GET_GAS_ALARM_V8 + NOTIFICATION_GET_PEST_CONTROL_V8 + NOTIFICATION_GET_LIGHT_SENSOR_V8 + NOTIFICATION_GET_WATER_QUALITY_MONITORING_V8 + NOTIFICATION_GET_HOME_MONITORING_V8
  };

  if (version < 5) {
    notification_count = 1;
  } else if (version < 8) {
    notification_count = 5;
  } else {
    notification_count = 9;
  }

  static unid_t test_unid                                        = "zw000001";
  static attribute_store_node_t endpoint_node                    = 0x011;
  static attribute_store_node_t supported_notification_type_node = 0x012;
  static attribute_store_node_t notification_type_node           = 0x013;

  zwave_unid_from_node_id_Expect(test_connection_info.remote.node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  zwave_node_get_command_class_version_ExpectAndReturn(
    COMMAND_CLASS_NOTIFICATION_V8,
    test_connection_info.remote.node_id,
    test_connection_info.remote.endpoint_id,
    version);

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    test_connection_info.remote.endpoint_id,
    endpoint_node);

  // Get and set the supported types
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_SUPPORTED_NOTIFICATION_TYPES,
    0,
    supported_notification_type_node);

  attribute_store_set_node_attribute_value_ExpectAndReturn(
    supported_notification_type_node,
    REPORTED_ATTRIBUTE,
    NULL,
    notification_count * sizeof(uint8_t),
    SL_STATUS_OK);
  attribute_store_set_node_attribute_value_IgnoreArg_value();

  for (int i = 0; i < notification_count; i++) {
    attribute_store_get_node_child_by_value_ExpectAndReturn(
      endpoint_node,
      ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_TYPE,
      REPORTED_ATTRIBUTE,
      NULL,
      sizeof(uint8_t),
      0,
      ATTRIBUTE_STORE_INVALID_NODE);
    attribute_store_get_node_child_by_value_IgnoreArg_value();

    attribute_store_add_node_ExpectAndReturn(
      ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_TYPE,
      endpoint_node,
      notification_type_node);

    attribute_store_set_node_attribute_value_ExpectAndReturn(
      notification_type_node,
      REPORTED_ATTRIBUTE,
      NULL,
      sizeof(uint8_t),
      SL_STATUS_OK);
    attribute_store_set_node_attribute_value_IgnoreArg_value();

    // Only add ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_SUPPORTED_STATES_OR_EVENTS
    // for version greater than 2
    if (version > 2) {
      attribute_store_add_node_ExpectAndReturn(
        ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_SUPPORTED_STATES_OR_EVENTS,
        notification_type_node,
        ATTRIBUTE_STORE_INVALID_NODE);
    }
  }

  if (version < 5) {
    TEST_ASSERT_EQUAL(
      SL_STATUS_OK,
      command_handler.control_handler(&test_connection_info,
                                      test_frame_data_1,
                                      sizeof(test_frame_data_1)));
  } else if (version < 8) {
    TEST_ASSERT_EQUAL(
      SL_STATUS_OK,
      command_handler.control_handler(&test_connection_info,
                                      test_frame_data_2,
                                      sizeof(test_frame_data_2)));
  } else {
    TEST_ASSERT_EQUAL(
      SL_STATUS_OK,
      command_handler.control_handler(&test_connection_info,
                                      test_frame_data_3,
                                      sizeof(test_frame_data_3)));
  }
}

#if 1
void test_zwave_command_class_supported_notification_types_report()
{
  for (size_t version = 0; version <= 8; version++) {
    helper_test_zwave_command_class_supported_notification_types_report(
      version);
  }
}
#endif

// This also test zwave_command_class_notification_types_attribute_update()
void test_zwave_command_class_notification_on_version_attribute_update_happy_case()
{
  attribute_store_node_t test_updated_node           = 0xf9a;
  attribute_store_node_t test_parent_node            = 0xde9e;
  attribute_store_node_t test_notification_type_node = 0xacb8b;

  is_zwave_command_class_filtered_for_root_device_verification(
    COMMAND_CLASS_NOTIFICATION_V3,
    test_updated_node);

  // The function will verify the type of the updated node:
  attribute_store_get_node_type_ExpectAndReturn(
    test_updated_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_VERSION);

  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    test_updated_node,
    NULL,
    NULL,
    SL_STATUS_OK);

  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_node_id(
    &test_connection_info.remote.node_id);
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
    &test_connection_info.remote.endpoint_id);

  zwave_node_get_command_class_version_ExpectAndReturn(
    COMMAND_CLASS_NOTIFICATION_V8,
    test_connection_info.remote.node_id,
    test_connection_info.remote.endpoint_id,
    8);

  // Then find out the parent node:
  attribute_store_get_node_parent_ExpectAndReturn(test_updated_node,
                                                  test_parent_node);

  // They must then try to find the ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_SUPPORTED_NOTIFICATION_TYPES
  // attribute under the parent, and create it if it does not exist.
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_parent_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_SUPPORTED_NOTIFICATION_TYPES,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);

  attribute_store_add_node_ExpectAndReturn(
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_SUPPORTED_NOTIFICATION_TYPES,
    test_parent_node,
    test_notification_type_node);

  // Make the call:
  notification_version_callback(test_updated_node, ATTRIBUTE_UPDATED);
}

void test_zwave_command_class_notification_on_version_attribute_update_node_exists()
{
  attribute_store_node_t test_updated_node = 0xf9a;
  attribute_store_node_t test_parent_node  = 0xde9e;

  is_zwave_command_class_filtered_for_root_device_verification(
    COMMAND_CLASS_NOTIFICATION_V3,
    test_updated_node);

  // The function will verify the type of the updated node:
  attribute_store_get_node_type_ExpectAndReturn(
    test_updated_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_VERSION);

  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    test_updated_node,
    NULL,
    NULL,
    SL_STATUS_OK);

  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_node_id(
    &test_connection_info.remote.node_id);
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
    &test_connection_info.remote.endpoint_id);

  zwave_node_get_command_class_version_ExpectAndReturn(
    COMMAND_CLASS_NOTIFICATION_V8,
    test_connection_info.remote.node_id,
    test_connection_info.remote.endpoint_id,
    8);

  // Then find out the parent node:
  attribute_store_get_node_parent_ExpectAndReturn(test_updated_node,
                                                  test_parent_node);

  // They must then try to find the ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_SUPPORTED_NOTIFICATION_TYPES
  // attribute under the parent, do not create it as a non NULL value is returned.
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_parent_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_SUPPORTED_NOTIFICATION_TYPES,
    0,
    0x01);

  // Make the call:
  notification_version_callback(test_updated_node, ATTRIBUTE_UPDATED);
}

// in V1 we don't expect any Notificaiton Support Get
void test_zwave_command_class_notification_on_version_attribute_update_v1()
{
  attribute_store_node_t test_updated_node = 0xf9a;

  is_zwave_command_class_filtered_for_root_device_verification(
    COMMAND_CLASS_NOTIFICATION_V3,
    test_updated_node);

  // The function will verify the type of the updated node:
  attribute_store_get_node_type_ExpectAndReturn(
    test_updated_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_VERSION);

  attribute_store_network_helper_get_zwave_ids_from_node_ExpectAndReturn(
    test_updated_node,
    NULL,
    NULL,
    SL_STATUS_OK);

  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_node_id();
  attribute_store_network_helper_get_zwave_ids_from_node_IgnoreArg_zwave_endpoint_id();
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_node_id(
    &test_connection_info.remote.node_id);
  attribute_store_network_helper_get_zwave_ids_from_node_ReturnThruPtr_zwave_endpoint_id(
    &test_connection_info.remote.endpoint_id);

  zwave_node_get_command_class_version_ExpectAndReturn(
    COMMAND_CLASS_NOTIFICATION_V8,
    test_connection_info.remote.node_id,
    test_connection_info.remote.endpoint_id,
    1);

  // Make the call:
  notification_version_callback(test_updated_node, ATTRIBUTE_UPDATED);
}

void test_zwave_command_class_notification_get()
{
  uint8_t frame[10];
  uint16_t frame_len;
  uint8_t notification_type_home_security                  = 0x07;
  attribute_store_node_t test_updated_node                 = 0x66;
  attribute_store_node_t test_updated_node_parent          = 0x55;
  attribute_store_node_t test_updated_node_parent_state    = 0x66;
  attribute_store_node_t test_updated_node_supported_state = 0x77;
  uint8_t test_notification_state                          = 0x00;
  uint8_t test_number_of_supported_states                  = 0x00;

  const uint8_t expected__notification_get[]
    = {COMMAND_CLASS_NOTIFICATION_V4, NOTIFICATION_GET_V4, 0x00, 0x07, 0x00};

  attribute_store_get_first_parent_with_type_ExpectAndReturn(
    test_updated_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_TYPE,
    test_updated_node_parent);

  attribute_store_get_reported_ExpectAndReturn(
    test_updated_node_parent,
    NULL,
    sizeof(notification_type_home_security),
    SL_STATUS_OK);
  attribute_store_get_reported_IgnoreArg_value();
  attribute_store_get_reported_ReturnMemThruPtr_value(
    &notification_type_home_security,
    sizeof(notification_type_home_security));

  attribute_store_get_reported_ExpectAndReturn(test_updated_node_parent_state,
                                               NULL,
                                               sizeof(test_notification_state),
                                               SL_STATUS_OK);
  attribute_store_get_reported_IgnoreArg_value();
  attribute_store_get_reported_ReturnMemThruPtr_value(
    &test_notification_state,
    sizeof(test_notification_state));

  attribute_store_get_first_parent_with_type_ExpectAndReturn(
    test_updated_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_STATE,
    test_updated_node_parent_state);
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_updated_node_parent,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_SUPPORTED_STATES_OR_EVENTS,
    0,
    test_updated_node_supported_state);
  attribute_store_get_node_attribute_value_ExpectAndReturn(
    test_updated_node_supported_state,
    REPORTED_ATTRIBUTE,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnMemThruPtr_value_size(
    &test_number_of_supported_states,
    sizeof(test_number_of_supported_states));

  // Set the node reported value to 0 to prevent Notification Get loops
  attribute_store_set_all_children_reported_ExpectAndReturn(
    test_updated_node_parent,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_EVENT,
    NULL,
    sizeof(int32_t),
    SL_STATUS_OK);
  attribute_store_set_all_children_reported_IgnoreArg_value();

  // Call the function to test
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    notification_state_or_event_callback(test_updated_node, frame, &frame_len));

  // There should have been 1 Get Command queued
  TEST_ASSERT_EQUAL(sizeof(ZW_NOTIFICATION_GET_V4_FRAME), frame_len);
  // We can only verify the payload of the command
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected__notification_get,
                                frame,
                                sizeof(ZW_NOTIFICATION_GET_V4_FRAME));
}

void test_zwave_command_class_supported_notification_types_get()
{
  uint8_t frame[10];
  uint16_t frame_len;
  attribute_store_node_t test_updated_node = 0x66;

  const uint8_t supported_notification_type_get[]
    = {COMMAND_CLASS_NOTIFICATION_V4, NOTIFICATION_SUPPORTED_GET_V4};

  // Call the function to test
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    supported_notification_types_callback(test_updated_node,
                                                          frame,
                                                          &frame_len));

  // There should have been 1 Get Command queued
  TEST_ASSERT_EQUAL(sizeof(ZW_NOTIFICATION_SUPPORTED_GET_V4_FRAME), frame_len);
  // We can only verify the payload of the command
  TEST_ASSERT_EQUAL_UINT8_ARRAY(supported_notification_type_get,
                                frame,
                                sizeof(ZW_NOTIFICATION_SUPPORTED_GET_V4_FRAME));
}

void test_zwave_command_class_support_notification_supported_state_event_types_report_sunny()
{
  const ZW_EVENT_SUPPORTED_REPORT_2BYTE_V3_FRAME frame = {
    .cmdClass         = COMMAND_CLASS_NOTIFICATION_V4,
    .cmd              = EVENT_SUPPORTED_REPORT_V4,
    .notificationType = NOTIFICATION_SMOKE_ALARM,
    .properties1      = 2,           // Number of bytes with bits
    .bitMask1         = 0b11111110,  // Support all events for Smoke Alarm
    .bitMask2         = 0b00000001   // Support all events for Smoke Alarm
  };
  unid_t test_unid                                           = "zw000001";
  const attribute_store_node_t endpoint_node                 = 0x017;
  const attribute_store_node_t notification_node             = 0x012;
  const attribute_store_node_t supported_state_or_event_node = 0x18;
  const attribute_store_node_t notification_state_node       = 0x20;
  const attribute_store_node_t notification_event_node       = 0x21;
  zwave_unid_from_node_id_Expect(test_connection_info.remote.node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    test_connection_info.remote.endpoint_id,
    endpoint_node);

  attribute_store_get_node_child_by_value_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_TYPE,
    REPORTED_ATTRIBUTE,
    &frame.notificationType,
    sizeof(frame.notificationType),
    0,
    notification_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    notification_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_SUPPORTED_STATES_OR_EVENTS,
    0,
    supported_state_or_event_node);

  bool state_created[NOTIFICATION_STATE_SMOKE_ALARM_DUST_IN_DEVICE_STATUS + 1]
    = {false};
  static const uint8_t expected_states[]
    = {NOTIFICATION_STATE_SMOKE_ALARM_SENSOR_STATUS,
       NOTIFICATION_STATE_SMOKE_ALARM_SENSOR_STATUS,
       NOTIFICATION_STATE_SMOKE_ALARM_ALARM_STATUS,
       NOTIFICATION_STATE_SMOKE_ALARM_MAINTENANCE_STATUS,
       NOTIFICATION_STATE_SMOKE_ALARM_MAINTENANCE_STATUS,
       NOTIFICATION_STATE_SMOKE_ALARM_ALARM_STATUS,
       NOTIFICATION_STATE_SMOKE_ALARM_PERIODIC_INSPECTION_STATUS,
       NOTIFICATION_STATE_SMOKE_ALARM_DUST_IN_DEVICE_STATUS};

  for (size_t i = 0; i < sizeof(expected_states); i++) {
    attribute_store_get_node_child_by_value_ExpectAndReturn(
      notification_node,
      ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_STATE,
      REPORTED_ATTRIBUTE,
      &expected_states[i],
      sizeof(expected_states[i]),
      0,
      state_created[expected_states[i]] ? notification_state_node
                                        : ATTRIBUTE_STORE_INVALID_NODE);
    if (!state_created[expected_states[i]]) {
      attribute_store_add_node_ExpectAndReturn(
        ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_STATE,
        notification_node,
        notification_state_node);

      attribute_store_set_node_attribute_value_ExpectAndReturn(
        notification_state_node,
        REPORTED_ATTRIBUTE,
        &expected_states[i],
        sizeof(expected_states[i]),
        SL_STATUS_OK);

      attribute_store_add_node_ExpectAndReturn(
        ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_EVENT,
        notification_state_node,
        notification_event_node);

      attribute_store_add_node_ExpectAndReturn(
        ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_EVENT_PARAMETERS,
        notification_event_node,
        42);

      state_created[expected_states[i]] = true;
    }
  }
  // Supported events are the bit indexes of the set bits
  const uint8_t expected_supported_events[] = {1, 2, 3, 4, 5, 6, 7, 8};
  attribute_store_set_node_attribute_value_ExpectAndReturn(
    supported_state_or_event_node,
    REPORTED_ATTRIBUTE,
    expected_supported_events,
    sizeof(expected_supported_events),
    SL_STATUS_OK);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    command_handler.control_handler(&test_connection_info,
                                                    (const uint8_t *)&frame,
                                                    sizeof(frame)));
}

void test_zwave_command_class_support_notification_supported_state_event_types_report_sunny_access_control()
{
  uint8_t frame_buf[sizeof(ZW_EVENT_SUPPORTED_REPORT_1BYTE_V4_FRAME)
                    + 9];  // Frame with 10 bytes of bitmasks

  ZW_EVENT_SUPPORTED_REPORT_1BYTE_V4_FRAME *frame
    = (ZW_EVENT_SUPPORTED_REPORT_1BYTE_V4_FRAME *)&frame_buf;
  frame->cmdClass         = COMMAND_CLASS_NOTIFICATION_V4;
  frame->cmd              = EVENT_SUPPORTED_REPORT_V4;
  frame->notificationType = NOTIFICATION_ACCESS_CONTROL;
  frame->properties1      = 10;  // Notification type not supported
  size_t bitmask1_offset = sizeof(ZW_EVENT_SUPPORTED_REPORT_1BYTE_V4_FRAME) - 1;
  frame_buf[bitmask1_offset]                                 = 0x00;
  frame_buf[bitmask1_offset + 1]                             = 0x00;
  frame_buf[bitmask1_offset + 2]                             = 0x00;
  frame_buf[bitmask1_offset + 3]                             = 0x02;
  frame_buf[bitmask1_offset + 4]                             = 0x00;
  frame_buf[bitmask1_offset + 5]                             = 0x00;
  frame_buf[bitmask1_offset + 6]                             = 0x00;
  frame_buf[bitmask1_offset + 7]                             = 0x00;
  frame_buf[bitmask1_offset + 8]                             = 0x00;
  frame_buf[bitmask1_offset + 9]                             = 0x00;
  unid_t test_unid                                           = "zw000001";
  const attribute_store_node_t endpoint_node                 = 0x017;
  const attribute_store_node_t notification_node             = 0x012;
  const attribute_store_node_t supported_state_or_event_node = 0x18;
  const attribute_store_node_t notification_state_node       = 0x20;
  const attribute_store_node_t notification_event_node       = 0x21;
  zwave_unid_from_node_id_Expect(test_connection_info.remote.node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    test_connection_info.remote.endpoint_id,
    endpoint_node);

  attribute_store_get_node_child_by_value_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_TYPE,
    REPORTED_ATTRIBUTE,
    &frame->notificationType,
    sizeof(frame->notificationType),
    0,
    notification_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    notification_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_SUPPORTED_STATES_OR_EVENTS,
    0,
    supported_state_or_event_node);

  static const uint8_t expected_state
    = NOTIFICATION_STATE_ACCESS_CONTROL_DOOR_HANDLE_STATE;

  attribute_store_get_node_child_by_value_ExpectAndReturn(
    notification_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_STATE,
    REPORTED_ATTRIBUTE,
    &expected_state,
    sizeof(expected_state),
    0,
    ATTRIBUTE_STORE_INVALID_NODE);

  attribute_store_add_node_ExpectAndReturn(
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_STATE,
    notification_node,
    notification_state_node);

  attribute_store_add_node_ExpectAndReturn(
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_EVENT,
    notification_state_node,
    notification_event_node);

  attribute_store_add_node_ExpectAndReturn(
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_EVENT_PARAMETERS,
    notification_event_node,
    42);

  attribute_store_set_node_attribute_value_ExpectAndReturn(
    notification_state_node,
    REPORTED_ATTRIBUTE,
    &expected_state,
    sizeof(expected_state),
    SL_STATUS_OK);
  // Supported events are the bit indexes of the set bits
  const uint8_t expected_supported_events[] = {25};
  attribute_store_set_node_attribute_value_ExpectAndReturn(
    supported_state_or_event_node,
    REPORTED_ATTRIBUTE,
    expected_supported_events,
    sizeof(expected_supported_events),
    SL_STATUS_OK);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    command_handler.control_handler(&test_connection_info,
                                                    frame_buf,
                                                    sizeof(frame_buf)));
}

void test_zwave_command_class_support_notification_supported_state_event_types_report_error_frame_too_small()
{
  const ZW_EVENT_SUPPORTED_REPORT_1BYTE_V4_FRAME frame = {
    .cmdClass         = COMMAND_CLASS_NOTIFICATION_V4,
    .cmd              = EVENT_SUPPORTED_REPORT_V4,
    .notificationType = NOTIFICATION_SMOKE_ALARM,
    .properties1      = 0,
    .bitMask1         = 0,
  };

  // Test with frame too small
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    command_handler.control_handler(&test_connection_info,
                                                    (const uint8_t *)&frame,
                                                    3));
}

void test_zwave_command_class_support_notification_supported_state_event_types_report_error_not_supported()
{
  const ZW_EVENT_SUPPORTED_REPORT_1BYTE_V4_FRAME frame = {
    .cmdClass         = COMMAND_CLASS_NOTIFICATION_V4,
    .cmd              = EVENT_SUPPORTED_REPORT_V4,
    .notificationType = NOTIFICATION_SMOKE_ALARM,
    .properties1      = 0,  // Notification type not supported
    .bitMask1         = 0,
  };

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    command_handler.control_handler(&test_connection_info,
                                                    (const uint8_t *)&frame,
                                                    sizeof(frame)));
}

void test_zwave_command_class_support_notification_supported_state_event_types_report_error_lookup_ep()
{
  const ZW_EVENT_SUPPORTED_REPORT_1BYTE_V4_FRAME frame = {
    .cmdClass         = COMMAND_CLASS_NOTIFICATION_V4,
    .cmd              = EVENT_SUPPORTED_REPORT_V4,
    .notificationType = NOTIFICATION_SMOKE_ALARM,
    .properties1      = 1,  // Notification type not supported
    .bitMask1         = 0b00000010,
  };
  unid_t test_unid = "zw000001";
  zwave_unid_from_node_id_Expect(test_connection_info.remote.node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    test_connection_info.remote.endpoint_id,
    ATTRIBUTE_STORE_INVALID_NODE);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    command_handler.control_handler(&test_connection_info,
                                                    (const uint8_t *)&frame,
                                                    sizeof(frame)));
}

void test_zwave_command_class_support_notification_supported_state_event_types_report_error_lookup_notification()
{
  const ZW_EVENT_SUPPORTED_REPORT_1BYTE_V4_FRAME frame = {
    .cmdClass         = COMMAND_CLASS_NOTIFICATION_V4,
    .cmd              = EVENT_SUPPORTED_REPORT_V4,
    .notificationType = NOTIFICATION_SMOKE_ALARM,
    .properties1      = 1,  // Notification type not supported
    .bitMask1         = 0b00000010,
  };
  unid_t test_unid                           = "zw000001";
  const attribute_store_node_t endpoint_node = 0x017;

  zwave_unid_from_node_id_Expect(test_connection_info.remote.node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    test_connection_info.remote.endpoint_id,
    endpoint_node);

  attribute_store_get_node_child_by_value_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_TYPE,
    REPORTED_ATTRIBUTE,
    &frame.notificationType,
    sizeof(frame.notificationType),
    0,
    ATTRIBUTE_STORE_INVALID_NODE);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    command_handler.control_handler(&test_connection_info,
                                                    (const uint8_t *)&frame,
                                                    sizeof(frame)));
}

void test_zwave_command_class_support_notification_supported_state_event_types_report_error_lookup_supported_state_or_event_node()
{
  const ZW_EVENT_SUPPORTED_REPORT_1BYTE_V4_FRAME frame = {
    .cmdClass         = COMMAND_CLASS_NOTIFICATION_V4,
    .cmd              = EVENT_SUPPORTED_REPORT_V4,
    .notificationType = NOTIFICATION_SMOKE_ALARM,
    .properties1      = 1,  // Notification type not supported
    .bitMask1         = 0b00000010,
  };
  unid_t test_unid                               = "zw000001";
  const attribute_store_node_t endpoint_node     = 0x017;
  const attribute_store_node_t notification_node = 0x012;

  zwave_unid_from_node_id_Expect(test_connection_info.remote.node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    test_connection_info.remote.endpoint_id,
    endpoint_node);

  attribute_store_get_node_child_by_value_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_TYPE,
    REPORTED_ATTRIBUTE,
    &frame.notificationType,
    sizeof(frame.notificationType),
    0,
    notification_node);

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    notification_node,
    ATTRIBUTE_COMMAND_CLASS_NOTIFICATION_SUPPORTED_STATES_OR_EVENTS,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    command_handler.control_handler(&test_connection_info,
                                                    (const uint8_t *)&frame,
                                                    sizeof(frame)));
}