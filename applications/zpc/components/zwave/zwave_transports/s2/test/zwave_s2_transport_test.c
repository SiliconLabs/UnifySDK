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

#include "cmock.h"
#include "contiki_test_helper.h"
#include "process.h"

#include "S2_mock.h"
#include "sl_log.h"
#include "sl_status.h"
#include "zwave_s2_transport.h"
#include "zwapi_protocol_mem_mock.h"
#include "zwave_s2_keystore_int.h"
#include "zwave_s2_internal.h"
#include "zwave_rx.h"
#include "zwave_controller_callbacks.h"
#include "zwave_tx_mock.h"
#include <string.h>

CTR_DRBG_CTX s2_ctr_drbg;

static uint8_t my_status;
static zwapi_tx_report_t my_tx_info;
static void *my_user;
static int num_callbacks = 0;

static zwave_controller_connection_info_t my_connection_info;
static zwave_rx_receive_options_t my_rx_options;
static uint8_t my_frame_data[100];
static uint16_t my_frame_length;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp()
{
  contiki_test_helper_init();
}

void tearDown() {}

// Used to catch the status codes
static void on_zwave_tx_send_data_complete(uint8_t status,
                                           const zwapi_tx_report_t *tx_info,
                                           void *user)
{
  my_status  = status;
  my_tx_info = *tx_info;
  my_user    = user;
  num_callbacks++;
}

void test_zwave_s2_send_data()
{
  zwave_controller_connection_info_t connection = {};

  uint8_t cmd_data[]            = "HelloWorld";
  zwave_tx_options_t tx_options = {};
  zwave_tx_session_id_t session;

  connection.encapsulation  = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS;
  connection.local.node_id  = 1;
  connection.remote.node_id = 2;

  s2_connection_t s2c = {};
  s2c.l_node          = 1;
  s2c.r_node          = 2;
  s2c.zw_tx_options   = 0;
  s2c.class_id        = 2;

  //get_protocol_ExpectAndReturn(s2c.r_node, PROTOCOL_ZWAVE);

  S2_send_data_singlecast_with_keyset_ExpectWithArrayAndReturn(
    0,
    0,
    &s2c,
    sizeof(s2_connection_t),
    UNKNOWN_KEYSET,
    cmd_data,
    sizeof(cmd_data),
    sizeof(cmd_data),
    1);

  //Test without verify delivery
  zwave_tx_get_number_of_responses_ExpectAndReturn(&session, 0);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_s2_send_data(&connection,
                                       sizeof(cmd_data),
                                       cmd_data,
                                       &tx_options,
                                       on_zwave_tx_send_data_complete,
                                       (void *)0x42,
                                       &session));

  //Test with verify delivery
  zwave_tx_get_number_of_responses_ExpectAndReturn(&session, 1);
  s2c.tx_options = S2_TXOPTION_VERIFY_DELIVERY;
  S2_send_data_singlecast_with_keyset_ExpectWithArrayAndReturn(
    0,
    0,
    &s2c,
    sizeof(s2_connection_t),
    UNKNOWN_KEYSET,
    cmd_data,
    sizeof(cmd_data),
    sizeof(cmd_data),
    1);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_s2_send_data(&connection,
                                       sizeof(cmd_data),
                                       cmd_data,
                                       &tx_options,
                                       on_zwave_tx_send_data_complete,
                                       (void *)0x42,
                                       &session));

  //Check the callback
  S2_send_done_event(0, S2_TRANSMIT_COMPLETE_VERIFIED);
  TEST_ASSERT_EQUAL_PTR((void *)0x42, my_user);
  TEST_ASSERT_EQUAL(1, num_callbacks);
  TEST_ASSERT_EQUAL(TRANSMIT_COMPLETE_VERIFIED, my_status);
}

static sl_status_t zwapi_memory_get_buffer_stub(uint16_t offset,
                                                uint8_t *buf,
                                                uint8_t length,
                                                int ncalls)
{
  static nvm_config_t nvm
    = {.magic           = NVM_MAGIC,
       .security_netkey = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
       .assigned_keys   = 0x87,
       .security2_key   = {{2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
                         {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
                         {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4}},
       .ecdh_priv_key
       = {0xab, 0xe,  0xab, 0xe,  0xab, 0xe,  0xab, 0xe,  0xab, 0xe,  0xab,
          0xe,  0xab, 0xe,  0xab, 0xe,  0xab, 0xe,  0xab, 0xe,  0xab, 0xe,
          0xab, 0xe,  0xab, 0xe,  0xab, 0xe,  0xab, 0xe,  0xab, 0xe}};
  const uint8_t *ptr = (const uint8_t *)&nvm;
  memcpy(buf, ptr + offset, length);
  return SL_STATUS_OK;
}

void test_zwave_s2_set_secure_nif()
{
  uint8_t nif[] = "HelloWorld";
  uint8_t *test_nif;
  uint8_t test_nif_length;

  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_s2_set_secure_nif(nif, sizeof(nif)));

  zwapi_memory_get_buffer_Stub(zwapi_memory_get_buffer_stub);
  S2_get_commands_supported(1,
                            2,
                            (const uint8_t **)&test_nif,
                            &test_nif_length);
  TEST_ASSERT_EQUAL(sizeof(nif), test_nif_length);
  TEST_ASSERT_EQUAL_CHAR_ARRAY(nif, test_nif, test_nif_length);
}

void test_zwave_s2_on_frame_received()
{
  zwave_controller_connection_info_t connection = {};

  uint8_t cmd_data[]                    = "\x9fHelloWorld";
  zwave_rx_receive_options_t rx_options = {};
  rx_options.rssi                       = 0x31;
  connection.encapsulation              = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  connection.local.node_id              = 1;
  connection.remote.node_id             = 2;

  s2_connection_t s2c = {};
  s2c.l_node          = 1;
  s2c.r_node          = 2;
  s2c.zw_rx_RSSIval   = 0x31;

  S2_application_command_handler_ExpectWithArray(0,
                                                 0,
                                                 &s2c,
                                                 sizeof(s2_connection_t),
                                                 cmd_data,
                                                 sizeof(cmd_data),
                                                 sizeof(cmd_data));
  zwave_s2_on_frame_received(&connection,
                             &rx_options,
                             cmd_data,
                             sizeof(cmd_data));
}

void my_on_frame_received(
  const zwave_controller_connection_info_t *connection_info,
  const zwave_rx_receive_options_t *rx_options,
  const uint8_t *frame_data,
  uint16_t frame_length)
{
  my_connection_info = *connection_info;
  my_rx_options      = *rx_options;
  memcpy(my_frame_data, frame_data, frame_length);
  my_frame_length = frame_length;
}

void test_S2_msg_received_event()
{
  static zwave_controller_callbacks_t cb
    = {.on_application_frame_received = my_on_frame_received};
  uint8_t cmd_data[] = "HelloWorld";

  s2_connection_t s2c = {};
  s2c.l_node          = 1;
  s2c.r_node          = 2;
  s2c.class_id        = 2;
  s2c.zw_rx_RSSIval   = 0x31;

  zwave_controller_register_callbacks(&cb);

  S2_msg_received_event(0, &s2c, cmd_data, sizeof(cmd_data));

  TEST_ASSERT_EQUAL(1, my_connection_info.local.node_id);
  TEST_ASSERT_EQUAL(2, my_connection_info.remote.node_id);
  TEST_ASSERT_EQUAL(ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS,
                    my_connection_info.encapsulation);
  TEST_ASSERT_EQUAL(sizeof(cmd_data), my_frame_length);
  TEST_ASSERT_EQUAL_CHAR_ARRAY(cmd_data, my_frame_data, sizeof(cmd_data));
}

void test_zwave_s2_abort_send_data()
{
  //Setup S2 frame transmission
  zwave_controller_connection_info_t connection = {};
  uint8_t cmd_data[]                            = "HelloWorld";
  zwave_tx_options_t tx_options                 = {};
  zwave_tx_session_id_t session                 = (void *)23;
  connection.encapsulation  = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS;
  connection.local.node_id  = 1;
  connection.remote.node_id = 2;
  s2_connection_t s2c       = {};
  s2c.l_node                = 1;
  s2c.r_node                = 2;
  s2c.zw_tx_options         = 0;
  s2c.class_id              = 2;

  // Aborting before triggering a tranmission
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND, zwave_s2_abort_send_data(session));

  //get_protocol_ExpectAndReturn(s2c.r_node, PROTOCOL_ZWAVE);
  S2_send_data_singlecast_with_keyset_ExpectWithArrayAndReturn(
    0,
    0,
    &s2c,
    sizeof(s2_connection_t),
    UNKNOWN_KEYSET,
    cmd_data,
    sizeof(cmd_data),
    sizeof(cmd_data),
    1);

  //Test without verify delivery
  zwave_tx_get_number_of_responses_ExpectAndReturn(session, 0);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_s2_send_data(&connection,
                                       sizeof(cmd_data),
                                       cmd_data,
                                       &tx_options,
                                       on_zwave_tx_send_data_complete,
                                       (void *)0x42,
                                       session));

  S2_send_frame_done_notify_Ignore();
  //Abort the tranmission
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_s2_abort_send_data(session));
}
