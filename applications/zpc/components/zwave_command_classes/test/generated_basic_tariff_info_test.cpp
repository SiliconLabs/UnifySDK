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
#include "zwave_COMMAND_CLASS_BASIC_TARIFF_INFO_handlers.h"
#include "zwave_COMMAND_CLASS_BASIC_TARIFF_INFO_attribute_id.h"

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

/// Report frame with DWORD, BYTE, BITFIELDS
void test_zwave_BASIC_TARIFF_INFO_REPORT()
{
  zwave_COMMAND_CLASS_BASIC_TARIFF_INFO_init();

  uint8_t test_frame[] = {
    COMMAND_CLASS_BASIC_TARIFF_INFO,
    0x02,  // BASIC_TARIFF_INFO_REPORT
    0xf5,  // TOTAL_NO_IMPORT_RATES (0x0f mask)
    0xf3,  // E_CURRENT_RATE_IN_USE (0x0f mask)
    0x12,  // E_RATE_CONSUMPTION_REGISTER MSB
    0x34,  // E_RATE_CONSUMPTION_REGISTER
    0x56,  // E_RATE_CONSUMPTION_REGISTER
    0x78,  // E_RATE_CONSUMPTION_REGISTER LSB
    0x04,  // E_TIME_FOR_NEXT_RATE_HOURS
    0x14,  // E_TIME_FOR_NEXT_RATE_MINUTES
    0x22,  // E_TIME_FOR_NEXT_RATE_SECONDS
    0xf4,  // E_CURRENT_RATE_IN_USE (0x0f mask)
    0x9a,  // E_RATE_CONSUMPTION_REGISTER MSB
    0xbc,  // E_RATE_CONSUMPTION_REGISTER
    0xde,  // E_RATE_CONSUMPTION_REGISTER
    0xf0   // E_RATE_CONSUMPTION_REGISTER LSB
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

  // 0
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_BASIC_TARIFF_INFO_TOTAL_NO_IMPORT_RATES,
    0,
    node_id_node);
  // 1
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_BASIC_TARIFF_INFO_E_CURRENT_RATE_IN_USE,
    0,
    node_id_node);
  // 2
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_BASIC_TARIFF_INFO_E_RATE_CONSUMPTION_REGISTER,
    0,
    node_id_node);
  // 3
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_BASIC_TARIFF_INFO_E_TIME_FOR_NEXT_RATE_HOURS,
    0,
    node_id_node);
  // 4
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_BASIC_TARIFF_INFO_E_TIME_FOR_NEXT_RATE_MINUTES,
    0,
    node_id_node);
  // 5
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_BASIC_TARIFF_INFO_E_TIME_FOR_NEXT_RATE_SECONDS,
    0,
    node_id_node);
  // 6
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_BASIC_TARIFF_INFO_E_CURRENT_RATE_IN_USE,
    0,
    node_id_node);
  // 7
  attribute_store_get_node_child_by_type_ExpectAndReturn(
    endpoint_node,
    ATTRIBUTE_BASIC_TARIFF_INFO_E_RATE_CONSUMPTION_REGISTER,
    0,
    node_id_node);

  uint32_t value_0 = test_frame[2] & 0x0f;
  // ATTRIBUTE_BASIC_TARIFF_INFO_TOTAL_NO_IMPORT_RATES
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
  // ATTRIBUTE_BASIC_TARIFF_INFO_E_CURRENT_RATE_IN_USE
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

  uint32_t value_2 = (test_frame[4] << 24) | (test_frame[5] << 16)
                     | (test_frame[6] << 8) | (test_frame[7]);
  // ATTRIBUTE_BASIC_TARIFF_INFO_E_RATE_CONSUMPTION_REGISTER
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

  uint32_t value_3 = test_frame[8];
  // ATTRIBUTE_BASIC_TARIFF_INFO_E_TIME_FOR_NEXT_RATE_HOURS
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

  uint32_t value_4 = test_frame[9];
  // ATTRIBUTE_BASIC_TARIFF_INFO_E_TIME_FOR_NEXT_RATE_MINUTES
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

  uint32_t value_5 = test_frame[10];
  // ATTRIBUTE_BASIC_TARIFF_INFO_E_TIME_FOR_NEXT_RATE_SECONDS
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

  uint32_t value_6 = test_frame[11] & 0x0f;
  // ATTRIBUTE_BASIC_TARIFF_INFO_E_CURRENT_RATE_IN_USE
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

  uint32_t value_7 = (test_frame[12] << 24) | (test_frame[13] << 16)
                     | (test_frame[14] << 8) | (test_frame[15]);
  // ATTRIBUTE_BASIC_TARIFF_INFO_E_RATE_CONSUMPTION_REGISTER
  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           REPORTED_ATTRIBUTE,
                                                           (uint8_t *)&value_7,
                                                           sizeof(int32_t),
                                                           SL_STATUS_OK);
  attribute_store_set_node_attribute_value_ExpectAndReturn(node_id_node,
                                                           DESIRED_ATTRIBUTE,
                                                           0,
                                                           0,
                                                           SL_STATUS_OK);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handlers[COMMAND_CLASS_BASIC_TARIFF_INFO]
                      .control_handler(&c, test_frame, sizeof(test_frame)));
}
}
