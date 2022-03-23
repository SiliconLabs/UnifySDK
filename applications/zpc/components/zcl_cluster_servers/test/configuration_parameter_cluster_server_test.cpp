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
#include "../src/configuration_parameter_cluster_server.h"

// ZPC Components
#include "attribute_store_defined_attribute_types.h"
#include "zwave_command_class_generic_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Unify components
#include "datastore.h"
#include "attribute_store_fixt.h"
#include "attribute_store.h"
#include "attribute.hpp"

// Interfaces
#include "ucl_definitions.h"
#include "zwave_command_class_version_types.h"
#include "zwave_command_class_configuration_types.h"

using namespace attribute_store;

extern "C" {
#include "unity.h"
#include "dotdot_mqtt_mock.h"

static uic_mqtt_dotdot_configuration_parameters_discover_parameter_callback_t
  configuration_parameters_discover_parameter_callback;
static uic_mqtt_dotdot_configuration_parameters_set_parameter_callback_t
  configuration_parameters_set_parameter_cb_save;
static uic_mqtt_dotdot_configuration_parameters_default_reset_all_parameters_callback_t
  configuration_parameters_default_reset_cb_save;
static unid_t test_unid = "zw-cafecafe-0004";
//static zwave_node_id_t test_node_id         = 0x09;
static zwave_endpoint_id_t test_endpoint_id = 0x03;

// Constants
#define ATTRIBUTE(type)     ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_##type
#define EXPECTED_BASE_TOPIC "ucl/by-unid/zw-CAFECAFE-0004/ep3"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  configuration_parameters_discover_parameter_callback = NULL;
  configuration_parameters_set_parameter_cb_save       = NULL;
  configuration_parameters_default_reset_cb_save       = NULL;
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

static void configuration_parameter_discover_paramter_command_stub(
  const uic_mqtt_dotdot_configuration_parameters_discover_parameter_callback_t
    callback,
  int cmock_num_calls)
{
  configuration_parameters_discover_parameter_callback = callback;
}

static void configuration_paramter_set_paramter_command_stub(
  const uic_mqtt_dotdot_configuration_parameters_set_parameter_callback_t
    callback,
  int cmock_num_calls)
{
  configuration_parameters_set_parameter_cb_save = callback;
}

static void configuration_parameters_default_reset_command_stub(
  const uic_mqtt_dotdot_configuration_parameters_default_reset_all_parameters_callback_t
    callback,
  int cmock_num_calls)
{
  configuration_parameters_default_reset_cb_save = callback;
}
/// Called before each and every test
void setUp()
{
  zpc_attribute_store_test_helper_create_network();
  // Callback stubs for mocking the commands
  uic_mqtt_dotdot_configuration_parameters_discover_parameter_callback_set_Stub(
    configuration_parameter_discover_paramter_command_stub);
  uic_mqtt_dotdot_configuration_parameters_set_parameter_callback_set_Stub(
    configuration_paramter_set_paramter_command_stub);
  uic_mqtt_dotdot_configuration_parameters_default_reset_all_parameters_callback_set_Stub(
    configuration_parameters_default_reset_command_stub);

  configuration_parameter_cluster_server_init();
}

void test_discover_parameter_not_supported()
{
  TEST_ASSERT_NOT_NULL(configuration_parameters_discover_parameter_callback);

  // Version 0 will just return a fail and do nothing
  zwave_cc_version_t version = 0;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(VERSION),
                                     &version,
                                     sizeof(version));
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    configuration_parameters_discover_parameter_callback(
                      test_unid,
                      test_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      1));

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    configuration_parameters_discover_parameter_callback(
                      test_unid,
                      test_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      1));

  // Version > 2 will also return a fail and do nothing
  version = 3;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(VERSION),
                                     &version,
                                     sizeof(version));

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    configuration_parameters_discover_parameter_callback(
                      test_unid,
                      test_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      1));

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    configuration_parameters_discover_parameter_callback(
                      test_unid,
                      test_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      1));
}

void test_discover_parameter_multiple_parameters()
{
  TEST_ASSERT_NOT_NULL(configuration_parameters_discover_parameter_callback);
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(VERSION),
                                     &version,
                                     sizeof(version));

  configuration_parameter_id_t parameter_id_test = 12;
  // Test if the callback is supported by unid/endpoint
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_parameters_discover_parameter_callback(
                      test_unid,
                      test_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      parameter_id_test));

  // Now ask for real.
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_parameters_discover_parameter_callback(
                      test_unid,
                      test_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      parameter_id_test));

  attribute_store_node_t parameters_list_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(PARAMETERS_TO_DISCOVER),
                                             0);

  // It should have saved value 12 in the list:
  TEST_ASSERT_EQUAL(1,
                    attribute_store_get_node_child_count(parameters_list_node));

  // Calling again will not make any modification:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_parameters_discover_parameter_callback(
                      test_unid,
                      test_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      parameter_id_test));
  TEST_ASSERT_EQUAL(1,
                    attribute_store_get_node_child_count(parameters_list_node));

  // Now add a NEXT_PARAMETER_ID attribute, it will get updated with the value
  // as soon as it has a reported value:
  attribute_store_node_t next_parameter_id_node
    = attribute_store_add_node(ATTRIBUTE(NEXT_SUPPORTED_PARAMETER_ID),
                               endpoint_id_node);

  TEST_ASSERT_FALSE(attribute_store_is_value_defined(next_parameter_id_node,
                                                     REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(next_parameter_id_node,
                                                     DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(1,
                    attribute_store_get_node_child_count(parameters_list_node));

  // Define the reported:
  attribute_store_set_reported(next_parameter_id_node,
                               &parameter_id_test,
                               sizeof(parameter_id_test));

  // Now if should have updated the whole thing:
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(next_parameter_id_node,
                                                     REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(next_parameter_id_node,
                                                    DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(0,
                    attribute_store_get_node_child_count(parameters_list_node));
  configuration_parameter_id_t desired_parameter_id = 0;
  attribute_store_get_desired(next_parameter_id_node,
                              &desired_parameter_id,
                              sizeof(desired_parameter_id));
  TEST_ASSERT_EQUAL(parameter_id_test, desired_parameter_id);

  // re-define the reported, nothing will happen as the list to discover is empty:
  attribute_store_set_reported(next_parameter_id_node,
                               &parameter_id_test,
                               sizeof(parameter_id_test));
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(next_parameter_id_node,
                                                    REPORTED_ATTRIBUTE));
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(next_parameter_id_node,
                                                    DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(0,
                    attribute_store_get_node_child_count(parameters_list_node));
}

void test_configuration_parameter_cluster_server_test_set_parm_command()
{
  TEST_ASSERT_NOT_NULL(configuration_parameters_set_parameter_cb_save);
  attribute attr_ep_node(endpoint_id_node);
  attr_ep_node.add_node(ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_VERSION)
    .set_reported<uint8_t>(1);
  uint16_t parameter_id_test   = 12;
  int64_t parameter_value_test = 11;
  // Test if the callback is supported by unid/endpoint
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_parameters_set_parameter_cb_save(
                      test_unid,
                      test_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      parameter_id_test,
                      parameter_value_test));

  // Test if paramter value of a given paramter id attribute value is updated
  attribute test_node_paramter_id
    = attr_ep_node.add_node(ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_ID);
  test_node_paramter_id.set_reported<uint16_t>(parameter_id_test);

  attribute paramter_value_node = test_node_paramter_id.add_node(
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_PARAMETER_VALUE);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_parameters_set_parameter_cb_save(
                      test_unid,
                      test_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      parameter_id_test,
                      parameter_value_test));

  int64_t desired_value = paramter_value_node.desired<int64_t>();

  TEST_ASSERT_EQUAL(parameter_value_test, desired_value);
}

void test_configuration_parameter_cluster_server_test_default_reset_command()
{
  TEST_ASSERT_NOT_NULL(configuration_parameters_default_reset_cb_save);
  attribute attr_ep_node(endpoint_id_node);
  attr_ep_node.add_node(ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_VERSION)
    .set_reported<uint8_t>(4);
  // Test if the callback is supported by unid/endpoint
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_parameters_default_reset_cb_save(
                      test_unid,
                      test_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  attribute default_rest_node = attr_ep_node.add_node(
    ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_DEFAULT_RESET_REQUESTED);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_parameters_default_reset_cb_save(
                      test_unid,
                      test_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_EQUAL(NEEDS_ONE_COMMAND,
                    default_rest_node.desired<command_status_values_t>());
}

void test_configuration_publishing_attributes_on_update_after_network_status_update()
{
  // Create a configuration parameter, but no network status yet. Nothing
  // should happen

  // clang-format off
  attribute endpoint_node(endpoint_id_node);
  endpoint_node.add_node(ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_VERSION).set_reported<zwave_cc_version_t>(1);
  attribute parameter_1_node = endpoint_node.add_node(ATTRIBUTE(PARAMETER_ID)).set_reported<uint16_t>(1);
  parameter_1_node.add_node(ATTRIBUTE(PARAMETER_FORMAT)).set_reported<uint8_t>(4);
  parameter_1_node.add_node(ATTRIBUTE(PARAMETER_NAME)).set_reported<uint8_t>(0);
  parameter_1_node.add_node(ATTRIBUTE(PARAMETER_INFO)).set_reported<uint8_t>(0);
  parameter_1_node.add_node(ATTRIBUTE(PARAMETER_VALUE)).set_reported<int64_t>(0);
  parameter_1_node.add_node(ATTRIBUTE(PARAMETER_MIN_VALUE)).set_reported<int64_t>(0);
  parameter_1_node.add_node(ATTRIBUTE(PARAMETER_MAX_VALUE)).set_reported<int64_t>(0);
  parameter_1_node.add_node(ATTRIBUTE(PARAMETER_DEFAULT_VALUE)).set_reported<int64_t>(0);
  parameter_1_node.add_node(ATTRIBUTE(PARAMETER_ADVANCED)).set_reported<bool>(0);
  parameter_1_node.add_node(ATTRIBUTE(PARAMETER_READ_ONLY)).set_reported<bool>(0);
  parameter_1_node.add_node(ATTRIBUTE(PARAMETER_ALTERING_CAPABILITIES)).set_reported<bool>(0);
  // clang-format on

  // Now add the Network status, the attributes should get published.
  uic_mqtt_dotdot_configuration_parameters_configuration_parameters_publish_ExpectAndReturn(
    EXPECTED_BASE_TOPIC,
    1,
    NULL,
    UCL_MQTT_PUBLISH_TYPE_DESIRED,
    SL_STATUS_OK);
  uic_mqtt_dotdot_configuration_parameters_configuration_parameters_publish_IgnoreArg_value();
  uic_mqtt_dotdot_configuration_parameters_configuration_parameters_publish_ExpectAndReturn(
    EXPECTED_BASE_TOPIC,
    1,
    NULL,
    UCL_MQTT_PUBLISH_TYPE_REPORTED,
    SL_STATUS_OK);
  uic_mqtt_dotdot_configuration_parameters_configuration_parameters_publish_IgnoreArg_value();
  uic_mqtt_dotdot_configuration_parameters_publish_cluster_revision_Expect(
    EXPECTED_BASE_TOPIC,
    1);

  attribute node_id(node_id_node);
  node_id.add_node(ATTRIBUTE_NETWORK_STATUS)
    .set_reported<node_state_topic_state_t>(NODE_STATE_TOPIC_STATE_INCLUDED);

  // Now make the parameter ID broken, it should make the table size of 0:
  uic_mqtt_dotdot_configuration_parameters_configuration_parameters_publish_ExpectAndReturn(
    EXPECTED_BASE_TOPIC,
    0,
    NULL,
    UCL_MQTT_PUBLISH_TYPE_DESIRED,
    SL_STATUS_OK);
  uic_mqtt_dotdot_configuration_parameters_configuration_parameters_publish_IgnoreArg_value();
  uic_mqtt_dotdot_configuration_parameters_configuration_parameters_publish_ExpectAndReturn(
    EXPECTED_BASE_TOPIC,
    0,
    NULL,
    UCL_MQTT_PUBLISH_TYPE_REPORTED,
    SL_STATUS_OK);
  uic_mqtt_dotdot_configuration_parameters_configuration_parameters_publish_IgnoreArg_value();
  attribute_store_set_child_reported(parameter_1_node,
                                     ATTRIBUTE(PARAMETER_VALUE),
                                     NULL,
                                     0);

  // Change back the network status
  node_id.child_by_type(ATTRIBUTE_NETWORK_STATUS)
    .set_reported<node_state_topic_state_t>(NODE_STATE_TOPIC_STATE_OFFLINE);

  // Reconfigure the value, but no publication should happen:
  int64_t new_value = 23;
  attribute_store_set_child_reported(parameter_1_node,
                                     ATTRIBUTE(PARAMETER_VALUE),
                                     &new_value,
                                     sizeof(new_value));
}
}