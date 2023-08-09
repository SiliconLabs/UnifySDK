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
#include "unity.h"
#include "configuration_parameter_cluster_server.h"

// ZPC Components
#include "attribute_store_defined_attribute_types.h"
#include "zwave_command_class_generic_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Unify components
#include "datastore.h"
#include "attribute_store_fixt.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "unify_dotdot_attribute_store_node_state.h"
#include "sl_log.h"

// Interfaces

#include "zwave_command_class_version_types.h"
#include "zwave_command_class_configuration_types.h"

// Mocks
#include "dotdot_mqtt_mock.h"
#include "zwave_network_management_mock.h"

static uic_mqtt_dotdot_configuration_parameters_discover_parameter_callback_t
  configuration_parameters_discover_parameter_callback;
static uic_mqtt_dotdot_configuration_parameters_set_parameter_callback_t
  configuration_parameters_set_parameter_cb_save;
static uic_mqtt_dotdot_configuration_parameters_default_reset_all_parameters_callback_t
  configuration_parameters_default_reset_cb_save;
static uic_mqtt_dotdot_configuration_parameters_discover_parameter_range_callback_t
  discover_parameter_range_callback;
static unid_t test_unid = "zw-cafecafe-0004";
//static zwave_node_id_t test_node_id         = 0x09;
static zwave_endpoint_id_t test_endpoint_id = 0x03;

// Constants
#define ATTRIBUTE(type)     ATTRIBUTE_COMMAND_CLASS_CONFIGURATION_##type
#define EXPECTED_BASE_TOPIC "ucl/by-unid/zw-CAFECAFE-0004/ep3"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

static void configuration_parameter_discover_parameter_command_stub(
  const uic_mqtt_dotdot_configuration_parameters_discover_parameter_callback_t
    callback,
  int cmock_num_calls)
{
  configuration_parameters_discover_parameter_callback = callback;
}

static void configuration_parameter_set_parameter_command_stub(
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

static void configuration_parameter_discover_parameter_range_command_stub(
  const uic_mqtt_dotdot_configuration_parameters_discover_parameter_range_callback_t
    callback,
  int cmock_num_calls)
{
  discover_parameter_range_callback = callback;
}

/// Called before each and every test
void setUp()
{
  datastore_init(":memory:");
  attribute_store_init();

  configuration_parameters_discover_parameter_callback = NULL;
  configuration_parameters_set_parameter_cb_save       = NULL;
  configuration_parameters_default_reset_cb_save       = NULL;
  discover_parameter_range_callback                    = NULL;

  zpc_attribute_store_test_helper_create_network();
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);
  zwave_network_management_get_node_id_IgnoreAndReturn(zpc_node_id);

  // Callback stubs for mocking the commands
  uic_mqtt_dotdot_configuration_parameters_discover_parameter_callback_set_Stub(
    configuration_parameter_discover_parameter_command_stub);
  uic_mqtt_dotdot_configuration_parameters_set_parameter_callback_set_Stub(
    configuration_parameter_set_parameter_command_stub);
  uic_mqtt_dotdot_configuration_parameters_default_reset_all_parameters_callback_set_Stub(
    configuration_parameters_default_reset_command_stub);
  uic_mqtt_dotdot_configuration_parameters_discover_parameter_range_callback_set_Stub(
    configuration_parameter_discover_parameter_range_command_stub);

  configuration_parameter_cluster_server_init();
}

void tearDown()
{
  attribute_store_teardown();
  datastore_teardown();
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

  // No version data will not do anything:
  configuration_parameter_id_t parameter_id_test       = 1;
  configuration_parameter_value_t parameter_value_test = 11;
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    configuration_parameters_set_parameter_cb_save(
                      test_unid,
                      test_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      parameter_id_test,
                      parameter_value_test));

  // Create a v1 value:
  zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(VERSION),
                                     &version,
                                     sizeof(version));

  // Test if the callback is supported by unid/endpoint
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_parameters_set_parameter_cb_save(
                      test_unid,
                      test_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      parameter_id_test,
                      parameter_value_test));

  // For v2 nodes, it will actually create the configuration parameter if it
  // does not exist
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_parameters_set_parameter_cb_save(
                      test_unid,
                      test_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      parameter_id_test,
                      parameter_value_test));

  attribute_store_node_t parameter_id_node
    = attribute_store_get_node_child_by_value(
      endpoint_id_node,
      ATTRIBUTE(PARAMETER_ID),
      REPORTED_ATTRIBUTE,
      (const uint8_t *)&parameter_id_test,
      sizeof(parameter_id_test),
      0);
  attribute_store_node_t value_node
    = attribute_store_get_first_child_by_type(parameter_id_node,
                                              ATTRIBUTE(PARAMETER_VALUE));

  configuration_parameter_value_t desired_value = 0;
  attribute_store_get_desired(value_node,
                              &desired_value,
                              sizeof(desired_value));

  TEST_ASSERT_EQUAL(parameter_value_test, desired_value);
}

void test_configuration_set_parameter_non_existing_for_v3_node()
{
  TEST_ASSERT_NOT_NULL(configuration_parameters_set_parameter_cb_save);

  zwave_cc_version_t version = 3;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(VERSION),
                                     &version,
                                     sizeof(version));

  configuration_parameter_id_t parameter_id_test       = 1;
  configuration_parameter_value_t parameter_value_test = 11;
  // Test if the callback is supported by unid/endpoint
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_parameters_set_parameter_cb_save(
                      test_unid,
                      test_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      parameter_id_test,
                      parameter_value_test));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_parameters_set_parameter_cb_save(
                      test_unid,
                      test_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                      parameter_id_test,
                      parameter_value_test));

  // This command does not create configuration parameters automatically for v3 nodes
  attribute_store_node_t parameter_id_node
    = attribute_store_get_node_child_by_value(
      endpoint_id_node,
      ATTRIBUTE(PARAMETER_ID),
      REPORTED_ATTRIBUTE,
      (const uint8_t *)&parameter_id_test,
      sizeof(parameter_id_test),
      0);

  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, parameter_id_node);
}

void test_configuration_parameter_cluster_server_test_default_reset_command()
{
  TEST_ASSERT_NOT_NULL(configuration_parameters_default_reset_cb_save);

  zwave_cc_version_t version = 4;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(VERSION),
                                     &version,
                                     sizeof(version));

  // Test if the callback is supported by unid/endpoint
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_parameters_default_reset_cb_save(
                      test_unid,
                      test_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  attribute_store_node_t default_reset_node
    = attribute_store_add_node(ATTRIBUTE(DEFAULT_RESET_REQUESTED),
                               endpoint_id_node);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_parameters_default_reset_cb_save(
                      test_unid,
                      test_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));

  command_status_values_t value = 0xFF;
  attribute_store_get_desired(default_reset_node, &value, sizeof(value));
  TEST_ASSERT_EQUAL(NEEDS_ONE_COMMAND, value);
}

void test_configuration_publishing_attributes_on_update_after_network_status_update()
{
  // Create a configuration parameter, but no network status yet. Nothing
  // should happen

  // clang-format off
  zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node, ATTRIBUTE(VERSION), &version, sizeof(version));

  configuration_parameter_id_t id = 1;
  attribute_store_node_t parameter_1_node = attribute_store_emplace(endpoint_id_node, ATTRIBUTE(PARAMETER_ID), &id, sizeof(id));

  configuration_parameter_format_t format = 4;
  attribute_store_set_child_reported(parameter_1_node, ATTRIBUTE(PARAMETER_FORMAT), &format, sizeof(format));

  const char empty_string[] = {'\0'};
  attribute_store_set_child_reported(parameter_1_node, ATTRIBUTE(PARAMETER_NAME), empty_string, sizeof(empty_string));
  attribute_store_set_child_reported(parameter_1_node, ATTRIBUTE(PARAMETER_INFO), empty_string, sizeof(empty_string));

  configuration_parameter_value_t value = 2;
  attribute_store_set_child_reported(parameter_1_node, ATTRIBUTE(PARAMETER_VALUE), &value, sizeof(value));
  value = 0;
  attribute_store_set_child_reported(parameter_1_node, ATTRIBUTE(PARAMETER_MIN_VALUE), &value, sizeof(value));
  value = 40;
  attribute_store_set_child_reported(parameter_1_node, ATTRIBUTE(PARAMETER_MAX_VALUE), &value, sizeof(value));
  value = 2;
  attribute_store_set_child_reported(parameter_1_node, ATTRIBUTE(PARAMETER_DEFAULT_VALUE), &value, sizeof(value));

  configuration_parameter_flag_t flag = false;
  attribute_store_set_child_reported(parameter_1_node, ATTRIBUTE(PARAMETER_ADVANCED), &flag, sizeof(flag));
  attribute_store_set_child_reported(parameter_1_node, ATTRIBUTE(PARAMETER_READ_ONLY), &flag, sizeof(flag));
  attribute_store_set_child_reported(parameter_1_node, ATTRIBUTE(PARAMETER_ALTERING_CAPABILITIES), &flag, sizeof(flag));
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

  NodeStateNetworkStatus network_status = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL;
  attribute_store_set_child_reported(node_id_node,
                                     DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));

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
  network_status = ZCL_NODE_STATE_NETWORK_STATUS_OFFLINE;
  attribute_store_set_child_reported(node_id_node,
                                     DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));

  // Reconfigure the value, but no publication should happen:
  int64_t new_value = 23;
  attribute_store_set_child_reported(parameter_1_node,
                                     ATTRIBUTE(PARAMETER_VALUE),
                                     &new_value,
                                     sizeof(new_value));
}

void test_discover_parameter_range_happy_case()
{
  TEST_ASSERT_NOT_NULL(discover_parameter_range_callback);
  const zwave_cc_version_t version = 2;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(VERSION),
                                     &version,
                                     sizeof(version));

  configuration_parameter_id_t first_parameter_id_test = 12;
  configuration_parameter_id_t last_parameter_id_test  = 300;
  // Test if the callback is supported by unid/endpoint
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    discover_parameter_range_callback(
                      test_unid,
                      test_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      first_parameter_id_test,
                      last_parameter_id_test));

  // Now ask for real.
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    discover_parameter_range_callback(test_unid,
                                      test_endpoint_id,
                                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                      first_parameter_id_test,
                                      last_parameter_id_test));

  attribute_store_node_t parameters_list_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(PARAMETERS_TO_DISCOVER),
                                             0);

  // It should have saved value 289 in the list:
  TEST_ASSERT_EQUAL(289,
                    attribute_store_get_node_child_count(parameters_list_node));

  for (configuration_parameter_id_t i = first_parameter_id_test;
       i <= last_parameter_id_test;
       ++i) {
    TEST_ASSERT_NOT_EQUAL(
      ATTRIBUTE_STORE_INVALID_NODE,
      attribute_store_get_node_child_by_value(parameters_list_node,
                                              ATTRIBUTE(PARAMETER_ID),
                                              REPORTED_ATTRIBUTE,
                                              (const uint8_t *)&i,
                                              sizeof(i),
                                              0));
  }

  // Calling again will not make any modification:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    discover_parameter_range_callback(test_unid,
                                      test_endpoint_id,
                                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                      first_parameter_id_test,
                                      last_parameter_id_test));
  TEST_ASSERT_EQUAL(289,
                    attribute_store_get_node_child_count(parameters_list_node));
}

void test_discover_parameter_range_first_param_is_zero()
{
  TEST_ASSERT_NOT_NULL(discover_parameter_range_callback);
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(VERSION),
                                     &version,
                                     sizeof(version));

  configuration_parameter_id_t first_parameter_id_test = 0;
  configuration_parameter_id_t last_parameter_id_test  = 3;
  // Test if the callback is supported by unid/endpoint
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    discover_parameter_range_callback(
                      test_unid,
                      test_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      first_parameter_id_test,
                      last_parameter_id_test));

  // Now ask for real.
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    discover_parameter_range_callback(test_unid,
                                      test_endpoint_id,
                                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                      first_parameter_id_test,
                                      last_parameter_id_test));

  attribute_store_node_t parameters_list_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(PARAMETERS_TO_DISCOVER),
                                             0);

  // It should have saved value 3 in the list:
  TEST_ASSERT_EQUAL(3,
                    attribute_store_get_node_child_count(parameters_list_node));

  for (configuration_parameter_id_t i = 1; i <= last_parameter_id_test; ++i) {
    TEST_ASSERT_NOT_EQUAL(
      ATTRIBUTE_STORE_INVALID_NODE,
      attribute_store_get_node_child_by_value(parameters_list_node,
                                              ATTRIBUTE(PARAMETER_ID),
                                              REPORTED_ATTRIBUTE,
                                              (const uint8_t *)&i,
                                              sizeof(i),
                                              0));
  }
  // Calling again will not make any modification:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    discover_parameter_range_callback(test_unid,
                                      test_endpoint_id,
                                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                      first_parameter_id_test,
                                      last_parameter_id_test));
  TEST_ASSERT_EQUAL(3,
                    attribute_store_get_node_child_count(parameters_list_node));
}

void test_discover_parameter_range_first_param_higher_than_last_param()
{
  TEST_ASSERT_NOT_NULL(discover_parameter_range_callback);
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(VERSION),
                                     &version,
                                     sizeof(version));

  configuration_parameter_id_t first_parameter_id_test = UINT16_MAX;
  configuration_parameter_id_t last_parameter_id_test  = UINT16_MAX - 23;
  // Test if the callback is supported by unid/endpoint
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    discover_parameter_range_callback(
                      test_unid,
                      test_endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      first_parameter_id_test,
                      last_parameter_id_test));

  // Now ask for real.
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    discover_parameter_range_callback(test_unid,
                                      test_endpoint_id,
                                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                      first_parameter_id_test,
                                      last_parameter_id_test));

  attribute_store_node_t parameters_list_node
    = attribute_store_get_node_child_by_type(endpoint_id_node,
                                             ATTRIBUTE(PARAMETERS_TO_DISCOVER),
                                             0);

  // It should have saved value 24 in the list:
  TEST_ASSERT_EQUAL(24,
                    attribute_store_get_node_child_count(parameters_list_node));

  for (configuration_parameter_id_t i = last_parameter_id_test;
       ((i <= first_parameter_id_test) && (i != 0));
       ++i) {
    TEST_ASSERT_NOT_EQUAL(
      ATTRIBUTE_STORE_INVALID_NODE,
      attribute_store_get_node_child_by_value(parameters_list_node,
                                              ATTRIBUTE(PARAMETER_ID),
                                              REPORTED_ATTRIBUTE,
                                              (const uint8_t *)&i,
                                              sizeof(i),
                                              0));
  }
  // Calling again will not make any modification:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    discover_parameter_range_callback(test_unid,
                                      test_endpoint_id,
                                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                      first_parameter_id_test,
                                      last_parameter_id_test));
  TEST_ASSERT_EQUAL(24,
                    attribute_store_get_node_child_count(parameters_list_node));
}
