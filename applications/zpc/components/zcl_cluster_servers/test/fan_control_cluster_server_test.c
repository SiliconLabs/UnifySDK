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
#include "fan_control_cluster_server.h"
#include "unify_dotdot_attribute_store.h"
#include "unity.h"

// Unify components
#include "datastore.h"
#include "attribute_store_fixt.h"
#include "attribute_store_helper.h"
#include "unify_dotdot_defined_attribute_types.h"
#include "dotdot_mqtt_mock.h"

// ZPC Components
#include "zwave_unid.h"
#include "zwave_command_class_thermostat_fan_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"
#include "attribute_store_defined_attribute_types.h"

uic_mqtt_dotdot_unify_fan_control_turn_off_callback_t
  uic_mqtt_dotdot_fan_control_turn_off_callback;

void uic_mqtt_dotdot_fan_control_turn_off_callback_set_stub(
  const uic_mqtt_dotdot_unify_fan_control_turn_off_callback_t callback,
  int cmock_num_calls)
{
  uic_mqtt_dotdot_fan_control_turn_off_callback = callback;
}

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

/// Called before each and every test
void setUp()
{
  zpc_attribute_store_test_helper_create_network();
  uic_mqtt_dotdot_unify_fan_control_turn_off_callback_set_Stub(
    &uic_mqtt_dotdot_fan_control_turn_off_callback_set_stub);

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, fan_control_cluster_server_init());
}

/// Called after each and every test
void tearDown()
{
  attribute_store_delete_node(attribute_store_get_root());
}

void test_fan_control_command_mapping()
{
  TEST_ASSERT_NOT_NULL(uic_mqtt_dotdot_fan_control_turn_off_callback);

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    uic_mqtt_dotdot_fan_control_turn_off_callback(
                      supporting_node_unid,
                      endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));

  attribute_store_node_t off_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_OFF_FLAG,
    endpoint_id_node);

  // test support
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_fan_control_turn_off_callback(
                      supporting_node_unid,
                      endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK));
  // Test callback
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_fan_control_turn_off_callback(
                      supporting_node_unid,
                      endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL));

  thermostat_fan_mode_off_flag_t off_flag = 0;

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    attribute_store_get_desired(off_node, &off_flag, sizeof(off_flag)),
    "Can't get Off flag value");
    
  // Test value
  TEST_ASSERT_EQUAL(1, off_flag);
}
