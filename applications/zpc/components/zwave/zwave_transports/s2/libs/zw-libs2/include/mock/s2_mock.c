/* © 2017 Silicon Laboratories Inc.
 */
/*
 * s2_mock.c
 *
 *  Created on: Sep 11, 2015
 *      Author: trasmussen
 */

#include "s2_protocol.h"
#include <stdint.h>

//#include "../S2.h"
#include <S2.h>
#include <string.h>
#include "mock_control.h"

#define MOCK_FILE "s2_mock.c"

/**
 * This section contains mocking of the functions for the s2 encapsulation library.
 */
// TODO: Extend comparison of S2 struct on need to have basis.
#define MOCK_CALL_COMPARE_INPUT_STRUCT_S2(P_MOCK, ARGUMENT, P_RECV_S2) do {                               \
  MOCK_CALL_COMPARE_STRUCT_MEMBER_UINT32(P_MOCK, ARGUMENT, P_RECV_S2, struct S2, my_home_id);             \
  MOCK_CALL_COMPARE_STRUCT_ARRAY_LENGTH_UINT8(P_MOCK, ARGUMENT, P_RECV_S2, struct S2, sg[0].enc_key, 16); \
  } while (0)

#define MOCK_CALL_COMPARE_INPUT_STRUCT_S2_CONNECTION(P_MOCK, ARGUMENT, P_ACTUAL) do {                                       \
  MOCK_CALL_COMPARE_STRUCT_MEMBER_UINT16(P_MOCK, ARGUMENT, P_ACTUAL, s2_connection_t, l_node);    \
  MOCK_CALL_COMPARE_STRUCT_MEMBER_UINT16(P_MOCK, ARGUMENT, P_ACTUAL, s2_connection_t, r_node);    \
  MOCK_CALL_COMPARE_STRUCT_MEMBER_UINT8(P_MOCK, ARGUMENT, P_ACTUAL, s2_connection_t, tx_options);\
  } while (0)


uint8_t S2_send_data(struct S2* ctx, s2_connection_t* dst ,const uint8_t* buf, uint16_t len)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_IF_USED_AS_STUB(0x01);
  MOCK_CALL_FIND_RETURN_ON_FAILURE(p_mock, 0xFF);
  MOCK_CALL_ACTUAL(p_mock, ctx, dst, buf, len);
  MOCK_CALL_RETURN_IF_ERROR_SET(p_mock, uint8_t);

  MOCK_CALL_COMPARE_INPUT_STRUCT_S2(p_mock, ARG0, ctx);
  MOCK_CALL_COMPARE_INPUT_STRUCT_S2_CONNECTION(p_mock, ARG1, dst);
  MOCK_CALL_COMPARE_INPUT_UINT8_ARRAY(p_mock, ARG2, p_mock->expect_arg[3].value, buf, len);

  MOCK_CALL_RETURN_VALUE(p_mock, uint8_t);
}


void S2_send_data_abort()
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_VOID_IF_USED_AS_STUB();
  MOCK_CALL_FIND_RETURN_VOID_ON_FAILURE(p_mock);
}

uint8_t S2_is_send_data_busy(struct S2* p_context)
{
  return 0;
}

void S2_init_prng(void)
{

}

struct S2* S2_init_ctx(uint32_t home)
{
  mock_t * p_mock;
  static struct S2 ctx;
  memset(&ctx, 0, sizeof(ctx));

  MOCK_CALL_RETURN_IF_USED_AS_STUB(&ctx); //
  MOCK_CALL_FIND_RETURN_ON_FAILURE(p_mock, NULL);
/*
  MOCK_CALL_ACTUAL(p_mock, net_key, s2_cmd_sup_frame, s2_cmd_sup_frame_size, home);

  MOCK_CALL_COMPARE_INPUT_UINT8_ARRAY(p_mock->compare_rule_arg[0], p_mock->expect_arg[0].pointer, sizeof(network_key_t),
                                                                   net_key,                 sizeof(network_key_t));*/
/*  MOCK_CALL_COMPARE_INPUT_UINT8_ARRAY(p_mock->compare_rule_arg[1], p_mock->expect_arg[1].pointer, p_mock->expect_arg[2].value,
                                                                   s2_cmd_sup_frame,        s2_cmd_sup_frame_size);*/

  MOCK_CALL_COMPARE_INPUT_UINT32(p_mock, ARG3, home);

  MOCK_CALL_RETURN_POINTER(p_mock, struct S2*);
}

void S2_application_command_handler(struct S2* ctx, s2_connection_t* src , uint8_t* buf, uint16_t len)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_VOID_IF_USED_AS_STUB();
  MOCK_CALL_FIND_RETURN_VOID_ON_FAILURE(p_mock);
  MOCK_CALL_ACTUAL(p_mock, ctx, src, buf, len);

  MOCK_CALL_COMPARE_INPUT_STRUCT_S2(p_mock, ARG0, ctx);
  MOCK_CALL_COMPARE_INPUT_STRUCT_S2_CONNECTION(p_mock, ARG1, src);
  MOCK_CALL_COMPARE_INPUT_UINT8_ARRAY(p_mock, ARG2, p_mock->expect_arg[3].value, buf, len);
}

void S2_timeout_notify(struct S2* ctxt)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_VOID_IF_USED_AS_STUB();
  MOCK_CALL_FIND_RETURN_VOID_ON_FAILURE(p_mock);
  MOCK_CALL_ACTUAL(p_mock, ctxt);

  MOCK_CALL_COMPARE_INPUT_STRUCT_S2(p_mock, ARG0, ctxt);
}

void S2_send_frame_done_notify(struct S2* ctxt, s2_tx_status_t status, uint16_t tx_time)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_VOID_IF_USED_AS_STUB();
  MOCK_CALL_FIND_RETURN_VOID_ON_FAILURE(p_mock);
  MOCK_CALL_ACTUAL(p_mock, ctxt, status);

  MOCK_CALL_COMPARE_INPUT_STRUCT_S2(p_mock, ARG0, ctxt);
  MOCK_CALL_COMPARE_INPUT_UINT32(p_mock, ARG1, status);
}
#include <stdio.h>
uint8_t S2_network_key_update(struct S2 *ctx, uint32_t key_id, security_class_t class_id,const network_key_t net_key, uint8_t temp_key_expand)
{
  mock_t * p_mock;

//  printf("%p Netkey %x class id %i\n",ctx,net_key[0],class_id);
  MOCK_CALL_RETURN_IF_USED_AS_STUB(1);
  MOCK_CALL_FIND_RETURN_ON_FAILURE(p_mock,0);
  MOCK_CALL_ACTUAL(p_mock, ctx, class_id, net_key, temp_key_expand);

  MOCK_CALL_COMPARE_INPUT_STRUCT_S2(p_mock, ARG0, ctx);
  MOCK_CALL_COMPARE_INPUT_UINT8(p_mock, ARG1, class_id);
  MOCK_CALL_COMPARE_INPUT_UINT8_ARRAY(p_mock, ARG2, sizeof(network_key_t), net_key, sizeof(network_key_t));
  MOCK_CALL_COMPARE_INPUT_UINT8(p_mock, ARG3, temp_key_expand);
  return 1;
}

uint8_t S2_send_data_multicast(struct S2* p_context, const s2_connection_t* con, const uint8_t* buf, uint16_t len)
{
  return 0;
}

uint8_t S2_is_send_data_multicast_busy(struct S2* p_context)
{
  return 0;
}

uint8_t S2_is_busy(struct S2* p_context)
{
  return 0;
}
