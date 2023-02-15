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
attribute_store_node_t test_get_endpoint_node(const dotdot_unid_t unid,
                                              dotdot_endpoint_id_t endpoint_id);
uic_mqtt_dotdot_color_control_move_to_hue_callback_t
  uic_mqtt_dotdot_color_control_move_to_hue_callback;
uic_mqtt_dotdot_color_control_move_hue_callback_t
  uic_mqtt_dotdot_color_control_move_hue_callback;
uic_mqtt_dotdot_color_control_step_hue_callback_t
  uic_mqtt_dotdot_color_control_step_hue_callback;
uic_mqtt_dotdot_color_control_move_to_hue_and_saturation_callback_t
  uic_mqtt_dotdot_color_control_move_to_hue_and_saturation_callback;
uic_mqtt_dotdot_color_control_move_to_saturation_callback_t
  uic_mqtt_dotdot_color_control_move_to_saturation_callback;
uic_mqtt_dotdot_color_control_move_saturation_callback_t
  uic_mqtt_dotdot_color_control_move_saturation_callback;
uic_mqtt_dotdot_color_control_step_saturation_callback_t
  uic_mqtt_dotdot_color_control_step_saturation_callback;
uic_mqtt_dotdot_color_control_move_to_color_temperature_callback_t
  uic_mqtt_dotdot_color_control_move_to_color_temperature_callback;
uic_mqtt_dotdot_color_control_move_color_temperature_callback_t
  uic_mqtt_dotdot_color_control_move_color_temperature_callback;
uic_mqtt_dotdot_color_control_step_color_temperature_callback_t
  uic_mqtt_dotdot_color_control_step_color_temperature_callback;
uic_mqtt_dotdot_color_control_stop_move_step_callback_t
  uic_mqtt_dotdot_color_control_stop_move_step_callback;

extern dotdot_unid_t expected_unid;
extern dotdot_endpoint_id_t expected_endpoint_id;
extern attribute_store_node_t expected_node_id;
extern unify_dotdot_attribute_store_configuration_t test_configuration;
//-----

void test_color_control_move_to_hue_command()
{
  // Configure auto-supported command setup
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_color_control_move_to_hue_callback
    = get_uic_mqtt_dotdot_color_control_move_to_hue_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_color_control_move_to_hue_callback);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_color_control_move_to_hue_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0,
                      0));

  // Add the Current Hue attribute (still not supported, we need both Hue and Saturation)

  attribute_store_node_t current_hue_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_HUE,
                               attribute_store_get_root());
  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_SATURATION,
                           attribute_store_get_root());
  attribute_store_node_t capabilities_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_CAPABILITIES,
    attribute_store_get_root());
  uint16_t capabilities = 2;  // Still not supported
  attribute_store_set_reported(capabilities_node,
                               &capabilities,
                               sizeof(capabilities));
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_color_control_move_to_hue_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0,
                      0));

  // Now it is supported:
  capabilities = 1;
  attribute_store_set_reported(capabilities_node,
                               &capabilities,
                               sizeof(capabilities));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_color_control_move_to_hue_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0,
                      0));

  // Try to set the Hue:
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(current_hue_node));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(current_hue_node, REPORTED_ATTRIBUTE));
  test_configuration.clear_reported_on_desired_updates = true;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_color_control_move_to_hue_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      0,
                      0,
                      0,
                      0,
                      0));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(current_hue_node));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(current_hue_node, REPORTED_ATTRIBUTE));

  // Try again with value 0xFF
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_color_control_move_to_hue_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      0xFF,
                      2,
                      2,
                      3,
                      4));

  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(current_hue_node));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(current_hue_node, REPORTED_ATTRIBUTE));
  uint8_t desired_hue = 0x00;
  attribute_store_get_desired(current_hue_node,
                              &desired_hue,
                              sizeof(desired_hue));
  TEST_ASSERT_EQUAL(0xFE, desired_hue);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(current_hue_node, REPORTED_ATTRIBUTE));
}

void test_color_control_move_hue_command()
{
  // Configure auto-supported command setup
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_color_control_move_hue_callback
    = get_uic_mqtt_dotdot_color_control_move_hue_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_color_control_move_hue_callback);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_color_control_move_hue_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  // Add the Current Hue attribute (still not supported, we need both Hue and Saturation)
  attribute_store_node_t current_hue_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_HUE,
                               attribute_store_get_root());

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_color_control_move_hue_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  // Adding support:
  attribute_store_node_t capabilities_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_CAPABILITIES,
    attribute_store_get_root());
  const uint16_t capabilities = 1;
  attribute_store_set_reported(capabilities_node,
                               &capabilities,
                               sizeof(capabilities));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_color_control_move_hue_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  // Try to move the Hue with Mode = 0x02
  // 5.2.2.3.5.2: The Move Mode field SHALL be one of the non-reserved values (0x02 Reserved)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_color_control_move_hue_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      0x02,
                      0x05,
                      0,
                      0));

  // Try to move the Hue with rate = 0
  // 5.2.2.3.5.3: If the Rate field has a value of zero, the command has no effect and a default response command (see Chapter 2) is
  // sent in response, with the status code set to INVALID_FIELD.
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_color_control_move_hue_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      ZCL_CC_MOVE_MODE_UP,
                      0,
                      0,
                      0));

  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(current_hue_node));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(current_hue_node, REPORTED_ATTRIBUTE));
  // Check ZCL_CC_MOVE_MODE_UP:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_color_control_move_hue_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      ZCL_CC_MOVE_MODE_UP,
                      2,
                      3,
                      4));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(current_hue_node));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(current_hue_node, REPORTED_ATTRIBUTE));
  uint8_t desired_hue = 0x00;
  attribute_store_get_desired(current_hue_node,
                              &desired_hue,
                              sizeof(desired_hue));
  TEST_ASSERT_EQUAL(0xFE, desired_hue);
  // Check ZCL_CC_MOVE_MODE_DOWN:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_color_control_move_hue_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      ZCL_CC_MOVE_MODE_DOWN,
                      2,
                      3,
                      4));
  attribute_store_get_desired(current_hue_node,
                              &desired_hue,
                              sizeof(desired_hue));
  TEST_ASSERT_EQUAL(0x00, desired_hue);

  // Check ZCL_CC_MOVE_MODE_STOP:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_color_control_move_hue_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      ZCL_CC_MOVE_MODE_UP,
                      1,
                      3,
                      4));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_color_control_move_hue_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      ZCL_CC_MOVE_MODE_STOP,
                      1,
                      3,
                      4));
  attribute_store_get_desired(current_hue_node,
                              &desired_hue,
                              sizeof(desired_hue));
  uint32_t reported_hue = 0x00;
  attribute_store_get_reported(current_hue_node,
                               &reported_hue,
                               sizeof(reported_hue));
  TEST_ASSERT_EQUAL(desired_hue, reported_hue);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(current_hue_node, REPORTED_ATTRIBUTE));
}

void test_color_control_step_hue_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_color_control_step_hue_callback
    = get_uic_mqtt_dotdot_color_control_step_hue_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_color_control_step_hue_callback);
  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_color_control_step_hue_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0,
                      0));

  // Add the Current Hue attribute (still not supported, we need both Hue and Saturation)
  attribute_store_node_t current_hue_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_HUE,
                               attribute_store_get_root());
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_color_control_step_hue_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0,
                      0));

  // Adding support:
  attribute_store_node_t capabilities_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_CAPABILITIES,
    attribute_store_get_root());
  const uint16_t capabilities = 1;
  attribute_store_set_reported(capabilities_node,
                               &capabilities,
                               sizeof(capabilities));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_color_control_step_hue_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0,
                      0));

  // 5.2.2.3.6.2 The Step Mode field SHALL be one of the non-reserved values (0x00, 0x02 Reserved)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_color_control_step_hue_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      0x02,
                      1,
                      2,
                      3,
                      4));

  uint8_t reported_hue = 0x00;
  uint8_t desired_hue  = 0x00;
  // Try to set ZCL_CC_STEP_MODE_UP:
  uint8_t step = 1;
  attribute_store_get_reported(current_hue_node,
                               &reported_hue,
                               sizeof(reported_hue));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_color_control_step_hue_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      ZCL_CC_STEP_MODE_UP,
                      step,
                      2,
                      3,
                      4));
  attribute_store_get_desired(current_hue_node,
                              &desired_hue,
                              sizeof(desired_hue));
  TEST_ASSERT_EQUAL(desired_hue, reported_hue + step);
}

void test_color_control_move_to_hue_and_saturation_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_color_control_move_to_hue_and_saturation_callback
    = get_uic_mqtt_dotdot_color_control_move_to_hue_and_saturation_callback();
  TEST_ASSERT_NOT_NULL(
    uic_mqtt_dotdot_color_control_move_to_hue_and_saturation_callback);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    uic_mqtt_dotdot_color_control_move_to_hue_and_saturation_callback(
      expected_unid,
      expected_endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
      0,
      0,
      0,
      0,
      0));

  // Add the Current Hue attribute (still not supported, we need both Hue and Saturation)
  attribute_store_node_t current_hue_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_HUE,
                               attribute_store_get_root());
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    uic_mqtt_dotdot_color_control_move_to_hue_and_saturation_callback(
      expected_unid,
      expected_endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
      0,
      0,
      0,
      0,
      0));

  // Adding support:
  attribute_store_node_t current_saturation_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_SATURATION,
    attribute_store_get_root());
  attribute_store_node_t capabilities_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_CAPABILITIES,
    attribute_store_get_root());
  const uint16_t capabilities = 1;
  attribute_store_set_reported(capabilities_node,
                               &capabilities,
                               sizeof(capabilities));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_color_control_move_to_hue_and_saturation_callback(
      expected_unid,
      expected_endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
      0,
      0,
      0,
      0,
      0));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(current_hue_node, DESIRED_ATTRIBUTE));

  // set saturation and hue
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_color_control_move_to_hue_and_saturation_callback(
      expected_unid,
      expected_endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
      0x02,
      0x05,
      1,
      0,
      0));

  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(current_hue_node, DESIRED_ATTRIBUTE));

  uint8_t desired_hue_value = 0x00;
  attribute_store_get_desired(current_hue_node,
                              &desired_hue_value,
                              sizeof(desired_hue_value));
  TEST_ASSERT_EQUAL(0x02, desired_hue_value);

  TEST_ASSERT_TRUE(attribute_store_is_value_defined(current_saturation_node,
                                                    DESIRED_ATTRIBUTE));

  uint8_t desired_saturation_value = 0x00;
  attribute_store_get_desired(current_saturation_node,
                              &desired_saturation_value,
                              sizeof(desired_saturation_value));
  TEST_ASSERT_EQUAL(0x05, desired_saturation_value);
}

void test_color_control_move_to_saturation_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_color_control_move_to_saturation_callback
    = get_uic_mqtt_dotdot_color_control_move_to_saturation_callback();
  TEST_ASSERT_NOT_NULL(
    uic_mqtt_dotdot_color_control_move_to_saturation_callback);
  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_color_control_move_to_saturation_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  // Create hue/saturation attributes, and capabilities
  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_HUE,
                           attribute_store_get_root());
  attribute_store_node_t current_saturation_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_SATURATION,
    attribute_store_get_root());
  attribute_store_node_t capabilities_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_CAPABILITIES,
    attribute_store_get_root());
  uint16_t capabilities = 0;  // Still not supported
  attribute_store_set_reported(capabilities_node,
                               &capabilities,
                               sizeof(capabilities));

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_color_control_move_to_saturation_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  // Now it is supported:
  capabilities = 1;
  attribute_store_set_reported(capabilities_node,
                               &capabilities,
                               sizeof(capabilities));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_color_control_move_to_saturation_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  // Try to set the Hue:
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(current_saturation_node,
                                                     DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_color_control_move_to_saturation_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      0,
                      0,
                      0,
                      0));
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(current_saturation_node,
                                                    DESIRED_ATTRIBUTE));

  // Try again with value 0xFF
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_color_control_move_to_saturation_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      0xFF,
                      2,
                      3,
                      4));
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(current_saturation_node,
                                                    DESIRED_ATTRIBUTE));
  uint8_t desired_saturation = 0x00;
  attribute_store_get_desired(current_saturation_node,
                              &desired_saturation,
                              sizeof(desired_saturation));
  TEST_ASSERT_EQUAL(0xFE, desired_saturation);
}

void test_color_control_move_saturation_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_color_control_move_saturation_callback
    = get_uic_mqtt_dotdot_color_control_move_saturation_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_color_control_move_saturation_callback);
  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_color_control_move_saturation_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  // Add the Current Hue attribute (still not supported, we need both Hue and Saturation)
  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_HUE,
                           attribute_store_get_root());
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_color_control_move_saturation_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  // Adding support
  attribute_store_node_t current_saturation_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_SATURATION,
    attribute_store_get_root());
  attribute_store_node_t capabilities_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_CAPABILITIES,
    attribute_store_get_root());
  const uint16_t capabilities = 1;
  attribute_store_set_reported(capabilities_node,
                               &capabilities,
                               sizeof(capabilities));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_color_control_move_saturation_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  // Try to move the Saturation with Mode = 0x02
  // 5.2.2.3.5.2: The Move Mode field SHALL be one of the non-reserved values (0x02 Reserved)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_color_control_move_saturation_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      0x02,
                      0x05,
                      0,
                      0));

  // Try to move the Saturation with rate = 0
  // 5.2.2.3.5.3: If the Rate field has a value of zero, the command has no effect and a default response command (see Chapter 2) is
  // sent in response, with the status code set to INVALID_FIELD.
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_color_control_move_saturation_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      ZCL_CC_MOVE_MODE_UP,
                      0,
                      0,
                      0));

  TEST_ASSERT_FALSE(attribute_store_is_value_defined(current_saturation_node,
                                                     DESIRED_ATTRIBUTE));

  // Check ZCL_CC_MOVE_MODE_UP:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_color_control_move_saturation_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      ZCL_CC_MOVE_MODE_UP,
                      2,
                      3,
                      4));

  TEST_ASSERT_TRUE(attribute_store_is_value_defined(current_saturation_node,
                                                    DESIRED_ATTRIBUTE));

  uint8_t desired_saturation = 0x00;
  attribute_store_get_desired(current_saturation_node,
                              &desired_saturation,
                              sizeof(desired_saturation));
  TEST_ASSERT_EQUAL(0xFE, desired_saturation);

  // Check ZCL_CC_MOVE_MODE_DOWN:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_color_control_move_saturation_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      ZCL_CC_MOVE_MODE_DOWN,
                      2,
                      3,
                      4));
  attribute_store_get_desired(current_saturation_node,
                              &desired_saturation,
                              sizeof(desired_saturation));
  TEST_ASSERT_EQUAL(0x00, desired_saturation);

  // Check ZCL_CC_MOVE_MODE_STOP:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_color_control_move_saturation_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      ZCL_CC_MOVE_MODE_UP,
                      1,
                      3,
                      4));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_color_control_move_saturation_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      ZCL_CC_MOVE_MODE_STOP,
                      1,
                      3,
                      4));
  attribute_store_get_desired(current_saturation_node,
                              &desired_saturation,
                              sizeof(desired_saturation));
  uint8_t reported_saturation = 0x00;
  attribute_store_get_reported(current_saturation_node,
                               &reported_saturation,
                               sizeof(reported_saturation));
  TEST_ASSERT_EQUAL(desired_saturation, reported_saturation);
}

void test_color_control_step_saturation_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_color_control_step_saturation_callback
    = get_uic_mqtt_dotdot_color_control_step_saturation_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_color_control_step_saturation_callback);

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_color_control_step_saturation_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0,
                      0));

  // Add the Current Hue attribute (still not supported, we need both Hue and Saturation)
  attribute_store_node_t current_saturation_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_SATURATION,
    attribute_store_get_root());
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_color_control_step_saturation_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0,
                      0));

  // Adding support
  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_SATURATION,
                           attribute_store_get_root());
  attribute_store_node_t capabilities_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_CAPABILITIES,
    attribute_store_get_root());
  const uint16_t capabilities = 1;
  attribute_store_set_reported(capabilities_node,
                               &capabilities,
                               sizeof(capabilities));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_color_control_step_saturation_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0,
                      0));

  // 5.2.2.3.6.2 The Step Mode field SHALL be one of the non-reserved values (0x00, 0x02 Reserved)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_color_control_step_saturation_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      0x02,
                      1,
                      2,
                      3,
                      4));

  uint8_t reported_saturation = 0x00;
  uint8_t desired_saturation  = 0x00;
  // Try to set ZCL_CC_STEP_MODE_UP:
  uint8_t step = 1;
  attribute_store_get_reported(current_saturation_node,
                               &reported_saturation,
                               sizeof(reported_saturation));
  desired_saturation = reported_saturation + step;

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_color_control_step_saturation_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      ZCL_CC_STEP_MODE_UP,
                      step,
                      2,
                      3,
                      4));
  attribute_store_get_desired(current_saturation_node,
                              &desired_saturation,
                              sizeof(desired_saturation));
  TEST_ASSERT_EQUAL(desired_saturation, reported_saturation + step);
}

void test_color_control_move_to_color_temperature_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_color_control_move_to_color_temperature_callback
    = get_uic_mqtt_dotdot_color_control_move_to_color_temperature_callback();
  TEST_ASSERT_NOT_NULL(
    uic_mqtt_dotdot_color_control_move_to_color_temperature_callback);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    uic_mqtt_dotdot_color_control_move_to_color_temperature_callback(
      expected_unid,
      expected_endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
      0,
      0,
      0,
      0));

  attribute_store_node_t capabilities_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_CAPABILITIES,
    attribute_store_get_root());
  uint16_t capabilities = 0x10;
  attribute_store_set_reported(capabilities_node,
                               &capabilities,
                               sizeof(capabilities));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_color_control_move_to_color_temperature_callback(
      expected_unid,
      expected_endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
      0,
      0,
      0,
      0));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_color_control_move_to_color_temperature_callback(
      expected_unid,
      expected_endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
      0,
      0,
      0,
      0));
}

void test_color_control_move_color_temperature_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_color_control_move_color_temperature_callback
    = get_uic_mqtt_dotdot_color_control_move_color_temperature_callback();
  TEST_ASSERT_NOT_NULL(
    uic_mqtt_dotdot_color_control_move_color_temperature_callback);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    uic_mqtt_dotdot_color_control_move_color_temperature_callback(
      expected_unid,
      expected_endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
      0,
      0,
      0,
      0,
      0,
      0));

  attribute_store_node_t capabilities_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_CAPABILITIES,
    attribute_store_get_root());
  uint16_t capabilities = 0x10;
  attribute_store_set_reported(capabilities_node,
                               &capabilities,
                               sizeof(capabilities));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_color_control_move_color_temperature_callback(
      expected_unid,
      expected_endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
      0,
      0,
      0,
      0,
      0,
      0));

  // Rate = 0
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_color_control_move_color_temperature_callback(
      expected_unid,
      expected_endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
      0,
      0,
      0,
      0,
      0,
      0));

  // Rate > 0, move STOP
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_color_control_move_color_temperature_callback(
      expected_unid,
      expected_endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
      ZCL_CC_MOVE_MODE_STOP,
      5,
      0,
      0,
      0,
      0));

  // Rate > 0, move UP
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_color_control_move_color_temperature_callback(
      expected_unid,
      expected_endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
      ZCL_CC_MOVE_MODE_UP,
      5,
      0,
      0,
      0,
      0));

  // Rate > 0, move DOWN
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_color_control_move_color_temperature_callback(
      expected_unid,
      expected_endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
      ZCL_CC_MOVE_MODE_DOWN,
      5,
      0,
      0,
      0,
      0));

  // Rate > 0, move UNKNOWN
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_color_control_move_color_temperature_callback(
      expected_unid,
      expected_endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
      ZCL_CC_MOVE_MODE_DOWN + 5,
      5,
      0,
      0,
      0,
      0));
}

void test_color_control_step_color_temperature_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_color_control_step_color_temperature_callback
    = get_uic_mqtt_dotdot_color_control_step_color_temperature_callback();
  TEST_ASSERT_NOT_NULL(
    uic_mqtt_dotdot_color_control_step_color_temperature_callback);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    uic_mqtt_dotdot_color_control_step_color_temperature_callback(
      expected_unid,
      expected_endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
      0,
      0,
      0,
      0,
      0,
      0,
      0));

  attribute_store_node_t capabilities_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_CAPABILITIES,
    attribute_store_get_root());
  uint16_t capabilities = 0x10;
  attribute_store_set_reported(capabilities_node,
                               &capabilities,
                               sizeof(capabilities));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_color_control_step_color_temperature_callback(
      expected_unid,
      expected_endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
      0,
      0,
      0,
      0,
      0,
      0,
      0));

  // Step size 0
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_color_control_step_color_temperature_callback(
      expected_unid,
      expected_endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
      0,
      0,
      0,
      0,
      0,
      0,
      0));

  // Step size >0, step mode UP
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_color_control_step_color_temperature_callback(
      expected_unid,
      expected_endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
      ZCL_CC_STEP_MODE_UP,
      4,
      0,
      0,
      0,
      0,
      0));

  // Step size >0, step mode DOWN
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_color_control_step_color_temperature_callback(
      expected_unid,
      expected_endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
      ZCL_CC_STEP_MODE_DOWN,
      4,
      0,
      0,
      0,
      0,
      0));

  // Step size >0, step mode UNKONWN
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    uic_mqtt_dotdot_color_control_step_color_temperature_callback(
      expected_unid,
      expected_endpoint_id,
      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
      ZCL_CC_STEP_MODE_DOWN + 1,
      4,
      0,
      0,
      0,
      0,
      0));
}

void test_color_control_stop_move_step_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_color_control_stop_move_step_callback
    = get_uic_mqtt_dotdot_color_control_stop_move_step_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_color_control_stop_move_step_callback);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_color_control_stop_move_step_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0));

  attribute_store_node_t capabilities_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_CAPABILITIES,
    attribute_store_get_root());
  uint16_t capabilities = 1;
  attribute_store_set_reported(capabilities_node,
                               &capabilities,
                               sizeof(capabilities));

  attribute_store_node_t current_hue_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_HUE,
                               attribute_store_get_root());
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_color_control_stop_move_step_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      0,
                      0));

  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(current_hue_node, DESIRED_ATTRIBUTE));
}
