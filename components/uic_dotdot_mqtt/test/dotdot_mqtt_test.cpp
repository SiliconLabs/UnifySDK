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

#include "workaround.hpp"
#include "unity.h"
#include "unity_helpers.h"
#include "cmock.h"

#include "sl_status.h"
#include "mqtt_test_helper.h"

#include "stdint.h"

extern "C" {
#include "dotdot_mqtt.h"
#include "dotdot_mqtt_generated_commands.h"

#include "dotdot_mqtt_test.include"

static int callback_count = 0;

/// Called before each and every test
void setUp()
{
  callback_count = 0;
  mqtt_test_helper_init();
  unset_all_callbacks();
  reset_callback_counters();
  num_command_callbacks = 0;
}

void test_dotdot_mqtt_happy_no_registered_handlers()
{
  uic_mqtt_dotdot_init();
  TEST_ASSERT_EQUAL(0, mqtt_test_helper_get_total_num_subscribers());
}

void test_dotdot_mqtt_commands_invalid_json()
{
  unsigned int num_callbacks = set_all_callbacks();
  uic_mqtt_dotdot_init();
  // NOTE: The by-group subscriptions are registered (x2 registrations) only if
  // the group dispatch callback is set. The tests currently do not do this.
  TEST_ASSERT_EQUAL(num_callbacks,
                    mqtt_test_helper_get_total_num_subscribers());

  // Test invalid json
  std::string json_str = "{\"foo: \"bar\"}";
  mqtt_test_helper_publish_to_all_topics(json_str.c_str(), json_str.length());
  TEST_ASSERT_EQUAL(0, num_command_callbacks);

  // Test empty json
  json_str = "";
  mqtt_test_helper_publish_to_all_topics(json_str.c_str(), json_str.length());
  TEST_ASSERT_EQUAL(0, num_command_callbacks);

  // Test valid but incomplete json
  json_str = R"({"foo":"bar")";
  mqtt_test_helper_publish_to_all_topics(json_str.c_str(), json_str.length());
  TEST_ASSERT_EQUAL(0, num_command_callbacks);
  unset_all_callbacks();
}

void test_dotdot_mqtt_missing_required()
{
  // We're arbitrarily using the DoorLock_LockDoor because it's as good as any.
  uic_mqtt_dotdot_door_lock_lock_door_callback_set(
    &uic_mqtt_dotdot_door_lock_lock_door_callback_func);
  uic_mqtt_dotdot_init();
  std::string msg    = R"({"PINOrRFIDCode":"1234"})";
  const char topic[] = "ucl/by-unid/zwDEADBEEF/ep0/DoorLock/Commands/LockDoor";
  mqtt_test_helper_publish(topic, msg.c_str(), msg.length());
  msg = R"({"Wazzap":"1234"})";
  mqtt_test_helper_publish(topic, msg.c_str(), msg.length());
  // Only the valid payload should have gone all the way through.
  TEST_ASSERT_EQUAL_INT(1, uic_mqtt_dotdot_door_lock_lock_door_callback_count);
}

// Write attributes test

uint8_t thermostat_systemmode = 0xff;
sl_status_t thermostat_WriteAttribute_callback(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uic_mqtt_dotdot_thermostat_state_t state,
  uic_mqtt_dotdot_thermostat_updated_state_t updated)
{
  if (updated.system_mode == true) {
    thermostat_systemmode = state.system_mode;
  }

  return SL_STATUS_OK;
}

// Test WriteAttributes accept enum as integer string
void test_dotdot_mqtt_write_attributes_integer_string()
{
  uic_mqtt_dotdot_set_thermostat_write_attributes_callback(
    thermostat_WriteAttribute_callback);
  uic_mqtt_dotdot_init();
  const char topic[]
    = "ucl/by-unid/zwDEADBEEF/ep0/Thermostat/Commands/WriteAttributes";

  // Test as integer string
  std::string payload = R"({"SystemMode": ")"
                        + std::to_string(ZCL_THERMOSTAT_SYSTEM_MODE_FAN_ONLY)
                        + "\"}";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL_INT_MESSAGE(
    static_cast<uint8_t>(ZCL_THERMOSTAT_SYSTEM_MODE_FAN_ONLY),
    thermostat_systemmode,
    "Test enum as int string");

  // Test as json integer
  payload = R"({"SystemMode": )"
            + std::to_string(ZCL_THERMOSTAT_SYSTEM_MODE_COOL) + "}";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL_INT_MESSAGE(
    static_cast<uint8_t>(ZCL_THERMOSTAT_SYSTEM_MODE_COOL),
    thermostat_systemmode,
    "Test enum as json int");

  // Test as string name
  payload = R"({"SystemMode": "EmergencyHeating"})";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL_INT_MESSAGE(
    static_cast<uint8_t>(ZCL_THERMOSTAT_SYSTEM_MODE_EMERGENCY_HEATING),
    thermostat_systemmode,
    "Test enum as String name");
}

static bool doorlock_enablelogging = false;
static sl_status_t doorlock_WriteAttribute_callback(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uic_mqtt_dotdot_door_lock_state_t state,
  uic_mqtt_dotdot_door_lock_updated_state_t updated)
{
  if (updated.enable_logging == true) {
    doorlock_enablelogging = state.enable_logging;
  }

  return SL_STATUS_OK;
}

// Test WriteAttributes accept bool
void test_dotdot_mqtt_write_attributes_bool()
{
  uic_mqtt_dotdot_set_door_lock_write_attributes_callback(
    doorlock_WriteAttribute_callback);
  uic_mqtt_dotdot_init();
  //"ucl/by-unid/zw-E18C3EA9-0005/ep0/Thermostat/WriteAttributes",
  const char topic[]
    = "ucl/by-unid/zwDEADBEEF/ep0/DoorLock/Commands/WriteAttributes";
  // as json bool
  std::string payload = R"({"EnableLogging": false})";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_FALSE_MESSAGE(doorlock_enablelogging, "Test bool as json bool");

  payload = R"({"EnableLogging": "true"})";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_TRUE_MESSAGE(doorlock_enablelogging, "Test bool as sting");

  payload = R"({"EnableLogging": 0})";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_FALSE_MESSAGE(doorlock_enablelogging, "Test bool as integer");

  payload = R"({"EnableLogging": "0"})";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_FALSE_MESSAGE(doorlock_enablelogging,
                            "Test bool as string with number '0' inside");

  payload = R"({"EnableLogging": "123"})";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_TRUE_MESSAGE(doorlock_enablelogging,
                           "Test bool as string with number '123' inside");
}

// Publish attributes tests

void test_dotdot_mqtt_publish_attributes_int()
{
  const char *topic_desired = "test/hest/Level/Attributes/CurrentLevel/Desired";
  const char *topic_reported
    = "test/hest/Level/Attributes/CurrentLevel/Reported";
  uic_mqtt_dotdot_level_current_level_publish("test/hest",
                                              42,
                                              UCL_MQTT_PUBLISH_TYPE_ALL);
  TEST_ASSERT_EQUAL(1, mqtt_test_helper_get_num_publish(topic_reported));
  TEST_ASSERT_EQUAL(1, mqtt_test_helper_get_num_publish(topic_desired));
  char result[100];
  mqtt_test_helper_pop_publish(topic_reported, result);
  TEST_ASSERT_EQUAL_JSON(R"({"value":42})", result);
  mqtt_test_helper_pop_publish(topic_desired, result);
  TEST_ASSERT_EQUAL_JSON(R"({"value":42})", result);
}

void test_dotdot_mqtt_publish_attributes_enum()
{
  const char *topic_desired
    = "test/hest/OTAUpgrade/Attributes/UpgradeTimeoutPolicy/Desired";
  const char *topic_reported
    = "test/hest/OTAUpgrade/Attributes/UpgradeTimeoutPolicy/Reported";
  uic_mqtt_dotdot_ota_upgrade_upgrade_timeout_policy_publish(
    "test/hest",
    ZCL_OTA_UPGRADE_UPGRADE_TIMEOUT_POLICY_APPLY_UPGRADE_AFTER_TIMEOUT,
    UCL_MQTT_PUBLISH_TYPE_ALL);
  TEST_ASSERT_EQUAL(1, mqtt_test_helper_get_num_publish(topic_reported));
  TEST_ASSERT_EQUAL(1, mqtt_test_helper_get_num_publish(topic_desired));
  char result[100];
  mqtt_test_helper_pop_publish(topic_reported, result);
  TEST_ASSERT_EQUAL_JSON(R"({"value":"ApplyUpgradeAfterTimeout"})", result);
  mqtt_test_helper_pop_publish(topic_desired, result);
  TEST_ASSERT_EQUAL_JSON(R"({"value":"ApplyUpgradeAfterTimeout"})", result);
}

static uint16_t user_id_value     = 0;
static uint8_t user_status_value  = 0;
static uint8_t user_type_value    = 0;
static std::string pin_code_value = "";
static sl_status_t uic_mqtt_dotdot_door_lock_setpin_code_test_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t callback_type,
  DrlkPINUserID user_id,
  DrlkSettableUserStatus user_status,
  DrlkUserType user_type,
  const char *pin_code)
{
  user_id_value     = user_id;
  user_status_value = user_status;
  user_type_value   = user_type;
  pin_code_value    = pin_code;

  return SL_STATUS_OK;
}

void test_dotdot_mqtt_enum_commands()
{
  const char topic[]
    = "ucl/by-unid/zwDEADBEEF/ep0/DoorLock/Commands/SetPINCode";

  uic_mqtt_dotdot_door_lock_setpin_code_callback_set(
    &uic_mqtt_dotdot_door_lock_setpin_code_test_callback);
  uic_mqtt_dotdot_init();

  std::string payload = R"({
      "UserID": 10,
      "UserStatus": "OccupiedEnabled",
      "UserType": "1",
      "PIN": "1243"
    })";

  uint16_t expected_user_id     = 10;
  uint8_t expected_user_status  = 1;
  uint8_t expected_user_type    = 1;
  std::string expected_pin_code = "1243";

  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());

  TEST_ASSERT_EQUAL(expected_user_id, user_id_value);
  TEST_ASSERT_EQUAL(expected_user_status, user_status_value);
  TEST_ASSERT_EQUAL(expected_user_type, user_type_value);
  TEST_ASSERT_EQUAL_STRING(expected_pin_code.c_str(), pin_code_value.c_str());

  user_id_value     = 0;
  user_status_value = 0;
  user_type_value   = 0;
  pin_code_value    = "";

  payload = R"({
      "UserID": 100,
      "UserStatus": "OccupiedDisabled",
      "UserType": 255,
      "PIN": "6666"
    })";

  expected_user_id     = 100;
  expected_user_status = 3;
  expected_user_type   = 255;
  expected_pin_code    = "6666";

  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());

  TEST_ASSERT_EQUAL(expected_user_id, user_id_value);
  TEST_ASSERT_EQUAL(expected_user_status, user_status_value);
  TEST_ASSERT_EQUAL(expected_user_type, user_type_value);
  TEST_ASSERT_EQUAL_STRING(expected_pin_code.c_str(), pin_code_value.c_str());
}

void test_dotdot_mqtt_publish_attributes_string()
{
  const char *topic_desired = "test/hest/DoorLock/Attributes/Language/Desired";
  const char *topic_reported
    = "test/hest/DoorLock/Attributes/Language/Reported";
  uic_mqtt_dotdot_door_lock_language_publish("test/hest",
                                             "Danglish",
                                             UCL_MQTT_PUBLISH_TYPE_ALL);
  TEST_ASSERT_EQUAL(1, mqtt_test_helper_get_num_publish(topic_reported));
  TEST_ASSERT_EQUAL(1, mqtt_test_helper_get_num_publish(topic_desired));
  char result[100];
  mqtt_test_helper_pop_publish(topic_reported, result);
  TEST_ASSERT_EQUAL_JSON(R"({"value":"Danglish"})", result);
  mqtt_test_helper_pop_publish(topic_desired, result);
  TEST_ASSERT_EQUAL_JSON(R"({"value":"Danglish"})", result);
}

// Test publishing reported and desired individually
void test_dotdot_mqtt_publish_attributes_reported_or_desired()
{
  const char *topic_desired = "test/hest/Level/Attributes/CurrentLevel/Desired";
  const char *topic_reported
    = "test/hest/Level/Attributes/CurrentLevel/Reported";
  uic_mqtt_dotdot_level_current_level_publish("test/hest",
                                              1,
                                              UCL_MQTT_PUBLISH_TYPE_DESIRED);
  TEST_ASSERT_EQUAL(1, mqtt_test_helper_get_num_publish(topic_desired));
  TEST_ASSERT_EQUAL(0, mqtt_test_helper_get_num_publish(topic_reported));
  char result[100];
  uic_mqtt_dotdot_level_current_level_publish("test/hest",
                                              2,
                                              UCL_MQTT_PUBLISH_TYPE_REPORTED);
  TEST_ASSERT_EQUAL(1, mqtt_test_helper_get_num_publish(topic_desired));
  TEST_ASSERT_EQUAL(1, mqtt_test_helper_get_num_publish(topic_reported));
  mqtt_test_helper_pop_publish(topic_desired, result);
  TEST_ASSERT_EQUAL_JSON(R"({"value":1})", result);
  mqtt_test_helper_pop_publish(topic_reported, result);
  TEST_ASSERT_EQUAL_JSON(R"({"value":2})", result);
}

// Test bitmaps desired reported attributes
void test_dotdot_mqtt_desired_reported_bool_and_enum_bitmaps()
{
  const char *topic_desired  = "test/hest/Thermostat/Attributes/"
                               "ThermostatProgrammingOperationMode/Desired";
  const char *topic_reported = "test/hest/Thermostat/Attributes/"
                               "ThermostatProgrammingOperationMode/Reported";
  char result[500];
  uic_mqtt_dotdot_thermostat_thermostat_programming_operation_mode_publish(
    "test/hest",
    THERMOSTAT_THERMOSTAT_PROGRAMMING_OPERATION_MODE_PROGRAMMING_MODE
      | THERMOSTAT_THERMOSTAT_PROGRAMMING_OPERATION_MODE_AUTO_OR_RECOVERY,
    UCL_MQTT_PUBLISH_TYPE_DESIRED);
  TEST_ASSERT_EQUAL(1, mqtt_test_helper_get_num_publish(topic_desired));
  TEST_ASSERT_EQUAL(0, mqtt_test_helper_get_num_publish(topic_reported));
  uic_mqtt_dotdot_thermostat_thermostat_programming_operation_mode_publish(
    "test/hest",
    THERMOSTAT_THERMOSTAT_PROGRAMMING_OPERATION_MODE_ECONOMY_OR_ENERGY_STAR
      | THERMOSTAT_THERMOSTAT_PROGRAMMING_OPERATION_MODE_AUTO_OR_RECOVERY,
    UCL_MQTT_PUBLISH_TYPE_REPORTED);
  TEST_ASSERT_EQUAL(1, mqtt_test_helper_get_num_publish(topic_desired));
  TEST_ASSERT_EQUAL(1, mqtt_test_helper_get_num_publish(topic_reported));
  mqtt_test_helper_pop_publish(topic_desired, result);
  TEST_ASSERT_EQUAL_JSON(
    R"({"value": {"ProgrammingMode":"Schedule","AutoOrRecovery":true,"EconomyOrEnergyStar":false}})",
    result);
  mqtt_test_helper_pop_publish(topic_reported, result);
  TEST_ASSERT_EQUAL_JSON(
    R"({"value": {"ProgrammingMode":"SimpleOrSetpoint","AutoOrRecovery":true,"EconomyOrEnergyStar":true}})",
    result);
}

// Write attributes test
uint8_t ac_error_code                         = 0xff;
uint8_t thermostat_programming_operation_mode = 0xff;
sl_status_t thermostat_bitmap_WriteAttribute_callback(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uic_mqtt_dotdot_thermostat_state_t state,
  uic_mqtt_dotdot_thermostat_updated_state_t updated)
{
  printf("Received callback thermostat");
  if (updated.ac_error_code == true) {
    printf("Updated ac error code");
    ac_error_code = state.ac_error_code;
  }
  if (updated.thermostat_programming_operation_mode == true) {
    printf("Updated programming_operation_mode");
    thermostat_programming_operation_mode
      = state.thermostat_programming_operation_mode;
  }

  return SL_STATUS_OK;
}

void test_dotdot_mqtt_write_attributes_bitmap()
{
  uic_mqtt_dotdot_set_thermostat_write_attributes_callback(
    thermostat_bitmap_WriteAttribute_callback);
  uic_mqtt_dotdot_init();
  const char topic[]
    = "ucl/by-unid/zwDEADBEEF/ep0/Thermostat/Commands/WriteAttributes";

  // Test as integer string
  uint32_t status
    = THERMOSTAT_AC_ERROR_CODE_COMPRESSOR_FAILURE_OR_REFRIGERANT_LEAKAGE
      | THERMOSTAT_AC_ERROR_CODE_ROOM_TEMPERATURE_SENSOR_FAILURE
      | THERMOSTAT_AC_ERROR_CODE_FAN_FAILURE
      | THERMOSTAT_AC_ERROR_CODE_INDOOR_COIL_TEMPERATURE_SENSOR_FAILURE
      | THERMOSTAT_AC_ERROR_CODE_OUTDOOR_TEMPERATURE_SENSOR_FAILURE;
  std::string payload = R"({"ACErrorCode": ")" + std::to_string(status) + "\"}";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL_INT_MESSAGE(static_cast<uint8_t>(status),
                                ac_error_code,
                                "Test bitmap as int string");

  ac_error_code = 0xff;
  // Test as json integer
  payload = R"({"ACErrorCode": )" + std::to_string(status) + "}";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL_INT_MESSAGE(static_cast<uint8_t>(status),
                                ac_error_code,
                                "Test bitmap as int");
  ac_error_code = 0xff;
  std::string status_as_json
    = R"({"CompressorFailureOrRefrigerantLeakage": true,
      "RoomTemperatureSensorFailure": "true",
      "OutdoorTemperatureSensorFailure": true,
      "IndoorCoilTemperatureSensorFailure": true,
      "FanFailure": "true"})";

  payload = R"({"ACErrorCode": )" + status_as_json + "}";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL_INT_MESSAGE(static_cast<uint8_t>(status),
                                ac_error_code,
                                "Test bitmap as json");

  uint32_t status_thermostat_programming_mode
    = THERMOSTAT_THERMOSTAT_PROGRAMMING_OPERATION_MODE_PROGRAMMING_MODE
      | THERMOSTAT_THERMOSTAT_PROGRAMMING_OPERATION_MODE_AUTO_OR_RECOVERY;

  status_as_json = R"({"ProgrammingMode": "Schedule",
      "AutoOrRecovery": "true",
      "EconomyOrEnergyStar": false})";

  payload = R"({"ThermostatProgrammingOperationMode": )" + status_as_json + "}";

  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL_INT_MESSAGE(
    static_cast<uint8_t>(status_thermostat_programming_mode),
    thermostat_programming_operation_mode,
    "Test bitmap as int");

  // Incorrect JSON
  thermostat_programming_operation_mode = 0xff;
  status_as_json                        = R"({"PmmingMode": "Schedule",
      "AtoOrRecovery": "true",
      "EconomyOrEnergySt": false})";
  payload = R"({"ThermostatProgrammingOperationMode": )" + status_as_json + "}";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL_INT_MESSAGE(0xff,
                                thermostat_programming_operation_mode,
                                "Test JSON with incorrect keys");
}

void test_dotdot_mqtt_bitmap_interpretation_publish_and_read()
{
  uic_mqtt_dotdot_set_thermostat_write_attributes_callback(
    thermostat_bitmap_WriteAttribute_callback);
  uic_mqtt_dotdot_init();
  const char topic[]
    = "ucl/by-unid/zwDEADBEEF/ep0/Thermostat/Commands/WriteAttributes";
  const char *topic_reported = "test/hest/Thermostat/Attributes/"
                               "ACErrorCode/Reported";

  // Test as integer string
  uint32_t status
    = THERMOSTAT_AC_ERROR_CODE_COMPRESSOR_FAILURE_OR_REFRIGERANT_LEAKAGE
      | THERMOSTAT_AC_ERROR_CODE_ROOM_TEMPERATURE_SENSOR_FAILURE
      | THERMOSTAT_AC_ERROR_CODE_OUTDOOR_TEMPERATURE_SENSOR_FAILURE
      | THERMOSTAT_AC_ERROR_CODE_INDOOR_COIL_TEMPERATURE_SENSOR_FAILURE
      | THERMOSTAT_AC_ERROR_CODE_FAN_FAILURE;

  std::string payload = R"({"ACErrorCode": ")" + std::to_string(status) + "\"}";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL_INT_MESSAGE(static_cast<uint8_t>(status),
                                ac_error_code,
                                "Test bitmap as int string");

  uic_mqtt_dotdot_thermostat_ac_error_code_publish(
    "test/hest",
    ac_error_code,
    UCL_MQTT_PUBLISH_TYPE_REPORTED);

  std::string status_as_json
    = R"({"value": {"CompressorFailureOrRefrigerantLeakage": true,
      "RoomTemperatureSensorFailure": true,
      "OutdoorTemperatureSensorFailure": true,
      "IndoorCoilTemperatureSensorFailure": true,
      "FanFailure": true}})";

  char result[500];
  mqtt_test_helper_pop_publish(topic_reported, result);
  TEST_ASSERT_EQUAL_JSON(status_as_json.c_str(), result);
}

uint8_t update_flags_bitmap_value = 0;
ColorLoopSetAction action_status;
CCColorLoopDirection direction_status;
uint16_t time_state                   = -1;
uint16_t start_hue_state              = -1;
uint8_t options_mask_bitmap_value     = 0;
uint8_t options_override_bitmap_value = 0;
static sl_status_t uic_mqtt_dotdot_color_control_color_loop_set_test_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t callback_type,
  uint8_t update_flags,
  ColorLoopSetAction action,
  CCColorLoopDirection direction,
  uint16_t time,
  uint16_t start_hue,
  uint8_t options_mask,
  uint8_t options_override)
{
  update_flags_bitmap_value     = update_flags;
  action_status                 = action;
  direction_status              = direction;
  time_state                    = time;
  start_hue_state               = start_hue;
  options_override_bitmap_value = options_override;
  options_mask_bitmap_value     = options_mask;
  return SL_STATUS_OK;
}

void test_dotdot_mqtt_bitmaps_commands()
{
  const char topic[]
    = "ucl/by-unid/zwDEADBEEF/ep0/ColorControl/Commands/ColorLoopSet";
  uic_mqtt_dotdot_color_control_color_loop_set_callback_set(
    &uic_mqtt_dotdot_color_control_color_loop_set_test_callback);
  uic_mqtt_dotdot_init();

  std::string payload = R"(
  {
    "UpdateFlags": {
        "UpdateAction": "true",
        "UpdateDirection": "false",
        "UpdateTime": true,
        "UpdateStartHue": false
    },
    "Action": "ActivateColorLoopFromColorLoopStartEnhancedHue",
    "Direction": "IncrementEnhancedCurrentHue",
    "Time": 1,
    "StartHue": 0,
    "OptionsMask": "0",
    "OptionsOverride": {
        "ExecuteIfOff": "true"
    }
  })";
  uint32_t update_flag_payload_status
    = COLOR_LOOP_SET_UPDATE_FLAGS_UPDATE_ACTION
      | COLOR_LOOP_SET_UPDATE_FLAGS_UPDATE_TIME;
  uint16_t time_payload_state       = 1;
  uint16_t start_hue_payload_state  = 0;
  uint32_t options_mask_payload     = 0;
  uint32_t options_override_payload = CC_COLOR_OPTIONS_EXECUTE_IF_OFF;

  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());

  TEST_ASSERT_EQUAL(update_flag_payload_status, update_flags_bitmap_value);
  TEST_ASSERT_EQUAL(
    ZCL_COLOR_LOOP_SET_ACTION_ACTIVATE_COLOR_LOOP_FROM_COLOR_LOOP_START_ENHANCED_HUE,
    action_status);
  TEST_ASSERT_EQUAL(ZCL_CC_COLOR_LOOP_DIRECTION_INCREMENT_ENHANCED_CURRENT_HUE,
                    direction_status);
  TEST_ASSERT_EQUAL(time_payload_state, time_state);
  TEST_ASSERT_EQUAL(start_hue_payload_state, start_hue_state);
  TEST_ASSERT_EQUAL(options_mask_payload, options_mask_bitmap_value);
  TEST_ASSERT_EQUAL(options_override_payload, options_override_bitmap_value);
}

void test_dotdot_mqtt_supported_commands()
{
  char result[1000];
  uic_mqtt_dotdot_init();
  // Test that supported commands isn't published, if no callbacks are registered
  uic_mqtt_dotdot_publish_supported_commands("zw-test-hest", 0);
  TEST_ASSERT_EQUAL(
    0,
    mqtt_test_helper_get_num_publish(
      "ucl/by-unid/zw-test-hest/ep0/DoorLock/SupportedCommands"));

  // Test that 1 supported command is published correct
  uic_mqtt_dotdot_door_lock_lock_door_callback_set(
    &uic_mqtt_dotdot_door_lock_lock_door_callback_func);
  uic_mqtt_dotdot_publish_supported_commands("zw-test-hest", 0);
  TEST_ASSERT_EQUAL(
    1,
    mqtt_test_helper_get_num_publish(
      "ucl/by-unid/zw-test-hest/ep0/DoorLock/SupportedCommands"));
  mqtt_test_helper_pop_publish(
    "ucl/by-unid/zw-test-hest/ep0/DoorLock/SupportedCommands",
    result);
  TEST_ASSERT_EQUAL_STRING(R"({"value": ["LockDoor"]})", result);

  // Test that multiple supported command is published correct
  uic_mqtt_dotdot_door_lock_toggle_response_callback_set(
    &uic_mqtt_dotdot_door_lock_toggle_response_callback_func);
  uic_mqtt_dotdot_publish_supported_commands("zw-test-hest", 0);
  TEST_ASSERT_EQUAL(
    2,
    mqtt_test_helper_get_num_publish(
      "ucl/by-unid/zw-test-hest/ep0/DoorLock/SupportedCommands"));
  mqtt_test_helper_pop_publish(
    "ucl/by-unid/zw-test-hest/ep0/DoorLock/SupportedCommands",
    result);
  TEST_ASSERT_EQUAL_STRING(R"({"value": ["LockDoor", "ToggleResponse"]})",
                           result);
}

void test_dotdot_mqtt_supported_commands_for_individual_clusters()
{
  char result[1000];
  uic_mqtt_dotdot_init();
  // Test that supported commands isn't published, if no callbacks are registered
  uic_mqtt_dotdot_publish_supported_commands("zw-test-unid", 0);
  TEST_ASSERT_EQUAL(
    0,
    mqtt_test_helper_get_num_publish(
      "ucl/by-unid/zw-test-unid/ep0/DoorLock/SupportedCommands"));

  // Set 1 command to ColorControl and one to Door Lock
  uic_mqtt_dotdot_color_control_color_loop_set_callback_set(
    &uic_mqtt_dotdot_color_control_color_loop_set_test_callback);
  uic_mqtt_dotdot_door_lock_lock_door_callback_set(
    &uic_mqtt_dotdot_door_lock_lock_door_callback_func);
  uic_mqtt_dotdot_door_lock_publish_supported_commands("zw-test-unid", 0);
  TEST_ASSERT_EQUAL(
    1,
    mqtt_test_helper_get_num_publish(
      "ucl/by-unid/zw-test-unid/ep0/DoorLock/SupportedCommands"));
  TEST_ASSERT_EQUAL(
    0,
    mqtt_test_helper_get_num_publish(
      "ucl/by-unid/zw-test-unid/ep0/ColorControl/SupportedCommands"));
  mqtt_test_helper_pop_publish(
    "ucl/by-unid/zw-test-unid/ep0/DoorLock/SupportedCommands",
    result);
  TEST_ASSERT_EQUAL_STRING(R"({"value": ["LockDoor"]})", result);

  uic_mqtt_dotdot_color_control_publish_supported_commands("zw-test-unid", 0);
  TEST_ASSERT_EQUAL(
    1,
    mqtt_test_helper_get_num_publish(
      "ucl/by-unid/zw-test-unid/ep0/DoorLock/SupportedCommands"));
  TEST_ASSERT_EQUAL(
    1,
    mqtt_test_helper_get_num_publish(
      "ucl/by-unid/zw-test-unid/ep0/ColorControl/SupportedCommands"));

  mqtt_test_helper_pop_publish(
    "ucl/by-unid/zw-test-unid/ep0/ColorControl/SupportedCommands",
    result);
  TEST_ASSERT_EQUAL_STRING(R"({"value": ["ColorLoopSet"]})", result);
}

void test_dotdot_mqtt_write_attributes_badtopic()
{
  uic_mqtt_dotdot_set_thermostat_write_attributes_callback(
    thermostat_WriteAttribute_callback);
  uic_mqtt_dotdot_init();

  thermostat_systemmode = 0;
  // Test as integer string
  std::string payload = R"({"SystemMode": ")"
                        + std::to_string(ZCL_THERMOSTAT_SYSTEM_MODE_FAN_ONLY)
                        + "\"}";
  mqtt_test_helper_publish(
    "ucl/by-unid/zwDEADBEEF/ep999/Thermostat/Commands/WriteAttributes",
    payload.c_str(),
    payload.length());
  mqtt_test_helper_publish(
    "ucl/by-unid/zwDEADBEEF/ep/Thermostat/Commands/WriteAttributes",
    payload.c_str(),
    payload.length());
  mqtt_test_helper_publish(
    "ucl/by-unid/zwDEADBEEF/e/Thermostat/Commands/WriteAttributes",
    payload.c_str(),
    payload.length());

  TEST_ASSERT_EQUAL_INT_MESSAGE(0,
                                thermostat_systemmode,
                                "Test enum as int string");
}

// Force read attributes test
uic_mqtt_dotdot_thermostat_updated_state_t thermostat_updated = {0};
sl_status_t thermostat_ForceReadAttributes_callback(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uic_mqtt_dotdot_thermostat_updated_state_t updated)
{
  thermostat_updated = updated;
  return SL_STATUS_OK;
}

// Test WriteAttributes accept enum as integer string
void test_dotdot_mqtt_force_read_attributes()
{
  uic_mqtt_dotdot_set_thermostat_force_read_attributes_callback(
    thermostat_ForceReadAttributes_callback);
  uic_mqtt_dotdot_init();
  const char topic[]
    = "ucl/by-unid/zwDEADBEEF/ep0/Thermostat/Commands/ForceReadAttributes";

  // Test empty json
  std::string payload = R"({"value": []})";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  // If an empty json is passed - all fields of "thermostat_updated" struct
  // should be set to "true". So if at least one of them is false - the test should fail
  bool test_value
    = thermostat_updated.local_temperature
      && thermostat_updated.outdoor_temperature && thermostat_updated.occupancy
      && thermostat_updated.abs_min_heat_setpoint_limit
      && thermostat_updated.abs_max_heat_setpoint_limit
      && thermostat_updated.abs_min_cool_setpoint_limit
      && thermostat_updated.abs_max_cool_setpoint_limit
      && thermostat_updated.pi_cooling_demand
      && thermostat_updated.pi_heating_demand
      && thermostat_updated.hvac_system_type_configuration
      && thermostat_updated.local_temperature_calibration
      && thermostat_updated.occupied_cooling_setpoint
      && thermostat_updated.occupied_heating_setpoint
      && thermostat_updated.unoccupied_cooling_setpoint
      && thermostat_updated.unoccupied_heating_setpoint
      && thermostat_updated.min_heat_setpoint_limit
      && thermostat_updated.max_heat_setpoint_limit
      && thermostat_updated.min_cool_setpoint_limit
      && thermostat_updated.max_cool_setpoint_limit
      && thermostat_updated.min_setpoint_dead_band
      && thermostat_updated.remote_sensing
      && thermostat_updated.control_sequence_of_operation
      && thermostat_updated.system_mode && thermostat_updated.alarm_mask
      && thermostat_updated.thermostat_running_mode
      && thermostat_updated.start_of_week
      && thermostat_updated.number_of_weekly_transitions
      && thermostat_updated.number_of_daily_transitions
      && thermostat_updated.temperature_setpoint_hold
      && thermostat_updated.temperature_setpoint_hold_duration
      && thermostat_updated.thermostat_programming_operation_mode
      && thermostat_updated.thermostat_running_state
      && thermostat_updated.setpoint_change_source
      && thermostat_updated.setpoint_change_amount
      && thermostat_updated.setpoint_change_source_timestamp
      && thermostat_updated.occupied_setback
      && thermostat_updated.occupied_setback_min
      && thermostat_updated.occupied_setback_max
      && thermostat_updated.unoccupied_setback
      && thermostat_updated.unoccupied_setback_min
      && thermostat_updated.unoccupied_setback_max
      && thermostat_updated.emergency_heat_delta && thermostat_updated.ac_type
      && thermostat_updated.ac_capacity
      && thermostat_updated.ac_refrigerant_type
      && thermostat_updated.ac_compressor_type
      && thermostat_updated.ac_error_code
      && thermostat_updated.ac_louver_position
      && thermostat_updated.ac_coil_temperature
      && thermostat_updated.ac_capacity_format;
  TEST_ASSERT_TRUE_MESSAGE(test_value, "Test empty json");

  // Test one value array json
  payload = R"({"value": ["AbsMinCoolSetpointLimit"]})";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_TRUE_MESSAGE(thermostat_updated.abs_min_cool_setpoint_limit,
                           "Test one value array json");

  // Test multiple values array json
  payload
    = R"({"value": ["AbsMinCoolSetpointLimit", "LocalTemperatureCalibration"]})";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_TRUE_MESSAGE(thermostat_updated.abs_min_cool_setpoint_limit,
                           "Test multiple values array json");
  TEST_ASSERT_TRUE_MESSAGE(thermostat_updated.local_temperature_calibration,
                           "Test multiple values array json");
}

void test_dotdot_mqtt_arbitrary_types_json()
{
  const uint8_t arr_size = 3;

  // test TransitionType struct serialisation
  TransitionType transition_arr[arr_size] = {
    {.TransitionTime = 1, .HeatSetPoint = 2, .CoolSetPoint = 3},
    {.TransitionTime = 4, .HeatSetPoint = 5, .CoolSetPoint = 6},
    {.TransitionTime = 7, .HeatSetPoint = 8, .CoolSetPoint = 9},
  };

  nlohmann::json transition_jsn_reported
    = add_transitions_to_json_obj(transition_arr, arr_size);

  const char *transition_jsn_desired = R"(
    [
      {
    	"CoolSetPoint": 3,
    	"HeatSetPoint": 2,
    	"TransitionTime": 1
      },
      {
        "CoolSetPoint": 6,
        "HeatSetPoint": 5,
        "TransitionTime": 4
      },
      {
        "CoolSetPoint": 9,
        "HeatSetPoint": 8,
        "TransitionTime": 7
      }
    ]
  )";

  TEST_ASSERT_EQUAL_JSON(transition_jsn_desired,
                         transition_jsn_reported.dump().c_str());

  // test SExtensionFieldSetList struct serialisation
  SExtensionFieldSetList ext_field_set_arr[arr_size]
    = {{.ClusterId = 1, .ExtensionFieldSet = "one"},
       {.ClusterId = 2, .ExtensionFieldSet = "two"},
       {.ClusterId = 3, .ExtensionFieldSet = "three"}};

  nlohmann::json ext_field_set_jsn_reported
    = add_extension_field_to_json_obj(ext_field_set_arr, arr_size);

  const char *ext_field_set_jsn_desired = R"(
    [
      {
        "ClusterId": 1,
        "ExtensionFieldSet": "one"
      },
      {
        "ClusterId": 2,
        "ExtensionFieldSet": "two"
      },
      {
        "ClusterId": 3,
        "ExtensionFieldSet": "three"
      }
    ]
  )";

  TEST_ASSERT_EQUAL_JSON(ext_field_set_jsn_desired,
                         ext_field_set_jsn_reported.dump().c_str());
}

void test_dotdot_mqtt_test_publish_integer_monotonous_string_array_attribute()
{
  const char *base_topic = "ucl/by-location/kitchen";
  uint8_t cluster_count  = 3;
  const char *value[cluster_count];
  value[0] = "OnOff";
  value[1] = "Level";
  value[2] = "NewClusterInTown";

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_binding_bindable_cluster_list_publish(
                      base_topic,
                      cluster_count,
                      value,
                      UCL_MQTT_PUBLISH_TYPE_DESIRED));

  const char *expected_topic
    = "ucl/by-location/kitchen/Binding/Attributes/BindableClusterList/Desired";

  char published_message[1000] = {};
  TEST_ASSERT_NOT_NULL(
    mqtt_test_helper_pop_publish(expected_topic, published_message));

  const char expected_published_message[]
    = R"({"value": ["OnOff","Level","NewClusterInTown"]})";

  TEST_ASSERT_EQUAL_JSON(expected_published_message, published_message);
}

void test_dotdot_mqtt_test_publish_integer_monotonous_struct_array_attribute()
{
  const char *base_topic                 = "ucl/by-location/desk";
  const uint8_t value_count              = 3;
  const BindingObject value[value_count] = {
    {.ClusterName = "OnOff", .DestinationUnid = "node_1", .DestinationEp = 4},
    {.ClusterName = "Level", .DestinationUnid = "node_1", .DestinationEp = 3},
    {.ClusterName = "OnOff", .DestinationUnid = "node_1", .DestinationEp = 3}};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_binding_binding_table_publish(
                      base_topic,
                      value_count,
                      value,
                      UCL_MQTT_PUBLISH_TYPE_REPORTED));

  const char *expected_topic
    = "ucl/by-location/desk/Binding/Attributes/BindingTable/Reported";

  char published_message[1000] = {};
  TEST_ASSERT_NOT_NULL(
    mqtt_test_helper_pop_publish(expected_topic, published_message));

  const char expected_published_message[] = R"({ "value":[
            { "ClusterName":"OnOff",
              "DestinationEp":4,
              "DestinationUnid":"node_1"},
            { "ClusterName":"Level",
              "DestinationEp":3,
              "DestinationUnid":"node_1"},
            { "ClusterName":"OnOff",
              "DestinationEp":3,
              "DestinationUnid":"node_1"}
            ]
          })";

  TEST_ASSERT_EQUAL_JSON(expected_published_message, published_message);
}

void test_dotdot_mqtt_test_publish_integer_monotonous_integer_array_attribute()
{
  const char *base_topic = "ucl/by-group/2";
  const uint8_t value[]  = {0, 23, 55, 3, 10};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    uic_mqtt_dotdot_state_endpoint_id_list_publish(
                      base_topic,
                      sizeof(value),
                      value,
                      UCL_MQTT_PUBLISH_TYPE_REPORTED));

  const char *expected_topic
    = "ucl/by-group/2/State/Attributes/EndpointIdList/Reported";

  char published_message[1000] = {};
  TEST_ASSERT_NOT_NULL(
    mqtt_test_helper_pop_publish(expected_topic, published_message));

  const char expected_published_message[] = R"({ "value":[ 0, 23, 55, 3, 10]})";

  TEST_ASSERT_EQUAL_JSON(expected_published_message, published_message);
}

void test_aox_locator_position_and_orientation_publish()
{
  const char *base_topic         = "ucl/by-unid/ble2222/ep0";
  CoordinateAndOrientation value = {.CoordinateX  = 2.3,
                                    .CoordinateY  = -34,
                                    .CoordinateZ  = -59.6,
                                    .OrientationX = 0.1,
                                    .OrientationY = 22,
                                    .OrientationZ = 1.111112};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_aox_locator_position_and_orientation_publish(
      base_topic,
      value,
      UCL_MQTT_PUBLISH_TYPE_REPORTED));

  // Verify that it did its job:
  const char *expected_topic = "ucl/by-unid/ble2222/ep0/AoXLocator/Attributes/"
                               "PositionAndOrientation/Reported";

  char published_message[1000] = {};
  TEST_ASSERT_NOT_NULL(
    mqtt_test_helper_pop_publish(expected_topic, published_message));

  const char expected_published_message[]
    = R"({"value":{"CoordinateX":2.3,"CoordinateY":-34.0,"CoordinateZ":-59.6,"OrientationX":0.1,"OrientationY":22.0,"OrientationZ":1.111112}})";

  TEST_ASSERT_EQUAL_JSON(expected_published_message, published_message);
}

void test_dotdot_mqtt_test_publish_monotonous_struct_array_attribute()
{
  const char *base_topic = "ucl/by-location/Kitchen";
  const ConfigurationParameter value[]
    = {{
         .ParameterId = 1,
         .Value       = 23,
         .Name        = "Parameter 1",
         .Info = "This parameter toggles the behavior of the main button",
         .MinimumValue         = 0,
         .MaximumValue         = 100,
         .DefaultValue         = 1,
         .DisplayFormat        = 0,
         .ReadOnly             = false,
         .Advanced             = false,
         .AlteringCapabilities = false,
       },
       {
         .ParameterId          = 2,
         .Value                = 24,
         .Name                 = "Parameter 2",
         .Info                 = "Play with this param at your own risks",
         .MinimumValue         = 0,
         .MaximumValue         = 1,
         .DefaultValue         = 40,
         .DisplayFormat        = 0,
         .ReadOnly             = false,
         .Advanced             = true,
         .AlteringCapabilities = true,
       }};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_configuration_parameters_configuration_parameters_publish(
      base_topic,
      2,
      value,
      UCL_MQTT_PUBLISH_TYPE_REPORTED));

  const char *expected_topic
    = "ucl/by-location/Kitchen/ConfigurationParameters/Attributes/"
      "ConfigurationParameters/Reported";

  char published_message[1000] = {};
  TEST_ASSERT_NOT_NULL(
    mqtt_test_helper_pop_publish(expected_topic, published_message));

  const char expected_published_message[] = R"({
  "value": [
    {
      "ParameterId": 1,
      "Value": 23,
      "Name": "Parameter 1",
      "Info": "This parameter toggles the behavior of the main button",
      "DisplayFormat": 0,
      "MinimumValue": 0,
      "MaximumValue": 100,
      "DefaultValue": 1,
      "ReadOnly": false,
      "Advanced": false,
      "AlteringCapabilities": false
    },
    {
      "ParameterId": 2,
      "Value": 24,
      "Name": "Parameter 2",
      "Info": "Play with this param at your own risks",
      "DisplayFormat": 0,
      "MinimumValue": 0,
      "DefaultValue": 40,
      "MaximumValue": 1,
      "ReadOnly": false,
      "Advanced": true,
      "AlteringCapabilities": true
    }
  ]
})";

  TEST_ASSERT_EQUAL_JSON(expected_published_message, published_message);
}

void test_dotdot_mqtt_test_publish_configuration_parameters_attribute()
{
  static const char info_str[]         = {'h',
                                          'e',
                                          'l',
                                          'l',
                                          'o',
                                          ' ',
                                          'w',
                                          'o',
                                          'r',
                                          'l',
                                          'd',
                                          static_cast<char>(0xa2),
                                          0};
  const char *base_topic               = "ucl/by-location/Kitchen";
  const ConfigurationParameter value[] = {{
    .ParameterId          = 1,
    .Value                = 23,
    .Name                 = "Parameter 1",
    .Info                 = info_str,
    .MinimumValue         = 0,
    .MaximumValue         = 100,
    .DefaultValue         = 1,
    .DisplayFormat        = 0,
    .ReadOnly             = false,
    .Advanced             = false,
    .AlteringCapabilities = false,
  }};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    uic_mqtt_dotdot_configuration_parameters_configuration_parameters_publish(
      base_topic,
      1,
      value,
      UCL_MQTT_PUBLISH_TYPE_REPORTED));

  const char *expected_topic
    = "ucl/by-location/Kitchen/ConfigurationParameters/Attributes/"
      "ConfigurationParameters/Reported";

  char published_message[1000] = {};
  TEST_ASSERT_NOT_NULL(
    mqtt_test_helper_pop_publish(expected_topic, published_message));

  const char expected_published_message[] = R"({
  "value": [
    {
      "ParameterId": 1,
      "Value": 23,
      "Name": "Parameter 1",
      "Info": "hello world�",
      "DisplayFormat": 0,
      "MinimumValue": 0,
      "MaximumValue": 100,
      "DefaultValue": 1,
      "ReadOnly": false,
      "Advanced": false,
      "AlteringCapabilities": false
    }
  ]
})";

  TEST_ASSERT_EQUAL_JSON(expected_published_message, published_message);
}

void test_dotdot_mqtt_test_publish_generated_aox_angle_report()
{
  const dotdot_unid_t unid      = "ble-1334";
  dotdot_endpoint_id_t endpoint = 0;
  uic_mqtt_dotdot_aox_locator_command_angle_report_fields_t fields;
  fields.tag_unid            = "ble-tag-23";
  fields.direction.Azimuth   = 34.2354563;
  fields.direction.Elevation = 3.00078;
  fields.direction.Distance  = 45.02;
  fields.deviation.Azimuth   = 0.001;
  fields.deviation.Elevation = 0.002;
  fields.deviation.Distance  = 0.00015;
  fields.sequence            = -10;

  // Ask DotDot MQTT to publish.
  uic_mqtt_dotdot_aox_locator_publish_generated_angle_report_command(unid,
                                                                     endpoint,
                                                                     &fields);

  // Verify that it did its job:
  const char *expected_topic
    = "ucl/by-unid/ble-1334/ep0/AoXLocator/GeneratedCommands/AngleReport";

  char published_message[1000] = {};
  TEST_ASSERT_NOT_NULL(
    mqtt_test_helper_pop_publish(expected_topic, published_message));

  const char expected_published_message[] = R"(
    {
      "Deviation": {
        "Azimuth": 0.001,
        "Distance": 0.00015,
        "Elevation": 0.002
      },
      "Direction": {
        "Azimuth": 34.2354563,
        "Distance": 45.02,
        "Elevation": 3.00078
      },
      "Sequence": -10,
      "TagUnid": "ble-tag-23"
    }
    )";

  TEST_ASSERT_EQUAL_JSON(expected_published_message, published_message);
}

// Test function
static sl_status_t
  uic_mqtt_dotdot_aox_locator_generated_angle_report_test_callback(
    dotdot_unid_t unid,
    dotdot_endpoint_id_t endpoint,
    uic_mqtt_dotdot_callback_call_type_t call_type,
    const char *tag_unid,
    SphericalCoordinates measurement,
    SphericalCoordinates deviation,
    int32_t sequence)
{
  TEST_ASSERT_EQUAL_STRING("ble1334", unid);
  TEST_ASSERT_EQUAL_UINT8(12, endpoint);
  TEST_ASSERT_EQUAL_UINT8(UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL, call_type);
  TEST_ASSERT_EQUAL_STRING("TestTagUnid", tag_unid);
  TEST_ASSERT_EQUAL_DOUBLE(12.12, measurement.Azimuth);
  TEST_ASSERT_EQUAL_DOUBLE(21.21, measurement.Elevation);
  TEST_ASSERT_EQUAL_DOUBLE(5.6, measurement.Distance);
  TEST_ASSERT_EQUAL_DOUBLE(12.34, deviation.Azimuth);
  TEST_ASSERT_EQUAL_DOUBLE(1.012, deviation.Elevation);
  TEST_ASSERT_EQUAL_DOUBLE(6.5, deviation.Distance);
  TEST_ASSERT_EQUAL_INT32(34, sequence);
  callback_count += 1;
  return SL_STATUS_OK;
}

void test_aox_locator_receive_generated_angle_report_command()
{
  // Configure a callback for the apply correction:
  uic_mqtt_dotdot_aox_locator_generated_angle_report_callback_set(
    &uic_mqtt_dotdot_aox_locator_generated_angle_report_test_callback);

  // Get Dotdot MQTT started
  uic_mqtt_dotdot_init();

  //Generate an incoming command published on MQTT:
  const char topic[]
    = "ucl/by-unid/ble1334/ep12/AoXLocator/GeneratedCommands/AngleReport";

  // Test AngleReport payload
  std::string payload = R"({
    "TagUnid" : "TestTagUnid",
    "Direction": {
      "Azimuth": 12.12,
      "Elevation": 21.21,
      "Distance": 5.6},
    "Deviation":{
      "Azimuth": 12.34,
      "Elevation": 1.012,
      "Distance": 6.5},
    "Sequence": 34})";

  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL(1, callback_count);
}

void test_dotdot_mqtt_test_publish_generated_aox_iq_report()
{
  const dotdot_unid_t unid      = "ble-1334";
  dotdot_endpoint_id_t endpoint = 0;
  int8_t sample_array[]         = {1, -2, 3, -4, 5};
  uic_mqtt_dotdot_aox_locator_command_iq_report_fields_t fields;
  fields.tag_unid      = "ble-tag-23";
  fields.channel       = 2;
  fields.rssi          = -90;
  fields.sequence      = -32;
  fields.samples_count = 5;
  fields.samples       = sample_array;

  // Ask DotDot MQTT to publish.
  uic_mqtt_dotdot_aox_locator_publish_generated_iq_report_command(unid,
                                                                  endpoint,
                                                                  &fields);

  // Verify that it did its job:
  const char *expected_topic
    = "ucl/by-unid/ble-1334/ep0/AoXLocator/GeneratedCommands/IQReport";

  char published_message[1000] = {};
  TEST_ASSERT_NOT_NULL(
    mqtt_test_helper_pop_publish(expected_topic, published_message));

  const char expected_published_message[] = R"(
    {"Channel":2,
    "RSSI":-90,
    "Samples":[1,-2,3,-4,5],
    "Sequence":-32,
    "TagUnid":"ble-tag-23"}
    )";

  TEST_ASSERT_EQUAL_JSON(expected_published_message, published_message);
}

// Test function
static sl_status_t uic_mqtt_dotdot_aox_locator_angle_correction_test_callback(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  const char *tag_unid,
  SphericalCoordinates direction,
  SphericalCoordinates deviation,
  int32_t sequence)
{
  TEST_ASSERT_EQUAL_STRING("ble1334", unid);
  TEST_ASSERT_EQUAL_UINT8(4, endpoint);
  TEST_ASSERT_EQUAL_UINT8(UIC_MQTT_DOTDOT_CALLBACK_TYPE_NORMAL, call_type);
  TEST_ASSERT_EQUAL_STRING("TestTagUnid", tag_unid);
  TEST_ASSERT_EQUAL_FLOAT(12.12, direction.Azimuth);
  TEST_ASSERT_EQUAL_FLOAT(21.21, direction.Elevation);
  TEST_ASSERT_EQUAL_FLOAT(5.6, direction.Distance);
  TEST_ASSERT_EQUAL_FLOAT(12.34, deviation.Azimuth);
  TEST_ASSERT_EQUAL_FLOAT(1.012, deviation.Elevation);
  TEST_ASSERT_EQUAL_FLOAT(6.5, deviation.Distance);
  TEST_ASSERT_EQUAL_UINT8(34, sequence);
  callback_count += 1;
  return SL_STATUS_OK;
}

void test_aox_apply_correction_callback()
{
  // Configure a callback for the apply correction:
  uic_mqtt_dotdot_aox_locator_angle_correction_callback_set(
    &uic_mqtt_dotdot_aox_locator_angle_correction_test_callback);

  // Get Dotdot MQTT started
  uic_mqtt_dotdot_init();

  //Generate an incoming command published on MQTT:
  const char topic[]
    = "ucl/by-unid/ble1334/ep4/AoXLocator/Commands/AngleCorrection";

  // Test ApplyCorrection payload
  std::string payload = R"({
    "TagUnid" : "TestTagUnid",
    "Direction": {
      "Azimuth": 12.12,
      "Elevation": 21.21,
      "Distance": 5.6},
    "Deviation":{
      "Azimuth": 12.34,
      "Elevation": 1.012,
      "Distance": 6.5} ,"Sequence": 34})";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL(1, callback_count);
}

static sl_status_t uic_mqtt_dotdot_aox_locator_write_attributes_test_callback(
  const dotdot_unid_t unid,
  const dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uic_mqtt_dotdot_aox_locator_state_t new_state,
  uic_mqtt_dotdot_aox_locator_updated_state_t updated_state)
{
  TEST_ASSERT_TRUE(updated_state.azimuth_mask);
  TEST_ASSERT_EQUAL(2, new_state.azimuth_mask_count);
  TEST_ASSERT_EQUAL_DOUBLE(12.12, new_state.azimuth_mask[0].Min);
  TEST_ASSERT_EQUAL_DOUBLE(21.21, new_state.azimuth_mask[0].Max);
  TEST_ASSERT_EQUAL(5, new_state.azimuth_mask[1].Min);
  TEST_ASSERT_EQUAL(6, new_state.azimuth_mask[1].Max);
  TEST_ASSERT_TRUE(updated_state.cte_mode);
  TEST_ASSERT_EQUAL(0, new_state.cte_mode);
  TEST_ASSERT_TRUE(updated_state.position_and_orientation);
  TEST_ASSERT_EQUAL_DOUBLE(-4.5,
                           new_state.position_and_orientation.CoordinateX);
  TEST_ASSERT_EQUAL_DOUBLE(98, new_state.position_and_orientation.CoordinateY);
  TEST_ASSERT_EQUAL_DOUBLE(9, new_state.position_and_orientation.CoordinateZ);
  TEST_ASSERT_EQUAL_DOUBLE(1.1,
                           new_state.position_and_orientation.OrientationX);
  TEST_ASSERT_EQUAL_DOUBLE(2.2,
                           new_state.position_and_orientation.OrientationY);
  TEST_ASSERT_EQUAL_DOUBLE(3.3,
                           new_state.position_and_orientation.OrientationZ);
  TEST_ASSERT_TRUE(updated_state.antenna_array);
  TEST_ASSERT_EQUAL(4, new_state.antenna_array_count);
  TEST_ASSERT_EQUAL(1, new_state.antenna_array[0]);
  TEST_ASSERT_EQUAL(2, new_state.antenna_array[1]);
  TEST_ASSERT_EQUAL(3, new_state.antenna_array[2]);
  TEST_ASSERT_EQUAL(-15, new_state.antenna_array[3]);
  TEST_ASSERT_TRUE(updated_state.angle_filtering);
  TEST_ASSERT_TRUE(new_state.angle_filtering);
  TEST_ASSERT_TRUE(updated_state.antenna_mode);
  TEST_ASSERT_EQUAL(3, new_state.antenna_mode);  //Array 4x4 DP URA
  TEST_ASSERT_TRUE(updated_state.reporting_mode);
  TEST_ASSERT_EQUAL(1, new_state.reporting_mode);  // Angle Reports
  callback_count += 1;
  return SL_STATUS_OK;
}

void test_aox_locator_write_attributes_callback()
{
  // Configure a callback for the apply correction:
  uic_mqtt_dotdot_set_aox_locator_write_attributes_callback(
    &uic_mqtt_dotdot_aox_locator_write_attributes_test_callback);

  // Get Dotdot MQTT started
  uic_mqtt_dotdot_init();

  //Generate an incoming command published on MQTT:
  const char topic[]
    = "ucl/by-unid/ble1334/ep4/AoXLocator/Commands/WriteAttributes";

  // Test WriteAttribute payload
  std::string payload = R"({
  "AzimuthMask": [{
    "Min": 12.12,
    "Max": 21.21
  },
  {
    "Min": 5,
    "Max": 6
  }],
  "CTEMode": "Silicon Labs",
  "PositionAndOrientation": {
    "CoordinateX": -4.5,
    "CoordinateY": 98,
    "CoordinateZ": 9,
    "OrientationX": 1.1,
    "OrientationY": 2.2,
    "OrientationZ": 3.3
  },
  "AntennaArray": [
    1,
    2,
    3,
    -15
  ],
  "AngleFiltering": true,
  "AntennaMode": "Array 4x4 DP URA",
  "ReportingMode": "AngleReport"
})";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL(1, callback_count);
}

uic_mqtt_dotdot_thermostat_command_set_weekly_schedule_fields_t
  stub_set_sched_cmd
  = {0};
bool stub_set_sched_called = false;
static sl_status_t stub_struct_array_set_weekly_schedule_cb(
  dotdot_unid_t unid,
  dotdot_endpoint_id_t endpoint,
  uic_mqtt_dotdot_callback_call_type_t call_type,
  uint8_t number_of_transitions,
  uint8_t day_of_week,
  uint8_t mode,
  uint8_t transitions_count,
  const TransitionType *transitions)
{
  TEST_ASSERT_EQUAL_STRING("zwDEADBEEF", unid);
  TEST_ASSERT_EQUAL(5, endpoint);

  TEST_ASSERT_EQUAL_HEX8(stub_set_sched_cmd.number_of_transitions,
                         number_of_transitions);
  TEST_ASSERT_EQUAL_HEX8(stub_set_sched_cmd.day_of_week, day_of_week);
  TEST_ASSERT_EQUAL_HEX8(stub_set_sched_cmd.mode, mode);
  TEST_ASSERT_EQUAL_HEX8(stub_set_sched_cmd.transitions_count,
                         transitions_count);
  for (uint8_t i = 0; i < transitions_count; i++) {
    TEST_ASSERT_EQUAL_UINT16(stub_set_sched_cmd.transitions[i].TransitionTime,
                             transitions[i].TransitionTime);
    TEST_ASSERT_EQUAL_INT16(stub_set_sched_cmd.transitions[i].HeatSetPoint,
                            transitions[i].HeatSetPoint);
    TEST_ASSERT_EQUAL_INT16(stub_set_sched_cmd.transitions[i].CoolSetPoint,
                            transitions[i].CoolSetPoint);
  }

  stub_set_sched_called = true;

  return SL_STATUS_OK;
}

// Test command with struct array argument
void test_command_argument_struct_array_callback(void)
{
  uic_mqtt_dotdot_thermostat_set_weekly_schedule_callback_set(
    &stub_struct_array_set_weekly_schedule_cb);

  // Get Dotdot MQTT started
  uic_mqtt_dotdot_init();

  TransitionType mock_transitions[2] = {
    {10000, 1100, 3300},
    {5000, 1200, 2700},
  };
  stub_set_sched_cmd.number_of_transitions = 2;
  stub_set_sched_cmd.day_of_week           = 0x41;
  stub_set_sched_cmd.mode                  = 0x3;
  stub_set_sched_cmd.transitions_count     = 2;
  stub_set_sched_cmd.transitions           = mock_transitions;
  stub_set_sched_called                    = false;

  std::string payload = R"({
  "NumberOfTransitions": 2,
  "DayOfWeek": {
    "Sunday": true,
    "Monday": false,
    "Tuesday": false,
    "Wednesday": false,
    "Thursday": false,
    "Friday": false,
    "Saturday": true,
    "AwayOrVacation": false
  },
  "Mode": { "Heat": true, "Cool": true },
  "Transitions": [
    { "TransitionTime": 10000, "HeatSetPoint": 1100, "CoolSetPoint": 3300 },
    { "TransitionTime": 5000, "HeatSetPoint": 1200, "CoolSetPoint": 2700 }
  ]
})";
  mqtt_test_helper_publish(
    "ucl/by-unid/zwDEADBEEF/ep5/Thermostat/Commands/SetWeeklySchedule",
    payload.c_str(),
    payload.length());

  TEST_ASSERT_TRUE(stub_set_sched_called);
}

uic_mqtt_dotdot_door_lock_state_t dl_writeattr_state = {0};
bool stub_dl_writeattr_called                        = false;
static sl_status_t
  stub_dl_writeattr_cb(const dotdot_unid_t unid,
                       const dotdot_endpoint_id_t endpoint,
                       uic_mqtt_dotdot_callback_call_type_t call_type,
                       uic_mqtt_dotdot_door_lock_state_t state,
                       uic_mqtt_dotdot_door_lock_updated_state_t updated)
{
  TEST_ASSERT_TRUE(updated.language);
  TEST_ASSERT_EQUAL_STRING(dl_writeattr_state.language, state.language);

  TEST_ASSERT_TRUE(updated.operating_mode);
  TEST_ASSERT_EQUAL_UINT8(dl_writeattr_state.operating_mode,
                          state.operating_mode);

  TEST_ASSERT_TRUE(updated.sound_volume);
  TEST_ASSERT_EQUAL_UINT8(dl_writeattr_state.sound_volume, state.sound_volume);

  TEST_ASSERT_TRUE(updated.alarm_mask);
  TEST_ASSERT_EQUAL_UINT16(dl_writeattr_state.alarm_mask, state.alarm_mask);

  stub_dl_writeattr_called = true;

  return SL_STATUS_OK;
}

// Test WriteAttributes command with string, enum, and bitmap types
void test_dotdot_mqtt_write_attributes_string_bitmap_enum(void)
{
  uic_mqtt_dotdot_set_door_lock_write_attributes_callback(stub_dl_writeattr_cb);
  uic_mqtt_dotdot_init();

  dl_writeattr_state.language       = "Universal";
  dl_writeattr_state.operating_mode = ZCL_DRLK_OPER_MODE_VACATION;
  dl_writeattr_state.sound_volume   = 2;
  dl_writeattr_state.alarm_mask     = 0x0011;
  stub_dl_writeattr_called          = false;

  std::string payload = R"({
    "Language": "Universal",
    "SoundVolume": 2,
    "OperatingMode": 1,
    "AlarmMask": {
      "DeadboltJammed": true,
      "LockResetToFactoryDefaults": false,
      "RFPowerModuleCycled": false,
      "TamperAlarmWrongCodeEntryLimit": true,
      "TamperAlarmFrontEscutcheonRemovedFromMain": false,
      "ForcedDoorOpenUnderDoorLockedCondition": false
    }
  })";
  mqtt_test_helper_publish(
    "ucl/by-unid/zwDEADBEEF/ep0/DoorLock/Commands/WriteAttributes",
    payload.c_str(),
    payload.length());

  TEST_ASSERT_TRUE(stub_dl_writeattr_called);
}

// Test Command callback that contains NULLs instead of strings
sl_status_t bind_callback(dotdot_unid_t unid,
                          dotdot_endpoint_id_t endpoint,
                          uic_mqtt_dotdot_callback_call_type_t call_type,
                          const char *cluster_name,
                          const char *destination_unid,
                          uint8_t destination_ep)
{
  callback_count += 1;
  return SL_STATUS_OK;
}

void test_dotdot_mqtt_incoming_command_with_null_json_object()
{
  uic_mqtt_dotdot_binding_bind_callback_set(&bind_callback);
  uic_mqtt_dotdot_init();
  const char topic[] = "ucl/by-unid/test_unid/ep0/Binding/Commands/Bind";

  // test DestinationUnid is a Null object.
  std::string payload
    = R"({"ClusterName":"OnOff","DestinationUnid":null,"DestinationEp": 0})";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());

  TEST_ASSERT_EQUAL(1, callback_count);

  payload
    = R"({"ClusterName":null,"DestinationUnid":"1234","DestinationEp": 0})";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());

  TEST_ASSERT_EQUAL(2, callback_count);

  payload
    = R"({"ClusterName":"OnOff","DestinationUnid":"1234","DestinationEp": 0})";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());

  TEST_ASSERT_EQUAL(3, callback_count);
}

// Test command with multiple callbacks attached
sl_status_t toggle_callback_1(dotdot_unid_t unid,
                              dotdot_endpoint_id_t endpoint,
                              uic_mqtt_dotdot_callback_call_type_t call_type)
{
  callback_count += 1;
  return SL_STATUS_OK;
}
sl_status_t toggle_callback_2(dotdot_unid_t unid,
                              dotdot_endpoint_id_t endpoint,
                              uic_mqtt_dotdot_callback_call_type_t call_type)
{
  callback_count += 1;
  return SL_STATUS_OK;
}

void test_dotdot_mqtt_incoming_command_with_multiple_callbacks()
{
  uic_mqtt_dotdot_on_off_toggle_callback_set(&toggle_callback_1);
  uic_mqtt_dotdot_on_off_toggle_callback_set(&toggle_callback_2);
  uic_mqtt_dotdot_init();
  const char topic[] = "ucl/by-unid/test_unid/ep92/OnOff/Commands/Toggle";

  // test DestinationUnid is a Null object.
  std::string payload = R"({})";
  mqtt_test_helper_publish(topic, payload.c_str(), payload.length());

  TEST_ASSERT_EQUAL(2, callback_count);
}

void test_generic_get_enum_value_name()
{
  std::string result = get_enum_value_name(0, 0, 0);
  TEST_ASSERT_EQUAL_STRING("", result.c_str());

  // Door Lock, "DoorState"
  result = get_enum_value_name(0x0101, 3, 0);
  TEST_ASSERT_EQUAL_STRING("Open", result.c_str());
  result = get_enum_value_name(0x0101, 3, 1);
  TEST_ASSERT_EQUAL_STRING("Closed", result.c_str());
  result = get_enum_value_name(0x0101, 3, 0xFF);
  TEST_ASSERT_EQUAL_STRING("Undefined", result.c_str());
  result = get_enum_value_name(0x0101, 3, 0xFF01);
  TEST_ASSERT_EQUAL_STRING("65281", result.c_str());

  // Color mode:
  result = get_enum_value_name(0x300, 8, 0);
  TEST_ASSERT_EQUAL_STRING("CurrentHueAndCurrentSaturation", result.c_str());
  result = get_enum_value_name(0x300, 8, 1);
  TEST_ASSERT_EQUAL_STRING("CurrentXAndCurrentY", result.c_str());
  result = get_enum_value_name(0x300, 8, 2);
  TEST_ASSERT_EQUAL_STRING("ColorTemperatureMireds", result.c_str());

  // ColorCapbilities 400A
  result = get_enum_value_name(0x300, 0x4002, 0);
  TEST_ASSERT_EQUAL_STRING("ColorLoopInactive", result.c_str());
  result = get_enum_value_name(0x300, 0x4002, 1);
  TEST_ASSERT_EQUAL_STRING("ColorLoopActive", result.c_str());
  result = get_enum_value_name(0x300, 0x4002, 2);
  TEST_ASSERT_EQUAL_STRING("2", result.c_str());
}

void test_generic_get_enum_name_value()
{
  std::string test_name = std::string("Open");
  uint32_t result       = 0xFFFF;

  // Door Lock, "DoorState"
  result = get_enum_name_value(0x0101, 3, test_name);
  TEST_ASSERT_EQUAL(0, result);
  test_name = std::string("Closed");
  result    = get_enum_name_value(0x0101, 3, test_name);
  TEST_ASSERT_EQUAL(1, result);
  test_name = std::string("Undefined");
  result    = get_enum_name_value(0x0101, 3, test_name);
  TEST_ASSERT_EQUAL(0xFF, result);
  test_name = std::string("65281");
  result    = get_enum_name_value(0x0101, 3, test_name);
  TEST_ASSERT_EQUAL(UINT32_MAX, result);

  // Color mode:
  test_name = std::string("CurrentHueAndCurrentSaturation");
  result    = get_enum_name_value(0x300, 8, test_name);
  TEST_ASSERT_EQUAL(0, result);
  test_name = std::string("CurrentXAndCurrentY");
  result    = get_enum_name_value(0x300, 8, test_name);
  TEST_ASSERT_EQUAL(1, result);
  test_name = std::string("ColorTemperatureMireds");
  result    = get_enum_name_value(0x300, 8, test_name);
  TEST_ASSERT_EQUAL(2, result);

  // ColorCapbilities 400A
  test_name = std::string("ColorLoopInactive");
  result    = get_enum_name_value(0x300, 0x4002, test_name);
  TEST_ASSERT_EQUAL(0, result);
  test_name = std::string("ColorLoopActive");
  result    = get_enum_name_value(0x300, 0x4002, test_name);
  TEST_ASSERT_EQUAL(1, result);
  test_name = std::string("2");
  result    = get_enum_name_value(0x300, 0x4002, test_name);
  TEST_ASSERT_EQUAL(UINT32_MAX, result);
}
}
