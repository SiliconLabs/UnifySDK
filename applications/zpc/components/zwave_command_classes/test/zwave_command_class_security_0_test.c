/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "zwave_command_class_security_0.h"
#include "zwave_command_handler_mock.h"
#include "zwave_command_class_test_helper.h"
#include "ZW_classcmd.h"
#include "zwave_unid_mock.h"
#include "zpc_attribute_store_network_helper_mock.h"
#include "attribute_store_defined_attribute_types.h"

#define LOG_TAG "zwave_command_class_security_0"

static sl_status_t zwave_command_handler_register_handler_CALLBACK(
  zwave_command_handler_t actual, int cmock_num_calls)
{
  zwave_command_handler_t expected = {0};
  expected.minimal_scheme          = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  expected.command_class           = COMMAND_CLASS_SECURITY;
  expected.command_class_name      = "Security";
  expected.version                 = SECURITY_VERSION;
  expected.support_handler = &zwave_command_class_security_0_support_handler;
  expected.control_handler = &zwave_command_class_security_0_control_handler;

  TEST_ASSERT_EQUAL_PTR(expected.support_handler, actual.support_handler);
  TEST_ASSERT_EQUAL_PTR(expected.control_handler, actual.control_handler);
  TEST_ASSERT_EQUAL(expected.minimal_scheme, actual.minimal_scheme);
  TEST_ASSERT_EQUAL(expected.command_class, actual.command_class);
  TEST_ASSERT_EQUAL_STRING(expected.command_class_name,
                           actual.command_class_name);
  TEST_ASSERT_EQUAL(expected.version, actual.version);
  TEST_ASSERT_TRUE(actual.manual_security_validation);
  return SL_STATUS_OK;
}

void test_zwave_command_class_s0_init_registers_handler()
{
  zwave_command_handler_register_handler_AddCallback(
    zwave_command_handler_register_handler_CALLBACK);
  zwave_command_handler_register_handler_IgnoreAndReturn(SL_STATUS_OK);

  zwave_command_class_security_0_init();
}

void test_zwave_command_class_s0_ignore_non_relevant_cc()
{
  uint8_t cmd_frame_unknown[] = {
    0xFF,
    0xFF  // arbitrary Command
  };

  execute_frame_expect_result(zwave_command_class_security_0_support_handler,
                              cmd_frame_unknown,
                              sizeof(cmd_frame_unknown),
                              SL_STATUS_NOT_SUPPORTED);
}

void test_zwave_command_class_s0_control_ignore_non_relevant_cc()
{
  uint8_t cmd_frame_unknown[] = {
    0xFF,
    0xFF  // arbitrary Command
  };

  execute_frame_expect_result(zwave_command_class_security_0_control_handler,
                              cmd_frame_unknown,
                              sizeof(cmd_frame_unknown),
                              SL_STATUS_NOT_SUPPORTED);
}

void test_zwave_command_class_security_0_ignore_non_relevant_cmd()
{
  uint8_t cmd_frame_unknown[] = {
    COMMAND_CLASS_MANUFACTURER_SPECIFIC,
    0xFF  // arbitrary Command
  };

  execute_frame_expect_result(zwave_command_class_security_0_support_handler,
                              cmd_frame_unknown,
                              sizeof(cmd_frame_unknown),
                              SL_STATUS_NOT_SUPPORTED);
}

static zwave_controller_connection_info_t test_connection_info = {};

void test_zwave_command_class_security_0_report_short_frame()
{
  const uint8_t cmd_frame_security_0_command[]
    = {COMMAND_CLASS_SECURITY, SECURITY_COMMANDS_SUPPORTED_REPORT, 0};

  test_connection_info.encapsulation
    = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0;
  sl_status_t result = zwave_command_class_security_0_control_handler(
    &test_connection_info,
    cmd_frame_security_0_command,
    sizeof(cmd_frame_security_0_command));

  TEST_ASSERT_EQUAL(result, SL_STATUS_OK);
}

void test_zwave_command_class_security_0_report_wrong_encap()
{
  const uint8_t cmd_frame_security_0_command[]
    = {COMMAND_CLASS_SECURITY,
       SECURITY_COMMANDS_SUPPORTED_REPORT,
       0,
       COMMAND_CLASS_TIME};

  test_connection_info.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  sl_status_t result = zwave_command_class_security_0_control_handler(
    &test_connection_info,
    cmd_frame_security_0_command,
    sizeof(cmd_frame_security_0_command));

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED, result);
}

void test_zwave_command_class_security_0_report()
{
  zwave_node_id_t nodeid = 42;
  const uint8_t nif[2]   = {COMMAND_CLASS_TIME, COMMAND_CLASS_ALARM};
  const uint8_t cmd_frame_security_0_command[]
    = {COMMAND_CLASS_SECURITY,
       SECURITY_COMMANDS_SUPPORTED_REPORT,
       0,
       nif[0],
       nif[1]};

  test_connection_info.encapsulation
    = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0;
  test_connection_info.remote.node_id = nodeid;
  zwave_unid_from_node_id_Expect(nodeid, 0);
  zwave_unid_from_node_id_IgnoreArg_unid();
  attribute_store_network_helper_get_endpoint_node_ExpectAndReturn(0, 0, 0);
  attribute_store_network_helper_get_endpoint_node_IgnoreArg_endpoint_id();
  attribute_store_network_helper_get_endpoint_node_IgnoreArg_node_unid();

  attribute_store_get_node_child_by_type_ExpectAndReturn(
    0,
    ATTRIBUTE_ZWAVE_SECURE_NIF,
    0,
    0);
  attribute_store_get_node_child_by_type_IgnoreArg_node();
  attribute_store_get_node_child_by_type_IgnoreArg_child_index();

  attribute_store_set_node_attribute_value_ExpectAndReturn(0,
                                                           REPORTED_ATTRIBUTE,
                                                           nif,
                                                           sizeof(nif),
                                                           0);
  attribute_store_set_node_attribute_value_IgnoreArg_node();

  sl_status_t result = zwave_command_class_security_0_control_handler(
    &test_connection_info,
    cmd_frame_security_0_command,
    sizeof(cmd_frame_security_0_command));

  TEST_ASSERT_EQUAL(result, SL_STATUS_OK);
}
