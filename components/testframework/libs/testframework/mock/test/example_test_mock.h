/* © 2017 Silicon Laboratories Inc. */
/*
 * example_test_mock.h
 *
 *  Created on: Aug 21, 2015
 *      Author: trasmussen
 */
#ifndef _EXAMPLE_TEST_MOCK_H_
#define _EXAMPLE_TEST_MOCK_H_

#include <stddef.h>
#include <stdint.h>

#if __C51__
  typedef uint16_t WORD;
  typedef uint8_t  BYTE;
  #define CODE   code
  #define VOID_CALLBACKFUNC(completedFunc)  void (CODE *completedFunc)
  #define STRIP_GENERIC_PTR(p) ((unsigned) (void *) (p))
  #define IS_NULL(x)  (STRIP_GENERIC_PTR(x) == 0)
  #define NON_NULL(x) (STRIP_GENERIC_PTR(x) != 0)
#elif __GNUC__
  typedef uint16_t WORD;
  typedef uint8_t  BYTE;
  #define CODE
  #define VOID_CALLBACKFUNC(completedFunc)  void (*completedFunc)
  #define IS_NULL(x)  (x == 0)
  #define NON_NULL(x) (x != 0)
#else
  #error Unknown compiler
#endif

/**@brief Mock file for testing the mock framework.
 *
 * In order to verify that the mock framework itself is testing correctly, this mock file has
 * been created. This file can also be used as a generic template for creation of new mocks.
 *
 * Feel free to add more mock templates and extend the test cases in order to make it easier for your
 * co-workers to create new mocks.
 */

/**@brief Example of a callback function which can be used for further testing.
 */
typedef void (*mock_callback_function_t)(uint32_t test_number);

typedef struct
{
  uint8_t  member_byte;
  uint32_t member;
  union
  {
    uint8_t  submember_a;
    uint32_t submember_b;
  }union_member;
}test_mock_struct_t;

/**@brief This function provides a simple function for mock calls that takes normal types.
 *
 * @param[in] arg0  First argument represented by uint8_t.
 * @param[in] arg1  Second argument represented by uint16_t.
 * @param[in] arg2  Third argument represented by uint32_t.
 *
 * @return Return value represented by uint32_t.
 */
uint32_t mock_function_simple_input(uint8_t arg0, uint16_t arg1, uint32_t arg2);

/**@brief This function provides a simple function for mock calls that takes pointers as input.
 *
 * @param[in] void_pointer  First argument represented by a void pointer.
 * @param[in] num_pointer   Second argument represented by  a pointer to uint32_t data.
 * @param[in] func_pointer  Third argument represented by a function pointer to a mock_callback_function_t callback function.
 *
 * @return Return value represented by uint32_t.
 */
uint32_t mock_function_pointer_input(void * void_pointer , uint32_t * num_pointer, mock_callback_function_t func_pointer);

/**@brief This function provides a function for mock calls that takes arrays with length information as input/output.
 *
 * @param[in]     in_array   Pointer to an array.
 * @param[in]     in_length  Length of array.
 * @param[out]    out_array  Pointer to an array.
 * @param[in,out] in_out_length Pointer where size of incoming array is specified, used for actual length when outgoing.
 *
 * @return Return value represented by uint32_t.
 */
uint32_t mock_function_array(uint8_t * in_array,  uint32_t in_length,
                             uint8_t * out_array, uint32_t *in_out_length);

/**@brief This function provides a function for void mock calls.
 */
void mock_function_void(void);

/**@brief This function provides a function for mock calls that takes a pointer to a struct as input.
 *
 * @param[in]     p_test_struct  Pointer to the test struct.
 */
void mock_function_struct(test_mock_struct_t * p_test_struct);


#endif /* _EXAMPLE_TEST_MOCK_H_ */
