/**
 * @file zwave_s0_sm_test.c
 * @brief Test of Zwave s0 inclusion state machine 
 *
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 */

#include "zwave_s0_network.h"
#include "zwave_network_management_mock.h"
#include "zwave_tx_mock.h"
#include "zwave_rx.h"
#include "ZW_classcmd.h"
#include "zwave_s0_sm.h"
#include "zwave_network_management.h"
#include "s2_classcmd.h"
#include "sl_log.h"
#include "s2_keystore.h"
#include "string.h"
#include "S2.h"

CTR_DRBG_CTX s2_ctr_drbg;
const uint8_t exp_frame_net_key_set[] = {COMMAND_CLASS_SECURITY,
                                         NETWORK_KEY_SET,
                                         0x01,
                                         0x02,
                                         0x03,
                                         0x04,
                                         0x05,
                                         0x06,
                                         0x07,
                                         0x08,
                                         0x09,
                                         0x10,
                                         0x11,
                                         0x12,
                                         0x13,
                                         0x14,
                                         0x15,
                                         0x16};

static int keystore_network_key_read_flag = 0;
bool keystore_network_key_read(uint8_t keyclass, uint8_t *buf)
{
  if (keystore_network_key_read_flag == 0) {
    memcpy(buf, &exp_frame_net_key_set[2], 16);
    return true;
  } else {
    return false;
  }
}

static uint8_t ggranted_keys  = 0;
static uint8_t gkex_fail_code = 0;

void my_s0_on_inclusion_complete_cb(zwave_keyset_t granted_keys,
                                    zwave_kex_fail_type_t kex_fail_code)
{
  TEST_ASSERT_EQUAL(ggranted_keys, granted_keys);
  TEST_ASSERT_EQUAL(gkex_fail_code, kex_fail_code);
}

static zwave_controller_connection_info_t test_connection_info = {};

on_zwave_tx_send_data_complete_t on_send_complete_cb;
sl_status_t my_callback(const zwave_controller_connection_info_t *connection,
                        uint16_t data_length,
                        const uint8_t *data,
                        const zwave_tx_options_t *tx_options,
                        const on_zwave_tx_send_data_complete_t on_send_complete,
                        void *user,
                        zwave_tx_session_id_t *session,
                        int cmock_num_calls)
{
  on_send_complete_cb = on_send_complete;
  return SL_STATUS_OK;
}

void setup_mock_for_zwave_tx_send_data(sl_status_t ret)
{
  zwave_tx_send_data_IgnoreArg_session();
  zwave_tx_send_data_IgnoreArg_user();
  zwave_tx_send_data_IgnoreArg_on_send_complete();
  zwave_tx_send_data_IgnoreArg_tx_options();
  zwave_tx_send_data_IgnoreArg_connection();
  zwave_tx_send_data_AddCallback(NULL);

  if (ret == SL_STATUS_OK) {
    zwave_tx_send_data_AddCallback(my_callback);
  }
}
void s0_expect_frame(const uint8_t *frame, uint8_t frame_len, sl_status_t ret)
{
  zwave_tx_send_data_ExpectWithArrayAndReturn(&test_connection_info,
                                              sizeof(test_connection_info),
                                              frame_len,
                                              frame,
                                              frame_len,
                                              NULL,
                                              0,
                                              NULL,
                                              NULL,
                                              0,
                                              NULL,
                                              0,
                                              ret);

  setup_mock_for_zwave_tx_send_data(ret);
}

const uint8_t exp_frame_scheme_get[]
  = {COMMAND_CLASS_SECURITY, SECURITY_SCHEME_GET, 0};

const uint8_t exp_frame_scheme_inherit[]
  = {COMMAND_CLASS_SECURITY, SECURITY_SCHEME_INHERIT, 0};

void test_zwave_s0_sm_end_node()
{
  zwave_network_management_get_node_id_IgnoreAndReturn(1);

  s0_expect_frame(exp_frame_scheme_get,
                  sizeof(exp_frame_scheme_get),
                  SL_STATUS_OK);
  TEST_ASSERT_EQUAL(S0_INC_IDLE, get_s0_sm_state());
  zwave_s0_start_bootstrapping(42, 0);
  TEST_ASSERT_EQUAL(S0_AWAITING_SCHEME_REPORT, get_s0_sm_state());
  s0_expect_frame(exp_frame_net_key_set,
                  sizeof(exp_frame_net_key_set),
                  SL_STATUS_OK);
  s0_scheme_report_received(0, 42);  //trigger scheme_report

  TEST_ASSERT_EQUAL(S0_AWAITING_KEY_VERIFY, get_s0_sm_state());
  zwave_s0_set_network_callbacks(my_s0_on_inclusion_complete_cb);
  ggranted_keys  = KEY_CLASS_S0;
  gkex_fail_code = ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE;
  s0_network_key_verify_received(42);  //trigger net key verify
  TEST_ASSERT_EQUAL(S0_INC_IDLE, get_s0_sm_state());
}

void test_zwave_s0_sm_controller()
{
  zwave_network_management_get_node_id_IgnoreAndReturn(1);

  s0_expect_frame(exp_frame_scheme_get,
                  sizeof(exp_frame_scheme_get),
                  SL_STATUS_OK);
  TEST_ASSERT_EQUAL(S0_INC_IDLE, get_s0_sm_state());
  zwave_s0_start_bootstrapping(42, 1);
  TEST_ASSERT_EQUAL(S0_AWAITING_SCHEME_REPORT, get_s0_sm_state());
  s0_expect_frame(exp_frame_net_key_set,
                  sizeof(exp_frame_net_key_set),
                  SL_STATUS_OK);
  s0_scheme_report_received(0, 42);  //trigger scheme_report

  s0_expect_frame(exp_frame_scheme_inherit,
                  sizeof(exp_frame_scheme_inherit),
                  SL_STATUS_OK);
  TEST_ASSERT_EQUAL(S0_AWAITING_KEY_VERIFY, get_s0_sm_state());
  s0_network_key_verify_received(42);  //trigger net key verify
  TEST_ASSERT_EQUAL(S0_AWAITING_2SCHEME_REPORT, get_s0_sm_state());
  zwave_s0_set_network_callbacks(my_s0_on_inclusion_complete_cb);
  ggranted_keys  = KEY_CLASS_S0;
  gkex_fail_code = ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE;
  s0_scheme_report_received(0, 42);  //trigger scheme report for scheme inherit
  TEST_ASSERT_EQUAL(S0_INC_IDLE, get_s0_sm_state());
}

/* There should not be any kind of validation on Supported security scheme
 * byte in Scheme report */
void test_zwave_s0_sm_controller_with_wrong_scheme_report()
{
  zwave_network_management_get_node_id_IgnoreAndReturn(1);

  s0_expect_frame(exp_frame_scheme_get,
                  sizeof(exp_frame_scheme_get),
                  SL_STATUS_OK);
  TEST_ASSERT_EQUAL(S0_INC_IDLE, get_s0_sm_state());
  zwave_s0_start_bootstrapping(42, 1);
  TEST_ASSERT_EQUAL(S0_AWAITING_SCHEME_REPORT, get_s0_sm_state());
  s0_expect_frame(exp_frame_net_key_set,
                  sizeof(exp_frame_net_key_set),
                  SL_STATUS_OK);
  s0_scheme_report_received(1, 42);  //trigger scheme_report

  s0_expect_frame(exp_frame_scheme_inherit,
                  sizeof(exp_frame_scheme_inherit),
                  SL_STATUS_OK);
  TEST_ASSERT_EQUAL(S0_AWAITING_KEY_VERIFY, get_s0_sm_state());
  s0_network_key_verify_received(42);  //trigger net key verify
  TEST_ASSERT_EQUAL(S0_AWAITING_2SCHEME_REPORT, get_s0_sm_state());
  zwave_s0_set_network_callbacks(my_s0_on_inclusion_complete_cb);
  ggranted_keys  = KEY_CLASS_S0;
  gkex_fail_code = ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE;
  s0_scheme_report_received(1, 42);  //trigger scheme report for scheme inherit
  TEST_ASSERT_EQUAL(S0_INC_IDLE, get_s0_sm_state());
}


void test_zwave_s0_sm_abort()
{
  zwave_network_management_get_node_id_IgnoreAndReturn(1);

  s0_expect_frame(exp_frame_scheme_get,
                  sizeof(exp_frame_scheme_get),
                  SL_STATUS_OK);
  zwave_tx_send_data_AddCallback(my_callback);
  zwave_s0_start_bootstrapping(42, 1);
  zwave_s0_set_network_callbacks(my_s0_on_inclusion_complete_cb);
  ggranted_keys  = 0;
  gkex_fail_code = ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_KEX_SCHEME;
  on_send_complete_cb(TRANSMIT_COMPLETE_FAIL, 0, 0);
}

void test_zwave_s0_sm_inclusion_when_not_idle()
{
  zwave_network_management_get_node_id_IgnoreAndReturn(1);

  TEST_ASSERT_EQUAL(S0_INC_IDLE, get_s0_sm_state());
  s0_expect_frame(exp_frame_scheme_get,
                  sizeof(exp_frame_scheme_get),
                  SL_STATUS_OK);
  zwave_tx_send_data_AddCallback(my_callback);

  zwave_s0_start_bootstrapping(42, 1);
  TEST_ASSERT_EQUAL(S0_AWAITING_SCHEME_REPORT, get_s0_sm_state());

  s0_expect_frame(exp_frame_net_key_set,
                  sizeof(exp_frame_net_key_set),
                  SL_STATUS_OK);

  s0_scheme_report_received(0, 42);  //trigger scheme_report
  TEST_ASSERT_EQUAL(S0_AWAITING_KEY_VERIFY, get_s0_sm_state());

  zwave_s0_start_bootstrapping(42, 1);
  // of existing one
  TEST_ASSERT_EQUAL(S0_AWAITING_KEY_VERIFY, get_s0_sm_state());

  s0_expect_frame(exp_frame_scheme_inherit,
                  sizeof(exp_frame_scheme_inherit),
                  SL_STATUS_OK);

  s0_network_key_verify_received(42);  //trigger net key verify
  TEST_ASSERT_EQUAL(S0_AWAITING_2SCHEME_REPORT, get_s0_sm_state());

  zwave_s0_set_network_callbacks(my_s0_on_inclusion_complete_cb);
  ggranted_keys  = KEY_CLASS_S0;
  gkex_fail_code = ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE;

  s0_scheme_report_received(0, 42);  //trigger scheme report for scheme inherit
  TEST_ASSERT_EQUAL(S0_INC_IDLE, get_s0_sm_state());
}

void test_zwave_s0_sm_scheme_report_when_not_expected()
{
  zwave_network_management_get_node_id_IgnoreAndReturn(1);

  TEST_ASSERT_EQUAL(S0_INC_IDLE, get_s0_sm_state());
  s0_expect_frame(exp_frame_scheme_get,
                  sizeof(exp_frame_scheme_get),
                  SL_STATUS_OK);
  zwave_tx_send_data_AddCallback(my_callback);

  s0_scheme_report_received(0, 42);  //trigger scheme_report
  TEST_ASSERT_EQUAL(S0_INC_IDLE, get_s0_sm_state());

  zwave_s0_start_bootstrapping(42, 1);
  TEST_ASSERT_EQUAL(S0_AWAITING_SCHEME_REPORT, get_s0_sm_state());

  s0_expect_frame(exp_frame_net_key_set,
                  sizeof(exp_frame_net_key_set),
                  SL_STATUS_OK);

  s0_scheme_report_received(0, 42);  //trigger scheme_report
  TEST_ASSERT_EQUAL(S0_AWAITING_KEY_VERIFY, get_s0_sm_state());

  s0_expect_frame(exp_frame_scheme_inherit,
                  sizeof(exp_frame_scheme_inherit),
                  SL_STATUS_OK);

  s0_scheme_report_received(0, 42);  //trigger scheme_report
  TEST_ASSERT_EQUAL(S0_AWAITING_KEY_VERIFY, get_s0_sm_state());

  s0_network_key_verify_received(42);  //trigger net key verify
  TEST_ASSERT_EQUAL(S0_AWAITING_2SCHEME_REPORT, get_s0_sm_state());

  zwave_s0_set_network_callbacks(my_s0_on_inclusion_complete_cb);
  ggranted_keys  = KEY_CLASS_S0;
  gkex_fail_code = ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_NONE;

  s0_scheme_report_received(0, 42);  //trigger scheme report for scheme inherit
  TEST_ASSERT_EQUAL(S0_INC_IDLE, get_s0_sm_state());
}
void test_zwave_s0_sm_scheme_get_send_failure()
{
  zwave_network_management_get_node_id_IgnoreAndReturn(1);

  s0_expect_frame(exp_frame_scheme_get,
                  sizeof(exp_frame_scheme_get),
                  SL_STATUS_FAIL);
  TEST_ASSERT_EQUAL(S0_INC_IDLE, get_s0_sm_state());
  zwave_s0_set_network_callbacks(my_s0_on_inclusion_complete_cb);
  ggranted_keys  = 0;
  gkex_fail_code = ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_KEX_SCHEME;
  zwave_s0_start_bootstrapping(42, 1);
  TEST_ASSERT_EQUAL(S0_INC_IDLE, get_s0_sm_state());
}

void test_zwave_s0_sm_net_key_get_send_failure()
{
  zwave_network_management_get_node_id_IgnoreAndReturn(1);
  s0_expect_frame(exp_frame_scheme_get,
                  sizeof(exp_frame_scheme_get),
                  SL_STATUS_OK);
  TEST_ASSERT_EQUAL(S0_INC_IDLE, get_s0_sm_state());
  zwave_s0_start_bootstrapping(42, 1);
  TEST_ASSERT_EQUAL(S0_AWAITING_SCHEME_REPORT, get_s0_sm_state());
  s0_expect_frame(exp_frame_net_key_set,
                  sizeof(exp_frame_net_key_set),
                  SL_STATUS_FAIL);
  s0_scheme_report_received(0, 42);  //trigger scheme_report

  zwave_s0_set_network_callbacks(my_s0_on_inclusion_complete_cb);
  ggranted_keys  = 0;
  gkex_fail_code = ZWAVE_NETWORK_MANAGEMENT_KEX_FAIL_KEX_SCHEME;
  TEST_ASSERT_EQUAL(S0_INC_IDLE, get_s0_sm_state());
}

void test_zwave_s0_sm_null_callback()
{
  zwave_network_management_get_node_id_IgnoreAndReturn(1);
  zwave_s0_set_network_callbacks(NULL);

  s0_expect_frame(exp_frame_scheme_get,
                  sizeof(exp_frame_scheme_get),
                  SL_STATUS_OK);
  TEST_ASSERT_EQUAL(S0_INC_IDLE, get_s0_sm_state());
  zwave_s0_start_bootstrapping(42, 1);
  TEST_ASSERT_EQUAL(S0_AWAITING_SCHEME_REPORT, get_s0_sm_state());
  s0_expect_frame(exp_frame_net_key_set,
                  sizeof(exp_frame_net_key_set),
                  SL_STATUS_OK);
  s0_scheme_report_received(0, 42);  //trigger scheme_report

  s0_expect_frame(exp_frame_scheme_inherit,
                  sizeof(exp_frame_scheme_inherit),
                  SL_STATUS_OK);
  TEST_ASSERT_EQUAL(S0_AWAITING_KEY_VERIFY, get_s0_sm_state());
  s0_network_key_verify_received(42);  //trigger net key verify
  TEST_ASSERT_EQUAL(S0_AWAITING_2SCHEME_REPORT, get_s0_sm_state());
  s0_scheme_report_received(0, 42);  //trigger scheme report for scheme inherit
  TEST_ASSERT_EQUAL(S0_INC_IDLE, get_s0_sm_state());
}

void test_zwave_s0_sm_failure_in_callback_to_net_key_set_send()
{
  zwave_network_management_get_node_id_IgnoreAndReturn(1);
  zwave_s0_set_network_callbacks(NULL);

  s0_expect_frame(exp_frame_scheme_get,
                  sizeof(exp_frame_scheme_get),
                  SL_STATUS_OK);
  TEST_ASSERT_EQUAL(S0_INC_IDLE, get_s0_sm_state());
  zwave_s0_start_bootstrapping(42, 1);
  TEST_ASSERT_EQUAL(S0_AWAITING_SCHEME_REPORT, get_s0_sm_state());
  s0_expect_frame(exp_frame_net_key_set,
                  sizeof(exp_frame_net_key_set),
                  SL_STATUS_OK);
  s0_scheme_report_received(0, 42);  //trigger scheme_report

  TEST_ASSERT_EQUAL(S0_AWAITING_KEY_VERIFY, get_s0_sm_state());

  on_send_complete_cb(TRANSMIT_COMPLETE_FAIL, 0, 0);
  TEST_ASSERT_EQUAL(S0_INC_IDLE, get_s0_sm_state());
}

void test_zwave_s0_sm_keystore_fail()
{
  keystore_network_key_read_flag = 1;

  zwave_network_management_get_node_id_IgnoreAndReturn(1);
  zwave_s0_set_network_callbacks(NULL);

  s0_expect_frame(exp_frame_scheme_get,
                  sizeof(exp_frame_scheme_get),
                  SL_STATUS_OK);
  TEST_ASSERT_EQUAL(S0_INC_IDLE, get_s0_sm_state());
  zwave_s0_start_bootstrapping(42, 1);
  TEST_ASSERT_EQUAL(S0_AWAITING_SCHEME_REPORT, get_s0_sm_state());
  s0_scheme_report_received(0, 42);  //trigger scheme_report

  TEST_ASSERT_EQUAL(S0_INC_IDLE, get_s0_sm_state());

  keystore_network_key_read_flag = 0;
}

void test_s0_network_init()
{
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_s0_network_init());
}
void test_s0_network_init_fail()
{
  keystore_network_key_read_flag = 1;
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_s0_network_init());
  keystore_network_key_read_flag = 0;
}
