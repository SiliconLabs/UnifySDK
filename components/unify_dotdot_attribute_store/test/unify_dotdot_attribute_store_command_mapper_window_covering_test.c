/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

// Generic includes
#include <string.h>

#include "unify_dotdot_attribute_store.h"
#include "unify_dotdot_attribute_store_helpers.h"
#include "unify_dotdot_defined_attribute_types.h"

extern dotdot_unid_t expected_unid;
extern dotdot_endpoint_id_t expected_endpoint_id;
extern attribute_store_node_t expected_node_id;
extern unify_dotdot_attribute_store_configuration_t test_configuration;
attribute_store_node_t test_get_endpoint_node(const dotdot_unid_t unid,
                                              dotdot_endpoint_id_t endpoint_id);
sl_status_t test_get_unid_endpoint(attribute_store_node_t node,
                                   char *unid,
                                   dotdot_endpoint_id_t *endpoint_id);

void test_window_covering_up_or_open_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_window_covering_up_or_open_callback_t up_or_open_command
    = get_uic_mqtt_dotdot_window_covering_up_or_open_callback();
  TEST_ASSERT_NOT_NULL(up_or_open_command);

  // Default value for support check is fail:
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    up_or_open_command(expected_unid,
                       expected_endpoint_id,
                       UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  dotdot_create_window_covering_window_covering_type(expected_unid,
                                                     expected_endpoint_id);

  // Now supported
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    up_or_open_command(expected_unid,
                       expected_endpoint_id,
                       UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  // Now create attribute and invoke the command
  dotdot_create_window_covering_config_or_status(expected_unid,
                                                 expected_endpoint_id);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    up_or_open_command(expected_unid,
                                       expected_endpoint_id,
                                       UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  dotdot_set_window_covering_config_or_status(
    expected_unid,
    expected_endpoint_id,
    REPORTED_ATTRIBUTE,
    WINDOW_COVERING_CONFIG_OR_STATUS_OPERATIONAL);
  dotdot_create_window_covering_current_position_lift(expected_unid,
                                                      expected_endpoint_id);
  dotdot_create_window_covering_current_position_lift_percentage(
    expected_unid,
    expected_endpoint_id);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    up_or_open_command(expected_unid,
                                       expected_endpoint_id,
                                       UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));

  TEST_ASSERT_FALSE(
    dotdot_window_covering_current_position_lift_is_reported_defined(
      expected_unid,
      expected_endpoint_id));
  TEST_ASSERT_FALSE(
    dotdot_window_covering_current_position_lift_percentage_is_reported_defined(
      expected_unid,
      expected_endpoint_id));
  TEST_ASSERT_EQUAL(
    0,
    dotdot_get_window_covering_current_position_lift(expected_unid,
                                                     expected_endpoint_id,
                                                     DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(0,
                    dotdot_get_window_covering_current_position_lift_percentage(
                      expected_unid,
                      expected_endpoint_id,
                      DESIRED_ATTRIBUTE));

  dotdot_set_window_covering_window_covering_type(
    expected_unid,
    expected_endpoint_id,
    REPORTED_ATTRIBUTE,
    ZCL_WINDOW_COVERING_WINDOW_COVERING_TYPE_TILT_BLIND_LIFT_AND_TILT);
  dotdot_create_window_covering_current_position_tilt(expected_unid,
                                                      expected_endpoint_id);
  dotdot_create_window_covering_current_position_tilt_percentage(expected_unid,
                                                          expected_endpoint_id);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    up_or_open_command(expected_unid,
                                       expected_endpoint_id,
                                       UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_FALSE(
    dotdot_window_covering_current_position_tilt_is_reported_defined(
      expected_unid,
      expected_endpoint_id));
  TEST_ASSERT_FALSE(
    dotdot_window_covering_current_position_tilt_percentage_is_reported_defined(
      expected_unid,
      expected_endpoint_id));
  TEST_ASSERT_EQUAL(
    0,
    dotdot_get_window_covering_current_position_tilt(expected_unid,
                                                     expected_endpoint_id,
                                                     DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(0,
                    dotdot_get_window_covering_current_position_tilt_percentage(
                      expected_unid,
                      expected_endpoint_id,
                      DESIRED_ATTRIBUTE));
}

void test_window_covering_down_or_close_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_window_covering_down_or_close_callback_t down_or_close_command
    = get_uic_mqtt_dotdot_window_covering_down_or_close_callback();
  TEST_ASSERT_NOT_NULL(down_or_close_command);

  // Default value for support check is fail:
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    down_or_close_command(expected_unid,
                          expected_endpoint_id,
                          UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  dotdot_create_window_covering_window_covering_type(expected_unid,
                                                     expected_endpoint_id);

  // Now supported
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    down_or_close_command(expected_unid,
                          expected_endpoint_id,
                          UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  // Now create attribute and invoke the command
  dotdot_create_window_covering_config_or_status(expected_unid,
                                                 expected_endpoint_id);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    down_or_close_command(expected_unid,
                          expected_endpoint_id,
                          UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
                          
  dotdot_set_window_covering_config_or_status(
    expected_unid,
    expected_endpoint_id,
    REPORTED_ATTRIBUTE,
    WINDOW_COVERING_CONFIG_OR_STATUS_OPERATIONAL);
  dotdot_create_window_covering_current_position_lift(expected_unid,
                                                      expected_endpoint_id);
  dotdot_create_window_covering_current_position_lift_percentage(
    expected_unid,
    expected_endpoint_id);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    down_or_close_command(expected_unid,
                          expected_endpoint_id,
                          UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));

  TEST_ASSERT_FALSE(
    dotdot_window_covering_current_position_lift_is_reported_defined(
      expected_unid,
      expected_endpoint_id));
  TEST_ASSERT_FALSE(
    dotdot_window_covering_current_position_lift_percentage_is_reported_defined(
      expected_unid,
      expected_endpoint_id));
  TEST_ASSERT_EQUAL(
    65535,
    dotdot_get_window_covering_current_position_lift(expected_unid,
                                                     expected_endpoint_id,
                                                     DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(100,
                    dotdot_get_window_covering_current_position_lift_percentage(
                      expected_unid,
                      expected_endpoint_id,
                      DESIRED_ATTRIBUTE));
  
  dotdot_set_window_covering_window_covering_type(
    expected_unid,
    expected_endpoint_id,
    REPORTED_ATTRIBUTE,
    ZCL_WINDOW_COVERING_WINDOW_COVERING_TYPE_TILT_BLIND_LIFT_AND_TILT);
  dotdot_create_window_covering_current_position_tilt(expected_unid,
                                                      expected_endpoint_id);
  dotdot_create_window_covering_current_position_tilt_percentage(expected_unid,
                                                          expected_endpoint_id);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    down_or_close_command(expected_unid,
                                       expected_endpoint_id,
                                       UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_FALSE(
    dotdot_window_covering_current_position_tilt_is_reported_defined(
      expected_unid,
      expected_endpoint_id));
  TEST_ASSERT_FALSE(
    dotdot_window_covering_current_position_tilt_percentage_is_reported_defined(
      expected_unid,
      expected_endpoint_id));
  TEST_ASSERT_EQUAL(
    65535,
    dotdot_get_window_covering_current_position_tilt(expected_unid,
                                                     expected_endpoint_id,
                                                     DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(100,
                    dotdot_get_window_covering_current_position_tilt_percentage(
                      expected_unid,
                      expected_endpoint_id,
                      DESIRED_ATTRIBUTE));
}

void test_window_covering_stop_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_window_covering_stop_callback_t stop_command
    = get_uic_mqtt_dotdot_window_covering_stop_callback();
  TEST_ASSERT_NOT_NULL(stop_command);

  // Default value for support check is fail:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    stop_command(expected_unid,
                                 expected_endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  dotdot_create_window_covering_window_covering_type(expected_unid,
                                                     expected_endpoint_id);

  // Now supported
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    stop_command(expected_unid,
                                 expected_endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  // Now create attribute and invoke the command
  dotdot_create_window_covering_current_position_lift(expected_unid,
                                                      expected_endpoint_id);
  dotdot_create_window_covering_current_position_lift_percentage(
    expected_unid,
    expected_endpoint_id);
  dotdot_create_window_covering_current_position_tilt(expected_unid,
                                                      expected_endpoint_id);
  dotdot_create_window_covering_current_position_tilt_percentage(
    expected_unid,
    expected_endpoint_id);
  dotdot_set_window_covering_current_position_lift(expected_unid,
                                                   expected_endpoint_id,
                                                   REPORTED_ATTRIBUTE,
                                                   42);
  dotdot_set_window_covering_current_position_lift_percentage(
    expected_unid,
    expected_endpoint_id,
    REPORTED_ATTRIBUTE,
    42);
  dotdot_set_window_covering_current_position_tilt(expected_unid,
                                                   expected_endpoint_id,
                                                   REPORTED_ATTRIBUTE,
                                                   42);
  dotdot_set_window_covering_current_position_tilt_percentage(
    expected_unid,
    expected_endpoint_id,
    REPORTED_ATTRIBUTE,
    42);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    stop_command(expected_unid,
                                 expected_endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));

  TEST_ASSERT_FALSE(
    dotdot_window_covering_current_position_lift_is_reported_defined(
      expected_unid,
      expected_endpoint_id));
  TEST_ASSERT_FALSE(
    dotdot_window_covering_current_position_lift_percentage_is_reported_defined(
      expected_unid,
      expected_endpoint_id));
  TEST_ASSERT_FALSE(
    dotdot_window_covering_current_position_tilt_is_reported_defined(
      expected_unid,
      expected_endpoint_id));
  TEST_ASSERT_FALSE(
    dotdot_window_covering_current_position_tilt_percentage_is_reported_defined(
      expected_unid,
      expected_endpoint_id));

  TEST_ASSERT_EQUAL(
    42,
    dotdot_get_window_covering_current_position_lift(expected_unid,
                                                     expected_endpoint_id,
                                                     DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(42,
                    dotdot_get_window_covering_current_position_lift_percentage(
                      expected_unid,
                      expected_endpoint_id,
                      DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(
    42,
    dotdot_get_window_covering_current_position_tilt(expected_unid,
                                                     expected_endpoint_id,
                                                     DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(42,
                    dotdot_get_window_covering_current_position_tilt_percentage(
                      expected_unid,
                      expected_endpoint_id,
                      DESIRED_ATTRIBUTE));
}

void test_window_covering_go_to_lift_value_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_window_covering_go_to_lift_value_callback_t
    go_to_lift_value_command
    = get_uic_mqtt_dotdot_window_covering_go_to_lift_value_callback();
  TEST_ASSERT_NOT_NULL(go_to_lift_value_command);

  // Default value for support check is fail:
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    go_to_lift_value_command(expected_unid,
                             expected_endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                             0));

  dotdot_create_window_covering_current_position_lift(expected_unid,
                                                      expected_endpoint_id);

  // Now supported
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    go_to_lift_value_command(expected_unid,
                             expected_endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                             0));

  dotdot_create_window_covering_config_or_status(expected_unid,
                                                 expected_endpoint_id);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    go_to_lift_value_command(expected_unid,
                             expected_endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                             42));
  dotdot_set_window_covering_config_or_status(
    expected_unid,
    expected_endpoint_id,
    REPORTED_ATTRIBUTE,
    WINDOW_COVERING_CONFIG_OR_STATUS_OPERATIONAL);
  
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    go_to_lift_value_command(expected_unid,
                             expected_endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                             42));

  TEST_ASSERT_FALSE(
    dotdot_window_covering_current_position_lift_is_reported_defined(
      expected_unid,
      expected_endpoint_id));

  TEST_ASSERT_EQUAL(
    42,
    dotdot_get_window_covering_current_position_lift(expected_unid,
                                                     expected_endpoint_id,
                                                     DESIRED_ATTRIBUTE));
}

void test_window_covering_go_to_lift_percentage_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_window_covering_go_to_lift_percentage_callback_t
    go_to_lift_percentage_command
    = get_uic_mqtt_dotdot_window_covering_go_to_lift_percentage_callback();
  TEST_ASSERT_NOT_NULL(go_to_lift_percentage_command);

  // Default value for support check is fail:
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    go_to_lift_percentage_command(expected_unid,
                                  expected_endpoint_id,
                                  UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                  0));

  dotdot_create_window_covering_current_position_lift_percentage(
    expected_unid,
    expected_endpoint_id);
  dotdot_create_window_covering_current_position_lift(
    expected_unid,
    expected_endpoint_id);

  // Now supported
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    go_to_lift_percentage_command(expected_unid,
                                  expected_endpoint_id,
                                  UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                  0));

  dotdot_create_window_covering_config_or_status(expected_unid,
                                                 expected_endpoint_id);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    go_to_lift_percentage_command(expected_unid,
                                  expected_endpoint_id,
                                  UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                  50));
  dotdot_set_window_covering_config_or_status(
    expected_unid,
    expected_endpoint_id,
    REPORTED_ATTRIBUTE,
    WINDOW_COVERING_CONFIG_OR_STATUS_OPERATIONAL);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    go_to_lift_percentage_command(expected_unid,
                                  expected_endpoint_id,
                                  UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                  50));

  TEST_ASSERT_FALSE(
    dotdot_window_covering_current_position_lift_percentage_is_reported_defined(
      expected_unid,
      expected_endpoint_id));

  TEST_ASSERT_EQUAL(50,
                    dotdot_get_window_covering_current_position_lift_percentage(
                      expected_unid,
                      expected_endpoint_id,
                      DESIRED_ATTRIBUTE));
}

void test_window_covering_go_to_tilt_value_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_window_covering_go_to_tilt_value_callback_t
    go_to_tilt_value_command
    = get_uic_mqtt_dotdot_window_covering_go_to_tilt_value_callback();
  TEST_ASSERT_NOT_NULL(go_to_tilt_value_command);

  // Default value for support check is fail:
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    go_to_tilt_value_command(expected_unid,
                             expected_endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                             0));

  dotdot_create_window_covering_current_position_tilt(expected_unid,
                                                      expected_endpoint_id);

  // Now supported
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    go_to_tilt_value_command(expected_unid,
                             expected_endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                             0));

  dotdot_create_window_covering_config_or_status(expected_unid,
                                                 expected_endpoint_id);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    go_to_tilt_value_command(expected_unid,
                             expected_endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                             42));                                               
  dotdot_set_window_covering_config_or_status(
    expected_unid,
    expected_endpoint_id,
    REPORTED_ATTRIBUTE,
    WINDOW_COVERING_CONFIG_OR_STATUS_OPERATIONAL);
  dotdot_create_window_covering_window_covering_type(expected_unid,
                                                 expected_endpoint_id);
  dotdot_set_window_covering_window_covering_type(
    expected_unid,
    expected_endpoint_id,
    REPORTED_ATTRIBUTE,
    ZCL_WINDOW_COVERING_WINDOW_COVERING_TYPE_SHUTTER);
  
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    go_to_tilt_value_command(expected_unid,
                             expected_endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                             42));

  TEST_ASSERT_FALSE(
    dotdot_window_covering_current_position_tilt_is_reported_defined(
      expected_unid,
      expected_endpoint_id));

  TEST_ASSERT_EQUAL(
    42,
    dotdot_get_window_covering_current_position_tilt(expected_unid,
                                                     expected_endpoint_id,
                                                     DESIRED_ATTRIBUTE));
}

void test_window_covering_go_to_tilt_percentage_command()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  uic_mqtt_dotdot_window_covering_go_to_tilt_percentage_callback_t
    go_to_tilt_percentage_command
    = get_uic_mqtt_dotdot_window_covering_go_to_tilt_percentage_callback();
  TEST_ASSERT_NOT_NULL(go_to_tilt_percentage_command);

  // Default value for support check is fail:
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    go_to_tilt_percentage_command(expected_unid,
                                  expected_endpoint_id,
                                  UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                  0));

  dotdot_create_window_covering_current_position_tilt_percentage(
    expected_unid,
    expected_endpoint_id);
  dotdot_create_window_covering_current_position_tilt(
    expected_unid,
    expected_endpoint_id);

  // Now supported
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    go_to_tilt_percentage_command(expected_unid,
                                  expected_endpoint_id,
                                  UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                  0));

  dotdot_create_window_covering_config_or_status(expected_unid,
                                                 expected_endpoint_id);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    go_to_tilt_percentage_command(expected_unid,
                                  expected_endpoint_id,
                                  UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                  50));
  dotdot_set_window_covering_config_or_status(
    expected_unid,
    expected_endpoint_id,
    REPORTED_ATTRIBUTE,
    WINDOW_COVERING_CONFIG_OR_STATUS_OPERATIONAL);
  dotdot_create_window_covering_window_covering_type(expected_unid,
                                                 expected_endpoint_id);
  dotdot_set_window_covering_window_covering_type(
    expected_unid,
    expected_endpoint_id,
    REPORTED_ATTRIBUTE,
    ZCL_WINDOW_COVERING_WINDOW_COVERING_TYPE_SHUTTER);
  
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    go_to_tilt_percentage_command(expected_unid,
                                  expected_endpoint_id,
                                  UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                  50));

  TEST_ASSERT_FALSE(
    dotdot_window_covering_current_position_tilt_percentage_is_reported_defined(
      expected_unid,
      expected_endpoint_id));

  TEST_ASSERT_EQUAL(50,
                    dotdot_get_window_covering_current_position_tilt_percentage(
                      expected_unid,
                      expected_endpoint_id,
                      DESIRED_ATTRIBUTE));
}

attribute_store_node_t test_wc_get_endpoint_node(const dotdot_unid_t unid,
                                              dotdot_endpoint_id_t endpoint_id)
{
  return attribute_store_get_first_child_by_type(attribute_store_get_root(), 0x1);
}

void test_window_covering_on_mode_update()
{
  test_configuration.get_endpoint_node_function = &test_wc_get_endpoint_node;
  test_configuration.endpoint_type = 0x1;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  attribute_store_add_node(0x1, attribute_store_get_root());

  dotdot_create_window_covering_config_or_status(expected_unid, expected_endpoint_id);
  dotdot_create_window_covering_mode(expected_unid, expected_endpoint_id);
  dotdot_set_window_covering_config_or_status(expected_unid, expected_endpoint_id, REPORTED_ATTRIBUTE, 0);
  dotdot_set_window_covering_mode(expected_unid, expected_endpoint_id, DESIRED_ATTRIBUTE, WINDOW_COVERING_MODE_MOTOR_DIRECTION_REVERSED);
  TEST_ASSERT_BIT_HIGH(WINDOW_COVERING_CONFIG_OR_STATUS_OPEN_AND_UP_COMMANDS_REVERSED_OFFSET, 
    dotdot_get_window_covering_config_or_status(expected_unid, expected_endpoint_id, DESIRED_ATTRIBUTE));

  dotdot_set_window_covering_config_or_status(expected_unid, expected_endpoint_id, REPORTED_ATTRIBUTE, WINDOW_COVERING_CONFIG_OR_STATUS_OPEN_AND_UP_COMMANDS_REVERSED);
  dotdot_set_window_covering_mode(expected_unid, expected_endpoint_id, DESIRED_ATTRIBUTE, 0);
  TEST_ASSERT_BIT_LOW(WINDOW_COVERING_CONFIG_OR_STATUS_OPEN_AND_UP_COMMANDS_REVERSED_OFFSET, 
    dotdot_get_window_covering_config_or_status(expected_unid, expected_endpoint_id, DESIRED_ATTRIBUTE));

  dotdot_set_window_covering_config_or_status(expected_unid, expected_endpoint_id, REPORTED_ATTRIBUTE, WINDOW_COVERING_CONFIG_OR_STATUS_OPERATIONAL);
  dotdot_set_window_covering_mode(expected_unid, expected_endpoint_id, DESIRED_ATTRIBUTE, WINDOW_COVERING_MODE_CALIBRATION_MODE);
  TEST_ASSERT_BIT_LOW(WINDOW_COVERING_CONFIG_OR_STATUS_OPERATIONAL_OFFSET, 
    dotdot_get_window_covering_config_or_status(expected_unid, expected_endpoint_id, DESIRED_ATTRIBUTE));

  dotdot_set_window_covering_config_or_status(expected_unid, expected_endpoint_id, REPORTED_ATTRIBUTE, 0);
  dotdot_set_window_covering_mode(expected_unid, expected_endpoint_id, DESIRED_ATTRIBUTE, 0);
  TEST_ASSERT_BIT_HIGH(WINDOW_COVERING_CONFIG_OR_STATUS_OPERATIONAL_OFFSET, 
    dotdot_get_window_covering_config_or_status(expected_unid, expected_endpoint_id, DESIRED_ATTRIBUTE));
  
}
