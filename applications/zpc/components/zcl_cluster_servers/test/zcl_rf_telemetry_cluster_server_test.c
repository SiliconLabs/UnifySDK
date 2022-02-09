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
#include "zcl_rf_telemetry_cluster_server.h"
#include "unity.h"

// Includes from other components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "attribute_store_defined_attribute_types.h"
#include "zpc_attribute_store_network_helper.h"
#include "dotdot_attributes.h"

// Test Helpers
#include "zpc_attribute_store_test_helper.h"

// Interfaces
#include "sl_status.h"
#include "zwave_controller_types.h"
#include "zwave_utils.h"

// Mocks
#include "zwave_controller_callbacks_mock.h"
#include "zwave_network_management_mock.h"
#include "dotdot_mqtt_mock.h"
#include "dotdot_mqtt_supported_generated_commands_mock.h"

#include "sl_log.h"

// Constants
static const uic_mqtt_dotdot_protocol_controller_rf_telemetry_supported_commands_t
  expected_supported_generated_commands
  = {.tx_report = true};

// Static variables
const zwave_controller_callbacks_t *zwave_controller_callbacks;

// Stub functions
static sl_status_t zwave_controller_register_callbacks_stub(
  const zwave_controller_callbacks_t *callbacks, int num_calls)
{
  TEST_ASSERT_NOT_NULL(callbacks);
  TEST_ASSERT_NOT_NULL(callbacks->on_frame_transmission);

  zwave_controller_callbacks = callbacks;
  return SL_STATUS_OK;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
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
  zwave_network_management_get_home_id_IgnoreAndReturn(home_id);
  zwave_network_management_get_node_id_IgnoreAndReturn(zpc_node_id);
  zwave_controller_register_callbacks_Stub(
    &zwave_controller_register_callbacks_stub);
}

void test_configure_create_tx_report_enabled_attribute()
{
  uic_mqtt_dotdot_publish_supported_commands_Expect(NULL, 0);
  uic_mqtt_dotdot_publish_supported_commands_IgnoreArg_unid();

  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_supported_generated_commands_ExpectWithArray(
    NULL,
    &expected_supported_generated_commands,
    sizeof(expected_supported_generated_commands));
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_supported_generated_commands_IgnoreArg_unid();

  // Empty attribute store, expect the subscription by group after init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zcl_rf_telemetry_cluster_server_init());

  // Verify that ZPC endpoint 0 has a TX Report enabled attribute:
  attribute_store_node_t tx_report_enabled_node
    = attribute_store_get_node_child_by_type(
      zpc_endpoint_id_node,
      DOTDOT_ATTRIBUTE_ID_PROTOCOL_CONTROLLER_RF_TELEMETRY_TX_REPORT_ENABLED,
      0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, tx_report_enabled_node);

  // Verify that the default value is 0.
  tx_report_enabled_t value = 0xEE;
  attribute_store_get_reported(tx_report_enabled_node, &value, sizeof(value));
  TEST_ASSERT_EQUAL(0, value);

  // Delete the attribute.
  attribute_store_delete_node(tx_report_enabled_node);

  // Ask for init again.
  uic_mqtt_dotdot_publish_supported_commands_Expect(NULL, 0);
  uic_mqtt_dotdot_publish_supported_commands_IgnoreArg_unid();
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_supported_generated_commands_ExpectWithArray(
    NULL,
    &expected_supported_generated_commands,
    sizeof(expected_supported_generated_commands));
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_supported_generated_commands_IgnoreArg_unid();
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zcl_rf_telemetry_cluster_server_init());

  // Same verifications again, attribute is created and set to default value:
  tx_report_enabled_node = attribute_store_get_node_child_by_type(
    zpc_endpoint_id_node,
    DOTDOT_ATTRIBUTE_ID_PROTOCOL_CONTROLLER_RF_TELEMETRY_TX_REPORT_ENABLED,
    0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, tx_report_enabled_node);
  value = 0xEE;
  attribute_store_get_reported(tx_report_enabled_node, &value, sizeof(value));
  TEST_ASSERT_EQUAL(0, value);

  // Extra init should not do anything special.
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_supported_generated_commands_ExpectWithArray(
    NULL,
    &expected_supported_generated_commands,
    sizeof(expected_supported_generated_commands));
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_supported_generated_commands_IgnoreArg_unid();

  TEST_ASSERT_EQUAL(SL_STATUS_OK, zcl_rf_telemetry_cluster_server_init());
}

void test_update_desired_value()
{
  uic_mqtt_dotdot_publish_supported_commands_Expect(NULL, 0);
  uic_mqtt_dotdot_publish_supported_commands_IgnoreArg_unid();
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_supported_generated_commands_ExpectWithArray(
    NULL,
    &expected_supported_generated_commands,
    sizeof(expected_supported_generated_commands));
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_supported_generated_commands_IgnoreArg_unid();

  // Empty attribute store, expect the subscription by group after init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zcl_rf_telemetry_cluster_server_init());

  // Verify that ZPC endpoint 0 has a TX Report enabled attribute:
  attribute_store_node_t tx_report_enabled_node
    = attribute_store_get_node_child_by_type(
      zpc_endpoint_id_node,
      DOTDOT_ATTRIBUTE_ID_PROTOCOL_CONTROLLER_RF_TELEMETRY_TX_REPORT_ENABLED,
      0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, tx_report_enabled_node);

  // Set the desired, the reported should follow.
  tx_report_enabled_t value = 0xEE;
  attribute_store_set_desired(tx_report_enabled_node, &value, sizeof(value));
  attribute_store_get_reported(tx_report_enabled_node, &value, sizeof(value));
  TEST_ASSERT_EQUAL(0xEE, value);

  // Empty desired does not get propagated to reported:
  attribute_store_set_desired(tx_report_enabled_node, NULL, 0);
  value = 0x00;
  attribute_store_get_reported(tx_report_enabled_node, &value, sizeof(value));
  TEST_ASSERT_EQUAL(0xEE, value);

  value = 0x00;
  attribute_store_set_desired(tx_report_enabled_node, &value, sizeof(value));
  attribute_store_get_reported(tx_report_enabled_node, &value, sizeof(value));
  TEST_ASSERT_EQUAL(0x00, value);
}

void test_publish_rf_report_null_pointer()
{
  uic_mqtt_dotdot_publish_supported_commands_Expect(NULL, 0);
  uic_mqtt_dotdot_publish_supported_commands_IgnoreArg_unid();

  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_supported_generated_commands_ExpectWithArray(
    NULL,
    &expected_supported_generated_commands,
    sizeof(expected_supported_generated_commands));
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_supported_generated_commands_IgnoreArg_unid();

  // Empty attribute store, expect the subscription by group after init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zcl_rf_telemetry_cluster_server_init());

  // Verify that ZPC endpoint 0 has a TX Report enabled attribute:
  attribute_store_node_t tx_report_enabled_node
    = attribute_store_get_node_child_by_type(
      zpc_endpoint_id_node,
      DOTDOT_ATTRIBUTE_ID_PROTOCOL_CONTROLLER_RF_TELEMETRY_TX_REPORT_ENABLED,
      0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, tx_report_enabled_node);

  // Telemetry disabled: nothing happens.
  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks->on_frame_transmission);
  zwave_controller_callbacks->on_frame_transmission(true, NULL, 4);

  // Enable Telemetry:
  tx_report_enabled_t value = 0x01;
  attribute_store_set_desired(tx_report_enabled_node, &value, sizeof(value));

  // Telemetry enabled but NULL pointer: nothing happens.
  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks->on_frame_transmission);
  zwave_controller_callbacks->on_frame_transmission(true, NULL, 4);

  // Delete the attribute for some test coverage.
  attribute_store_delete_node(tx_report_enabled_node);
}

void test_publish_rf_report()
{
  uic_mqtt_dotdot_publish_supported_commands_Expect(NULL, 0);
  uic_mqtt_dotdot_publish_supported_commands_IgnoreArg_unid();
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_supported_generated_commands_ExpectWithArray(
    NULL,
    &expected_supported_generated_commands,
    sizeof(expected_supported_generated_commands));
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_supported_generated_commands_IgnoreArg_unid();

  // Empty attribute store, expect the subscription by group after init
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zcl_rf_telemetry_cluster_server_init());

  // Verify that ZPC endpoint 0 has a TX Report enabled attribute:
  attribute_store_node_t tx_report_enabled_node
    = attribute_store_get_node_child_by_type(
      zpc_endpoint_id_node,
      DOTDOT_ATTRIBUTE_ID_PROTOCOL_CONTROLLER_RF_TELEMETRY_TX_REPORT_ENABLED,
      0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, tx_report_enabled_node);

  // Prepare a test TX Report

  zwapi_tx_report_t tx_report       = {};
  tx_report.transmit_ticks          = 23;
  tx_report.last_route_speed        = 1;
  tx_report.routing_attempts        = 2;
  tx_report.route_scheme_state      = ROUTINGSCHEME_CACHED_ROUTE;
  tx_report.rssi_values.incoming[0] = -120;
  tx_report.rssi_values.incoming[1] = -121;
  tx_report.rssi_values.incoming[2] = -122;
  tx_report.rssi_values.incoming[3] = -123;
  tx_report.number_of_repeaters     = 4;
  tx_report.last_route_repeaters[0] = 10;
  tx_report.last_route_repeaters[1] = 2;
  tx_report.last_route_repeaters[2] = 5;
  tx_report.last_route_repeaters[3] = 3;

  // UNID data:
  unid_t zpc_unid;
  zwave_unid_from_node_id(zpc_node_id, zpc_unid);
  unid_t remote_node_unid;
  zwave_unid_from_node_id(node_id, remote_node_unid);

  // Telemetry disabled: nothing happens.
  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks->on_frame_transmission);
  zwave_controller_callbacks->on_frame_transmission(true, &tx_report, 4);

  // Enable Telemetry:
  tx_report_enabled_t value = 0x01;
  attribute_store_set_desired(tx_report_enabled_node, &value, sizeof(value));

  // Telemetry enabled : Expect to pass the data to dotdot mqtt
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_generated_tx_report_command_Expect(
    zpc_unid,
    NULL);

  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_generated_tx_report_command_IgnoreArg_unid();
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_generated_tx_report_command_IgnoreArg_fields();

  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks->on_frame_transmission);
  zwave_controller_callbacks->on_frame_transmission(true, &tx_report, node_id);

  // Again with different parameters
  tx_report.route_scheme_state      = ROUTINGSCHEME_IDLE;
  tx_report.last_route_speed        = 2;
  tx_report.last_failed_link.from   = 23;
  tx_report.last_failed_link.to     = 24;
  tx_report.number_of_repeaters     = 1;
  tx_report.last_route_repeaters[0] = 34;
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_generated_tx_report_command_Expect(
    zpc_unid,
    NULL);

  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_generated_tx_report_command_IgnoreArg_unid();
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_generated_tx_report_command_IgnoreArg_fields();

  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks->on_frame_transmission);
  zwave_controller_callbacks->on_frame_transmission(true, &tx_report, node_id);

  // Again with failed transmission
  tx_report.last_route_speed = 3;

  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_generated_tx_report_command_Expect(
    zpc_unid,
    NULL);

  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_generated_tx_report_command_IgnoreArg_unid();
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_generated_tx_report_command_IgnoreArg_fields();

  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks->on_frame_transmission);
  zwave_controller_callbacks->on_frame_transmission(false, &tx_report, node_id);

  //Z-Wave Long Range 100kbit transmit speed
  tx_report.last_route_speed = 4;

  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_generated_tx_report_command_Expect(
    zpc_unid,
    NULL);

  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_generated_tx_report_command_IgnoreArg_unid();
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_generated_tx_report_command_IgnoreArg_fields();

  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks->on_frame_transmission);
  zwave_controller_callbacks->on_frame_transmission(false, &tx_report, node_id);

  // Unknown transmit speed
  tx_report.last_route_speed = 5;

  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_generated_tx_report_command_Expect(
    zpc_unid,
    NULL);

  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_generated_tx_report_command_IgnoreArg_unid();
  uic_mqtt_dotdot_protocol_controller_rf_telemetry_publish_generated_tx_report_command_IgnoreArg_fields();

  TEST_ASSERT_NOT_NULL(zwave_controller_callbacks->on_frame_transmission);
  zwave_controller_callbacks->on_frame_transmission(false, &tx_report, node_id);
}