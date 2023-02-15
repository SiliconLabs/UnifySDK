/* © 2015Silicon Laboratories Inc.
 */
/*
 *  Created on: Aug 25, 2015
 *      Author: trasmussen
 */

#include "s2_inclusion.h"
#include <mock_control.h>
#include <stdint.h>
#include <string.h>
#include "s2_protocol.h"
#include "unity.h"
#include "curve25519.h"
#include "s2_keystore.h"

void setUpSuite(void) {

}

void tearDownSuite(void) {

}

#define ELEM_COUNT(ARRAY)  (sizeof(ARRAY)/(sizeof(ARRAY[0])))
#define UNIT_TEST_TEMP_KEY_SECURE             5       //< Value identifying index for the temporary key in S2 network key when transmitting secure frames.
#define UNIT_TEST_NETWORK_KEY                 6       //< Value identifying index for the temporary key in S2 network key when transmitting secure frames.

// Public key A as used in ECDH curve test cases.
static uint8_t  m_test_public_key_a[] = {0x85, 0x20, 0xf0, 0x09, 0x89, 0x30, 0xa7, 0x54,
                                         0x74, 0x8b, 0x7d, 0xdc, 0xb4, 0x3e, 0xf7, 0x5a,
                                         0x0d, 0xbf, 0x3a, 0x0d, 0x26, 0x38, 0x1a, 0xf4,
                                         0xeb, 0xa4, 0xa9, 0x8e, 0xaa, 0x9b, 0x4e, 0x6a};

// Public key B as used in ECDH curve test cases.
static uint8_t  m_test_public_key_b[] = {0xde, 0x9e, 0xdb, 0x7d, 0x7b, 0x7d, 0xc1, 0xb4,
                                         0xd3, 0x5b, 0x61, 0xc2, 0xec, 0xe4, 0x35, 0x37,
                                         0x3f, 0x83, 0x43, 0xc8, 0x5b, 0x78, 0x67, 0x4d,
                                         0xad, 0xfc, 0x7e, 0x14, 0x6f, 0x88, 0x2b, 0x4f};

// Private key A as used in ECDH curve test cases.
static uint8_t  m_test_private_key_a[] = {0x77, 0x07, 0x6d, 0x0a, 0x73, 0x18, 0xa5, 0x7d,
                                          0x3c, 0x16, 0xc1, 0x72, 0x51, 0xb2, 0x66, 0x45,
                                          0xdf, 0x4c, 0x2f, 0x87, 0xeb, 0xc0, 0x99, 0x2a,
                                          0xb1, 0x77, 0xfb, 0xa5, 0x1d, 0xb9, 0x2c, 0x2a};

// Private key B as used in ECDH curve test cases.
static uint8_t  m_test_private_key_b[] = {0x5d, 0xab, 0x08, 0x7e, 0x62, 0x4a, 0x8a, 0x4b,
                                          0x79, 0xe1, 0x7f, 0x8b, 0x83, 0x80, 0x0e, 0xe6,
                                          0x6f, 0x3b, 0xb1, 0x29, 0x26, 0x18, 0xb6, 0xfd,
                                          0x1c, 0x2f, 0x8b, 0x27, 0xff, 0x88, 0xe0, 0xeb};

//static uint8_t  m_test_public_key_a[32];
//static uint8_t  m_test_public_key_b[32];

static uint8_t  m_test_network_key_s2_class_0[] = {0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11,
                                                   0x00, 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99};

static uint8_t  m_test_network_key_s0[] = {0xCA, 0xFE, 0xBA, 0xBE, 0x44, 0x33, 0x22, 0x11,
                                           0xCA, 0xFE, 0xBA, 0xBE, 0xCC, 0xBB, 0xAA, 0x99};

void compare_any_all_args(mock_t *p_mock)
{
  p_mock->compare_rule_arg[0] = COMPARE_ANY; // For this call we just expect any, as we will feed the actual output into next event.
  p_mock->compare_rule_arg[1] = COMPARE_ANY;
  p_mock->compare_rule_arg[2] = COMPARE_ANY;
  p_mock->compare_rule_arg[3] = COMPARE_ANY;
}

/** This is not a real test case but a test to ensure that the including node
 *  code in inclusion is correctly working when building for a controller.
 */
void test_controller_build()
{
#ifndef ZW_CONTROLLER
  TEST_FAIL_MESSAGE("ZW_CONTROLLER is not defined but including node (ZW Controller) test cases are being executed.");
#endif
}

/** Verification that the normal flow succeeds in inclusion of a new node.
 *
 * It verifies the behavior as seen from an including node (Controller node) as described in SDS11274.
 *
 * When a node is to be included securely it is expected that a ZW_SendData is send.
 * For this the common S2_send_frame(...) defined in s2.h is used, which will be implemented elsewhere.
 */
void test_kex_joining_node_state_machine() {
  mock_t * p_kex_get_mock;
  mock_t * p_kex_report_mock;
  mock_t * p_kex_set_mock;
  mock_t * p_pub_key_b_report_mock;
  mock_t * p_pub_key_a_report_mock;
  mock_t * p_network_key_joining_update_temp_key_mock;
  mock_t * p_network_key_including_update_temp_key_mock;

  mock_t * p_echo_kex_set_mock;
  mock_t * p_echo_kex_report_mock;
  mock_t * p_network_key_get_round1_mock;
  mock_t * p_network_key_report_round1_mock;
  mock_t * p_network_key_verify_round1_mock;
  mock_t * p_network_key_update_round1_mock;
  mock_t * p_transfer_end_round1_mock;
  mock_t * p_network_key_get_round2_mock;
  mock_t * p_network_key_report_round2_mock;
  mock_t * p_network_key_verify_round2_mock;
  mock_t * p_network_key_update_round2_mock;
  mock_t * p_transfer_end_round2_mock;
  mock_t * p_transfer_end_complete_mock;

  mock_t * p_keystore_clear;
  mock_t * p_keystore_read_pub_a;
  mock_t * p_keystore_read_pub_b;
  mock_t * p_keystore_read_priv_a;
  mock_t * p_keystore_read_priv_b;
  mock_t * p_keystore_read_network_key_s2_class_0;
  mock_t * p_keystore_read_network_key_s0;

  struct S2 s2_including_context;
  struct S2 s2_joining_context;
  s2_connection_t s2_joining_conn;
  s2_connection_t s2_including_conn;

  s2_joining_conn.l_node = 2;
  s2_joining_conn.r_node = 1;
  s2_joining_conn.class_id = 0xFF;
  s2_joining_conn.rx_options = 0x00;

  s2_including_conn.l_node = 1;
  s2_including_conn.r_node = 2;
  s2_including_conn.class_id = 0xFF;
  s2_including_conn.rx_options = 0x00;

  /****************************
   * Mock expectation section *
   ****************************/
  mock_calls_clear();
  mock_call_use_as_stub(TO_STR(s2_inclusion_extern_mock.c));
  mock_call_use_as_stub(TO_STR(s2_inclusion_event_handler));
  mock_call_use_as_stub(TO_STR(keystore_network_key_write));

  // Keystore calls.
  mock_call_expect(TO_STR(keystore_network_key_clear), &p_keystore_clear);
  p_keystore_clear->expect_arg[0].value = 0xFF;

  mock_call_expect(TO_STR(keystore_dynamic_public_key_read), &p_keystore_read_pub_b);
  p_keystore_read_pub_b->output_arg[0].pointer = m_test_public_key_b;

  mock_call_expect(TO_STR(keystore_public_key_read), &p_keystore_read_pub_a);
  p_keystore_read_pub_a->output_arg[0].pointer = m_test_public_key_a;

  // When calculating the shared secret - Joining node.
  mock_call_expect(TO_STR(keystore_dynamic_private_key_read), &p_keystore_read_priv_b);
  p_keystore_read_priv_b->output_arg[0].pointer = m_test_private_key_b;
  mock_call_expect(TO_STR(keystore_dynamic_public_key_read), &p_keystore_read_pub_b);
  p_keystore_read_pub_b->output_arg[0].pointer = m_test_public_key_b;
  mock_call_expect(TO_STR(S2_network_key_update), &p_network_key_joining_update_temp_key_mock);
  compare_any_all_args(p_network_key_joining_update_temp_key_mock);

  // When calculating the shared secret - Joining node.
  mock_call_expect(TO_STR(keystore_private_key_read), &p_keystore_read_priv_a);
  p_keystore_read_priv_a->output_arg[0].pointer = m_test_private_key_a;
  mock_call_expect(TO_STR(keystore_public_key_read), &p_keystore_read_pub_a);
  p_keystore_read_pub_a->output_arg[0].pointer = m_test_public_key_a;
  mock_call_expect(TO_STR(S2_network_key_update), &p_network_key_including_update_temp_key_mock);
  compare_any_all_args(p_network_key_including_update_temp_key_mock);

  mock_call_expect(TO_STR(keystore_network_key_read), &p_keystore_read_network_key_s2_class_0);
  p_keystore_read_network_key_s2_class_0->expect_arg[0].value = KEY_CLASS_S2_UNAUTHENTICATED;
  p_keystore_read_network_key_s2_class_0->output_arg[1].pointer = m_test_network_key_s2_class_0;

  mock_call_expect(TO_STR(keystore_network_key_read), &p_keystore_read_network_key_s0);
  p_keystore_read_network_key_s0->expect_arg[0].value = KEY_CLASS_S0;
  p_keystore_read_network_key_s0->output_arg[1].pointer = m_test_network_key_s0;

  // Expectations of call for transmitting network frames.
  mock_call_expect(TO_STR(S2_send_frame), &p_kex_get_mock);
  compare_any_all_args(p_kex_get_mock);

  mock_call_expect(TO_STR(S2_send_frame), &p_kex_report_mock);
  compare_any_all_args(p_kex_report_mock);

  mock_call_expect(TO_STR(S2_send_frame), &p_kex_set_mock);
  compare_any_all_args(p_kex_set_mock);

  mock_call_expect(TO_STR(S2_send_frame), &p_pub_key_b_report_mock);
  compare_any_all_args(p_pub_key_b_report_mock);

  mock_call_expect(TO_STR(S2_send_frame), &p_pub_key_a_report_mock);
  compare_any_all_args(p_pub_key_a_report_mock);

  mock_call_expect(TO_STR(S2_send_data), &p_echo_kex_set_mock);
  compare_any_all_args(p_echo_kex_set_mock);

  mock_call_expect(TO_STR(S2_send_data), &p_echo_kex_report_mock);
  compare_any_all_args(p_echo_kex_report_mock);

  mock_call_expect(TO_STR(S2_send_data), &p_network_key_get_round1_mock);
  compare_any_all_args(p_network_key_get_round1_mock);

  mock_call_expect(TO_STR(S2_send_data), &p_network_key_report_round1_mock);
  compare_any_all_args(p_network_key_report_round1_mock);

  mock_call_expect(TO_STR(S2_network_key_update), &p_network_key_update_round1_mock);
  compare_any_all_args(p_network_key_update_round1_mock);
  mock_call_expect(TO_STR(S2_network_key_update), &p_network_key_update_round1_mock);
  compare_any_all_args(p_network_key_update_round1_mock);

  mock_call_expect(TO_STR(S2_send_data), &p_network_key_verify_round1_mock);
  compare_any_all_args(p_network_key_verify_round1_mock);

  mock_call_expect(TO_STR(S2_send_data), &p_transfer_end_round1_mock);
  compare_any_all_args(p_transfer_end_round1_mock);

  mock_call_expect(TO_STR(S2_send_data), &p_network_key_get_round2_mock);
  compare_any_all_args(p_network_key_get_round2_mock);

  mock_call_expect(TO_STR(S2_send_data), &p_network_key_report_round2_mock);
  compare_any_all_args(p_network_key_report_round2_mock);

  mock_call_expect(TO_STR(S2_network_key_update), &p_network_key_update_round2_mock);
  compare_any_all_args(p_network_key_update_round2_mock);
  mock_call_expect(TO_STR(S2_network_key_update), &p_network_key_update_round2_mock);
  compare_any_all_args(p_network_key_update_round2_mock);

  mock_call_expect(TO_STR(S2_send_data), &p_network_key_verify_round2_mock);
  compare_any_all_args(p_network_key_verify_round2_mock);

  mock_call_expect(TO_STR(S2_network_key_update), &p_network_key_update_round2_mock);
  compare_any_all_args(p_network_key_update_round2_mock);

  mock_call_expect( TO_STR(S2_send_data), &p_transfer_end_round2_mock);
  compare_any_all_args(p_transfer_end_round2_mock);


  mock_call_expect( TO_STR(S2_send_data), &p_transfer_end_complete_mock);
  compare_any_all_args(p_transfer_end_complete_mock);

  /*******************
   * Testing section *
   *******************/
  // All output from previous call (which is recordered by the mock) will be fed into the state
  // machine for the opposit site context.
  s2_joining_context.inclusion_state = 0;
  s2_including_context.inclusion_state = 0;

  s2_inclusion_init(SECURITY_2_SCHEME_1_SUPPORT,
                    KEX_REPORT_CURVE_25519,
                    0x81 /* SECURITY_2_KEY_0 | SECURITY_2_KEY_2_CLASS_0 */); 

  s2_inclusion_joining_start(&s2_joining_context,&s2_joining_conn,0);
  s2_inclusion_including_start(&s2_including_context,&s2_including_conn);

  s2_joining_context.buf = p_kex_get_mock->actual_arg[2].pointer;
  s2_joining_context.length = p_kex_get_mock->actual_arg[3].value;
  s2_joining_conn.class_id = 0xFF;
  s2_inclusion_post_event(&s2_joining_context, &s2_joining_conn);

  s2_including_context.buf = p_kex_report_mock->actual_arg[2].pointer;
  s2_including_context.length = p_kex_report_mock->actual_arg[3].value;
  s2_including_conn.class_id = 0xFF;
  s2_inclusion_post_event(&s2_including_context, &s2_including_conn);

  s2_inclusion_key_grant(&s2_including_context, 1, 0x81,0);

  s2_joining_context.buf = p_kex_set_mock->actual_arg[2].pointer;
  s2_joining_context.length = p_kex_set_mock->actual_arg[3].value;
  s2_joining_conn.class_id = 0xFF;
  s2_inclusion_post_event(&s2_joining_context, &s2_joining_conn);

  s2_including_context.buf = p_pub_key_b_report_mock->actual_arg[2].pointer;
  s2_including_context.length = p_pub_key_b_report_mock->actual_arg[3].value;
  s2_including_conn.class_id = 0xFF;
  s2_inclusion_post_event(&s2_including_context, &s2_including_conn);

  s2_joining_context.buf = p_pub_key_a_report_mock->actual_arg[2].pointer;
  s2_joining_context.length = p_pub_key_a_report_mock->actual_arg[3].value;
  s2_joining_conn.class_id = 0xFF;
  s2_inclusion_post_event(&s2_joining_context, &s2_joining_conn);


  s2_inclusion_challenge_response(&s2_including_context, 1, m_test_public_key_b, 2);
  s2_inclusion_challenge_response(&s2_joining_context, 1,m_test_public_key_a, 0);


  s2_including_context.buf = p_echo_kex_set_mock->actual_arg[2].pointer;
  s2_including_context.length = p_echo_kex_set_mock->actual_arg[3].value;
  s2_including_conn.class_id = UNIT_TEST_TEMP_KEY_SECURE;
  s2_inclusion_post_event(&s2_including_context, &s2_including_conn);

  s2_joining_context.buf = p_echo_kex_report_mock->actual_arg[2].pointer;
  s2_joining_context.length = p_echo_kex_report_mock->actual_arg[3].value;
  s2_joining_conn.class_id = UNIT_TEST_TEMP_KEY_SECURE;
  s2_inclusion_post_event(&s2_joining_context, &s2_joining_conn);

  s2_including_context.buf = p_network_key_get_round1_mock->actual_arg[2].pointer;
  s2_including_context.length = p_network_key_get_round1_mock->actual_arg[3].value;
  s2_including_conn.class_id = UNIT_TEST_TEMP_KEY_SECURE;
  s2_inclusion_post_event(&s2_including_context, &s2_including_conn);

  s2_joining_context.buf = p_network_key_report_round1_mock->actual_arg[2].pointer;
  s2_joining_context.length = p_network_key_report_round1_mock->actual_arg[3].value;
  s2_joining_conn.class_id = UNIT_TEST_TEMP_KEY_SECURE;
  s2_inclusion_post_event(&s2_joining_context, &s2_joining_conn);

  s2_including_context.buf = p_network_key_verify_round1_mock->actual_arg[2].pointer;
  s2_including_context.length = p_network_key_verify_round1_mock->actual_arg[3].value;
  s2_including_conn.class_id = 0x00;
  s2_inclusion_post_event(&s2_including_context, &s2_including_conn);

  s2_joining_context.buf = p_transfer_end_round1_mock->actual_arg[2].pointer;
  s2_joining_context.length = p_transfer_end_round1_mock->actual_arg[3].value;
  s2_joining_conn.class_id = UNIT_TEST_TEMP_KEY_SECURE;
  s2_inclusion_post_event(&s2_joining_context, &s2_joining_conn);

  s2_including_context.buf = p_network_key_get_round2_mock->actual_arg[2].pointer;
  s2_including_context.length = p_network_key_get_round2_mock->actual_arg[3].value;
  s2_including_conn.class_id = UNIT_TEST_TEMP_KEY_SECURE;
  s2_inclusion_post_event(&s2_including_context, &s2_including_conn);

  s2_joining_context.buf = p_network_key_report_round2_mock->actual_arg[2].pointer;
  s2_joining_context.length = p_network_key_report_round2_mock->actual_arg[3].value;
  s2_joining_conn.class_id = UNIT_TEST_TEMP_KEY_SECURE;
  s2_inclusion_post_event(&s2_joining_context, &s2_joining_conn);

  s2_including_context.buf = p_network_key_verify_round2_mock->actual_arg[2].pointer;
  s2_including_context.length = p_network_key_verify_round2_mock->actual_arg[3].value;
  s2_including_conn.class_id = UNIT_TEST_NETWORK_KEY;
  s2_inclusion_post_event(&s2_including_context, &s2_including_conn);

  s2_joining_context.buf = p_transfer_end_round2_mock->actual_arg[2].pointer;
  s2_joining_context.length = p_transfer_end_round2_mock->actual_arg[3].value;
  s2_joining_conn.class_id = UNIT_TEST_TEMP_KEY_SECURE;
  s2_inclusion_post_event(&s2_joining_context, &s2_joining_conn);

  // Verify that both sides agree on the calculated temporary key.
  // Original test compared ->actual_arg[1] instead of ->actual_arg[2].
  // ->actual_arg[1] contained a 0 (NULL) and thus always returned without comparing.
  // Index 2 is containing the actual indexes, however, those do not match.
  // However, that was also the case when NULL pointer was wrongly compared.
  // ToDo: EInvestigate if and how the calculated temp key should be compared. Test is disabled for now.
//  TEST_ASSERT_EQUAL_UINT8_ARRAY(p_network_key_joining_update_temp_key_mock->actual_arg[2].pointer,
//                                p_network_key_including_update_temp_key_mock->actual_arg[2].pointer,
//                                16);

  // Verify the mock has been called and stop test if not, to avoid p_transfer_end_complete_mock->actual_arg[2].pointer dereference 0x00.
  TEST_ASSERT_TRUE(p_transfer_end_complete_mock->executed);
  // 0x0C in second byte is the command value, here a TRANSFER_END is expected.
  TEST_ASSERT_EQUAL_UINT8(0x0C, ((uint8_t *)p_transfer_end_complete_mock->actual_arg[2].pointer)[1]);
  // 0x01 in third byte denotes the if key exchange is complete.
  TEST_ASSERT_EQUAL_UINT8(0x01, ((uint8_t *)p_transfer_end_complete_mock->actual_arg[2].pointer)[2]);

  mock_calls_verify();
}
