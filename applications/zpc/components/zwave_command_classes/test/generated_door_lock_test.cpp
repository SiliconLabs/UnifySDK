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
#include <stdlib.h>
#include "workaround_for_test.hpp"

extern "C" {
#include "unity.h"
#include "attribute_store_mock.h"
#include "attribute_store_helper_mock.h"
#include "attribute_store_defined_attribute_types.h"
#include "attribute_resolver_mock.h"
#include "attribute_resolver_rule_mock.h"
#include "zwave_command_handler_mock.h"
#include "zpc_attribute_store_network_helper_mock.h"
#include "attribute_store_defined_attribute_types.h"
#include "zwave_unid_mock.h"

// Command Class specific
#include "zwave_COMMAND_CLASS_DOOR_LOCK_handlers.h"
#include "zwave_COMMAND_CLASS_DOOR_LOCK_attribute_id.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

struct functions {
  attribute_resolver_function_t set;
  attribute_resolver_function_t get;
};

std::map<attribute_store_type_t, functions> rules;
std::map<uint32_t, zwave_command_handler_t> handlers;
std::map<uint32_t, attribute_store_node_changed_callback_t> ats_callbacks;

static sl_status_t get_function_ptrs(attribute_store_type_t node_type,
                                     attribute_resolver_function_t set_func,
                                     attribute_resolver_function_t get_func,
                                     int cmock_num_calls)
{
  functions funcs = {set_func, get_func};
  rules.insert(std::make_pair(node_type, funcs));
  return SL_STATUS_OK;
}

static sl_status_t get_handler(zwave_command_handler_t h, int cmock_num_calls)
{
  handlers.insert(std::make_pair(h.command_class, h));
  return SL_STATUS_OK;
}

static sl_status_t
  get_ats_callbacks(attribute_store_node_changed_callback_t callback_function,
                    attribute_store_type_t type,
                    int cmock_num_calls)
{
  ats_callbacks.insert(std::make_pair(type, callback_function));
  return SL_STATUS_OK;
}

/// Called before each and every test
void setUp()
{
  rules.clear();
  handlers.clear();
  ats_callbacks.clear();
  attribute_resolver_register_rule_Stub(get_function_ptrs);
  zwave_command_handler_register_handler_Stub(get_handler);
  attribute_store_register_callback_by_type_Stub(get_ats_callbacks);
}

/// Report frame with testing
void test_zwave_DOOR_LOCK_CAPABILITIES_REPORT()
{
#if (COMMAND_CLASS_DOOR_LOCK_VERSION < 4)
  TEST_IGNORE();
#else
  zwave_COMMAND_CLASS_DOOR_LOCK_init();
  // Pure test data :) Standard bitmask_length and list_length is smaller
  uint8_t test_frame[] = {
    COMMAND_CLASS_DOOR_LOCK,
    0x08,  // DOOR_LOCK_CAPABILITIES_REPORT
    0x06,  // SUPPORTED_OPERATION_TYPE_BIT_MASK_LENGTH
    0xde,
    0xad,
    0xbe,
    0xef,
    0xca,
    0xfe,  // SUPPORTED_OPERATION_TYPE_BIT_MASK
    0x05,  // SUPPORTED_DOOR_LOCK_MODE_LIST_LENGTH
    0x11,
    0x12,
    0x13,
    0x14,
    0x15,  // SUPPORTED_DOOR_LOCK_MODE
    0xcb,  // INSIDE_HANDLE_MODES + OUTSIDE_HANDLE_MODES
    0x03   // SUPPORTED_DOOR_COMPONENTS
  };

  zwave_controller_connection_info_t c = {};
  c.remote.node_id                     = 23;
  c.remote.endpoint_id                 = 2;

  unid_t test_unid                     = "zw39485";
  attribute_store_node_t endpoint_node = 0x3485;
  attribute_store_node_t node_id_node  = 0x98;

  // Receive a Report Command
  zwave_unid_from_node_id_Expect(c.remote.node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    c.remote.endpoint_id,
    endpoint_node);

  // 1
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_CAPABILITIES_SUPPORTED_OPERATION_TYPE_BIT_MASK_LENGTH,
    0,
    node_id_node);
  // 2
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_CAPABILITIES_SUPPORTED_OPERATION_TYPE_BIT_MASK,
    0,
    node_id_node);
  // 3
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_CAPABILITIES_SUPPORTED_DOOR_LOCK_MODE_LIST_LENGTH,
    0,
    node_id_node);
  // 4
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_CAPABILITIES_SUPPORTED_DOOR_LOCK_MODE,
    0,
    node_id_node);
  // 5
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_CAPABILITIES_SUPPORTED_INSIDE_HANDLE_MODES_BITMASK,
    0,
    node_id_node);
  // 6
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_CAPABILITIES_SUPPORTED_OUTSIDE_HANDLE_MODES_BITMASK,
    0,
    node_id_node);
  // 7
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_CAPABILITIES_SUPPORTED_DOOR_COMPONENTS,
    0,
    node_id_node);

  uint8_t values[4] = {0x06, 0x00, 0x00, 0x00};
  // ATTRIBUTE_DOOR_LOCK_CAPABILITIES_SUPPORTED_OPERATION_TYPE_BIT_MASK_LENGTH
  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           REPORTED_ATTRIBUTE,
                                                           &values[0],
                                                           sizeof(int32_t),
                                                           SL_STATUS_OK);

  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           DESIRED_ATTRIBUTE,
                                                           0,
                                                           0,
                                                           SL_STATUS_OK);

  // value = test_frame[3];
  // ATTRIBUTE_DOOR_LOCK_CAPABILITIES_SUPPORTED_OPERATION_TYPE_BIT_MASK
  attribute_store_set_node_attribute_value_ExpectWithArrayAndReturn(
    node_id_node,
    REPORTED_ATTRIBUTE,
    &test_frame[3],
    test_frame[2] & 0x1f,
    sizeof(int8_t[test_frame[2] & 0x1f]),
    SL_STATUS_OK);

  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           DESIRED_ATTRIBUTE,
                                                           0,
                                                           0,
                                                           SL_STATUS_OK);

  uint32_t value_1 = test_frame[9];
  // ATTRIBUTE_DOOR_LOCK_CAPABILITIES_SUPPORTED_DOOR_LOCK_MODE_LIST_LENGTH
  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           REPORTED_ATTRIBUTE,
                                                           (uint8_t *)&value_1,
                                                           sizeof(int32_t),
                                                           SL_STATUS_OK);

  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           DESIRED_ATTRIBUTE,
                                                           0,
                                                           0,
                                                           SL_STATUS_OK);

  // value = test_frame[4];
  // ATTRIBUTE_DOOR_LOCK_CAPABILITIES_SUPPORTED_DOOR_LOCK_MODE
  attribute_store_set_node_attribute_value_ExpectWithArrayAndReturn(
    node_id_node,
    REPORTED_ATTRIBUTE,
    &test_frame[10],
    test_frame[9],
    sizeof(int8_t[test_frame[9]]),
    SL_STATUS_OK);

  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           DESIRED_ATTRIBUTE,
                                                           0,
                                                           0,
                                                           SL_STATUS_OK);

  uint8_t values_bm1[] = {0, 0, 0, 0};
  values_bm1[0]        = test_frame[15] & 0x0f;
  // ATTRIBUTE_DOOR_LOCK_CAPABILITIES_SUPPORTED_INSIDE_HANDLE_MODES_BITMASK
  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           REPORTED_ATTRIBUTE,
                                                           &values_bm1[0],
                                                           sizeof(int32_t),
                                                           SL_STATUS_OK);

  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           DESIRED_ATTRIBUTE,
                                                           0,
                                                           0,
                                                           SL_STATUS_OK);

  uint8_t values_bm2[] = {0, 0, 0, 0};
  values_bm2[0]        = (test_frame[15] & 0xf0) >> 4;
  // ATTRIBUTE_DOOR_LOCK_CAPABILITIES_SUPPORTED_OUTSIDE_HANDLE_MODES_BITMASK
  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           REPORTED_ATTRIBUTE,
                                                           &values_bm2[0],
                                                           sizeof(int32_t),
                                                           SL_STATUS_OK);

  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           DESIRED_ATTRIBUTE,
                                                           0,
                                                           0,
                                                           SL_STATUS_OK);

  uint32_t value_2 = test_frame[16];
  // ATTRIBUTE_DOOR_LOCK_CAPABILITIES_SUPPORTED_DOOR_COMPONENTS
  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           REPORTED_ATTRIBUTE,
                                                           (uint8_t *)&value_2,
                                                           sizeof(int32_t),
                                                           SL_STATUS_OK);

  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           DESIRED_ATTRIBUTE,
                                                           0,
                                                           0,
                                                           SL_STATUS_OK);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handlers[COMMAND_CLASS_DOOR_LOCK]
                      .control_handler(&c, test_frame, sizeof(test_frame)));
#endif
}

/// Report frame with WORD, CONST, BYTE, BITFIELDS
void test_zwave_DOOR_LOCK_CONFIGURATION_REPORT()
{
#if (COMMAND_CLASS_DOOR_LOCK_VERSION < 4)
  TEST_IGNORE();
#else

  zwave_COMMAND_CLASS_DOOR_LOCK_init();
  // Pure test data
  uint8_t test_frame[] = {COMMAND_CLASS_DOOR_LOCK,
                          0x06,  // DOOR_LOCK_CONFIGURATION_REPORT
                          0x07,  // CONFIGURATION_OPERATION_TYPE
                          0xde,  // INSIDE_HANDLE_MODES | OUTSIDE_HANDLE_MODES
                          0x05,  // LOCK_TIMEOUT_MINUTES
                          0x11,  // LOCK_TIMEOUT_SECONDS
                          0xcb,  // AUTO_RELOCK_TIME MSB
                          0x03,  // AUTO_RELOCK_TIME LSB
                          0x04,  // HOLD_AND_RELEASE_TIME MSB
                          0x05,  // HOLD_AND_RELEASE_TIME LSB
                          0x00};

  zwave_controller_connection_info_t c = {};
  c.remote.node_id                     = 23;
  c.remote.endpoint_id                 = 2;

  unid_t test_unid                     = "zw39485";
  attribute_store_node_t endpoint_node = 0x3485;
  attribute_store_node_t node_id_node  = 0x98;

  // Receive a Report Command
  zwave_unid_from_node_id_Expect(c.remote.node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    c.remote.endpoint_id,
    endpoint_node);

  // 0
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OPERATION_TYPE,
    0,
    node_id_node);
  // 1
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_INSIDE_DOOR_HANDLES_STATE,
    0,
    node_id_node);
  // 2
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OUTSIDE_DOOR_HANDLES_STATE,
    0,
    node_id_node);
  // 3
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_MINUTES,
    0,
    node_id_node);
  // 4
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_SECONDS,
    0,
    node_id_node);
  // 5
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_AUTO_RELOCK_TIME,
    0,
    node_id_node);
  // 6
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_HOLD_AND_RELEASE_TIME,
    0,
    node_id_node);

  uint32_t value_0 = test_frame[2];
  // ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OPERATION_TYPE
  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           REPORTED_ATTRIBUTE,
                                                           (uint8_t *)&value_0,
                                                           sizeof(int32_t),
                                                           SL_STATUS_OK);
  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           DESIRED_ATTRIBUTE,
                                                           0,
                                                           0,
                                                           SL_STATUS_OK);

  uint32_t value_1 = test_frame[3] & 0x0f;
  // ATTRIBUTE_DOOR_LOCK_CONFIGURATION_INSIDE_DOOR_HANDLES_STATE
  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           REPORTED_ATTRIBUTE,
                                                           (uint8_t *)&value_1,
                                                           sizeof(int32_t),
                                                           SL_STATUS_OK);
  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           DESIRED_ATTRIBUTE,
                                                           0,
                                                           0,
                                                           SL_STATUS_OK);

  uint32_t value_2 = (test_frame[3] & 0xf0) >> 4;
  // ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OUTSIDE_DOOR_HANDLES_STATE
  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           REPORTED_ATTRIBUTE,
                                                           (uint8_t *)&value_2,
                                                           sizeof(int32_t),
                                                           SL_STATUS_OK);
  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           DESIRED_ATTRIBUTE,
                                                           0,
                                                           0,
                                                           SL_STATUS_OK);

  uint32_t value_3 = test_frame[4];
  // ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_MINUTES
  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           REPORTED_ATTRIBUTE,
                                                           (uint8_t *)&value_3,
                                                           sizeof(int32_t),
                                                           SL_STATUS_OK);
  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           DESIRED_ATTRIBUTE,
                                                           0,
                                                           0,
                                                           SL_STATUS_OK);

  uint32_t value_4 = test_frame[5];
  // ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_SECONDS
  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           REPORTED_ATTRIBUTE,
                                                           (uint8_t *)&value_4,
                                                           sizeof(int32_t),
                                                           SL_STATUS_OK);
  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           DESIRED_ATTRIBUTE,
                                                           0,
                                                           0,
                                                           SL_STATUS_OK);

  uint32_t value_5 = (test_frame[6] << 8) | test_frame[7];
  // ATTRIBUTE_DOOR_LOCK_CONFIGURATION_AUTO_RELOCK_TIME
  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           REPORTED_ATTRIBUTE,
                                                           (uint8_t *)&value_5,
                                                           sizeof(int32_t),
                                                           SL_STATUS_OK);
  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           DESIRED_ATTRIBUTE,
                                                           0,
                                                           0,
                                                           SL_STATUS_OK);

  uint32_t value_6 = (test_frame[8] << 8) | test_frame[9];
  // ATTRIBUTE_DOOR_LOCK_CONFIGURATION_HOLD_AND_RELEASE_TIME
  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           REPORTED_ATTRIBUTE,
                                                           (uint8_t *)&value_6,
                                                           sizeof(int32_t),
                                                           SL_STATUS_OK);
  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           DESIRED_ATTRIBUTE,
                                                           0,
                                                           0,
                                                           SL_STATUS_OK);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handlers[COMMAND_CLASS_DOOR_LOCK]
                      .control_handler(&c, test_frame, sizeof(test_frame)));
#endif
}

// Simple GET
void test_zwave_generator_DOOR_LOCK_CAPABILITY_GET()
{
#if (COMMAND_CLASS_DOOR_LOCK_VERSION < 4)
  TEST_IGNORE();
#else
  uint8_t frame[255] = {};
  uint16_t frame_len;
  attribute_store_node_t node = 0x42;

  zwave_COMMAND_CLASS_DOOR_LOCK_init();
  attribute_store_get_first_parent_with_type_ExpectAndReturn(
    node,
    ATTRIBUTE_ENDPOINT_ID,
    node);
  attribute_store_get_node_parent_ExpectAndReturn(0x42, SL_STATUS_OK);
  rules[ATTRIBUTE_DOOR_LOCK_CAPABILITIES_SUPPORTED_OPERATION_TYPE_BIT_MASK].get(
    node,
    frame,
    &frame_len);

  TEST_ASSERT_EQUAL(2, frame_len);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_DOOR_LOCK, frame[0]);
  TEST_ASSERT_EQUAL(0x07, frame[1]);
#endif
}

// Door Lock Configuration Set Command
// uint8_t frame[] = {0x62, 0x04, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x00}
void test_zwave_generator_DOOR_LOCK_CONFIGURATION_SET()
{
#if (COMMAND_CLASS_DOOR_LOCK_VERSION < 4)
  TEST_IGNORE();
#else
  uint8_t frame[255] = {0};
  uint16_t frame_len;
  attribute_store_node_t node         = 0x00;
  attribute_store_node_t node_id_node = 0x98;

  static attribute_store_node_t
    my_test_wakeup_interval_in_attribute_store_update
    = 0x0098;

  static zwave_node_id_t my_test_node_id = 0x01;
  static uint8_t my_test_node_id_size    = sizeof(zwave_node_id_t);

  static uint8_t my_test_wakeup_interval[]
    = {0xab, 0xcd, 0xef, 0x67};  //1 seconds
  static uint8_t my_test_wakeup_interval_value_size
    = sizeof(my_test_wakeup_interval);

  zwave_COMMAND_CLASS_DOOR_LOCK_init();
  attribute_store_get_first_parent_with_type_ExpectAndReturn(
    node,
    ATTRIBUTE_ENDPOINT_ID,
    node);
  attribute_store_get_node_parent_ExpectAndReturn(0x00, SL_STATUS_OK);

  // 0
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OPERATION_TYPE,
    0,
    node_id_node);

  // 1
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_INSIDE_DOOR_HANDLES_STATE,
    0,
    node_id_node);

  // 2
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OUTSIDE_DOOR_HANDLES_STATE,
    0,
    node_id_node);

  // 3
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_MINUTES,
    0,
    node_id_node);

  // 4
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_SECONDS,
    0,
    node_id_node);

  // 5
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_AUTO_RELOCK_TIME,
    0,
    node_id_node);

  // 6
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    node,
    ATTRIBUTE_DOOR_LOCK_CONFIGURATION_HOLD_AND_RELEASE_TIME,
    0,
    node_id_node);

  // ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OPERATION_TYPE
  uint32_t test_value0 = 0x12;
  attribute_store_get_node_attribute_value_ExpectAndReturn(
    my_test_wakeup_interval_in_attribute_store_update,
    DESIRED_ATTRIBUTE,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnArrayThruPtr_value(
    (uint8_t *)&test_value0,
    4);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &my_test_wakeup_interval_value_size);

  // ATTRIBUTE_DOOR_LOCK_CONFIGURATION_INSIDE_DOOR_HANDLES_STATE
  uint32_t test_value1 = 0x04;

  attribute_store_get_node_attribute_value_ExpectAndReturn(
    my_test_wakeup_interval_in_attribute_store_update,
    DESIRED_ATTRIBUTE,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnArrayThruPtr_value(
    (uint8_t *)&test_value1,
    4);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &my_test_wakeup_interval_value_size);

  // ATTRIBUTE_DOOR_LOCK_CONFIGURATION_OUTSIDE_DOOR_HANDLES_STATE
  uint32_t test_value2 = 0x03;

  attribute_store_get_node_attribute_value_ExpectAndReturn(
    my_test_wakeup_interval_in_attribute_store_update,
    DESIRED_ATTRIBUTE,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnArrayThruPtr_value(
    (uint8_t *)&test_value2,
    4);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &my_test_wakeup_interval_value_size);

  // ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_MINUTES
  uint32_t test_value3 = 0x56;
  attribute_store_get_node_attribute_value_ExpectAndReturn(
    my_test_wakeup_interval_in_attribute_store_update,
    DESIRED_ATTRIBUTE,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnArrayThruPtr_value(
    (uint8_t *)&test_value3,
    4);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &my_test_wakeup_interval_value_size);

  // ATTRIBUTE_DOOR_LOCK_CONFIGURATION_LOCK_TIMEOUT_SECONDS
  uint32_t test_value4 = 0x78;
  attribute_store_get_node_attribute_value_ExpectAndReturn(
    my_test_wakeup_interval_in_attribute_store_update,
    DESIRED_ATTRIBUTE,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnArrayThruPtr_value(
    (uint8_t *)&test_value4,
    4);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &my_test_wakeup_interval_value_size);

  // ATTRIBUTE_DOOR_LOCK_CONFIGURATION_AUTO_RELOCK_TIME
  uint32_t test_value5 = 0xbc9a;
  attribute_store_get_node_attribute_value_ExpectAndReturn(
    my_test_wakeup_interval_in_attribute_store_update,
    DESIRED_ATTRIBUTE,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnArrayThruPtr_value(
    (uint8_t *)&test_value5,
    4);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &my_test_wakeup_interval_value_size);

  // ATTRIBUTE_DOOR_LOCK_CONFIGURATION_HOLD_AND_RELEASE_TIME
  uint32_t test_value6 = 0xf0de;

  attribute_store_get_node_attribute_value_ExpectAndReturn(
    my_test_wakeup_interval_in_attribute_store_update,
    DESIRED_ATTRIBUTE,
    NULL,
    NULL,
    SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnArrayThruPtr_value(
    (uint8_t *)&test_value6,
    4);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &my_test_wakeup_interval_value_size);

  rules[ATTRIBUTE_DOOR_LOCK_CONFIGURATION_INSIDE_DOOR_HANDLES_STATE].set(
    0x00,
    frame,
    &frame_len);

  TEST_ASSERT_EQUAL(11, frame_len);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_DOOR_LOCK, frame[0]);
  TEST_ASSERT_EQUAL(0x04, frame[1]);
  TEST_ASSERT_EQUAL(0x12, frame[2]);  // OPERATION_TYPE
  TEST_ASSERT_EQUAL(
    0x34,
    frame[3]);  // INSIDE_DOOR_HANDLES_STATE | OUTSIDE_DOOR_HANDLES_STATE
  TEST_ASSERT_EQUAL(0x56, frame[4]);   // LOCK_TIMEOUT_MINUTES
  TEST_ASSERT_EQUAL(0x78, frame[5]);   // LOCK_TIMEOUT_SECONDS
  TEST_ASSERT_EQUAL(0x9a, frame[6]);   // AUTO_RELOCK_TIME MSB
  TEST_ASSERT_EQUAL(0xbc, frame[7]);   // AUTO_RELOCK_TIME LSB
  TEST_ASSERT_EQUAL(0xde, frame[8]);   // HOLD_AND_RELEASE_TIME MSB
  TEST_ASSERT_EQUAL(0xf0, frame[9]);   // HOLD_AND_RELEASE_TIME LSB
  TEST_ASSERT_EQUAL(0x00, frame[10]);  // 0x00
#endif
}
}
