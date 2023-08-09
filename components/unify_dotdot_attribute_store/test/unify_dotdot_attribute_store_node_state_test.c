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
#include "unify_dotdot_attribute_store.h"
#include "unify_dotdot_attribute_store_node_state.h"
#include "unify_dotdot_defined_attribute_types.h"
#include "unity.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"

// Test helpers
//#include "zpc_attribute_store_test_helper.h"
#include "contiki_test_helper.h"
#include "process.h"

// Mock includes
#include "dotdot_mqtt_mock.h"
#include "uic_mqtt_mock.h"

// Declare the process name of the component, so we can shut it down manually in the test.
PROCESS_NAME(unify_dotdot_attribute_store_node_state_process);

#define TEST_ENDPOINT_TYPE 3
#define TEST_NODE_TYPE     2

#define STATE(attribute) DOTDOT_ATTRIBUTE_ID_STATE_##attribute

#define TEST_NETWORK_LIST   "networkId-nodeId"

// Test functions used for the configuration the Unify Dotdot Attribute Store Component.
static attribute_store_node_t
  get_endpoint_node(const dotdot_unid_t unid, dotdot_endpoint_id_t endpoint_id)
{
  attribute_store_node_t node = attribute_store_get_root();

  node = attribute_store_get_node_child_by_value(node,
                                                 TEST_NODE_TYPE,
                                                 REPORTED_ATTRIBUTE,
                                                 (const uint8_t *)unid,
                                                 strlen(unid) + 1,
                                                 0);

  node = attribute_store_get_node_child_by_value(node,
                                                 TEST_ENDPOINT_TYPE,
                                                 REPORTED_ATTRIBUTE,
                                                 &endpoint_id,
                                                 sizeof(endpoint_id),
                                                 0);

  return node;
}

static sl_status_t get_unid(attribute_store_node_t node, char *unid)
{
  if (attribute_store_get_node_type(node) != TEST_NODE_TYPE) {
    node = attribute_store_get_first_parent_with_type(node, TEST_NODE_TYPE);
  }

  return attribute_store_get_reported_string(node, unid, MAXIMUM_UNID_SIZE);
}

static sl_status_t get_unid_endpoint(attribute_store_node_t node,
                                     char *unid,
                                     dotdot_endpoint_id_t *endpoint_id)
{
  if (attribute_store_get_node_type(node) != TEST_ENDPOINT_TYPE) {
    node = attribute_store_get_first_parent_with_type(node, TEST_ENDPOINT_TYPE);
  }
  if (SL_STATUS_OK
      != attribute_store_get_reported(node,
                                      endpoint_id,
                                      sizeof(dotdot_endpoint_id_t))) {
    return SL_STATUS_FAIL;
  }
  return get_unid(node, unid);
}

const unify_dotdot_attribute_store_configuration_t test_configuration = {
  .get_endpoint_node_function                  = &get_endpoint_node,
  .get_unid_endpoint_function                  = &get_unid_endpoint,
  .get_unid_function                           = &get_unid,
  .update_attribute_desired_values_on_commands = false,
  .clear_reported_on_desired_updates           = false,
  .automatic_deduction_of_supported_commands   = false,
  .force_read_attributes_enabled               = false,
  .write_attributes_enabled                    = false,
  .publish_desired_attribute_values_to_mqtt    = false,
  .publish_reported_attribute_values_to_mqtt   = false,
  .node_type                                   = TEST_NODE_TYPE,
  .endpoint_type                               = TEST_ENDPOINT_TYPE,
};

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  contiki_test_helper_init();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  uic_mqtt_unretain_Ignore();
  uic_mqtt_dotdot_state_endpoint_id_list_publish_IgnoreAndReturn(SL_STATUS_OK);
  attribute_store_delete_node(attribute_store_get_root());
  uic_mqtt_unretain_StopIgnore();
  uic_mqtt_dotdot_state_endpoint_id_list_publish_StopIgnore();

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    unify_dotdot_attribute_store_node_state_init());

  unify_dotdot_attribute_store_set_configuration(&test_configuration);
  contiki_test_helper_run(0);
}

/// Called after each and every test
void tearDown()
{
  uic_mqtt_publish_Ignore();
  uic_mqtt_unretain_by_regex_Ignore();
  process_exit(&unify_dotdot_attribute_store_node_state_process);
  contiki_test_helper_run(0);
}

/**
 * @brief Helper function that create a UNID/EP node in the attribute store
*/
static attribute_store_node_t create_node_ep(char *unid,
                                             dotdot_endpoint_id_t endpoint_id)
{
  attribute_store_node_t unid_node
    = attribute_store_add_node(TEST_NODE_TYPE, attribute_store_get_root());
  attribute_store_node_t endpoint_node
    = attribute_store_add_node(TEST_ENDPOINT_TYPE, unid_node);

  attribute_store_set_reported_string(unid_node, unid);
  attribute_store_set_reported(endpoint_node,
                               &endpoint_id,
                               sizeof(endpoint_id));

  return endpoint_node;
}

void test_publish_node_state_happy_case()
{
  // Create an unid/endpoint:
  char unid_value[]                = "unid-42";
  dotdot_endpoint_id_t endpoint_id = 4;
  attribute_store_node_t endpoint_node
    = create_node_ep(unid_value, endpoint_id);

  // Check what's its network status: (should be nothing yet)
  TEST_ASSERT_EQUAL(ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE,
                    unify_attribute_store_node_state_get_status(endpoint_node));

  // Add the state attributes. No publication until we know the Network Status
  NodeStateSecurity security_level = ZCL_NODE_STATE_SECURITY_ZIGBEE_Z3;
  attribute_store_set_child_reported(endpoint_node,
                                     STATE(SECURITY),
                                     &security_level,
                                     sizeof(NodeStateSecurity));
  uint32_t maximum_command_delay = 0;
  attribute_store_set_child_reported(endpoint_node,
                                     STATE(MAXIMUM_COMMAND_DELAY),
                                     &maximum_command_delay,
                                     sizeof(uint32_t));

  // Now add the network status: it will trigger a few publications:
  uic_mqtt_dotdot_publish_supported_commands_Expect(NULL, endpoint_id);
  uic_mqtt_dotdot_publish_supported_commands_IgnoreArg_unid();
  uic_mqtt_dotdot_state_publish_supported_commands_Expect(NULL, 0);
  uic_mqtt_dotdot_state_publish_supported_commands_IgnoreArg_unid();
  uic_mqtt_dotdot_state_endpoint_id_list_publish_ExpectAndReturn(
    "ucl/by-unid/unid-42",
    1,
    NULL,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_state_endpoint_id_list_publish_IgnoreArg_value();
  NodeStateNetworkStatus network_status
    = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL;
  attribute_store_set_child_reported(
    attribute_store_get_node_parent(endpoint_node),
    STATE(NETWORK_STATUS),
    &network_status,
    sizeof(network_status));

  TEST_ASSERT_EQUAL(network_status,
                    unify_attribute_store_node_state_get_status(endpoint_node));

}

void test_publish_network_list()
{
  // Create an unid/endpoint:
  char unid_value[]                = "unid-42";
  dotdot_endpoint_id_t endpoint_id = 4;
  attribute_store_node_t endpoint_node
    = create_node_ep(unid_value, endpoint_id);

  uic_mqtt_dotdot_state_network_list_publish_ExpectAndReturn(
    "ucl/by-unid/unid-42",
    1,
    NULL,
    UCL_MQTT_PUBLISH_TYPE_ALL,
    SL_STATUS_OK);
  uic_mqtt_dotdot_state_network_list_publish_IgnoreArg_value();
  attribute_store_set_child_reported(
    attribute_store_get_node_parent(endpoint_node),
    STATE(NETWORK_LIST),
    TEST_NETWORK_LIST,
    strlen(TEST_NETWORK_LIST) + 1);
}

void test_publish_node_unavailable_on_shutdown()
{
  // Create an endpoint:
  char unid_value[]                = "unid-03948575";
  dotdot_endpoint_id_t endpoint_id = 0;
  create_node_ep(unid_value, endpoint_id);

  // For the endpoint/node, the node state will be unpublished on shutdown
  uic_mqtt_publish_Expect("ucl/by-unid/unid-03948575/State",
                          "{\"MaximumCommandDelay\":0,\"NetworkList\":[\"\"],"
                          "\"NetworkStatus\":\"Unavailable\",\"Security\":"
                          "\"None\"}",
                          92,
                          true);
  uic_mqtt_unretain_by_regex_Expect(REGEX_NOT_STATE_OR_MQTT_CLIENT_TOPICS);
  process_exit(&unify_dotdot_attribute_store_node_state_process);
  contiki_test_helper_run(0);
}

void test_config_with_invalid_node_types()
{
  const unify_dotdot_attribute_store_configuration_t config_invalid_types = {
    .get_endpoint_node_function                  = NULL,
    .get_unid_endpoint_function                  = NULL,
    .get_unid_function                           = NULL,
    .update_attribute_desired_values_on_commands = false,
    .clear_reported_on_desired_updates           = false,
    .automatic_deduction_of_supported_commands   = false,
    .force_read_attributes_enabled               = false,
    .write_attributes_enabled                    = false,
    .publish_reported_attribute_values_to_mqtt   = false,
    .publish_desired_attribute_values_to_mqtt    = false,
    .node_type     = ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
    .endpoint_type = ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
  };

  // Nothing bad should happen here:
  unify_dotdot_attribute_store_set_configuration(&config_invalid_types);
  contiki_test_helper_run(0);

  // Nothing will be published when creating attributes:
  // Create an unid/endpoint:
  char unid_value[]                = "this-is-a-long-unid";
  dotdot_endpoint_id_t endpoint_id = 1;
  attribute_store_node_t endpoint_node
    = create_node_ep(unid_value, endpoint_id);

  // Check what's its network status: (should be nothing yet)
  TEST_ASSERT_EQUAL(ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE,
                    unify_attribute_store_node_state_get_status(endpoint_node));

  // Add the state attributes. No publication until we know the Network Status
  NodeStateSecurity security_level = ZCL_NODE_STATE_SECURITY_ZIGBEE_Z3;
  attribute_store_set_child_reported(endpoint_node,
                                     STATE(SECURITY),
                                     &security_level,
                                     sizeof(NodeStateSecurity));
  uint32_t maximum_command_delay = 0;
  attribute_store_set_child_reported(endpoint_node,
                                     STATE(MAXIMUM_COMMAND_DELAY),
                                     &maximum_command_delay,
                                     sizeof(uint32_t));
  NodeStateNetworkStatus network_status
    = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL;
  attribute_store_set_child_reported(
    attribute_store_get_node_parent(endpoint_node),
    STATE(NETWORK_STATUS),
    &network_status,
    sizeof(network_status));
}
