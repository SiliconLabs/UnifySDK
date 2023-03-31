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
#include "uic_mqtt_mock.h"

// Generic includes
#include <string.h>

#include "unify_dotdot_attribute_store.h"
#include "unify_dotdot_attribute_store_helpers.h"
#include "unify_dotdot_defined_attribute_types.h"

// Things from unify_dotdot_attribute_store_test_c.zapt
// (unify_dotdot_attribute_store_test.c)

uic_mqtt_dotdot_barrier_control_go_to_percent_callback_t
  uic_mqtt_dotdot_barrier_control_go_to_percent_callback;
uic_mqtt_dotdot_barrier_control_stop_callback_t
  uic_mqtt_dotdot_barrier_control_stop_callback;

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

void test_barrier_control_stop_command_support()
{
  // Configure auto-supported command setup
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.update_attribute_desired_values_on_commands = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Check that the dispatch on_off_command exists.
  uic_mqtt_dotdot_barrier_control_stop_callback
    = get_uic_mqtt_dotdot_barrier_control_stop_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_barrier_control_stop_callback);

  // Add a moving state attribute
  dotdot_create_barrier_control_moving_state(expected_unid,
                                             expected_endpoint_id);
  dotdot_set_barrier_control_moving_state(expected_unid,
                                          expected_endpoint_id,
                                          DESIRED_ATTRIBUTE,
                                          4);

  // Now it is supported
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_barrier_control_stop_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));

  TEST_ASSERT_EQUAL(
    0,
    dotdot_get_barrier_control_moving_state(expected_unid,
                                            expected_endpoint_id,
                                            DESIRED_ATTRIBUTE));

  // Update a little the configuration
  test_configuration.clear_reported_on_desired_updates = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_barrier_control_stop_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
}

void test_barrier_control_go_to_percent_command_support()
{
  // Configure auto-supported command setup
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  test_configuration.update_attribute_desired_values_on_commands = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Check that the dispatch on_off_command exists.
  uic_mqtt_dotdot_barrier_control_go_to_percent_callback
    = get_uic_mqtt_dotdot_barrier_control_go_to_percent_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_barrier_control_go_to_percent_callback);

  // Add a moving state attribute
  dotdot_create_barrier_control_barrier_position(expected_unid,
                                                 expected_endpoint_id);
  dotdot_set_barrier_control_barrier_position(expected_unid,
                                              expected_endpoint_id,
                                              DESIRED_ATTRIBUTE,
                                              45);
  // Try higher than 100%
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_barrier_control_go_to_percent_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      120));

  TEST_ASSERT_EQUAL(
    100,
    dotdot_get_barrier_control_barrier_position(expected_unid,
                                                expected_endpoint_id,
                                                DESIRED_ATTRIBUTE));

  // Try a valid value
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_barrier_control_go_to_percent_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      88));

  TEST_ASSERT_EQUAL(
    88,
    dotdot_get_barrier_control_barrier_position(expected_unid,
                                                expected_endpoint_id,
                                                DESIRED_ATTRIBUTE));

  // Update a little the configuration
  test_configuration.clear_reported_on_desired_updates = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_barrier_control_go_to_percent_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      0));

  TEST_ASSERT_EQUAL(
    0,
    dotdot_get_barrier_control_barrier_position(expected_unid,
                                                expected_endpoint_id,
                                                DESIRED_ATTRIBUTE));
}
