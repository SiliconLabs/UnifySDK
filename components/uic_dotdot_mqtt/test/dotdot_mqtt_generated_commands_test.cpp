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

extern "C" {
#include "dotdot_mqtt_send_commands.h"
#include "dotdot_mqtt_generated_commands.h"

/// Called before each and every test
void setUp()
{
  mqtt_test_helper_init();
}

void test_rf_telemetry_publish_generated_write_attribute_command()
{
  const dotdot_unid_t unid = "ble-1334";

  uic_mqtt_dotdot_protocol_controller_rf_telemetry_state_t attribute_values
    = {.tx_report_enabled = false, .pti_enabled = false};
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_updated_state_t
    attribute_list
    = {.tx_report_enabled = false, .pti_enabled = true};

  // Ask DotDot MQTT to publish.
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_generated_write_attributes_command(
    unid,
    attribute_values,
    attribute_list);

  // Verify that it did its job:
  const char *expected_topic = "ucl/by-unid/ble-1334/ProtocolController/"
                               "RFTelemetry/GeneratedCommands/WriteAttributes";

  char published_message[1000] = {};
  TEST_ASSERT_NOT_NULL(
    mqtt_test_helper_pop_publish(expected_topic, published_message));

  const char expected_published_message[] = R"({ "PTIEnabled": false } )";

  TEST_ASSERT_EQUAL_JSON(expected_published_message, published_message);
}

void test_thermostat_publish_generated_write_attribute_command()
{
  const dotdot_unid_t unid                  = "zw-1234567-1234";
  dotdot_endpoint_id_t destination_endpoint = 99;

  uic_mqtt_dotdot_thermostat_state_t attribute_values
    = {.remote_sensing                = 2,
       .control_sequence_of_operation = 3,
       .system_mode                   = 4};
  uic_mqtt_dotdot_thermostat_updated_state_t attribute_list
    = {.remote_sensing                = true,
       .control_sequence_of_operation = true,
       .system_mode                   = true};

  // Ask DotDot MQTT to publish.
  uic_mqtt_dotdot_thermostat_publish_generated_write_attributes_command(
    unid,
    destination_endpoint,
    attribute_values,
    attribute_list);

  // Verify that it did its job:
  const char *expected_topic = "ucl/by-unid/zw-1234567-1234/ep99/"
                               "Thermostat/GeneratedCommands/WriteAttributes";

  char published_message[1000] = {};
  TEST_ASSERT_NOT_NULL(
    mqtt_test_helper_pop_publish(expected_topic, published_message));

  const char expected_published_message[] = R"(
    {
      "ControlSequenceOfOperation":"HeatingWithReheat",
      "RemoteSensing":{
        "LocalTemperatureRemote":"false",
        "OccupancyRemote":"false",
        "OutdoorTemperatureRemote":"true"},
      "SystemMode":"Heat"
    } )";

  TEST_ASSERT_EQUAL_JSON(expected_published_message, published_message);
}

void test_publish_generated_write_attributes_command_non_utf8()
{
  const dotdot_unid_t unid                  = "zw-1234567-1234";
  dotdot_endpoint_id_t destination_endpoint = 99;
  const char str[] = {'h', 'e', 'l', 'l', 'o', static_cast<char>(0xa2), 0};
  uic_mqtt_dotdot_basic_state_t attribute_values
    = {.location_description = str};
  uic_mqtt_dotdot_basic_updated_state_t attribute_list
    = {.location_description = true};
  uic_mqtt_dotdot_basic_publish_generated_write_attributes_command(
    unid,
    destination_endpoint,
    attribute_values,
    attribute_list);
  // Verify that it did its job:
  const char *expected_topic = "ucl/by-unid/zw-1234567-1234/ep99/"
                               "Basic/GeneratedCommands/WriteAttributes";

  char published_message[1000] = {};
  TEST_ASSERT_NOT_NULL(
    mqtt_test_helper_pop_publish(expected_topic, published_message));

  const char expected_published_message[] = R"(
    {
      "LocationDescription":"hello�"
    } )";

  TEST_ASSERT_EQUAL_JSON(expected_published_message, published_message);
}
}
