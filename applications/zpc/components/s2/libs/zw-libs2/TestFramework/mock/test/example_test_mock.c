/* © 2014 Silicon Laboratories Inc.
 */

/*
 * ZW_timer_mock.c
 *
 *  Created on: Aug 14, 2015
 *      Author: trasmussen
 */

#include "example_test_mock.h"
#include <stdint.h>
#include <string.h>
#include "mock_control.h"
#include "unity.h"

#define MOCK_FILE "example_test_mock.c"

uint32_t mock_function_simple_input(uint8_t arg0, uint16_t arg1, uint32_t arg2)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_IF_USED_AS_STUB(0x00);
  MOCK_CALL_FIND_RETURN_ON_FAILURE(p_mock, 0xFF);
  MOCK_CALL_ACTUAL(p_mock, arg0, arg1, arg2);
  MOCK_CALL_RETURN_IF_ERROR_SET(p_mock, uint32_t);
  MOCK_CALL_COMPARE_INPUT_UINT8(p_mock, ARG0, arg0);
  MOCK_CALL_COMPARE_INPUT_UINT16(p_mock, ARG1, arg1);
  MOCK_CALL_COMPARE_INPUT_UINT32(p_mock, ARG2, arg2);
  MOCK_CALL_RETURN_VALUE(p_mock, uint32_t);
}

uint32_t mock_function_pointer_input(void * void_pointer , uint32_t * num_pointer, mock_callback_function_t func_pointer)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_IF_USED_AS_STUB(0x00);
  MOCK_CALL_FIND_RETURN_ON_FAILURE(p_mock, 0xFF);
  MOCK_CALL_ACTUAL(p_mock, void_pointer, num_pointer, func_pointer);
  MOCK_CALL_RETURN_IF_ERROR_SET(p_mock, uint32_t);
  MOCK_CALL_COMPARE_INPUT_POINTER(p_mock, ARG0, void_pointer);
  MOCK_CALL_COMPARE_INPUT_POINTER(p_mock, ARG1, num_pointer);
  MOCK_CALL_COMPARE_INPUT_POINTER(p_mock, ARG2, func_pointer);
  MOCK_CALL_RETURN_VALUE(p_mock, uint32_t);
}

/**
 * @brief This function provides a fake implementation example when using the mock function
 *        \ref mock_function_array as fake.
 *
 * @param[in]     p_in_array   Pointer to an array.
 * @param[in]     in_length  Length of array.
 * @param[out]    p_out_array  Pointer to an array.
 * @param[in,out] p_in_out_length Pointer where size of incoming array is specified, used for actual length when outgoing.
 *
 * @return Return value represented by uint32_t.

 */
static uint32_t mock_function_array_fake(uint8_t * p_in_array,  uint32_t in_length,
                                  uint8_t * p_out_array, uint32_t *p_in_out_length)
{
    memcpy(p_out_array, p_in_array, in_length);
    return 0;
}

/**@brief This function provides a function for mock calls that takes arrays with length information as input/output.
 *
 * @param[in]     p_in_array   Pointer to an array.
 * @param[in]     in_length  Length of array.
 * @param[out]    p_out_array  Pointer to an array.
 * @param[in,out] p_in_out_length Pointer where size of incoming array is specified, used for actual length when outgoing.
 *
 * @return Return value represented by uint32_t.
 */
uint32_t mock_function_array(uint8_t * p_in_array,  uint32_t in_length,
                             uint8_t * p_out_array, uint32_t *p_in_out_length)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_IF_USED_AS_STUB(0x00);
  MOCK_CALL_RETURN_IF_USED_AS_FAKE(mock_function_array_fake, p_in_array, in_length, p_out_array, p_in_out_length);
  MOCK_CALL_FIND_RETURN_ON_FAILURE(p_mock, 0xFF);
  MOCK_CALL_ACTUAL(p_mock, p_in_array, in_length, p_out_array, p_in_out_length);
  MOCK_CALL_RETURN_IF_ERROR_SET(p_mock, uint32_t);

  // Validating array input according to expectation setup in unit test.
  MOCK_CALL_COMPARE_INPUT_UINT8_ARRAY(p_mock, ARG0, p_mock->expect_arg[1].v, p_in_array, in_length);
  MOCK_CALL_COMPARE_INPUT_UINT8(p_mock, ARG3, *p_in_out_length);

  // Setting data output  according to rules setup in unit test.
  MOCK_CALL_SET_OUTPUT_ARRAY(p_mock->output_arg[2].p, p_out_array, p_mock->output_arg[3].v, uint8_t);
  *p_in_out_length = p_mock->output_arg[3].v;

  MOCK_CALL_RETURN_VALUE(p_mock, uint32_t);
}

/**
 * Fake implementation example when using the mock function \ref mock_function_struct as fake.
 */
static void mock_function_struct_fake(test_mock_struct_t * p_test_struct)
{
  p_test_struct->member_byte              = p_test_struct->member_byte + 1;
  p_test_struct->member                   = p_test_struct->member + 2;
  p_test_struct->union_member.submember_b = p_test_struct->union_member.submember_b + 3;
}

void mock_function_struct(test_mock_struct_t * p_test_struct)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_VOID_IF_USED_AS_STUB();
  MOCK_CALL_RETURN_VOID_IF_USED_AS_FAKE(mock_function_struct_fake, p_test_struct);
  MOCK_CALL_FIND_RETURN_VOID_ON_FAILURE(p_mock);
  MOCK_CALL_ACTUAL(p_mock, p_test_struct);

  // This type of compare is comparing raw memory.
  // It is usefull if it can be ensure that ALL mem is filled out as expected.
  // If only certain fields are to be examined, used the example below.
  //MOCK_CALL_COMPARE_INPUT_POINTER(p_mock, ARG0, p_test_struct);

  // This validation checks only fields required in the struct - recommended.
  MOCK_CALL_COMPARE_STRUCT_MEMBER_UINT8(p_mock, ARG0, p_test_struct, test_mock_struct_t,  member_byte);
  MOCK_CALL_COMPARE_STRUCT_MEMBER_UINT32(p_mock, ARG0, p_test_struct, test_mock_struct_t, member);
  MOCK_CALL_COMPARE_STRUCT_MEMBER_UINT32(p_mock, ARG0, p_test_struct, test_mock_struct_t, union_member.submember_a);
  MOCK_CALL_COMPARE_STRUCT_MEMBER_UINT32(p_mock, ARG0, p_test_struct, test_mock_struct_t, union_member.submember_b);
}


void mock_function_void(void)
{
  mock_t * p_mock;

  MOCK_CALL_RETURN_VOID_IF_USED_AS_STUB();
  MOCK_CALL_FIND_RETURN_VOID_ON_FAILURE(p_mock);
}
