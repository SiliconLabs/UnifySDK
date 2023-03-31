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

uic_mqtt_dotdot_on_off_force_read_attributes_callback_t
  uic_mqtt_dotdot_on_off_force_read_attributes_callback;
uic_mqtt_dotdot_on_off_write_attributes_callback_t
  uic_mqtt_dotdot_on_off_write_attributes_callback;
uic_mqtt_dotdot_on_off_off_callback_t uic_mqtt_dotdot_on_off_off_callback;
uic_mqtt_dotdot_on_off_on_callback_t uic_mqtt_dotdot_on_off_on_callback;
uic_mqtt_dotdot_on_off_toggle_callback_t uic_mqtt_dotdot_on_off_toggle_callback;
uic_mqtt_dotdot_on_off_off_with_effect_callback_t
  uic_mqtt_dotdot_on_off_off_with_effect_callback;
uic_mqtt_dotdot_on_off_on_with_recall_global_scene_callback_t
  uic_mqtt_dotdot_on_off_on_with_recall_global_scene_callback;
uic_mqtt_dotdot_on_off_on_with_timed_off_callback_t
  uic_mqtt_dotdot_on_off_on_with_timed_off_callback;

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

void test_no_get_endpoint_function_registered()
{
  unify_dotdot_attribute_store_set_configuration(NULL);

  // Default value for support check is fail:
  uic_mqtt_dotdot_on_off_on_callback = get_uic_mqtt_dotdot_on_off_on_callback();
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_on_off_on_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  // Default value for normal call is OK:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_on_off_on_callback(expected_unid,
                                       expected_endpoint_id,
                                       UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
}

void test_on_off_on_command_support()
{
  // Configure auto-supported command setup
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.automatic_deduction_of_supported_commands = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Check that the dispatch on_off_command exists.
  uic_mqtt_dotdot_on_off_on_callback = get_uic_mqtt_dotdot_on_off_on_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_on_off_on_callback);

  // First test the support (not supported without OnOff attribute)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_on_off_on_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  // Add the OnOff attribute
  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                           attribute_store_get_root());

  // Now it is supported
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_on_off_on_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  // Configure for no SupportedCommand support
  test_configuration.automatic_deduction_of_supported_commands = false;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Now it is not supported, feature is disabled
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_on_off_on_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));
}

void test_on_off_toggle_command_support()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.automatic_deduction_of_supported_commands = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Check that the dispatch on_off_command exists.
  uic_mqtt_dotdot_on_off_toggle_callback
    = get_uic_mqtt_dotdot_on_off_toggle_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_on_off_toggle_callback);

  // First test the support (not supported without OnOff attribute)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_on_off_toggle_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  // Add the OnOff attribute
  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                           attribute_store_get_root());

  // Now it is supported
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_on_off_toggle_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  // Configure for no SupportedCommand support
  test_configuration.automatic_deduction_of_supported_commands = false;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Now it is not supported, feature is disabled
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_on_off_toggle_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));
}

void test_on_off_on_command_update_desired()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Configure auto-desired update and auto-supported command setup

  // Check that the dispatch on_off_command exists.
  uic_mqtt_dotdot_on_off_on_callback = get_uic_mqtt_dotdot_on_off_on_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_on_off_on_callback);

  // Nothing happens when no Attributes are under the endpoint (root in our test)
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_on_off_on_callback(expected_unid,
                                       expected_endpoint_id,
                                       UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_EQUAL(
    0,
    attribute_store_get_node_child_count(attribute_store_get_root()));

  // Add the OnOff attribute
  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               attribute_store_get_root());
  bool value = true;
  attribute_store_set_reported(on_off_node, &value, sizeof(value));

  // Now the desired value gets set to On when receiving the On Command:
  expected_endpoint_id = 4;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_on_off_on_callback(expected_unid,
                                       expected_endpoint_id,
                                       UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(on_off_node));
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(on_off_node));
  TEST_ASSERT_TRUE(dotdot_get_on_off_on_off(expected_unid,
                                            expected_endpoint_id,
                                            DESIRED_ATTRIBUTE));

  // Configure for no desired value update
  test_configuration.update_attribute_desired_values_on_commands = false;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  attribute_store_undefine_desired(on_off_node);

  // Now it is not supported, feature is disabled
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_on_off_on_callback(expected_unid,
                                       expected_endpoint_id,
                                       UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(on_off_node));
}

void test_on_off_on_command_clear_reported()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Check that the dispatch on_off_command exists.
  uic_mqtt_dotdot_on_off_on_callback = get_uic_mqtt_dotdot_on_off_on_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_on_off_on_callback);

  // Add the OnOff attribute
  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               attribute_store_get_root());
  bool value = true;
  attribute_store_set_reported(on_off_node, &value, sizeof(value));

  // Now the desired value gets set to On when receiving the On Command:
  expected_endpoint_id = 4;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_on_off_on_callback(expected_unid,
                                       expected_endpoint_id,
                                       UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(on_off_node));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(on_off_node));
  TEST_ASSERT_TRUE(dotdot_get_on_off_on_off(expected_unid,
                                            expected_endpoint_id,
                                            DESIRED_ATTRIBUTE));
}

void test_on_off_off_command_update_desired()
{
  // Configure auto-desired update
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Check that the dispatch on_off_command exists.
  uic_mqtt_dotdot_on_off_off_callback
    = get_uic_mqtt_dotdot_on_off_off_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_on_off_off_callback);

  // Nothing happens when no Attributes are under the endpoint (root in our test)
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_on_off_off_callback(expected_unid,
                                        expected_endpoint_id,
                                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_EQUAL(
    0,
    attribute_store_get_node_child_count(attribute_store_get_root()));

  // Add the OnOff attribute
  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               attribute_store_get_root());
  bool value = true;
  attribute_store_set_reported(on_off_node, &value, sizeof(value));

  // Now the desired value gets set to On when receiving the On Command:
  expected_endpoint_id = 4;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_on_off_off_callback(expected_unid,
                                        expected_endpoint_id,
                                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(on_off_node));
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(on_off_node));
  TEST_ASSERT_FALSE(dotdot_get_on_off_on_off(expected_unid,
                                             expected_endpoint_id,
                                             DESIRED_ATTRIBUTE));

  // Configure for no desired value update
  test_configuration.update_attribute_desired_values_on_commands = false;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);
  attribute_store_undefine_desired(on_off_node);

  // Now it is not supported, feature is disabled
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_on_off_off_callback(expected_unid,
                                        expected_endpoint_id,
                                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(on_off_node));
}

void test_on_off_off_command_clear_reported()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Check that the dispatch on_off_command exists.
  uic_mqtt_dotdot_on_off_on_callback = get_uic_mqtt_dotdot_on_off_on_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_on_off_on_callback);

  // Add the OnOff attribute
  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               attribute_store_get_root());
  bool value = true;
  attribute_store_set_reported(on_off_node, &value, sizeof(value));

  // Now the desired value gets set to On when receiving the On Command:
  expected_endpoint_id = 4;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_on_off_off_callback(expected_unid,
                                        expected_endpoint_id,
                                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(on_off_node));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(on_off_node));
  TEST_ASSERT_FALSE(dotdot_get_on_off_on_off(expected_unid,
                                             expected_endpoint_id,
                                             DESIRED_ATTRIBUTE));
}

void test_on_off_toggle_command_update_desired()
{
  // Configure auto-desired update
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Check that the dispatch on_off_command exists.
  uic_mqtt_dotdot_on_off_toggle_callback
    = get_uic_mqtt_dotdot_on_off_toggle_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_on_off_toggle_callback);

  // Nothing happens when no Attributes are under the endpoint (root in our test)
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_on_off_toggle_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_EQUAL(
    0,
    attribute_store_get_node_child_count(attribute_store_get_root()));

  // Add the OnOff attribute
  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               attribute_store_get_root());

  // Now the desired value gets set to Off when receiving the Toggle Command:
  expected_endpoint_id = 34;
  dotdot_set_on_off_on_off(expected_unid,
                           expected_endpoint_id,
                           REPORTED_ATTRIBUTE,
                           true);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_on_off_toggle_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(on_off_node));
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(on_off_node));
  TEST_ASSERT_FALSE(dotdot_get_on_off_on_off(expected_unid,
                                             expected_endpoint_id,
                                             DESIRED_ATTRIBUTE));

  dotdot_set_on_off_on_off(expected_unid,
                           expected_endpoint_id,
                           REPORTED_ATTRIBUTE,
                           false);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_on_off_toggle_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(on_off_node));
  TEST_ASSERT_TRUE(dotdot_get_on_off_on_off(expected_unid,
                                            expected_endpoint_id,
                                            DESIRED_ATTRIBUTE));

  // Configure for no desired value update
  test_configuration.update_attribute_desired_values_on_commands = false;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  attribute_store_undefine_desired(on_off_node);

  // Now it is not supported, feature is disabled
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_on_off_toggle_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(on_off_node));
}

void test_on_off_toggle_command_clear_reported()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Check that the dispatch on_off_command exists.
  uic_mqtt_dotdot_on_off_on_callback = get_uic_mqtt_dotdot_on_off_on_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_on_off_on_callback);

  // Add the OnOff attribute
  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               attribute_store_get_root());
  bool value = true;
  attribute_store_set_reported(on_off_node, &value, sizeof(value));

  // Now the desired value gets set to On when receiving the On Command:
  expected_endpoint_id = 23;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_on_off_toggle_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(on_off_node));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(on_off_node));
  TEST_ASSERT_FALSE(dotdot_get_on_off_on_off(expected_unid,
                                             expected_endpoint_id,
                                             DESIRED_ATTRIBUTE));
}

void test_publish_no_get_unid_function_registered()
{
  test_configuration.get_unid_endpoint_function       = NULL;
  test_configuration.publish_attribute_values_to_mqtt = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Add an OnOff attribute with reported value
  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               attribute_store_get_root());
  bool value = false;
  attribute_store_set_reported(on_off_node, &value, sizeof(value));

  // No MQTT publications.
}

void test_publish_reported_value()
{
  test_configuration.get_unid_endpoint_function       = &test_get_unid_endpoint;
  test_configuration.publish_attribute_values_to_mqtt = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Add an OnOff attribute
  expected_endpoint_id = 4;
  uic_mqtt_count_topics_ExpectAndReturn("ucl/by-unid/" TEST_UNID
                                        "/ep4/OnOff/Attributes/ClusterRevision",
                                        0);
  uic_mqtt_dotdot_on_off_publish_cluster_revision_Expect(
    "ucl/by-unid/" TEST_UNID "/ep4",
    2);
  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               attribute_store_get_root());
  expected_node_id = on_off_node;
  bool value       = false;

  // Now the Cluster attribute will get published:
  // No more cluster revision if topics > 0
  uic_mqtt_count_topics_IgnoreAndReturn(1);
  uic_mqtt_dotdot_on_off_on_off_publish_ExpectAndReturn(
    "ucl/by-unid/" TEST_UNID "/ep4",
    value,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  attribute_store_set_reported(on_off_node, &value, sizeof(value));
}

void test_publish_desired_value()
{
  test_configuration.get_unid_endpoint_function       = &test_get_unid_endpoint;
  test_configuration.publish_attribute_values_to_mqtt = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Add an OnOff attribute with reported value
  expected_endpoint_id = 4;
  uic_mqtt_count_topics_ExpectAndReturn("ucl/by-unid/" TEST_UNID
                                        "/ep4/OnOff/Attributes/ClusterRevision",
                                        0);
  uic_mqtt_dotdot_on_off_publish_cluster_revision_Expect(
    "ucl/by-unid/" TEST_UNID "/ep4",
    2);
  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               attribute_store_get_root());
  expected_node_id = on_off_node;
  bool value       = false;

  // Now the desired value only attribute will get published:
  // No more cluster revision if topics > 0
  uic_mqtt_count_topics_IgnoreAndReturn(1);
  uic_mqtt_dotdot_on_off_on_off_publish_ExpectAndReturn(
    "ucl/by-unid/" TEST_UNID "/ep4",
    value,
    UCL_MQTT_PUBLISH_TYPE_DESIRED,
    SL_STATUS_OK);
  attribute_store_set_desired(on_off_node, &value, sizeof(value));

  // And now the reported only:
  value = true;
  uic_mqtt_dotdot_on_off_on_off_publish_ExpectAndReturn(
    "ucl/by-unid/" TEST_UNID "/ep4",
    value,
    UCL_MQTT_PUBLISH_TYPE_REPORTED,
    SL_STATUS_OK);
  attribute_store_set_reported(on_off_node, &value, sizeof(value));
}

void test_unretain_published_values()
{
  test_configuration.get_unid_endpoint_function       = &test_get_unid_endpoint;
  test_configuration.get_endpoint_node_function       = &test_get_endpoint_node;
  test_configuration.publish_attribute_values_to_mqtt = false;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Add an OnOff attribute with reported value
  expected_endpoint_id = 32;
  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               attribute_store_get_root());
  bool value = false;
  attribute_store_set_desired(on_off_node, &value, sizeof(value));

  attribute_store_node_t on_off_global_scene_control_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_GLOBAL_SCENE_CONTROL,
                               attribute_store_get_root());

  // Enable publishing
  test_configuration.publish_attribute_values_to_mqtt = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Delete an OnOff attribute:
  expected_node_id = on_off_node;
  uic_mqtt_dotdot_on_off_on_off_unretain_ExpectAndReturn(
    "ucl/by-unid/" TEST_UNID "/ep32",
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  attribute_store_delete_node(on_off_node);

  expected_node_id = on_off_global_scene_control_node;
  uic_mqtt_dotdot_on_off_global_scene_control_unretain_ExpectAndReturn(
    "ucl/by-unid/" TEST_UNID "/ep32",
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_unretain_Expect("ucl/by-unid/" TEST_UNID "/ep32/OnOff");
  attribute_store_delete_node(on_off_global_scene_control_node);
}

void test_on_off_off_command_support()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.automatic_deduction_of_supported_commands   = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Check that the dispatch on_off_command exists.
  uic_mqtt_dotdot_on_off_off_callback
    = get_uic_mqtt_dotdot_on_off_off_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_on_off_off_callback);

  // First test the support (not supported without OnOff attribute)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_on_off_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  // Add the OnOff attribute
  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                           attribute_store_get_root());

  // Now it is supported
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_on_off_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  // Configure for no SupportedCommand support
  test_configuration.automatic_deduction_of_supported_commands = false;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);
  // Now it is not supported, feature is disabled
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_on_off_off_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));
}

void test_on_off_off_command_update_desired_with_level_supported()
{
  // Configure auto-desired update
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Check that the dispatch on_off_command exists.
  uic_mqtt_dotdot_on_off_off_callback
    = get_uic_mqtt_dotdot_on_off_off_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_on_off_off_callback);

  // Add the OnOff attribute
  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               attribute_store_get_root());
  bool value = true;
  attribute_store_set_reported(on_off_node, &value, sizeof(value));

  // Add a level capability
  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
                           attribute_store_get_root());
  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_ON_OFF_TRANSITION_TIME,
                           attribute_store_get_root());

  // Now the desired value gets set to On when receiving the On Command:
  expected_endpoint_id = 4;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_on_off_off_callback(expected_unid,
                                        expected_endpoint_id,
                                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(on_off_node));
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(on_off_node));
  TEST_ASSERT_FALSE(dotdot_get_on_off_on_off(expected_unid,
                                             expected_endpoint_id,
                                             DESIRED_ATTRIBUTE));

  // Check the level got moved down to 0:
  TEST_ASSERT_EQUAL(0,
                    dotdot_get_level_current_level(expected_unid,
                                                   expected_endpoint_id,
                                                   DESIRED_ATTRIBUTE));
}

void test_on_off_on_command_update_desired_with_level_supported_on_level()
{
  // Configure auto-desired update
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Check that the dispatch on_off_command exists.
  uic_mqtt_dotdot_on_off_on_callback = get_uic_mqtt_dotdot_on_off_on_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_on_off_on_callback);

  // Add the OnOff attribute
  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               attribute_store_get_root());
  bool value = false;
  attribute_store_set_reported(on_off_node, &value, sizeof(value));

  // Add a level capability
  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
                           attribute_store_get_root());
  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_ON_OFF_TRANSITION_TIME,
                           attribute_store_get_root());

  // Configure the OnLevel to 33:
  dotdot_create_level_on_level(expected_unid, expected_endpoint_id);
  dotdot_set_level_on_level(expected_unid,
                            expected_endpoint_id,
                            REPORTED_ATTRIBUTE,
                            33);

  // Now the desired value gets set to On when receiving the On Command:
  expected_endpoint_id = 4;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_on_off_on_callback(expected_unid,
                                       expected_endpoint_id,
                                       UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(on_off_node));
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(on_off_node));
  TEST_ASSERT_TRUE(dotdot_get_on_off_on_off(expected_unid,
                                            expected_endpoint_id,
                                            DESIRED_ATTRIBUTE));

  // Check the level got moved to 33:
  TEST_ASSERT_EQUAL(33,
                    dotdot_get_level_current_level(expected_unid,
                                                   expected_endpoint_id,
                                                   DESIRED_ATTRIBUTE));
}

void test_on_off_on_command_update_desired_with_level_supported_last_non_zero_level()
{
  // Configure auto-desired update
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Check that the dispatch on_off_command exists.
  uic_mqtt_dotdot_on_off_on_callback = get_uic_mqtt_dotdot_on_off_on_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_on_off_on_callback);

  // Add the OnOff attribute
  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               attribute_store_get_root());
  bool value = false;
  attribute_store_set_reported(on_off_node, &value, sizeof(value));

  // Add a level capability
  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
                           attribute_store_get_root());
  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_ON_OFF_TRANSITION_TIME,
                           attribute_store_get_root());

  // Configure the Last non zero level to 66:
  attribute_store_node_t last_non_zero_level_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL_LAST_NON_ZERO_VALUE,
    attribute_store_get_root());
  attribute_store_set_reported_number(last_non_zero_level_node, 66);

  // Now the desired value gets set to On when receiving the On Command:
  expected_endpoint_id = 4;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_on_off_on_callback(expected_unid,
                                       expected_endpoint_id,
                                       UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(on_off_node));
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(on_off_node));
  TEST_ASSERT_TRUE(dotdot_get_on_off_on_off(expected_unid,
                                            expected_endpoint_id,
                                            DESIRED_ATTRIBUTE));

  // Check the level got moved to 66:
  TEST_ASSERT_EQUAL(66,
                    dotdot_get_level_current_level(expected_unid,
                                                   expected_endpoint_id,
                                                   DESIRED_ATTRIBUTE));
}
