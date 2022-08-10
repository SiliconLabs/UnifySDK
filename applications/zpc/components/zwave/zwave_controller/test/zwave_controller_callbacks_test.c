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
#include "zwave_controller_callbacks.h"
#include "zwave_controller_transport.h"
#include "zwave_controller_internal.h"
#include "zwave_controller.h"

#include <stdbool.h>

#include "unity.h"
#include "cmock.h"
#include "sl_log.h"

/// Static variables
static zwave_controller_callbacks_t callbacks                   = {};
static int zwave_controller_on_frame_transmission_call_count    = 0;
static int zwave_controller_on_node_event_test_call_count       = 0;
static int zwave_controller_on_rx_frame_received_count          = 0;
static int zwave_controller_on_network_address_update_count     = 0;
static int zwave_controller_on_application_frame_received_count = 0;
static zwave_controller_connection_info_t info                  = {};
static zwave_rx_receive_options_t rx_options                    = {};
const uint8_t frame_data[]                                      = {0x00};
const uint8_t transport_frame_data[]                            = {0xbb};
static sl_status_t transport_return_code                        = SL_STATUS_OK;

// A Z-Wave Controller transport:
static sl_status_t test_transport_on_frame_received(
  const zwave_controller_connection_info_t *connection_info,
  const zwave_rx_receive_options_t *rx_options,
  const uint8_t *incoming_frame_data,
  uint16_t frame_length)
{
  return transport_return_code;
}

static zwave_controller_transport_t test_transport = {
  .command_class     = 0xbb,
  .priority          = 1,
  .version           = 2,
  .send_data         = NULL,
  .on_frame_received = test_transport_on_frame_received,
};

static void zwave_controller_on_frame_transmission_test(
  bool transmission_successful,
  const zwapi_tx_report_t *tx_status,
  zwave_node_id_t node_id)
{
  zwave_controller_on_frame_transmission_call_count += 1;
}

static void
  zwave_controller_on_network_address_update_test(zwave_home_id_t home_id,
                                                  zwave_node_id_t node_id)
{
  zwave_controller_on_network_address_update_count += 1;
}

static void
  zwave_controller_on_rx_frame_received_callback(zwave_node_id_t node_id)
{
  zwave_controller_on_rx_frame_received_count += 1;
}

static void zwave_controller_on_application_frame_received(
  const zwave_controller_connection_info_t *connection_info,
  const zwave_rx_receive_options_t *rx_options,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  zwave_controller_on_application_frame_received_count += 1;
}

static void zwave_controller_on_node_event_test(zwave_node_id_t node_id)
{
  zwave_controller_on_node_event_test_call_count += 1;
}

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_controller_transport_register(&test_transport));
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp()
{
  zwave_controller_on_frame_transmission_call_count    = 0;
  zwave_controller_on_node_event_test_call_count       = 0;
  zwave_controller_on_rx_frame_received_count          = 0;
  zwave_controller_on_network_address_update_count     = 0;
  zwave_controller_on_application_frame_received_count = 0;
  transport_return_code                                = SL_STATUS_OK;
  zwave_controller_callbacks_init();
}

/// Test of callback registration
void test_zwave_controller_callback_registration_wrong_input()
{
  // try to register a NULL pointer callback
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, zwave_controller_register_callbacks(NULL));
  // try to deregister a NULL pointer callback
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_controller_deregister_callbacks(NULL));

  //Try to register too many pointers:
  zwave_controller_callbacks_t
    too_many_callbacks[ZWAVE_CONTROLLER_MAXIMUM_CALLBACK_SUBSCRIBERS + 1];
  for (uint8_t i = 0; i < ZWAVE_CONTROLLER_MAXIMUM_CALLBACK_SUBSCRIBERS; i++) {
    TEST_ASSERT_EQUAL(
      SL_STATUS_OK,
      zwave_controller_register_callbacks(&too_many_callbacks[i]));
  }
  // The next registration must not work
  TEST_ASSERT_EQUAL(
    SL_STATUS_NO_MORE_RESOURCE,
    zwave_controller_register_callbacks(
      &too_many_callbacks[ZWAVE_CONTROLLER_MAXIMUM_CALLBACK_SUBSCRIBERS]));
}

void test_zwave_controller_on_frame_transmission_happy()
{
  zwave_controller_on_frame_transmission(false, NULL, 28);
  TEST_ASSERT_EQUAL(0, zwave_controller_on_frame_transmission_call_count);

  callbacks.on_frame_transmission
    = &zwave_controller_on_frame_transmission_test;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_controller_register_callbacks(&callbacks));

  zwave_controller_on_frame_transmission(false, NULL, 28);
  TEST_ASSERT_EQUAL(1, zwave_controller_on_frame_transmission_call_count);

  zwave_controller_on_frame_transmission(true, NULL, 28);
  TEST_ASSERT_EQUAL(2, zwave_controller_on_frame_transmission_call_count);

  callbacks.on_frame_transmission = NULL;
}

void test_zwave_controller_on_node_deleted_happy()
{
  zwave_controller_on_node_deleted(28);
  TEST_ASSERT_EQUAL(0, zwave_controller_on_node_event_test_call_count);

  callbacks.on_node_deleted = &zwave_controller_on_node_event_test;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_controller_register_callbacks(&callbacks));

  zwave_controller_on_node_deleted(28);
  TEST_ASSERT_EQUAL(1, zwave_controller_on_node_event_test_call_count);

  zwave_controller_on_node_deleted(0);
  TEST_ASSERT_EQUAL(2, zwave_controller_on_node_event_test_call_count);

  callbacks.on_node_deleted = NULL;
}

void test_zwave_controller_on_controller_new_suc_happy()
{
  zwave_controller_on_new_suc(28);
  TEST_ASSERT_EQUAL(0, zwave_controller_on_node_event_test_call_count);

  callbacks.on_new_suc = &zwave_controller_on_node_event_test;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_controller_register_callbacks(&callbacks));

  zwave_controller_on_new_suc(28);
  TEST_ASSERT_EQUAL(1, zwave_controller_on_node_event_test_call_count);

  zwave_controller_on_new_suc(0);
  TEST_ASSERT_EQUAL(2, zwave_controller_on_node_event_test_call_count);

  callbacks.on_new_suc = NULL;
}

void test_zwave_controller_on_controller_rx_frames()
{
  // Rx frame is ignored without callbacks.
  zwave_controller_on_frame_reception(20);
  TEST_ASSERT_EQUAL(0, zwave_controller_on_rx_frame_received_count);

  callbacks.on_rx_frame_received
    = &zwave_controller_on_rx_frame_received_callback;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_controller_register_callbacks(&callbacks));

  zwave_controller_on_frame_reception(20);
  TEST_ASSERT_EQUAL(1, zwave_controller_on_rx_frame_received_count);

  zwave_controller_on_frame_reception(20);
  TEST_ASSERT_EQUAL(0, zwave_controller_on_application_frame_received_count);
  TEST_ASSERT_EQUAL(2, zwave_controller_on_rx_frame_received_count);

  // remove callbacks:
  callbacks.on_rx_frame_received = NULL;
}

void test_zwave_controller_on_controller_application_frames()
{
  // We need to inject a valid frame, as this will go through the transports
  zwave_controller_on_frame_received(&info,
                                     &rx_options,
                                     frame_data,
                                     sizeof(frame_data));
  TEST_ASSERT_EQUAL(0, zwave_controller_on_rx_frame_received_count);
  TEST_ASSERT_EQUAL(0, zwave_controller_on_application_frame_received_count);

  callbacks.on_rx_frame_received
    = &zwave_controller_on_rx_frame_received_callback;
  callbacks.on_application_frame_received
    = &zwave_controller_on_application_frame_received;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_controller_register_callbacks(&callbacks));

  zwave_controller_on_frame_received(&info,
                                     &rx_options,
                                     frame_data,
                                     sizeof(frame_data));
  TEST_ASSERT_EQUAL(1, zwave_controller_on_application_frame_received_count);
  TEST_ASSERT_EQUAL(0, zwave_controller_on_rx_frame_received_count);

  zwave_controller_on_frame_received(&info,
                                     &rx_options,
                                     frame_data,
                                     sizeof(frame_data));
  TEST_ASSERT_EQUAL(2, zwave_controller_on_application_frame_received_count);
  TEST_ASSERT_EQUAL(0, zwave_controller_on_rx_frame_received_count);

  // Try a frame absorbed by transports
  zwave_controller_on_frame_received(&info,
                                     &rx_options,
                                     transport_frame_data,
                                     sizeof(transport_frame_data));
  TEST_ASSERT_EQUAL(2, zwave_controller_on_application_frame_received_count);
  TEST_ASSERT_EQUAL(0, zwave_controller_on_rx_frame_received_count);

  // Try again, with the transport frame rejected by the transport.
  // Then it goes to the application layer
  transport_return_code = SL_STATUS_NOT_FOUND;
  zwave_controller_on_frame_received(&info,
                                     &rx_options,
                                     transport_frame_data,
                                     sizeof(transport_frame_data));
  TEST_ASSERT_EQUAL(3, zwave_controller_on_application_frame_received_count);
  TEST_ASSERT_EQUAL(0, zwave_controller_on_rx_frame_received_count);

  // remove these callbacks:
  callbacks.on_rx_frame_received          = NULL;
  callbacks.on_application_frame_received = NULL;
}

void test_zwave_controller_on_network_address_update()
{
  // Nothing happens with no callback registered
  zwave_controller_on_network_address_update(1, 1);
  TEST_ASSERT_EQUAL(0, zwave_controller_on_network_address_update_count);

  callbacks.on_network_address_update
    = &zwave_controller_on_network_address_update_test;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_controller_register_callbacks(&callbacks));

  zwave_controller_on_network_address_update(2, 2);

  TEST_ASSERT_EQUAL(1, zwave_controller_on_network_address_update_count);
}