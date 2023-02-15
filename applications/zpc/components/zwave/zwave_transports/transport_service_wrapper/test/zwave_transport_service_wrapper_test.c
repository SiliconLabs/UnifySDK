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
#include "zwave_transport_service_wrapper.h"

// Mock includes
#include "zwave_controller_internal_mock.h"
#include "zwave_controller_transport_mock.h"
#include "transport_service_mock.h"
#include "ZW_classcmd.h"
#include "zwave_controller_utils_mock.h"
#include "zwave_controller_storage_mock.h"
#include "zwave_tx_scheme_selector_mock.h"
#include "zwave_tx_mock.h"

#define MAX_MSDU_SIZE 34

static zwave_controller_transport_t ts_transport = {};
sl_status_t zwave_controller_transport_register_stub(
  const zwave_controller_transport_t *transport, int cmock_num_calls)
{
  TEST_ASSERT_EQUAL(COMMAND_CLASS_TRANSPORT_SERVICE_V2,
                    transport->command_class);
  TEST_ASSERT_EQUAL(2, transport->version);
  TEST_ASSERT_EQUAL(1, transport->priority);

  // Save the transport for our tests
  ts_transport = *transport;

  return SL_STATUS_OK;
}

upper_layer_command_handler_t upper_layer_command_handler_cb;
send_data_t send_data_cb;
void mytransport_service_init_cb(
  ts_node_id_t my_node_id,
  const upper_layer_command_handler_t upper_layer_command_handler,
  const send_data_t send_data,
  int cmock_num_calls)
{
  upper_layer_command_handler_cb = upper_layer_command_handler;
  send_data_cb                   = send_data;
}

uint8_t data[68] = {0x55, 0xC0, 3, 4, 5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                    1,    2,    3, 4, 5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                    1,    2,    3, 4, 5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                    1,    2,    3, 4, 5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

uint8_t subseq_frag[68]
  = {0x55, 0xE0, 3, 4, 5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
     1,    2,    3, 4, 5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
     1,    2,    3, 4, 5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
     1,    2,    3, 4, 5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

// Keep this test first
void setUp()
{
  // Nothing to test here really. We intercept the registered transport.
  zwave_controller_transport_register_AddCallback(
    zwave_controller_transport_register_stub);

  zwave_controller_transport_register_ExpectAndReturn(NULL, SL_STATUS_OK);
  zwave_controller_transport_register_IgnoreArg_transport();
  transport_service_init_Expect(0, 0, 0);
  transport_service_init_IgnoreArg_my_node_id();
  transport_service_init_IgnoreArg_send_data();
  transport_service_init_IgnoreArg_upper_layer_command_handler();
  transport_service_init_AddCallback(&mytransport_service_init_cb);
  // Call the function
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_transport_service_transport_init());
}

void test_upper_layer_command_handler_cb()
{
  zwave_controller_connection_info_t conn = {};
  zwave_rx_receive_options_t rx_options   = {};

  conn.local.node_id  = 2;  //dest
  conn.remote.node_id = 1;  //src
  conn.encapsulation  = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;

  zwave_controller_on_frame_received_ExpectWithArray(&conn,
                                                     sizeof(conn),
                                                     &rx_options,
                                                     sizeof(rx_options),
                                                     data,
                                                     sizeof(data),
                                                     sizeof(data));
  upper_layer_command_handler_cb(conn.remote.node_id,
                                 conn.local.node_id,
                                 data,
                                 sizeof(data));
}
void test_send_data_cb_subseq_frag()
{
  zwave_controller_connection_info_t conn = {};
  zwave_tx_options_t tx_options           = {};

  conn.local.node_id      = 2;  //dest
  conn.remote.node_id     = 1;  //src
  conn.encapsulation      = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  tx_options.qos_priority = ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY
                            + (ZWAVE_TX_RECOMMENDED_QOS_GAP * 2);
  tx_options.transport.ignore_incoming_frames_back_off = true;

  zwave_tx_send_data_ExpectWithArrayAndReturn(&conn,
                                              sizeof(conn),
                                              sizeof(data),
                                              data,
                                              sizeof(data),
                                              &tx_options,
                                              sizeof(tx_options),
                                              0,
                                              0,
                                              0,
                                              0,
                                              0,
                                              SL_STATUS_OK);
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_on_send_complete();

  TEST_ASSERT_EQUAL(send_data_cb(conn.local.node_id,
                                 conn.remote.node_id,
                                 data,
                                 sizeof(data),
                                 0,
                                 0),
                    0);
}

void test_send_data_fail()
{
  zwave_controller_connection_info_t conn = {};
  zwave_tx_options_t tx_options           = {};

  conn.local.node_id      = 2;  //dest
  conn.remote.node_id     = 1;  //src
  conn.encapsulation      = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  tx_options.qos_priority = ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY
                            + (ZWAVE_TX_RECOMMENDED_QOS_GAP * 2);
  tx_options.number_of_responses                       = 1;
  tx_options.transport.ignore_incoming_frames_back_off = true;

  zwave_tx_send_data_ExpectWithArrayAndReturn(&conn,
                                              sizeof(conn),
                                              sizeof(subseq_frag),
                                              subseq_frag,
                                              sizeof(subseq_frag),
                                              &tx_options,
                                              sizeof(tx_options),
                                              0,
                                              0,
                                              0,
                                              0,
                                              0,
                                              SL_STATUS_FAIL);
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_on_send_complete();

  TEST_ASSERT_EQUAL(send_data_cb(conn.local.node_id,
                                 conn.remote.node_id,
                                 subseq_frag,
                                 sizeof(subseq_frag),
                                 1,
                                 0),
                    1);
}

on_zwave_tx_send_data_complete_t my_on_send_complete_cb;
sl_status_t
  my_tx_send_data_cb(const zwave_controller_connection_info_t *connection,
                     uint16_t data_length,
                     const uint8_t *data,
                     const zwave_tx_options_t *tx_options,
                     const on_zwave_tx_send_data_complete_t on_send_complete,
                     void *user,
                     zwave_tx_session_id_t *session,
                     int cmock_num_calls)
{
  my_on_send_complete_cb = on_send_complete;
  return SL_STATUS_OK;
}

void on_lower_layer_send_data_complete(uint8_t status, void *user)
{
  TEST_ASSERT_EQUAL(status, 0);
}

void test_send_data_cb()
{
  zwave_controller_connection_info_t conn = {};
  zwave_tx_options_t tx_options           = {};

  conn.local.node_id      = 2;  //dest
  conn.remote.node_id     = 1;  //src
  conn.encapsulation      = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  tx_options.qos_priority = ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY
                            + (ZWAVE_TX_RECOMMENDED_QOS_GAP * 2);
  tx_options.number_of_responses                       = 1;
  tx_options.transport.ignore_incoming_frames_back_off = true;

  zwave_tx_send_data_AddCallback(my_tx_send_data_cb);
  zwave_tx_send_data_ExpectWithArrayAndReturn(&conn,
                                              sizeof(conn),
                                              sizeof(subseq_frag),
                                              subseq_frag,
                                              sizeof(subseq_frag),
                                              &tx_options,
                                              sizeof(tx_options),
                                              0,
                                              0,
                                              0,
                                              0,
                                              0,
                                              SL_STATUS_OK);
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_on_send_complete();

  TEST_ASSERT_EQUAL(send_data_cb(conn.local.node_id,
                                 conn.remote.node_id,
                                 subseq_frag,
                                 sizeof(subseq_frag),
                                 1,
                                 on_lower_layer_send_data_complete),
                    0);
  my_on_send_complete_cb(TRANSMIT_COMPLETE_OK, 0, 0);
}

void on_lower_layer_send_data_complete_fail(uint8_t status, void *user)
{
  TEST_ASSERT_EQUAL(status, 1);
}

void test_send_data_cb_fail()
{
  zwave_controller_connection_info_t conn = {};
  zwave_tx_options_t tx_options           = {};

  conn.local.node_id      = 2;  //dest
  conn.remote.node_id     = 1;  //src
  conn.encapsulation      = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  tx_options.qos_priority = ZWAVE_TX_QOS_RECOMMENDED_GET_ANSWER_PRIORITY
                            + (ZWAVE_TX_RECOMMENDED_QOS_GAP * 2);
  tx_options.number_of_responses                       = 1;
  tx_options.transport.ignore_incoming_frames_back_off = true;

  zwave_tx_send_data_AddCallback(my_tx_send_data_cb);
  zwave_tx_send_data_ExpectWithArrayAndReturn(&conn,
                                              sizeof(conn),
                                              sizeof(subseq_frag),
                                              subseq_frag,
                                              sizeof(subseq_frag),
                                              &tx_options,
                                              sizeof(tx_options),
                                              0,
                                              0,
                                              0,
                                              0,
                                              0,
                                              SL_STATUS_OK);
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_on_send_complete();

  TEST_ASSERT_EQUAL(send_data_cb(conn.local.node_id,
                                 conn.remote.node_id,
                                 subseq_frag,
                                 sizeof(subseq_frag),
                                 1,
                                 on_lower_layer_send_data_complete_fail),
                    0);
  my_on_send_complete_cb(TRANSMIT_COMPLETE_FAIL, 0, 0);
}

void test_zwave_transport_service_on_frame_received_singlecast()
{
  zwave_controller_connection_info_t connection_info = {};
  zwave_rx_receive_options_t rx_options              = {};

  transport_service_on_frame_received_ExpectAndReturn(1,
                                                      2,
                                                      SINGLECAST,
                                                      data,
                                                      sizeof(data),
                                                      1);

  connection_info.local.node_id  = 2;
  connection_info.remote.node_id = 1;
  rx_options.status_flags        = RECEIVE_STATUS_TYPE_SINGLE;

  TEST_ASSERT_EQUAL(zwave_transport_service_on_frame_received(&connection_info,
                                                              &rx_options,
                                                              data,
                                                              sizeof(data)),
                    SL_STATUS_OK);
  uint8_t bkup = data[1];
  data[1]      = COMMAND_SEGMENT_COMPLETE_V2;
  transport_service_on_frame_received_ExpectAndReturn(1,
                                                      2,
                                                      SINGLECAST,
                                                      data,
                                                      sizeof(data),
                                                      1);
  TEST_ASSERT_EQUAL(zwave_transport_service_on_frame_received(&connection_info,
                                                              &rx_options,
                                                              data,
                                                              sizeof(data)),
                    SL_STATUS_OK);
  data[1] = COMMAND_SEGMENT_REQUEST_V2;
  transport_service_on_frame_received_ExpectAndReturn(1,
                                                      2,
                                                      SINGLECAST,
                                                      data,
                                                      sizeof(data),
                                                      1);
  TEST_ASSERT_EQUAL(zwave_transport_service_on_frame_received(&connection_info,
                                                              &rx_options,
                                                              data,
                                                              sizeof(data)),
                    SL_STATUS_OK);
  data[1] = COMMAND_SEGMENT_WAIT_V2;
  transport_service_on_frame_received_ExpectAndReturn(1,
                                                      2,
                                                      SINGLECAST,
                                                      data,
                                                      sizeof(data),
                                                      1);
  TEST_ASSERT_EQUAL(zwave_transport_service_on_frame_received(&connection_info,
                                                              &rx_options,
                                                              data,
                                                              sizeof(data)),
                    SL_STATUS_OK);
  data[1] = COMMAND_SUBSEQUENT_SEGMENT_V2;
  transport_service_on_frame_received_ExpectAndReturn(1,
                                                      2,
                                                      SINGLECAST,
                                                      data,
                                                      sizeof(data),
                                                      1);
  TEST_ASSERT_EQUAL(zwave_transport_service_on_frame_received(&connection_info,
                                                              &rx_options,
                                                              data,
                                                              sizeof(data)),
                    SL_STATUS_OK);
  data[1] = 0;
  TEST_ASSERT_EQUAL(zwave_transport_service_on_frame_received(&connection_info,
                                                              &rx_options,
                                                              data,
                                                              sizeof(data)),
                    SL_STATUS_NOT_FOUND);
  data[1] = bkup;
  bkup    = data[0];
  data[0] = 0;
  TEST_ASSERT_EQUAL(zwave_transport_service_on_frame_received(&connection_info,
                                                              &rx_options,
                                                              data,
                                                              sizeof(data)),
                    SL_STATUS_NOT_SUPPORTED);
  data[0] = bkup;
}

void test_zwave_transport_service_on_frame_received_broadcast()
{
  zwave_controller_connection_info_t connection_info = {};
  zwave_rx_receive_options_t rx_options              = {};

  transport_service_on_frame_received_ExpectAndReturn(1,
                                                      2,
                                                      BROADCAST,
                                                      data,
                                                      sizeof(data),
                                                      1);

  connection_info.local.node_id  = 2;
  connection_info.remote.node_id = 1;
  rx_options.status_flags        = RECEIVE_STATUS_TYPE_BROAD;

  TEST_ASSERT_EQUAL(zwave_transport_service_on_frame_received(&connection_info,
                                                              &rx_options,
                                                              data,
                                                              sizeof(data)),
                    SL_STATUS_OK);
}

void test_zwave_transport_service_on_frame_received_multicast()
{
  zwave_controller_connection_info_t connection_info = {};
  zwave_rx_receive_options_t rx_options              = {};

  transport_service_on_frame_received_ExpectAndReturn(1,
                                                      2,
                                                      MULTICAST,
                                                      data,
                                                      sizeof(data),
                                                      true);

  connection_info.local.node_id  = 2;
  connection_info.remote.node_id = 1;
  rx_options.status_flags        = RECEIVE_STATUS_TYPE_MULTI;

  TEST_ASSERT_EQUAL(zwave_transport_service_on_frame_received(&connection_info,
                                                              &rx_options,
                                                              data,
                                                              sizeof(data)),
                    SL_STATUS_OK);
}

void test_zwave_transport_service_on_frame_received_multicast_failure()
{
  zwave_controller_connection_info_t connection_info = {};
  zwave_rx_receive_options_t rx_options              = {};

  transport_service_on_frame_received_ExpectAndReturn(1,
                                                      2,
                                                      MULTICAST,
                                                      data,
                                                      sizeof(data),
                                                      false);

  connection_info.local.node_id  = 2;
  connection_info.remote.node_id = 1;
  rx_options.status_flags        = RECEIVE_STATUS_TYPE_MULTI;

  TEST_ASSERT_EQUAL(zwave_transport_service_on_frame_received(&connection_info,
                                                              &rx_options,
                                                              data,
                                                              sizeof(data)),
                    SL_STATUS_FAIL);
}

void test_zwave_transport_service_send_data()
{
  zwave_controller_connection_info_t connection_info = {};
  zwave_tx_options_t tx_options                      = {};
  connection_info.local.node_id                      = 1;
  connection_info.remote.node_id                     = 2;
  connection_info.remote.endpoint_id                 = 0;

  zwave_tx_scheme_get_max_payload_ExpectAndReturn(
    connection_info.remote.node_id,
    MAX_MSDU_SIZE);
  zwave_controller_storage_is_node_s2_capable_ExpectAndReturn(
    connection_info.remote.node_id,
    true);
  transport_service_send_data_ExpectAndReturn(1,
                                              2,
                                              data,
                                              sizeof(data),
                                              MAX_MSDU_SIZE,
                                              0,
                                              TRANSPORT_SERVICE_SEND_SUCCESS);
  transport_service_send_data_IgnoreArg_on_send_complete();

  TEST_ASSERT_EQUAL(zwave_transport_service_send_data(&connection_info,
                                                      sizeof(data),
                                                      data,
                                                      &tx_options,
                                                      0,
                                                      0,
                                                      0),
                    SL_STATUS_OK);
}

void test_zwave_transport_service_send_data_overflow()
{
  zwave_controller_connection_info_t connection_info = {};
  zwave_tx_options_t tx_options                      = {};

  connection_info.local.node_id      = 1;
  connection_info.remote.node_id     = 2;
  connection_info.remote.endpoint_id = 0;

  zwave_tx_scheme_get_max_payload_ExpectAndReturn(
    connection_info.remote.node_id,
    MAX_MSDU_SIZE);
  zwave_controller_storage_is_node_s2_capable_ExpectAndReturn(
    connection_info.remote.node_id,
    false);
  zwave_controller_storage_get_cc_version_ExpectAndReturn(
    COMMAND_CLASS_TRANSPORT_SERVICE_V2,
    connection_info.remote.node_id,
    connection_info.remote.endpoint_id,
    2);
  transport_service_send_data_ExpectAndReturn(1,
                                              2,
                                              data,
                                              sizeof(data),
                                              MAX_MSDU_SIZE,
                                              0,
                                              TRANSPORT_SERVICE_WILL_OVERFLOW);
  transport_service_send_data_IgnoreArg_on_send_complete();

  TEST_ASSERT_EQUAL(SL_STATUS_WOULD_OVERFLOW,
                    zwave_transport_service_send_data(&connection_info,
                                                      sizeof(data),
                                                      data,
                                                      &tx_options,
                                                      0,
                                                      0,
                                                      0));
}

void test_zwave_transport_service_send_data_fail()
{
  zwave_controller_connection_info_t connection_info = {};
  zwave_tx_options_t tx_options                      = {};

  connection_info.local.node_id      = 1;
  connection_info.remote.node_id     = 2;
  connection_info.remote.endpoint_id = 0;

  zwave_tx_scheme_get_max_payload_ExpectAndReturn(
    connection_info.remote.node_id,
    MAX_MSDU_SIZE);
  zwave_controller_storage_is_node_s2_capable_ExpectAndReturn(
    connection_info.remote.node_id,
    true);
  transport_service_send_data_ExpectAndReturn(1,
                                              2,
                                              data,
                                              sizeof(data),
                                              MAX_MSDU_SIZE,
                                              0,
                                              TRANSPORT_SERVICE_SEND_FAILURE);
  transport_service_send_data_IgnoreArg_on_send_complete();

  TEST_ASSERT_EQUAL(zwave_transport_service_send_data(&connection_info,
                                                      sizeof(data),
                                                      data,
                                                      &tx_options,
                                                      0,
                                                      0,
                                                      0),
                    SL_STATUS_FAIL);
}

void test_zwave_transport_service_send_data_cc_not_supported()
{
  zwave_controller_connection_info_t connection_info = {};
  zwave_tx_options_t tx_options                      = {};

  connection_info.local.node_id  = 1;
  connection_info.remote.node_id = 2;
  zwave_tx_scheme_get_max_payload_ExpectAndReturn(
    connection_info.remote.node_id,
    MAX_MSDU_SIZE);
  zwave_controller_storage_is_node_s2_capable_ExpectAndReturn(
    connection_info.remote.node_id,
    false);
  zwave_controller_storage_get_cc_version_ExpectAndReturn(
    COMMAND_CLASS_TRANSPORT_SERVICE_V2,
    connection_info.remote.node_id,
    connection_info.remote.endpoint_id,
    1);
  TEST_ASSERT_EQUAL(zwave_transport_service_send_data(&connection_info,
                                                      sizeof(data),
                                                      data,
                                                      &tx_options,
                                                      0,
                                                      0,
                                                      0),
                    SL_STATUS_NOT_SUPPORTED);
}

void test_zwave_transport_service_send_data_not_needed()
{
  zwave_controller_connection_info_t connection_info = {};
  zwave_tx_options_t tx_options                      = {};

  connection_info.local.node_id  = 1;
  connection_info.remote.node_id = 2;
  zwave_tx_scheme_get_max_payload_ExpectAndReturn(
    connection_info.remote.node_id,
    MAX_MSDU_SIZE + 1000);
  TEST_ASSERT_EQUAL(zwave_transport_service_send_data(&connection_info,
                                                      sizeof(data),
                                                      data,
                                                      &tx_options,
                                                      0,
                                                      0,
                                                      0),
                    SL_STATUS_NOT_SUPPORTED);
}
