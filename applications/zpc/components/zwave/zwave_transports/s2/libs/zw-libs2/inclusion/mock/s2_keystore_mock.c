/* © 2014 Silicon Laboratories Inc. */
/*
 * s2_keystore_mock.c
 *
 *  Created on: Sep 29, 2015
 *      Author: trasmussen
 */

#include "s2_keystore.h"

#include <stdbool.h>
#include <stdint.h>
#include "mock_control.h"

#define MOCK_FILE "s2_keystore_mock.c"


void keystore_public_key_read(uint8_t *buf)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_VOID_IF_USED_AS_STUB();
  MOCK_CALL_FIND_RETURN_VOID_ON_FAILURE(p_mock);

  MOCK_CALL_ACTUAL(p_mock, buf);

  MOCK_CALL_SET_OUTPUT_ARRAY(p_mock->output_arg[0].pointer, buf, 32, uint8_t);
}

void keystore_private_key_read(uint8_t *buf)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_VOID_IF_USED_AS_STUB();
  MOCK_CALL_FIND_RETURN_VOID_ON_FAILURE(p_mock);

  MOCK_CALL_ACTUAL(p_mock, buf);

  MOCK_CALL_SET_OUTPUT_ARRAY(p_mock->output_arg[0].pointer, buf, 32, uint8_t);
}

bool keystore_network_key_read(uint8_t keyclass, uint8_t *buf)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_IF_USED_AS_STUB(true);
  MOCK_CALL_FIND_RETURN_ON_FAILURE(p_mock, true);

  MOCK_CALL_ACTUAL(p_mock, keyclass, buf);

  MOCK_CALL_COMPARE_INPUT_UINT8(p_mock, ARG0, keyclass);

  MOCK_CALL_SET_OUTPUT_ARRAY(p_mock->output_arg[1].pointer, buf, 16, uint8_t);

  MOCK_CALL_RETURN_VALUE(p_mock, bool);
}

bool keystore_network_key_write(uint8_t keyclass,const uint8_t *buf)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_IF_USED_AS_STUB(true);
  MOCK_CALL_FIND_RETURN_ON_FAILURE(p_mock, true);

  MOCK_CALL_ACTUAL(p_mock, keyclass, buf);

  MOCK_CALL_COMPARE_INPUT_UINT8(p_mock, ARG0, keyclass);
  MOCK_CALL_COMPARE_INPUT_UINT8_ARRAY(p_mock, ARG1, 16, buf, 16);

  MOCK_CALL_RETURN_VALUE(p_mock, bool);
}

bool keystore_network_key_clear(uint8_t keyclass)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_IF_USED_AS_STUB(true);
  MOCK_CALL_FIND_RETURN_ON_FAILURE(p_mock, true);

  MOCK_CALL_ACTUAL(p_mock, keyclass);

  MOCK_CALL_COMPARE_INPUT_UINT8(p_mock, ARG0, keyclass);

  MOCK_CALL_RETURN_VALUE(p_mock, bool);
}
