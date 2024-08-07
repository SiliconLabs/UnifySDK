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
// Test includes
#include "unity.h"
#include "unify_dotdot_attribute_store_test.h"

// Unify components
#include "datastore.h"
#include "attribute_store_fixt.h"
#include "attribute_store_helper.h"
#include "sl_log.h"
// Mock includes
#include "dotdot_mqtt_mock.h"
#include "uic_mqtt_mock.h"

// Generic includes
#include <string.h>

#include "unify_dotdot_attribute_store.h"
#include "unify_dotdot_attribute_store_helpers.h"
#include "unify_dotdot_defined_attribute_types.h"

// Things from unify_dotdot_attribute_store_test_c.zapt
// (unify_dotdot_attribute_store_test.c)

uic_mqtt_dotdot_unify_fan_control_set_fan_mode_callback_t
  uic_mqtt_dotdot_fan_control_set_fan_mode_callback;

#define TEST_UNID "test-unid-123"
extern dotdot_unid_t expected_unid;
extern dotdot_endpoint_id_t expected_endpoint_id;
extern attribute_store_node_t expected_node_id;
extern unify_dotdot_attribute_store_configuration_t test_configuration;
attribute_store_node_t test_get_endpoint_node(const dotdot_unid_t unid,
                                              dotdot_endpoint_id_t endpoint_id);
sl_status_t test_get_unid_endpoint(attribute_store_node_t node,
                                   char *unid,
                                   dotdot_endpoint_id_t *endpoint_id);
//-----

void test_zwave_fan_control_no_get_endpoint_function_registered()
{
  unify_dotdot_attribute_store_set_configuration(NULL);

  // Default value for support check is fail:
  uic_mqtt_dotdot_fan_control_set_fan_mode_callback
    = get_uic_mqtt_dotdot_unify_fan_control_set_fan_mode_callback();
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_fan_control_set_fan_mode_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      ZCL_Z_WAVE_FAN_MODE_ENUM_AUTO_HIGH));

  // Default value for normal call is OK:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_fan_control_set_fan_mode_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      ZCL_Z_WAVE_FAN_MODE_ENUM_AUTO_HIGH));
}

void test_zwave_fan_control_set_fan_mode_command_support()
{
  // Configure auto-supported command setup
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.automatic_deduction_of_supported_commands = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Check that the dispatch exists.
  uic_mqtt_dotdot_fan_control_set_fan_mode_callback
    = get_uic_mqtt_dotdot_unify_fan_control_set_fan_mode_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_fan_control_set_fan_mode_callback);

  // First test the support (not supported without attributes)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_fan_control_set_fan_mode_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      ZCL_Z_WAVE_FAN_MODE_ENUM_AUTO_HIGH));

  // Add the DOTDOT_ATTRIBUTE_ID_FAN_CONTROL_Z_WAVE_FAN_MODE_ENUM attribute
  attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_UNIFY_FAN_CONTROL_Z_WAVE_FAN_MODE,
    attribute_store_get_root());

  // Now it is supported
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_fan_control_set_fan_mode_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      ZCL_Z_WAVE_FAN_MODE_ENUM_AUTO_HIGH));

  // Configure for no SupportedCommand support
  test_configuration.automatic_deduction_of_supported_commands = false;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Now it is not supported, feature is disabled
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_fan_control_set_fan_mode_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      ZCL_Z_WAVE_FAN_MODE_ENUM_AUTO_HIGH));
}

void test_zwave_fan_control_set_fan_mode_command_update_desired()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Configure auto-desired update and auto-supported command setup

  // Check that the dispatch exists.
  uic_mqtt_dotdot_fan_control_set_fan_mode_callback
    = get_uic_mqtt_dotdot_unify_fan_control_set_fan_mode_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_fan_control_set_fan_mode_callback);

  // Nothing happens when no Attributes are under the endpoint (root in our test)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_fan_control_set_fan_mode_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      ZCL_Z_WAVE_FAN_MODE_ENUM_AUTO_HIGH));
  TEST_ASSERT_EQUAL(
    0,
    attribute_store_get_node_child_count(attribute_store_get_root()));

  // Add attributes
  attribute_store_node_t fan_control_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_UNIFY_FAN_CONTROL_Z_WAVE_FAN_MODE,
    attribute_store_get_root());
  ZWaveFanModeEnum value = ZCL_Z_WAVE_FAN_MODE_ENUM_LOW;
  attribute_store_set_reported(fan_control_node, &value, sizeof(value));

  // Now the desired value gets set to On when receiving the On Command:
  expected_endpoint_id = 4;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_fan_control_set_fan_mode_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      value));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(fan_control_node));
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(fan_control_node));
  TEST_ASSERT_EQUAL(
    ZCL_Z_WAVE_FAN_MODE_ENUM_LOW,
    dotdot_get_unify_fan_control_z_wave_fan_mode(expected_unid,
                                                 expected_endpoint_id,
                                                 DESIRED_ATTRIBUTE));
  // Configure for no desired value update
  test_configuration.update_attribute_desired_values_on_commands = false;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  attribute_store_undefine_desired(fan_control_node);

  // Now it is not supported, feature is disabled
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_fan_control_set_fan_mode_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      value));
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(fan_control_node));
}

void test_zwave_fan_control_set_fan_mode_command_clear_reported()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Check that the dispatch on_off_command exists.

  // Check that the dispatch exists.
  uic_mqtt_dotdot_fan_control_set_fan_mode_callback
    = get_uic_mqtt_dotdot_unify_fan_control_set_fan_mode_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_fan_control_set_fan_mode_callback);

  // Nothing happens when no Attributes are under the endpoint (root in our test)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_fan_control_set_fan_mode_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      ZCL_Z_WAVE_FAN_MODE_ENUM_AUTO_HIGH));
  TEST_ASSERT_EQUAL(
    0,
    attribute_store_get_node_child_count(attribute_store_get_root()));

  // Add attributes
  attribute_store_node_t fan_control_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_UNIFY_FAN_CONTROL_Z_WAVE_FAN_MODE,
    attribute_store_get_root());
  ZWaveFanModeEnum value = ZCL_Z_WAVE_FAN_MODE_ENUM_LOW;
  attribute_store_set_reported(fan_control_node, &value, sizeof(value));

  // Now the desired value gets set to On when receiving the On Command:
  expected_endpoint_id = 4;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_fan_control_set_fan_mode_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      value));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(fan_control_node));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(fan_control_node));
  TEST_ASSERT_EQUAL(
    ZCL_Z_WAVE_FAN_MODE_ENUM_LOW,
    dotdot_get_unify_fan_control_z_wave_fan_mode(expected_unid,
                                                 expected_endpoint_id,
                                                 DESIRED_ATTRIBUTE));
}