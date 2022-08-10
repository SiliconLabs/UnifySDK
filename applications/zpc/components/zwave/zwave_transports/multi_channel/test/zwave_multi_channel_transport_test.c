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
#include "zwave_multi_channel_transport.h"

// Generic includes
#include <string.h>

// Test includes
#include "unity.h"

// Includes from other components
#include "ZW_classcmd.h"

// Mocks
#include "zwave_controller_transport_mock.h"
#include "zwave_controller_internal_mock.h"
#include "zwave_tx_mock.h"

// Static variables
static zwave_controller_transport_t multi_channel_transport             = {};
static zwave_controller_connection_info_t last_received_connection_info = {};
static zwave_tx_options_t last_received_tx_options                      = {};
static zwave_rx_receive_options_t last_received_rx_options              = {};
static uint8_t last_received_frame[ZWAVE_MAX_FRAME_SIZE]                = {};
static uint8_t last_received_frame_length                               = 0;
static on_zwave_tx_send_data_complete_t multi_channel_on_send_complete  = NULL;
static void *last_received_user_pointer                                 = NULL;
static uint8_t send_data_callback_counter                               = 0;
static uint8_t my_user_variable                                         = 0x93;

sl_status_t zwave_controller_transport_register_stub(
  const zwave_controller_transport_t *transport, int cmock_num_calls)
{
  TEST_ASSERT_EQUAL(COMMAND_CLASS_MULTI_CHANNEL_V4, transport->command_class);
  TEST_ASSERT_EQUAL(MULTI_CHANNEL_VERSION, transport->version);
  TEST_ASSERT_EQUAL(5, transport->priority);

  // Save the transport for our tests
  multi_channel_transport = *transport;

  return SL_STATUS_OK;
}

void zwave_controller_on_frame_received_stub(
  const zwave_controller_connection_info_t *connection_info,
  const zwave_rx_receive_options_t *rx_options,
  const uint8_t *frame_data,
  uint16_t frame_length,
  int cmock_num_calls)
{
  // Save the received data for test verification
  last_received_connection_info = *connection_info;
  last_received_rx_options      = *rx_options;
  memcpy(last_received_frame, frame_data, frame_length);
  last_received_frame_length = frame_length;
}

sl_status_t zwave_tx_send_data_stub(
  const zwave_controller_connection_info_t *connection,
  uint16_t data_length,
  const uint8_t *data,
  const zwave_tx_options_t *tx_options,
  const on_zwave_tx_send_data_complete_t on_send_complete,
  void *user,
  zwave_tx_session_id_t *session,
  int cmock_num_calls)
{
  // Save the received data for test verification
  last_received_connection_info  = *connection;
  last_received_tx_options       = *tx_options;
  multi_channel_on_send_complete = on_send_complete;
  last_received_user_pointer     = user;
  memcpy(last_received_frame, data, data_length);
  last_received_frame_length = data_length;

  TEST_ASSERT_EQUAL_PTR(NULL, session);
  return SL_STATUS_OK;
}

static void test_send_data_callback(uint8_t status,
                                    const zwapi_tx_report_t *tx_info,
                                    void *user)
{
  send_data_callback_counter += 1;
  TEST_ASSERT_EQUAL_PTR(&my_user_variable, user);
  TEST_ASSERT_EQUAL_PTR(NULL, tx_info);
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
  send_data_callback_counter     = 0;
  multi_channel_on_send_complete = NULL;
  last_received_user_pointer     = NULL;
}

// Keep this test first
void test_zwave_multi_channel_transport_init()
{
  // Nothing to test here really. We intercept the registered transport.
  zwave_controller_transport_register_AddCallback(
    zwave_controller_transport_register_stub);

  zwave_controller_transport_register_ExpectAndReturn(NULL, SL_STATUS_OK);
  zwave_controller_transport_register_IgnoreArg_transport();
  // Call the function
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_multi_channel_transport_init());
}

void test_decapsulation_happy_case()
{
  TEST_ASSERT_NOT_NULL(multi_channel_transport.on_frame_received);

  zwave_controller_connection_info_t connection_info = {};
  zwave_rx_receive_options_t rx_options              = {};
  const uint8_t frame_data[] = {COMMAND_CLASS_MULTI_CHANNEL_V4,
                                MULTI_CHANNEL_CMD_ENCAP_V4,
                                0x01,
                                0x02,
                                0x03,
                                0x04,
                                0x05};

  zwave_controller_on_frame_received_Stub(
    zwave_controller_on_frame_received_stub);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    multi_channel_transport.on_frame_received(&connection_info,
                                              &rx_options,
                                              frame_data,
                                              sizeof(frame_data)));

  // Verify that our decapsulation worked
  TEST_ASSERT_EQUAL(0x01, last_received_connection_info.remote.endpoint_id);
  TEST_ASSERT_EQUAL(0x02, last_received_connection_info.local.endpoint_id);
  TEST_ASSERT_EQUAL(false, last_received_connection_info.local.is_multicast);
  TEST_ASSERT_EQUAL(3, last_received_frame_length);
  const uint8_t expected_frame_data[] = {0x03, 0x04, 0x05};
  TEST_ASSERT_EQUAL_INT8_ARRAY(expected_frame_data,
                               last_received_frame,
                               last_received_frame_length);
}

void test_decapsulation_overflow()
{
  //zwave_command_class_multi_channel_transport_init_verification();
  // Test with wrong command
  TEST_ASSERT_NOT_NULL(multi_channel_transport.on_frame_received);

  zwave_controller_connection_info_t connection_info = {};
  zwave_rx_receive_options_t rx_options              = {};
  const uint8_t frame_data[] = {COMMAND_CLASS_MULTI_CHANNEL_V4,
                                MULTI_CHANNEL_CMD_ENCAP_V4,
                                0x01,
                                0x02,
                                0x03,
                                0x04,
                                0x05};

  TEST_ASSERT_EQUAL(
    SL_STATUS_WOULD_OVERFLOW,
    multi_channel_transport.on_frame_received(
      &connection_info,
      &rx_options,
      frame_data,
      ZWAVE_MAX_FRAME_SIZE + MULTI_CHANNEL_ENCAPSULATION_OVERHEAD + 1));
}

void test_decapsulation_too_short_frame()
{
  //zwave_command_class_multi_channel_transport_init_verification();
  // Test with wrong command
  TEST_ASSERT_NOT_NULL(multi_channel_transport.on_frame_received);

  zwave_controller_connection_info_t connection_info = {};
  zwave_rx_receive_options_t rx_options              = {};
  const uint8_t frame_data[] = {COMMAND_CLASS_MULTI_CHANNEL_V4,
                                MULTI_CHANNEL_CMD_ENCAP_V4,
                                0x01,
                                0x02,
                                0x03,
                                0x04,
                                0x05};

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    multi_channel_transport.on_frame_received(&connection_info,
                                                              &rx_options,
                                                              frame_data,
                                                              4));
}

void test_decapsulation_too_wrong_command()
{
  TEST_ASSERT_NOT_NULL(multi_channel_transport.on_frame_received);

  zwave_controller_connection_info_t connection_info = {};
  zwave_rx_receive_options_t rx_options              = {};
  const uint8_t frame_data[] = {COMMAND_CLASS_MULTI_CHANNEL_V4,
                                MULTI_CHANNEL_CMD_ENCAP_V4 + 1,
                                0x01,
                                0x02,
                                0x03,
                                0x04,
                                0x05};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_FOUND,
    multi_channel_transport.on_frame_received(&connection_info,
                                              &rx_options,
                                              frame_data,
                                              sizeof(frame_data)));
}

void test_decapsulation_too_wrong_command_class()
{
  TEST_ASSERT_NOT_NULL(multi_channel_transport.on_frame_received);

  zwave_controller_connection_info_t connection_info = {};
  zwave_rx_receive_options_t rx_options              = {};
  const uint8_t frame_data[] = {COMMAND_CLASS_MULTI_CHANNEL_V4 + 1,
                                MULTI_CHANNEL_CMD_ENCAP_V4,
                                0x01,
                                0x02,
                                0x03,
                                0x04,
                                0x05};

  TEST_ASSERT_EQUAL(
    SL_STATUS_NOT_FOUND,
    multi_channel_transport.on_frame_received(&connection_info,
                                              &rx_options,
                                              frame_data,
                                              sizeof(frame_data)));
}

void test_decapsulation_bit_address()
{
  TEST_ASSERT_NOT_NULL(multi_channel_transport.on_frame_received);

  zwave_controller_connection_info_t connection_info = {};
  zwave_rx_receive_options_t rx_options              = {};
  const uint8_t frame_data[] = {COMMAND_CLASS_MULTI_CHANNEL_V4,
                                MULTI_CHANNEL_CMD_ENCAP_V4,
                                0xFF,
                                0xFF,
                                0x03,
                                0x04,
                                0x05};

  zwave_controller_on_frame_received_Stub(
    zwave_controller_on_frame_received_stub);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    multi_channel_transport.on_frame_received(&connection_info,
                                              &rx_options,
                                              frame_data,
                                              sizeof(frame_data)));

  // Verify that our decapsulation worked
  TEST_ASSERT_EQUAL(0x7F, last_received_connection_info.remote.endpoint_id);
  TEST_ASSERT_EQUAL(0xFF, last_received_connection_info.local.endpoint_id);
  TEST_ASSERT_EQUAL(true, last_received_connection_info.local.is_multicast);
  TEST_ASSERT_EQUAL(3, last_received_frame_length);
  const uint8_t expected_frame_data[] = {0x03, 0x04, 0x05};
  TEST_ASSERT_EQUAL_INT8_ARRAY(expected_frame_data,
                               last_received_frame,
                               last_received_frame_length);
}

void test_encapsulation_happy_case()
{
  TEST_ASSERT_NOT_NULL(multi_channel_transport.send_data);

  zwave_controller_connection_info_t connection_info = {};
  zwave_tx_options_t tx_options                      = {};
  const uint8_t frame_data[]                         = {0x03, 0x04, 0x05};
  connection_info.remote.endpoint_id                 = 2;
  connection_info.local.endpoint_id                  = 1;
  zwave_tx_session_id_t parent_session_id            = (void *)23;

  zwave_tx_send_data_Stub(zwave_tx_send_data_stub);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    multi_channel_transport.send_data(&connection_info,
                                                      sizeof(frame_data),
                                                      frame_data,
                                                      &tx_options,
                                                      test_send_data_callback,
                                                      &my_user_variable,
                                                      parent_session_id));

  // Verify that our decapsulation worked
  TEST_ASSERT_EQUAL(0, last_received_connection_info.remote.endpoint_id);
  TEST_ASSERT_EQUAL(0, last_received_connection_info.local.endpoint_id);
  TEST_ASSERT_TRUE(last_received_tx_options.transport.valid_parent_session_id);
  TEST_ASSERT_EQUAL_PTR(parent_session_id,
                        last_received_tx_options.transport.parent_session_id);
  const uint8_t expected_frame_data[] = {COMMAND_CLASS_MULTI_CHANNEL_V4,
                                         MULTI_CHANNEL_CMD_ENCAP_V4,
                                         0x01,
                                         0x02,
                                         0x03,
                                         0x04,
                                         0x05};
  TEST_ASSERT_EQUAL(sizeof(expected_frame_data), last_received_frame_length);
  TEST_ASSERT_EQUAL_INT8_ARRAY(expected_frame_data,
                               last_received_frame,
                               last_received_frame_length);

  TEST_ASSERT_EQUAL(0, send_data_callback_counter);

  TEST_ASSERT_NOT_NULL(multi_channel_on_send_complete);
  multi_channel_on_send_complete(TRANSMIT_COMPLETE_OK,
                                 NULL,
                                 last_received_user_pointer);

  TEST_ASSERT_EQUAL(1, send_data_callback_counter);
}

void test_encapsulation_overflow()
{
  TEST_ASSERT_NOT_NULL(multi_channel_transport.send_data);

  zwave_controller_connection_info_t connection_info = {};
  connection_info.remote.endpoint_id                 = 2;
  connection_info.local.endpoint_id                  = 1;
  const uint8_t frame_data[]                         = {0x03, 0x04, 0x05};

  TEST_ASSERT_EQUAL(SL_STATUS_WOULD_OVERFLOW,
                    multi_channel_transport.send_data(
                      &connection_info,
                      MULTI_CHANNEL_ENCAPSULATED_COMMAND_MAXIMUM_SIZE + 1,
                      frame_data,
                      NULL,
                      NULL,
                      NULL,
                      NULL));

  TEST_ASSERT_EQUAL(0, send_data_callback_counter);
}

void test_encapsulation_multi_channel_capability_get()
{
  TEST_ASSERT_NOT_NULL(multi_channel_transport.send_data);

  zwave_controller_connection_info_t connection_info = {};
  zwave_tx_options_t tx_options                      = {};
  const uint8_t frame_data[]
    = {COMMAND_CLASS_MULTI_CHANNEL_V4, MULTI_CHANNEL_CAPABILITY_GET_V4, 0x05};
  connection_info.remote.endpoint_id      = 2;
  connection_info.local.endpoint_id       = 1;
  zwave_tx_session_id_t parent_session_id = (void *)23;

  zwave_tx_send_data_Stub(zwave_tx_send_data_stub);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    multi_channel_transport.send_data(&connection_info,
                                                      sizeof(frame_data),
                                                      frame_data,
                                                      &tx_options,
                                                      test_send_data_callback,
                                                      &my_user_variable,
                                                      parent_session_id));

  // Verify that our decapsulation worked
  TEST_ASSERT_EQUAL(0, last_received_connection_info.remote.endpoint_id);
  TEST_ASSERT_EQUAL(0, last_received_connection_info.local.endpoint_id);
  TEST_ASSERT_TRUE(last_received_tx_options.transport.valid_parent_session_id);
  TEST_ASSERT_EQUAL_PTR(parent_session_id,
                        last_received_tx_options.transport.parent_session_id);
  const uint8_t expected_frame_data[]
    = {COMMAND_CLASS_MULTI_CHANNEL_V4,
       MULTI_CHANNEL_CMD_ENCAP_V4,
       0x01,
       0x00,  // Expect 0 here because of the exception.
       COMMAND_CLASS_MULTI_CHANNEL_V4,
       MULTI_CHANNEL_CAPABILITY_GET_V4,
       0x05};
  TEST_ASSERT_EQUAL(sizeof(expected_frame_data), last_received_frame_length);
  TEST_ASSERT_EQUAL_INT8_ARRAY(expected_frame_data,
                               last_received_frame,
                               last_received_frame_length);

  TEST_ASSERT_EQUAL(0, send_data_callback_counter);

  TEST_ASSERT_NOT_NULL(multi_channel_on_send_complete);
  multi_channel_on_send_complete(TRANSMIT_COMPLETE_OK,
                                 NULL,
                                 last_received_user_pointer);

  TEST_ASSERT_EQUAL(1, send_data_callback_counter);
}

void test_encapsulation_multi_channel_aggregated_member_get()
{
  TEST_ASSERT_NOT_NULL(multi_channel_transport.send_data);

  zwave_controller_connection_info_t connection_info = {};
  zwave_tx_options_t tx_options                      = {};
  const uint8_t frame_data[]              = {COMMAND_CLASS_MULTI_CHANNEL_V4,
                                MULTI_CHANNEL_AGGREGATED_MEMBERS_GET_V4,
                                0x05};
  connection_info.remote.endpoint_id      = 5;
  connection_info.local.endpoint_id       = 1;
  zwave_tx_session_id_t parent_session_id = (void *)23;

  zwave_tx_send_data_Stub(zwave_tx_send_data_stub);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    multi_channel_transport.send_data(&connection_info,
                                                      sizeof(frame_data),
                                                      frame_data,
                                                      &tx_options,
                                                      test_send_data_callback,
                                                      &my_user_variable,
                                                      parent_session_id));

  // Verify that our decapsulation worked
  TEST_ASSERT_EQUAL(0, last_received_connection_info.remote.endpoint_id);
  TEST_ASSERT_EQUAL(0, last_received_connection_info.local.endpoint_id);
  TEST_ASSERT_TRUE(last_received_tx_options.transport.valid_parent_session_id);
  TEST_ASSERT_EQUAL_PTR(parent_session_id,
                        last_received_tx_options.transport.parent_session_id);
  const uint8_t expected_frame_data[]
    = {COMMAND_CLASS_MULTI_CHANNEL_V4,
       MULTI_CHANNEL_CMD_ENCAP_V4,
       0x01,
       0x00,  // Expect 0 here because of the exception.
       COMMAND_CLASS_MULTI_CHANNEL_V4,
       MULTI_CHANNEL_AGGREGATED_MEMBERS_GET_V4,
       0x05};
  TEST_ASSERT_EQUAL(sizeof(expected_frame_data), last_received_frame_length);
  TEST_ASSERT_EQUAL_INT8_ARRAY(expected_frame_data,
                               last_received_frame,
                               last_received_frame_length);

  TEST_ASSERT_EQUAL(0, send_data_callback_counter);

  TEST_ASSERT_NOT_NULL(multi_channel_on_send_complete);
  multi_channel_on_send_complete(TRANSMIT_COMPLETE_OK,
                                 NULL,
                                 last_received_user_pointer);

  TEST_ASSERT_EQUAL(1, send_data_callback_counter);
}

void test_encapsulation_multi_channel_version_command_class_get()
{
  TEST_ASSERT_NOT_NULL(multi_channel_transport.send_data);

  zwave_controller_connection_info_t connection_info = {};
  zwave_tx_options_t tx_options                      = {};
  const uint8_t frame_data[]
    = {COMMAND_CLASS_VERSION_V3, VERSION_COMMAND_CLASS_GET_V3, 0x25};
  connection_info.remote.endpoint_id      = 5;
  connection_info.local.endpoint_id       = 0;
  zwave_tx_session_id_t parent_session_id = (void *)23;

  zwave_tx_send_data_Stub(zwave_tx_send_data_stub);

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    multi_channel_transport.send_data(&connection_info,
                                                      sizeof(frame_data),
                                                      frame_data,
                                                      &tx_options,
                                                      test_send_data_callback,
                                                      &my_user_variable,
                                                      parent_session_id));

  TEST_ASSERT_EQUAL(0, send_data_callback_counter);
}

void test_encapsulation_no_endpoints()
{
  TEST_ASSERT_NOT_NULL(multi_channel_transport.send_data);

  zwave_controller_connection_info_t connection_info = {};
  zwave_tx_options_t tx_options                      = {};
  const uint8_t frame_data[]                         = {0x03, 0x04, 0x05};
  connection_info.remote.endpoint_id                 = 0;
  connection_info.local.endpoint_id                  = 0;
  zwave_tx_session_id_t parent_session_id            = (void *)23;

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_SUPPORTED,
                    multi_channel_transport.send_data(&connection_info,
                                                      sizeof(frame_data),
                                                      frame_data,
                                                      &tx_options,
                                                      test_send_data_callback,
                                                      &my_user_variable,
                                                      parent_session_id));

  TEST_ASSERT_EQUAL(0, send_data_callback_counter);
}

void test_encapsulation_session_max_out()
{
  TEST_ASSERT_NOT_NULL(multi_channel_transport.send_data);

  zwave_controller_connection_info_t connection_info = {};
  zwave_tx_options_t tx_options                      = {};
  const uint8_t frame_data[]                         = {0x03, 0x04, 0x05};
  connection_info.remote.endpoint_id                 = 2;
  connection_info.local.endpoint_id                  = 1;
  zwave_tx_session_id_t parent_session_id            = (void *)23;

  zwave_tx_send_data_Stub(zwave_tx_send_data_stub);

  // First attempt
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    multi_channel_transport.send_data(&connection_info,
                                                      sizeof(frame_data),
                                                      frame_data,
                                                      &tx_options,
                                                      test_send_data_callback,
                                                      &my_user_variable,
                                                      parent_session_id));

  // Verify that our decapsulation worked
  TEST_ASSERT_EQUAL(0, last_received_connection_info.remote.endpoint_id);
  TEST_ASSERT_EQUAL(0, last_received_connection_info.local.endpoint_id);
  TEST_ASSERT_TRUE(last_received_tx_options.transport.valid_parent_session_id);
  TEST_ASSERT_EQUAL_PTR(parent_session_id,
                        last_received_tx_options.transport.parent_session_id);
  const uint8_t expected_frame_data[] = {COMMAND_CLASS_MULTI_CHANNEL_V4,
                                         MULTI_CHANNEL_CMD_ENCAP_V4,
                                         0x01,
                                         0x02,
                                         0x03,
                                         0x04,
                                         0x05};
  TEST_ASSERT_EQUAL(sizeof(expected_frame_data), last_received_frame_length);
  TEST_ASSERT_EQUAL_INT8_ARRAY(expected_frame_data,
                               last_received_frame,
                               last_received_frame_length);

  TEST_ASSERT_EQUAL(0, send_data_callback_counter);

  // We haven't told from the Z-Wave TX Side that those transmissions are completed.
  // Now we expect a rejection
  TEST_ASSERT_EQUAL(SL_STATUS_BUSY,
                    multi_channel_transport.send_data(&connection_info,
                                                      sizeof(frame_data),
                                                      frame_data,
                                                      &tx_options,
                                                      test_send_data_callback,
                                                      &my_user_variable,
                                                      parent_session_id));

  TEST_ASSERT_EQUAL(0, send_data_callback_counter);
  TEST_ASSERT_NOT_NULL(multi_channel_on_send_complete);
  multi_channel_on_send_complete(TRANSMIT_COMPLETE_OK,
                                 NULL,
                                 last_received_user_pointer);

  // We only saved the last received user pointer, so it will finish only 1
  // of the multi channel transport sessions
  TEST_ASSERT_EQUAL(1, send_data_callback_counter);
}

void test_abort_send_data()
{
  TEST_ASSERT_NOT_NULL(multi_channel_transport.send_data);
  TEST_ASSERT_NOT_NULL(multi_channel_transport.abort_send_data);

  zwave_controller_connection_info_t connection_info = {};
  zwave_tx_options_t tx_options                      = {};
  const uint8_t frame_data[]                         = {0x03, 0x04, 0x05};
  connection_info.remote.endpoint_id                 = 2;
  connection_info.local.endpoint_id                  = 1;
  zwave_tx_session_id_t parent_session_id            = (void *)23;

  // Try to abort now, nothing will happen
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    multi_channel_transport.abort_send_data(parent_session_id));

  zwave_tx_send_data_Stub(zwave_tx_send_data_stub);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    multi_channel_transport.send_data(&connection_info,
                                                      sizeof(frame_data),
                                                      frame_data,
                                                      &tx_options,
                                                      test_send_data_callback,
                                                      &my_user_variable,
                                                      parent_session_id));

  // Verify that our decapsulation worked
  TEST_ASSERT_EQUAL(0, last_received_connection_info.remote.endpoint_id);
  TEST_ASSERT_EQUAL(0, last_received_connection_info.local.endpoint_id);
  TEST_ASSERT_TRUE(last_received_tx_options.transport.valid_parent_session_id);
  TEST_ASSERT_EQUAL_PTR(parent_session_id,
                        last_received_tx_options.transport.parent_session_id);
  const uint8_t expected_frame_data[] = {COMMAND_CLASS_MULTI_CHANNEL_V4,
                                         MULTI_CHANNEL_CMD_ENCAP_V4,
                                         0x01,
                                         0x02,
                                         0x03,
                                         0x04,
                                         0x05};
  TEST_ASSERT_EQUAL(sizeof(expected_frame_data), last_received_frame_length);
  TEST_ASSERT_EQUAL_INT8_ARRAY(expected_frame_data,
                               last_received_frame,
                               last_received_frame_length);

  TEST_ASSERT_EQUAL(0, send_data_callback_counter);

  // Try to abort now, it will trigger a callback
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    multi_channel_transport.abort_send_data(parent_session_id));

  TEST_ASSERT_EQUAL(1, send_data_callback_counter);
}
