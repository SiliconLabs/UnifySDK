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
#include "zwave_command_class_zwave_plus_info.h"
#include "zwave_raw_frame_test_helper.h"

// Includes from other components
#include "zwave_tx_mock.h"
#include "zwave_command_handler_mock.h"
#include "zwave_controller_connection_info.h"
#include "ZW_classcmd.h"

// Test includes
#include "unity.h"

#define ZWAVEPLUS_VERSION_V2 0x02
//Test powerelvel get on 700 series chip
void test_zwave_command_class_zwave_plus_info_get()
{
  const uint8_t cmd_frame_zwave_plus_info_command[]
    = {COMMAND_CLASS_ZWAVEPLUS_INFO, ZWAVEPLUS_INFO_GET_V2};

  const uint8_t exp_frame_zwave_plus_info_command[]
    = {COMMAND_CLASS_ZWAVEPLUS_INFO,
       ZWAVEPLUS_INFO_REPORT,
       ZWAVEPLUS_VERSION_V2,
       ZWAVEPLUS_INFO_REPORT_ROLE_TYPE_CONTROLLER_CENTRAL_STATIC,
       ZWAVEPLUS_INFO_REPORT_NODE_TYPE_ZWAVEPLUS_NODE,
       (ICON_TYPE_GENERIC_CENTRAL_CONTROLLER >> 8) & 0xFF,
       (ICON_TYPE_GENERIC_CENTRAL_CONTROLLER & 0xFF),
       (ICON_TYPE_GENERIC_CENTRAL_CONTROLLER >> 8) & 0xFF,
       (ICON_TYPE_GENERIC_CENTRAL_CONTROLLER & 0xFF)};

  execute_frame_expect_frame(
    zwave_command_class_zwave_plus_info_support_handler,
    cmd_frame_zwave_plus_info_command,
    sizeof(cmd_frame_zwave_plus_info_command),
    exp_frame_zwave_plus_info_command,
    sizeof(exp_frame_zwave_plus_info_command),
    SL_STATUS_OK);
}

void test_zwave_command_class_zwave_plus_info_multicast_not_allowed()
{
  uint8_t cmd_frame_zwave_plus_info_command[]
    = {COMMAND_CLASS_ZWAVEPLUS_INFO, ZWAVEPLUS_INFO_GET_V2};

  assert_dissallow_multicast(
    zwave_command_class_zwave_plus_info_support_handler,
    cmd_frame_zwave_plus_info_command);
}

void test_zwave_command_class_zwave_plus_info_ignore_non_relevant_cmd()
{
  uint8_t cmd_frame_unknown[] = {
    COMMAND_CLASS_ZWAVEPLUS_INFO,
    0xFF  // arbitrary Command
  };

  execute_frame_expect_result(
    zwave_command_class_zwave_plus_info_support_handler,
    cmd_frame_unknown,
    sizeof(cmd_frame_unknown),
    SL_STATUS_NOT_SUPPORTED);
}

void test_zwave_command_class_zwave_plus_info_ignore_short_cmd()
{
  uint8_t cmd_frame_unknown[] = {
    COMMAND_CLASS_ZWAVEPLUS_INFO,
  };

  execute_frame_expect_result(
    zwave_command_class_zwave_plus_info_support_handler,
    cmd_frame_unknown,
    sizeof(cmd_frame_unknown),
    SL_STATUS_NOT_SUPPORTED);
}

static sl_status_t zwave_command_handler_register_handler_CALLBACK(
  zwave_command_handler_t actual, int cmock_num_calls)
{
  zwave_command_handler_t expected = {0};
  expected.minimal_scheme          = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  expected.command_class           = COMMAND_CLASS_ZWAVEPLUS_INFO;
  expected.command_class_name      = "Z-Wave Plus Info";
  expected.version                 = ZWAVEPLUS_INFO_VERSION_V2;
  expected.support_handler
    = &zwave_command_class_zwave_plus_info_support_handler;
  expected.control_handler = NULL;

  TEST_ASSERT_EQUAL_PTR(expected.support_handler, actual.support_handler);
  TEST_ASSERT_EQUAL_PTR(expected.control_handler, actual.control_handler);
  TEST_ASSERT_EQUAL(expected.minimal_scheme, actual.minimal_scheme);
  TEST_ASSERT_EQUAL(expected.command_class, actual.command_class);
  TEST_ASSERT_EQUAL_STRING(expected.command_class_name,
                           actual.command_class_name);
  TEST_ASSERT_EQUAL(expected.version, actual.version);
  TEST_ASSERT_FALSE(actual.manual_security_validation);
  return SL_STATUS_OK;
}

//Test handler resistering
void test_zwave_command_class_zwave_plus_info_init_registers_handler()
{
  zwave_command_handler_register_handler_AddCallback(
    zwave_command_handler_register_handler_CALLBACK);
  zwave_command_handler_register_handler_IgnoreAndReturn(SL_STATUS_OK);
  zwave_command_class_zwave_plus_info_init();
}
