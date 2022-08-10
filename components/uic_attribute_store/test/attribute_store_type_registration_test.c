/******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

void test_attribute_store_type_registration()
{
  // Try registerting some attributes
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_type(1, "first", 2, UNKNOWN_STORAGE_TYPE));

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    attribute_store_register_type(1,
                                                  "first again",
                                                  3,
                                                  BYTE_ARRAY_STORAGE_TYPE));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_type(2, "second", 3, U16_STORAGE_TYPE));

  // Cannot register attribute type as their own parent:
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_register_type(3, "NodeID", 3, C_STRING_STORAGE_TYPE));

  // Cannot register an invalid storage type:
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_register_type(3, "NodeID", 4, INVALID_STORAGE_TYPE));

  // Verify if we can read back what iattribute_store_get_type_names registered and what is not.
  TEST_ASSERT_EQUAL(true, attribute_store_is_type_registered(1));
  TEST_ASSERT_EQUAL(true, attribute_store_is_type_registered(2));
  TEST_ASSERT_EQUAL(false, attribute_store_is_type_registered(3));
  TEST_ASSERT_EQUAL(false, attribute_store_is_type_registered(4));
  TEST_ASSERT_EQUAL(false, attribute_store_is_type_registered(12345));

  // Test attribute_store_get_type_by_name
  TEST_ASSERT_EQUAL(1, attribute_store_get_type_by_name("first"));
  TEST_ASSERT_EQUAL(2, attribute_store_get_type_by_name("second"));

  // Adding another type with the same name will not be a problem
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_type(10, "first", 3, U8_STORAGE_TYPE));
  TEST_ASSERT_EQUAL(1, attribute_store_get_type_by_name("first"));

  // Modify names:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    attribute_store_set_type_name(12, "New name"));
  TEST_ASSERT_EQUAL(SL_STATUS_OK, attribute_store_set_type_name(1, "HomeID"));

  // Now get the attribute type with name first will be attribute 10, because 1 has been renamed:
  TEST_ASSERT_EQUAL(10, attribute_store_get_type_by_name("first"));

  // Test attribute_store_get_type_name
  TEST_ASSERT_EQUAL_STRING("HomeID", attribute_store_get_type_name(1));
  TEST_ASSERT_EQUAL_STRING("second", attribute_store_get_type_name(2));
  TEST_ASSERT_EQUAL_STRING("Unknown 0x00000003",
                           attribute_store_get_type_name(3));
  TEST_ASSERT_EQUAL_STRING("first", attribute_store_get_type_name(10));

  // Test get storage type:
  TEST_ASSERT_EQUAL(UNKNOWN_STORAGE_TYPE,
                    attribute_store_get_storage_type(999));
  TEST_ASSERT_EQUAL(UNKNOWN_STORAGE_TYPE, attribute_store_get_storage_type(0));
  TEST_ASSERT_EQUAL(UNKNOWN_STORAGE_TYPE, attribute_store_get_storage_type(1));
  TEST_ASSERT_EQUAL(U16_STORAGE_TYPE, attribute_store_get_storage_type(2));

  // Test the parent type
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
                    attribute_store_get_registered_parent_type(999));
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
                    attribute_store_get_registered_parent_type(0));
  TEST_ASSERT_EQUAL(2, attribute_store_get_registered_parent_type(1));
  TEST_ASSERT_EQUAL(3, attribute_store_get_registered_parent_type(2));
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
                    attribute_store_get_registered_parent_type(3));
}
