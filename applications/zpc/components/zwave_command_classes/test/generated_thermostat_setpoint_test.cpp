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
#include "zwave_COMMAND_CLASS_THERMOSTAT_SETPOINT_handlers.h"
#include "zwave_COMMAND_CLASS_THERMOSTAT_SETPOINT_attribute_id.h"

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
std::map<uint32_t, attribute_store_node_update_callback_t> ats_callbacks;

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
  get_ats_callbacks(attribute_store_node_update_callback_t callback_function,
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

// Get with parameter ATTRIBUTE_THERMOSTAT_SETPOINT_CAPABILITIES_SETPOINT_TYPE
void test_zwave_THERMOSTAT_SETPOINT_CAPABILITIES_GET()
{
  uint8_t frame[255]                       = {0};
  uint16_t frame_len                       = 0;
  attribute_store_node_t node              = 0x00;
  static attribute_store_node_t test_value = 0x00;

  zwave_COMMAND_CLASS_THERMOSTAT_SETPOINT_init();
  attribute_store_get_first_parent_with_type_ExpectAndReturn(
    node,
    ATTRIBUTE_ENDPOINT_ID,
    node);

  // 1
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    node,
    ATTRIBUTE_THERMOSTAT_SETPOINT_CAPABILITIES_SETPOINT_TYPE,
    0,
    0);

  // ATTRIBUTE_THERMOSTAT_SETPOINT_CAPABILITIES_SETPOINT_TYPE
  uint32_t test_value0    = 0x0b;
  uint8_t test_value_size = 4;

  attribute_store_get_node_attribute_value_ExpectAndReturn(test_value,
                                                           REPORTED_ATTRIBUTE,
                                                           NULL,
                                                           NULL,
                                                           SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnArrayThruPtr_value(
    (uint8_t *)&test_value0,
    4);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &test_value_size);

  attribute_store_get_node_parent_ExpectAndReturn(0x00, SL_STATUS_OK);
  rules[ATTRIBUTE_THERMOSTAT_SETPOINT_CAPABILITIES_MIN_VALUE].get(node,
                                                                  frame,
                                                                  &frame_len);

  TEST_ASSERT_EQUAL(3, frame_len);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_THERMOSTAT_SETPOINT, frame[0]);
  TEST_ASSERT_EQUAL(0x09, frame[1]);
  TEST_ASSERT_EQUAL(0x0b, frame[2]);
}

// Get with parameter ATTRIBUTE_THERMOSTAT_SETPOINT_SETPOINT_TYPE
void test_zwave_THERMOSTAT_SETPOINT_GET()
{
  uint8_t frame[255]                       = {0};
  uint16_t frame_len                       = 0;
  attribute_store_node_t node              = 0x00;
  static attribute_store_node_t test_value = 0x00;

  zwave_COMMAND_CLASS_THERMOSTAT_SETPOINT_init();
  attribute_store_get_first_parent_with_type_ExpectAndReturn(
    node,
    ATTRIBUTE_ENDPOINT_ID,
    node);

  // 1
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    node,
    ATTRIBUTE_THERMOSTAT_SETPOINT_SETPOINT_TYPE,
    0,
    0);

  // ATTRIBUTE_THERMOSTAT_SETPOINT_SETPOINT_TYPE
  uint32_t test_value0    = 0xfa;
  uint8_t test_value_size = 4;

  attribute_store_get_node_attribute_value_ExpectAndReturn(test_value,
                                                           REPORTED_ATTRIBUTE,
                                                           NULL,
                                                           NULL,
                                                           SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnArrayThruPtr_value(
    (uint8_t *)&test_value0,
    4);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &test_value_size);

  attribute_store_get_node_parent_ExpectAndReturn(0x00, SL_STATUS_OK);
  rules[ATTRIBUTE_THERMOSTAT_SETPOINT_VALUE].get(node, frame, &frame_len);

  TEST_ASSERT_EQUAL(3, frame_len);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_THERMOSTAT_SETPOINT, frame[0]);
  TEST_ASSERT_EQUAL(0x02, frame[1]);
  TEST_ASSERT_EQUAL(0x0a, frame[2]);
}

void test_zwave_THERMOSTAT_SETPOINT_REPORT()
{
  uint8_t test_frame[] = {
    COMMAND_CLASS_THERMOSTAT_SETPOINT,
    0x03,  // THERMOSTAT_SETPOINT_REPORT
    0xfb,  // THERMOSTAT_SETPOINT_SETPOINT_TYPE (0x0f mask)
    0x34,  // SETPOINT_PRECISION (3 bits) | SETPOINT_SCALE (2 bits) | VALUE_SIZE (3 bits)
    0xab,  // VALUE_0
    0xcd,  // VALUE_1
    0xef,  // VALUE_2
    0x12   // VALUE_3
  };

  zwave_controller_connection_info_t c = {};
  c.remote.node_id                     = 23;
  c.remote.endpoint_id                 = 2;

  unid_t test_unid                     = "zw39485";
  attribute_store_node_t endpoint_node = 0x85;
  attribute_store_node_t node_id_node  = 0x98;

  zwave_COMMAND_CLASS_THERMOSTAT_SETPOINT_init();

  // Receive a Report Command
  zwave_unid_from_node_id_Expect(c.remote.node_id, NULL);
  zwave_unid_from_node_id_IgnoreArg_unid();
  zwave_unid_from_node_id_ReturnMemThruPtr_unid(test_unid, sizeof(unid_t));

  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(
    test_unid,
    c.remote.endpoint_id,
    endpoint_node);

  // 0
  uint8_t value_0 = test_frame[2] & 0x0f;

  attribute_store_get_node_child_by_value_ExpectAndReturn(
    endpoint_node,                                // node
    ATTRIBUTE_THERMOSTAT_SETPOINT_SETPOINT_TYPE,  // child type
    REPORTED_ATTRIBUTE,                           // value state
    &value_0,                                     // value
    1,                                            // value size
    0,                                            // child index
    endpoint_node);                               // return value

  // 1
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_THERMOSTAT_SETPOINT_SCALE,
    0,
    node_id_node);
  // 2
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_THERMOSTAT_SETPOINT_PRECISION,
    0,
    node_id_node);
  // 3
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_THERMOSTAT_SETPOINT_VALUE,
    0,
    node_id_node);

  uint32_t value_1 = (test_frame[3] & 0x18) >> 3;
  // ATTRIBUTE_THERMOSTAT_SETPOINT_SCALE
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

  uint32_t value_2 = (test_frame[3] & 0xE0) >> 5;
  // ATTRIBUTE_THERMOSTAT_SETPOINT_PRECISION
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

  uint32_t value_3 = test_frame[7] << 24 | test_frame[6] << 16
                     | test_frame[5] << 8 | test_frame[4];
  // ATTRIBUTE_THERMOSTAT_SETPOINT_VALUE
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

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handlers[COMMAND_CLASS_THERMOSTAT_SETPOINT]
                      .control_handler(&c, test_frame, sizeof(test_frame)));
}

// Thermostat Setpoint Set Command
// uint8_t frame[] = {0x43, 0x01, 0x0b, 0x34, 0xab, 0xcd, 0xef, 0x12}
void test_zwave_THERMOSTAT_SETPOINT_SET()
{
  uint8_t frame[255] = {0};
  uint16_t frame_len;
  attribute_store_node_t node         = 0x00;
  attribute_store_node_t node_id_node = 0x00;

  static attribute_store_node_t
    my_test_wakeup_interval_in_attribute_store_update
    = 0x00;

  static zwave_node_id_t my_test_node_id = 0x01;
  static uint8_t my_test_node_id_size    = sizeof(zwave_node_id_t);

  static uint8_t my_test_wakeup_interval[]
    = {0xab, 0xcd, 0xef, 0x67};  //1 seconds

  static uint8_t my_test_wakeup_interval_value_size
    = sizeof(my_test_wakeup_interval);

  zwave_COMMAND_CLASS_THERMOSTAT_SETPOINT_init();

  // attribute_store_get_node_parent_ExpectAndReturn(0x00, SL_STATUS_OK);
  attribute_store_get_first_parent_with_type_ExpectAndReturn(
    node,
    ATTRIBUTE_ENDPOINT_ID,
    node_id_node);
  attribute_store_get_node_parent_ExpectAndReturn(0x00, SL_STATUS_OK);
  // 4
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    node,
    ATTRIBUTE_THERMOSTAT_SETPOINT_SCALE,
    0,
    node_id_node);

  // 5
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    node,
    ATTRIBUTE_THERMOSTAT_SETPOINT_PRECISION,
    0,
    node_id_node);

  // 6
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    node,
    ATTRIBUTE_THERMOSTAT_SETPOINT_VALUE,
    0,
    node_id_node);

  // ATTRIBUTE_THERMOSTAT_SETPOINT_SETPOINT_TYPE
  uint32_t test_value3 = 0x0b;
  attribute_store_get_node_attribute_value_ExpectAndReturn(
    my_test_wakeup_interval_in_attribute_store_update,
    REPORTED_ATTRIBUTE,
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

  // ATTRIBUTE_THERMOSTAT_SETPOINT_SCALE
  uint32_t test_value4 = 0x02;
  attribute_store_get_node_attribute_value_ExpectAndReturn(
    my_test_wakeup_interval_in_attribute_store_update,
    DESIRED_OR_REPORTED_ATTRIBUTE,
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

  // ATTRIBUTE_THERMOSTAT_SETPOINT_PRECISION
  uint32_t test_value5 = 0x01;
  attribute_store_get_node_attribute_value_ExpectAndReturn(
    my_test_wakeup_interval_in_attribute_store_update,
    DESIRED_OR_REPORTED_ATTRIBUTE,
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

  // ATTRIBUTE_THERMOSTAT_SETPOINT_VALUE
  uint32_t test_value6 = 0x12efcdab;

  attribute_store_get_node_attribute_value_ExpectAndReturn(
    my_test_wakeup_interval_in_attribute_store_update,
    DESIRED_OR_REPORTED_ATTRIBUTE,
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

  rules[ATTRIBUTE_THERMOSTAT_SETPOINT_VALUE].set(0x00, frame, &frame_len);

  TEST_ASSERT_EQUAL(8, frame_len);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_THERMOSTAT_SETPOINT, frame[0]);
  TEST_ASSERT_EQUAL(0x01, frame[1]);  // THERMOSTAT_SETPOINT_SET
  TEST_ASSERT_EQUAL(0x0b,
                    frame[2]);  // THERMOSTAT_SETPOINT_SETPOINT_TYPE (0x0f mask)
  TEST_ASSERT_EQUAL(
    0x34,
    frame
      [3]);  // SETPOINT_PRECISION (3 bits) | SETPOINT_SCALE (2 bits) | VALUE_SIZE (3 bits)
  TEST_ASSERT_EQUAL(0xab, frame[4]);  // VALUE_0
  TEST_ASSERT_EQUAL(0xcd, frame[5]);  // VALUE_1
  TEST_ASSERT_EQUAL(0xef, frame[6]);  // VALUE_2
  TEST_ASSERT_EQUAL(0x12, frame[7]);  // VALUE_3
}

void test_zwave_THERMOSTAT_SETPOINT_ADD_SETPOINT_TYPE()
{
  attribute_store_node_t node             = 0x89;
  attribute_store_node_t attr_id_endpoint = 90;
  attribute_store_node_t new_node         = 0x73;
  uint8_t setpoint_type                   = 0x0b;  // Energy Save Heating

  zwave_COMMAND_CLASS_THERMOSTAT_SETPOINT_init();

  attribute_store_node_t test_node_id_node = 23;
  // 0
  attribute_store_get_first_parent_with_type_ExpectAndReturn(
    node,
    ATTRIBUTE_ENDPOINT_ID,
    test_node_id_node);
  // 1
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    test_node_id_node,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VERSION,
    0,
    test_node_id_node);
  attribute_store_add_node_ExpectAndReturn(
    ATTRIBUTE_THERMOSTAT_SETPOINT_SETPOINT_TYPE,
    node,
    attr_id_endpoint);
  uint8_t value_1 = setpoint_type;
  // ATTRIBUTE_THERMOSTAT_SETPOINT_SETPOINT_TYPE
  attribute_store_set_node_attribute_value_ExpectAndReturn(attr_id_endpoint,
                                                           REPORTED_ATTRIBUTE,
                                                           &value_1,
                                                           sizeof(int8_t),
                                                           SL_STATUS_OK);
  // ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VERSION
  uint8_t test_value_1      = 0x01;
  uint8_t test_value_1_size = 1;

  attribute_store_get_node_attribute_value_ExpectAndReturn(test_node_id_node,
                                                           REPORTED_ATTRIBUTE,
                                                           NULL,
                                                           NULL,
                                                           SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnArrayThruPtr_value(
    (uint8_t *)&test_value_1,
    1);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &test_value_1_size);

  // ATTRIBUTE_THERMOSTAT_SETPOINT_SCALE
  // We don't have this attribute yet and should add it
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    attr_id_endpoint,
    ATTRIBUTE_THERMOSTAT_SETPOINT_SCALE,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);

  attribute_store_add_node_ExpectAndReturn(ATTRIBUTE_THERMOSTAT_SETPOINT_SCALE,
                                           attr_id_endpoint,
                                           new_node);

  // ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VERSION
  uint8_t test_value_2      = 0x02;
  uint8_t test_value_2_size = 1;

  attribute_store_get_node_attribute_value_ExpectAndReturn(test_node_id_node,
                                                           REPORTED_ATTRIBUTE,
                                                           NULL,
                                                           NULL,
                                                           SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnArrayThruPtr_value(
    (uint8_t *)&test_value_2,
    1);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &test_value_2_size);

  // ATTRIBUTE_THERMOSTAT_SETPOINT_PRECISION
  // We already have this attribute and shouldn't add it
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    attr_id_endpoint,
    ATTRIBUTE_THERMOSTAT_SETPOINT_PRECISION,
    0,
    test_node_id_node);

  // ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_VERSION
  uint8_t test_value_3      = 0x03;
  uint8_t test_value_3_size = 1;

  attribute_store_get_node_attribute_value_ExpectAndReturn(test_node_id_node,
                                                           REPORTED_ATTRIBUTE,
                                                           NULL,
                                                           NULL,
                                                           SL_STATUS_OK);
  attribute_store_get_node_attribute_value_IgnoreArg_value();
  attribute_store_get_node_attribute_value_IgnoreArg_value_size();
  attribute_store_get_node_attribute_value_ReturnArrayThruPtr_value(
    (uint8_t *)&test_value_3,
    1);
  attribute_store_get_node_attribute_value_ReturnThruPtr_value_size(
    &test_value_3_size);

  // ATTRIBUTE_THERMOSTAT_SETPOINT_VALUE
  // We don't have this attribute yet and should add it
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    attr_id_endpoint,
    ATTRIBUTE_THERMOSTAT_SETPOINT_VALUE,
    0,
    ATTRIBUTE_STORE_INVALID_NODE);

  attribute_store_add_node_ExpectAndReturn(ATTRIBUTE_THERMOSTAT_SETPOINT_VALUE,
                                           attr_id_endpoint,
                                           new_node);

  zwave_COMMAND_CLASS_THERMOSTAT_SETPOINT_add_THERMOSTAT_SETPOINT_SETPOINT_TYPE(
    node,
    setpoint_type);
}
}