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

attribute_store_node_t test_get_endpoint_node(const dotdot_unid_t unid,
                                              dotdot_endpoint_id_t endpoint_id);
uic_mqtt_dotdot_door_lock_lock_door_callback_t
  uic_mqtt_dotdot_door_lock_lock_door_callback;
uic_mqtt_dotdot_door_lock_unlock_door_callback_t
  uic_mqtt_dotdot_door_lock_unlock_door_callback;
static uic_mqtt_dotdot_door_lock_toggle_callback_t toggle_command;
static uic_mqtt_dotdot_door_lock_unlock_with_timeout_callback_t
  unlock_with_timeout;

// Shared variables
extern dotdot_unid_t expected_unid;
extern dotdot_endpoint_id_t expected_endpoint_id;
extern attribute_store_node_t expected_node_id;
extern unify_dotdot_attribute_store_configuration_t test_configuration;

void test_door_lock_lock_unlock_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_door_lock_lock_door_callback
    = get_uic_mqtt_dotdot_door_lock_lock_door_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_door_lock_lock_door_callback);
  uic_mqtt_dotdot_door_lock_unlock_door_callback
    = get_uic_mqtt_dotdot_door_lock_unlock_door_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_door_lock_unlock_door_callback);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_door_lock_lock_door_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0));

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_door_lock_unlock_door_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0));

  attribute_store_node_t current_lock_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_LOCK_STATE,
                               attribute_store_get_root());
  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_door_lock_lock_door_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0));
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(current_lock_node));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_door_lock_lock_door_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      0));
  uint8_t desired_lock_state = 0x00;
  attribute_store_get_desired(current_lock_node,
                              &desired_lock_state,
                              sizeof(desired_lock_state));
  TEST_ASSERT_EQUAL(1, desired_lock_state);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(current_lock_node, REPORTED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_door_lock_unlock_door_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      0));
  desired_lock_state = 0x00;
  attribute_store_get_desired(current_lock_node,
                              &desired_lock_state,
                              sizeof(desired_lock_state));
  TEST_ASSERT_EQUAL(2, desired_lock_state);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(current_lock_node, REPORTED_ATTRIBUTE));
}

void test_door_lock_toggle_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  toggle_command = get_uic_mqtt_dotdot_door_lock_toggle_callback();

  // No supported without a Door Lock state attribute
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    toggle_command(expected_unid,
                                   expected_endpoint_id,
                                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                   NULL));

  attribute_store_node_t lock_state_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_LOCK_STATE,
                               attribute_store_get_root());
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    toggle_command(expected_unid,
                                   expected_endpoint_id,
                                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                   NULL));

  attribute_store_set_reported_number(lock_state_node,
                                      ZCL_DOOR_LOCK_LOCK_STATE_LOCKED);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    toggle_command(expected_unid,
                                   expected_endpoint_id,
                                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                   NULL));

  TEST_ASSERT_EQUAL(ZCL_DOOR_LOCK_LOCK_STATE_UNLOCKED,
                    attribute_store_get_desired_number(lock_state_node));

  attribute_store_set_reported_as_desired(lock_state_node);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    toggle_command(expected_unid,
                                   expected_endpoint_id,
                                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                   NULL));
  TEST_ASSERT_EQUAL(ZCL_DOOR_LOCK_LOCK_STATE_LOCKED,
                    attribute_store_get_desired_number(lock_state_node));
}

void test_door_lock_unlock_with_timeout_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  test_configuration.endpoint_type = ATTRIBUTE_TREE_ROOT;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  unlock_with_timeout
    = get_uic_mqtt_dotdot_door_lock_unlock_with_timeout_callback();

  // No supported without a Door Lock state and auto-relock attribute
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    unlock_with_timeout(expected_unid,
                        expected_endpoint_id,
                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                        0,
                        NULL));

  attribute_store_node_t lock_state_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_LOCK_STATE,
                               attribute_store_get_root());
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    unlock_with_timeout(expected_unid,
                        expected_endpoint_id,
                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                        0,
                        NULL));
  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_DOOR_LOCK_AUTO_RELOCK_TIME,
                           attribute_store_get_root());

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    unlock_with_timeout(expected_unid,
                        expected_endpoint_id,
                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                        0,
                        NULL));

  // Now run the command:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    unlock_with_timeout(expected_unid,
                                        expected_endpoint_id,
                                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                        20,
                                        NULL));

  TEST_ASSERT_EQUAL(ZCL_DOOR_LOCK_LOCK_STATE_UNLOCKED,
                    attribute_store_get_desired_number(lock_state_node));
  attribute_store_set_reported_as_desired(lock_state_node);

  contiki_test_helper_run(1000);
  TEST_ASSERT_EQUAL(ZCL_DOOR_LOCK_LOCK_STATE_UNLOCKED,
                    attribute_store_get_reported_number(lock_state_node));

  contiki_test_helper_run(20000);
  TEST_ASSERT_EQUAL(ZCL_DOOR_LOCK_LOCK_STATE_LOCKED,
                    attribute_store_get_reported_number(lock_state_node));
}