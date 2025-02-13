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

// Generic includes
#include <stdlib.h>

// Test includes
#include "workaround.hpp"
#include "unity.h"
#include "unity_helpers.h"
#include "cmock.h"

// Interfaces
#include "sl_status.h"

// Mock includes
#include "mqtt_test_helper.h"

extern "C" {
// Component being tested
#include "dotdot_mqtt_supported_generated_commands.h"

/// Called before each and every test
void setUp()
{
  mqtt_test_helper_init();
}

void test_aox_locator_publish_scene_supported_generated_commands()
{
  const dotdot_unid_t unid      = "ble-1334";
  dotdot_endpoint_id_t endpoint = 99;

  uic_mqtt_dotdot_scenes_supported_commands_t command_list
    = {.add_scene = true, .view_scene_response = true, .recall_scene = true};

  // Ask DotDot MQTT to publish.
  uic_mqtt_dotdot_scenes_publish_supported_generated_commands(unid,
                                                              endpoint,
                                                              &command_list);

  // Verify that it did its job:
  const char *expected_topic
    = "ucl/by-unid/ble-1334/ep99/Scenes/SupportedGeneratedCommands";

  char published_message[1000] = {};
  TEST_ASSERT_NOT_NULL(
    mqtt_test_helper_pop_publish(expected_topic, published_message));

  const char expected_published_message[]
    = R"({"value": ["AddScene","ViewSceneResponse","RecallScene"]})";

  TEST_ASSERT_EQUAL_JSON(expected_published_message, published_message);
}

void test_aox_locator_publish_carbon_monoxide_supported_generated_commands()
{
  const dotdot_unid_t unid      = "A Weird Unid";
  dotdot_endpoint_id_t endpoint = 0;

  uic_mqtt_dotdot_carbon_monoxide_supported_commands_t command_list
    = {.write_attributes = true};

  // Ask DotDot MQTT to publish.
  uic_mqtt_dotdot_carbon_monoxide_publish_supported_generated_commands(
    unid,
    endpoint,
    &command_list);

  // Verify that it did its job:
  const char *expected_topic
    = "ucl/by-unid/A Weird Unid/ep0/CarbonMonoxide/SupportedGeneratedCommands";

  char published_message[1000] = {};
  TEST_ASSERT_NOT_NULL(
    mqtt_test_helper_pop_publish(expected_topic, published_message));

  const char expected_published_message[] = R"({"value": ["WriteAttributes"]})";

  TEST_ASSERT_EQUAL_JSON(expected_published_message, published_message);
}

void test_aox_locator_publish_rf_telemetry_supported_generated_commands_no_endpoint()
{
  const dotdot_unid_t unid = "ZPC_unid_1234";

  uic_mqtt_dotdot_protocol_controller_rf_telemetry_supported_commands_t
    command_list
    = {.tx_report = true};

  // Ask DotDot MQTT to publish.
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_supported_generated_commands(
    unid,
    &command_list);

  // Verify that it did its job:
  const char *expected_topic = "ucl/by-unid/ZPC_unid_1234/ProtocolController/"
                               "RFTelemetry/SupportedGeneratedCommands";

  char published_message[1000] = {};
  TEST_ASSERT_NOT_NULL(
    mqtt_test_helper_pop_publish(expected_topic, published_message));

  const char expected_published_message[] = R"({"value": ["TxReport"]})";

  TEST_ASSERT_EQUAL_JSON(expected_published_message, published_message);
}

}
