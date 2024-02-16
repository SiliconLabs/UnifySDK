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

#include "zwave_command_class_humidity_control_operating_state.h"
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

static attribute_resolver_function_t current_humidity_control_state_get = NULL;

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
  if (
    node_type
    == ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_OPERATING_STATE_CURRENT_STATE) {
    TEST_ASSERT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    current_humidity_control_state_get = get_func;
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_HUMIDITY_CONTROL_OPERATING_STATE,
                    handler.command_class);
  TEST_ASSERT_EQUAL(1, handler.version);
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
  current_humidity_control_state_get = NULL;
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
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    zwave_command_class_humidity_control_operating_state_init());
}

/// Called after each and every test
void tearDown() {}

////////////////////////////////////////////////////////////////////////////
// HELPERS
////////////////////////////////////////////////////////////////////////////
// Happy case : not setting reserved bit to 1
void helper_humidity_control_state_report(
  humidity_control_operating_state_t expected_state, bool happy_case)
{
  uint8_t happy_case_mask = happy_case ? 0x00 : 0xF0;

  const uint8_t frame[] = {COMMAND_CLASS_HUMIDITY_CONTROL_OPERATING_STATE,
                           HUMIDITY_CONTROL_OPERATING_STATE_REPORT,
                           expected_state | happy_case_mask};

  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info, frame, sizeof(frame)));

  attribute_store_node_t humidity_control_state_node
    = attribute_store_get_node_child_by_type(
      endpoint_id_node,
      ATTRIBUTE_COMMAND_CLASS_HUMIDITY_CONTROL_OPERATING_STATE_CURRENT_STATE,
      0);

  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                        humidity_control_state_node);

  humidity_control_operating_state_t reported_state = 0x00;
  attribute_store_get_reported(humidity_control_state_node,
                               &reported_state,
                               sizeof(reported_state));
  TEST_ASSERT_EQUAL(expected_state, reported_state);
}

////////////////////////////////////////////////////////////////////////////
// THERMOSTAT_FAN_STATE
////////////////////////////////////////////////////////////////////////////
void test_humidity_control_operating_state_get_happy_case()
{
  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL(current_humidity_control_state_get);
  current_humidity_control_state_get(0, received_frame, &received_frame_size);
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_HUMIDITY_CONTROL_OPERATING_STATE,
       HUMIDITY_CONTROL_OPERATING_STATE_GET};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_humidity_control_operating_state_report_happy_case()
{
  helper_humidity_control_state_report(0x02, true);
}

void test_humidity_control_operating_state_report_off_bit()
{
  helper_humidity_control_state_report(0x03, false);
}