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

#include "zwave_command_class_humidity_control_mode.h"
#include "zwave_command_class_humidity_control_types.h"
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

static zwave_command_handler_t handler = {};

static attribute_resolver_function_t current_expected_humidity_control_mode_get
  = NULL;
static attribute_resolver_function_t current_expected_humidity_control_mode_set
  = NULL;
static attribute_resolver_function_t
  supported_expected_humidity_control_modes_get
  = NULL;

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
  if (node_type == ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_CURRENT_MODE) {
    TEST_ASSERT_NOT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    current_expected_humidity_control_mode_get = get_func;
    current_expected_humidity_control_mode_set = set_func;
  } else if (node_type
             == ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_SUPPORTED_MODES) {
    TEST_ASSERT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    supported_expected_humidity_control_modes_get = get_func;
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_HUMIDITY_CONTROL_MODE, handler.command_class);
  TEST_ASSERT_EQUAL(2, handler.version);
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
  current_expected_humidity_control_mode_get    = NULL;
  current_expected_humidity_control_mode_set    = NULL;
  supported_expected_humidity_control_modes_get = NULL;
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
                    zwave_command_class_humidity_control_mode_init());
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
    ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_VERSION,
    endpoint_id_node);

  attribute_store_set_reported(version_node, &version, sizeof(version));
}

void set_current_mode(humidity_control_mode_t humidity_control_mode)
{
  sl_status_t reported_status = attribute_store_set_child_reported(
    endpoint_id_node,
    ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_CURRENT_MODE,
    &humidity_control_mode,
    sizeof(humidity_control_mode));

  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    reported_status,
    "Should be able to set humidity control mode value");
}

void set_supported_modes(
  humidity_control_supported_modes_t supported_modes)
{
  sl_status_t status = attribute_store_set_child_reported(
    endpoint_id_node,
    ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_SUPPORTED_MODES,
    &supported_modes,
    sizeof(supported_modes));

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            status,
                            "Should be able to set supported mode node");
}

////////////////////////////////////////////////////////////////////////////
// Helper
////////////////////////////////////////////////////////////////////////////
void helper_humidity_control_mode_report(zwave_cc_version_t version,
                                         bool happy_case)
{
  printf("Testing report version %d. Happy case : %d\n", version, happy_case);

  set_version(version);

  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&info, NULL, 0));

  // Set supported mode based on the happy case flag
  // If not happy case, no supported mode
  humidity_control_supported_modes_t supported_modes;
  humidity_control_mode_t expected_humidity_control_mode;
  switch (version) {
    case 1:
      supported_modes = happy_case ? 0b00000110 : 0b00000000;
      expected_humidity_control_mode
        = HUMIDITY_CONTROL_MODE_SET_MODE_DEHUMIDIFY;
      break;
    case 2:
      supported_modes                = happy_case ? 0b00001110 : 0b00000000;
      expected_humidity_control_mode = HUMIDITY_CONTROL_MODE_SET_MODE_AUTO_V2;
      break;
    default:
      TEST_ABORT();
  }
  
  set_supported_modes(supported_modes);

  uint8_t frame[] = {COMMAND_CLASS_HUMIDITY_CONTROL_MODE,
                     HUMIDITY_CONTROL_MODE_REPORT,
                     expected_humidity_control_mode};

  sl_status_t expected_status
    = happy_case ? SL_STATUS_OK : SL_STATUS_NOT_SUPPORTED;
  TEST_ASSERT_EQUAL(expected_status,
                    handler.control_handler(&info, frame, sizeof(frame)));

  humidity_control_mode_t reported_humidity_control_mode = 0;
  sl_status_t reported_get_status = attribute_store_get_child_reported(
    endpoint_id_node,
    ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_CURRENT_MODE,
    &reported_humidity_control_mode,
    sizeof(reported_humidity_control_mode));

  expected_status = happy_case ? SL_STATUS_OK : SL_STATUS_FAIL;
  // Test if value is defined or not
  TEST_ASSERT_EQUAL(expected_status, reported_get_status);
  if (happy_case) {
    TEST_ASSERT_EQUAL_MESSAGE(
      expected_humidity_control_mode,
      reported_humidity_control_mode,
      "Humidity control mode should be equal to reported one");
  } else {
    // Test version mismatch even if supported says ok
    switch (version) {
      case 1:
        supported_modes = 0b00001111;
        // Not supported in v1
        expected_humidity_control_mode = HUMIDITY_CONTROL_MODE_SET_MODE_AUTO_V2;
        break;
      case 2:
        supported_modes = 0b00011111;
        // Not supported in v2
        expected_humidity_control_mode = 0x04;
        break;
      default:
        TEST_ABORT();
    }
    set_supported_modes(supported_modes);

    uint8_t frame[] = {COMMAND_CLASS_HUMIDITY_CONTROL_MODE,
                       HUMIDITY_CONTROL_MODE_REPORT,
                       expected_humidity_control_mode};

    TEST_ASSERT_EQUAL_MESSAGE(
      SL_STATUS_NOT_SUPPORTED,
      handler.control_handler(&info, frame, sizeof(frame)),
      "Report should return SL_STATUS_NOT_SUPPORTED for version mismatch");

    reported_get_status = attribute_store_get_child_reported(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_CURRENT_MODE,
      &reported_humidity_control_mode,
      sizeof(reported_humidity_control_mode));

    TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_FAIL,
                              reported_get_status,
                              "ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_"
                              "CURRENT_MODE should not have a value");
  }
}

void helper_humidity_control_mode_set(zwave_cc_version_t version,
                                      bool happy_case)
{
  TEST_ASSERT_NOT_NULL(current_expected_humidity_control_mode_set);
  set_version(version);

  attribute_store_node_t current_mode_node
    = attribute_store_get_first_child_by_type(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_CURRENT_MODE);

  // Set supported mode based on the happy case flag
  // If not happy case, no supported mode
  humidity_control_supported_modes_t supported_modes;
  humidity_control_mode_t expected_humidity_control_mode;
  switch (version) {
    case 1:
      supported_modes = happy_case ? 0b00000110 : 0b00000000;
      expected_humidity_control_mode
        = HUMIDITY_CONTROL_MODE_SET_MODE_DEHUMIDIFY;
      break;
    case 2:
      supported_modes                = happy_case ? 0b00001110 : 0b00000000;
      expected_humidity_control_mode = HUMIDITY_CONTROL_MODE_SET_MODE_AUTO_V2;
      break;
    default:
      TEST_ABORT();
  }
  set_supported_modes(supported_modes);
  set_current_mode(expected_humidity_control_mode);

  // Call set 
  sl_status_t reported_set_status
    = current_expected_humidity_control_mode_set(current_mode_node,
                                                 received_frame,
                                                 &received_frame_size);

  if (happy_case) {
    TEST_ASSERT_EQUAL(SL_STATUS_OK, reported_set_status);
    const uint8_t expected_frame[] = {COMMAND_CLASS_HUMIDITY_CONTROL_MODE,
                                      HUMIDITY_CONTROL_MODE_SET,
                                      expected_humidity_control_mode};
    TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                  received_frame,
                                  received_frame_size);
  } else {
    TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED, reported_set_status);

    // Test version mismatch even if supported says ok
    switch (version) {
      case 1:
        supported_modes = 0b00001111;
        // Not supported in v1
        expected_humidity_control_mode = HUMIDITY_CONTROL_MODE_SET_MODE_AUTO_V2;
        break;
      case 2:
        supported_modes = 0b00011111;
        // Not supported in v2
        expected_humidity_control_mode = 0x04;
        break;
      default:
        TEST_ABORT();
    }
    set_supported_modes(supported_modes);
    set_current_mode(expected_humidity_control_mode);
    
    // Call set 
    reported_set_status
      = current_expected_humidity_control_mode_set(current_mode_node,
                                                   received_frame,
                                                   &received_frame_size);
    TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED, reported_set_status);
  }
}
////////////////////////////////////////////////////////////////////////////
// Humidity Control Mode Supported 
////////////////////////////////////////////////////////////////////////////
void test_humidity_control_mode_supported_get_happy_case()
{
  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL(supported_expected_humidity_control_modes_get);
  supported_expected_humidity_control_modes_get(0,
                                                received_frame,
                                                &received_frame_size);
  const uint8_t expected_frame[] = {COMMAND_CLASS_HUMIDITY_CONTROL_MODE,
                                    HUMIDITY_CONTROL_MODE_SUPPORTED_GET};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_humidity_control_mode_supported_report_happy_case()
{
    zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  humidity_control_supported_modes_t expected_supported_modes = 0b00000110;

  uint8_t frame[] = {COMMAND_CLASS_HUMIDITY_CONTROL_MODE,
                     HUMIDITY_CONTROL_MODE_SUPPORTED_REPORT,
                     expected_supported_modes};

  // test with invalid frame
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&info, NULL, 0));

  // test with valid frame
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info, frame, sizeof(frame)));

  humidity_control_supported_modes_t reported_supported_modes;
  sl_status_t reported_status = attribute_store_get_child_reported(
    endpoint_id_node,
    ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_SUPPORTED_MODES,
    &reported_supported_modes,
    sizeof(reported_supported_modes));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,reported_status);
  TEST_ASSERT_EQUAL(expected_supported_modes,reported_supported_modes);
}



////////////////////////////////////////////////////////////////////////////
// Humidity Control Mode Get/Set/Report
////////////////////////////////////////////////////////////////////////////
void test_humidity_control_mode_get_happy_case()
{
  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL(current_expected_humidity_control_mode_get);
  current_expected_humidity_control_mode_get(0,
                                             received_frame,
                                             &received_frame_size);
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_HUMIDITY_CONTROL_MODE, HUMIDITY_CONTROL_MODE_GET};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_humidity_control_mode_report_v1_happy_case()
{
  helper_humidity_control_mode_report(HUMIDITY_CONTROL_MODE_VERSION, true);
}

void test_humidity_control_mode_report_v2_happy_case()
{
  helper_humidity_control_mode_report(HUMIDITY_CONTROL_MODE_VERSION_V2, true);
}

void test_humidity_control_mode_report_v1_version_and_supported_mismatch()
{
  helper_humidity_control_mode_report(HUMIDITY_CONTROL_MODE_VERSION, false);
}

void test_humidity_control_mode_report_v2_version_and_supported_mismatch()
{
  helper_humidity_control_mode_report(HUMIDITY_CONTROL_MODE_VERSION_V2, false);
}

void test_humidity_control_mode_set_v1_happy_case()
{
  helper_humidity_control_mode_set(HUMIDITY_CONTROL_MODE_VERSION, true);
}

void test_humidity_control_mode_set_v2_happy_case()
{
  helper_humidity_control_mode_set(HUMIDITY_CONTROL_MODE_VERSION_V2, true);
}

void test_humidity_control_mode_set_v1_version_and_supported_mismatch()
{
  helper_humidity_control_mode_set(HUMIDITY_CONTROL_MODE_VERSION, false);
}

void test_humidity_control_mode_set_v2_version_and_supported_mismatch()
{
  helper_humidity_control_mode_set(HUMIDITY_CONTROL_MODE_VERSION_V2, false);
}

////////////////////////////////////////////////////////////////////////////
// Attribute creation
////////////////////////////////////////////////////////////////////////////
void test_attribute_creation_happy_case()
{
  attribute_store_type_t testing_types[]
    = {ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_SUPPORTED_MODES,
       ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_MODE_CURRENT_MODE};
  const int testing_types_count = 2;

  for (int i = 0; i < testing_types_count; i++) {
    attribute_store_node_t current_testing_node
      = attribute_store_get_first_child_by_type(endpoint_id_node,
                                                testing_types[i]);
    TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, current_testing_node);
  }

  // Trigger version attribute that creates those attributes
  set_version(1);

  for (int i = 0; i < testing_types_count; i++) {
    attribute_store_node_t current_testing_node
      = attribute_store_get_first_child_by_type(endpoint_id_node,
                                                testing_types[i]);
    TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, current_testing_node);
  }
}