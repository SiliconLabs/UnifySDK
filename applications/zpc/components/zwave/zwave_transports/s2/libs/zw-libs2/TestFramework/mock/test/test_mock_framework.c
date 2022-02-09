/* © 2014 Silicon Laboratories Inc.
 */

/*
 * test_kex.c
 *
 *  Created on: Aug 12, 2015
 *      Author: trasmussen
 */

/** Extensions to mock framework:
  *  - Setting of output values:
  *    - Structs, due to nature of structs, this must be handled by the mock itself.
  *  - void functions (in mock), when used as stub and default value in error, done, needs testing.
  */

#include <mock_control.h>
#include <stdint.h>
#include <string.h>
#include "unity.h"
#include "example_test_mock.h"

static bool     m_callback_executed = false;
static uint32_t m_callback_test_number;

void callback_func_used_for_mock_verification(uint32_t test_number)
{
  m_callback_executed = true;
  m_callback_test_number = test_number;
}

/* Test that ensures the mock framework and db  are correctly configured :-)
 * This test verifies normal mock operations with following compare rules:
 * Compare Strict   : The value received in the mock must match exactly the value expected (default setting)
 * Compare Any      : Any value will be expepted in the mock
 * Compare Delta    : The value +/- the delta will be expepted in the mock
 * Expected outcome: Test pass
 */
void test_mock_framework_compare_rules_simple() {
  mock_t * p_mock;

  // Expected values.
  const uint8_t expected_return_value = 0;

  // Clearing the mock db.
  mock_calls_clear();

  // Compare strict (default setting)
  mock_call_expect(TO_STR(mock_function_simple_input), &p_mock);
  p_mock->expect_arg[0].value = 0x12;        // Default rule: Compare strict.
  p_mock->expect_arg[1].value = 0x1234;      // Default rule: Compare strict.
  p_mock->expect_arg[2].value = 0x12345678;  // Default rule: Compare strict.
  p_mock->return_code.value   = 0;

  mock_call_expect(TO_STR(mock_function_simple_input), &p_mock);
  p_mock->compare_rule_arg[0] = COMPARE_DELTA;
  p_mock->compare_rule_arg[1] = COMPARE_STRICT;
  p_mock->compare_rule_arg[2] = COMPARE_ANY;
  p_mock->expect_arg[0].value = 10;
  p_mock->delta_arg[0].value  = 2;
  p_mock->expect_arg[1].value = 0x12;
  p_mock->return_code.value   = 0;

  mock_call_expect(TO_STR(mock_function_simple_input), &p_mock);
  p_mock->compare_rule_arg[0] = COMPARE_DELTA;
  p_mock->compare_rule_arg[1] = COMPARE_DELTA;
  p_mock->compare_rule_arg[2] = COMPARE_DELTA;
  p_mock->expect_arg[0].value = 0x50;
  p_mock->delta_arg[0].value  = 0x10;
  p_mock->expect_arg[1].value = 0x5050;
  p_mock->delta_arg[1].value  = 0x500;
  p_mock->expect_arg[2].value = 0x12121212;
  p_mock->delta_arg[2].value  = 0x100000;
  p_mock->return_code.value   = 0;

  // When starting the timer we expect to receive the first handle.
  uint8_t retval = mock_function_simple_input(0x12, 0x1234, 0x12345678);
  TEST_ASSERT_EQUAL_INT(expected_return_value, retval);

  retval = mock_function_simple_input(12, 0x12, 15);
  TEST_ASSERT_EQUAL_INT(expected_return_value, retval);

  retval = mock_function_simple_input(0x40, 0x5500, 0x12021212);
  TEST_ASSERT_EQUAL_INT(expected_return_value, retval);

  mock_calls_verify();
}

/* Test that ensures the mock framework and db  are correctly configured :-)
 * This test ensures that is the mock function is configured to return an error for error handling
 * verification, then arguments are not verified.
 *
 * Expected outcome: Test pass
 */
void test_mock_framework_error_return() {
  mock_t * p_mock;

  // Expected values.
  const uint8_t expected_return_value = 0xFF;

  // Clearing the mock db.
  mock_calls_clear();

  // Expected mock calls and corresponding setup.
  // 1) TimerStart, which returns an error.
  mock_call_expect(TO_STR(mock_function_simple_input), &p_mock);
  p_mock->error_code.value = 255;

  // When calling the mock we expect to receive an error back to ensure the mock behavois as defined.
  uint8_t retval = mock_function_simple_input(1, 2, 3);
  TEST_ASSERT_EQUAL_INT(expected_return_value, retval);

  mock_calls_verify();
}

/* Test that ensures the mock framework and db  are correctly configured :-)
 *
 */
void test_mock_framework_stub_function()
{
  // Expected mock calls and corresponding setup.
  mock_call_use_as_stub(TO_STR(mock_function_simple_input));

  mock_function_simple_input(1, 2 , 3); // When used as stub, then anything should be accepted. Return value will be the default encoded in the stub.
  mock_function_simple_input(0x12, 0x9876, 0x98765432); // When used as stub, then anything should be accepted. Return value will be the default encoded in the stub.
  mock_function_simple_input(0, 0, 0); // When used as stub, then anything should be accepted. Return value will be the default encoded in the stub.

  mock_calls_verify();
}

/* Test that ensures the mock framework and db  are correctly configured :-)
 *
 * This test ensures that a given mock file is completely used as a stub on all functions.
 */
void test_mock_framework_stub_file()
{
  uint32_t   number = 10;
  uint32_t * p_number = &number;
  void     * p_void   = &number;

  // Clearing the mock db.
  mock_calls_clear();

  // Expected mock calls and corresponding setup.
  mock_call_use_as_stub("example_test_mock.c");

  mock_function_simple_input(1, 2 , 3); // When used as stub, then anything should be accepted. Return value will be the default encoded in the stub.
  mock_function_simple_input(0x12, 0x9876, 0x98765432); // When used as stub, then anything should be accepted. Return value will be the default encoded in the stub.
  mock_function_simple_input(0, 0, 0); // When used as stub, then anything should be accepted. Return value will be the default encoded in the stub.
  mock_function_pointer_input(p_void, p_number, callback_func_used_for_mock_verification); // When used as stub, then anything should be accepted. Return value will be the default encoded in the stub.

  mock_calls_verify();
}

void test_mock_framework_array()
{
  mock_t   * p_mock;
  // This is the expected array that will be specified as expected in p_mock.
  uint8_t    expected_array[10]     = {9, 8, 7, 6, 5, 4, 3, 2, 1, 9};
  uint32_t   expected_length        = 10;
  // This is the actual array that will be passed to the function call (mock) and will internally
  // within the mock be compared against the expected.
  uint8_t    actual_array[10]       = {9, 8, 7, 6, 5, 4, 3, 2, 1, 9};
  uint32_t   actual_length          = 10;

  // This is the out array that will be specified as output in p_mock, the mock will copy the
  // content to the parameter passed in the function call.
  uint8_t    out_array[10]          = {9, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  uint32_t   out_length             = 10;

  // This is the return array which contains no data, but is expected to be filled with out array
  // in p_mock, the mock is expected to copy the content of out_array to the parameter passed in
  // the function call (return_array).
  uint8_t    return_array[20];
  uint32_t   return_length      = 20;

  // This is the expected array that whichi is used in this test to ensure that what was returned
  // in return array as expected.
  uint8_t    expected_out_array[10] = {9, 1, 2, 3, 4, 5, 6, 7, 8, 9};;
  uint32_t   expected_out_length    = 10;

  // Clearing the mock db.
  mock_calls_clear();

  mock_call_expect(TO_STR(mock_function_array), &p_mock);
  p_mock->expect_arg[0].pointer = expected_array;     // Default rule: Compare strict.
  p_mock->expect_arg[1].value   = expected_length;    // Default rule: Compare strict.
  p_mock->expect_arg[3].value   = return_length;      // Default rule: Compare strict.
  // Control the output from the mock when mock_function_array(...) is called.
  p_mock->output_arg[2].pointer = out_array;     // Output: Reversed array.
  p_mock->output_arg[3].value   = out_length;    // Output: Reversed array.

  p_mock->return_code.value     = 0;

  mock_function_array(actual_array, actual_length, return_array, &return_length);

  // Validating that the mock returned the correct output.
  TEST_ASSERT_EQUAL_UINT32(expected_out_length, return_length);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_out_array, return_array, expected_out_length);

  mock_calls_verify();
}


/* Test that ensures the mock framework and db  are correctly configured :-)
 *
 */
void test_mock_framework_pointer()
{
  mock_t * p_mock;
  // Clearing the mock db.
  mock_calls_clear();

  // As we are emulating a unit under test providing an internal function pointer, we don't have the knowledge on which.
  // It is just expected that the function pointer is NOT NULL.
  // Thereafter we execute whatever actual argument that was stored to ensure the functionality was indeed as expected.
  mock_call_expect(TO_STR(mock_function_pointer_input), &p_mock);
  p_mock->compare_rule_arg[0] = COMPARE_ANY;
  p_mock->compare_rule_arg[1] = COMPARE_ANY;
  p_mock->compare_rule_arg[2] = COMPARE_NOT_NULL; // This is where we expect the function pointer tro be passed and retrieved.
  p_mock->return_code.value   = 0;

  mock_function_pointer_input(NULL, NULL, callback_func_used_for_mock_verification);

  // Potential value which can be fetched, e.g. to get a callback handle inside the mock or post validation of a value/struct.
  // We try and fetch actual_arg[2] from the mock, which should contain argument 2 above (callback_func_used_for_mock_verification),
  // in case the mock is correctly created and then cast it.
  mock_callback_function_t cb = (mock_callback_function_t)p_mock->actual_arg[2].pointer;
  TEST_ASSERT_EQUAL_PTR(callback_func_used_for_mock_verification, cb);

  // By calling the callback function here, then it is ensured that the actual callback passed to
  // the mock, and received back through the actual_arg list are the same.
  // The function pointers has already been compared so this is an additional verification.
  // Values are not correct until the mock has been called so we verify m_callback_executed is FALSE.
  TEST_ASSERT_FALSE(m_callback_executed);
  cb(0x34325354);
  TEST_ASSERT_TRUE(m_callback_executed);
  TEST_ASSERT_EQUAL_UINT32(0x34325354, m_callback_test_number);

  mock_calls_verify();
}

void mock_function_void(void);
/* Test that ensures the mock framework and db  are correctly configured :-)
 *
 */
void test_mock_framework_void_function()
{
  mock_t * p_mock;
  // Clearing the mock db.
  mock_calls_clear();

  // As we are emulating a unit under test providing an internal function pointer, we don't have the knowledge on which.
  // It is just expected that the function pointer is NOT NULL.
  // Thereafter we execute whatever actual argument that was stored to ensure the functionality was indeed as expected.
  mock_call_expect(TO_STR(mock_function_void), &p_mock);
  mock_call_expect(TO_STR(mock_function_void), &p_mock);
  mock_call_expect(TO_STR(mock_function_void), &p_mock);

  mock_function_void();
  mock_function_void();
  mock_function_void();

  mock_calls_verify();
}

/** TODO Test array embedded in struct. */
void test_mock_framework_struct() {
  mock_t * p_mock;

  test_mock_struct_t expect_struct_2 =
  {
    .member_byte = 0xAB,
    .member      = 0x23455432,
    .union_member = {.submember_b = 0x3333}
  };

  test_mock_struct_t actual_struct_1;

  test_mock_struct_t actual_struct_2 =
  {
    .member_byte = 0xAB,
    .member      = 0x23455432,
    .union_member = {.submember_b = 0x3333}
  };

  // Clearing the mock db.
  mock_calls_clear();

  // Compare strict (default setting)
  mock_call_expect(TO_STR(mock_function_struct), &p_mock);
  p_mock->compare_rule_arg[0] = COMPARE_ANY;

  mock_call_expect(TO_STR(mock_function_struct), &p_mock);
  p_mock->expect_arg[0].pointer = &expect_struct_2;

  mock_function_struct(&actual_struct_1);

  mock_function_struct(&actual_struct_2);

  mock_calls_verify();
}

/* Test that ensures the framework and cmake files are correctly configured :-) */
/* Seconde test to ensure the framework and cmake files are correctly configured :-)
 * Expected outcome: Test pass
 */
void test_mock_first_build_pass() {
  TEST_ASSERT_TRUE(1);
}

/* Test that ensures that fake implementation of a function is correctly executed.
 * When the mock is setup as a fake then it is expected that data will be copied from buffer_in to buffer_out.
 * Expected outcome: Test pass
 *                   Buffer_out == buffer_in
 */
void test_mock_fake_handling() {
  uint8_t  buffer_in[]       = {0xDE, 0xAD, 0xBE, 0xEF};
  uint8_t  buffer_out[]      = {0xCA, 0xFE, 0xBA, 0xBE};
  uint32_t buffer_out_length = sizeof(buffer_out);
  test_mock_struct_t test_struct = {.member_byte             = 0xA5,
                                    .member                  = 0xDEADC0DE,
                                    .union_member.submember_b = 0xDEADBEEF
                                    };

  // Clearing the mock db.
  mock_calls_clear();

  // Setup the array function to be use its fake implementation (memcpy)
  mock_call_use_as_fake(TO_STR(mock_function_array));
  mock_call_use_as_fake(TO_STR(mock_function_struct));

  uint32_t compare_result = memcmp(buffer_in, buffer_out, sizeof(buffer_in));
  TEST_ASSERT_TRUE(compare_result != 0);

  // Calling mock_function_array(...) should result in the fake implementation to be used, thus
  // content of buffer_in should be copied to buffer_out.
  mock_function_array(buffer_in, sizeof(buffer_in), buffer_out, &buffer_out_length);

  TEST_ASSERT_EQUAL_UINT32(sizeof(buffer_in), buffer_out_length);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(buffer_in, buffer_out, sizeof(buffer_in));

  // Calling mock_function_array(...) should result in the fake implementation to be used, thus
  // content of buffer_in should be copied to buffer_out.
  mock_function_array(buffer_in, sizeof(buffer_in), buffer_out, &buffer_out_length);

  TEST_ASSERT_EQUAL_UINT32(sizeof(buffer_in), buffer_out_length);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(buffer_in, buffer_out, sizeof(buffer_in));

  // Calling mock_function_struct(...) should result in the fake implementation to be used, thus
  // content of test_struct should be update with +1, +2, +3, as seen in the fake implementation.
  mock_function_struct(&test_struct);
  TEST_ASSERT_EQUAL_UINT8(0xA6, test_struct.member_byte);
  TEST_ASSERT_EQUAL_UINT32(0xDEADC0E0, test_struct.member);
  TEST_ASSERT_EQUAL_UINT32(0xDEADBEF2, test_struct.union_member.submember_b);

  mock_calls_verify();
}

