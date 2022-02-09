/* © 2014 Silicon Laboratories Inc. */
/*
 * curve25529_mock.c
 *
 *  Created on: Oct 20, 2015
 *      Author: trasmussen
 */

#include <stdint.h>
#include "curve25519.h"
#include "mock_control.h"

#define MOCK_FILE "curve25519_mock.c"

void crypto_scalarmult_curve25519(uint8_t *r, const uint8_t *s, const uint8_t *p)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_VOID_IF_USED_AS_STUB();
  MOCK_CALL_FIND_RETURN_VOID_ON_FAILURE(p_mock);

  MOCK_CALL_ACTUAL(p_mock, r, s, p);

  MOCK_CALL_COMPARE_INPUT_UINT8_ARRAY(p_mock, ARG1, 32, s, 32);
  MOCK_CALL_COMPARE_INPUT_UINT8_ARRAY(p_mock, ARG2, 32, p, 32);
  MOCK_CALL_SET_OUTPUT_ARRAY(p_mock->output_arg[0].pointer, r, 32, uint8_t);
}

