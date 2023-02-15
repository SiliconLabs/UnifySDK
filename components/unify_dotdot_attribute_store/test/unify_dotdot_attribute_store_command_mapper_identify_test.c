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

uic_mqtt_dotdot_identify_identify_callback_t
  uic_mqtt_dotdot_identify_identify_callback;
uic_mqtt_dotdot_identify_identify_query_callback_t
  uic_mqtt_dotdot_identify_identify_query_callback;
uic_mqtt_dotdot_identify_trigger_effect_callback_t
  uic_mqtt_dotdot_identify_trigger_effect_callback;

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

void test_identify_no_get_endpoint_function_registered()
{
  unify_dotdot_attribute_store_set_configuration(NULL);

  uic_mqtt_dotdot_identify_identify_query_callback
    = get_uic_mqtt_dotdot_identify_identify_query_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_identify_identify_query_callback);

  // Default value for support check is fail:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_identify_identify_query_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  // Default value for normal call is OK:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_identify_identify_query_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
}

void test_identify_identify_query_command_support()
{
  // Configure auto-supported command setup
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.automatic_deduction_of_supported_commands = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Check that the dispatch identify command exists.
  uic_mqtt_dotdot_identify_identify_query_callback
    = get_uic_mqtt_dotdot_identify_identify_query_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_identify_identify_query_callback);

  // First test the support (not supported without Identify attribute)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_identify_identify_query_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));
  // Add the Identify attribute
  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_IDENTIFY_IDENTIFY_TIME,
                           attribute_store_get_root());
  // Now it is supported
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_identify_identify_query_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));
  // Configure for no SupportedCommand support
  test_configuration.automatic_deduction_of_supported_commands = false;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);
  // Now it is not supported, feature is disabled
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_identify_identify_query_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));
}

void test_identify_identify_query_command_clear_reported()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Check that the dispatch identify_query_command exists.
  uic_mqtt_dotdot_identify_identify_query_callback
    = get_uic_mqtt_dotdot_identify_identify_query_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_identify_identify_query_callback);

  // Add the Identify attribute
  attribute_store_node_t identify_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_IDENTIFY_IDENTIFY_TIME,
                               attribute_store_get_root());

  uint16_t value = 0;
  attribute_store_set_desired(identify_node, &value, sizeof(value));

  // Now the desired value gets set to 10 when receiving the Identify Query Command:
  expected_endpoint_id = 4;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_identify_identify_query_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(identify_node));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(identify_node));
  TEST_ASSERT_EQUAL_UINT16(
    value,
    dotdot_get_identify_identify_time(expected_unid,
                                      expected_endpoint_id,
                                      DESIRED_ATTRIBUTE));
}

void test_identify_identify_command_support()
{
  // Configure auto-supported command setup
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.automatic_deduction_of_supported_commands = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Check that the dispatch identify command exists.
  uic_mqtt_dotdot_identify_identify_callback
    = get_uic_mqtt_dotdot_identify_identify_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_identify_identify_callback);

  // First test the support (not supported without Identify attribute)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_identify_identify_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0));

  // Add the Identify attribute
  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_IDENTIFY_IDENTIFY_TIME,
                           attribute_store_get_root());

  // Now it is supported
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_identify_identify_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0));

  // Configure for no SupportedCommand support
  test_configuration.automatic_deduction_of_supported_commands = false;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Now it is not supported, feature is disabled
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_identify_identify_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0));
}

void test_identify_identify_command_update_desired()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Check that the dispatch identify_command exists.
  uic_mqtt_dotdot_identify_identify_callback
    = get_uic_mqtt_dotdot_identify_identify_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_identify_identify_callback);

  uint16_t value = 10;
  // Nothing happens when no Attributes are under the endpoint (root in our test)
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_identify_identify_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      value));
  TEST_ASSERT_EQUAL(
    0,
    attribute_store_get_node_child_count(attribute_store_get_root()));

  // Add the Identify Time attribute
  attribute_store_node_t identify_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_IDENTIFY_IDENTIFY_TIME,
                               attribute_store_get_root());

  attribute_store_set_reported(identify_node, &value, sizeof(value));

  // Now the desired value gets set to 10 when receiving the Identify Command:
  expected_endpoint_id = 4;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_identify_identify_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      value));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(identify_node));
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(identify_node));
  TEST_ASSERT_EQUAL_INT16(
    value,
    dotdot_get_identify_identify_time(expected_unid,
                                      expected_endpoint_id,
                                      DESIRED_ATTRIBUTE));

  // Configure for no desired value update
  test_configuration.update_attribute_desired_values_on_commands = false;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);
  attribute_store_undefine_desired(identify_node);

  // Now it is not supported, feature is disabled
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_identify_identify_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      value));
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(identify_node));
}

void test_identify_identify_command_clear_reported()
{
  test_configuration.get_endpoint_node_function = &test_get_endpoint_node;
  test_configuration.update_attribute_desired_values_on_commands = true;
  test_configuration.clear_reported_on_desired_updates           = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Check that the dispatch identify_command exists.
  uic_mqtt_dotdot_identify_identify_callback
    = get_uic_mqtt_dotdot_identify_identify_callback();
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_identify_identify_callback);

  // Add the Identify attribute
  attribute_store_node_t identify_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_IDENTIFY_IDENTIFY_TIME,
                               attribute_store_get_root());

  uint16_t value = 10;
  attribute_store_set_reported(identify_node, &value, sizeof(value));

  // Now the desired value gets set to 10 when receiving the Identify Command:
  expected_endpoint_id = 23;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_identify_identify_callback(
                      expected_unid,
                      expected_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      value));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(identify_node));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(identify_node));
  TEST_ASSERT_EQUAL_INT16(
    value,
    dotdot_get_identify_identify_time(expected_unid,
                                      expected_endpoint_id,
                                      DESIRED_ATTRIBUTE));
}

void test_identify_publish_no_get_unid_function_registered()
{
  test_configuration.get_unid_endpoint_function       = NULL;
  test_configuration.publish_attribute_values_to_mqtt = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Add an Identify attribute with reported value
  attribute_store_node_t identify_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_IDENTIFY_IDENTIFY_TIME,
                               attribute_store_get_root());
  int16_t value = 10;
  attribute_store_set_reported(identify_node, &value, sizeof(value));

  // No MQTT publications.
}

void test_identify_publish_reported_value()
{
  test_configuration.get_unid_endpoint_function       = &test_get_unid_endpoint;
  test_configuration.publish_attribute_values_to_mqtt = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Add an Identify attribute
  expected_endpoint_id = 4;
  uic_mqtt_count_topics_ExpectAndReturn(
    "ucl/by-unid/" TEST_UNID "/ep4/Identify/Attributes/ClusterRevision",
    0);
  uic_mqtt_dotdot_identify_publish_cluster_revision_Expect(
    "ucl/by-unid/" TEST_UNID "/ep4",
    1);
  attribute_store_node_t identify_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_IDENTIFY_IDENTIFY_TIME,
                               attribute_store_get_root());
  expected_node_id = identify_node;
  int16_t value    = 10;

  // Now the Cluster attribute will get published:
  // No more cluster revision if topics > 0
  uic_mqtt_count_topics_IgnoreAndReturn(1);
  uic_mqtt_dotdot_identify_identify_time_publish_ExpectAndReturn(
    "ucl/by-unid/" TEST_UNID "/ep4",
    value,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  attribute_store_set_reported(identify_node, &value, sizeof(value));
}

void test_identify_publish_desired_value()
{
  test_configuration.get_unid_endpoint_function       = &test_get_unid_endpoint;
  test_configuration.publish_attribute_values_to_mqtt = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Add an Identify attribute with reported value
  expected_endpoint_id = 4;
  uic_mqtt_count_topics_ExpectAndReturn(
    "ucl/by-unid/" TEST_UNID "/ep4/Identify/Attributes/ClusterRevision",
    0);
  uic_mqtt_dotdot_identify_publish_cluster_revision_Expect(
    "ucl/by-unid/" TEST_UNID "/ep4",
    1);
  attribute_store_node_t identify_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_IDENTIFY_IDENTIFY_TIME,
                               attribute_store_get_root());
  expected_node_id = identify_node;
  uint16_t value   = 10;

  // Now the desired value only attribute will get published:
  // No more cluster revision if topics > 0
  uic_mqtt_count_topics_IgnoreAndReturn(1);
  uic_mqtt_dotdot_identify_identify_time_publish_ExpectAndReturn(
    "ucl/by-unid/" TEST_UNID "/ep4",
    value,
    UCL_MQTT_PUBLISH_TYPE_DESIRED,
    SL_STATUS_OK);
  attribute_store_set_desired(identify_node, &value, sizeof(value));

  // And now the reported only:
  value = 15;
  uic_mqtt_dotdot_identify_identify_time_publish_ExpectAndReturn(
    "ucl/by-unid/" TEST_UNID "/ep4",
    value,
    UCL_MQTT_PUBLISH_TYPE_REPORTED,
    SL_STATUS_OK);
  attribute_store_set_reported(identify_node, &value, sizeof(value));
}

void test_identify_unretain_published_values()
{
  test_configuration.get_unid_endpoint_function       = &test_get_unid_endpoint;
  test_configuration.get_endpoint_node_function       = &test_get_endpoint_node;
  test_configuration.publish_attribute_values_to_mqtt = false;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Add an Identify attribute with reported value
  expected_endpoint_id = 32;
  attribute_store_node_t node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_IDENTIFY_IDENTIFY_TIME,
                               attribute_store_get_root());
  uint16_t value = 10;
  attribute_store_set_desired(node, &value, sizeof(value));

  // Enable publishing
  test_configuration.publish_attribute_values_to_mqtt = true;
  unify_dotdot_attribute_store_set_configuration(&test_configuration);

  // Delete an Identify attribute:
  expected_node_id = node;
  uic_mqtt_dotdot_identify_identify_time_unretain_ExpectAndReturn(
    "ucl/by-unid/" TEST_UNID "/ep32",
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_unretain_Expect("ucl/by-unid/" TEST_UNID "/ep32/Identify");
  attribute_store_delete_node(node);
}

void test_identify_time_countdown()
{
  // Create some IdentifyTime attribute
  attribute_store_node_t node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_IDENTIFY_IDENTIFY_TIME,
                               attribute_store_get_root());
  uint16_t value = 10;
  attribute_store_set_desired(node, &value, sizeof(value));
  contiki_test_helper_run(100);
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(node));

  // Set the reported to 10 too
  attribute_store_set_reported_as_desired(node);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(10, attribute_store_get_reported_number(node));

  contiki_test_helper_run(1000);
  TEST_ASSERT_EQUAL(9, attribute_store_get_reported_number(node));

  contiki_test_helper_run(1000);
  TEST_ASSERT_EQUAL(8, attribute_store_get_reported_number(node));

  // Undefine the reported value
  attribute_store_undefine_reported(node);
  contiki_test_helper_run(1000);
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(node));

  // Delete the node to tinker a bit with the attribute transitions module.
  attribute_store_delete_node(node);
  contiki_test_helper_run(100);
}
