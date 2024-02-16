/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "unity.h"
#include "zwave_command_class_thermostat_setpoint.h"

// Generic includes
#include <string.h>

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "zpc_attribute_store_type_registration.h"
#include "zwave_unid.h"

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

// Attribute macro, shortening those long defines for attribute types:
#define ATTRIBUTE(type) ATTRIBUTE_COMMAND_CLASS_THERMOSTAT_SETPOINT_##type

// Static variables
static attribute_resolver_function_t thermostat_setpoint_set = NULL;
static attribute_resolver_function_t thermostat_setpoint_get = NULL;
static attribute_resolver_function_t thermostat_setpoint_capabilities_get
  = NULL;
static attribute_resolver_function_t thermostat_setpoint_supported_get = NULL;
static zwave_command_handler_t thermostat_handler                      = {};

static uint8_t received_frame[255]  = {};
static uint16_t received_frame_size = 0;

// Stub functions
static sl_status_t
  attribute_resolver_register_rule_stub(attribute_store_type_t node_type,
                                        attribute_resolver_function_t set_func,
                                        attribute_resolver_function_t get_func,
                                        int cmock_num_calls)
{
  if (node_type == ATTRIBUTE(VALUE)) {
    TEST_ASSERT_NOT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    thermostat_setpoint_set = set_func;
    thermostat_setpoint_get = get_func;
  } else if (node_type == ATTRIBUTE(MIN_VALUE)) {
    TEST_ASSERT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    thermostat_setpoint_capabilities_get = get_func;
  } else if (node_type == ATTRIBUTE(SUPPORTED_SETPOINT_TYPES)) {
    TEST_ASSERT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    thermostat_setpoint_supported_get = get_func;
  } else {
    TEST_FAIL_MESSAGE("Attribute rule registration on a wrong type");
  }

  return SL_STATUS_OK;
}

static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  thermostat_handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    thermostat_handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_THERMOSTAT_SETPOINT_V3,
                    thermostat_handler.command_class);
  TEST_ASSERT_EQUAL(THERMOSTAT_SETPOINT_VERSION_V3, thermostat_handler.version);
  TEST_ASSERT_NOT_NULL(thermostat_handler.control_handler);
  TEST_ASSERT_NULL(thermostat_handler.support_handler);
  TEST_ASSERT_FALSE(thermostat_handler.manual_security_validation);

  return SL_STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
// UTILS
////////////////////////////////////////////////////////////////////////////
// Set version and thus initialize the attribute tree
void set_version(zwave_cc_version_t version)
{
  attribute_store_node_t version_node
    = attribute_store_add_node(ATTRIBUTE(VERSION), endpoint_id_node);

  attribute_store_set_reported(version_node, &version, sizeof(version));
}

// Test if all the types in setpoint_mode_expected_types are present in the attribute tree
void helper_setpoint_type_validator(const uint8_t* setpoint_mode_expected_types,
                                    uint8_t setpoint_mode_excepted_length)
{
  size_t attribute_count
    = attribute_store_get_node_child_count_by_type(endpoint_id_node,
                                                   ATTRIBUTE(TYPE));

  TEST_ASSERT_EQUAL_MESSAGE(
    setpoint_mode_excepted_length,
    attribute_count,
    "Incorrect supported thermostat setpoint mode NODE count");

  for (int i = 0; i < setpoint_mode_excepted_length; i++) {
    attribute_store_node_t fan_mode_supported_node
      = attribute_store_get_node_child_by_type(endpoint_id_node,
                                               ATTRIBUTE(TYPE),
                                               i);

    uint8_t reported_type;
    attribute_store_get_reported(fan_mode_supported_node,
                                 &reported_type,
                                 sizeof(reported_type));

    TEST_ASSERT_EQUAL_MESSAGE(
      setpoint_mode_expected_types[i],
      reported_type,
      "Incorrect reported thermostat setpoint type supported");
  }
}

// happy_case : if true only accepted bit are sent
//              if false fill all unused bit with 1 to see if they are ignored correctly
void helper_thermostat_setpoint_mode_supported_report(
  zwave_cc_version_t version, bool happy_case, bool use_b_interpretation)
{
  set_version(version);

  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  TEST_ASSERT_NOT_NULL(thermostat_handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    thermostat_handler.control_handler(&info, NULL, 0));

  uint8_t supported_bit1          = 0b00000000;
  uint8_t supported_bit2          = 0b00000000;
  size_t setpoint_mode_excepted_length = 0;
  uint8_t setpoint_mode_expected_types[16];

  uint8_t use_b_interpretation_value = use_b_interpretation;

  attribute_store_node_t thermostat_setpoint_bitmask_node
    = attribute_store_get_node_child_by_type(
      endpoint_id_node,
      ATTRIBUTE(SUPPORTED_SETPOINT_TYPES),
      0);

  sl_status_t result
    = attribute_store_set_child_reported(thermostat_setpoint_bitmask_node,
                                         ATTRIBUTE(USE_B_INTERPRETATION),
                                         &use_b_interpretation_value,
                                         sizeof(use_b_interpretation_value));

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            result,
                            "Can't set b interpretation flag");

  switch (version) {
    case 1:
      if (use_b_interpretation) {
        supported_bit1 = happy_case ? 0b10000110 : 0b11111111;
        supported_bit2 = happy_case ? 0b00000111 : 0b11111111;
      } else {
        supported_bit1 = happy_case ? 0b01111110 : 0b11111111;
        supported_bit2 = happy_case ? 0b00000000 : 0b11111111;
      }
      setpoint_mode_excepted_length = 6;
      setpoint_mode_expected_types[0]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_HEATING_1;
      setpoint_mode_expected_types[1]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_COOLING_1;
      setpoint_mode_expected_types[2]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_FURNACE_V3;
      setpoint_mode_expected_types[3]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_DRY_AIR_V3;
      setpoint_mode_expected_types[4]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_MOIST_AIR_V3;
      setpoint_mode_expected_types[5]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_AUTO_CHANGEOVER_V3;
      break;

    case 2:
      if (use_b_interpretation) {
        supported_bit1 = happy_case ? 0b10000110 : 0b11111111;
        supported_bit2 = happy_case ? 0b00111111 : 0b11111111;
      } else {
        supported_bit1 = happy_case ? 0b11111110 : 0b11111111;
        supported_bit2 = happy_case ? 0b00000011 : 0b11111111;
      }
      setpoint_mode_excepted_length = 9;
      setpoint_mode_expected_types[0]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_HEATING_1_V2;
      setpoint_mode_expected_types[1]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_COOLING_1_V2;
      setpoint_mode_expected_types[2]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_FURNACE_V2;
      setpoint_mode_expected_types[3]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_DRY_AIR_V2;
      setpoint_mode_expected_types[4]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_MOIST_AIR_V2;
      setpoint_mode_expected_types[5]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_AUTO_CHANGEOVER_V2;
      setpoint_mode_expected_types[6]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_ENERGY_SAVE_HEATING_V2;
      setpoint_mode_expected_types[7]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_ENERGY_SAVE_COOLING_V2;
      setpoint_mode_expected_types[8]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_AWAY_HEATING_V2;
      break;
    case 3:
      if (use_b_interpretation) {
        supported_bit1 = happy_case ? 0b10000110 : 0b11111111;
        supported_bit2 = happy_case ? 0b11111111 : 0b11111111;
      } else {
        supported_bit1 = happy_case ? 0b11111110 : 0b11111111;
        supported_bit2 = happy_case ? 0b00001111 : 0b11111111;
      }
      setpoint_mode_excepted_length = 11;
      setpoint_mode_expected_types[0]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_HEATING_1_V3;
      setpoint_mode_expected_types[1]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_COOLING_1_V3;
      setpoint_mode_expected_types[2]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_FURNACE_V3;
      setpoint_mode_expected_types[3]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_DRY_AIR_V3;
      setpoint_mode_expected_types[4]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_MOIST_AIR_V3;
      setpoint_mode_expected_types[5]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_AUTO_CHANGEOVER_V3;
      setpoint_mode_expected_types[6]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_ENERGY_SAVE_HEATING_V3;
      setpoint_mode_expected_types[7]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_ENERGY_SAVE_COOLING_V3;
      setpoint_mode_expected_types[8]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_AWAY_HEATING_V3;
      setpoint_mode_expected_types[9]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_AWAY_COOLING_V3;
      setpoint_mode_expected_types[10]
        = THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_FULL_POWER_V3;
      break;
  }

  const uint8_t frame[] = {COMMAND_CLASS_THERMOSTAT_SETPOINT,
                           THERMOSTAT_SETPOINT_SUPPORTED_REPORT,
                           supported_bit1,
                           supported_bit2};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    thermostat_handler.control_handler(&info, frame, sizeof(frame)));



  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                        thermostat_setpoint_bitmask_node);

  uint32_t reported_bitmask = 0;
  attribute_store_get_reported(thermostat_setpoint_bitmask_node,
                               &reported_bitmask,
                               sizeof(reported_bitmask));
  uint32_t expected_bitmask = (supported_bit2 << 8) | supported_bit1;
  TEST_ASSERT_EQUAL_MESSAGE(
    expected_bitmask,
    reported_bitmask,
    "Incorrect supported thermostat setpoint mode bitmask");

  helper_setpoint_type_validator(setpoint_mode_expected_types,
                                 setpoint_mode_excepted_length);
}

////////////////////////////////////////////////////////////////////////////
// Setup
////////////////////////////////////////////////////////////////////////////

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  zpc_attribute_store_register_known_attribute_types();
  zwave_unid_set_home_id(home_id);
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

static void zwave_command_class_thermostat_setpoint_init_verification()
{
  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);

  // Handler registration
  zwave_command_handler_register_handler_Stub(
    &zwave_command_handler_register_handler_stub);

  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_command_class_thermostat_setpoint_init());
}

/// Called before each and every test
void setUp()
{
  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size = 0;
  memset(&thermostat_handler, 0, sizeof(zwave_command_handler_t));
  thermostat_setpoint_set              = NULL;
  thermostat_setpoint_get              = NULL;
  thermostat_setpoint_capabilities_get = NULL;
  thermostat_setpoint_supported_get    = NULL;

  zpc_attribute_store_test_helper_create_network();

  zwave_command_class_thermostat_setpoint_init_verification();
}

void test_zwave_command_class_thermostat_setpoint_set_v1()
{
  TEST_ASSERT_NOT_NULL(thermostat_setpoint_set);

  // Version 1 node
  const zwave_cc_version_t version = 1;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(VERSION),
                                     &version,
                                     sizeof(version));
  const uint8_t type = 34;
  attribute_store_node_t setpoint_type_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(TYPE),
                              &type,
                              sizeof(type));

  int32_t desired_value = 123456;
  attribute_store_node_t value_node
    = attribute_store_add_node(ATTRIBUTE(VALUE), setpoint_type_node);
  attribute_store_set_desired(value_node,
                              &desired_value,
                              sizeof(desired_value));

  TEST_ASSERT_NOT_NULL(thermostat_setpoint_set);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    thermostat_setpoint_set(value_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_THERMOSTAT_SETPOINT_V3,
                                    THERMOSTAT_SETPOINT_SET_V3,
                                    type,
                                    SET_DEFAULT_PRECISION | SET_DEFAULT_SIZE,
                                    (desired_value & 0xFF000000) >> 24,  // MSB
                                    (desired_value & 0x00FF0000) >> 16,
                                    (desired_value & 0x0000FF00) >> 8,
                                    (desired_value & 0x000000FF)};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_zwave_command_class_thermostat_setpoint_set_v3_clip_lower_bound()
{
  TEST_ASSERT_NOT_NULL(thermostat_setpoint_set);

  // Version 1 node
  const zwave_cc_version_t version = 3;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(VERSION),
                                     &version,
                                     sizeof(version));
  const uint8_t type = 2;
  attribute_store_node_t setpoint_type_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(TYPE),
                              &type,
                              sizeof(type));

  int32_t desired_value = DEFAULT_MIN_VALUE - 10;
  attribute_store_node_t value_node
    = attribute_store_add_node(ATTRIBUTE(VALUE), setpoint_type_node);
  attribute_store_set_desired(value_node,
                              &desired_value,
                              sizeof(desired_value));

  const int32_t min_value = DEFAULT_MIN_VALUE;
  attribute_store_set_child_reported(setpoint_type_node,
                                     ATTRIBUTE(MIN_VALUE),
                                     &min_value,
                                     sizeof(min_value));

  TEST_ASSERT_NOT_NULL(thermostat_setpoint_set);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    thermostat_setpoint_set(value_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_THERMOSTAT_SETPOINT_V3,
                                    THERMOSTAT_SETPOINT_SET_V3,
                                    type,
                                    SET_DEFAULT_PRECISION | SET_DEFAULT_SIZE,
                                    (min_value & 0xFF000000) >> 24,  // MSB
                                    (min_value & 0x00FF0000) >> 16,
                                    (min_value & 0x0000FF00) >> 8,
                                    (min_value & 0x000000FF)};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_zwave_command_class_thermostat_setpoint_set_v3_clip_upper_bound()
{
  TEST_ASSERT_NOT_NULL(thermostat_setpoint_set);

  // Version 1 node
  const zwave_cc_version_t version = 3;
  attribute_store_set_child_reported(endpoint_id_node,
                                     ATTRIBUTE(VERSION),
                                     &version,
                                     sizeof(version));
  const uint8_t type = 2;
  attribute_store_node_t setpoint_type_node
    = attribute_store_emplace(endpoint_id_node,
                              ATTRIBUTE(TYPE),
                              &type,
                              sizeof(type));

  int32_t desired_value = DEFAULT_MAX_VALUE + 10;
  attribute_store_node_t value_node
    = attribute_store_add_node(ATTRIBUTE(VALUE), setpoint_type_node);
  attribute_store_set_desired(value_node,
                              &desired_value,
                              sizeof(desired_value));

  const int32_t max_value = DEFAULT_MAX_VALUE;
  attribute_store_set_child_reported(setpoint_type_node,
                                     ATTRIBUTE(MAX_VALUE),
                                     &max_value,
                                     sizeof(max_value));

  TEST_ASSERT_NOT_NULL(thermostat_setpoint_set);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    thermostat_setpoint_set(value_node, received_frame, &received_frame_size));

  const uint8_t expected_frame[] = {COMMAND_CLASS_THERMOSTAT_SETPOINT_V3,
                                    THERMOSTAT_SETPOINT_SET_V3,
                                    type,
                                    SET_DEFAULT_PRECISION | SET_DEFAULT_SIZE,
                                    (max_value & 0xFF000000) >> 24,  // MSB
                                    (max_value & 0x00FF0000) >> 16,
                                    (max_value & 0x0000FF00) >> 8,
                                    (max_value & 0x000000FF)};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                sizeof(expected_frame));
}

void test_thermostat_setpoint_mode_supported_report_v1_happy_case()
{
  helper_thermostat_setpoint_mode_supported_report(THERMOSTAT_SETPOINT_VERSION,
                                                   true,
                                                   false);
}
void test_thermostat_setpoint_mode_supported_report_v2_happy_case()
{
  helper_thermostat_setpoint_mode_supported_report(
    THERMOSTAT_SETPOINT_VERSION_V2,
    true,
    false);
}
void test_thermostat_setpoint_mode_supported_report_v3_happy_case()
{
  helper_thermostat_setpoint_mode_supported_report(
    THERMOSTAT_SETPOINT_VERSION_V3,
    true,
    false);
}

void test_thermostat_setpoint_mode_supported_report_v1_b_interpretation_happy_case()
{
  helper_thermostat_setpoint_mode_supported_report(THERMOSTAT_SETPOINT_VERSION,
                                                   true,
                                                   true);
}
void test_thermostat_setpoint_mode_supported_report_v2_b_interpretation_happy_case()
{
  helper_thermostat_setpoint_mode_supported_report(
    THERMOSTAT_SETPOINT_VERSION_V2,
    true,
    true);
}
void test_thermostat_setpoint_mode_supported_report_v3_b_interpretation_happy_case()
{
  helper_thermostat_setpoint_mode_supported_report(
    THERMOSTAT_SETPOINT_VERSION_V3,
    true,
    true);
}

void test_thermostat_setpoint_mode_supported_report_v1_bit_mismatch()
{
  helper_thermostat_setpoint_mode_supported_report(THERMOSTAT_SETPOINT_VERSION,
                                                   false,
                                                   false);
}
void test_thermostat_setpoint_mode_supported_report_v2_bit_mismatch()
{
  helper_thermostat_setpoint_mode_supported_report(
    THERMOSTAT_SETPOINT_VERSION_V2,
    false,
    false);
}
void test_thermostat_setpoint_mode_supported_report_v3_bit_mismatch()
{
  helper_thermostat_setpoint_mode_supported_report(
    THERMOSTAT_SETPOINT_VERSION_V3,
    false,
    false);
}

void test_thermostat_setpoint_mode_supported_report_v1_b_interpretation_bit_mismatch()
{
  helper_thermostat_setpoint_mode_supported_report(THERMOSTAT_SETPOINT_VERSION,
                                                   false,
                                                   true);
}
void test_thermostat_setpoint_mode_supported_report_v2_b_interpretation_bit_mismatch()
{
  helper_thermostat_setpoint_mode_supported_report(
    THERMOSTAT_SETPOINT_VERSION_V2,
    false,
    true);
}

void test_thermostat_setpoint_mode_supported_report_v3_b_interpretation_bit_mismatch()
{
  helper_thermostat_setpoint_mode_supported_report(
    THERMOSTAT_SETPOINT_VERSION_V3,
    false,
    true);
}

void test_thermostat_setpoint_mode_change_interpretation()
{
  set_version(2);

  attribute_store_node_t supported_setpoint_types_node
    = attribute_store_get_first_child_by_type(
      endpoint_id_node,
      ATTRIBUTE(SUPPORTED_SETPOINT_TYPES));
  attribute_store_node_t b_interpretation_node
    = attribute_store_get_first_child_by_type(supported_setpoint_types_node,
                                              ATTRIBUTE(USE_B_INTERPRETATION));

  uint32_t setpoint_bitmask = 0b11111110;
  sl_status_t status = attribute_store_set_reported(supported_setpoint_types_node,
                               &setpoint_bitmask,
                               sizeof(setpoint_bitmask));

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK, status, "Should be able to set supported setpoint types");

  
  // Check with B interpretation (flag = on)
  uint8_t flag = 1;
  // This should make the attribute store to update based on the new interpretation 
  status = attribute_store_set_reported(b_interpretation_node,
                               &flag,
                               sizeof(flag));

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK, status, "Should be able to set b interpretation flag to 1");

  const uint8_t b_interpretation_expected_types[] = {
    THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_HEATING_1,
    THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_COOLING_1,
    THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_FURNACE_V2
  };
  helper_setpoint_type_validator(b_interpretation_expected_types,
                                 sizeof(b_interpretation_expected_types));

  // Check with A interpretation (flag = off)
  flag = 0;
  // This should make the attribute store to update based on the new interpretation 
  status = attribute_store_set_reported(b_interpretation_node,
                               &flag,
                               sizeof(flag));

  const uint8_t a_interpretation_expected_types[] = {
    THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_HEATING_1,
    THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_COOLING_1,
    THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_FURNACE_V3,
    THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_DRY_AIR_V3,
    THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_MOIST_AIR_V3,
    THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_AUTO_CHANGEOVER_V3,
    THERMOSTAT_SETPOINT_REPORT_SETPOINT_TYPE_ENERGY_SAVE_HEATING_V3
  };
  helper_setpoint_type_validator(a_interpretation_expected_types,
                                 sizeof(a_interpretation_expected_types));

}

void test_thermostat_setpoint_mode_attribute_creation() {
  attribute_store_node_t supported_setpoint_types_node =
    attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(SUPPORTED_SETPOINT_TYPES));
  
  attribute_store_node_t b_interpretation_node
    = attribute_store_get_first_child_by_type(supported_setpoint_types_node,
                                              ATTRIBUTE(USE_B_INTERPRETATION));

  TEST_ASSERT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                       b_interpretation_node,
                       "USE_B_INTERPRETATION attribute should not exist yet");
  TEST_ASSERT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                       supported_setpoint_types_node,
                       "SUPPORTED_SETPOINT_TYPES attribute should not exist yet");

  // Trigger creation version attributes
  set_version(1);

  supported_setpoint_types_node =
    attribute_store_get_first_child_by_type(endpoint_id_node,
                                              ATTRIBUTE(SUPPORTED_SETPOINT_TYPES));
  
  b_interpretation_node
    = attribute_store_get_first_child_by_type(supported_setpoint_types_node,
                                              ATTRIBUTE(USE_B_INTERPRETATION));

  TEST_ASSERT_NOT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                       b_interpretation_node,
                       "USE_B_INTERPRETATION attribute should exist by now");
  
  TEST_ASSERT_NOT_EQUAL_MESSAGE(ATTRIBUTE_STORE_INVALID_NODE,
                       supported_setpoint_types_node,
                       "SUPPORTED_SETPOINT_TYPES attribute should exist by now");


  uint8_t b_interpretation_value = 1;
  attribute_store_get_reported(b_interpretation_node,
                               &b_interpretation_value,
                               sizeof(b_interpretation_value));
  TEST_ASSERT_EQUAL_MESSAGE(0,
                       b_interpretation_value,
                       "Default value of USE_B_INTERPRETATION should be 0");                      

  b_interpretation_value = 1;
  sl_status_t status
    = attribute_store_set_reported(b_interpretation_node,
                                   &b_interpretation_value,
                                   sizeof(b_interpretation_value));
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                       status,
                       "Should be able to modify b interpretation_node to 1");

  // Trigger version change to see if flag value is not reset back to 0
  set_version(2);

  attribute_store_get_reported(b_interpretation_node,
                               &b_interpretation_value,
                               sizeof(b_interpretation_value));
  TEST_ASSERT_EQUAL_MESSAGE(1,
                       b_interpretation_value,
                       "Current value of USE_B_INTERPRETATION should be 1");  
}