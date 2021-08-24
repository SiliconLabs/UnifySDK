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
// Includes from this component
#include "zwave_command_class_manufacturer_specific.h"
#include "zwave_command_class_test_helper.h"

#define MANUFACTURER_ID 0x1234
#define PRODUCT_TYPE    0x5678
#define PRODUCT_ID      0x9ABC
#define DEVICE_ID       "DFCE1324576809"

// Includes from other components
#include "zwave_tx_mock.h"
#include "zwave_command_handler_mock.h"
#include "zwave_controller_utils_mock.h"
#include "zwave_controller_utils_mock.h"
#include "zwave_security_validation_mock.h"

#include "zwave_controller_connection_info.h"

#include "ZW_classcmd.h"
#include <string.h>

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void test_zwave_command_class_manufacturer_specific_get()
{
  const uint8_t cmd_frame_manufacturer_specific_command[]
    = {COMMAND_CLASS_MANUFACTURER_SPECIFIC, MANUFACTURER_SPECIFIC_GET};

  const uint8_t exp_frame_manufacturer_specific_command[]
    = {COMMAND_CLASS_MANUFACTURER_SPECIFIC,
       MANUFACTURER_SPECIFIC_REPORT,
       MANUFACTURER_ID >> 8,
       MANUFACTURER_ID & 0xff,
       PRODUCT_TYPE >> 8,
       PRODUCT_TYPE & 0xff,
       PRODUCT_ID >> 8,
       PRODUCT_ID & 0xff};

  zwave_security_validation_is_security_valid_for_support_ExpectAndReturn(
    ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME,
    NULL,
    true);
  zwave_security_validation_is_security_valid_for_support_IgnoreArg_connection();

  execute_frame_expect_frame(zwave_command_class_manufacturer_specific_support_handler,
                             cmd_frame_manufacturer_specific_command,
                             sizeof(cmd_frame_manufacturer_specific_command),
                             exp_frame_manufacturer_specific_command,
                             sizeof(exp_frame_manufacturer_specific_command),
                             SL_STATUS_OK);
}

void test_zwave_command_class_default_device_specific_get()
{
  const uint8_t cmd_frame_device_specific_cmd[]
    = {COMMAND_CLASS_MANUFACTURER_SPECIFIC, DEVICE_SPECIFIC_GET_V2};

  const uint8_t exp_frame_device_specific_cmd[]
    = {COMMAND_CLASS_MANUFACTURER_SPECIFIC,
       DEVICE_SPECIFIC_REPORT_V2,
       DEVICE_SPECIFIC_GET_DEVICE_ID_TYPE_SERIAL_NUMBER_V2,
       ((DEVICE_SPECIFIC_REPORT_DEVICE_ID_DATA_FORMAT_BINARY_V2
         << DEVICE_SPECIFIC_REPORT_PROPERTIES2_DEVICE_ID_DATA_FORMAT_SHIFT_V2)
        | (strlen(DEVICE_ID) / 2)),
       0xDF,
       0xCE,
       0x13,
       0x24,
       0x57,
       0x68,
       0x09};

  zwave_security_validation_is_security_valid_for_support_ExpectAndReturn(
    ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME,
    NULL,
    true);
  zwave_security_validation_is_security_valid_for_support_IgnoreArg_connection();

  execute_frame_expect_frame(zwave_command_class_manufacturer_specific_support_handler,
                             cmd_frame_device_specific_cmd,
                             sizeof(cmd_frame_device_specific_cmd),
                             exp_frame_device_specific_cmd,
                             sizeof(exp_frame_device_specific_cmd),
                             SL_STATUS_OK);
}

void test_zwave_command_class_manufacturer_specific_not_supported_result()
{
  execute_frame_expect_result(zwave_command_class_manufacturer_specific_support_handler,
                              NULL,
                              0,
                              SL_STATUS_NOT_SUPPORTED);
  execute_frame_expect_result(zwave_command_class_manufacturer_specific_support_handler,
                              NULL,
                              1,
                              SL_STATUS_NOT_SUPPORTED);
  uint8_t cmd_frame_without_command[] = {
    COMMAND_CLASS_MANUFACTURER_SPECIFIC,
  };
  execute_frame_expect_result(zwave_command_class_manufacturer_specific_support_handler,
                              cmd_frame_without_command,
                              sizeof(cmd_frame_without_command),
                              SL_STATUS_NOT_SUPPORTED);
}

void test_zwave_command_class_manufacturer_specific_ignore_non_relevant_cmd()
{
  uint8_t cmd_frame_unknown[] = {
    COMMAND_CLASS_MANUFACTURER_SPECIFIC,
    0xFF  // arbitrary Command
  };

  execute_frame_expect_result(zwave_command_class_manufacturer_specific_support_handler,
                              cmd_frame_unknown,
                              sizeof(cmd_frame_unknown),
                              SL_STATUS_NOT_SUPPORTED);
}

void test_zwave_command_class_manufacturer_specific_ignore_non_relevant_cc()
{
  uint8_t cmd_frame_unknown[] = {
    0xFF,
    0xFF  // arbitrary Command
  };

  execute_frame_expect_result(zwave_command_class_manufacturer_specific_support_handler,
                              cmd_frame_unknown,
                              sizeof(cmd_frame_unknown),
                              SL_STATUS_NOT_SUPPORTED);
}

static sl_status_t zwave_command_handler_register_handler_CALLBACK(
  zwave_command_handler_t actual, int cmock_num_calls)
{
  zwave_command_handler_t expected = {0};
  expected.minimal_scheme     = ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME;
  expected.command_class      = COMMAND_CLASS_MANUFACTURER_SPECIFIC_V2;
  expected.command_class_name = "Manufacturer Specific";
  expected.version            = MANUFACTURER_SPECIFIC_VERSION_V2;
  expected.support_handler = &zwave_command_class_manufacturer_specific_support_handler;
  expected.control_handler = NULL;

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

void test_zwave_command_class_manuf_spec_init_registers_handler()
{
  zwave_command_handler_register_handler_AddCallback(
    zwave_command_handler_register_handler_CALLBACK);
  zwave_command_handler_register_handler_IgnoreAndReturn(SL_STATUS_OK);
  zwave_command_class_manufacturer_specific_init();
}

void test_zwave_command_class_manufacturer_specific_multicast_not_allowed()
{
  uint8_t cmd_frame_device_get_command[]
    = {COMMAND_CLASS_MANUFACTURER_SPECIFIC_V2, DEVICE_SPECIFIC_GET_V2};
  zwave_security_validation_is_security_valid_for_support_ExpectAndReturn(
    ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME,
    NULL,
    true);
  zwave_security_validation_is_security_valid_for_support_IgnoreArg_connection();
  assert_dissallow_multicast(zwave_command_class_manufacturer_specific_support_handler,
                             cmd_frame_device_get_command);

  uint8_t cmd_frame_manu_spec_get_command[]
    = {COMMAND_CLASS_MANUFACTURER_SPECIFIC_V2, MANUFACTURER_SPECIFIC_GET};

  zwave_security_validation_is_security_valid_for_support_ExpectAndReturn(
    ZWAVE_CONTROLLER_ENCAPSULATION_NETWORK_SCHEME,
    NULL,
    true);
  zwave_security_validation_is_security_valid_for_support_IgnoreArg_connection();
  assert_dissallow_multicast(zwave_command_class_manufacturer_specific_support_handler,
                             cmd_frame_manu_spec_get_command);
}
