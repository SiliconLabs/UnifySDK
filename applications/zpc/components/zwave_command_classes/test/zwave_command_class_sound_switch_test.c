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
#include "zwave_command_class_sound_switch.h"
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

// Private variables
static zwave_command_handler_t handler = {};

static attribute_resolver_function_t configuration_get  = NULL;
static attribute_resolver_function_t configuration_set  = NULL;
static attribute_resolver_function_t tone_number_get  = NULL;
static attribute_resolver_function_t tone_info_get  = NULL;
static attribute_resolver_function_t play_get  = NULL;
static attribute_resolver_function_t play_set  = NULL;
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
  if (node_type == ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_VOLUME) {
    TEST_ASSERT_NOT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    configuration_get = get_func;
    configuration_set = set_func;
  } else if (node_type == ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONES_NUMBER) {
    TEST_ASSERT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    tone_number_get = get_func;
  } else if (node_type == ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_INFO_IDENTIFIER) {
    TEST_ASSERT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    tone_info_get = get_func;
  } else if (node_type == ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_PLAY) {
    TEST_ASSERT_NOT_NULL(set_func);
    TEST_ASSERT_NOT_NULL(get_func);
    play_get = get_func;
    play_set = set_func;
  }

  return SL_STATUS_OK;
}


// Stub for registering command classes
static sl_status_t zwave_command_handler_register_handler_stub(
  zwave_command_handler_t new_command_class_handler, int cmock_num_calls)
{
  handler = new_command_class_handler;

  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_NONE,
                    handler.minimal_scheme);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SOUND_SWITCH, handler.command_class);
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
  configuration_get  = NULL;
  configuration_set  = NULL;
  tone_number_get  = NULL;
  tone_info_get = NULL;
  memset(received_frame, 0, sizeof(received_frame));
  received_frame_size = 0;
  // Resolution functions
  attribute_resolver_register_rule_Stub(&attribute_resolver_register_rule_stub);
  zwave_command_handler_register_handler_Stub(&zwave_command_handler_register_handler_stub);
  // Call init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_command_class_sound_switch_init());
}

/// Called after each and every test
void tearDown() {}

/// Helpers

void set_supported_version(zwave_cc_version_t version) {
  attribute_store_node_t version_node = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_VERSION, endpoint_id_node);
  attribute_store_set_reported(version_node, &version, sizeof(version));
}
zwave_cc_version_t get_supported_version() {
  zwave_cc_version_t version = 0;
  attribute_store_get_child_reported(endpoint_id_node, ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_VERSION, &version, sizeof(version));
  return version;
}

void set_volume_and_tone(uint8_t volume, uint8_t tone) {
  // Attribute tree is empty as this point so we add it here
  attribute_store_node_t volume_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_VOLUME, endpoint_id_node);
  attribute_store_node_t tone_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_TONE_IDENTIFIER, endpoint_id_node);

  attribute_store_set_desired(volume_node, &volume, sizeof(volume));
  attribute_store_set_desired(tone_node, &tone, sizeof(tone));

  TEST_ASSERT_NOT_NULL(configuration_set);
  // We can either set the volume_node or tone_here here.
  configuration_set(volume_node, received_frame, &received_frame_size);

  const uint8_t expected_frame[]
    = {COMMAND_CLASS_SOUND_SWITCH, SOUND_SWITCH_CONFIGURATION_SET, volume, tone};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}


void set_play(uint8_t tone_id, uint8_t volume) {
  // Get supported version
  zwave_cc_version_t version = get_supported_version();

  // Since we are simulating a specific version the attribute should be created.
  attribute_store_node_t play_node
    = attribute_store_get_node_child_by_type(endpoint_id_node, ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_PLAY, 0);
  attribute_store_node_t volume_node;
  if (version == 2) {
    volume_node = attribute_store_get_node_child_by_type(endpoint_id_node,ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_VOLUME, 0);
  }

  if (tone_id > 0 && tone_id < 255) {
    // Workaround to allow to send a specific tone ID though this attribute.
    //
    // The OnOff UCL cluster only support boolean states and we need a uint8_t to be able to 
    // send specific toneID though the Play Set command.
    // With this workaround we can :
    //  - Play the default tone (255) with the ZCL OnOff attribute (1)
    //  - Stop the tone (0) with the ZCL OnOff attribute (0)
    //  - Play specific tone (1-254) with the ZCL OnTime attribute (1-254) that will be "copied" into our ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_PLAY
    //    - It can be controlled by the command WriteAttribute of the OnOff cluster though MQTT : ucl/by-unid/+/ep0/OnOff/Commands/WriteAttributes {"OnTime":2}

    // WARNING : DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_TIME is on 16 bits not 8
    uint16_t tone_id_16 = tone_id;
    attribute_store_node_t ontime_node = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_TIME, endpoint_id_node);
    sl_status_t result = attribute_store_set_desired(ontime_node, &tone_id_16, sizeof(tone_id_16));
    TEST_ASSERT_EQUAL(SL_STATUS_OK, result );

    // Check that the original play mode has been updated
    uint8_t expected_tone;
    result = attribute_store_get_desired(play_node, &expected_tone, sizeof(expected_tone));
    TEST_ASSERT_EQUAL(SL_STATUS_OK, result);

    // Result should be undefined as we only use this as a buffer
    result = attribute_store_get_desired(ontime_node, &expected_tone, sizeof(expected_tone));
    TEST_ASSERT_EQUAL(SL_STATUS_FAIL, result);

    TEST_ASSERT_EQUAL(expected_tone, tone_id);
  } else {
    // Simple cases : 0 & 255
    attribute_store_set_desired(play_node, &tone_id, sizeof(tone_id));
  }

  if (version == 2) {
    attribute_store_set_desired(volume_node, &volume, sizeof(volume));
  }

  play_set(play_node, received_frame, &received_frame_size);

  if (version == 2) {
    const uint8_t expected_frame[]
      = {COMMAND_CLASS_SOUND_SWITCH, SOUND_SWITCH_TONE_PLAY_SET, tone_id, volume};
    TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                  received_frame,
                                  received_frame_size);
  } else {
    const uint8_t expected_frame[]
      = {COMMAND_CLASS_SOUND_SWITCH, SOUND_SWITCH_TONE_PLAY_SET, tone_id};
    TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                  received_frame,
                                  received_frame_size);
  }
}


void tone_play_report_internal(const uint8_t* frame, int frame_size) {
  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&info, NULL, 0));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info, frame, frame_size));
}

void tone_play_report(uint8_t tone_id) {
  const uint8_t frame[] = {COMMAND_CLASS_SOUND_SWITCH,
                           SOUND_SWITCH_TONE_PLAY_REPORT,
                           tone_id};

  tone_play_report_internal(frame, sizeof(frame));

  // No volume so it shouldn't be set
  attribute_store_node_t volume_node = attribute_store_get_node_child_by_type(
    endpoint_id_node,
    ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_VOLUME,
    0);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, volume_node);

  attribute_store_node_t tone_node = attribute_store_get_node_child_by_type(
    endpoint_id_node,
    ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_PLAY,
    0);
  TEST_ASSERT_EQUAL(tone_id, attribute_store_get_reported_number(tone_node));
}

void tone_play_report_with_volume(uint8_t tone_id, uint8_t volume) {
  const uint8_t frame[] = {COMMAND_CLASS_SOUND_SWITCH,
                           SOUND_SWITCH_TONE_PLAY_REPORT,
                           tone_id,
                           volume};

  tone_play_report_internal(frame, sizeof(frame));

  // No volume so it shouldn't be set
  attribute_store_node_t volume_node = attribute_store_get_node_child_by_type(
    endpoint_id_node,
    ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_VOLUME,
    0);
  TEST_ASSERT_EQUAL(volume, attribute_store_get_reported_number(volume_node));

  attribute_store_node_t tone_node = attribute_store_get_node_child_by_type(
    endpoint_id_node,
    ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_PLAY,
    0);
  TEST_ASSERT_EQUAL(tone_id, attribute_store_get_reported_number(tone_node));
}

/// Test cases

void test_sound_switch_configuration_report_volume_over_100()
{
  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&info, NULL, 0));

  const uint8_t frame[] = {COMMAND_CLASS_SOUND_SWITCH,
                           SOUND_SWITCH_CONFIGURATION_REPORT,
                           101,
                           0x55};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info, frame, sizeof(frame)));

  attribute_store_node_t volume_node = attribute_store_get_node_child_by_type(
    endpoint_id_node,
    ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_VOLUME,
    0);
  TEST_ASSERT_EQUAL(100, attribute_store_get_reported_number(volume_node));

  attribute_store_node_t tone_node = attribute_store_get_node_child_by_type(
    endpoint_id_node,
    ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_CONFIGURED_DEFAULT_TONE_IDENTIFIER,
    0);
  TEST_ASSERT_EQUAL(0x55, attribute_store_get_reported_number(tone_node));
}

void test_sound_switch_tone_number_report_happy_case()
{
    // Attribute tree is empty as this point so we add it here
  attribute_store_node_t tone_node
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONES_NUMBER, endpoint_id_node);

  const uint8_t EXPECTED_TONE_NUMBER = 12;

  zwave_controller_connection_info_t info = {};
  info.remote.node_id                     = node_id;
  info.remote.endpoint_id                 = endpoint_id;
  info.local.is_multicast                 = false;

  TEST_ASSERT_NOT_NULL(handler.control_handler);
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    handler.control_handler(&info, NULL, 0));

  // First test the report of all tone numbers
  const uint8_t frame[] = {COMMAND_CLASS_SOUND_SWITCH,
                           SOUND_SWITCH_TONES_NUMBER_REPORT,
                           EXPECTED_TONE_NUMBER};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    handler.control_handler(&info, frame, sizeof(frame)));

  TEST_ASSERT_EQUAL(attribute_store_get_node_child_count(tone_node), 
    (size_t)EXPECTED_TONE_NUMBER);

  // Test if all the tone number ID are correctly defined
  for (uint8_t i=1; i<=EXPECTED_TONE_NUMBER;i++) {
      attribute_store_node_t current_tone_id_node = attribute_store_get_node_child(tone_node, i-1);
      uint8_t expected_value;
      sl_status_t status = attribute_store_get_desired(current_tone_id_node, &expected_value, sizeof(expected_value));
      TEST_ASSERT_EQUAL(status, SL_STATUS_OK);
      TEST_ASSERT_EQUAL(i, expected_value);
  }

  // Now we simulate a report for all node and see if their info are correctly updated
  for (uint8_t i=1; i<EXPECTED_TONE_NUMBER;i++) {
      uint8_t expected_duration = i + 12;
      char expected_name[3] = {i >= 10 ? '1' : '0',  (i % 10) + '0', '\0'};

      uint8_t report_tone_info[] = {COMMAND_CLASS_SOUND_SWITCH,
          SOUND_SWITCH_TONE_INFO_REPORT,
          i, // Tone Identifier
          0, // Byte 1 Tone duration
          expected_duration, // Byte 2 Tone duration
          2, // Name length
          expected_name[0], // Name 1
          expected_name[1] // Name 2
      };

      TEST_ASSERT_EQUAL(SL_STATUS_OK,
              handler.control_handler(&info, report_tone_info, sizeof(report_tone_info)));

      attribute_store_node_t current_tone_id_node = attribute_store_get_node_child(tone_node, i-1);
      
      // Duration test
      uint16_t duration;
      attribute_store_get_child_reported(current_tone_id_node, 
        ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_INFO_DURATION_SECONDS, &duration, sizeof(duration));
      TEST_ASSERT_EQUAL(duration, (uint16_t)expected_duration);

      // Name test
      char name[3];
      attribute_store_node_t name_node = attribute_store_get_node_child_by_type(current_tone_id_node,
        ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_INFO_NAME, 0);

      attribute_store_get_reported_string(name_node, name, 3);
      TEST_ASSERT_EQUAL_CHAR_ARRAY(expected_name, name, 3);
  }
}

void test_sound_switch_configuration_get_happy_case()
{
  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL(configuration_get);
  configuration_get(0, received_frame, &received_frame_size);
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_SOUND_SWITCH, SOUND_SWITCH_CONFIGURATION_GET};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}


void test_sound_switch_configuration_set_happy_case()
{
  set_volume_and_tone(15,30);
}

void test_sound_switch_configuration_set_volume_0()
{
  set_volume_and_tone(0,30);
}

void test_sound_switch_play_get_play()
{
  // Ask for a Get Command, should always be the same
  TEST_ASSERT_NOT_NULL(play_get);
  play_get(0, received_frame, &received_frame_size);
  const uint8_t expected_frame[]
    = {COMMAND_CLASS_SOUND_SWITCH, SOUND_SWITCH_TONE_PLAY_GET};
  TEST_ASSERT_EQUAL(sizeof(expected_frame), received_frame_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_frame,
                                received_frame,
                                received_frame_size);
}

void test_sound_switch_play_report_v1()
{
  tone_play_report(12);
}

void test_sound_switch_play_report_v2()
{
  tone_play_report_with_volume(12,24);
}

void test_sound_switch_play_set_play_default_v1()
{
  set_supported_version(1);
  set_play(255,255);
}

void test_sound_switch_play_set_stop_v1()
{
  set_supported_version(1);
  set_play(0,255);
}

void test_sound_switch_play_set_custom_tone_v1()
{
  set_supported_version(1);
  set_play(55,255);
}


void test_sound_switch_play_set_play_default_v2()
{
  set_supported_version(2);
  set_play(255,255);
}

void test_sound_switch_play_set_stop_v2()
{
  set_supported_version(2);
  set_play(0,255);
}

void test_sound_switch_play_set_custom_tone_v2()
{
  set_supported_version(2);
  set_play(55,255);
}

void test_sound_switch_play_set_custom_tone_and_volume_v2()
{
  set_supported_version(2);
  set_play(55,15);
}

// Test if sound switch attributes are note created if DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_TIME is changed
void test_no_update_if_no_sound_switch() {
    // WARNING : DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_TIME is on 16 bits not 8
    uint16_t tone_id_16 = 12;
    attribute_store_node_t ontime_node = attribute_store_add_node(DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_TIME, endpoint_id_node);
    sl_status_t result = attribute_store_set_desired(ontime_node, &tone_id_16, sizeof(tone_id_16));
    TEST_ASSERT_EQUAL(SL_STATUS_OK, result );


    // This node shouldn't exist since DOTDOT_ATTRIBUTE_ID_ON_OFF_ON_TIME must not create it.
    attribute_store_node_t play_node = 
      attribute_store_get_node_child_by_type(ATTRIBUTE_COMMAND_CLASS_SOUND_SWITCH_TONE_PLAY, endpoint_id_node, 0);

    TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, play_node);
}