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
#include "level_cluster_mapper.h"
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

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Private variables
static unid_t node_unid;
static uic_mqtt_dotdot_level_move_to_level_callback_t move_to_level_command
  = NULL;
static uic_mqtt_dotdot_level_move_callback_t move_command = NULL;
static uic_mqtt_dotdot_level_step_callback_t step_command = NULL;
static uic_mqtt_dotdot_level_stop_callback_t stop_command = NULL;
static uic_mqtt_dotdot_level_move_to_level_with_on_off_callback_t
  move_to_level_with_on_off_command
  = NULL;
static uic_mqtt_dotdot_level_move_with_on_off_callback_t
  move_with_on_off_command
  = NULL;
static uic_mqtt_dotdot_level_step_with_on_off_callback_t
  step_with_on_off_command
  = NULL;
static uic_mqtt_dotdot_level_stop_with_on_off_callback_t
  stop_with_on_off_command
  = NULL;

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
void uic_mqtt_dotdot_level_move_to_level_callback_set_stub(
  const uic_mqtt_dotdot_level_move_to_level_callback_t callback,
  int cmock_num_calls)
{
  move_to_level_command = callback;
}

void uic_mqtt_dotdot_level_move_callback_set_stub(
  const uic_mqtt_dotdot_level_move_callback_t callback, int cmock_num_calls)
{
  move_command = callback;
}

void uic_mqtt_dotdot_level_step_callback_set_stub(
  const uic_mqtt_dotdot_level_step_callback_t callback, int cmock_num_calls)
{
  step_command = callback;
}

void uic_mqtt_dotdot_level_stop_callback_set_stub(
  const uic_mqtt_dotdot_level_stop_callback_t callback, int cmock_num_calls)
{
  stop_command = callback;
}

void uic_mqtt_dotdot_level_move_to_level_with_on_off_callback_set_stub(
  const uic_mqtt_dotdot_level_move_to_level_with_on_off_callback_t callback,
  int cmock_num_calls)
{
  move_to_level_with_on_off_command = callback;
}

void uic_mqtt_dotdot_level_move_with_on_off_callback_set_stub(
  const uic_mqtt_dotdot_level_move_with_on_off_callback_t callback,
  int cmock_num_calls)
{
  move_with_on_off_command = callback;
}

void uic_mqtt_dotdot_level_step_with_on_off_callback_set_stub(
  const uic_mqtt_dotdot_level_step_with_on_off_callback_t callback,
  int cmock_num_calls)
{
  step_with_on_off_command = callback;
}

void uic_mqtt_dotdot_level_stop_with_on_off_callback_set_stub(
  const uic_mqtt_dotdot_level_stop_with_on_off_callback_t callback,
  int cmock_num_calls)
{
  stop_with_on_off_command = callback;
}

/// Called before each and every test
void setUp()
{
  move_to_level_command             = NULL;
  move_command                      = NULL;
  step_command                      = NULL;
  stop_command                      = NULL;
  move_to_level_with_on_off_command = NULL;
  move_with_on_off_command          = NULL;
  step_with_on_off_command          = NULL;
  stop_with_on_off_command          = NULL;

  uic_mqtt_dotdot_level_move_to_level_callback_set_Stub(
    &uic_mqtt_dotdot_level_move_to_level_callback_set_stub);

  uic_mqtt_dotdot_level_move_callback_set_Stub(
    &uic_mqtt_dotdot_level_move_callback_set_stub);

  uic_mqtt_dotdot_level_step_callback_set_Stub(
    &uic_mqtt_dotdot_level_step_callback_set_stub);

  uic_mqtt_dotdot_level_stop_callback_set_Stub(
    &uic_mqtt_dotdot_level_stop_callback_set_stub);

  uic_mqtt_dotdot_level_move_to_level_with_on_off_callback_set_Stub(
    &uic_mqtt_dotdot_level_move_to_level_with_on_off_callback_set_stub);

  uic_mqtt_dotdot_level_move_with_on_off_callback_set_Stub(
    &uic_mqtt_dotdot_level_move_with_on_off_callback_set_stub);

  uic_mqtt_dotdot_level_step_with_on_off_callback_set_Stub(
    &uic_mqtt_dotdot_level_step_with_on_off_callback_set_stub);

  uic_mqtt_dotdot_level_stop_with_on_off_callback_set_Stub(
    &uic_mqtt_dotdot_level_stop_with_on_off_callback_set_stub);

  // Call init
  TEST_ASSERT_TRUE(level_cluster_mapper_init());

  // Create a test network
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_from_node_id(node_id, node_unid);
}

/// Called after each and every test
void tearDown()
{
  TEST_ASSERT_EQUAL(0, level_cluster_mapper_teardown());
  attribute_store_delete_node(attribute_store_get_root());
}

void test_move_to_level_command()
{
  TEST_ASSERT_NOT_NULL(move_to_level_command);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    move_to_level_command(node_unid,
                          endpoint_id,
                          UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                          0,
                          0,
                          0,
                          0));

  // Add the level attribute (still not supported, we need both level and on/off)
  attribute_store_node_t level_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
                               endpoint_id_node);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    move_to_level_command(node_unid,
                          endpoint_id,
                          UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                          0,
                          0,
                          0,
                          0));

  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF, endpoint_id_node);

  // Now it is supported:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_to_level_command(node_unid,
                          endpoint_id,
                          UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                          0,
                          0,
                          0,
                          0));

  // Try to set the level:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    move_to_level_command(node_unid,
                                          endpoint_id,
                                          UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                          0,
                                          0,
                                          0,
                                          0));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));

  // Try again with a transition time
  attribute_store_node_t transition_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_ON_OFF_TRANSITION_TIME,
                               endpoint_id_node);
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    move_to_level_command(node_unid,
                                          endpoint_id,
                                          UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                          150,
                                          2,
                                          3,
                                          4));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
}

void test_move_command()
{
  TEST_ASSERT_NOT_NULL(move_command);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    move_command(node_unid,
                                 endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                 0,
                                 0,
                                 0,
                                 0));

  // Add the level attribute (still not supported, we need both level and on/off)
  attribute_store_node_t level_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
                               endpoint_id_node);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    move_command(node_unid,
                                 endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                 0,
                                 0,
                                 0,
                                 0));

  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF, endpoint_id_node);

  // Still not supported:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    move_command(node_unid,
                                 endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                 0,
                                 0,
                                 0,
                                 0));

  attribute_store_node_t transition_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_ON_OFF_TRANSITION_TIME,
                               endpoint_id_node);

  // Now it is supported
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    move_command(node_unid,
                                 endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                 0,
                                 0,
                                 0,
                                 0));

  // Try to set the level:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    move_command(node_unid,
                                 endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                 ZCL_MOVE_STEP_MODE_DOWN,
                                 0x10,
                                 0,
                                 0));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
}

void test_step_command()
{
  TEST_ASSERT_NOT_NULL(step_command);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    step_command(node_unid,
                                 endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0));

  // Add the level attribute (still not supported, we need both level and on/off)
  attribute_store_node_t level_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
                               endpoint_id_node);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    step_command(node_unid,
                                 endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0));

  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF, endpoint_id_node);

  // Still not supported:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    step_command(node_unid,
                                 endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0));

  attribute_store_node_t transition_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_ON_OFF_TRANSITION_TIME,
                               endpoint_id_node);

  // Now it is supported
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    step_command(node_unid,
                                 endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                 0,
                                 0,
                                 0,
                                 0,
                                 0));

  // Step size 0 will do nothing:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    step_command(node_unid,
                                 endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                 ZCL_MOVE_STEP_MODE_DOWN,
                                 0,
                                 0,
                                 0,
                                 0));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));

  // Move up:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    step_command(node_unid,
                                 endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                 ZCL_MOVE_STEP_MODE_UP,
                                 2,
                                 0xFFFF,
                                 0,
                                 0));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
}

void test_stop_command()
{
  TEST_ASSERT_NOT_NULL(stop_command);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    stop_command(node_unid,
                                 endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                 0,
                                 0));

  // Add the level attribute (still not supported, we need both level and on/off)
  attribute_store_node_t level_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
                               endpoint_id_node);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    stop_command(node_unid,
                                 endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                 0,
                                 0));

  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF, endpoint_id_node);

  // Still not supported:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    stop_command(node_unid,
                                 endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                 0,
                                 0));

  attribute_store_node_t transition_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_ON_OFF_TRANSITION_TIME,
                               endpoint_id_node);

  // Now it is supported
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    stop_command(node_unid,
                                 endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                 0,
                                 0));

  // Try to stop:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    stop_command(node_unid,
                                 endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                 0,
                                 0));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
}

void test_move_to_level_with_on_off_command()
{
  TEST_ASSERT_NOT_NULL(move_to_level_with_on_off_command);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    move_to_level_with_on_off_command(
                      node_unid,
                      endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  // Add the level attribute (still not supported, we need both level and on/off)
  attribute_store_node_t level_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
                               endpoint_id_node);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    move_to_level_with_on_off_command(
                      node_unid,
                      endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  attribute_store_node_t transition_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_ON_OFF_TRANSITION_TIME,
                               endpoint_id_node);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    move_to_level_with_on_off_command(
                      node_unid,
                      endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               endpoint_id_node);

  // Now it is supported:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    move_to_level_with_on_off_command(
                      node_unid,
                      endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  // Try to set the level:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_to_level_with_on_off_command(node_unid,
                                      endpoint_id,
                                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                      0,
                                      0,
                                      0,
                                      0));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));

  // Try again with > MIN value
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_to_level_with_on_off_command(node_unid,
                                      endpoint_id,
                                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                      150,
                                      2,
                                      3,
                                      4));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));
}

void test_move_with_on_off_command()
{
  TEST_ASSERT_NOT_NULL(move_with_on_off_command);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    move_with_on_off_command(node_unid,
                             endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                             0,
                             0,
                             0,
                             0));

  // Add the level attribute (still not supported, we need both level and on/off)
  attribute_store_node_t level_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
                               endpoint_id_node);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    move_with_on_off_command(node_unid,
                             endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                             0,
                             0,
                             0,
                             0));

  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               endpoint_id_node);

  // Still not supported:
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    move_with_on_off_command(node_unid,
                             endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                             0,
                             0,
                             0,
                             0));

  attribute_store_node_t transition_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_ON_OFF_TRANSITION_TIME,
                               endpoint_id_node);

  // Now it is supported
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_with_on_off_command(node_unid,
                             endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                             0,
                             0,
                             0,
                             0));

  // Try to set the level:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_with_on_off_command(node_unid,
                             endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                             ZCL_MOVE_STEP_MODE_DOWN,
                             0x10,
                             0,
                             0));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));

  // Try again to move up:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_with_on_off_command(node_unid,
                             endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                             ZCL_MOVE_STEP_MODE_UP,
                             0x10,
                             0,
                             0));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));
}

void test_step_with_on_off_command()
{
  TEST_ASSERT_NOT_NULL(step_with_on_off_command);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    step_with_on_off_command(node_unid,
                             endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                             0,
                             0,
                             0,
                             0,
                             0));

  // Add the level attribute (still not supported, we need both level and on/off)
  attribute_store_node_t level_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
                               endpoint_id_node);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    step_with_on_off_command(node_unid,
                             endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                             0,
                             0,
                             0,
                             0,
                             0));

  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               endpoint_id_node);

  // Still not supported:
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    step_with_on_off_command(node_unid,
                             endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                             0,
                             0,
                             0,
                             0,
                             0));

  attribute_store_node_t transition_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_ON_OFF_TRANSITION_TIME,
                               endpoint_id_node);

  // Now it is supported
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    step_with_on_off_command(node_unid,
                             endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                             0,
                             0,
                             0,
                             0,
                             0));

  // Step size 0 will do nothing:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    step_with_on_off_command(node_unid,
                             endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                             ZCL_MOVE_STEP_MODE_DOWN,
                             0,
                             0,
                             0,
                             0));

  // Move up:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    step_with_on_off_command(node_unid,
                             endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                             ZCL_MOVE_STEP_MODE_UP,
                             2,
                             0xFF00,
                             0,
                             0));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));
}

void test_stop_with_on_off_command()
{
  TEST_ASSERT_NOT_NULL(stop_with_on_off_command);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    stop_with_on_off_command(node_unid,
                             endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                             0,
                             0));

  // Add the level attribute (still not supported, we need both level and on/off)
  attribute_store_node_t level_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_CURRENT_LEVEL,
                               endpoint_id_node);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    stop_command(node_unid,
                                 endpoint_id,
                                 UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                 0,
                                 0));

  attribute_store_node_t on_off_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_OFF,
                               endpoint_id_node);

  // Still not supported:
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    stop_with_on_off_command(node_unid,
                             endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                             0,
                             0));

  attribute_store_node_t transition_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_LEVEL_ON_OFF_TRANSITION_TIME,
                               endpoint_id_node);

  // Now it is supported
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    stop_with_on_off_command(node_unid,
                             endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                             0,
                             0));

  // Try to stop:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    stop_with_on_off_command(node_unid,
                             endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                             0,
                             0));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(level_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(transition_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));

  int8_t test_level = 23;
  attribute_store_set_reported(level_node, &test_level, sizeof(test_level));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    stop_with_on_off_command(node_unid,
                             endpoint_id,
                             UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                             0,
                             0));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(on_off_node, DESIRED_ATTRIBUTE));
}