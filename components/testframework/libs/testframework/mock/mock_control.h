/* © 2017 Silicon Laboratories Inc.
 */
/*
 * mock_control.h
 *
 *  Created on: Aug 14, 2015
 *      Author: trasmussen
 */

#ifndef _MOCK_CONTROL_H_
#define _MOCK_CONTROL_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "unity.h"

#define TO_STR(TEXT) #TEXT /**< Macro for converting anything to a string, usefull when expecting a function call. */
#define TO_STR_MACRO(TEXT)  TO_STR(TEXT)

#define ARG0  0
#define ARG1  1
#define ARG2  2
#define ARG3  3
#define ARG4  4
#define ARG5  5
#define ARG6  6
#define ARG7  7

#define NUMBER_OF_ARGS 8

extern uint32_t g_mock_index;
extern struct _Unity Unity;

typedef enum
{
  COMPARE_STRICT = 0x00,
  COMPARE_NULL,
  COMPARE_NOT_NULL,
  COMPARE_ANY,
  COMPARE_DELTA,
}mock_rules_t;
// Other compare rules to be considered: COMPARE_RANGE, ...

// The type of "value" should have same size as the "void *",
// if they differ in size test will fail on MIPS32 and other BE platforms,
// thus using uintptr_t which is a unsigned int with same size as a pointer.
typedef union
{
  void      * pointer;
  uintptr_t value;
  // Short versions of above types for macro use.
  void      * p;
  uintptr_t v;
}mock_type_t;

typedef struct
{
  const char       * p_func_name;                      /**< Pointer to the name of the function that this mock object covers. The function name
                                                      is specified when setting up an expected call in the mock framework, as:
                                                      mock_call_expect(TO_STR(<name_of_function>), &p_mock); */
  mock_type_t  expect_arg[NUMBER_OF_ARGS];       /**< Array of expected arguments which will be specified in the unit test, and validated
                                                      in the mock, when mocked is called by the tested unit. Setup of an expected argument
                                                      on the mock is done as (example is first argument, 0, expected to be 0x11):
                                                      p_mock->expect_arg[0].value = 0x11;
                                                      p_mock->expect_arg[0].pointer = p_some_pointer; */
  mock_type_t  delta_arg[NUMBER_OF_ARGS];        /**< Array of delta values when compare rule is set to COMPARE_DELTA.
                                                      Following example would pass on any value between 5 and 15:
                                                      p_mock->compare_rule[0]     = COMPARE_DELTA;
                                                      p_mock->expect_arg[0].value = 10;
                                                      p_mock->delta_arg[0].value  = 5; */
  mock_type_t  actual_arg[NUMBER_OF_ARGS];       /**< Array of the actual args. The actual parameters passed in the call to the function
                                                      mocked will be stored in actual_arg. This can be used for implementing additional
                                                      post processing checks or validation rules not implemented in the mock itself.
                                                      It may also be used for obtaining e.g. function pointers to callback functions. */
  mock_type_t  output_arg[NUMBER_OF_ARGS];       /**< Array of output arguments. This array holds data to be copied as output valus in
                                                      parameters list when the functoin is called. As example, an array could be returned:
                                                      p_mock->output[0].value   = length_output_array;
                                                      p_mock->output[1].pointer = output_array;  */
  mock_type_t  error_code;                       /**< Error code. The error code is useful for verifying the behavior of the unit under
                                                      test when a function call returns an error. When the error code is set, the mock
                                                      will return the error code imidiately, and not validate parametes. (This is
                                                      identical to setting compare_rule_rule to COMPARE_ANY)
                                                      Note that not all functions returns errors code, so the API should be examined to
                                                      see if this type can be used.
                                                      p_mock->error_code.value = ERROR_CODE_TO_TEST; */
  mock_type_t  return_code;                      /**< Return value/code to be returned when mock is called. This is the return value of
                                                      the function and not to be mistaken with output values provided thought pointer
                                                      parameters of the function. To have the function returning the value 10, simply do:
                                                      p_mock->return_code.value = 10; */
  mock_rules_t compare_rule_arg[NUMBER_OF_ARGS]; /**< Compare rule to be used for parameter at index n.
                                                      Default compare rule is COMPARE_STRICT. So this value needs only to be set if the
                                                      parameter should be checked against a different rule, as example, COMPARE_ANY.
                                                      For a full list of vsupported compare rules, refer to @ref mock_rules_t. */
  bool         executed;                         /**< Internal boolean used for identifying executed mocks. */
  uint32_t     mock_index;                       /**< Internal index for identifying the index of the mock. */
  uint32_t     line_number;                      /**< Internal line number to identify the line in the unit test where the mock was created. */
}mock_t;
// If compare range is added, then consider to also add: expect_arg_bound_upper, expect_arg_bound_lower.

typedef struct
{
  const char    * p_name;
}stub_t;

typedef struct
{
  char    * p_name;
}fake_t;

#define MOCK_CALL_COMPARE_INPUT_UINT8(P_MOCK, ARGUMENT, RECV_VALUE) \
        MOCK_CALL_COMPARE_INPUT(P_MOCK, ARGUMENT, RECV_VALUE, UINT8)

#define MOCK_CALL_COMPARE_INPUT_UINT16(P_MOCK, ARGUMENT, RECV_VALUE) \
        MOCK_CALL_COMPARE_INPUT(P_MOCK, ARGUMENT, RECV_VALUE, UINT16)

#define MOCK_CALL_COMPARE_INPUT_UINT32(P_MOCK, ARGUMENT, RECV_VALUE) \
        MOCK_CALL_COMPARE_INPUT(P_MOCK, ARGUMENT, RECV_VALUE, UINT32)

#define MOCK_CALL_COMPARE_INPUT_INT32(P_MOCK, ARGUMENT, RECV_VALUE) \
        MOCK_CALL_COMPARE_INPUT(P_MOCK, ARGUMENT, RECV_VALUE, INT32)


#define MOCK_CALL_COMPARE_INPUT(P_MOCK, ARGUMENT, RECV_VALUE, TYPE) do {            \
    char __str[4096];                                                                \
    switch (P_MOCK->compare_rule_arg[ARGUMENT])                                     \
    {                                                                               \
      case COMPARE_STRICT:                                                          \
        sprintf(__str, " as argument " #ARGUMENT " in %s(...), mock call expect@%s:%lu.", __FUNCTION__, Unity.CurrentTestName, (long unsigned)P_MOCK->line_number); \
        TEST_ASSERT_EQUAL_##TYPE##_MESSAGE(P_MOCK->expect_arg[ARGUMENT].v, RECV_VALUE, __str);  \
      break;                                                                        \
      case COMPARE_DELTA:                                                           \
        sprintf(__str, " as argument " #ARGUMENT " in %s(...), mock call expect@%s:%lu.", __FUNCTION__, Unity.CurrentTestName, (long unsigned)P_MOCK->line_number); \
        TEST_ASSERT_##TYPE##_WITHIN_MESSAGE(P_MOCK->delta_arg[ARGUMENT].value, P_MOCK->expect_arg[ARGUMENT].value, RECV_VALUE, __str);  \
        break;                                                                      \
      case COMPARE_NOT_NULL:                                                        \
      case COMPARE_NULL:                                                            \
        TEST_FAIL_MESSAGE("Comparing as pointer on something not a pointer in " TO_STR_MACRO(__FILE__) ":" TO_STR_MACRO(__LINE__) );       \
        break;                                                                      \
      case COMPARE_ANY:                                                             \
        break;                                                                      \
    }                                                                               \
  }while(0)


#define MOCK_CALL_COMPARE_INPUT_BOOL(P_MOCK, ARGUMENT, RECV_VALUE)  do {            \
    char __str[4096];                                                                \
    switch (P_MOCK->compare_rule_arg[ARGUMENT])                                     \
    {                                                                               \
      case COMPARE_STRICT:                                                          \
        sprintf(__str, " as argument " #ARGUMENT " in %s(...), mock call expect@%s:%lu.", __FUNCTION__, Unity.CurrentTestName, (long unsigned)P_MOCK->line_number); \
        TEST_ASSERT_MESSAGE((P_MOCK->expect_arg[ARGUMENT].v == RECV_VALUE), __str);  \
      break;                                                                        \
      case COMPARE_DELTA:                                                           \
        TEST_FAIL_MESSAGE("Comparing as bool in " TO_STR_MACRO(__FILE__) ":" TO_STR_MACRO(__LINE__) );       \
        break;                                                                      \
      case COMPARE_NOT_NULL:                                                        \
      case COMPARE_NULL:                                                            \
        TEST_FAIL_MESSAGE("Comparing as pointer on something not a pointer in " TO_STR_MACRO(__FILE__) ":" TO_STR_MACRO(__LINE__) );       \
        break;                                                                      \
      case COMPARE_ANY:                                                             \
        break;                                                                      \
    }                                                                               \
  }while(0)

#define MOCK_CALL_COMPARE_STRUCT_MEMBER_UINT8(P_MOCK, ARGUMENT, RECV_POINTER, STRUCT_TYPE, MEMBER) \
        MOCK_CALL_COMPARE_STRUCT_MEMBER(P_MOCK, ARGUMENT, RECV_POINTER, STRUCT_TYPE, MEMBER, UINT8)

#define MOCK_CALL_COMPARE_STRUCT_MEMBER_UINT16(P_MOCK, ARGUMENT, RECV_POINTER, STRUCT_TYPE, MEMBER) \
        MOCK_CALL_COMPARE_STRUCT_MEMBER(P_MOCK, ARGUMENT, RECV_POINTER, STRUCT_TYPE, MEMBER, UINT16)

#define MOCK_CALL_COMPARE_STRUCT_MEMBER_UINT32(P_MOCK, ARGUMENT, RECV_POINTER, STRUCT_TYPE, MEMBER) \
        MOCK_CALL_COMPARE_STRUCT_MEMBER(P_MOCK, ARGUMENT, RECV_POINTER, STRUCT_TYPE, MEMBER, UINT32)

#define MOCK_CALL_COMPARE_STRUCT_MEMBER(P_MOCK, ARGUMENT, RECV_POINTER, STRUCT_TYPE, MEMBER, TYPE) do { \
    char __str[4096];                                                                                 \
    switch (P_MOCK->compare_rule_arg[ARGUMENT])                                                      \
    {                                                                                                \
      case COMPARE_STRICT:                                                                           \
        sprintf(__str, " as '" TO_STR(MEMBER) "' in argument " #ARGUMENT " in %s(...), mock call expect@%s:%lu.", __FUNCTION__, Unity.CurrentTestName, (long unsigned)P_MOCK->line_number); \
        TEST_ASSERT_EQUAL_##TYPE##_MESSAGE(((STRUCT_TYPE *)P_MOCK->expect_arg[ARGUMENT].p)->MEMBER,  \
                                            ((STRUCT_TYPE *)RECV_POINTER)->MEMBER, __str);           \
      break;                                                                                         \
      case COMPARE_DELTA:                                                                            \
        sprintf(__str, " as '" TO_STR(MEMBER) "' in argument " #ARGUMENT " in %s(...), mock call expect@%s:%lu.", __FUNCTION__, Unity.CurrentTestName, (long unsigned)P_MOCK->line_number); \
        TEST_ASSERT_##TYPE##_WITHIN_MESSAGE(P_MOCK->delta_arg[ARGUMENT].value,                       \
                                            ((STRUCT_TYPE *)P_MOCK->expect_arg[ARGUMENT].p)->MEMBER, \
                                            ((STRUCT_TYPE *)RECV_POINTER)->MEMBER, __str);           \
        break;                                                                                       \
      case COMPARE_NOT_NULL:                                                                         \
        sprintf(__str, " Expected Non-NULL in %s(...), mock call expect@%s:%lu, ", __FUNCTION__, Unity.CurrentTestName, (long unsigned)P_MOCK->line_number); \
        TEST_ASSERT_NOT_NULL_MESSAGE(RECV_POINTER, __str);                                           \
        break;                                                                                       \
      case COMPARE_NULL:                                                                             \
        sprintf(__str, " Expected NULL in %s(...), mock call expect@%s:%lu, ", __FUNCTION__, Unity.CurrentTestName, (long unsigned)P_MOCK->line_number); \
        TEST_ASSERT_NULL_MESSAGE(RECV_POINTER, __str);                                               \
        break;                                                                                       \
      case COMPARE_ANY:                                                                              \
        break;                                                                                       \
    }                                                                                                \
  }while(0)

#define MOCK_CALL_COMPARE_STRUCT_ARRAY_UINT8(P_MOCK, ARGUMENT, RECV_POINTER, STRUCT_TYPE, ARRAY_MEMBER, LENGTH_MEMBER) \
        MOCK_CALL_COMPARE_STRUCT_ARRAY(P_MOCK, ARGUMENT, RECV_POINTER, STRUCT_TYPE, ARRAY_MEMBER, LENGTH_MEMBER, UINT8)

#define MOCK_CALL_COMPARE_STRUCT_ARRAY_UINT16(P_MOCK, ARGUMENT, RECV_POINTER, STRUCT_TYPE, ARRAY_MEMBER, LENGTH_MEMBER) \
        MOCK_CALL_COMPARE_STRUCT_ARRAY(P_MOCK, ARGUMENT, RECV_POINTER, STRUCT_TYPE, ARRAY_MEMBER, LENGTH_MEMBER, UINT16)

#define MOCK_CALL_COMPARE_STRUCT_ARRAY_UINT32(P_MOCK, ARGUMENT, RECV_POINTER, STRUCT_TYPE, ARRAY_MEMBER, LENGTH_MEMBER) \
        MOCK_CALL_COMPARE_STRUCT_ARRAY(P_MOCK, ARGUMENT, RECV_POINTER, STRUCT_TYPE, ARRAY_MEMBER, LENGTH_MEMBER, UINT32)

#define MOCK_CALL_COMPARE_STRUCT_ARRAY(P_MOCK, ARGUMENT, RECV_POINTER, STRUCT_TYPE, ARRAY_MEMBER, LENGTH_MEMBER, TYPE) do {   \
    char __str[4096];                                                                                                          \
    switch (P_MOCK->compare_rule_arg[ARGUMENT])                                                                               \
    {                                                                                                                         \
      case COMPARE_STRICT:                                                                                                    \
        sprintf(__str, " as '" TO_STR(LENGTH_MEMBER) "' in argument " #ARGUMENT " in %s(...), mock call expect@%s:%lu.", __FUNCTION__, Unity.CurrentTestName, (long unsigned)P_MOCK->line_number); \
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(((STRUCT_TYPE *)P_MOCK->expect_arg[ARGUMENT].p)->LENGTH_MEMBER,                      \
                                         ((STRUCT_TYPE *)RECV_POINTER)->LENGTH_MEMBER, __str);                                \
        sprintf(__str, " as '" TO_STR(ARRAY_MEMBER) "' in argument " #ARGUMENT " in %s(...), mock call expect@%s:%lu.", __FUNCTION__, Unity.CurrentTestName, (long unsigned)P_MOCK->line_number); \
        TEST_ASSERT_EQUAL_##TYPE##_ARRAY_MESSAGE(((STRUCT_TYPE *)P_MOCK->expect_arg[ARGUMENT].p)->ARRAY_MEMBER,               \
                                                 ((STRUCT_TYPE *)RECV_POINTER)->ARRAY_MEMBER,                                 \
                                                 ((STRUCT_TYPE *)P_MOCK->expect_arg[ARGUMENT].p)->LENGTH_MEMBER, __str);      \
      break;                                                                                                                  \
      case COMPARE_NOT_NULL:                                                                                                  \
        sprintf(__str, " Expected Non-NULL in %s(...), mock call expect@%s:%lu, ", __FUNCTION__, Unity.CurrentTestName, (long unsigned)P_MOCK->line_number); \
        TEST_ASSERT_NOT_NULL_MESSAGE(((STRUCT_TYPE *)RECV_POINTER)->ARRAY_MEMBER, __str);                                     \
        break;                                                                                                                \
      case COMPARE_NULL:                                                                                                      \
        sprintf(__str, " Expected NULL in %s(...), mock call expect@%s:%lu, ", __FUNCTION__, Unity.CurrentTestName, (long unsigned)P_MOCK->line_number); \
        TEST_ASSERT_NULL_MESSAGE(((STRUCT_TYPE *)RECV_POINTER)->ARRAY_MEMBER, __str);                                         \
        break;                                                                                                                \
      case COMPARE_DELTA:                                                                                                     \
        TEST_FAIL_MESSAGE("Comparing delta on array is not possible in " TO_STR_MACRO(__FILE__) ":" TO_STR_MACRO(__LINE__) ); \
        break;                                                                                                                \
        case COMPARE_ANY:                                                                                                     \
        break;                                                                                                                \
    }                                                                                                                         \
  }while(0)

#define MOCK_CALL_COMPARE_STRUCT_ARRAY_LENGTH_UINT8(P_MOCK, ARGUMENT, RECV_POINTER, STRUCT_TYPE, ARRAY_MEMBER, LENGTH) \
        MOCK_CALL_COMPARE_STRUCT_ARRAY_LENGTH(P_MOCK, ARGUMENT, RECV_POINTER, STRUCT_TYPE, ARRAY_MEMBER, LENGTH, UINT8)

#define MOCK_CALL_COMPARE_STRUCT_ARRAY_LENGTH_UINT16(P_MOCK, ARGUMENT, RECV_POINTER, STRUCT_TYPE, ARRAY_MEMBER, LENGTH) \
        MOCK_CALL_COMPARE_STRUCT_ARRAY_LENGTH(P_MOCK, ARGUMENT, RECV_POINTER, STRUCT_TYPE, ARRAY_MEMBER, LENGTH, UINT16)

#define MOCK_CALL_COMPARE_STRUCT_ARRAY_LENGTH_UINT32(P_MOCK, ARGUMENT, RECV_POINTER, STRUCT_TYPE, ARRAY_MEMBER, LENGTH) \
        MOCK_CALL_COMPARE_STRUCT_ARRAY_LENGTH(P_MOCK, ARGUMENT, RECV_POINTER, STRUCT_TYPE, ARRAY_MEMBER, LENGTH, UINT32)

#define MOCK_CALL_COMPARE_STRUCT_ARRAY_LENGTH(P_MOCK, ARGUMENT, RECV_POINTER, STRUCT_TYPE, ARRAY_MEMBER, LENGTH, TYPE) do {   \
    char __str[4096];                                                                                                          \
    switch (P_MOCK->compare_rule_arg[ARGUMENT])                                                                               \
    {                                                                                                                         \
      case COMPARE_STRICT:                                                                                                    \
        sprintf(__str, " as '" TO_STR(ARRAY_MEMBER) "' in argument " #ARGUMENT " in %s(...), mock call expect@%s:%lu.", __FUNCTION__, Unity.CurrentTestName, (long unsigned)P_MOCK->line_number); \
        TEST_ASSERT_EQUAL_##TYPE##_ARRAY_MESSAGE(((STRUCT_TYPE *)P_MOCK->expect_arg[ARGUMENT].p)->ARRAY_MEMBER,               \
                                                 ((STRUCT_TYPE *)RECV_POINTER)->ARRAY_MEMBER, LENGTH, __str);                         \
      break;                                                                                                                  \
      case COMPARE_NOT_NULL:                                                                                                  \
        TEST_ASSERT_NOT_NULL(((STRUCT_TYPE *)RECV_POINTER)->ARRAY_MEMBER);                                                    \
        break;                                                                                                                \
      case COMPARE_NULL:                                                                                                      \
        TEST_ASSERT_NULL(((STRUCT_TYPE *)RECV_POINTER)->ARRAY_MEMBER);                                                        \
        break;                                                                                                                \
      case COMPARE_DELTA:                                                                                                     \
        TEST_FAIL_MESSAGE("Comparing delta on array is not possible in " TO_STR_MACRO(__FILE__) ":" TO_STR_MACRO(__LINE__) ); \
        break;                                                                                                                \
        case COMPARE_ANY:                                                                                                     \
        break;                                                                                                                \
    }                                                                                                                         \
  }while(0)

#define MOCK_CALL_COMPARE_INPUT_UINT8_ARRAY(P_MOCK, ARGUMENT, EXPECT_LENGTH, P_RECV_ARRAY, RECV_LENGTH) \
        MOCK_CALL_COMPARE_INPUT_ARRAY(P_MOCK, ARGUMENT, EXPECT_LENGTH, P_RECV_ARRAY, RECV_LENGTH, UINT8)

#define MOCK_CALL_COMPARE_INPUT_UINT16_ARRAY(P_MOCK, ARGUMENT, EXPECT_LENGTH, P_RECV_ARRAY, RECV_LENGTH) \
        MOCK_CALL_COMPARE_INPUT_ARRAY(P_MOCK, ARGUMENT, EXPECT_LENGTH, P_RECV_ARRAY, RECV_LENGTH, UINT16)

#define MOCK_CALL_COMPARE_INPUT_UINT32_ARRAY(P_MOCK, ARGUMENT, EXPECT_LENGTH, P_RECV_ARRAY, RECV_LENGTH) \
        MOCK_CALL_COMPARE_INPUT_ARRAY(P_MOCK, ARGUMENT, EXPECT_LENGTH, P_RECV_ARRAY, RECV_LENGTH, UINT32)

#define MOCK_CALL_COMPARE_INPUT_INT_ARRAY(P_MOCK, ARGUMENT, EXPECT_LENGTH, P_RECV_ARRAY, RECV_LENGTH) \
        MOCK_CALL_COMPARE_INPUT_ARRAY(P_MOCK, ARGUMENT, EXPECT_LENGTH, P_RECV_ARRAY, RECV_LENGTH, INT)

#define MOCK_CALL_COMPARE_INPUT_ARRAY(P_MOCK, ARGUMENT, EXPECT_LENGTH, P_RECV_ARRAY, RECV_LENGTH, TYPE) do { \
    char __str[4096];                                                                                         \
    switch (P_MOCK->compare_rule_arg[ARGUMENT])                                                              \
    {                                                                                                        \
      case COMPARE_STRICT:                                                                                   \
        sprintf(__str, " in %s(...), mock call expect@%s:%lu, ", __FUNCTION__, Unity.CurrentTestName, (long unsigned)P_MOCK->line_number); \
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(EXPECT_LENGTH, RECV_LENGTH, __str);                                 \
        sprintf(__str, " as arguments " #ARGUMENT " in %s(...), mock call expect@%s:%lu.", __FUNCTION__, Unity.CurrentTestName, (long unsigned)P_MOCK->line_number); \
        TEST_ASSERT_EQUAL_##TYPE##_ARRAY_MESSAGE(P_MOCK->expect_arg[ARGUMENT].p, P_RECV_ARRAY, EXPECT_LENGTH, __str);  \
        break;                                                                                               \
      case COMPARE_NOT_NULL:                                                                                 \
        sprintf(__str, " Expected Non-NULL in %s(...), mock call expect@%s:%lu, ", __FUNCTION__, Unity.CurrentTestName, (long unsigned)P_MOCK->line_number); \
	    TEST_ASSERT_NOT_NULL_MESSAGE(P_RECV_ARRAY, __str);                                                   \
        break;                                                                                               \
      case COMPARE_NULL:                                                                                     \
        sprintf(__str, " Expected NULL in %s(...), mock call expect@%s:%lu, ", __FUNCTION__, Unity.CurrentTestName, (long unsigned)P_MOCK->line_number); \
	    TEST_ASSERT_NULL_MESSAGE(P_RECV_ARRAY, __str);                                                       \
        break;                                                                                               \
      case COMPARE_DELTA:                                                                                    \
        TEST_FAIL_MESSAGE("Comparing delta on array is not possible in " TO_STR_MACRO(__FILE__) ":" TO_STR_MACRO(__LINE__) ); \
        break;                                                                                               \
      case COMPARE_ANY:                                                                                      \
        break;                                                                                               \
    }                                                                                                        \
  }while(0)

#define MOCK_CALL_SET_OUTPUT_ARRAY(P_RESULT_ARRAY, P_OUT_ARRAY, LENGTH, TYPE) do { \
    uint32_t   __loop_i;                                                           \
    TYPE     * __result_array = (TYPE *)P_RESULT_ARRAY;                            \
    for (__loop_i = 0; __loop_i < LENGTH; __loop_i++)                              \
    {                                                                              \
      P_OUT_ARRAY[__loop_i] = __result_array[__loop_i];                            \
    }                                                                              \
  }while(0)

#define MOCK_CALL_COMPARE_INPUT_POINTER(P_MOCK, ARGUMENT, P_RECV_VALUE) do {                \
    char __str[4096];                                                                        \
    switch (P_MOCK->compare_rule_arg[ARGUMENT])                                             \
    {                                                                                       \
      case COMPARE_STRICT:                                                                  \
        sprintf(__str, " as argument " #ARGUMENT " in %s(...), mock index=%lu, " __FILE__ ":"  TO_STR_MACRO(__LINE__), __FUNCTION__, (long unsigned)g_mock_index); \
        TEST_ASSERT_EQUAL_PTR_MESSAGE(P_MOCK->expect_arg[ARGUMENT].p, P_RECV_VALUE, __str); \
        break;                                                                              \
      case COMPARE_NOT_NULL:                                                                \
        sprintf(__str, " Expected Non-NULL in %s(...), mock call expect@%s:%lu, ", __FUNCTION__, Unity.CurrentTestName, (long unsigned)P_MOCK->line_number); \
        TEST_ASSERT_NOT_NULL_MESSAGE(P_RECV_VALUE, __str);                                  \
        break;                                                                              \
      case COMPARE_NULL:                                                                    \
        sprintf(__str, " Expected NULL in %s(...), mock call expect@%s:%lu, ", __FUNCTION__, Unity.CurrentTestName, (long unsigned)P_MOCK->line_number); \
        TEST_ASSERT_NULL_MESSAGE(P_RECV_VALUE, __str);                                      \
        break;                                                                              \
      case COMPARE_ANY:                                                                     \
        break;                                                                              \
      case COMPARE_DELTA:                                                                   \
        TEST_FAIL_MESSAGE("Comparing delta on pointer in " TO_STR_MACRO(__FILE__) ":" TO_STR_MACRO(__LINE__) ); \
        break;                                                                              \
    }                                                                                       \
  }while(0)

#define MOCK_CALL_FIND_RETURN_ON_FAILURE(P_MOCK, DEFAULT_VAL) do {  \
    if (!mock_call_find(__FUNCTION__, &P_MOCK))                     \
    {                                                               \
      return DEFAULT_VAL;                                           \
    }                                                               \
}while(0)

#define MOCK_CALL_FIND_RETURN_VOID_ON_FAILURE(P_MOCK) do {  \
    if (!mock_call_find(__FUNCTION__, &P_MOCK))                          \
    {                                                                    \
      return;                                                            \
    }                                                                    \
}while(0)

#define MOCK_CALL_RETURN_IF_USED_AS_STUB(DEFAULT_VAL) do {        \
    if (mock_call_used_as_stub( MOCK_FILE, __FUNCTION__) == true) \
    {                                                             \
      return DEFAULT_VAL;                                         \
    }                                                             \
}while(0)

#define MOCK_CALL_RETURN_VOID_IF_USED_AS_STUB() do {   \
    if (mock_call_used_as_stub( MOCK_FILE, __FUNCTION__) == true) \
    {                                                             \
      return;                                                     \
    }                                                             \
}while(0)

#define MOCK_CALL_RETURN_FAKE_ARG1(P_FUNCTION, ARG0) do {         \
    if (mock_call_used_as_fake( MOCK_FILE, __FUNCTION__) == true) \
    {                                                             \
      return P_FUNCTION(ARG0);                                    \
    }                                                             \
}while(0)

#define MOCK_CALL_RETURN_FAKE_ARG2(P_FUNCTION, ARG0, ARG1) do {    \
    if (mock_call_used_as_fake( MOCK_FILE, __FUNCTION__) == true)  \
    {                                                              \
      return P_FUNCTION(ARG0, ARG1);                               \
    }                                                              \
}while(0)

#define MOCK_CALL_RETURN_FAKE_ARG3(P_FUNCTION, ARG0, ARG1, ARG2) do { \
    if (mock_call_used_as_fake( MOCK_FILE, __FUNCTION__) == true)     \
    {                                                                 \
      return P_FUNCTION(ARG0, ARG1, ARG2);                            \
    }                                                                 \
}while(0)

#define MOCK_CALL_RETURN_FAKE_ARG4(P_FUNCTION, ARG0, ARG1, ARG2, ARG3) do { \
    if (mock_call_used_as_fake( MOCK_FILE, __FUNCTION__) == true)           \
    {                                                                       \
      return P_FUNCTION(ARG0, ARG1, ARG2, ARG3);                            \
    }                                                                       \
}while(0)

#define MOCK_CALL_RETURN_FAKE_ARG5(P_FUNCTION, ARG0, ARG1, ARG2, ARG3, ARG4) do { \
    if (mock_call_used_as_fake( MOCK_FILE, __FUNCTION__) == true)                 \
    {                                                                             \
      return P_FUNCTION(ARG0, ARG1, ARG2, ARG3, ARG4);                            \
    }                                                                             \
}while(0)

#define MOCK_CALL_RETURN_FAKE_ARG6(P_FUNCTION, ARG0, ARG1, ARG2, ARG3, ARG4, ARG5) do { \
    if (mock_call_used_as_fake( MOCK_FILE, __FUNCTION__) == true)                       \
    {                                                                                   \
      return P_FUNCTION(ARG0, ARG1, ARG2, ARG3, ARG4, ARG5);                            \
    }                                                                                   \
}while(0)

#define MOCK_CALL_RETURN_FAKE_ARG7(P_FUNCTION, ARG0, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) do { \
    if (mock_call_used_as_fake( MOCK_FILE, __FUNCTION__) == true)                             \
    {                                                                                         \
      return P_FUNCTION(ARG0, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);                            \
    }                                                                                         \
}while(0)

#define MOCK_CALL_RETURN_FAKE_ARG8(P_FUNCTION, ARG0, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) do { \
    if (mock_call_used_as_fake( MOCK_FILE, __FUNCTION__) == true)                                   \
    {                                                                                               \
      return P_FUNCTION(ARG0, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);                            \
    }                                                                                               \
}while(0)

#define MOCK_CALL_RETURN_VOID_FAKE_ARG1(P_FUNCTION, ARG0) do {    \
    if (mock_call_used_as_fake( MOCK_FILE, __FUNCTION__) == true) \
    {                                                             \
      P_FUNCTION(ARG0);                                           \
      return;                                                     \
    }                                                             \
}while(0)

#define MOCK_CALL_RETURN_VOID_FAKE_ARG2(P_FUNCTION, ARG0, ARG1) do {    \
    if (mock_call_used_as_fake( MOCK_FILE, __FUNCTION__) == true)       \
    {                                                                   \
      P_FUNCTION(ARG0, ARG1);                                           \
      return;                                                           \
    }                                                                   \
}while(0)

#define MOCK_CALL_RETURN_VOID_FAKE_ARG3(P_FUNCTION, ARG0, ARG1, ARG2) do { \
    if (mock_call_used_as_fake( MOCK_FILE, __FUNCTION__) == true)          \
    {                                                                      \
      P_FUNCTION(ARG0, ARG1, ARG2);                                        \
      return;                                                              \
    }                                                                      \
}while(0)

#define MOCK_CALL_RETURN_VOID_FAKE_ARG4(P_FUNCTION, ARG0, ARG1, ARG2, ARG3) do { \
    if (mock_call_used_as_fake( MOCK_FILE, __FUNCTION__) == true)                \
    {                                                                            \
      P_FUNCTION(ARG0, ARG1, ARG2, ARG3);                                        \
      return;                                                                    \
    }                                                                            \
}while(0)

#define MOCK_CALL_RETURN_VOID_FAKE_ARG5(P_FUNCTION, ARG0, ARG1, ARG2, ARG3, ARG4) do { \
    if (mock_call_used_as_fake( MOCK_FILE, __FUNCTION__) == true)                      \
    {                                                                                  \
      P_FUNCTION(ARG0, ARG1, ARG2, ARG3, ARG4);                                        \
      return;                                                                          \
    }                                                                                  \
}while(0)

#define MOCK_CALL_RETURN_VOID_FAKE_ARG6(P_FUNCTION, ARG0, ARG1, ARG2, ARG3, ARG4, ARG5) do { \
    if (mock_call_used_as_fake( MOCK_FILE, __FUNCTION__) == true)                            \
    {                                                                                        \
      P_FUNCTION(ARG0, ARG1, ARG2, ARG3, ARG4, ARG5);                                        \
      return;                                                                                \
    }                                                                                        \
}while(0)

#define MOCK_CALL_RETURN_VOID_FAKE_ARG7(P_FUNCTION, ARG0, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) do { \
    if (mock_call_used_as_fake( MOCK_FILE, __FUNCTION__) == true)                                  \
    {                                                                                              \
      P_FUNCTION(ARG0, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6);                                        \
      return;                                                                                      \
    }                                                                                              \
}while(0)

#define MOCK_CALL_RETURN_VOID_FAKE_ARG8(P_FUNCTION, ARG0, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) do { \
    if (mock_call_used_as_fake( MOCK_FILE, __FUNCTION__) == true)                                        \
    {                                                                                                    \
      P_FUNCTION(ARG0, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7);                                        \
      return;                                                                                            \
    }                                                                                                    \
}while(0)

#define MOCK_CALL_RETURN_IF_ERROR_SET(P_MOCK, TYPE) do {  \
    if (P_MOCK->error_code.value != 0 )                   \
    {                                                     \
      return (TYPE)P_MOCK->error_code.value;              \
    }                                                     \
}while(0)

#define MOCK_CALL_RETURN_VALUE(P_MOCK, TYPE) do { \
    return (TYPE)P_MOCK->return_code.value;             \
}while(0)

#define MOCK_CALL_RETURN_POINTER(P_MOCK, TYPE) do { \
    return (TYPE)P_MOCK->return_code.pointer;             \
}while(0)


#if __C51__
#define MOCK_CALL_ACTUAL_ARG1(P_MOCK, ARG0) do {  \
    P_MOCK->actual_arg[0].value = (uint32_t) ARG0;   \
  }while(0)

#define MOCK_CALL_ACTUAL_ARG2(P_MOCK, ARG0, ARG1) do {  \
    P_MOCK->actual_arg[0].value = (uint32_t) ARG0;            \
    P_MOCK->actual_arg[1].value = (uint32_t) ARG1;            \
  }while(0)

#define MOCK_CALL_ACTUAL_ARG3(P_MOCK, ARG0, ARG1, ARG2) do {  \
    P_MOCK->actual_arg[0].value = (uint32_t) ARG0;                  \
    P_MOCK->actual_arg[1].value = (uint32_t) ARG1;                  \
    P_MOCK->actual_arg[2].value = (uint32_t) ARG2;                  \
  }while(0)

#define MOCK_CALL_ACTUAL_ARG4(P_MOCK, ARG0, ARG1, ARG2, ARG3) do { \
    P_MOCK->actual_arg[0].value = (uint32_t) ARG0;                       \
    P_MOCK->actual_arg[1].value = (uint32_t) ARG1;                       \
    P_MOCK->actual_arg[2].value = (uint32_t) ARG2;                       \
    P_MOCK->actual_arg[3].value = (uint32_t) ARG3;                       \
  }while(0)

#define MOCK_CALL_ACTUAL_ARG5(P_MOCK, ARG0, ARG1, ARG2, ARG3, ARG4) do {  \
    P_MOCK->actual_arg[0].value = (uint32_t) ARG0;                              \
    P_MOCK->actual_arg[1].value = (uint32_t) ARG1;                              \
    P_MOCK->actual_arg[2].value = (uint32_t) ARG2;                              \
    P_MOCK->actual_arg[3].value = (uint32_t) ARG3;                              \
    P_MOCK->actual_arg[4].value = (uint32_t) ARG4;                              \
  }while(0)

#define MOCK_CALL_ACTUAL_ARG6(P_MOCK, ARG0, ARG1, ARG2, ARG3, ARG4, ARG5) do {  \
    P_MOCK->actual_arg[0].value = (uint32_t) ARG0;                                    \
    P_MOCK->actual_arg[1].value = (uint32_t) ARG1;                                    \
    P_MOCK->actual_arg[2].value = (uint32_t) ARG2;                                    \
    P_MOCK->actual_arg[3].value = (uint32_t) ARG3;                                    \
    P_MOCK->actual_arg[4].value = (uint32_t) ARG4;                                    \
    P_MOCK->actual_arg[5].value = (uint32_t) ARG5;                                    \
  }while(0)
#else
#define MOCK_CALL_ACTUAL_ARG1(P_MOCK, ARG0) do {  \
    P_MOCK->actual_arg[0].pointer = (void *)(uintptr_t) ARG0;   \
  }while(0)

#define MOCK_CALL_ACTUAL_ARG2(P_MOCK, ARG0, ARG1) do {  \
    P_MOCK->actual_arg[0].pointer = (void *)(uintptr_t) ARG0;            \
    P_MOCK->actual_arg[1].pointer = (void *)(uintptr_t) ARG1;            \
  }while(0)

#define MOCK_CALL_ACTUAL_ARG3(P_MOCK, ARG0, ARG1, ARG2) do {  \
    P_MOCK->actual_arg[0].pointer = (void *)(uintptr_t) ARG0;                  \
    P_MOCK->actual_arg[1].pointer = (void *)(uintptr_t) ARG1;                  \
    P_MOCK->actual_arg[2].pointer = (void *)(uintptr_t) ARG2;                  \
  }while(0)

#define MOCK_CALL_ACTUAL_ARG4(P_MOCK, ARG0, ARG1, ARG2, ARG3) do { \
    P_MOCK->actual_arg[0].pointer = (void *)(uintptr_t) ARG0;                       \
    P_MOCK->actual_arg[1].pointer = (void *)(uintptr_t) ARG1;                       \
    P_MOCK->actual_arg[2].pointer = (void *)(uintptr_t) ARG2;                       \
    P_MOCK->actual_arg[3].pointer = (void *)(uintptr_t) ARG3;                       \
  }while(0)

#define MOCK_CALL_ACTUAL_ARG5(P_MOCK, ARG0, ARG1, ARG2, ARG3, ARG4) do {  \
    P_MOCK->actual_arg[0].pointer = (void *)(uintptr_t) ARG0;                              \
    P_MOCK->actual_arg[1].pointer = (void *)(uintptr_t) ARG1;                              \
    P_MOCK->actual_arg[2].pointer = (void *)(uintptr_t) ARG2;                              \
    P_MOCK->actual_arg[3].pointer = (void *)(uintptr_t) ARG3;                              \
    P_MOCK->actual_arg[4].pointer = (void *)(uintptr_t) ARG4;                              \
  }while(0)

#define MOCK_CALL_ACTUAL_ARG6(P_MOCK, ARG0, ARG1, ARG2, ARG3, ARG4, ARG5) do {  \
    P_MOCK->actual_arg[0].pointer = (void *)(uintptr_t) ARG0;                                    \
    P_MOCK->actual_arg[1].pointer = (void *)(uintptr_t) ARG1;                                    \
    P_MOCK->actual_arg[2].pointer = (void *)(uintptr_t) ARG2;                                    \
    P_MOCK->actual_arg[3].pointer = (void *)(uintptr_t) ARG3;                                    \
    P_MOCK->actual_arg[4].pointer = (void *)(uintptr_t) ARG4;                                    \
    P_MOCK->actual_arg[5].pointer = (void *)(uintptr_t) ARG5;                                    \
  }while(0)

#define MOCK_CALL_ACTUAL_ARG7(P_MOCK, ARG0, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) do {  \
    P_MOCK->actual_arg[0].pointer = (void *)(uintptr_t) ARG0;                                    \
    P_MOCK->actual_arg[1].pointer = (void *)(uintptr_t) ARG1;                                    \
    P_MOCK->actual_arg[2].pointer = (void *)(uintptr_t) ARG2;                                    \
    P_MOCK->actual_arg[3].pointer = (void *)(uintptr_t) ARG3;                                    \
    P_MOCK->actual_arg[4].pointer = (void *)(uintptr_t) ARG4;                                    \
    P_MOCK->actual_arg[5].pointer = (void *)(uintptr_t) ARG5;                                    \
    P_MOCK->actual_arg[6].pointer = (void *)(uintptr_t) ARG6;                                    \
  }while(0)

#define MOCK_CALL_ACTUAL_ARG8(P_MOCK, ARG0, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6, ARG7) do {  \
    P_MOCK->actual_arg[0].pointer = (void *)(uintptr_t) ARG0;                                    \
    P_MOCK->actual_arg[1].pointer = (void *)(uintptr_t) ARG1;                                    \
    P_MOCK->actual_arg[2].pointer = (void *)(uintptr_t) ARG2;                                    \
    P_MOCK->actual_arg[3].pointer = (void *)(uintptr_t) ARG3;                                    \
    P_MOCK->actual_arg[4].pointer = (void *)(uintptr_t) ARG4;                                    \
    P_MOCK->actual_arg[5].pointer = (void *)(uintptr_t) ARG5;                                    \
    P_MOCK->actual_arg[6].pointer = (void *)(uintptr_t) ARG6;                                    \
    P_MOCK->actual_arg[7].pointer = (void *)(uintptr_t) ARG7;                                    \
  }while(0)
#endif

#define N_ARGS_COUNT(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define N_ARGS(...)                                  N_ARGS_COUNT(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1)

#define MOCK_CALL_RETURN_VOID_FAKE_HELPER_IMPL(P_FUNCTION, COUNT, ...) MOCK_CALL_RETURN_VOID_FAKE_ARG ## COUNT (P_FUNCTION, __VA_ARGS__)
#define MOCK_CALL_RETURN_VOID_FAKE_HELPER(P_FUNCTION, COUNT, ...)      MOCK_CALL_RETURN_VOID_FAKE_HELPER_IMPL(P_FUNCTION, COUNT, __VA_ARGS__)
#define MOCK_CALL_RETURN_VOID_IF_USED_AS_FAKE(P_FUNCTION, ...)         MOCK_CALL_RETURN_VOID_FAKE_HELPER(P_FUNCTION, N_ARGS(__VA_ARGS__), __VA_ARGS__)

#define MOCK_CALL_RETURN_FAKE_HELPER_IMPL(P_FUNCTION, COUNT, ...) MOCK_CALL_RETURN_FAKE_ARG ## COUNT (P_FUNCTION, __VA_ARGS__)
#define MOCK_CALL_RETURN_FAKE_HELPER(P_FUNCTION, COUNT, ...)      MOCK_CALL_RETURN_FAKE_HELPER_IMPL(P_FUNCTION, COUNT, __VA_ARGS__)
#define MOCK_CALL_RETURN_IF_USED_AS_FAKE(P_FUNCTION, ...)         MOCK_CALL_RETURN_FAKE_HELPER(P_FUNCTION, N_ARGS(__VA_ARGS__), __VA_ARGS__)

#define MOCK_CALL_ACTUAL_IMPL(P_MOCK, COUNT, ...)    MOCK_CALL_ACTUAL_ARG ## COUNT (P_MOCK, __VA_ARGS__)
#define MOCK_CALL_ACTUAL_HELPER(P_MOCK, COUNT, ...)  MOCK_CALL_ACTUAL_IMPL(P_MOCK, COUNT, __VA_ARGS__)
#define MOCK_CALL_ACTUAL(P_MOCK, ...)                MOCK_CALL_ACTUAL_HELPER(P_MOCK, N_ARGS(__VA_ARGS__), __VA_ARGS__)

void mock_call_expect_ex(uint32_t line_number, const char * p_func_name, mock_t ** pp_mock);
#define  mock_call_expect(fn, mock) mock_call_expect_ex(__LINE__, fn, mock)

/**@brief Function for clearing all expectation in the mock framework.
 *
 * @details A prematurely unit abort might result in the mock database not being cleaned up.
 *          This can potentially interfere with following test cases.
 *          To ensure that the mock db is properly initialized for a test case, then it is advise
 *          to clear the db at the start of a test case, by calling @ref mock_calls_clear.
 */
void mock_calls_clear(void);

/**@brief Function for verifying that all expected calls to mocks has occured.
 *
 * @details During a unit test several mock calls may be expected. During test execution the mock
 *          framework monitors the calls to each expected call. The mock framework can evaluate
 *          upon each mock call whether the call is expected.
 *          At the end of a unit test @ref mock_calls_verify must be executed to ensure that all
 *          expectation regarding mock calls are met. As example, function_A was expected to be
 *          called from the unit under test, but due to error in implementation, this never happened.
 *          This is validated when @ref mock_calls_verify  is executed.
 */
void mock_calls_verify(void);

bool mock_call_find(char const * const p_func_name, mock_t ** pp_mock);

bool mock_call_used_as_stub(char const * const p_file_name, char const * const p_function_name);

void mock_call_use_as_stub(const char * const p_name);

bool mock_call_used_as_fake(char const * const p_file_name, char const * const p_function_name);

void mock_call_use_as_fake(char * const p_name);

#endif // _MOCK_CONTROL_H_

