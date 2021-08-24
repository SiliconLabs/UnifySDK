/******************************************************************************  * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "zwave_s0_transport.h"
#include "zwave_s0_internal.h"
#include "zwave_controller_internal_mock.h"
#include "zwave_controller_transport_mock.h"
#include "ZW_classcmd.h"
#include "zwave_s0_sm_mock.h"
#include "zwave_tx_mock.h"
#include "zwave_s0_network_mock.h"
#include "S2.h"
#include "string.h"
#include "zwave_tx_scheme_selector_mock.h"
#include "zwave_controller_keyset_mock.h"
#include "zwave_network_management_mock.h"

/* Random number generator Mocks */
CTR_DRBG_CTX s2_ctr_drbg;
void AES_CTR_DRBG_Instantiate(CTR_DRBG_CTX *ctx,
                              uint8_t *entropy,
                              const uint8_t *personal)
{
  printf("AES_CTR_DRBG_Instantiate Test. You should not see this outside "
          "unit test invocation\n");
}
static int ctr_drbg_run = 0;
/* This function will be called twice when running test_zwave_s0_new() test.
 * Return two differnt random 16 byte arrays as expected by the test to
 * simulate the uic-674.zlf on UIC-674 JIRA ticket
 */

/* if you use write a unit test which calls this function,  you have to add new
 * random array to be sent as it is supposed to send new array every time */
void AES_CTR_DRBG_Generate(CTR_DRBG_CTX *ctx, uint8_t *rand)
{
  printf("AES_CTR_DRBG_Generate Test. You should not see this outside "
          "unit test invocation\n");
  uint8_t my_rand3[16] = {0x52, 0x44, 0x41, 0x78, 0xD7, 0xE5, 0x1F, 0x40, 0xD6,
                          0x00, 0x62, 0xA1, 0x15, 0xD5, 0x00, 0xC8};
  uint8_t my_rand4[16] = {0x1A, 0xF0, 0x78, 0x62, 0x83, 0x14, 0xBD, 0x42, 0xD6,
                          0x00, 0x62, 0xA1, 0x15, 0xD5, 0x00, 0xC8};
  uint8_t my_rand5[16] = {0x1B, 0xF0, 0x78, 0x62, 0x83, 0x14, 0xBD, 0x42, 0xD6,
                          0x00, 0x62, 0xA1, 0x15, 0xD5, 0x00, 0xC8};
  switch (ctr_drbg_run) {
    case 0:
      memcpy(rand, my_rand3, sizeof(my_rand3));
      ctr_drbg_run++;
      break;
    case 1:
      memcpy(rand, my_rand4, sizeof(my_rand4));
      ctr_drbg_run++;
      break;
    case 2:
      memcpy(rand, my_rand5, sizeof(my_rand5));
      ctr_drbg_run++;
      break;

  }
}
/*-----------------------------------------------------------------*/
zwave_controller_connection_info_t connection_info = {};
zwave_rx_receive_options_t rx_options;
const zwave_tx_options_t tx_options;
uint8_t nonce_get[]           = {COMMAND_CLASS_SECURITY, SECURITY_NONCE_GET};
void setup_connection_info()
{
  memset(&connection_info, 0, sizeof(connection_info));
  connection_info.local.node_id  = 1;
  connection_info.remote.node_id = 2;
}

void on_zwave_tx_send_data_complete_failure(uint8_t status,
                                            const zwapi_tx_report_t *tx_info,
                                            void *user)
{
  TEST_ASSERT_EQUAL(status, TRANSMIT_COMPLETE_FAIL);
}
void on_zwave_tx_send_data_complete_success(uint8_t status,
                                            const zwapi_tx_report_t *tx_info,
                                            void *user)
{
  TEST_ASSERT_EQUAL(status, TRANSMIT_COMPLETE_OK);
}

void test_zwave_s0_on_frame_received_wrong_cc()
{
  setup_connection_info();
  const uint8_t frame_data[2] = {0, 0};
  TEST_ASSERT_EQUAL(zwave_s0_on_frame_received(&connection_info,
                                               &rx_options,
                                               frame_data,
                                               sizeof(frame_data)),
                    SL_STATUS_NOT_FOUND);
}

void test_zwave_s0_on_frame_received_long_frame()
{

  setup_connection_info();
  const uint8_t frame_data[ZWAVE_MAX_FRAME_SIZE + 1] = {0};
  TEST_ASSERT_EQUAL(zwave_s0_on_frame_received(&connection_info,
                                               &rx_options,
                                               frame_data,
                                               sizeof(frame_data)),
                    SL_STATUS_WOULD_OVERFLOW);
}

void test_zwave_s0_on_frame_received_wrong_cmd()
{

  setup_connection_info();
  const uint8_t frame_data[2] = {COMMAND_CLASS_SECURITY, 0};
  TEST_ASSERT_EQUAL(zwave_s0_on_frame_received(&connection_info,
                                               &rx_options,
                                               frame_data,
                                               sizeof(frame_data)),
                    SL_STATUS_NOT_FOUND);
}

void test_zwave_s0_on_frame_received_scheme_report()
{


  setup_connection_info();
  ZW_SECURITY_SCHEME_REPORT_FRAME frame_data
    = {.cmdClass                 = COMMAND_CLASS_SECURITY,
       .cmd                      = SECURITY_SCHEME_REPORT,
       .supportedSecuritySchemes = 1};

  s0_scheme_report_received_Expect(1, 2);
  TEST_ASSERT_EQUAL(zwave_s0_on_frame_received(&connection_info,
                                               &rx_options,
                                               (const uint8_t *)&frame_data,
                                               sizeof(frame_data)),
                    SL_STATUS_OK);
}

void test_zwave_s0_on_frame_received_net_key_verify()
{


  setup_connection_info();
  ZW_NETWORK_KEY_VERIFY_FRAME frame_data
    = {.cmdClass = COMMAND_CLASS_SECURITY, .cmd = NETWORK_KEY_VERIFY};

  s0_network_key_verify_received_Expect(2);
  TEST_ASSERT_EQUAL(zwave_s0_on_frame_received(&connection_info,
                                               &rx_options,
                                               (const uint8_t *)&frame_data,
                                               sizeof(frame_data)),
                    SL_STATUS_OK);
}

sl_status_t zwave_controller_transport_register_stub(
  const zwave_controller_transport_t *transport, int cmock_num_calls)
{
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY, transport->command_class);
  TEST_ASSERT_EQUAL(COMMAND_CLASS_SECURITY_VERSION, transport->version);
  TEST_ASSERT_EQUAL(3, transport->priority);

  return SL_STATUS_OK;
}

void test_zwave_s0_transport_init()
{

  setup_connection_info();
  // Nothing to test here really. We intercept the registered transport.
  zwave_controller_transport_register_AddCallback(
    zwave_controller_transport_register_stub);

  zwave_controller_transport_register_ExpectAndReturn(NULL, SL_STATUS_OK);
  zwave_controller_transport_register_IgnoreArg_transport();
  // Call the function_
  zwave_s0_network_init_ExpectAndReturn(SL_STATUS_OK);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_s0_transport_init());
}

void test_zwave_s0_transport_init_fail()
{

  setup_connection_info();
  // Nothing to test here really. We intercept the registered transport.
  zwave_controller_transport_register_AddCallback(
    zwave_controller_transport_register_stub);

  zwave_controller_transport_register_ExpectAndReturn(NULL, SL_STATUS_OK);
  zwave_controller_transport_register_IgnoreArg_transport();
  // Call the function_
  zwave_s0_network_init_ExpectAndReturn(SL_STATUS_FAIL);
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, zwave_s0_transport_init());
}


static on_zwave_tx_send_data_complete_t my_on_send_complete;
static void *my_user;
sl_status_t my_callback(const zwave_controller_connection_info_t *connection,
                        uint16_t data_length,
                        const uint8_t *data,
                        const zwave_tx_options_t *tx_options,
                        const on_zwave_tx_send_data_complete_t on_send_complete,
                        void *user,
                        zwave_tx_session_id_t *session,
                        int cmock_num_calls)
{
  my_on_send_complete = on_send_complete;
  my_user             = user;
  return SL_STATUS_OK;
}

/* This test is developed by inspiring from real Security 0 frame flow
 * Please Refer UIC-674 JIRA uic-674.zlf */
void test_zwave_s0_new()
{
  ctr_drbg_run = 0;
  s0_abort_all_tx_sessions();
  setup_connection_info();
  /* S0 key used
   * Please Refer UIC-674 JIRA attachment uic-674.zlf */
  const uint8_t key[16] = {0x98, 0x70, 0x7E, 0xE1, 0x48, 0xBB, 0xC8, 0x53, 0xB2,
                           0xFA, 0x38, 0x81, 0x8A, 0x65, 0x59, 0x73};

  connection_info.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  uint8_t version_get_encrypted[] = {COMMAND_CLASS_SECURITY,
                                     SECURITY_MESSAGE_ENCAPSULATION, 0x12, 0x9B,
                                     0x79, 0x8C, 0x2E, 0x84, 0x35, 0x7F, 0x15,
                                     0xAA, 0xC0, 0x52, 0x67, 0xC7, 0x8E, 0xCD,
                                     0xD9, 0x29, 0xF3, 0x41};
  s0_set_key(key);  //Set the security 0 key

  //Expect nonce report being sent by S0
  uint8_t nonce_report[] = {COMMAND_CLASS_SECURITY,
                            SECURITY_NONCE_REPORT,
                            0x52,
                            0x44,
                            0x41,
                            0x78,
                            0xD7,
                            0xE5,
                            0x1F,
                            0x40};

  zwave_tx_send_data_ExpectWithArrayAndReturn(NULL,
                                              0,
                                              sizeof(nonce_report),
                                              nonce_report,
                                              sizeof(nonce_report),
                                              NULL,
                                              0,
                                              NULL,
                                              NULL,
                                              0,
                                              NULL,
                                              0,
                                              SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_connection();

  // Send nonce get to s0
  TEST_ASSERT_EQUAL(zwave_s0_on_frame_received(&connection_info,
                                               &rx_options,
                                               (const uint8_t *)nonce_get,
                                               sizeof(nonce_get)),
                    SL_STATUS_OK);
  const uint8_t version_get[] = {0x86, 0x11};

  //Expect decrypted frame
  zwave_controller_on_frame_received_ExpectWithArray(NULL,
                                                     0,
                                                     NULL,
                                                     0,
                                                     version_get,
                                                     sizeof(version_get),
                                                     sizeof(version_get));
  zwave_controller_on_frame_received_IgnoreArg_connection_info();
  zwave_controller_on_frame_received_IgnoreArg_rx_options();

  /*Send Encrypted frame to S0*/
  TEST_ASSERT_EQUAL(
    zwave_s0_on_frame_received(&connection_info,
                               &rx_options,
                               (const uint8_t *)version_get_encrypted,
                               sizeof(version_get_encrypted)),
    SL_STATUS_OK);

  //Expect nonce get sent on zwave_tx by S0 as we will ask S0 to send
  //version report
  zwave_tx_send_data_ExpectWithArrayAndReturn(NULL,
                                              0,
                                              sizeof(nonce_get),
                                              nonce_get,
                                              sizeof(nonce_get),
                                              NULL,
                                              0,
                                              NULL,
                                              NULL,
                                              0,
                                              NULL,
                                              0,
                                              SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_AddCallback(my_callback);

  const uint8_t version_report[]
    = {0x86, 0x12, 0x07, 0x07, 0x0B, 0x07, 0x0B, 0x01, 0x01, 0x00, 0x00};

  connection_info.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0;
  const zwave_tx_options_t tx_options;
  //Ask s0 to send version report
  TEST_ASSERT_EQUAL(zwave_s0_send_data(&connection_info,
                                       sizeof(version_report),
                                       version_report,
                                       &tx_options,
                                       on_zwave_tx_send_data_complete_success,
                                       0,
                                       0),
                    SL_STATUS_OK);

  // Tell S0 that nonce_get was sent successfully on zwave_tx
  my_on_send_complete(TRANSMIT_COMPLETE_OK, 0, my_user);

  uint8_t version_report_encrypted[]
      = {COMMAND_CLASS_SECURITY, SECURITY_MESSAGE_ENCAPSULATION, 0x1A, 0xF0,
         0x78, 0x62, 0x83, 0x14, 0xBD, 0x42, 0x2E, 0xA2, 0x50, 0x49, 0x19, 0xAD,
         0xFC, 0x59, 0xA0, 0x92, 0x3D, 0x7B, 0xD9, 0x78, 0x2A, 0x35, 0x31, 0xF3,
         0xA4, 0xB5, 0x0E};

  // Expect encrypted frame sent by S0 as the test is going to reply
  // S0 nonce report to s0 nonce get which S0 sent on zwave_tx
  zwave_tx_send_data_ExpectWithArrayAndReturn(NULL,
                                              0,
                                              sizeof(version_report_encrypted),
                                              version_report_encrypted,
                                              sizeof(version_report_encrypted),
                                              NULL,
                                              0,
                                              NULL,
                                              NULL,
                                              0,
                                              NULL,
                                              0,
                                              SL_STATUS_OK);
  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_AddCallback(my_callback);
  connection_info.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;

  uint8_t nonce_report_frame[] = {COMMAND_CLASS_SECURITY,
                                  SECURITY_NONCE_REPORT,
                                  0xD9,
                                  0xE8,
                                  0xED,
                                  0x19,
                                  0x13,
                                  0x77,
                                  0xD8,
                                  0x5C};
  //Send nonce report to S0 as response to s0 nonce get it sent on zwave_tx
  TEST_ASSERT_EQUAL(
    zwave_s0_on_frame_received(&connection_info,
                               &rx_options,
                               (const uint8_t *)&nonce_report_frame,
                               sizeof(nonce_report_frame)),
    SL_STATUS_OK);

  //Tell S0 that version_report_encrypted[] is sent
  my_on_send_complete(TRANSMIT_COMPLETE_OK, 0, my_user);
}

void test_zwave_s0_nonce_get_callback_failed()
{
  setup_connection_info();
  //Expect nonce get sent on zwave_tx by S0 as we will ask S0 to send
  //version report
  zwave_tx_send_data_ExpectWithArrayAndReturn(NULL,
                                              0,
                                              sizeof(nonce_get),
                                              nonce_get,
                                              sizeof(nonce_get),
                                              NULL,
                                              0,
                                              NULL,
                                              NULL,
                                              0,
                                              NULL,
                                              0,
                                              SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_AddCallback(my_callback);

  const uint8_t version_report[]
    = {0x86, 0x12, 0x07, 0x07, 0x0B, 0x07, 0x0B, 0x01, 0x01, 0x00, 0x00};

  connection_info.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0;
  const zwave_tx_options_t tx_options;
  //Ask s0 to send version report
  TEST_ASSERT_EQUAL(zwave_s0_send_data(&connection_info,
                                       sizeof(version_report),
                                       version_report,
                                       &tx_options,
                                       on_zwave_tx_send_data_complete_failure,
                                       0,
                                       0),
                    SL_STATUS_OK);

  // Tell S0 that nonce_get send failed on zwave_tx
  my_on_send_complete(TRANSMIT_COMPLETE_FAIL, 0, my_user);

}

void test_zwave_s0_enc_send_callback_failed()
{
  setup_connection_info();
  ctr_drbg_run = 0;
  //Expect nonce get sent on zwave_tx by S0 as we will ask S0 to send
  //version report
  zwave_tx_send_data_ExpectWithArrayAndReturn(NULL,
                                              0,
                                              sizeof(nonce_get),
                                              nonce_get,
                                              sizeof(nonce_get),
                                              NULL,
                                              0,
                                              NULL,
                                              NULL,
                                              0,
                                              NULL,
                                              0,
                                              SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_AddCallback(my_callback);

  const uint8_t version_report[]
    = {0x86, 0x12, 0x07, 0x07, 0x0B, 0x07, 0x0B, 0x01, 0x01, 0x00, 0x00};

  connection_info.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0;
  //Ask s0 to send version report
  TEST_ASSERT_EQUAL(zwave_s0_send_data(&connection_info,
                                       sizeof(version_report),
                                       version_report,
                                       &tx_options,
                                       on_zwave_tx_send_data_complete_failure,
                                       0,
                                       0),
                    SL_STATUS_OK);

  // Tell S0 that nonce_get was sent successfully on zwave_tx
  my_on_send_complete(TRANSMIT_COMPLETE_OK, 0, my_user);

  // Expect encrypted frame sent by S0 as the test is going to reply
  // S0 nonce report to s0 nonce get which S0 sent on zwave_tx
  zwave_tx_send_data_ExpectWithArrayAndReturn(NULL,
                                              0,
                                              0,
                                              NULL,
                                              0,
                                              NULL,
                                              0,
                                              NULL,
                                              NULL,
                                              0,
                                              NULL,
                                              0,
                                              SL_STATUS_OK);
  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_data_length();
  zwave_tx_send_data_IgnoreArg_data();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_AddCallback(my_callback);
  connection_info.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;

  uint8_t nonce_report_frame[] = {COMMAND_CLASS_SECURITY,
                                  SECURITY_NONCE_REPORT,
                                  0xD9,
                                  0xE8,
                                  0xED,
                                  0x19,
                                  0x13,
                                  0x77,
                                  0xD8,
                                  0x5C};
  //Send nonce report to S0 as response to s0 nonce get it sent on zwave_tx
  TEST_ASSERT_EQUAL(
    zwave_s0_on_frame_received(&connection_info,
                               &rx_options,
                               (const uint8_t *)&nonce_report_frame,
                               sizeof(nonce_report_frame)),
    SL_STATUS_OK);
  nonce_report_frame[3] = 0xFF;
  TEST_ASSERT_EQUAL(
    zwave_s0_on_frame_received(&connection_info,
                               &rx_options,
                               (const uint8_t *)&nonce_report_frame,
                               sizeof(nonce_report_frame)),
    SL_STATUS_OK);


  //Tell S0 that version_report_encrypted[] is sent
  my_on_send_complete(TRANSMIT_COMPLETE_FAIL, 0, my_user);
}

void test_zwave_s0_multicast()
{
  const uint8_t version_report[]
    = {0x86, 0x12, 0x07, 0x07, 0x0B, 0x07, 0x0B, 0x01, 0x01, 0x00, 0x00};
  connection_info.remote.is_multicast = true;
  connection_info.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0;
  //Ask s0 to send version report
  TEST_ASSERT_EQUAL(zwave_s0_send_data(&connection_info,
                                       sizeof(version_report),
                                       version_report,
                                       &tx_options,
                                       on_zwave_tx_send_data_complete_success,
                                       0,
                                       0),
                    SL_STATUS_OK);

}

void test_zwave_s0_non_s0_encapsulated_frame_send_data()
{
  const uint8_t version_report[]
    = {0x86, 0x12, 0x07, 0x07, 0x0B, 0x07, 0x0B, 0x01, 0x01, 0x00, 0x00};
  connection_info.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  //Ask s0 to send version report
  TEST_ASSERT_EQUAL(zwave_s0_send_data(&connection_info,
                                       sizeof(version_report),
                                       version_report,
                                       &tx_options,
                                       on_zwave_tx_send_data_complete_success,
                                       0,
                                       0),
                    SL_STATUS_NOT_SUPPORTED);

}

void test_zwave_s0_new_short_encrypted_frame()
{
  ctr_drbg_run = 0;
  s0_abort_all_tx_sessions();
  setup_connection_info();
  /* S0 key used
   * Please Refer UIC-674 JIRA attachment uic-674.zlf */
  const uint8_t key[16] = {0x98, 0x70, 0x7E, 0xE1, 0x48, 0xBB, 0xC8, 0x53, 0xB2,
                           0xFA, 0x38, 0x81, 0x8A, 0x65, 0x59, 0x73};

  connection_info.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  uint8_t version_get_encrypted[] = {COMMAND_CLASS_SECURITY,
                                     SECURITY_MESSAGE_ENCAPSULATION, 0x12, 0x9B,
                                     0x79, 0x8C, 0x2E, 0x84, 0x35, 0x7F, 0x15,
                                     0xAA, 0xC0, 0x52, 0x67, 0xC7, 0x8E, 0xCD,
                                     0xD9, 0x29, 0xF3, 0x41};
  s0_set_key(key);  //Set the security 0 key

  //Expect nonce report being sent by S0
  uint8_t nonce_report[] = {COMMAND_CLASS_SECURITY,
                            SECURITY_NONCE_REPORT,
                            0x1A, 0xF0, 0x78, 0x62, 0x83, 0x14, 0xBD, 0x42};

  zwave_tx_send_data_ExpectWithArrayAndReturn(NULL,
                                              0,
                                              sizeof(nonce_report),
                                              nonce_report,
                                              sizeof(nonce_report),
                                              NULL,
                                              0,
                                              NULL,
                                              NULL,
                                              0,
                                              NULL,
                                              0,
                                              SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_connection();

  // Send nonce get to s0
  TEST_ASSERT_EQUAL(zwave_s0_on_frame_received(&connection_info,
                                               &rx_options,
                                               (const uint8_t *)nonce_get,
                                               sizeof(nonce_get)),
                    SL_STATUS_OK);
  const uint8_t version_get[] = {0x86, 0x11};

  //Expect decrypted frame
  zwave_controller_on_frame_received_ExpectWithArray(NULL,
                                                     0,
                                                     NULL,
                                                     0,
                                                     version_get,
                                                     sizeof(version_get),
                                                     sizeof(version_get));
  zwave_controller_on_frame_received_IgnoreArg_connection_info();
  zwave_controller_on_frame_received_IgnoreArg_rx_options();

  /*Send Encrypted frame to S0*/
  TEST_ASSERT_EQUAL(
    zwave_s0_on_frame_received(&connection_info,
                               &rx_options,
                               (const uint8_t *)version_get_encrypted,
                               10), //Wrong length, short frame.
    SL_STATUS_FAIL);
}

void test_zwave_s0_s0_send_nonce_zwave_tx_send_data_failed()
{
  ctr_drbg_run = 0;
  s0_abort_all_tx_sessions();
  setup_connection_info();
  /* S0 key used
   * Please Refer UIC-674 JIRA attachment uic-674.zlf */
  const uint8_t key[16] = {0x98, 0x70, 0x7E, 0xE1, 0x48, 0xBB, 0xC8, 0x53, 0xB2,
                           0xFA, 0x38, 0x81, 0x8A, 0x65, 0x59, 0x73};

  connection_info.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_NONE;
  s0_set_key(key);  //Set the security 0 key

  zwave_tx_send_data_IgnoreAndReturn(SL_STATUS_BUSY);

  // Send nonce get to s0
  TEST_ASSERT_EQUAL(zwave_s0_on_frame_received(&connection_info,
                                               &rx_options,
                                               (const uint8_t *)nonce_get,
                                               sizeof(nonce_get)),
                    SL_STATUS_OK);
}
void test_zwave_s0_on_frame_received_s0_command_supported_get()
{


  setup_connection_info();
  ZW_SECURITY_COMMANDS_SUPPORTED_GET_FRAME frame_data
    = {.cmdClass = COMMAND_CLASS_SECURITY,
       .cmd      = SECURITY_COMMANDS_SUPPORTED_GET};

  uint8_t empty_s0_command_supported_report[3]
    = {COMMAND_CLASS_SECURITY, SECURITY_COMMANDS_SUPPORTED_REPORT, 0x00};

  zwave_tx_send_data_ExpectWithArrayAndReturn(
    NULL,
    0,
    sizeof(empty_s0_command_supported_report),
    empty_s0_command_supported_report,
    sizeof(empty_s0_command_supported_report),
    NULL,
    0,
    NULL,
    NULL,
    0,
    NULL,
    0,
    SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_connection();


  connection_info.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0;
  zwave_network_management_get_node_id_ExpectAndReturn(
    connection_info.local.node_id);
  zwave_tx_scheme_get_node_granted_keys_Expect(connection_info.local.node_id,
                                               0);
  zwave_tx_scheme_get_node_granted_keys_IgnoreArg_keys();
  zwave_keyset_t key = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0;
  zwave_tx_scheme_get_node_granted_keys_ReturnThruPtr_keys(&key);
  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0);
  TEST_ASSERT_EQUAL(zwave_s0_on_frame_received(&connection_info,
                                               &rx_options,
                                               (const uint8_t *)&frame_data,
                                               sizeof(frame_data)),
                    SL_STATUS_NOT_SUPPORTED);
}

void test_zwave_s0_on_frame_received_s0_command_supported_get1()
{


  setup_connection_info();
  ZW_SECURITY_COMMANDS_SUPPORTED_GET_FRAME frame_data
    = {.cmdClass = COMMAND_CLASS_SECURITY,
       .cmd      = SECURITY_COMMANDS_SUPPORTED_GET};

  uint8_t empty_s0_command_supported_report[3]
    = {COMMAND_CLASS_SECURITY, SECURITY_COMMANDS_SUPPORTED_REPORT, 0x00};

  zwave_tx_send_data_ExpectWithArrayAndReturn(
    NULL,
    0,
    sizeof(empty_s0_command_supported_report),
    empty_s0_command_supported_report,
    sizeof(empty_s0_command_supported_report),
    NULL,
    0,
    NULL,
    NULL,
    0,
    NULL,
    0,
    SL_STATUS_OK);

  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_connection();


  connection_info.encapsulation = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0;
  zwave_network_management_get_node_id_ExpectAndReturn(
    connection_info.local.node_id);
  zwave_tx_scheme_get_node_granted_keys_Expect(connection_info.local.node_id,
                                               0);
  zwave_tx_scheme_get_node_granted_keys_IgnoreArg_keys();
  zwave_keyset_t key = ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0;
  zwave_tx_scheme_get_node_granted_keys_ReturnThruPtr_keys(&key);
  zwave_controller_get_highest_encapsulation_ExpectAndReturn(
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_0,
    ZWAVE_CONTROLLER_ENCAPSULATION_SECURITY_2_ACCESS);
  TEST_ASSERT_EQUAL(zwave_s0_on_frame_received(&connection_info,
                                               &rx_options,
                                               (const uint8_t *)&frame_data,
                                               sizeof(frame_data)),
                    SL_STATUS_OK);
}
