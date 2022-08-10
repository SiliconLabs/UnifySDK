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
#include "color_control_cluster_mapper.h"
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
static uic_mqtt_dotdot_color_control_move_to_hue_callback_t move_to_hue_command
  = NULL;
static uic_mqtt_dotdot_color_control_move_hue_callback_t move_hue_command
  = NULL;
static uic_mqtt_dotdot_color_control_step_hue_callback_t step_hue_command
  = NULL;

static uic_mqtt_dotdot_color_control_move_to_saturation_callback_t
  move_to_saturation_command
  = NULL;
static uic_mqtt_dotdot_color_control_move_saturation_callback_t
  move_saturation_command
  = NULL;
static uic_mqtt_dotdot_color_control_step_saturation_callback_t
  step_saturation_command
  = NULL;

static uic_mqtt_dotdot_color_control_move_to_hue_and_saturation_callback_t
  move_to_hue_and_saturation_command
  = NULL;

static uic_mqtt_dotdot_color_control_move_to_color_temperature_callback_t
  move_to_color_temperature_command
  = NULL;
static uic_mqtt_dotdot_color_control_move_color_temperature_callback_t
  move_color_temperature_command
  = NULL;
static uic_mqtt_dotdot_color_control_step_color_temperature_callback_t
  step_color_temperature_command
  = NULL;

static uic_mqtt_dotdot_color_control_stop_move_step_callback_t
  stop_move_step_command
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
void uic_mqtt_dotdot_color_control_move_to_hue_callback_set_stub(
  const uic_mqtt_dotdot_color_control_move_to_hue_callback_t callback,
  int cmock_num_calls)
{
  move_to_hue_command = callback;
}

void uic_mqtt_dotdot_color_control_move_hue_callback_set_stub(
  const uic_mqtt_dotdot_color_control_move_hue_callback_t callback,
  int cmock_num_calls)
{
  move_hue_command = callback;
}

void uic_mqtt_dotdot_color_control_step_hue_callback_set_stub(
  const uic_mqtt_dotdot_color_control_step_hue_callback_t callback,
  int cmock_num_calls)
{
  step_hue_command = callback;
}

void uic_mqtt_dotdot_color_control_move_to_saturation_callback_set_stub(
  const uic_mqtt_dotdot_color_control_move_to_saturation_callback_t callback,
  int cmock_num_calls)
{
  move_to_saturation_command = callback;
}

void uic_mqtt_dotdot_color_control_move_saturation_callback_set_stub(
  const uic_mqtt_dotdot_color_control_move_saturation_callback_t callback,
  int cmock_num_calls)
{
  move_saturation_command = callback;
}
void uic_mqtt_dotdot_color_control_step_saturation_callback_set_stub(
  const uic_mqtt_dotdot_color_control_step_saturation_callback_t callback,
  int cmock_num_calls)
{
  step_saturation_command = callback;
}

void uic_mqtt_dotdot_color_control_move_to_hue_and_saturation_callback_set_stub(
  const uic_mqtt_dotdot_color_control_move_to_hue_and_saturation_callback_t
    callback,
  int cmock_num_calls)
{
  move_to_hue_and_saturation_command = callback;
}

void uic_mqtt_dotdot_color_control_move_to_color_temperature_callback_set_stub(
  const uic_mqtt_dotdot_color_control_move_to_color_temperature_callback_t
    callback,
  int cmock_num_calls)
{
  move_to_color_temperature_command = callback;
}

void uic_mqtt_dotdot_color_control_move_color_temperature_callback_set_stub(
  const uic_mqtt_dotdot_color_control_move_color_temperature_callback_t
    callback,
  int cmock_num_calls)
{
  move_color_temperature_command = callback;
}

void uic_mqtt_dotdot_color_control_step_color_temperature_callback_set_stub(
  const uic_mqtt_dotdot_color_control_step_color_temperature_callback_t
    callback,
  int cmock_num_calls)
{
  step_color_temperature_command = callback;
}

void uic_mqtt_dotdot_color_control_stop_move_step_callback_set_stub(
  const uic_mqtt_dotdot_color_control_stop_move_step_callback_t callback,
  int cmock_num_calls)
{
  stop_move_step_command = callback;
}

/// Called before each and every test
void setUp()
{
  move_to_hue_command                = NULL;
  move_hue_command                   = NULL;
  step_hue_command                   = NULL;
  move_to_saturation_command         = NULL;
  move_saturation_command            = NULL;
  step_saturation_command            = NULL;
  move_to_hue_and_saturation_command = NULL;
  move_to_color_temperature_command  = NULL;
  move_color_temperature_command     = NULL;
  step_color_temperature_command     = NULL;
  stop_move_step_command             = NULL;

  // Set up the stubs
  // Hue commands
  uic_mqtt_dotdot_color_control_move_to_hue_callback_set_Stub(
    &uic_mqtt_dotdot_color_control_move_to_hue_callback_set_stub);
  uic_mqtt_dotdot_color_control_move_hue_callback_set_Stub(
    &uic_mqtt_dotdot_color_control_move_hue_callback_set_stub);
  uic_mqtt_dotdot_color_control_step_hue_callback_set_Stub(
    &uic_mqtt_dotdot_color_control_step_hue_callback_set_stub);

  // Saturation commands
  uic_mqtt_dotdot_color_control_move_to_saturation_callback_set_Stub(
    &uic_mqtt_dotdot_color_control_move_to_saturation_callback_set_stub);
  uic_mqtt_dotdot_color_control_move_saturation_callback_set_Stub(
    &uic_mqtt_dotdot_color_control_move_saturation_callback_set_stub);
  uic_mqtt_dotdot_color_control_step_saturation_callback_set_Stub(
    &uic_mqtt_dotdot_color_control_step_saturation_callback_set_stub);

  // Hue and Saturation commands
  uic_mqtt_dotdot_color_control_move_to_hue_and_saturation_callback_set_Stub(
    &uic_mqtt_dotdot_color_control_move_to_hue_and_saturation_callback_set_stub);

  // Temperature commands
  uic_mqtt_dotdot_color_control_move_to_color_temperature_callback_set_Stub(
    &uic_mqtt_dotdot_color_control_move_to_color_temperature_callback_set_stub);
  uic_mqtt_dotdot_color_control_move_color_temperature_callback_set_Stub(
    &uic_mqtt_dotdot_color_control_move_color_temperature_callback_set_stub);
  uic_mqtt_dotdot_color_control_step_color_temperature_callback_set_Stub(
    &uic_mqtt_dotdot_color_control_step_color_temperature_callback_set_stub);

  // Generic commands
  uic_mqtt_dotdot_color_control_stop_move_step_callback_set_Stub(
    &uic_mqtt_dotdot_color_control_stop_move_step_callback_set_stub);

  // Call init
  TEST_ASSERT_TRUE(color_control_cluster_mapper_init());

  // Create a test network
  zpc_attribute_store_test_helper_create_network();
  zwave_unid_from_node_id(node_id, node_unid);
}

/// Called after each and every test
void tearDown()
{
  TEST_ASSERT_EQUAL(0, color_control_cluster_mapper_teardown());
  attribute_store_delete_node(attribute_store_get_root());
}

void test_move_to_hue_command()
{
  TEST_ASSERT_NOT_NULL(move_to_hue_command);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    move_to_hue_command(node_unid,
                        endpoint_id,
                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                        0,
                        0,
                        0,
                        0,
                        0));

  // Add the Current Hue attribute (still not supported, we need both Hue and Saturation)
  attribute_store_node_t current_hue_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_HUE,
                               endpoint_id_node);
  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_SATURATION,
                           endpoint_id_node);
  attribute_store_node_t capabilities_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_CAPABILITIES,
    endpoint_id_node);
  uint16_t capabilities = 2;  // Still not supported
  attribute_store_set_reported(capabilities_node,
                               &capabilities,
                               sizeof(capabilities));
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    move_to_hue_command(node_unid,
                        endpoint_id,
                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                        0,
                        0,
                        0,
                        0,
                        0));

  // Now it is supported:
  capabilities = 1;
  attribute_store_set_reported(capabilities_node,
                               &capabilities,
                               sizeof(capabilities));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_to_hue_command(node_unid,
                        endpoint_id,
                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                        0,
                        0,
                        0,
                        0,
                        0));

  // Try to set the Hue:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(current_hue_node, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    move_to_hue_command(node_unid,
                                        endpoint_id,
                                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(current_hue_node, DESIRED_ATTRIBUTE));

  // Try again with value 0xFF
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    move_to_hue_command(node_unid,
                                        endpoint_id,
                                        UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                        0xFF,
                                        2,
                                        2,
                                        3,
                                        4));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(current_hue_node, DESIRED_ATTRIBUTE));
  uint8_t desired_hue = 0x00;
  attribute_store_get_desired(current_hue_node,
                              &desired_hue,
                              sizeof(desired_hue));
  TEST_ASSERT_EQUAL(0xFE, desired_hue);
}

void test_move_hue_command()
{
  TEST_ASSERT_NOT_NULL(move_hue_command);
  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    move_hue_command(node_unid,
                     endpoint_id,
                     UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                     0,
                     0,
                     0,
                     0));

  // Add the Current Hue attribute (still not supported, we need both Hue and Saturation)
  attribute_store_node_t current_hue_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_HUE,
                               endpoint_id_node);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    move_hue_command(node_unid,
                     endpoint_id,
                     UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                     0,
                     0,
                     0,
                     0));

  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_SATURATION,
                           endpoint_id_node);

  // Try to move the Hue with Mode = 0x02
  // 5.2.2.3.5.2: The Move Mode field SHALL be one of the non-reserved values (0x02 Reserved)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    move_hue_command(node_unid,
                                     endpoint_id,
                                     UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                     0x02,
                                     0x05,
                                     0,
                                     0));

  // Try to move the Hue with rate = 0
  // 5.2.2.3.5.3: If the Rate field has a value of zero, the command has no effect and a default response command (see Chapter 2) is
  // sent in response, with the status code set to INVALID_FIELD.
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    move_hue_command(node_unid,
                                     endpoint_id,
                                     UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                     ZCL_CC_MOVE_MODE_UP,
                                     0,
                                     0,
                                     0));

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(current_hue_node, DESIRED_ATTRIBUTE));

  // Check ZCL_CC_MOVE_MODE_UP:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    move_hue_command(node_unid,
                                     endpoint_id,
                                     UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                     ZCL_CC_MOVE_MODE_UP,
                                     2,
                                     3,
                                     4));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(current_hue_node, DESIRED_ATTRIBUTE));

  uint8_t desired_hue = 0x00;
  attribute_store_get_desired(current_hue_node,
                              &desired_hue,
                              sizeof(desired_hue));
  TEST_ASSERT_EQUAL(0xFE, desired_hue);

  // Check ZCL_CC_MOVE_MODE_DOWN:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    move_hue_command(node_unid,
                                     endpoint_id,
                                     UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                     ZCL_CC_MOVE_MODE_DOWN,
                                     2,
                                     3,
                                     4));
  attribute_store_get_desired(current_hue_node,
                              &desired_hue,
                              sizeof(desired_hue));
  TEST_ASSERT_EQUAL(0x00, desired_hue);

  // Check ZCL_CC_MOVE_MODE_STOP:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    move_hue_command(node_unid,
                                     endpoint_id,
                                     UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                     ZCL_CC_MOVE_MODE_UP,
                                     1,
                                     3,
                                     4));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    move_hue_command(node_unid,
                                     endpoint_id,
                                     UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                     ZCL_CC_MOVE_MODE_STOP,
                                     1,
                                     3,
                                     4));
  attribute_store_get_desired(current_hue_node,
                              &desired_hue,
                              sizeof(desired_hue));
  uint32_t reported_hue = 0x00;
  attribute_store_get_reported(current_hue_node,
                               &reported_hue,
                               sizeof(reported_hue));
  TEST_ASSERT_EQUAL(desired_hue, reported_hue);
}

void test_step_hue_command()
{
  TEST_ASSERT_NOT_NULL(step_hue_command);
  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    step_hue_command(node_unid,
                     endpoint_id,
                     UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                     0,
                     0,
                     0,
                     0,
                     0));

  // Add the Current Hue attribute (still not supported, we need both Hue and Saturation)
  attribute_store_node_t current_hue_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_HUE,
                               endpoint_id_node);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    step_hue_command(node_unid,
                     endpoint_id,
                     UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                     0,
                     0,
                     0,
                     0,
                     0));

  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_SATURATION,
                           endpoint_id_node);

  // 5.2.2.3.6.2 The Step Mode field SHALL be one of the non-reserved values (0x00, 0x02 Reserved)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    step_hue_command(node_unid,
                                     endpoint_id,
                                     UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                     0x02,
                                     1,
                                     2,
                                     3,
                                     4));

  uint8_t reported_hue = 0x00;
  uint8_t desired_hue  = 0x00;
  // Try to set ZCL_CC_STEP_MODE_UP:
  uint8_t step = 1;
  attribute_store_get_reported(current_hue_node,
                               &reported_hue,
                               sizeof(reported_hue));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    step_hue_command(node_unid,
                                     endpoint_id,
                                     UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                     ZCL_CC_STEP_MODE_UP,
                                     step,
                                     2,
                                     3,
                                     4));
  attribute_store_get_desired(current_hue_node,
                              &desired_hue,
                              sizeof(desired_hue));
  TEST_ASSERT_EQUAL(desired_hue, reported_hue + step);
}

void test_move_to_saturation_command()
{
  TEST_ASSERT_NOT_NULL(move_to_saturation_command);
  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    move_to_saturation_command(node_unid,
                               endpoint_id,
                               UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                               0,
                               0,
                               0,
                               0));

  // Create hue/saturation attributes, and capabilities
  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_HUE,
                           endpoint_id_node);
  attribute_store_node_t current_saturation_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_SATURATION,
    endpoint_id_node);
  attribute_store_node_t capabilities_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_CAPABILITIES,
    endpoint_id_node);
  uint16_t capabilities = 0;  // Still not supported
  attribute_store_set_reported(capabilities_node,
                               &capabilities,
                               sizeof(capabilities));

  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    move_to_saturation_command(node_unid,
                               endpoint_id,
                               UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                               0,
                               0,
                               0,
                               0));

  // Now it is supported:
  capabilities = 1;
  attribute_store_set_reported(capabilities_node,
                               &capabilities,
                               sizeof(capabilities));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_to_saturation_command(node_unid,
                               endpoint_id,
                               UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                               0,
                               0,
                               0,
                               0));

  // Try to set the Hue:
  TEST_ASSERT_FALSE(attribute_store_is_value_defined(current_saturation_node,
                                                     DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_to_saturation_command(node_unid,
                               endpoint_id,
                               UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                               0,
                               0,
                               0,
                               0));
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(current_saturation_node,
                                                    DESIRED_ATTRIBUTE));

  // Try again with value 0xFF
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_to_saturation_command(node_unid,
                               endpoint_id,
                               UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                               0xFF,
                               2,
                               3,
                               4));
  TEST_ASSERT_TRUE(attribute_store_is_value_defined(current_saturation_node,
                                                    DESIRED_ATTRIBUTE));
  uint8_t desired_saturation = 0x00;
  attribute_store_get_desired(current_saturation_node,
                              &desired_saturation,
                              sizeof(desired_saturation));
  TEST_ASSERT_EQUAL(0xFE, desired_saturation);
}

void test_move_saturation_command()
{
  TEST_ASSERT_NOT_NULL(move_saturation_command);
  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    move_saturation_command(node_unid,
                            endpoint_id,
                            UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                            0,
                            0,
                            0,
                            0));

  // Add the Current Hue attribute (still not supported, we need both Hue and Saturation)
  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_HUE,
                           endpoint_id_node);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    move_saturation_command(node_unid,
                            endpoint_id,
                            UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                            0,
                            0,
                            0,
                            0));

  attribute_store_node_t current_saturation_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_SATURATION,
    endpoint_id_node);

  // Try to move the Saturation with Mode = 0x02
  // 5.2.2.3.5.2: The Move Mode field SHALL be one of the non-reserved values (0x02 Reserved)
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    move_saturation_command(node_unid,
                            endpoint_id,
                            UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                            0x02,
                            0x05,
                            0,
                            0));

  // Try to move the Saturation with rate = 0
  // 5.2.2.3.5.3: If the Rate field has a value of zero, the command has no effect and a default response command (see Chapter 2) is
  // sent in response, with the status code set to INVALID_FIELD.
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    move_saturation_command(node_unid,
                            endpoint_id,
                            UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                            ZCL_CC_MOVE_MODE_UP,
                            0,
                            0,
                            0));

  TEST_ASSERT_FALSE(attribute_store_is_value_defined(current_saturation_node,
                                                     DESIRED_ATTRIBUTE));

  // Check ZCL_CC_MOVE_MODE_UP:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_saturation_command(node_unid,
                            endpoint_id,
                            UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                            ZCL_CC_MOVE_MODE_UP,
                            2,
                            3,
                            4));

  TEST_ASSERT_TRUE(attribute_store_is_value_defined(current_saturation_node,
                                                    DESIRED_ATTRIBUTE));

  uint8_t desired_saturation = 0x00;
  attribute_store_get_desired(current_saturation_node,
                              &desired_saturation,
                              sizeof(desired_saturation));
  TEST_ASSERT_EQUAL(0xFE, desired_saturation);

  // Check ZCL_CC_MOVE_MODE_DOWN:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_saturation_command(node_unid,
                            endpoint_id,
                            UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                            ZCL_CC_MOVE_MODE_DOWN,
                            2,
                            3,
                            4));
  attribute_store_get_desired(current_saturation_node,
                              &desired_saturation,
                              sizeof(desired_saturation));
  TEST_ASSERT_EQUAL(0x00, desired_saturation);

  // Check ZCL_CC_MOVE_MODE_STOP:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_saturation_command(node_unid,
                            endpoint_id,
                            UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                            ZCL_CC_MOVE_MODE_UP,
                            1,
                            3,
                            4));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_saturation_command(node_unid,
                            endpoint_id,
                            UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                            ZCL_CC_MOVE_MODE_STOP,
                            1,
                            3,
                            4));
  attribute_store_get_desired(current_saturation_node,
                              &desired_saturation,
                              sizeof(desired_saturation));
  uint8_t reported_saturation = 0x00;
  attribute_store_get_reported(current_saturation_node,
                               &reported_saturation,
                               sizeof(reported_saturation));
  TEST_ASSERT_EQUAL(desired_saturation, reported_saturation);
}

void test_step_saturation_command()
{
  TEST_ASSERT_NOT_NULL(step_saturation_command);

  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    step_saturation_command(node_unid,
                            endpoint_id,
                            UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                            0,
                            0,
                            0,
                            0,
                            0));

  // Add the Current Hue attribute (still not supported, we need both Hue and Saturation)
  attribute_store_node_t current_saturation_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_SATURATION,
    endpoint_id_node);
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    step_saturation_command(node_unid,
                            endpoint_id,
                            UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                            0,
                            0,
                            0,
                            0,
                            0));

  attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_SATURATION,
                           endpoint_id_node);

  // 5.2.2.3.6.2 The Step Mode field SHALL be one of the non-reserved values (0x00, 0x02 Reserved)
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    step_saturation_command(node_unid,
                            endpoint_id,
                            UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                            0x02,
                            1,
                            2,
                            3,
                            4));

  uint8_t reported_saturation = 0x00;
  uint8_t desired_saturation  = 0x00;
  // Try to set ZCL_CC_STEP_MODE_UP:
  uint8_t step = 1;
  attribute_store_get_reported(current_saturation_node,
                               &reported_saturation,
                               sizeof(reported_saturation));
  desired_saturation = reported_saturation + step;

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    step_saturation_command(node_unid,
                            endpoint_id,
                            UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                            ZCL_CC_STEP_MODE_UP,
                            step,
                            2,
                            3,
                            4));
  attribute_store_get_desired(current_saturation_node,
                              &desired_saturation,
                              sizeof(desired_saturation));
  TEST_ASSERT_EQUAL(desired_saturation, reported_saturation + step);
}

void test_move_to_hue_and_saturation_command()
{
  TEST_ASSERT_NOT_NULL(move_to_hue_and_saturation_command);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    move_to_hue_and_saturation_command(
                      node_unid,
                      endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0,
                      0));

  // Add the Current Hue attribute (still not supported, we need both Hue and Saturation)
  attribute_store_node_t current_hue_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_HUE,
                               endpoint_id_node);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    move_to_hue_and_saturation_command(
                      node_unid,
                      endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0,
                      0));

  attribute_store_node_t current_saturation_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_SATURATION,
    endpoint_id_node);

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(current_hue_node, DESIRED_ATTRIBUTE));

  // set saturation and hue
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_to_hue_and_saturation_command(node_unid,
                                       endpoint_id,
                                       UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                       0x02,
                                       0x05,
                                       1,
                                       0,
                                       0));

  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(current_hue_node, DESIRED_ATTRIBUTE));

  uint8_t desired_hue_value = 0x00;
  attribute_store_get_desired(current_hue_node,
                              &desired_hue_value,
                              sizeof(desired_hue_value));
  TEST_ASSERT_EQUAL(0x02, desired_hue_value);

  TEST_ASSERT_TRUE(attribute_store_is_value_defined(current_saturation_node,
                                                    DESIRED_ATTRIBUTE));

  uint8_t desired_saturation_value = 0x00;
  attribute_store_get_desired(current_saturation_node,
                              &desired_saturation_value,
                              sizeof(desired_saturation_value));
  TEST_ASSERT_EQUAL(0x05, desired_saturation_value);
}

void test_move_to_color_temperature_command()
{
  TEST_ASSERT_NOT_NULL(move_to_color_temperature_command);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    move_to_color_temperature_command(
                      node_unid,
                      endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  attribute_store_node_t capabilities_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_CAPABILITIES,
    endpoint_id_node);
  uint16_t capabilities = 0x10;
  attribute_store_set_reported(capabilities_node,
                               &capabilities,
                               sizeof(capabilities));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    move_to_color_temperature_command(
                      node_unid,
                      endpoint_id,
                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                      0,
                      0,
                      0,
                      0));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_to_color_temperature_command(node_unid,
                                      endpoint_id,
                                      UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                      0,
                                      0,
                                      0,
                                      0));
}

void test_move_color_temperature_command()
{
  TEST_ASSERT_NOT_NULL(move_color_temperature_command);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    move_color_temperature_command(node_unid,
                                   endpoint_id,
                                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                   0,
                                   0,
                                   0,
                                   0,
                                   0,
                                   0));

  attribute_store_node_t capabilities_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_CAPABILITIES,
    endpoint_id_node);
  uint16_t capabilities = 0x10;
  attribute_store_set_reported(capabilities_node,
                               &capabilities,
                               sizeof(capabilities));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_color_temperature_command(node_unid,
                                   endpoint_id,
                                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                   0,
                                   0,
                                   0,
                                   0,
                                   0,
                                   0));

  // Rate = 0
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_color_temperature_command(node_unid,
                                   endpoint_id,
                                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                   0,
                                   0,
                                   0,
                                   0,
                                   0,
                                   0));

  // Rate > 0, move STOP
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_color_temperature_command(node_unid,
                                   endpoint_id,
                                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                   ZCL_CC_MOVE_MODE_STOP,
                                   5,
                                   0,
                                   0,
                                   0,
                                   0));

  // Rate > 0, move UP
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_color_temperature_command(node_unid,
                                   endpoint_id,
                                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                   ZCL_CC_MOVE_MODE_UP,
                                   5,
                                   0,
                                   0,
                                   0,
                                   0));

  // Rate > 0, move DOWN
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_color_temperature_command(node_unid,
                                   endpoint_id,
                                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                   ZCL_CC_MOVE_MODE_DOWN,
                                   5,
                                   0,
                                   0,
                                   0,
                                   0));

  // Rate > 0, move UNKNOWN
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    move_color_temperature_command(node_unid,
                                   endpoint_id,
                                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                   ZCL_CC_MOVE_MODE_DOWN + 5,
                                   5,
                                   0,
                                   0,
                                   0,
                                   0));
}

void test_step_color_temperature_command()
{
  TEST_ASSERT_NOT_NULL(step_color_temperature_command);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    step_color_temperature_command(node_unid,
                                   endpoint_id,
                                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                   0,
                                   0,
                                   0,
                                   0,
                                   0,
                                   0,
                                   0));

  attribute_store_node_t capabilities_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_CAPABILITIES,
    endpoint_id_node);
  uint16_t capabilities = 0x10;
  attribute_store_set_reported(capabilities_node,
                               &capabilities,
                               sizeof(capabilities));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    step_color_temperature_command(node_unid,
                                   endpoint_id,
                                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                                   0,
                                   0,
                                   0,
                                   0,
                                   0,
                                   0,
                                   0));

  // Step size 0
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    step_color_temperature_command(node_unid,
                                   endpoint_id,
                                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                   0,
                                   0,
                                   0,
                                   0,
                                   0,
                                   0,
                                   0));

  // Step size >0, step mode UP
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    step_color_temperature_command(node_unid,
                                   endpoint_id,
                                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                   ZCL_CC_STEP_MODE_UP,
                                   4,
                                   0,
                                   0,
                                   0,
                                   0,
                                   0));

  // Step size >0, step mode DOWN
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    step_color_temperature_command(node_unid,
                                   endpoint_id,
                                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                   ZCL_CC_STEP_MODE_DOWN,
                                   4,
                                   0,
                                   0,
                                   0,
                                   0,
                                   0));

  // Step size >0, step mode UNKONWN
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    step_color_temperature_command(node_unid,
                                   endpoint_id,
                                   UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                   ZCL_CC_STEP_MODE_DOWN + 1,
                                   4,
                                   0,
                                   0,
                                   0,
                                   0,
                                   0));
}

void test_stop_move_step_command()
{
  TEST_ASSERT_NOT_NULL(stop_move_step_command);

  // First test the support (not supported by default)
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    stop_move_step_command(node_unid,
                           endpoint_id,
                           UIC_MQTT_DOTDOT_CALLBACK_TYPE_SUPPORT_CHECK,
                           0,
                           0));

  attribute_store_node_t capabilities_node = attribute_store_add_node(
    DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_COLOR_CAPABILITIES,
    endpoint_id_node);
  uint16_t capabilities = 1;
  attribute_store_set_reported(capabilities_node,
                               &capabilities,
                               sizeof(capabilities));

  attribute_store_node_t current_hue_node
    = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_COLOR_CONTROL_CURRENT_HUE,
                               endpoint_id_node);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    stop_move_step_command(node_unid,
                                           endpoint_id,
                                           UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL,
                                           0,
                                           0));

  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(current_hue_node, DESIRED_ATTRIBUTE));
}