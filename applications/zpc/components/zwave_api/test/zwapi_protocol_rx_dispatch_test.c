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
#include "zwapi_internal.h"
#include "unity.h"
#include "zwapi_init_mock.h"

// Generic includes
#include <string.h>

// List of static test variables
static int zwapi_send_data_test_callback_call_count                     = 0;
static int zwapi_application_controller_update_test_function_call_count = 0;
static int zwapi_zwave_api_started_test_function_call_count             = 0;
static int zwapi_application_command_handler_test_function_call_count   = 0;

// book-keeping variables
static uint8_t received_status              = 0;
static zwapi_tx_report_t received_tx_report = {};
static zwave_node_id_t received_node_id     = 0;
static zwave_home_id_t received_home_id     = 0;

// Z-Wave API callbacks (set at init)
static zwapi_callbacks_t test_zwapi_callbacks = {};

// Test callbacks
void zwapi_send_data_test_callback(uint8_t status, zwapi_tx_report_t *tx_report)
{
  received_status = status;
  if (tx_report != NULL) {
    received_tx_report = *tx_report;
  }
  zwapi_send_data_test_callback_call_count += 1;
}

void zwapi_application_controller_update_test_function(
  uint8_t status,
  zwave_node_id_t node_id,
  const uint8_t *zwave_nif,
  uint8_t zwave_nif_length,
  zwave_home_id_t nwi_home_id)
{
  received_node_id = node_id;
  received_home_id = nwi_home_id;
  zwapi_application_controller_update_test_function_call_count += 1;
}

void zwapi_application_command_handler_test_function(
  uint8_t rx_status,
  zwave_node_id_t destination_node_id,
  zwave_node_id_t source_node_id,
  const uint8_t *received_frame,
  uint8_t received_frame_length,
  int8_t rssi_value)
{
  zwapi_application_command_handler_test_function_call_count += 1;
  received_node_id = source_node_id;
}

void zwapi_zwave_api_started_test_function(const uint8_t *buffer,
                                           uint8_t buffer_length)
{
  (void)buffer;
  (void)buffer_length;
  zwapi_zwave_api_started_test_function_call_count += 1;
}

static void reset_all_test_variables()
{
  // callback counters
  zwapi_send_data_test_callback_call_count                     = 0;
  zwapi_application_controller_update_test_function_call_count = 0;
  zwapi_zwave_api_started_test_function_call_count             = 0;
  zwapi_application_command_handler_test_function_call_count   = 0;

  // Received data
  received_status  = 0;
  received_node_id = 0;
  received_home_id = 0;
  memset(&received_tx_report, 0, sizeof(received_tx_report));
  memset(&test_zwapi_callbacks, 0, sizeof(test_zwapi_callbacks));
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  reset_all_test_variables();
  // Un-init the Z-Wave API callbacks.
  // Make sure the dispatch module starts with NULL pointers for all callbacks
  zwave_api_protocol_init_callbacks();
}

void test_zwapi_protocol_rx_dispatch_test_send_data_callback_tx_report_happy()
{
  // Manually add our callback:
  zwave_api_get_callbacks_IgnoreAndReturn(&test_zwapi_callbacks);
  zwapi_send_data_callback = &zwapi_send_data_test_callback;
  uint8_t frame[] = {0x1D, 0x00, 0x13, 0x00, 0x00, 0x12, 0x23, 0x05, 0xCF, 0x7D,
                     0x7F, 0x6F, 0x7F, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00,
                     0x02, 0x01, 0x00, 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
  zwave_api_protocol_rx_dispatch(frame, sizeof(frame));

  TEST_ASSERT_EQUAL(1, zwapi_send_data_test_callback_call_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, received_status);
  TEST_ASSERT_EQUAL(0x1223, received_tx_report.transmit_ticks);
  TEST_ASSERT_EQUAL(0x05, received_tx_report.number_of_repeaters);
  TEST_ASSERT_EQUAL((int8_t)0xCF, received_tx_report.ack_rssi);
  TEST_ASSERT_EQUAL(0x7D, received_tx_report.rssi_values.incoming[0]);
  TEST_ASSERT_EQUAL(0x7F, received_tx_report.rssi_values.incoming[1]);
  TEST_ASSERT_EQUAL(0x6F, received_tx_report.rssi_values.incoming[2]);
  TEST_ASSERT_EQUAL((int8_t)0xAA, received_tx_report.tx_power);
  TEST_ASSERT_EQUAL((int8_t)0xBB, received_tx_report.measured_noise_floor);
  TEST_ASSERT_EQUAL((int8_t)0xCC,
                    received_tx_report.destination_ack_mpdu_tx_power);
  TEST_ASSERT_EQUAL((int8_t)0xDD,
                    received_tx_report.destination_ack_mpdu_measured_rssi);
  TEST_ASSERT_EQUAL(
    (int8_t)0xEE,
    received_tx_report.destination_ack_mpdu_measured_noise_floor);
}

void test_zwapi_protocol_rx_dispatch_test_send_data_callback_tx_report_no_long_range_data()
{
  // Manually add our callback:
  zwave_api_get_callbacks_IgnoreAndReturn(&test_zwapi_callbacks);
  zwapi_send_data_callback = &zwapi_send_data_test_callback;
  uint8_t frame[]
    = {0x1D, 0x00, 0x13, 0x00, 0x00, 0x12, 0x23, 0x05, 0xCF, 0x7D, 0x7F, 0x6F,
       0x7F, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00};
  zwave_api_protocol_rx_dispatch(frame, sizeof(frame));

  TEST_ASSERT_EQUAL(1, zwapi_send_data_test_callback_call_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, received_status);
  TEST_ASSERT_EQUAL(0x1223, received_tx_report.transmit_ticks);
  TEST_ASSERT_EQUAL(0x05, received_tx_report.number_of_repeaters);
  TEST_ASSERT_EQUAL((int8_t)0xCF, received_tx_report.ack_rssi);
  TEST_ASSERT_EQUAL(0x7D, received_tx_report.rssi_values.incoming[0]);
  TEST_ASSERT_EQUAL(0x7F, received_tx_report.rssi_values.incoming[1]);
  TEST_ASSERT_EQUAL(0x6F, received_tx_report.rssi_values.incoming[2]);

  // This should be the RSSI_NOT_AVAILABLE value.
  TEST_ASSERT_EQUAL((int8_t)0x7F, received_tx_report.tx_power);
  TEST_ASSERT_EQUAL((int8_t)0x7F, received_tx_report.measured_noise_floor);
  TEST_ASSERT_EQUAL((int8_t)0x7F,
                    received_tx_report.destination_ack_mpdu_tx_power);
  TEST_ASSERT_EQUAL((int8_t)0x7F,
                    received_tx_report.destination_ack_mpdu_measured_rssi);
  TEST_ASSERT_EQUAL(
    (int8_t)0x7F,
    received_tx_report.destination_ack_mpdu_measured_noise_floor);
}

void test_zwapi_protocol_rx_dispatch_test_send_data_callback_no_callback()
{
  // Manually add our callback:
  zwapi_send_data_callback = &zwapi_send_data_test_callback;
  zwave_api_get_callbacks_IgnoreAndReturn(NULL);
  // callbacks is not set, due to lacking init
  uint8_t frame[]
    = {0x1D, 0x00, 0x13, 0x00, 0x00, 0x12, 0x23, 0x05, 0xCF, 0x7D, 0x7F, 0x6F,
       0x7F, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00};
  zwave_api_protocol_rx_dispatch(frame, sizeof(frame));

  TEST_ASSERT_EQUAL(0, zwapi_send_data_test_callback_call_count);
}

void test_zwapi_protocol_rx_dispatch_test_send_data_callback_no_init()
{
  // Manually add our callback:
  zwave_api_get_callbacks_IgnoreAndReturn(&test_zwapi_callbacks);
  uint8_t frame[]
    = {0x1D, 0x00, 0x13, 0x00, 0x00, 0x12, 0x23, 0x05, 0xCF, 0x7D, 0x7F, 0x6F,
       0x7F, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00};
  zwave_api_protocol_rx_dispatch(frame, sizeof(frame));

  TEST_ASSERT_EQUAL(0, zwapi_send_data_test_callback_call_count);
}

void test_zwapi_protocol_rx_dispatch_test_send_nop_callback_tx_report()
{
  // Manually add our callback:
  zwave_api_get_callbacks_IgnoreAndReturn(&test_zwapi_callbacks);
  zwapi_send_nop_callback = &zwapi_send_data_test_callback;
  uint8_t frame[]
    = {0x1D, 0x00, 0xE9, 0x00, 0x00, 0x12, 0x23, 0x05, 0xCF, 0x7D, 0x7F, 0x6F,
       0x7F, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00};
  zwave_api_protocol_rx_dispatch(frame, sizeof(frame));

  TEST_ASSERT_EQUAL(1, zwapi_send_data_test_callback_call_count);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_OK, received_status);
  TEST_ASSERT_EQUAL(0x1223, received_tx_report.transmit_ticks);
  TEST_ASSERT_EQUAL(0x05, received_tx_report.number_of_repeaters);
  TEST_ASSERT_EQUAL((int8_t)0xCF, received_tx_report.ack_rssi);
  TEST_ASSERT_EQUAL(0x7D, received_tx_report.rssi_values.incoming[0]);
  TEST_ASSERT_EQUAL(0x7F, received_tx_report.rssi_values.incoming[1]);
  TEST_ASSERT_EQUAL(0x6F, received_tx_report.rssi_values.incoming[2]);

  // This should be the RSSI_NOT_AVAILABLE value.
  TEST_ASSERT_EQUAL((int8_t)0x7F, received_tx_report.tx_power);
  TEST_ASSERT_EQUAL((int8_t)0x7F, received_tx_report.measured_noise_floor);
  TEST_ASSERT_EQUAL((int8_t)0x7F,
                    received_tx_report.destination_ack_mpdu_tx_power);
  TEST_ASSERT_EQUAL((int8_t)0x7F,
                    received_tx_report.destination_ack_mpdu_measured_rssi);
  TEST_ASSERT_EQUAL(
    (int8_t)0x7F,
    received_tx_report.destination_ack_mpdu_measured_noise_floor);
}

void test_zwapi_protocol_rx_dispatch_test_send_data_bridge_callback_no_tx_report()
{
  // Manually add our callback:
  zwave_api_get_callbacks_IgnoreAndReturn(&test_zwapi_callbacks);
  zwapi_send_data_bridge_callback = &zwapi_send_data_test_callback;
  uint8_t frame[]                 = {0x1D, 0x00, 0xA9, 0x00, 0x10};
  zwave_api_protocol_rx_dispatch(frame, sizeof(frame));

  TEST_ASSERT_EQUAL(1, zwapi_send_data_test_callback_call_count);
  TEST_ASSERT_EQUAL(0x10, received_status);
}

void test_zwapi_protocol_rx_dispatch_unknown_func_id()
{
  // Manually add our callback:
  zwave_api_get_callbacks_IgnoreAndReturn(&test_zwapi_callbacks);
  uint8_t frame[] = {5, 0x00, 0xFF, 0x00, 0x10};
  zwave_api_protocol_rx_dispatch(frame, sizeof(frame));

  // Nothing to expect here
  TEST_ASSERT_EQUAL(0, zwapi_send_data_test_callback_call_count);
}

void test_zwapi_protocol_rx_dispatch_response_frame()
{
  // Manually add our callback:
  zwave_api_get_callbacks_IgnoreAndReturn(&test_zwapi_callbacks);
  zwapi_send_data_bridge_callback = &zwapi_send_data_test_callback;
  uint8_t frame[]                 = {0x1D, 0x01, 0xA9, 0x00, 0x10};
  zwave_api_protocol_rx_dispatch(frame, sizeof(frame));

  TEST_ASSERT_EQUAL(0, zwapi_send_data_test_callback_call_count);
}

void test_zwapi_protocol_rx_dispatch_controller_update_no_callback()
{
  // No callback
  test_zwapi_callbacks.application_controller_update = NULL;
  zwave_api_get_callbacks_IgnoreAndReturn(&test_zwapi_callbacks);

  uint8_t frame[] = {0x0F,
                     0x00,
                     0x49,
                     0x84,
                     0x00,
                     0x02,
                     0x08,
                     0x04,
                     0x18,
                     0x00,
                     0x5E,
                     0x55,
                     0x98,
                     0x9F,
                     0x6C};
  zwave_api_protocol_rx_dispatch(frame, sizeof(frame));

  TEST_ASSERT_EQUAL(
    0,
    zwapi_application_controller_update_test_function_call_count);
}

void test_zwapi_protocol_rx_dispatch_controller_update_smart_start_prime_lr()
{
  // Configure callback
  test_zwapi_callbacks.application_controller_update
    = &zwapi_application_controller_update_test_function;
  zwave_api_get_callbacks_IgnoreAndReturn(&test_zwapi_callbacks);

  uint8_t frame[] = {0x0F,
                     0x00,
                     0x49,
                     0x87,
                     0x00,
                     0x02,
                     0x08,
                     0x04,
                     0x18,
                     0x00,
                     0x5E,
                     0x55,
                     0x98,
                     0x9F,
                     0x6C};
  zwave_api_protocol_rx_dispatch(frame, sizeof(frame));

  TEST_ASSERT_EQUAL(
    1,
    zwapi_application_controller_update_test_function_call_count);
}

void test_zwapi_protocol_rx_dispatch_buffer_overflow()
{
  // Manually add our callback:
  test_zwapi_callbacks.application_controller_update
    = &zwapi_application_controller_update_test_function;
  zwave_api_get_callbacks_IgnoreAndReturn(&test_zwapi_callbacks);

  uint8_t frame[] = {14,
                     0x00,
                     0x49,
                     0x84,
                     0x04,
                     181,  // too long
                     0x04,
                     0x10,
                     0x00,
                     0x5E,
                     0x55,
                     0x98,
                     0x9F,
                     0x6C};
  zwave_api_protocol_rx_dispatch(frame, sizeof(frame));

  TEST_ASSERT_EQUAL(
    0,
    zwapi_application_controller_update_test_function_call_count);
}

void test_zwapi_protocol_rx_dispatch_zwave_api_started()
{
  // Manually add our callback:
  test_zwapi_callbacks.zwapi_started = &zwapi_zwave_api_started_test_function;
  zwave_api_get_callbacks_IgnoreAndReturn(&test_zwapi_callbacks);

  uint8_t frame[]
    = {22,   0x00, 0x0A, 0x03, 0x00, 0x01, 0x01, 0x00, 0x18, 0x5E, 0x55, 0x6C,
       0x74, 0x8A, 0x8F, 0x98, 0x9F, 0xEF, 0x25, 0x26, 0x30, 0x31, 0x32, 0x33,
       0x40, 0x43, 0x5B, 0x60, 0x62, 0x70, 0x71, 0x80, 0x84, 0x00};
  zwapi_set_awaiting_zwave_api_started_Expect(false);
  zwave_api_protocol_rx_dispatch(frame, sizeof(frame));

  TEST_ASSERT_EQUAL(1, zwapi_zwave_api_started_test_function_call_count);
}

void test_zwapi_protocol_rx_dispatch_application_command_handler_bridge()
{
  // Manually add our callback:
  test_zwapi_callbacks.application_command_handler_bridge
    = &zwapi_application_command_handler_test_function;
  zwave_api_get_callbacks_IgnoreAndReturn(&test_zwapi_callbacks);

  uint8_t frame[]
    = {0x1E, 0x00, 0xA8, 0x00, 0x00, 0x01, 0x00, 0x02, 0x10, 0x9F,
       0x03, 0x59, 0x01, 0x02, 0x7F, 0x68, 0x1C, 0x60, 0x4B, 0xE2,
       0x40, 0x58, 0x9B, 0x2C, 0x6D, 0x00, 0xB1, 0x00, 0x7F, 0x7F};
  zwave_api_protocol_rx_dispatch(frame, sizeof(frame));

  TEST_ASSERT_EQUAL(1,
                    zwapi_application_command_handler_test_function_call_count);
}

void test_zwapi_protocol_rx_dispatch_application_command_handler()
{
  // Manually add our callback:
  test_zwapi_callbacks.application_command_handler
    = &zwapi_application_command_handler_test_function;
  zwave_api_get_callbacks_IgnoreAndReturn(&test_zwapi_callbacks);

  uint8_t frame[]
    = {0x1E, 0x00, 0x04, 0x00, 0x00, 0x01, 0x00, 0x02, 0x10, 0x9F,
       0x03, 0x59, 0x01, 0x02, 0x7F, 0x68, 0x1C, 0x60, 0x4B, 0xE2,
       0x40, 0x58, 0x9B, 0x2C, 0x6D, 0x00, 0xB1, 0x00, 0x7F, 0x7F};
  zwave_api_protocol_rx_dispatch(frame, sizeof(frame));

  TEST_ASSERT_EQUAL(1,
                    zwapi_application_command_handler_test_function_call_count);
}

void test_zwapi_protocol_rx_dispatch_inif_received()
{
  // Manually add our callback:
  test_zwapi_callbacks.application_controller_update
    = &zwapi_application_controller_update_test_function;
  zwave_api_get_callbacks_IgnoreAndReturn(&test_zwapi_callbacks);

  uint8_t frame[]
    = {0x0B, 0x00, 0x49, 0x86, 0x02, 0x05, 0x14, 0xEA, 0x15, 0x15, 0x88};
  zwave_api_protocol_rx_dispatch(frame, sizeof(frame));

  TEST_ASSERT_EQUAL(
    1,
    zwapi_application_controller_update_test_function_call_count);

  TEST_ASSERT_EQUAL(0x02, received_node_id);
  TEST_ASSERT_EQUAL(0xEA151588, received_home_id);
}

void test_zwapi_protocol_rx_dispatch_application_controller_unknonwn_status_received()
{
  // Manually add our callback:
  test_zwapi_callbacks.application_controller_update
    = &zwapi_application_controller_update_test_function;
  zwave_api_get_callbacks_IgnoreAndReturn(&test_zwapi_callbacks);

  uint8_t frame[]
    = {0x0B, 0x00, 0x49, 0x90, 0x22, 0x05, 0x14, 0xEA, 0x15, 0x15, 0x88};
  zwave_api_protocol_rx_dispatch(frame, sizeof(frame));

  TEST_ASSERT_EQUAL(
    1,
    zwapi_application_controller_update_test_function_call_count);

  TEST_ASSERT_EQUAL(0x22, received_node_id);
}

void test_zwapi_protocol_rx_dispatch_promiscuous_appplication_handler()
{
  // Manually add our callback:
  test_zwapi_callbacks.application_command_handler
    = &zwapi_application_command_handler_test_function;
  zwave_api_get_callbacks_IgnoreAndReturn(&test_zwapi_callbacks);

  uint8_t frame[]
    = {0x0B, 0x00, 0xD1, 0x86, 0x04, 0x05, 0x14, 0xEA, 0x15, 0x15, 0x88};
  zwave_api_protocol_rx_dispatch(frame, sizeof(frame));

  TEST_ASSERT_EQUAL(1,
                    zwapi_application_command_handler_test_function_call_count);

  TEST_ASSERT_EQUAL(0x04, received_node_id);
}