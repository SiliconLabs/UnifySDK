/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "zwave_command_class_door_lock_control.h"
// #include "zwave_command_classes_utils.h"
// #include "unity.h"
#include "zwave_COMMAND_CLASS_DOOR_LOCK_handlers.h"
#include "zwave_COMMAND_CLASS_DOOR_LOCK_attribute_id.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"

// Interface includes
#include "attribute_store_defined_attribute_types.h"
// #include "ZW_classcmd.h"

// ZPC includes
#include "zpc_attribute_store_type_registration.h"
// #include "zwave_utils.h"
// #include "zwave_controller_types.h"
// #include "attribute_store_defined_attribute_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "attribute_resolver_mock.h"
// #include "zpc_attribute_resolver_mock.h"
#include "zwave_command_handler_mock.h"

// #include "dotdot_mqtt_mock.h"
// #include "dotdot_mqtt_generated_commands_mock.h"

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_##type

// Static variables
static attribute_resolver_function_t operation_get        = NULL;
static attribute_resolver_function_t operation_set        = NULL;
static attribute_resolver_function_t configuration_get    = NULL;
static attribute_resolver_function_t configuration_set    = NULL;
static attribute_resolver_function_t capabilities_get     = NULL;
static zwave_command_handler_t door_lock_handler          = {};
static zwave_controller_connection_info_t connection_info = {};

static uint8_t received_frame[255]  = {};
static uint16_t received_frame_size = 0;

static uint8_t u8_value   = 0;
static uint32_t u32_value = 0;

void check_uint8_reported_attr(attribute_store_node_t endpoint,
                               attribute_store_type_t attr_type,
                               const uint8_t *incoming_frame)
{
  attribute_store_node_t node
    = attribute_store_get_first_child_by_type(endpoint, attr_type);

  TEST_ASSERT_TRUE(attribute_store_node_exists(node));
  uint8_t frame_size = 0;
  attribute_store_get_node_attribute_value(node,
                                           REPORTED_ATTRIBUTE,
                                           received_frame,
                                           &frame_size);
  TEST_ASSERT_EQUAL(1, frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(incoming_frame, received_frame, frame_size);
}

void check_uint16_reported_attr(attribute_store_node_t endpoint,
                                attribute_store_type_t attr_type,
                                const uint16_t *incoming_frame)
{
  attribute_store_node_t node
    = attribute_store_get_first_child_by_type(endpoint, attr_type);

  TEST_ASSERT_TRUE(attribute_store_node_exists(node));
  uint8_t frame_size = 0;
  attribute_store_get_node_attribute_value(node,
                                           REPORTED_ATTRIBUTE,
                                           received_frame,
                                           &frame_size);
  TEST_ASSERT_EQUAL(2, frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(incoming_frame, received_frame, frame_size);
}

void check_int32_reported_attr(attribute_store_node_t endpoint,
                               attribute_store_type_t attr_type,
                               const int32_t *incoming_frame)
{
  attribute_store_node_t node
    = attribute_store_get_first_child_by_type(endpoint, attr_type);

  TEST_ASSERT_TRUE(attribute_store_node_exists(node));
  uint8_t frame_size = 0;
  attribute_store_get_node_attribute_value(node,
                                           REPORTED_ATTRIBUTE,
                                           received_frame,
                                           &frame_size);
  TEST_ASSERT_EQUAL(4, frame_size);
  TEST_ASSERT_EQUAL_INT8_ARRAY(incoming_frame, &received_frame, 1);
}

// Stub functions
static sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  switch (node_type) {
    case ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_LOCK_MODE:
      operation_get = get_func;
      operation_set = set_func;
      break;
    case ATTRIBUTE_DOOR_LOCK_OPERATION_INSIDE_DOOR_HANDLES_MODE:
    case ATTRIBUTE_DOOR_LOCK_OPERATION_OUTSIDE_DOOR_HANDLES_MODE:
    case ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_CONDITION:
    case ATTRIBUTE_DOOR_LOCK_OPERATION_LOCK_TIMEOUT_MINUTES:
    case ATTRIBUTE_DOOR_LOCK_OPERATION_LOCK_TIMEOUT_SECONDS:
    case ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT_TARGET_MODE:
    case ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT_DURATION:
      operation_get = get_func;
      TEST_ASSERT_NULL(set_func);
      break;
    case ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OPERATION_TYPE:
    case ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OUTSIDE_DOOR_HANDLES_STATE:
    case ATTRIBUTE_DOOR_LOCK_CONFIGURATION_INSIDE_DOOR_HANDLES_STATE:
    case ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_MINUTES:
    case ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_SECONDS:
    case ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_AUTO_RELOCK:
    case ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_HOLD_RELEASE_TIME:
    case ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_BTB:
    case ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_TA:
      configuration_get = get_func;
      configuration_set = set_func;
      break;
    case ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES_OPERATION_TYPE_BITMASK:
    case ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES_LOCK_MODE:
    case ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_OUTSIDE_HANDLE_MODES_BITMASK:
    case ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_INSIDE_HANDLE_MODES_BITMASK:
    case ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_DOOR_COMPONENT_BITMASK:
    case ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_ARS:
    case ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_HRS:
    case ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_TAS:
    case ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_BTBS:
      capabilities_get = get_func;
      TEST_ASSERT_NULL(set_func);
      break;
    default:
      TEST_FAIL_MESSAGE("Attribute rule registration on a wrong type");
      break;
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  door_lock_handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME,
                    door_lock_handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_DOOR_LOCK_V4,
                    door_lock_handler.command_class);
  TEST_ASSERT_EQUAL(DOOR_LOCK_VERSION_V4, door_lock_handler.version);
  TEST_ASSERT_NOT_NULL(door_lock_handler.control_handler);
  TEST_ASSERT_NULL(door_lock_handler.support_handler);
  TEST_ASSERT_FALSE(door_lock_handler.manual_security_validation);

  return SL_STATUS_OK;
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
  zpc_attribute_store_test_helper_create_network();
  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size = 0;
  u8_value            = 0;
  u32_value           = 0;
  operation_get       = NULL;
  operation_set       = NULL;
  configuration_get   = NULL;
  configuration_set   = NULL;
  capabilities_get    = NULL;
  memset(&door_lock_handler, 0, sizeof(zwave_command_handler_t));

  // Prepare a default connection info with the supporting node
  memset(&connection_info, 0, sizeof(zwave_controller_connection_info_t));
  attribute_store_get_reported(node_id_node,
                               &(connection_info.remote.node_id),
                               sizeof(connection_info.remote.node_id));
  attribute_store_get_reported(endpoint_id_node,
                               &(connection_info.remote.endpoint_id),
                               sizeof(connection_info.remote.endpoint_id));

  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  //Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_door_lock_init());
}

void test_door_lock_incoming_unknown_command()
{
  TEST_ASSERT_NOT_NULL(door_lock_handler.control_handler);
  uint8_t incoming_frame[] = {COMMAND_CLASS_DOOR_LOCK_V4, 0xFE};

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    door_lock_handler.control_handler(&connection_info,
                                                      incoming_frame,
                                                      sizeof(incoming_frame)));
}

void test_door_lock_version_attribute_created()
{
  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(endpoint_id_node));
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_VERSION,
                               endpoint_id_node);

  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  zwave_cc_version_t version = 0;
  attribute_store_set_reported(version_node, &version, sizeof(version));
  TEST_ASSERT_EQUAL(1, attribute_store_get_node_child_count(endpoint_id_node));

  version = 1;
  attribute_store_set_reported(version_node, &version, sizeof(version));
  TEST_ASSERT_EQUAL(1 + 11,
                    attribute_store_get_node_child_count(endpoint_id_node));
}

void test_door_lock_operation_set()
{
  const int32_t door_lock_mode = 10;
  attribute_store_node_t mode_node
    = attribute_store_add_node(ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_LOCK_MODE,
                               endpoint_id_node);
  attribute_store_set_desired(mode_node,
                              &door_lock_mode,
                              sizeof(door_lock_mode));

  TEST_ASSERT_NOT_NULL(operation_set);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    operation_set(mode_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_DOOR_LOCK_V4, DOOR_LOCK_OPERATION_SET_V4, door_lock_mode};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_door_lock_operation_get()
{
  TEST_ASSERT_NOT_NULL(operation_get);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    operation_get(ATTRIBUTE_STORE_INVALID_NODE,
                                  received_frame,
                                  &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_DOOR_LOCK_V4, DOOR_LOCK_OPERATION_GET_V4};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(&expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_door_lock_incoming_operation_report_happy_case()
{
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_VERSION,
                               endpoint_id_node);

  zwave_cc_version_t version = 4;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  TEST_ASSERT_NOT_NULL(door_lock_handler.control_handler);
  uint8_t incoming_frame[] = {COMMAND_CLASS_DOOR_LOCK_V4,
                              DOOR_LOCK_OPERATION_REPORT_V4,
                              0x11,
                              0xAB,
                              0x03,
                              0x55,
                              0x44,
                              0x10,
                              0x07};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    door_lock_handler.control_handler(&connection_info,
                                                      incoming_frame,
                                                      sizeof(incoming_frame)));
  int32_t mode = (int32_t)incoming_frame[2];
  check_int32_reported_attr(endpoint_id_node,
                            ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_LOCK_MODE,
                            &mode);

  uint8_t handles = incoming_frame[3];
  TEST_ASSERT_EQUAL(0xAB, handles);
  int32_t outside_handles = (handles & 0xF0) >> 4;
  check_int32_reported_attr(
    endpoint_id_node,
    ATTRIBUTE_DOOR_LOCK_OPERATION_OUTSIDE_DOOR_HANDLES_MODE,
    &outside_handles);

  int32_t inside_handles = handles & 0b1111;
  TEST_ASSERT_EQUAL(0x0B, inside_handles);
  check_int32_reported_attr(
    endpoint_id_node,
    ATTRIBUTE_DOOR_LOCK_OPERATION_INSIDE_DOOR_HANDLES_MODE,
    &inside_handles);

  int32_t condition = (int32_t)incoming_frame[4];
  check_int32_reported_attr(endpoint_id_node,
                            ATTRIBUTE_DOOR_LOCK_OPERATION_DOOR_CONDITION,
                            &condition);

  int32_t minutes = (int32_t)incoming_frame[5];
  check_int32_reported_attr(endpoint_id_node,
                            ATTRIBUTE_DOOR_LOCK_OPERATION_LOCK_TIMEOUT_MINUTES,
                            &minutes);

  int32_t seconds = (int32_t)incoming_frame[6];
  check_int32_reported_attr(endpoint_id_node,
                            ATTRIBUTE_DOOR_LOCK_OPERATION_LOCK_TIMEOUT_SECONDS,
                            &seconds);

  attribute_store_node_t operation_node
    = attribute_store_get_first_child_by_type(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT);

  attribute_store_node_t node = attribute_store_get_first_child_by_type(
    operation_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT_TARGET_MODE);

  TEST_ASSERT_TRUE(attribute_store_node_exists(node));

  check_uint8_reported_attr(
    operation_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT_TARGET_MODE,
    &incoming_frame[7]);

  check_uint8_reported_attr(
    operation_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_OPERATION_REPORT_DURATION,
    &incoming_frame[8]);
}

void test_door_lock_configuration_set()
{
  TEST_ASSERT_NOT_NULL(configuration_set);

  ZW_DOOR_LOCK_CONFIGURATION_SET_V4_FRAME expected_frame;
  expected_frame.cmdClass            = COMMAND_CLASS_DOOR_LOCK_V4;
  expected_frame.cmd                 = DOOR_LOCK_CONFIGURATION_SET_V4;
  expected_frame.operationType       = 0x01;
  expected_frame.properties1         = 0x11;
  expected_frame.lockTimeoutMinutes  = 0x15;
  expected_frame.lockTimeoutSeconds  = 0x20;
  expected_frame.autoRelockTime1     = 0x10;
  expected_frame.autoRelockTime2     = 0x15;
  expected_frame.holdAndReleaseTime1 = 0x25;
  expected_frame.holdAndReleaseTime2 = 0x30;
  expected_frame.properties2         = 0x03;

  int32_t operationType = (int32_t)expected_frame.operationType;
  attribute_store_node_t operation_type_node
    = attribute_store_add_node(ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OPERATION_TYPE,
                               endpoint_id_node);
  attribute_store_set_desired(operation_type_node,
                              &operationType,
                              sizeof(operationType));

  attribute_store_node_t outside_handles_node = attribute_store_add_node(
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OUTSIDE_DOOR_HANDLES_STATE,
    endpoint_id_node);
  int32_t outside_handles = expected_frame.properties1 >> 4;
  attribute_store_set_desired(outside_handles_node,
                              &outside_handles,
                              sizeof(outside_handles));

  attribute_store_node_t inside_handles_node = attribute_store_add_node(
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_INSIDE_DOOR_HANDLES_STATE,
    endpoint_id_node);
  int32_t inside_handles = expected_frame.properties1 & 0b1111;
  attribute_store_set_desired(inside_handles_node,
                              &inside_handles,
                              sizeof(inside_handles));

  int32_t lockTimeoutMinutes = (int32_t)expected_frame.lockTimeoutMinutes;
  attribute_store_node_t timeout_min_node = attribute_store_add_node(
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_MINUTES,
    endpoint_id_node);
  attribute_store_set_desired(timeout_min_node,
                              &lockTimeoutMinutes,
                              sizeof(lockTimeoutMinutes));

  int32_t lockTimeoutSeconds = (int32_t)expected_frame.lockTimeoutSeconds;
  attribute_store_node_t timeout_sec_node = attribute_store_add_node(
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_SECONDS,
    endpoint_id_node);
  attribute_store_set_desired(timeout_sec_node,
                              &lockTimeoutSeconds,
                              sizeof(lockTimeoutSeconds));

  attribute_store_node_t configuration_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION,
                               endpoint_id_node);

  TEST_ASSERT_TRUE(attribute_store_node_exists(configuration_node));

  attribute_store_node_t relock_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_AUTO_RELOCK,
    configuration_node);
  uint16_t relock
    = (expected_frame.autoRelockTime1 << 8) | expected_frame.autoRelockTime2;
  attribute_store_set_desired(relock_node, &relock, sizeof(relock));

  attribute_store_node_t release_time_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_HOLD_RELEASE_TIME,
    configuration_node);
  uint16_t release_time = (expected_frame.holdAndReleaseTime1 << 8)
                          | expected_frame.holdAndReleaseTime2;
  attribute_store_set_desired(release_time_node,
                              &release_time,
                              sizeof(release_time));

  attribute_store_node_t btb_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_BTB,
    configuration_node);

  uint8_t btb = ((expected_frame.properties2 & 0b10) >> 1);
  attribute_store_set_desired(btb_node, &btb, sizeof(btb));

  attribute_store_node_t ta_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_TA,
    configuration_node);
  uint8_t ta = expected_frame.properties2 & 1;
  attribute_store_set_desired(ta_node, &ta, sizeof(ta));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    configuration_set(ta_node, received_frame, &received_frame_size));
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(&expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_door_lock_configuration_get()
{
  TEST_ASSERT_NOT_NULL(configuration_get);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    configuration_get(ATTRIBUTE_STORE_INVALID_NODE,
                                      received_frame,
                                      &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_DOOR_LOCK_V4, DOOR_LOCK_CONFIGURATION_GET_V4};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(&expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_door_lock_incoming_configuration_report_happy_case()
{
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_VERSION,
                               endpoint_id_node);

  zwave_cc_version_t version = 4;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  TEST_ASSERT_NOT_NULL(door_lock_handler.control_handler);
  uint8_t incoming_frame[] = {COMMAND_CLASS_DOOR_LOCK_V4,
                              DOOR_LOCK_CONFIGURATION_REPORT_V4,
                              0x01,
                              0xAB,
                              0x07,
                              0x55,
                              0x04,
                              0x05,
                              0x07,
                              0x01,
                              0x03};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    door_lock_handler.control_handler(&connection_info,
                                                      incoming_frame,
                                                      sizeof(incoming_frame)));

  int32_t oper_type = (int32_t)incoming_frame[2];
  check_int32_reported_attr(endpoint_id_node,
                            ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OPERATION_TYPE,
                            &oper_type);

  int32_t outside_handles = incoming_frame[3] >> 4;
  check_int32_reported_attr(
    endpoint_id_node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OUTSIDE_DOOR_HANDLES_STATE,
    &outside_handles);

  int32_t inside_handles = incoming_frame[3] & 0b1111;
  check_int32_reported_attr(
    endpoint_id_node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_INSIDE_DOOR_HANDLES_STATE,
    &inside_handles);

  int32_t minutes = incoming_frame[4];
  check_int32_reported_attr(
    endpoint_id_node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_MINUTES,
    &minutes);

  int32_t seconds = incoming_frame[5];
  check_int32_reported_attr(
    endpoint_id_node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_SECONDS,
    &seconds);

  attribute_store_node_t configuration_node
    = attribute_store_get_first_child_by_type(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION);

  TEST_ASSERT_TRUE(attribute_store_node_exists(configuration_node));

  uint16_t auto_relock
    = (((uint16_t)incoming_frame[6]) << 8) | (uint16_t)incoming_frame[7];
  check_uint16_reported_attr(
    configuration_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_AUTO_RELOCK,
    &auto_relock);

  uint16_t hold_release
    = (((uint16_t)incoming_frame[8]) << 8) | incoming_frame[9];
  check_uint16_reported_attr(
    configuration_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_HOLD_RELEASE_TIME,
    &hold_release);

  uint8_t btb = (incoming_frame[10] & 0b10) >> 1;
  check_uint8_reported_attr(configuration_node,
                            ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_BTB,
                            &btb);

  uint8_t ta = incoming_frame[10] & 0b1;
  check_uint8_reported_attr(configuration_node,
                            ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CONFIGURATION_TA,
                            &ta);
}

void test_door_lock_capabtilies_get()
{
  TEST_ASSERT_NOT_NULL(capabilities_get);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    capabilities_get(ATTRIBUTE_STORE_INVALID_NODE,
                                     received_frame,
                                     &received_frame_size));

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_DOOR_LOCK_V4, DOOR_LOCK_CAPABILITIES_GET_V4};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_door_lock_incoming_capabilities_report_happy_case()
{
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_VERSION,
                               endpoint_id_node);

  zwave_cc_version_t version = 4;
  attribute_store_set_reported(version_node, &version, sizeof(version));

  TEST_ASSERT_NOT_NULL(door_lock_handler.control_handler);
  uint8_t incoming_frame[] = {COMMAND_CLASS_DOOR_LOCK_V4,
                              DOOR_LOCK_CAPABILITIES_REPORT_V4,
                              0x04,
                              0x01,
                              0x01,
                              0x01,
                              0x01,
                              0x02,
                              0xFF,
                              0xFF,
                              0x88,
                              0x44,
                              0xFF};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    door_lock_handler.control_handler(&connection_info,
                                                      incoming_frame,
                                                      sizeof(incoming_frame)));

  attribute_store_node_t capabilities_node
    = attribute_store_get_first_child_by_type(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES);

  attribute_store_node_t operation_bitmask_node
    = attribute_store_get_first_child_by_type(
      capabilities_node,
      ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES_OPERATION_TYPE_BITMASK);

  uint8_t frame_size = 0;
  TEST_ASSERT_TRUE(attribute_store_node_exists(operation_bitmask_node));
  attribute_store_get_node_attribute_value(operation_bitmask_node,
                                           REPORTED_ATTRIBUTE,
                                           received_frame,
                                           &frame_size);
  TEST_ASSERT_EQUAL(4, frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(&incoming_frame[3], received_frame, frame_size);

  attribute_store_node_t mode_list = attribute_store_get_first_child_by_type(
    capabilities_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOCK_CAPABILITIES_LOCK_MODE);

  TEST_ASSERT_TRUE(attribute_store_node_exists(mode_list));

  attribute_store_get_node_attribute_value(mode_list,
                                           REPORTED_ATTRIBUTE,
                                           received_frame,
                                           &frame_size);
  TEST_ASSERT_EQUAL(2, frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(&incoming_frame[8], received_frame, frame_size);

  uint8_t handle         = incoming_frame[10];
  uint8_t outside_handle = handle >> 4;
  check_uint8_reported_attr(
    capabilities_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_OUTSIDE_HANDLE_MODES_BITMASK,
    &outside_handle);

  uint8_t inside_handle = handle & 0b1111;
  check_uint8_reported_attr(
    capabilities_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_INSIDE_HANDLE_MODES_BITMASK,
    &inside_handle);

  check_uint8_reported_attr(
    capabilities_node,
    ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_DOOR_COMPONENT_BITMASK,
    &incoming_frame[11]);

  uint8_t props = incoming_frame[12];
  uint8_t ars   = (props & 0b1000) >> 3;
  check_uint8_reported_attr(capabilities_node,
                            ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_ARS,
                            &ars);

  uint8_t hrs = (props & 0b100) >> 2;
  check_uint8_reported_attr(capabilities_node,
                            ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_HRS,
                            &hrs);

  uint8_t tas = (props & 0b10) >> 1;
  check_uint8_reported_attr(capabilities_node,
                            ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_TAS,
                            &tas);

  uint8_t btbs = props & 0b1;
  check_uint8_reported_attr(capabilities_node,
                            ATTRIBUTE_COMMAND_CLASS_DOOR_LOOK_CAPABILITIES_BTBS,
                            &btbs);
}
