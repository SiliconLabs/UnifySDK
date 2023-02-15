/**
 * @file kderiv_mock.c
 * @copyright 2022 Silicon Laboratories Inc.
 */
#include <stdint.h>
#include "kderiv.h"
#include "mock_control.h"

#define MOCK_FILE "kderiv_mock.c"

void tempkey_extract(const uint8_t  *const ecdh_share_secret, const uint8_t  *const auth_tag, uint8_t *pseudo_random_keymat_output)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_VOID_IF_USED_AS_STUB();
  MOCK_CALL_FIND_RETURN_VOID_ON_FAILURE(p_mock);

  MOCK_CALL_ACTUAL(p_mock, ecdh_share_secret, auth_tag, pseudo_random_keymat_output);

  MOCK_CALL_COMPARE_INPUT_UINT8_ARRAY(p_mock, ARG0, 32, ecdh_share_secret, 32);
  MOCK_CALL_COMPARE_INPUT_UINT8_ARRAY(p_mock, ARG1, 64, auth_tag, 64);
  MOCK_CALL_SET_OUTPUT_ARRAY(p_mock->output_arg[2].pointer, pseudo_random_keymat_output, 32, uint8_t);
}
