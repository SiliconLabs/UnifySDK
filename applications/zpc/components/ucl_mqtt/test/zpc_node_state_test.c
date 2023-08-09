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
#include "zpc_node_state.h"

// Unify components
#include "datastore.h"
#include "attribute_store_fixt.h"
#include "attribute_store_helper.h"
#include "sl_log.h"

// Interface includes
#include "attribute_store_defined_attribute_types.h"
#include "zwave_command_class_generic_types.h"

// ZPC includes
#include "zpc_attribute_store_type_registration.h"
//#include "zwave_utils.h"
//#include "zwave_controller_types.h"

// Generic includes
#include <string.h>

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "dotdot_mqtt_mock.h"
#include "zwave_network_management_mock.h"

static uic_mqtt_dotdot_state_remove_offline_callback_t remove_offline_command
  = NULL;
static uic_mqtt_dotdot_state_discover_neighbors_callback_t
  discover_neighbors_command
  = NULL;
static uic_mqtt_dotdot_state_interview_callback_t interview_command = NULL;
static uic_mqtt_dotdot_state_discover_security_callback_t
  discover_security_command
  = NULL;

// Stub functions
static void uic_mqtt_dotdot_state_remove_offline_callback_set_stub(
  const uic_mqtt_dotdot_state_remove_offline_callback_t callback,
  int cmock_num_calls)
{
  remove_offline_command = callback;
}

static void uic_mqtt_dotdot_state_discover_neighbors_callback_set_stub(
  const uic_mqtt_dotdot_state_discover_neighbors_callback_t callback,
  int cmock_num_calls)
{
  discover_neighbors_command = callback;
}

static void uic_mqtt_dotdot_state_interview_callback_set_stub(
  const uic_mqtt_dotdot_state_interview_callback_t callback,
  int cmock_num_calls)
{
  interview_command = callback;
}

static void uic_mqtt_dotdot_state_discover_security_callback_set_stub(
  const uic_mqtt_dotdot_state_discover_security_callback_t callback,
  int cmock_num_calls)
{
  discover_security_command = callback;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  zpc_attribute_store_register_known_attribute_types();
  zwave_unid_set_home_id(home_id);
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
  remove_offline_command     = NULL;
  discover_neighbors_command = NULL;
  interview_command          = NULL;
  discover_security_command  = NULL;

  // Always respond with HomeID / NodeID from the simulated network
  zwave_network_management_get_node_id_IgnoreAndReturn(zpc_node_id);
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);

  // Prep our attribute store
  zpc_attribute_store_test_helper_create_network();

  // Intercept the callbacks
  uic_mqtt_dotdot_state_remove_offline_callback_set_Stub(
    uic_mqtt_dotdot_state_remove_offline_callback_set_stub);

  uic_mqtt_dotdot_state_discover_neighbors_callback_set_Stub(
    uic_mqtt_dotdot_state_discover_neighbors_callback_set_stub);

  uic_mqtt_dotdot_state_interview_callback_set_Stub(
    uic_mqtt_dotdot_state_interview_callback_set_stub);

  uic_mqtt_dotdot_state_discover_security_callback_set_Stub(
    uic_mqtt_dotdot_state_discover_security_callback_set_stub);

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zpc_node_state_init());

  // Check all the callbacks have been intercepted correctly:
  TEST_ASSERT_NOT_NULL(remove_offline_command);
  TEST_ASSERT_NOT_NULL(discover_neighbors_command);
  TEST_ASSERT_NOT_NULL(interview_command);
  TEST_ASSERT_NOT_NULL(discover_security_command);
}

/// Called after each and every test
void tearDown() {}

void test_discover_security_support_check()
{
  // Set our node to online functional
  NodeStateNetworkStatus network_status
    = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL;
  attribute_store_set_child_reported(node_id_node,
                                     DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));

  // Discover Security should be supported:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    discover_security_command(supporting_node_unid,
                              endpoint_id,
                              UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  // Offline should not show this state command
  network_status = ZCL_NODE_STATE_NETWORK_STATUS_OFFLINE;
  attribute_store_set_child_reported(node_id_node,
                                     DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    discover_security_command(supporting_node_unid,
                              endpoint_id,
                              UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  // Unavailable should not show this state command
  network_status = ZCL_NODE_STATE_NETWORK_STATUS_UNAVAILABLE;
  attribute_store_set_child_reported(node_id_node,
                                     DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    discover_security_command(supporting_node_unid,
                              endpoint_id,
                              UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  // ZPC SHOULD not support this state command either
  network_status = ZCL_NODE_STATE_NETWORK_STATUS_ONLINE_FUNCTIONAL;
  attribute_store_set_child_reported(zpc_node_id_node,
                                     DOTDOT_ATTRIBUTE_ID_STATE_NETWORK_STATUS,
                                     &network_status,
                                     sizeof(network_status));
  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    discover_security_command(zpc_unid,
                              zpc_endpoint_id,
                              UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));
}

void test_discover_security_command()
{
  // Set some granted keys:
  attribute_store_node_t granted_keys_node
    = attribute_store_add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS, node_id_node);
  attribute_store_set_reported_number(granted_keys_node, 0xFF);

  // Granted keys get undefined
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    discover_security_command(supporting_node_unid,
                              endpoint_id,
                              UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(granted_keys_node));

  // Set some granted keys under the ZPC:
  granted_keys_node = attribute_store_add_node(ATTRIBUTE_GRANTED_SECURITY_KEYS,
                                               zpc_node_id_node);
  attribute_store_set_reported_number(granted_keys_node, 0xFF);

  //  ZPC must not lose its keys:
  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_SUPPORTED,
    discover_security_command(zpc_unid,
                              zpc_endpoint_id,
                              UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(granted_keys_node));
}
