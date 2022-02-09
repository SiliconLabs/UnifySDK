/* © 2014 Silicon Laboratories Inc. */
/*
 * s2_inclusion_extern_mock.c
 *
 *  Created on: Sep 10, 2015
 *      Author: trasmussen
 */

#include "s2_inclusion.h"

#include <stdint.h>
#include "s2_protocol.h"
#include "mock_control.h"

#define MOCK_FILE "s2_inclusion_extern_mock.c"

// TODO: Extend comparison of S2 struct on need to have basis.
#define MOCK_CALL_COMPARE_INPUT_STRUCT_S2(P_MOCK, ARGUMENT, P_RECV_S2) do {                               \
  MOCK_CALL_COMPARE_STRUCT_MEMBER_UINT32(P_MOCK, ARGUMENT, P_RECV_S2, struct S2, my_home_id);             \
  MOCK_CALL_COMPARE_STRUCT_ARRAY_LENGTH_UINT8(P_MOCK, ARGUMENT, P_RECV_S2, struct S2, sg[0].enc_key, 16); \
  } while (0)

uint8_t s2_inclusion_set_timeout(struct S2* ctxt, uint32_t interval)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_IF_USED_AS_STUB(0x00);
  MOCK_CALL_FIND_RETURN_ON_FAILURE(p_mock, 0xFF);

  MOCK_CALL_ACTUAL(p_mock, ctxt, interval);

  MOCK_CALL_COMPARE_INPUT_STRUCT_S2(p_mock, ARG0, ctxt);
  MOCK_CALL_COMPARE_INPUT_UINT32(p_mock, ARG1, interval);

  MOCK_CALL_RETURN_VALUE(p_mock, uint8_t);
}

void s2_inclusion_stop_timeout(void)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_VOID_IF_USED_AS_STUB();
  MOCK_CALL_FIND_RETURN_VOID_ON_FAILURE(p_mock);
}

