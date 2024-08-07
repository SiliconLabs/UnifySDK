/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include "zwave_command_class_thermostat_fan_mode.h"
#include "zwave_command_class_thermostat_fan_types.h"
#include "zwave_command_classes_utils.h"
#include "unity.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "zpc_attribute_store_type_registration.h"

// Interface includes
#include "attribute_store_defined_attribute_types.h"
#include "ZW_classcmd.h"
#include "zwave_utils.h"
#include "zwave_controller_types.h"

// Test helpers
#include "zpc_attribute_store_test_helper.h"

// Mock includes
#include "attribute_resolver_mock.h"
#include "zpc_attribute_resolver_mock.h"
#include "zwave_command_handler_mock.h"
#include "dotdot_mqtt_mock.h"
#include "dotdot_mqtt_generated_commands_mock.h"

// Missing for now
#define THERMOSTAT_FAN_MODE_VERSION_V5 0x05
// Value that is unsupported for all Thermostat Fan Mode versions
#define THERMOSTAT_FAN_MODE_UNSUPPORTED 0x0F


static zwave_command_handler_t handler = {};

static attribute_resolver_function_t current_fan_mode_get = NULL;
static attribute_resolver_function_t current_fan_mode_set = NULL;
static attribute_resolver_function_t supported_fan_get    = NULL;

// Buffer for frame
static uint8_t received_frame[255]  = {};
static uint16_t received_frame_size = 0;

// Stub functions
static sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  if (node_type == ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_CURRENT_MODE
      || node_type == ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_OFF_FLAG) {
    TEST_ASSERT_NOT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    current_fan_mode_get = get_func;
    current_fan_mode_set = set_func;
  } else if (node_type
             == ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_SUPPORTED_MODES) {
    TEST_ASSERT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    supported_fan_get = get_func;
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_THERMOSTAT_FAN_MODE, handler.command_class);
  TEST_ASSERT_EQUAL(5, handler.version);
  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_NULL(handler.support_handler);
  TEST_ASSERT_FALSE(handler.manual_security_validation);

  return SL_STATUS_OK;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  zpc_attribute_store_register_known_attribute_types();
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

  // Unset previous definition get/set functions
  current_fan_mode_get = NULL;
  current_fan_mode_set = NULL;
  supported_fan_get    = NULL;
  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size = 0;
  // Unset previous definition of handler
  memset(&handler, 0, sizeof(zwave_command_handler_t));

  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);
  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);
  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_thermostat_fan_mode_init());
}

/// Called after each and every test
void tearDown() {}

////////////////////////////////////////////////////////////////////////////
// UTILS
////////////////////////////////////////////////////////////////////////////

// Set version and thus initialize the attribute tree
void set_version(zwave_cc_version_t version)
{
  attribute_store_node_t version_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_VERSION,
    endpoint_id_node);

  attribute_store_set_reported(version_node, &version, sizeof(version));
}

void set_supported_modes(thermostat_fan_supported_modes_t supported_modes)
{
  sl_status_t status = attribute_store_set_child_reported(
    endpoint_id_node,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_SUPPORTED_MODES,
    &supported_modes,
    sizeof(supported_modes));

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            status,
                            "Should be able to set supported mode node");
}

// Help test happy case
void helper_fan_mode_set(zwave_cc_version_t version, bool happy_case)
{
  printf("helper_fan_mode_set: version %d, happy_case : %d\n",
         version,
         happy_case);

  set_version(version);

  thermostat_fan_mode_off_flag_t off_flag;
  uint8_t supported_bit1          = 0b00000000;
  uint8_t supported_bit2          = 0b00000000;
  size_t supported_fan_mode_count = 0;
  thermostat_fan_mode_t supported_fan_mode_type[16];

  switch (version) {
    case 1:
      supported_bit1 = 0b00001010;
      supported_bit2 = 0b00000000;

      supported_fan_mode_count   = 2;
      supported_fan_mode_type[0] = THERMOSTAT_FAN_MODE_SET_FAN_MODE_LOW;
      supported_fan_mode_type[1] = THERMOSTAT_FAN_MODE_SET_FAN_MODE_HIGH;
      break;
    case 2:
      off_flag = 1;

      supported_bit1 = 0b00010101;
      supported_bit2 = 0b00000000;

      supported_fan_mode_count   = 3;
      supported_fan_mode_type[0] = THERMOSTAT_FAN_MODE_SET_FAN_MODE_AUTO_LOW;
      supported_fan_mode_type[1] = THERMOSTAT_FAN_MODE_SET_FAN_MODE_AUTO_HIGH;
      supported_fan_mode_type[2]
        = THERMOSTAT_FAN_MODE_SET_FAN_MODE_AUTO_MEDIUM_V2;
      break;
    case 3:
      off_flag = 5;  // Should be taken down to 0

      supported_bit1 = 0b11100000;
      supported_bit2 = 0b00000000;

      supported_fan_mode_count   = 3;
      supported_fan_mode_type[0] = THERMOSTAT_FAN_MODE_SET_FAN_MODE_MEDIUM_V3;
      supported_fan_mode_type[1]
        = THERMOSTAT_FAN_MODE_SET_FAN_MODE_CIRCULATION_V3;
      supported_fan_mode_type[2] = THERMOSTAT_FAN_MODE_SET_FAN_MODE_HUMIDITY_V3;
      break;
    case 4:
      off_flag = 0;

      supported_bit1 = 0b01000001;
      supported_bit2 = 0b00000011;

      supported_fan_mode_count   = 4;
      supported_fan_mode_type[0] = THERMOSTAT_FAN_MODE_SET_FAN_MODE_AUTO_LOW;
      supported_fan_mode_type[1]
        = THERMOSTAT_FAN_MODE_SET_FAN_MODE_CIRCULATION_V3;
      supported_fan_mode_type[2]
        = THERMOSTAT_FAN_MODE_SET_FAN_MODE_LEFT_RIGHT_V4;
      supported_fan_mode_type[3] = THERMOSTAT_FAN_MODE_SET_FAN_MODE_UP_DOWN_V4;
      break;
    case 5:
      off_flag = 1;

      supported_bit1 = 0b00010101;
      supported_bit2 = 0b00001100;

      supported_fan_mode_count   = 5;
      supported_fan_mode_type[0] = THERMOSTAT_FAN_MODE_SET_FAN_MODE_AUTO_LOW;
      supported_fan_mode_type[1] = THERMOSTAT_FAN_MODE_SET_FAN_MODE_AUTO_HIGH;
      supported_fan_mode_type[2]
        = THERMOSTAT_FAN_MODE_SET_FAN_MODE_AUTO_MEDIUM_V2;
      supported_fan_mode_type[3] = THERMOSTAT_FAN_MODE_SET_FAN_MODE_QUIET_V4;
      supported_fan_mode_type[4] = THERMOSTAT_FAN_MODE_REPORT_FAN_MODE_EXTERNAL_CIRCULATION_V5;
      break;
      TEST_FAIL_MESSAGE("Version not supported in helper_fan_mode_set()");
  }

  // Invert bit to make sure that the supported values are not defined
  if (!happy_case) {
    supported_bit1 = ~supported_bit1;
    supported_bit2 = ~supported_bit2;
  }

  thermostat_fan_supported_modes_t expected_bitmask
    = (supported_bit2 << 8) | supported_bit1;
  set_supported_modes(expected_bitmask);

  for (int i = 0; i < supported_fan_mode_count; i++) {
    // Setup attribute store
    thermostat_fan_mode_t fan_mode = supported_fan_mode_type[i];

    attribute_store_node_t fan_mode_node
      = attribute_store_get_node_child_by_type(
        endpoint_id_node,
        ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_CURRENT_MODE,
        0);
    sl_status_t result = attribute_store_set_desired(fan_mode_node,
                                                      &fan_mode,
                                                      sizeof(fan_mode));
    TEST_ASSERT_EQUAL(SL_STATUS_OK, result);

    // Not existant in v1
    attribute_store_node_t off_flag_node
      = attribute_store_get_node_child_by_type(
        endpoint_id_node,
        ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_OFF_FLAG,
        0);

    if (version == 1) {
      TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, off_flag_node);
    } else {
      result = attribute_store_set_desired(off_flag_node,
                                            &off_flag,
                                            sizeof(off_flag));
      TEST_ASSERT_EQUAL(SL_STATUS_OK, result);
      TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, off_flag_node);
    }

    if (happy_case) {
      // Call set
      TEST_ASSERT_NOT_NULL(current_fan_mode_set);
      sl_status_t set_reported_status
        = current_fan_mode_set(fan_mode_node,
                               received_frame,
                               &received_frame_size);
      TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                                set_reported_status,
                                "Set function should have worked");

      // Compute value
      uint8_t expected_value = fan_mode;
      if (version > 1) {
        off_flag       = off_flag >= 1 ? 1 : 0;  // Adjust off flag if needed
        expected_value = fan_mode | (off_flag << 7);
      }

      // Check frame
      const uint8_t expected_frame[] = {COMMAND_CLASS_THERMOSTAT_FAN_MODE,
                                        THERMOSTAT_FAN_MODE_SET,
                                        expected_value};
      TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
      TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                    received_frame,
                                    received_frame_size);

    } else {
      // Call set and should bail
      TEST_ASSERT_NOT_NULL(current_fan_mode_set);
      sl_status_t set_reported_status
        = current_fan_mode_set(fan_mode_node,
                               received_frame,
                               &received_frame_size);
      TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_NOT_SUPPORTED,
                                set_reported_status,
                                "Set function should NOT have worked");


      // Now we want to test version mismatch 
      // Set all supported modes
      set_supported_modes(0xFFFF);
      // Set fan mode to one that isn't supported by any of the version
      fan_mode = THERMOSTAT_FAN_MODE_UNSUPPORTED;  
      result = attribute_store_set_desired(fan_mode_node,
                                            &fan_mode,
                                            sizeof(fan_mode));
      TEST_ASSERT_EQUAL(SL_STATUS_OK, result);

      set_reported_status
        = current_fan_mode_set(fan_mode_node,
                               received_frame,
                               &received_frame_size);
      TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_NOT_SUPPORTED,
                                set_reported_status,
                                "Set function should NOT have worked");
      // Reset supported modes
      set_supported_modes(expected_bitmask);
    }
  }
}

void helper_fan_mode_report(zwave_cc_version_t version,
                                       bool happy_case)
{
  set_version(version);

  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&info, NULL, 0));

  thermostat_fan_mode_t fan_mode;
  thermostat_fan_mode_off_flag_t off_flag;
  thermostat_fan_supported_modes_t current_supported_bitmask;

  switch (version) {
    case 1:
      fan_mode = THERMOSTAT_FAN_MODE_SET_FAN_MODE_HIGH;
      current_supported_bitmask = 0x08;
      // Set to 0 to ignore value (not supported)
      off_flag = 0;
      break;
    case 2:
      fan_mode = THERMOSTAT_FAN_MODE_SET_FAN_MODE_AUTO_MEDIUM_V2;
      current_supported_bitmask = 0x10;
      // Test if the value is correctly ignored since it is not supported in V2
      off_flag = 1;
      break;
    case 3:
      fan_mode = THERMOSTAT_FAN_MODE_SET_FAN_MODE_CIRCULATION_V3;
      current_supported_bitmask = 0x40;
      off_flag = 1;
      break;
    case 4:
      fan_mode = THERMOSTAT_FAN_MODE_SET_FAN_MODE_LEFT_RIGHT_V4;
      current_supported_bitmask = 0x100;
      off_flag = 0;
      break;
    case 5:
      fan_mode = THERMOSTAT_FAN_MODE_REPORT_FAN_MODE_EXTERNAL_CIRCULATION_V5;
      current_supported_bitmask = 0x800;
      off_flag = 1;
      break;
    default:
      TEST_FAIL_MESSAGE("Version not supported in helper_fan_mode_set");
  }

  uint8_t frame[] = {COMMAND_CLASS_THERMOSTAT_FAN_MODE,
                      THERMOSTAT_FAN_MODE_REPORT,
                      fan_mode | (off_flag << 7)};

  attribute_store_node_t current_fan_mode_node
    = attribute_store_get_node_child_by_type(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_CURRENT_MODE,
      0);

  if (happy_case) {
    set_supported_modes(current_supported_bitmask);

    TEST_ASSERT_EQUAL(SL_STATUS_OK,
                      handler.control_handler(&info, frame, sizeof(frame)));

    TEST_ASSERT_EQUAL(
      fan_mode,
      attribute_store_get_reported_number(current_fan_mode_node));

    attribute_store_node_t off_flag_node
      = attribute_store_get_node_child_by_type(
        endpoint_id_node,
        ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_OFF_FLAG,
        0);
    if (version == 1) {
      attribute_store_node_t off_flag_node
        = attribute_store_get_node_child_by_type(
          endpoint_id_node,
          ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_OFF_FLAG,
          0);

      TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, off_flag_node);
    } else if (version == 2) {
      // The attribute is defined bcp it exists in the SET command.
      // But the value should be ignored since v2 doesn't support Off bit in report
      // The test sets the value to 1 in the report and it should not be updated in the attribute store.
      TEST_ASSERT_EQUAL(0, attribute_store_get_reported_number(off_flag_node));
    } else {
      TEST_ASSERT_EQUAL(off_flag,
                        attribute_store_get_reported_number(off_flag_node));
    }
  } else {
    // Make sure that the reported value is not in the supported bitmask
    set_supported_modes(~current_supported_bitmask);
    TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                      handler.control_handler(&info, frame, sizeof(frame)));

    thermostat_fan_mode_t reported_fan_mode;
    TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                      attribute_store_get_reported(current_fan_mode_node,
                                                   &reported_fan_mode,
                                                   sizeof(reported_fan_mode)));
    
    // Now we support all but we test version mismatch
    set_supported_modes(0xFFFF);                                               
    uint8_t frame[] = {COMMAND_CLASS_THERMOSTAT_FAN_MODE,
                      THERMOSTAT_FAN_MODE_REPORT,
                      THERMOSTAT_FAN_MODE_UNSUPPORTED | (off_flag << 7)};

     TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                      handler.control_handler(&info, frame, sizeof(frame)));
     TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                       attribute_store_get_reported(current_fan_mode_node,
                                                    &reported_fan_mode,
                                                    sizeof(reported_fan_mode)));
  }
}

// happy_case : if true only accepted bit are sent
//              if false fill all unused bit with 1 to see if they are ignored correctly
void helper_fan_mode_supported_mode_report(zwave_cc_version_t version)
{
  set_version(version);

  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&info, NULL, 0));

  uint8_t supported_bit1 = 0b00000000;
  uint8_t supported_bit2 = 0b00000000;

  switch (version) {
    case 1:
      supported_bit1 = 0b00001010;
      supported_bit2 = 0b00000000;
      break;
    case 2:
      supported_bit1 = 0b00010101;
      supported_bit2 = 0b00000000;
      break;
    case 3:
      supported_bit1 = 0b11100000;
      supported_bit2 = 0b00000000;
      break;
    case 4:
      supported_bit1 = 0b01000001;
      supported_bit2 = 0b00000011;
      break;
    case 5:
      supported_bit1 = 0b00010101;
      supported_bit2 = 0b00000100;
      break;
  }
  thermostat_fan_supported_modes_t expected_bitmask
    = (supported_bit2 << 8) | supported_bit1;
  set_supported_modes(expected_bitmask);

  const uint8_t frame[] = {COMMAND_CLASS_THERMOSTAT_FAN_MODE,
                           THERMOSTAT_FAN_MODE_SUPPORTED_REPORT,
                           supported_bit1,
                           supported_bit2};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info, frame, sizeof(frame)));

  thermostat_fan_supported_modes_t reported_bitmask;

  sl_status_t reported_status = attribute_store_get_child_reported(
    endpoint_id_node,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_SUPPORTED_MODES,
    &reported_bitmask,
    sizeof(reported_bitmask));

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    reported_status,
    "Should be able to get reported value of "
    "ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_FAN_MODE_SUPPORTED_MODES");
  TEST_ASSERT_EQUAL_MESSAGE(expected_bitmask,
                            reported_bitmask,
                            "Incorrect supported fan mode bitmask");
}
////////////////////////////////////////////////////////////////////////////
// THERMOSTAT_FAN_MODE_SUPPORTED
////////////////////////////////////////////////////////////////////////////
void test_thermostat_fan_mode_supported_get_happy_case()
{
  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL(supported_fan_get);
  supported_fan_get(0, received_frame, &received_frame_size);
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_THERMOSTAT_FAN_MODE, THERMOSTAT_FAN_MODE_SUPPORTED_GET};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_thermostat_fan_mode_supported_report_v1_happy_case()
{
  helper_fan_mode_supported_mode_report(THERMOSTAT_FAN_MODE_VERSION);
}
void test_thermostat_fan_mode_supported_report_v2_happy_case()
{
  helper_fan_mode_supported_mode_report(THERMOSTAT_FAN_MODE_VERSION_V2);
}
void test_thermostat_fan_mode_supported_report_v3_happy_case()
{
  helper_fan_mode_supported_mode_report(THERMOSTAT_FAN_MODE_VERSION_V3);
}
void test_thermostat_fan_mode_supported_report_v4_happy_case()
{
  helper_fan_mode_supported_mode_report(THERMOSTAT_FAN_MODE_VERSION_V4);
}
void test_thermostat_fan_mode_supported_report_v5_happy_case()
{
  helper_fan_mode_supported_mode_report(THERMOSTAT_FAN_MODE_VERSION_V5);
}

////////////////////////////////////////////////////////////////////////////
// THERMOSTAT_FAN_MODE
////////////////////////////////////////////////////////////////////////////
void test_thermostat_fan_mode_get_happy_case()
{
  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL(current_fan_mode_get);
  current_fan_mode_get(0, received_frame, &received_frame_size);
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_THERMOSTAT_FAN_MODE, THERMOSTAT_FAN_MODE_GET};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_thermostat_fan_mode_set_v1_happy_case()
{
  helper_fan_mode_set(THERMOSTAT_FAN_MODE_VERSION, true);
}
void test_thermostat_fan_mode_set_v2_happy_case()
{
  helper_fan_mode_set(THERMOSTAT_FAN_MODE_VERSION_V2, true);
}
void test_thermostat_fan_mode_set_v3_happy_case()
{
  helper_fan_mode_set(THERMOSTAT_FAN_MODE_VERSION_V3, true);
}
void test_thermostat_fan_mode_set_v4_happy_case()
{
  helper_fan_mode_set(THERMOSTAT_FAN_MODE_VERSION_V4, true);
}
void test_thermostat_fan_mode_set_v5_happy_case()
{
  helper_fan_mode_set(THERMOSTAT_FAN_MODE_VERSION_V5, true);
}
void test_thermostat_fan_mode_set_v1_not_supported_types()
{
  helper_fan_mode_set(THERMOSTAT_FAN_MODE_VERSION, false);
}
void test_thermostat_fan_mode_set_v2_not_supported_types()
{
  helper_fan_mode_set(THERMOSTAT_FAN_MODE_VERSION_V2, false);
}
void test_thermostat_fan_mode_set_v3_not_supported_types()
{
  helper_fan_mode_set(THERMOSTAT_FAN_MODE_VERSION_V3, false);
}
void test_thermostat_fan_mode_set_v4_not_supported_types()
{
  helper_fan_mode_set(THERMOSTAT_FAN_MODE_VERSION_V4, false);
}
void test_thermostat_fan_mode_set_v5_not_supported_types()
{
  helper_fan_mode_set(THERMOSTAT_FAN_MODE_VERSION_V5, false);
}


void test_thermostat_fan_mode_report_v1_happy_case()
{
  helper_fan_mode_report(THERMOSTAT_FAN_MODE_VERSION, true);
}
void test_thermostat_fan_mode_report_v2_happy_case()
{
  helper_fan_mode_report(THERMOSTAT_FAN_MODE_VERSION_V2, true);
}
void test_thermostat_fan_mode_report_v3_happy_case()
{
  helper_fan_mode_report(THERMOSTAT_FAN_MODE_VERSION_V3, true);
}
void test_thermostat_fan_mode_report_v4_happy_case()
{
  helper_fan_mode_report(THERMOSTAT_FAN_MODE_VERSION_V4, true);
}
void test_thermostat_fan_mode_report_v5_happy_case()
{
  helper_fan_mode_report(THERMOSTAT_FAN_MODE_VERSION_V5, true);
}

void test_thermostat_fan_mode_report_v1_not_supported_types()
{
  helper_fan_mode_report(THERMOSTAT_FAN_MODE_VERSION, false);
}
void test_thermostat_fan_mode_report_v2_not_supported_types()
{
  helper_fan_mode_report(THERMOSTAT_FAN_MODE_VERSION_V2, false);
}
void test_thermostat_fan_mode_report_v3_not_supported_types()
{
  helper_fan_mode_report(THERMOSTAT_FAN_MODE_VERSION_V3, false);
}
void test_thermostat_fan_mode_report_v4_not_supported_types()
{
  helper_fan_mode_report(THERMOSTAT_FAN_MODE_VERSION_V4, false);
}
void test_thermostat_fan_mode_report_v5_not_supported_types()
{
  helper_fan_mode_report(THERMOSTAT_FAN_MODE_VERSION_V5, false);
}