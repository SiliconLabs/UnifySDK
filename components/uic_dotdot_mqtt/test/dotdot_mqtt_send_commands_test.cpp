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

void test_on_off_publish_on_off_command()
{
  const dotdot_unid_t destination_unid      = "zw-cafecafe-1010";
  dotdot_endpoint_id_t destination_endpoint = 2;

  // Ask DotDot MQTT to publish.
  uic_mqtt_dotdot_on_off_publish_toggle_command(destination_unid,
                                                destination_endpoint);

  // Verify that it did its job:
  const char *expected_topic
    = "ucl/by-unid/zw-cafecafe-1010/ep2/OnOff/Commands/Toggle";

  char published_message[1000] = {};
  TEST_ASSERT_NOT_NULL(
    mqtt_test_helper_pop_publish(expected_topic, published_message));

  const char expected_published_message[] = "{}";
  TEST_ASSERT_EQUAL_JSON(expected_published_message, published_message);
}

void test_level_publish_move_command_to_group()
{
  uint16_t destination_group_id = 4545;
  uic_mqtt_dotdot_level_command_move_to_level_fields_t fields;
  fields.level            = 20;
  fields.transition_time  = 30;
  fields.options_mask     = 3;
  fields.options_override = 0;

  // Ask DotDot MQTT to publish.
  uic_mqtt_dotdot_level_publish_move_to_level_command_to_group(
    destination_group_id,
    &fields);

  // Verify that it did its job:
  const char *expected_topic = "ucl/by-group/4545/Level/Commands/MoveToLevel";

  char published_message[1000] = {};
  TEST_ASSERT_NOT_NULL(
    mqtt_test_helper_pop_publish(expected_topic, published_message));

  const char expected_published_message[] = R"({
  "Level": 20,
  "TransitionTime": 30,
  "OptionsMask": {
    "CoupleColorTempToLevel": true,
    "ExecuteIfOff": true
  },
  "OptionsOverride": {
    "CoupleColorTempToLevel": false,
    "ExecuteIfOff": false
  }
})";
  TEST_ASSERT_EQUAL_JSON(expected_published_message, published_message);
}

void test_publish_generated_rf_telemetry_tx_report()
{
  const dotdot_unid_t destination_unid = "zw-cafecafe-1010";
  const char *repeaters[]              = {"Repeater1", "Repeater2"};
  const int8_t repeaters_rssi[]        = {-70, -40};

  const uic_mqtt_dotdot_protocol_controller_rf_telemetry_command_tx_report_fields_t
    fields
    = {
      .sourceunid              = "SourceUNIDTest",
      .destinationunid         = "DestinationsUNIDTest",
      .transmission_successful = true,
      .transmission_time_ms    = 1020,
      .tx_powerd_bm            = 10,
      .tx_channel              = 3,
      .routing_attempts        = 20,
      .route_changed           = true,
      .transmission_speed
      = ZCL_TX_REPORT_TRANSMISSION_SPEED_Z_WAVE_LONG_RANGE_100_KBITS_PER_SECOND,
      .measured_noise_floord_bm                  = -99,
      .last_route_repeaters_count                = 2,
      .last_route_repeaters                      = repeaters,
      .incoming_rssi_repeaters_count              = 2,
      .incoming_rssi_repeaters                    = repeaters_rssi,
      .ackrssi                                   = -50,
      .ack_channel                               = 2,
      .last_route_failed_link_functionalunid     = "",
      .last_route_failed_link_non_functionalunid = "",
      .destination_ack_tx_powerd_bm              = -52,
      .destination_ack_measuredrssi              = -53,
      .destination_ack_measured_noise_floor      = -96};

  // Ask DotDot MQTT to publish.
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_generated_tx_report_command(
    destination_unid,
    &fields);

  // Verify that it did its job:
  const char *expected_topic
    = "ucl/by-unid/zw-cafecafe-1010/ProtocolController/RFTelemetry/"
      "GeneratedCommands/TxReport";

  char published_message[1000] = {};
  TEST_ASSERT_NOT_NULL(
    mqtt_test_helper_pop_publish(expected_topic, published_message));

  const char expected_published_message[] = R"({
  "AckChannel": 2,
  "AckRSSI": -50,
  "DestinationAckMeasuredNoiseFloor": -96,
  "DestinationAckMeasuredRSSI": -53,
  "DestinationAckTxPowerdBm": -52,
  "DestinationUNID": "DestinationsUNIDTest",
  "IncomingRSSIRepeaters": [
    -70,
    -40
  ],
  "LastRouteFailedLinkFunctionalUNID": "",
  "LastRouteFailedLinkNonFunctionalUNID": "",
  "LastRouteRepeaters": [
    "Repeater1",
    "Repeater2"
  ],
  "MeasuredNoiseFloordBm": -99,
  "RouteChanged": true,
  "RoutingAttempts": 20,
  "SourceUNID": "SourceUNIDTest",
  "TransmissionSpeed": "Z-WaveLongRange-100-kbits-per-second",
  "TransmissionSuccessful": true,
  "TransmissionTimeMs": 1020,
  "TxChannel": 3,
  "TxPowerdBm": 10
  }
  )";
  TEST_ASSERT_EQUAL_JSON(expected_published_message, published_message);
}

void test_publish_state_interview_command()
{
  const dotdot_unid_t destination_unid = "ble-pd12344321";

  // Ask DotDot MQTT to publish.
  uic_mqtt_dotdot_state_publish_interview_command(destination_unid);

  // Verify that it did its job:
  const char *expected_topic
    = "ucl/by-unid/ble-pd12344321/State/Commands/Interview";

  char published_message[4] = {};
  TEST_ASSERT_NOT_NULL(
    mqtt_test_helper_pop_publish(expected_topic, published_message));

  const char expected_published_message[] = R"({})";
  TEST_ASSERT_EQUAL_JSON(expected_published_message, published_message);
}
}
