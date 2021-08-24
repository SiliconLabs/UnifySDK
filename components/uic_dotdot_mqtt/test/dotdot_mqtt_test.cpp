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

#include "mqtt_mock_helper.h"

extern "C" {
#include "dotdot_mqtt.h"

#include "dotdot_mqtt_test.include"

/// Called before each and every test
void setUp()
{
  mqtt_mock_helper_init();
  unset_all_callbacks();
  reset_callback_counters();
  num_command_callbacks = 0;
}

void test_dotdot_mqtt_happy_no_registered_handlers()
{
  uic_mqtt_dotdot_init();
  TEST_ASSERT_EQUAL(0, mqtt_mock_helper_get_total_num_subscribers());
}

void test_dotdot_mqtt_commands_invalid_json()
{
  unsigned int num_callbacks = set_all_callbacks();
  uic_mqtt_dotdot_init();
  // NOTE: The by-group subscriptions are registered (x2 registrations) only if
  // the group dispatch callback is set. The tests currently do not do this.
  TEST_ASSERT_EQUAL(num_callbacks,
                    mqtt_mock_helper_get_total_num_subscribers());

  // Test invalid json
  std::string json_str = "{\"foo: \"bar\"}";
  mqtt_mock_helper_publish_to_all_topics(json_str.c_str(), json_str.length());
  TEST_ASSERT_EQUAL(0, num_command_callbacks);

  // Test empty json
  json_str = "";
  mqtt_mock_helper_publish_to_all_topics(json_str.c_str(), json_str.length());
  TEST_ASSERT_EQUAL(0, num_command_callbacks);

  // Test valid but incomplete json
  json_str = R"({"foo":"bar")";
  mqtt_mock_helper_publish_to_all_topics(json_str.c_str(), json_str.length());
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
  mqtt_mock_helper_publish(topic, msg.c_str(), msg.length());
  msg = R"({"Wazzap":"1234"})";
  mqtt_mock_helper_publish(topic, msg.c_str(), msg.length());
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
  mqtt_mock_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL_INT_MESSAGE(
    static_cast<uint8_t>(ZCL_THERMOSTAT_SYSTEM_MODE_FAN_ONLY),
    thermostat_systemmode,
    "Test enum as int string");

  // Test as json integer
  payload = R"({"SystemMode": )"
            + std::to_string(ZCL_THERMOSTAT_SYSTEM_MODE_COOL) + "}";
  mqtt_mock_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL_INT_MESSAGE(
    static_cast<uint8_t>(ZCL_THERMOSTAT_SYSTEM_MODE_COOL),
    thermostat_systemmode,
    "Test enum as json int");

  // Test as string name
  payload = R"({"SystemMode": "EmergencyHeating"})";
  mqtt_mock_helper_publish(topic, payload.c_str(), payload.length());
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
  mqtt_mock_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_FALSE_MESSAGE(doorlock_enablelogging, "Test bool as json bool");

  payload = R"({"EnableLogging": "true"})";
  mqtt_mock_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_TRUE_MESSAGE(doorlock_enablelogging, "Test bool as sting");

  payload = R"({"EnableLogging": 0})";
  mqtt_mock_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_FALSE_MESSAGE(doorlock_enablelogging, "Test bool as integer");
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
  TEST_ASSERT_EQUAL(1, mqtt_mock_helper_get_num_publish(topic_reported));
  TEST_ASSERT_EQUAL(1, mqtt_mock_helper_get_num_publish(topic_desired));
  char result[100];
  mqtt_mock_helper_pop_publish(topic_reported, result);
  TEST_ASSERT_EQUAL_JSON(R"({"value":"42"})", result);
  mqtt_mock_helper_pop_publish(topic_desired, result);
  TEST_ASSERT_EQUAL_JSON(R"({"value":"42"})", result);
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
  TEST_ASSERT_EQUAL(1, mqtt_mock_helper_get_num_publish(topic_reported));
  TEST_ASSERT_EQUAL(1, mqtt_mock_helper_get_num_publish(topic_desired));
  char result[100];
  mqtt_mock_helper_pop_publish(topic_reported, result);
  TEST_ASSERT_EQUAL_JSON(R"({"value":"ApplyUpgradeAfterTimeout"})", result);
  mqtt_mock_helper_pop_publish(topic_desired, result);
  TEST_ASSERT_EQUAL_JSON(R"({"value":"ApplyUpgradeAfterTimeout"})", result);
}

uint16_t user_id_value     = 0;
uint8_t user_status_value  = 0;
uint8_t user_type_value    = 0;
std::string pin_code_value = "";
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

  mqtt_mock_helper_publish(topic, payload.c_str(), payload.length());

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

  mqtt_mock_helper_publish(topic, payload.c_str(), payload.length());

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
  TEST_ASSERT_EQUAL(1, mqtt_mock_helper_get_num_publish(topic_reported));
  TEST_ASSERT_EQUAL(1, mqtt_mock_helper_get_num_publish(topic_desired));
  char result[100];
  mqtt_mock_helper_pop_publish(topic_reported, result);
  TEST_ASSERT_EQUAL_JSON(R"({"value":"Danglish"})", result);
  mqtt_mock_helper_pop_publish(topic_desired, result);
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
  TEST_ASSERT_EQUAL(1, mqtt_mock_helper_get_num_publish(topic_desired));
  TEST_ASSERT_EQUAL(0, mqtt_mock_helper_get_num_publish(topic_reported));
  char result[100];
  uic_mqtt_dotdot_level_current_level_publish("test/hest",
                                              2,
                                              UCL_MQTT_PUBLISH_TYPE_REPORTED);
  TEST_ASSERT_EQUAL(1, mqtt_mock_helper_get_num_publish(topic_desired));
  TEST_ASSERT_EQUAL(1, mqtt_mock_helper_get_num_publish(topic_reported));
  mqtt_mock_helper_pop_publish(topic_desired, result);
  TEST_ASSERT_EQUAL_JSON(R"({"value":"1"})", result);
  mqtt_mock_helper_pop_publish(topic_reported, result);
  TEST_ASSERT_EQUAL_JSON(R"({"value":"2"})", result);
}

// Test bitmaps desired reported attributes
void test_dotdot_mqtt_desired_reported_bool_and_enum_bitmaps()
{
  const char *topic_desired = "test/hest/Thermostat/Attributes/"
                              "ThermostatProgrammingOperationMode/Desired";
  const char *topic_reported = "test/hest/Thermostat/Attributes/"
                               "ThermostatProgrammingOperationMode/Reported";
  char result[500];
  uic_mqtt_dotdot_thermostat_thermostat_programming_operation_mode_publish(
    "test/hest",
    THERMOSTAT_THERMOSTAT_PROGRAMMING_OPERATION_MODE_PROGRAMMING_MODE
      | THERMOSTAT_THERMOSTAT_PROGRAMMING_OPERATION_MODE_AUTO_OR_RECOVERY,
    UCL_MQTT_PUBLISH_TYPE_DESIRED);
  TEST_ASSERT_EQUAL(1, mqtt_mock_helper_get_num_publish(topic_desired));
  TEST_ASSERT_EQUAL(0, mqtt_mock_helper_get_num_publish(topic_reported));
  uic_mqtt_dotdot_thermostat_thermostat_programming_operation_mode_publish(
    "test/hest",
    THERMOSTAT_THERMOSTAT_PROGRAMMING_OPERATION_MODE_ECONOMY_OR_ENERGY_STAR
      | THERMOSTAT_THERMOSTAT_PROGRAMMING_OPERATION_MODE_AUTO_OR_RECOVERY,
    UCL_MQTT_PUBLISH_TYPE_REPORTED);
  TEST_ASSERT_EQUAL(1, mqtt_mock_helper_get_num_publish(topic_desired));
  TEST_ASSERT_EQUAL(1, mqtt_mock_helper_get_num_publish(topic_reported));
  mqtt_mock_helper_pop_publish(topic_desired, result);
  TEST_ASSERT_EQUAL_JSON(
    R"({"value": {"ProgrammingMode":"Schedule","AutoOrRecovery":true,"EconomyOrEnergyStar":false}})",
    result);
  mqtt_mock_helper_pop_publish(topic_reported, result);
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
  mqtt_mock_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL_INT_MESSAGE(static_cast<uint8_t>(status),
                                ac_error_code,
                                "Test bitmap as int string");

  ac_error_code = 0xff;
  // Test as json integer
  payload = R"({"ACErrorCode": )" + std::to_string(status) + "}";
  mqtt_mock_helper_publish(topic, payload.c_str(), payload.length());
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
  mqtt_mock_helper_publish(topic, payload.c_str(), payload.length());
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

  mqtt_mock_helper_publish(topic, payload.c_str(), payload.length());
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
  mqtt_mock_helper_publish(topic, payload.c_str(), payload.length());
  TEST_ASSERT_EQUAL_INT_MESSAGE(0xff,
                                thermostat_programming_operation_mode,
                                "Test bitmap as int");
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
  mqtt_mock_helper_publish(topic, payload.c_str(), payload.length());
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
  mqtt_mock_helper_pop_publish(topic_reported, result);
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

  mqtt_mock_helper_publish(topic, payload.c_str(), payload.length());

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
    mqtt_mock_helper_get_num_publish(
      "ucl/by-unid/zw-test-hest/ep0/DoorLock/SupportedCommands"));

  // Test that 1 supported command is published correct
  uic_mqtt_dotdot_door_lock_lock_door_callback_set(
    &uic_mqtt_dotdot_door_lock_lock_door_callback_func);
  uic_mqtt_dotdot_publish_supported_commands("zw-test-hest", 0);
  TEST_ASSERT_EQUAL(
    1,
    mqtt_mock_helper_get_num_publish(
      "ucl/by-unid/zw-test-hest/ep0/DoorLock/SupportedCommands"));
  mqtt_mock_helper_pop_publish(
    "ucl/by-unid/zw-test-hest/ep0/DoorLock/SupportedCommands",
    result);
  TEST_ASSERT_EQUAL_STRING(R"({"value": ["LockDoor"]})", result);

  // Test that multiple supported command is published correct
  uic_mqtt_dotdot_door_lock_toggle_response_callback_set(
    &uic_mqtt_dotdot_door_lock_toggle_response_callback_func);
  uic_mqtt_dotdot_publish_supported_commands("zw-test-hest", 0);
  TEST_ASSERT_EQUAL(
    2,
    mqtt_mock_helper_get_num_publish(
      "ucl/by-unid/zw-test-hest/ep0/DoorLock/SupportedCommands"));
  mqtt_mock_helper_pop_publish(
    "ucl/by-unid/zw-test-hest/ep0/DoorLock/SupportedCommands",
    result);
  TEST_ASSERT_EQUAL_STRING(R"({"value": ["LockDoor","ToggleResponse"]})", result);
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
  mqtt_mock_helper_publish(
    "ucl/by-unid/zwDEADBEEF/ep999/Thermostat/Commands/WriteAttributes",
    payload.c_str(),
    payload.length());
  mqtt_mock_helper_publish(
    "ucl/by-unid/zwDEADBEEF/ep/Thermostat/Commands/WriteAttributes",
    payload.c_str(),
    payload.length());
  mqtt_mock_helper_publish(
    "ucl/by-unid/zwDEADBEEF/e/Thermostat/Commands/WriteAttributes",
    payload.c_str(),
    payload.length());

  TEST_ASSERT_EQUAL_INT_MESSAGE(0,
                                thermostat_systemmode,
                                "Test enum as int string");
}
}
