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
#include "unity.h"
#include "zwave_controller_transport.h"
#include "zwave_controller_transport_test_callbacks_mock.h"
#include "zwave_controller_internal.h"
#include "zwave_tx_mock.h"
/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

#define COMMAND_CLASS_VERSION           0x86
#define VERSION_COMMAND_CLASS_GET_V2    0x13
#define VERSION_COMMAND_CLASS_REPORT_V2 0x14

zwave_controller_transport_t t1 = {
  .command_class     = 0xaa,
  .priority          = 2,
  .version           = 0x22,
  .send_data         = send_data1,
  .on_frame_received = on_frame_received1,
};
zwave_controller_transport_t t2 = {
  .command_class     = 0xbb,
  .priority          = 1,
  .version           = 2,
  .send_data         = send_data2,
  .on_frame_received = on_frame_received2,
  .abort_send_data   = abort_send_data2,
};
zwave_controller_callbacks_t c
  = {.on_application_frame_received = on_frame_received3};

const uint8_t data1[] = {0xaa, 1, 2, 3};
const uint8_t data2[] = {0xbb, 1, 2, 3};
const uint8_t data3[] = {0xcc, 1, 2, 3};

/// Called before each and every test
void setUp()
{
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_controller_transport_init());
}

void test_zwave_controller_transport_test()
{
  zwave_controller_register_callbacks(&c);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_controller_transport_register(&t1));
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_controller_transport_register(&t2));
  //Test that we cannot register the same transport twice
  TEST_ASSERT_NOT_EQUAL(SL_STATUS_OK, zwave_controller_transport_register(&t2));

  //Test reception
  zwave_rx_receive_options_t opt          = {};
  zwave_controller_connection_info_t conn = {};

  //Test plugin 1
  on_frame_received1_ExpectAndReturn(&conn,
                                     &opt,
                                     data1,
                                     sizeof(data1),
                                     SL_STATUS_OK);
  zwave_controller_on_frame_received(&conn, &opt, data1, sizeof(data1));

  //Test plugin 2
  on_frame_received2_ExpectAndReturn(&conn,
                                     &opt,
                                     data2,
                                     sizeof(data2),
                                     SL_STATUS_OK);
  zwave_controller_on_frame_received(&conn, &opt, data2, sizeof(data2));

  //Test forward to upper layer
  on_frame_received3_Expect(&conn, &opt, data3, sizeof(data3));
  zwave_controller_on_frame_received(&conn, &opt, data3, sizeof(data3));

  // Test sending
  zwave_tx_options_t tx_opt = {};

  //Expect send_data1 to be tried first as it has the highest priority number
  send_data1_ExpectAndReturn(&conn,
                             sizeof(data1),
                             data1,
                             &tx_opt,
                             0,
                             0,
                             0,
                             SL_STATUS_OK);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_controller_transport_send_data(&conn,
                                                         sizeof(data1),
                                                         data1,
                                                         &tx_opt,
                                                         0,
                                                         0,
                                                         0));

  //Test that if send_data1 reject the frame the we try with the next plugin
  send_data1_ExpectAndReturn(&conn,
                             sizeof(data1),
                             data1,
                             &tx_opt,
                             0,
                             0,
                             0,
                             SL_STATUS_NOT_SUPPORTED);
  send_data2_ExpectAndReturn(&conn,
                             sizeof(data1),
                             data1,
                             &tx_opt,
                             0,
                             0,
                             0,
                             SL_STATUS_OK);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_controller_transport_send_data(&conn,
                                                         sizeof(data1),
                                                         data1,
                                                         &tx_opt,
                                                         0,
                                                         0,
                                                         0));

  //Test that if both plugins reject the frame the frame we get another return code
  send_data1_ExpectAndReturn(&conn,
                             sizeof(data1),
                             data1,
                             &tx_opt,
                             0,
                             0,
                             0,
                             SL_STATUS_NOT_SUPPORTED);
  send_data2_ExpectAndReturn(&conn,
                             sizeof(data1),
                             data1,
                             &tx_opt,
                             0,
                             0,
                             0,
                             SL_STATUS_NOT_SUPPORTED);

  TEST_ASSERT_NOT_EQUAL(SL_STATUS_OK,
                        zwave_controller_transport_send_data(&conn,
                                                             sizeof(data1),
                                                             data1,
                                                             &tx_opt,
                                                             0,
                                                             0,
                                                             0));
}

void test_zwave_controller_transport_abort_test()
{
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_controller_transport_register(&t1));
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_controller_transport_register(&t2));

  zwave_tx_session_id_t session_id = (zwave_tx_session_id_t)12;
  // Abort a known session ID:
  abort_send_data2_ExpectAndReturn(session_id, SL_STATUS_NOT_SUPPORTED);

  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    zwave_controller_transport_abort_send_data(session_id));

  // Unknown session ID for abort:
  session_id = (zwave_tx_session_id_t)23;
  abort_send_data2_ExpectAndReturn(session_id, SL_STATUS_OK);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_controller_transport_abort_send_data(session_id));
}

void test_encapsulation_cc_versions_test()
{
  zwave_controller_transport_t t10 = {
    .command_class = 0xaa,
    .priority      = 1,
    .version       = 0x22,
  };
  zwave_controller_transport_t t11 = {
    .command_class = 0x9F,
    .priority      = 2,
    .version       = 10,
  };
  zwave_controller_transport_t t12 = {
    .command_class = 0x98,
    .priority      = 3,
    .version       = 20,
  };

  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_controller_transport_register(&t10));
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_controller_transport_register(&t11));
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_controller_transport_register(&t12));

  TEST_ASSERT_EQUAL(
    t10.version,
    zwave_controller_transport_is_encapsulation_cc(t10.command_class));
  TEST_ASSERT_EQUAL(
    t11.version,
    zwave_controller_transport_is_encapsulation_cc(t11.command_class));
  TEST_ASSERT_EQUAL(
    t12.version,
    zwave_controller_transport_is_encapsulation_cc(t12.command_class));
}