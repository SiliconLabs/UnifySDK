/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
// Includes from this component
#include "attribute_store_fixt.h"
#include "attribute_store_type_registration.h"
#include "attribute_store_validation.h"
#include "attribute_store_configuration.h"

// Test includes
#include "unity.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

// Before every test, ensure to clean up the datastore file, else it will grow for ever
void setUp()
{
  // This trigger a deletion of everything before each test
  TEST_ASSERT_EQUAL(SL_STATUS_OK, attribute_store_init());
  attribute_store_delete_node(attribute_store_get_root());
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

// After every test, clear up the registered attribute types
void tearDown()
{
  attribute_store_teardown();
}

void test_attribute_store_uint8_t_validation_test()
{
  // Try registerting some attributes
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_register_type(1, "", 0, U8_STORAGE_TYPE));

  attribute_store_configuration_set_type_validation(false);

  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(uint8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(uint16_t)));

  attribute_store_configuration_set_type_validation(true);

  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(uint8_t)));
  TEST_ASSERT_FALSE(is_write_operation_valid(1, NULL, sizeof(uint16_t)));
}

void test_attribute_store_uint16_t_validation_test()
{
  // Try registerting some attributes
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_register_type(1, "", 0, U16_STORAGE_TYPE));

  attribute_store_configuration_set_type_validation(false);

  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(uint8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(uint16_t)));

  attribute_store_configuration_set_type_validation(true);

  TEST_ASSERT_FALSE(is_write_operation_valid(1, NULL, sizeof(uint8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(uint16_t)));
}

void test_attribute_store_uint32_t_validation_test()
{
  // Try registerting some attributes
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_register_type(1, "", 0, U32_STORAGE_TYPE));

  attribute_store_configuration_set_type_validation(false);

  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(uint8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(uint16_t)));

  attribute_store_configuration_set_type_validation(true);

  TEST_ASSERT_FALSE(is_write_operation_valid(1, NULL, sizeof(uint8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(uint32_t)));
}

void test_attribute_store_uint64_t_validation_test()
{
  // Try registerting some attributes
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_register_type(1, "", 0, U64_STORAGE_TYPE));

  attribute_store_configuration_set_type_validation(false);

  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(uint8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(uint16_t)));

  attribute_store_configuration_set_type_validation(true);

  TEST_ASSERT_FALSE(is_write_operation_valid(1, NULL, sizeof(uint8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(uint64_t)));
}

void test_attribute_store_int8_t_validation_test()
{
  // Try registerting some attributes
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_register_type(1, "", 0, I8_STORAGE_TYPE));

  attribute_store_configuration_set_type_validation(false);

  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(int8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(int16_t)));

  attribute_store_configuration_set_type_validation(true);

  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(int8_t)));
  TEST_ASSERT_FALSE(is_write_operation_valid(1, NULL, sizeof(int16_t)));
}

void test_attribute_store_int16_t_validation_test()
{
  // Try registerting some attributes
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_register_type(1, "", 0, I16_STORAGE_TYPE));

  attribute_store_configuration_set_type_validation(false);

  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(int8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(int16_t)));

  attribute_store_configuration_set_type_validation(true);

  TEST_ASSERT_FALSE(is_write_operation_valid(1, NULL, sizeof(int8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(int16_t)));
}

void test_attribute_store_int32_t_validation_test()
{
  // Try registerting some attributes
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_register_type(1, "", 0, I32_STORAGE_TYPE));

  attribute_store_configuration_set_type_validation(false);

  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(int8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(int16_t)));

  attribute_store_configuration_set_type_validation(true);

  TEST_ASSERT_FALSE(is_write_operation_valid(1, NULL, sizeof(int8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(int32_t)));
}

void test_attribute_store_int64_t_validation_test()
{
  // Try registerting some attributes
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_register_type(1, "", 0, I64_STORAGE_TYPE));

  attribute_store_configuration_set_type_validation(false);

  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(int8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(int16_t)));

  attribute_store_configuration_set_type_validation(true);

  TEST_ASSERT_FALSE(is_write_operation_valid(1, NULL, sizeof(int8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(int64_t)));
}

void test_attribute_store_unkonwn_type_validation_test()
{
  attribute_store_configuration_set_type_validation(false);

  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(int8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(int16_t)));

  attribute_store_configuration_set_type_validation(true);

  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(int8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(int64_t)));
}

void test_attribute_store_float_validation_test()
{
  // Try registerting some attributes
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_type(1, "", 0, FLOAT_STORAGE_TYPE));

  attribute_store_configuration_set_type_validation(false);

  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(int8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(float)));

  attribute_store_configuration_set_type_validation(true);

  TEST_ASSERT_FALSE(is_write_operation_valid(1, NULL, sizeof(int8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(float)));
}

void test_attribute_store_double_validation_test()
{
  // Try registerting some attributes
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_type(1, "", 0, DOUBLE_STORAGE_TYPE));

  attribute_store_configuration_set_type_validation(false);

  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(int8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(double)));

  attribute_store_configuration_set_type_validation(true);

  TEST_ASSERT_FALSE(is_write_operation_valid(1, NULL, sizeof(int8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(double)));
}

void test_attribute_store_empty_type_validation_test()
{
  // Try registerting some attributes
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_type(1, "", 0, EMPTY_STORAGE_TYPE));

  attribute_store_configuration_set_type_validation(false);

  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(int8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(double)));

  attribute_store_configuration_set_type_validation(true);

  TEST_ASSERT_FALSE(is_write_operation_valid(1, NULL, sizeof(int8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, 0));
}

void test_attribute_store_c_string_validation_test()
{
  // Try registerting some attributes
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_type(1, "", 0, C_STRING_STORAGE_TYPE));

  attribute_store_configuration_set_type_validation(false);

  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(int8_t)));
  TEST_ASSERT_TRUE(is_write_operation_valid(1, NULL, sizeof(double)));

  attribute_store_configuration_set_type_validation(true);

  const uint8_t test_invalid_string[] = {1, 2, 3, 4, 5, 6, 0, 4};
  const uint8_t test_valid_string[]   = {1, 2, 3, 4, 5, 6, 0};

  TEST_ASSERT_FALSE(is_write_operation_valid(1,
                                             test_invalid_string,
                                             sizeof(test_invalid_string)));
  TEST_ASSERT_TRUE(
    is_write_operation_valid(1, test_valid_string, sizeof(test_valid_string)));
}

void test_attribute_store_node_addition_validation_test()
{
  // Try registerting some attributes
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_type(1, "", 0, UNKNOWN_STORAGE_TYPE));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_type(2, "", 3, UNKNOWN_STORAGE_TYPE));

  attribute_store_configuration_set_type_validation(false);

  TEST_ASSERT_TRUE(is_node_addition_valid(1, 2));
  TEST_ASSERT_TRUE(is_node_addition_valid(2, 1));
  TEST_ASSERT_TRUE(is_node_addition_valid(2, 3));

  attribute_store_configuration_set_type_validation(true);

  TEST_ASSERT_TRUE(is_node_addition_valid(1, 2));
  TEST_ASSERT_FALSE(is_node_addition_valid(2, 1));
  TEST_ASSERT_TRUE(is_node_addition_valid(2, 3));
}