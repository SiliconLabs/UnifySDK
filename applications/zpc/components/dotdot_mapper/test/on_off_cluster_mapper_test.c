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
#include "on_off_cluster_mapper.h"
#include "unity.h"

// Unify components
#include "datastore.h"
#include "attribute_store_fixt.h"
#include "attribute_store_helper.h"

// ZPC Components
#include "zwave_unid.h"
#include "dotdot_attributes.h"

// Mock includes
#include "dotdot_mqtt_mock.h"
#include "zwave_network_management_mock.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"
#include "attribute_store_defined_attribute_types.h"

// Private variables
static unid_t node_unid;
static uic_mqtt_dotdot_on_off_on_callback_t on_command         = NULL;
static uic_mqtt_dotdot_on_off_off_callback_t off_command       = NULL;
static uic_mqtt_dotdot_on_off_toggle_callback_t toggle_command = NULL;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
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

// Stub functions
void uic_mqtt_dotdot_on_off_on_callback_set_stub(
  const uic_mqtt_dotdot_on_off_on_callback_t callback, int cmock_num_calls)
{
  on_command = callback;
}

void uic_mqtt_dotdot_on_off_off_callback_set_stub(
  const uic_mqtt_dotdot_on_off_off_callback_t callback, int cmock_num_calls)
{
  off_command = callback;
}

void uic_mqtt_dotdot_on_off_toggle_callback_set_stub(
  const uic_mqtt_dotdot_on_off_toggle_callback_t callback, int cmock_num_calls)
{
  toggle_command = callback;
}

/// Called before each and every test
void setUp()
{
  on_command     = NULL;
  off_command    = NULL;
  toggle_command = NULL;

  uic_mqtt_dotdot_on_off_on_callback_set_Stub(
    &uic_mqtt_dotdot_on_off_on_callback_set_stub);

  uic_mqtt_dotdot_on_off_off_callback_set_Stub(
    &uic_mqtt_dotdot_on_off_off_callback_set_stub);

  uic_mqtt_dotdot_on_off_toggle_callback_set_Stub(
    &uic_mqtt_dotdot_on_off_toggle_callback_set_stub);

  // Call init
  TEST_ASSERT_TRUE(on_off_cluster_mapper_init());

  // Create a test network
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_from_node_id(node_id, node_unid);
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);
}

/// Called after each and every test
void tearDown()
{
  TEST_ASSERT_EQUAL(0, on_off_cluster_mapper_teardown());
  attribute_store_delete_node(attribute_store_get_root());
}

void test_on_command()
{
  TEST_ASSERT_NOT_NULL(on_command);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    on_command(node_unid,
                               endpoint_id,
                               UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  // Add the level attribute (still not supported, we need both level and on/off)
  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               endpoint_id_node);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    on_command(node_unid,
                               endpoint_id,
                               UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    on_command(node_unid, endpoint_id, UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));
}

void test_off_command()
{
  TEST_ASSERT_NOT_NULL(off_command);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    off_command(node_unid,
                                endpoint_id,
                                UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  // Add the level attribute (still not supported, we need both level and on/off)
  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               endpoint_id_node);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    off_command(node_unid,
                                endpoint_id,
                                UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    off_command(node_unid, endpoint_id, UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));
}

void test_toggle_command()
{
  TEST_ASSERT_NOT_NULL(toggle_command);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    toggle_command(node_unid,
                   endpoint_id,
                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  // Add the level attribute (still not supported, we need both level and on/off)
  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               endpoint_id_node);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    toggle_command(node_unid,
                   endpoint_id,
                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    toggle_command(node_unid,
                                   endpoint_id,
                                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));
}

void test_on_onoff_attribute_created_due_to_basic_value()
{
  // set the network status of
  attribute_store_node_t network_status_node
    = attribute_store_add_node(ATTRIBUTE_NETWORK_STATUS, node_id_node);
  uint8_t test_online_network_status = 0x01;
  attribute_store_set_node_attribute_value(network_status_node,
                                           REPORTED_ATTRIBUTE,
                                           &test_online_network_status,
                                           sizeof(test_online_network_status));
  uic_mqtt_dotdot_publish_supported_commands_Expect(NULL, endpoint_id);
  uic_mqtt_dotdot_publish_supported_commands_IgnoreArg_unid();
  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF, endpoint_id_node);
}