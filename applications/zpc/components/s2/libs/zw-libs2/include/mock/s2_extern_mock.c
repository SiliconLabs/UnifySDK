/* © 2017 Silicon Laboratories Inc.
 */
/*
 * s2_extern_mock.c
 *
 *  Created on: Aug 31, 2015
 *      Author: trasmussen
 */

#include "s2_protocol.h"

#include <stdint.h>

#include "S2.h"
#include "s2_protocol.h"
#include "mock_control.h"

#define MOCK_FILE "s2_extern_mock.c"



// TODO: Extend comparison of S2 struct on need to have basis.
#define MOCK_CALL_COMPARE_INPUT_STRUCT_S2(P_MOCK, ARGUMENT, P_RECV_S2) do {                               \
  MOCK_CALL_COMPARE_STRUCT_MEMBER_UINT32(P_MOCK, ARGUMENT, P_RECV_S2, struct S2, my_home_id);             \
  MOCK_CALL_COMPARE_STRUCT_ARRAY_LENGTH_UINT8(P_MOCK, ARGUMENT, P_RECV_S2, struct S2, sg[0].enc_key, 16); \
  } while (0)

#define MOCK_CALL_COMPARE_INPUT_STRUCT_S2_CONNECTION(P_MOCK, ARGUMENT, P_ACTUAL) do {            \
  MOCK_CALL_COMPARE_STRUCT_MEMBER_UINT16(P_MOCK, ARGUMENT, P_ACTUAL, s2_connection_t, l_node);    \
  MOCK_CALL_COMPARE_STRUCT_MEMBER_UINT16(P_MOCK, ARGUMENT, P_ACTUAL, s2_connection_t, r_node);    \
  MOCK_CALL_COMPARE_STRUCT_MEMBER_UINT8(P_MOCK, ARGUMENT, P_ACTUAL, s2_connection_t, tx_options);\
  } while (0)


/**
 * This section contains mocking of the functions that are expeted to be implemented externally of
 * the s2 library.
 */
void S2_send_done_event(struct S2* ctxt, s2_tx_status_t status)
{
  mock_t * p_mock;
  MOCK_CALL_RETURN_VOID_IF_USED_AS_STUB();
  MOCK_CALL_FIND_RETURN_VOID_ON_FAILURE(p_mock);

  MOCK_CALL_ACTUAL(p_mock, ctxt, status);

  MOCK_CALL_COMPARE_INPUT_STRUCT_S2(p_mock, ARG0, ctxt);

  MOCK_CALL_COMPARE_INPUT_UINT8(p_mock, ARG1, status);
}

void S2_msg_received_event(struct S2* ctxt,s2_connection_t* src , uint8_t* buf, uint16_t len)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_VOID_IF_USED_AS_STUB();
  MOCK_CALL_FIND_RETURN_VOID_ON_FAILURE(p_mock);

  MOCK_CALL_ACTUAL(p_mock, ctxt, src, buf, len);

  MOCK_CALL_COMPARE_INPUT_STRUCT_S2(p_mock, ARG0, ctxt);
  MOCK_CALL_COMPARE_INPUT_STRUCT_S2_CONNECTION(p_mock, ARG1, src);
  MOCK_CALL_COMPARE_INPUT_UINT8_ARRAY(p_mock, ARG2, p_mock->expect_arg[3].value, buf, len);
}

uint8_t S2_send_frame(struct S2* ctxt,const s2_connection_t* conn, uint8_t* buf, uint16_t len)
{
  //printf("Sending %02x%02x%02x%02x len = %u\n", buf[0], buf[1], buf[2], buf[3], len);
  mock_t * p_mock;

  MOCK_CALL_RETURN_IF_USED_AS_STUB(0x01);
  MOCK_CALL_FIND_RETURN_ON_FAILURE(p_mock, 0x00);

  MOCK_CALL_ACTUAL(p_mock, ctxt, conn, buf, len);

  MOCK_CALL_COMPARE_INPUT_STRUCT_S2(p_mock, ARG0, ctxt);
  MOCK_CALL_COMPARE_INPUT_STRUCT_S2_CONNECTION(p_mock, ARG1, conn);
  MOCK_CALL_COMPARE_INPUT_UINT8_ARRAY(p_mock, ARG2, p_mock->expect_arg[3].value, buf, len);

  MOCK_CALL_RETURN_VALUE(p_mock, uint8_t);
}

void S2_set_timeout(struct S2* ctxt, uint32_t interval)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_VOID_IF_USED_AS_STUB();
  MOCK_CALL_FIND_RETURN_VOID_ON_FAILURE(p_mock);

  MOCK_CALL_ACTUAL(p_mock, ctxt, interval);

  MOCK_CALL_COMPARE_INPUT_STRUCT_S2(p_mock, ARG0, ctxt);
  MOCK_CALL_COMPARE_INPUT_UINT32(p_mock, ARG1, interval);
}

void S2_stop_timeout(struct S2* ctxt)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_VOID_IF_USED_AS_STUB();
  MOCK_CALL_FIND_RETURN_VOID_ON_FAILURE(p_mock);

  MOCK_CALL_ACTUAL(p_mock, ctxt);

  MOCK_CALL_COMPARE_INPUT_STRUCT_S2(p_mock, ARG0, ctxt);
}

void S2_get_hw_random(uint8_t *buf, uint8_t len)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_VOID_IF_USED_AS_STUB();
  MOCK_CALL_FIND_RETURN_VOID_ON_FAILURE(p_mock);

  MOCK_CALL_ACTUAL(p_mock, buf, len);

  MOCK_CALL_COMPARE_INPUT_UINT8(p_mock, ARG1, len);

  MOCK_CALL_SET_OUTPUT_ARRAY(p_mock->output_arg[0].pointer, buf, len, uint8_t);
}
