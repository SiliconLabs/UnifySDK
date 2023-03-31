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
#include "contiki_test_helper.h"

// Generic includes
#include <string.h>

#include "unify_dotdot_attribute_store.h"
#include "unify_dotdot_attribute_store_helpers.h"
#include "unify_dotdot_defined_attribute_types.h"
uic_mqtt_dotdot_level_force_read_attributes_callback_t
  uic_mqtt_dotdot_level_force_read_attributes_callback;
uic_mqtt_dotdot_level_write_attributes_callback_t
  uic_mqtt_dotdot_level_write_attributes_callback;
uic_mqtt_dotdot_level_move_to_level_callback_t
  uic_mqtt_dotdot_level_move_to_level_callback;
uic_mqtt_dotdot_level_move_callback_t uic_mqtt_dotdot_level_move_callback;
uic_mqtt_dotdot_level_step_callback_t uic_mqtt_dotdot_level_step_callback;
uic_mqtt_dotdot_level_stop_callback_t uic_mqtt_dotdot_level_stop_callback;
uic_mqtt_dotdot_level_move_to_level_with_on_off_callback_t
  uic_mqtt_dotdot_level_move_to_level_with_on_off_callback;
uic_mqtt_dotdot_level_move_with_on_off_callback_t
  uic_mqtt_dotdot_level_move_with_on_off_callback;
uic_mqtt_dotdot_level_step_with_on_off_callback_t
  uic_mqtt_dotdot_level_step_with_on_off_callback;
uic_mqtt_dotdot_level_stop_with_on_off_callback_t
  uic_mqtt_dotdot_level_stop_with_on_off_callback;
uic_mqtt_dotdot_level_move_to_closest_frequency_callback_t
  uic_mqtt_dotdot_level_move_to_closest_frequency_callback;

extern dotdot_unid_t expected_unid;
extern dotdot_endpoint_id_t expected_endpoint_id;
extern attribute_store_node_t expected_node_id;
extern unify_dotdot_attribute_store_configuration_t test_configuration;

attribute_store_node_t test_get_endpoint_node(const dotdot_unid_t unid,
                                              dotdot_endpoint_id_t endpoint_id);
void test_level_move_to_level_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_level_move_to_level_callback
    = get_uic_mqtt_dotdot_level_move_to_level_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_level_move_to_level_callback);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_move_to_level_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  // Add the level attribute (still not supported, we need both level and on/off)
  attribute_store_node_t level_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
                               attribute_store_get_root());
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_move_to_level_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               attribute_store_get_root());
  bool on_off_value = true;
  attribute_store_set_reported(on_off_node,
                               &on_off_value,
                               sizeof(on_off_value));

  attribute_store_node_t transition_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_ON_OFF_TRANSITION_TIME,
                               attribute_store_get_root());

  // Now it is supported:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_move_to_level_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  // Try to set the level:
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(level_node));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_move_to_level_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      0,
                      0,
                      0,
                      0));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));

  // Try again with a transition time
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_move_to_level_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      150,
                      2,
                      3,
                      4));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(2, attribute_store_get_desired_number(transition_node));
}

void test_level_move_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_level_move_callback
    = get_uic_mqtt_dotdot_level_move_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_level_move_callback);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_move_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  // Add the level attribute (still not supported, we need both level and on/off)
  attribute_store_node_t level_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
                               attribute_store_get_root());
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_move_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               attribute_store_get_root());
  bool on_off_value = true;
  attribute_store_set_reported(on_off_node,
                               &on_off_value,
                               sizeof(on_off_value));

  // Still not supported:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_move_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  attribute_store_node_t transition_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_ON_OFF_TRANSITION_TIME,
                               attribute_store_get_root());

  // Now it is supported
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_move_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  // Try to set the level:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_level_move_callback(expected_unid,
                                        expected_endpoint_id,
                                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                        ZCL_MOVE_STEP_MODE_DOWN,
                                        0x10,
                                        0,
                                        0));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
}

void test_level_step_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_level_step_callback
    = get_uic_mqtt_dotdot_level_step_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_level_step_callback);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_step_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0,
                      0));

  // Add the level attribute (still not supported, we need both level and on/off)
  attribute_store_node_t level_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
                               attribute_store_get_root());
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_step_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0,
                      0));

  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               attribute_store_get_root());
  bool on_off_value = true;
  attribute_store_set_reported(on_off_node,
                               &on_off_value,
                               sizeof(on_off_value));

  // Still not supported:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_step_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0,
                      0));

  attribute_store_node_t transition_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_ON_OFF_TRANSITION_TIME,
                               attribute_store_get_root());

  // Now it is supported
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_step_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0,
                      0));

  // Step size 0 will do nothing:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_level_step_callback(expected_unid,
                                        expected_endpoint_id,
                                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                        ZCL_MOVE_STEP_MODE_DOWN,
                                        0,
                                        0,
                                        0,
                                        0));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));

  // Move up:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_level_step_callback(expected_unid,
                                        expected_endpoint_id,
                                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                        ZCL_MOVE_STEP_MODE_UP,
                                        2,
                                        0xFFFF,
                                        0,
                                        0));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
}

void test_level_stop_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_level_stop_callback
    = get_uic_mqtt_dotdot_level_stop_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_level_stop_callback);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_stop_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0));

  // Add the level attribute (still not supported, we need both level and on/off)
  attribute_store_node_t level_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
                               attribute_store_get_root());
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_stop_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0));

  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                           attribute_store_get_root());

  // Still not supported:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_stop_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0));

  attribute_store_node_t transition_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_ON_OFF_TRANSITION_TIME,
                               attribute_store_get_root());

  // Now it is supported
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_stop_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0));

  // Try to stop:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_level_stop_callback(expected_unid,
                                        expected_endpoint_id,
                                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                        0,
                                        0));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
}

void test_level_move_to_level_with_on_off_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_level_move_to_level_with_on_off_callback
    = get_uic_mqtt_dotdot_level_move_to_level_with_on_off_callback();
  TEST_ASSERT_NOT_NULL(
    uic_mqtt_dotdot_level_move_to_level_with_on_off_callback);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_move_to_level_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  // Add the level attribute (still not supported, we need both level and on/off)
  attribute_store_node_t level_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
                               attribute_store_get_root());
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_move_to_level_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  attribute_store_node_t transition_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_ON_OFF_TRANSITION_TIME,
                               attribute_store_get_root());
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_move_to_level_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               attribute_store_get_root());
  bool on_off_value = true;
  attribute_store_set_reported(on_off_node,
                               &on_off_value,
                               sizeof(on_off_value));

  // Now it is supported:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_move_to_level_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  // Try to set the level:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_move_to_level_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      0,
                      0,
                      0,
                      0));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));

  // Try again with > MIN value
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_move_to_level_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      150,
                      2,
                      3,
                      4));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));
}

void test_level_move_with_on_off_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_level_move_with_on_off_callback
    = get_uic_mqtt_dotdot_level_move_with_on_off_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_level_move_with_on_off_callback);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_move_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  // Add the level attribute (still not supported, we need both level and on/off)
  attribute_store_node_t level_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
                               attribute_store_get_root());
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_move_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               attribute_store_get_root());

  // Still not supported:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_move_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  attribute_store_node_t transition_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_ON_OFF_TRANSITION_TIME,
                               attribute_store_get_root());

  // Now it is supported
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_move_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  // Try to set the level:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_move_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      ZCL_MOVE_STEP_MODE_DOWN,
                      0x10,
                      0,
                      0));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));

  // Try again to move up:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_move_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      ZCL_MOVE_STEP_MODE_UP,
                      0x10,
                      0,
                      0));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));
}

void test_level_step_with_on_off_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_level_step_with_on_off_callback
    = get_uic_mqtt_dotdot_level_step_with_on_off_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_level_step_with_on_off_callback);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_step_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0,
                      0));

  // Add the level attribute (still not supported, we need both level and on/off)
  attribute_store_node_t level_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
                               attribute_store_get_root());
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_step_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0,
                      0));

  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               attribute_store_get_root());

  // Still not supported:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_step_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0,
                      0));

  attribute_store_node_t transition_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_ON_OFF_TRANSITION_TIME,
                               attribute_store_get_root());

  // Now it is supported
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_step_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0,
                      0));

  // Step size 0 will do nothing:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_step_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      ZCL_MOVE_STEP_MODE_DOWN,
                      0,
                      0,
                      0,
                      0));

  // Move up:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_step_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      ZCL_MOVE_STEP_MODE_UP,
                      2,
                      0xFF00,
                      0,
                      0));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));
}

void test_level_stop_with_on_off_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_level_stop_with_on_off_callback
    = get_uic_mqtt_dotdot_level_stop_with_on_off_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_level_stop_with_on_off_callback);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_stop_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0));

  // Add the level attribute (still not supported, we need both level and on/off)
  attribute_store_node_t level_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
                               attribute_store_get_root());
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_stop_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0));

  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               attribute_store_get_root());

  // Still not supported:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_stop_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0));

  attribute_store_node_t transition_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_ON_OFF_TRANSITION_TIME,
                               attribute_store_get_root());

  // Now it is supported
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_stop_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0));

  // Try to stop:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_stop_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      0,
                      0));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));

  int8_t test_level = 23;
  attribute_store_set_reported(level_node, &test_level, sizeof(test_level));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_stop_with_on_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      0,
                      0));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));
}

void test_level_current_level_validation()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_level_move_to_level_callback
    = get_uic_mqtt_dotdot_level_move_to_level_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_level_move_to_level_callback);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_level_move_to_level_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  // Add the level attribute (still not supported, we need both level and on/off)
  attribute_store_node_t level_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
                               attribute_store_get_root());
  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               attribute_store_get_root());
  bool on_off_value = true;
  attribute_store_set_reported(on_off_node,
                               &on_off_value,
                               sizeof(on_off_value));

  // Try to set the level to 0:
  uint8_t level = 1;
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(level_node));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_move_to_level_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      level,
                      0,
                      0,
                      0));
  TEST_ASSERT_EQUAL(level, attribute_store_get_desired_number(level_node));

  // Try to set the level to 255:
  level = 255;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_move_to_level_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      level,
                      0,
                      0,
                      0));
  TEST_ASSERT_EQUAL(level, attribute_store_get_desired_number(level_node));

  // Now add a max level restriction to 120:
  attribute_store_node_t max_level_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_MAX_LEVEL,
                               attribute_store_get_root());
  attribute_store_set_reported_number(max_level_node, 120);

  // Try to set the level to 255, it will clip to 120:
  level = 255;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_move_to_level_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      level,
                      0,
                      0,
                      0));
  TEST_ASSERT_EQUAL(120, attribute_store_get_desired_number(level_node));

  // Now add a min level restriction to 25:
  attribute_store_node_t min_level_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_MIN_LEVEL,
                               attribute_store_get_root());
  attribute_store_set_reported_number(min_level_node, 25);

  // Try to set the level to 10, it will clip to 25:
  level = 10;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_level_move_to_level_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      level,
                      0,
                      0,
                      0));
  TEST_ASSERT_EQUAL(25, attribute_store_get_desired_number(level_node));
}

void test_level_remaining_time_countdown()
{
  // Create some IdentifyTime attribute
  attribute_store_node_t node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_REMAINING_TIME,
                               attribute_store_get_root());
  uint16_t value = 100;
  attribute_store_set_desired(node, &value, sizeof(value));
  contiki_test_helper_run(100);
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(node));

  // Set the reported to 10 too
  attribute_store_set_reported_as_desired(node);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(100, attribute_store_get_reported_number(node));

  contiki_test_helper_run(1000);
  TEST_ASSERT_EQUAL(90, attribute_store_get_reported_number(node));

  contiki_test_helper_run(1000);
  TEST_ASSERT_EQUAL(80, attribute_store_get_reported_number(node));

  // Undefine the reported value
  attribute_store_undefine_reported(node);
  contiki_test_helper_run(1000);
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(node));

  // Delete the node to tinker a bit with the attribute transitions module.
  attribute_store_delete_node(node);
  contiki_test_helper_run(100);
}
