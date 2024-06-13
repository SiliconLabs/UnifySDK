/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include "zwave_command_class_thermostat_operating_state.h"
#include "zwave_command_class_thermostat_operating_state_types.h"
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

static attribute_resolver_function_t operating_state_get   = NULL;
static attribute_resolver_function_t logging_supported_get = NULL;
static attribute_resolver_function_t logging_get           = NULL;

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
  if (node_type
      == ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_CURRENT_STATE) {
    TEST_ASSERT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    operating_state_get = get_func;
  } else if (
    node_type
    == ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_SUPPORTED_BITMASK) {
    TEST_ASSERT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    logging_supported_get = get_func;
  } else if (
    node_type
    == ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_BITMASK) {
    TEST_ASSERT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    logging_get = get_func;
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_THERMOSTAT_OPERATING_STATE,
                    handler.command_class);
  TEST_ASSERT_EQUAL(THERMOSTAT_OPERATING_STATE_VERSION_V2, handler.version);
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
  operating_state_get   = NULL;
  logging_supported_get = NULL;
  logging_get           = NULL;

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
                    zwave_command_class_thermostat_operating_state_init());
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
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_VERSION,
    endpoint_id_node);

  attribute_store_set_reported(version_node, &version, sizeof(version));
}

////////////////////////////////////////////////////////////////////////////
// Versioning
////////////////////////////////////////////////////////////////////////////
void test_thermostat_operating_state_v1_happy_case()
{
  set_version(1);

  attribute_store_node_t state_node = attribute_store_get_first_child_by_type(
    endpoint_id_node,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_CURRENT_STATE);

  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, state_node);

  attribute_store_node_t log_supported_node
    = attribute_store_get_first_child_by_type(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_SUPPORTED_BITMASK);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, log_supported_node);
}

void test_thermostat_operating_state_v2_happy_case()
{
  set_version(2);

  attribute_store_node_t state_node = attribute_store_get_first_child_by_type(
    endpoint_id_node,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_CURRENT_STATE);

  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, state_node);

  attribute_store_node_t log_supported_node
    = attribute_store_get_first_child_by_type(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_SUPPORTED_BITMASK);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, log_supported_node);
}

////////////////////////////////////////////////////////////////////////////
// Operating State Get/Report
////////////////////////////////////////////////////////////////////////////
void test_thermostat_operating_state_get_happy_case()
{
  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL(operating_state_get);
  operating_state_get(0, received_frame, &received_frame_size);
  const uint8_t expected_frame[] = {COMMAND_CLASS_THERMOSTAT_OPERATING_STATE,
                                    THERMOSTAT_OPERATING_STATE_GET};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_thermostat_operating_state_report_happy_case()
{
  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&info, NULL, 0));

  thermostat_operating_state_t operating_state
    = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_HEATING;
  const uint8_t frame[] = {COMMAND_CLASS_THERMOSTAT_OPERATING_STATE,
                           THERMOSTAT_OPERATING_STATE_REPORT,
                           operating_state};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info, frame, sizeof(frame)));

  attribute_store_node_t operating_state_node
    = attribute_store_get_node_child_by_type(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_CURRENT_STATE,
      0);
  TEST_ASSERT_EQUAL(operating_state,
                    attribute_store_get_reported_number(operating_state_node));
}

////////////////////////////////////////////////////////////////////////////
// Supported Log Get/Report
////////////////////////////////////////////////////////////////////////////
void test_thermostat_operating_state_log_supported_get_happy_case()
{
  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL(operating_state_get);
  logging_supported_get(0, received_frame, &received_frame_size);
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_THERMOSTAT_OPERATING_STATE,
       THERMOSTAT_OPERATING_STATE_LOGGING_SUPPORTED_GET_V2};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void helper_thermostat_operating_state_log_supported_report_happy_case(
  uint8_t test_case)
{
  printf("test_thermostat_operating_state_log_supported_report_happy_case test "
         "#%d\n",
         test_case);

  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&info, NULL, 0));

  uint8_t supported_log_count = 11;
  uint8_t bitmask1            = 0b11111111;
  uint8_t bitmask2            = 0b11111111;
  thermostat_operating_state_t expecting_states[11];

  switch (test_case) {
    case 1:  // All types
      supported_log_count = 11;
      bitmask1            = 0b11111111;
      bitmask2            = 0b11111111;
      expecting_states[0]
        = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_HEATING;
      expecting_states[1]
        = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_COOLING;
      expecting_states[2]
        = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_FAN_ONLY;
      expecting_states[3]
        = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_PENDING_HEAT;
      expecting_states[4]
        = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_PENDING_COOL;
      expecting_states[5]
        = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_VENT_ECONOMIZER;
      expecting_states[6]
        = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_AUX_HEATING_V2;
      expecting_states[7]
        = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_2ND_STAGE_HEATING_V2;
      expecting_states[8]
        = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_2ND_STAGE_COOLING_V2;
      expecting_states[9]
        = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_2ND_STAGE_AUX_HEAT_V2;
      expecting_states[10]
        = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_3RD_STAGE_AUX_HEAT_V2;
      break;

    case 2:  // Only bit 1
      supported_log_count = 2;
      bitmask1            = 0b10000001;
      bitmask2            = 0b00000000;
      expecting_states[0]
        = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_HEATING;
      expecting_states[1]
        = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_2ND_STAGE_HEATING_V2;
      break;

    case 3:  // Only bit 2
      supported_log_count = 2;
      bitmask1            = 0b00000000;
      bitmask2            = 0b00000011;
      expecting_states[0]
        = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_2ND_STAGE_COOLING_V2;
      expecting_states[1]
        = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_2ND_STAGE_AUX_HEAT_V2;
      break;

    case 4:  // Bit of both
      supported_log_count = 4;
      bitmask1            = 0b00101010;  // 3 Here
      bitmask2            = 0b11000100;  // Only 1 here
      expecting_states[0]
        = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_COOLING;
      expecting_states[1]
        = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_PENDING_HEAT;
      expecting_states[2]
        = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_VENT_ECONOMIZER;
      expecting_states[3]
        = THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_3RD_STAGE_AUX_HEAT_V2;
      break;

    default:
      TEST_MESSAGE("Undefined test_case aborting");
      TEST_ABORT();
  }

  const uint8_t frame[] = {COMMAND_CLASS_THERMOSTAT_OPERATING_STATE,
                           THERMOSTAT_OPERATING_LOGGING_SUPPORTED_REPORT_V2,
                           bitmask1,
                           bitmask2};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info, frame, sizeof(frame)));

  attribute_store_node_t operating_state_node
    = attribute_store_get_node_child_by_type(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_SUPPORTED_BITMASK,
      0);

  TEST_ASSERT_EQUAL((bitmask2 << 8) + bitmask1,
                    attribute_store_get_reported_number(operating_state_node));

  TEST_ASSERT_EQUAL(supported_log_count,
                    attribute_store_get_node_child_count(operating_state_node));

  for (int i = 0; i < supported_log_count; i++) {
    thermostat_operating_state_t expected_state = expecting_states[i];

    attribute_store_node_t current_node
      = attribute_store_get_node_child_by_type(
        operating_state_node,
        ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_SUPPORTED,
        i);

    TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, current_node);

    thermostat_operating_state_t reported_state;
    sl_status_t status = attribute_store_get_reported(current_node,
                                                      &reported_state,
                                                      sizeof(reported_state));

    TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
    TEST_ASSERT_EQUAL(expected_state, reported_state);
  }

  // Check if ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_BITMASK is created once we have the supported states
  attribute_store_node_t log_bitmask_node
    = attribute_store_get_first_child_by_type(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_BITMASK);

  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, log_bitmask_node);
}

void test_thermostat_operating_state_log_supported_report_happy_case()
{
  set_version(2);
  for (int i = 1; i <= 4; i++) {
    helper_thermostat_operating_state_log_supported_report_happy_case(i);
  }
}

void test_thermostat_operating_state_log_supported_report_versioning()
{
  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&info, NULL, 0));

  const uint8_t frame[] = {COMMAND_CLASS_THERMOSTAT_OPERATING_STATE,
                           THERMOSTAT_OPERATING_LOGGING_SUPPORTED_REPORT_V2,
                           0b0000000,
                           0b0000000};
  // Undefined in v1
  set_version(1);
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&info, frame, sizeof(frame)));

  set_version(2);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info, frame, sizeof(frame)));
}

////////////////////////////////////////////////////////////////////////////
// Log Get/Report
////////////////////////////////////////////////////////////////////////////

void test_thermostat_operating_state_log_get_happy_case()
{
  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL(operating_state_get);
  sl_status_t status = logging_get(0, received_frame, &received_frame_size);

  // No ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_BITMASK yet
  TEST_ASSERT_EQUAL(SL_STATUS_IS_WAITING, status);

  // We add it and test
  attribute_store_node_t log_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_BITMASK,
    endpoint_id_node);

  TEST_ASSERT_TRUE(attribute_store_node_exists(log_node));

  uint32_t expected_bitmask = 0b0000000100000110;
  attribute_store_set_desired(log_node,
                              &expected_bitmask,
                              sizeof(expected_bitmask));

  status = logging_get(log_node, received_frame, &received_frame_size);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);

  const uint8_t expected_frame[] = {COMMAND_CLASS_THERMOSTAT_OPERATING_STATE,
                                    THERMOSTAT_OPERATING_STATE_LOGGING_GET_V2,
                                    0b00000110,
                                    0b00000001};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_thermostat_operating_state_log_no_report_happy_case()
{
  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_BITMASK,
    endpoint_id_node);

  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&info, NULL, 0));

  const uint8_t frame[] = {COMMAND_CLASS_THERMOSTAT_OPERATING_STATE,
                           THERMOSTAT_OPERATING_STATE_LOGGING_REPORT_V2,
                           0};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info, frame, sizeof(frame)));
}

void test_thermostat_operating_state_log_report_happy_case()
{
  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  attribute_store_node_t log_node = attribute_store_add_node(
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_BITMASK,
    endpoint_id_node);

  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&info, NULL, 0));
  const uint8_t report_number = 2;
  const uint8_t reports[]
    = {THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_HEATING,
       11,
       12,
       13,
       14,
       THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_VENT_ECONOMIZER,
       21,
       22,
       23,
       24};

  const uint8_t frame[] = {
    COMMAND_CLASS_THERMOSTAT_OPERATING_STATE,
    THERMOSTAT_OPERATING_STATE_LOGGING_REPORT_V2,
    report_number,
    0b11110001,  // Test if we correctly ignore the first 4 bits (THERMOSTAT_OPERATING_STATE_REPORT_OPERATING_STATE_HEATING)
    reports[1],
    reports[2],
    reports[3],
    reports[4],
    reports[5],
    reports[6],
    reports[7],
    reports[8],
    reports[9],
  };

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info, frame, sizeof(frame)));

  TEST_ASSERT_EQUAL(report_number,
                    attribute_store_get_node_child_count(log_node));

  const attribute_store_type_t attribute_store_types[] = {
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_USAGE_TODAY_HOURS,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_USAGE_TODAY_MIN,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_USAGE_YESTERDAY_HOURS,
    ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_OPERATING_STATE_LOG_USAGE_YESTERDAY_MIN};

  for (uint8_t i = 0; i < report_number; i++) {
    attribute_store_node_t current_report_node
      = attribute_store_get_node_child(log_node, i);

    thermostat_operating_state_t reported_state;
    attribute_store_get_reported(current_report_node,
                                 &reported_state,
                                 sizeof(reported_state));
    // Expected index :
    // 0
    // 5
    TEST_ASSERT_EQUAL_MESSAGE(reports[(5 * i)],
                              reported_state,
                              "Error while checking Operating State Log Type");

    for (int j = 0; j < 4; j++) {
      thermostat_operating_state_usage_t reported_value;

      attribute_store_get_child_reported(current_report_node,
                                         attribute_store_types[j],
                                         &reported_value,
                                         sizeof(reported_value));
      // Expected index :
      // 1,2,3,4 (1 + (5*i) + j)
      // 6,7,8,9
      const uint8_t index = 1 + (5 * i) + j;
      printf("Checking report value of type %d with index %d\n",
             attribute_store_types[j],
             index);
      TEST_ASSERT_EQUAL_MESSAGE(
        reports[index],
        reported_value,
        "Error while checking Operating State Log contents");
    }
  }
}