/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "humidity_control_cluster_server.h"
#include "unify_dotdot_attribute_store.h"
#include "unity.h"

// Unify components
#include "datastore.h"
#include "attribute_store_fixt.h"
#include "attribute_store_helper.h"
#include "unify_dotdot_defined_attribute_types.h"
#include "dotdot_mqtt_mock.h"

// ZPC Components
#include "zwave_unid.h"
#include "zwave_command_class_humidity_control_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"
#include "attribute_store_defined_attribute_types.h"

uic_mqtt_dotdot_unify_humidity_control_mode_set_callback_t
  uic_mqtt_dotdot_unify_humidity_control_mode_set_callback;

uic_mqtt_dotdot_unify_humidity_control_setpoint_set_callback_t
  uic_mqtt_dotdot_unify_humidity_control_setpoint_set_callback;

void uic_mqtt_dotdot_unify_humidity_control_mode_set_callback_set_stub(
  const uic_mqtt_dotdot_unify_humidity_control_mode_set_callback_t callback,
  int cmock_num_calls)
{
  uic_mqtt_dotdot_unify_humidity_control_mode_set_callback = callback;
}

void uic_mqtt_dotdot_unify_humidity_control_setpoint_set_callback_set_stub(
  const uic_mqtt_dotdot_unify_humidity_control_setpoint_set_callback_t callback,
  int cmock_num_calls)
{
  uic_mqtt_dotdot_unify_humidity_control_setpoint_set_callback = callback;
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
  uic_mqtt_dotdot_unify_humidity_control_mode_set_callback_set_Stub(
    &uic_mqtt_dotdot_unify_humidity_control_mode_set_callback_set_stub);
  uic_mqtt_dotdot_unify_humidity_control_setpoint_set_callback_set_Stub(
    &uic_mqtt_dotdot_unify_humidity_control_setpoint_set_callback_set_stub);

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, humidity_control_cluster_server_init());
}

/// Called after each and every test
void tearDown()
{
  attribute_store_delete_node(attribute_store_get_root());
}

void test_humidity_control_set_mode_command_mapping()
{
  TEST_ASSERT_NOT_NULL(
    uic_mqtt_dotdot_unify_humidity_control_mode_set_callback);

  ModeType expected_current_mode = ZCL_MODE_TYPE_DEHUMIDIFY;

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_unify_humidity_control_mode_set_callback(
                      supporting_node_unid,
                      endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      expected_current_mode));

  attribute_store_node_t current_mode_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_CURRENT_MODE,
    endpoint_id_node);

  // test support
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_unify_humidity_control_mode_set_callback(
                      supporting_node_unid,
                      endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      expected_current_mode));
  // Test callback
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_unify_humidity_control_mode_set_callback(
                      supporting_node_unid,
                      endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      expected_current_mode));

  humidity_control_mode_t current_mode = 0;

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            attribute_store_get_desired(current_mode_node,
                                                        &current_mode,
                                                        sizeof(current_mode)),
                            "Can't get current mode value");

  // Test value
  TEST_ASSERT_EQUAL(expected_current_mode, current_mode);
}

void test_humidity_control_setpoint_set_command_mapping()
{
  TEST_ASSERT_NOT_NULL(
    uic_mqtt_dotdot_unify_humidity_control_setpoint_set_callback);

  SetpointType expected_setpoint_type_ucl = ZCL_SETPOINT_TYPE_DEHUMIDIFIER;
  uint8_t expected_precision_ucl          = 2;
  uint8_t expected_scale_ucl              = 1;
  int32_t expected_value_ucl              = 10;

  // Always supported
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_unify_humidity_control_setpoint_set_callback(
      supporting_node_unid,
      endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
      expected_setpoint_type_ucl,
      expected_precision_ucl,
      expected_scale_ucl,
      expected_value_ucl));

  humidity_control_setpoint_type_t expected_type
    = expected_setpoint_type_ucl + 1;
  // Emplace with wrong setpoint type
  attribute_store_emplace(
    endpoint_id_node,
    ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_TYPE,
    &expected_type,
    sizeof(expected_type));

  // Should fail
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    uic_mqtt_dotdot_unify_humidity_control_setpoint_set_callback(
      supporting_node_unid,
      endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
      expected_setpoint_type_ucl,
      expected_precision_ucl,
      expected_scale_ucl,
      expected_value_ucl));

  expected_type = expected_setpoint_type_ucl;
  // Emplace with wrong setpoint type
  attribute_store_node_t current_mode_node = attribute_store_emplace(
    endpoint_id_node,
    ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_TYPE,
    &expected_type,
    sizeof(expected_type));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_unify_humidity_control_setpoint_set_callback(
      supporting_node_unid,
      endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
      expected_setpoint_type_ucl,
      expected_precision_ucl,
      expected_scale_ucl,
      expected_value_ucl));

  humidity_control_setpoint_scale_t reported_scale;
  humidity_control_setpoint_value_t reported_value;
  humidity_control_setpoint_precision_t reported_precision;

  attribute_store_get_child_reported(
    current_mode_node,
    ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_VALUE_PRECISION,
    &reported_precision,
    sizeof(reported_precision));
  TEST_ASSERT_EQUAL_MESSAGE(expected_precision_ucl,
                            reported_precision,
                            "Precision reported value mismatch");
  attribute_store_get_child_reported(
    current_mode_node,
    ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_VALUE_SCALE,
    &reported_scale,
    sizeof(reported_scale));
  TEST_ASSERT_EQUAL_MESSAGE(expected_scale_ucl,
                            reported_scale,
                            "Scale reported value mismatch");

  // Value is desired since it will be sent to end-device
  attribute_store_node_t value_node = attribute_store_get_first_child_by_type(
    current_mode_node,
    ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_SETPOINT_VALUE);

  attribute_store_get_desired(value_node,
                              &reported_value,
                              sizeof(reported_value));
  TEST_ASSERT_EQUAL_MESSAGE(expected_value_ucl,
                            reported_value,
                            "Value desired value mismatch");
}