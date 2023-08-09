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
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "attribute_store_type_registration.h"

// Includes from other components
#include "sl_status.h"
#include "sl_log.h"
#include "config.h"
#include "datastore_fixt.h"

// Test includes
#include "unity.h"

// Generic includes
#include <float.h>

#define ATTRIBUTE_HOME_ID                            101
#define ATTRIBUTE_NODE_ID                            102
#define ATTRIBUTE_ENDPOINT_ID                        103
#define ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID 104
typedef uint32_t zwave_home_id_t;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_fixt_setup(":memory:");
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  datastore_fixt_teardown();
  return num_failures;
}

// Before every test, ensure to clean up the datastore file, else it will grow for ever
void setUp()
{
  // This trigger a deletion of everything before each test
  TEST_ASSERT_EQUAL(SL_STATUS_OK, attribute_store_init());
  attribute_store_delete_node(attribute_store_get_root());
}

// After every test, clear up the registered attribute types
void tearDown()
{
  attribute_store_teardown();
}

void test_attribute_store_read_value_helper()
{
  uint8_t test_values[] = {0x01, 0x02, 0x03, 0x04, 0x05};
  /////////////////////////////////////////////
  // Create a network
  /////////////////////////////////////////////
  attribute_store_node_t root_node;
  root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t new_node_1;
  new_node_1 = attribute_store_add_node(ATTRIBUTE_HOME_ID, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, new_node_1);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_node_attribute_value(new_node_1,
                                             REPORTED_ATTRIBUTE,
                                             test_values,
                                             sizeof(test_values)));

  attribute_store_node_t new_node_2;
  new_node_2 = attribute_store_add_node(ATTRIBUTE_NODE_ID, new_node_1);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, new_node_2);

  test_values[0] = 0xAA;  // This will be NodeID AA
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_node_attribute_value(new_node_2,
                                                             REPORTED_ATTRIBUTE,
                                                             test_values,
                                                             1));

  test_values[0] = 0x03;  // We also set a desired value for this guy
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_node_attribute_value(
                      new_node_2,
                      DESIRED_ATTRIBUTE,
                      test_values,
                      3));  // 3 bytes, so [0x03, 0x02, 0x03]

  attribute_store_node_t new_node_3;
  new_node_3 = attribute_store_add_node(ATTRIBUTE_NODE_ID, new_node_1);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, new_node_3);

  test_values[0] = 0x02;  // This will be NodeID 02
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_node_attribute_value(new_node_3,
                                                             REPORTED_ATTRIBUTE,
                                                             test_values,
                                                             1));

  attribute_store_node_t new_node_4;
  new_node_4 = attribute_store_add_node(ATTRIBUTE_ENDPOINT_ID, new_node_2);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, new_node_4);

  test_values[0] = 0x05;  // This will be endpoint 05
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_node_attribute_value(new_node_4,
                                                             REPORTED_ATTRIBUTE,
                                                             test_values,
                                                             1));

  attribute_store_node_t new_node_5;
  // Invalid node test, it will not be accepted, as only the root has no parent.
  new_node_5
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                               ATTRIBUTE_STORE_INVALID_NODE);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, new_node_5);

  new_node_5
    = attribute_store_add_node(ATTRIBUTE_COMMAND_CLASS_ASSOCIATION_GROUP_ID,
                               new_node_4);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, new_node_5);

  test_values[0] = 0x01;  // This will be association group ID 01
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_node_attribute_value(new_node_5,
                                                             REPORTED_ATTRIBUTE,
                                                             test_values,
                                                             1));

  zwave_home_id_t test_home_id = 0x11223344;
  attribute_store_node_t new_node_6;
  new_node_6 = attribute_store_add_node(ATTRIBUTE_HOME_ID, new_node_5);
  TEST_ASSERT_NOT_EQUAL(0, new_node_6);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_node_attribute_value(new_node_6,
                                             REPORTED_ATTRIBUTE,
                                             (uint8_t *)&test_home_id,
                                             sizeof(zwave_home_id_t)));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_node_attribute_value(new_node_6,
                                             DESIRED_ATTRIBUTE,
                                             (uint8_t *)&test_home_id,
                                             sizeof(zwave_home_id_t)));
  // Print out our network, it helps to see what are the expected values down below:
  attribute_store_log();

  /////////////////////////////////////////////
  // Test the is_value_matched function:
  /////////////////////////////////////////////
  TEST_ASSERT_FALSE(attribute_store_is_value_matched(new_node_5));

  TEST_ASSERT_TRUE(attribute_store_is_value_matched(new_node_6));

  /////////////////////////////////////////////
  // Test the is_value_defined function:
  /////////////////////////////////////////////
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(new_node_5, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(0, attribute_store_get_desired_size(new_node_5));
  TEST_ASSERT_EQUAL(1,
                    attribute_store_get_desired_else_reported_size(new_node_5));
  TEST_ASSERT_EQUAL(0, attribute_store_get_node_value_size(new_node_5, 0xFF));

  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(new_node_5, REPORTED_ATTRIBUTE));

  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(new_node_4, REPORTED_ATTRIBUTE));

  /////////////////////////////////////////////
  // Test the set desired to reported / and vice versa
  /////////////////////////////////////////////
  TEST_ASSERT_FALSE(attribute_store_is_value_matched(new_node_1));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_desired_as_reported(new_node_1));

  // Now value is matched
  TEST_ASSERT_TRUE(attribute_store_is_value_matched(new_node_1));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_node_attribute_value(new_node_1,
                                                             REPORTED_ATTRIBUTE,
                                                             NULL,
                                                             0));

  TEST_ASSERT_FALSE(attribute_store_is_value_matched(new_node_1));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_desired_as_reported(new_node_1));
  TEST_ASSERT_TRUE(attribute_store_is_value_matched(new_node_1));

  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_set_desired_as_reported(ATTRIBUTE_STORE_INVALID_NODE));

  // The other way around now
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_node_attribute_value(new_node_1,
                                             DESIRED_ATTRIBUTE,
                                             test_values,
                                             sizeof(test_values)));
  TEST_ASSERT_FALSE(attribute_store_is_value_matched(new_node_1));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_reported_as_desired(new_node_1));

  TEST_ASSERT_TRUE(attribute_store_is_value_matched(new_node_1));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_node_attribute_value(new_node_1,
                                                             DESIRED_ATTRIBUTE,
                                                             NULL,
                                                             0));

  TEST_ASSERT_FALSE(attribute_store_is_value_matched(new_node_1));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_reported_as_desired(new_node_1));
  TEST_ASSERT_TRUE(attribute_store_is_value_matched(new_node_1));

  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_set_reported_as_desired(ATTRIBUTE_STORE_INVALID_NODE));

  /////////////////////////////////////////////
  // Test the attribute_store_copy_value()
  /////////////////////////////////////////////
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    attribute_store_copy_value(ATTRIBUTE_STORE_INVALID_NODE,
                                               ATTRIBUTE_STORE_INVALID_NODE,
                                               DESIRED_OR_REPORTED_ATTRIBUTE));

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    attribute_store_copy_value(new_node_1,
                                               ATTRIBUTE_STORE_INVALID_NODE,
                                               REPORTED_ATTRIBUTE));
  // Copy to itself should work
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_copy_value(new_node_1, new_node_1, REPORTED_ATTRIBUTE));

  // new_node_1 reported value is undefined.
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_copy_value(new_node_1, new_node_2, REPORTED_ATTRIBUTE));

  // new_node_2 reported value must also be undefined undefined.
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(new_node_2, REPORTED_ATTRIBUTE));

  // new_node_2 desired value is 03 02 03. Copy to new_node_1
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_copy_value(new_node_2, new_node_1, DESIRED_ATTRIBUTE));

  // Verify that new_node_1 desired value is 03 02 03
  uint8_t received_value_size = 0;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_get_node_attribute_value(new_node_1,
                                             DESIRED_ATTRIBUTE,
                                             test_values,
                                             &received_value_size));
  TEST_ASSERT_EQUAL(3, received_value_size);
  TEST_ASSERT_EQUAL(03, test_values[0]);
  TEST_ASSERT_EQUAL(02, test_values[1]);
  TEST_ASSERT_EQUAL(03, test_values[2]);

  // Try with non-valid value state:
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    attribute_store_copy_value(new_node_2,
                                               new_node_1,
                                               DESIRED_OR_REPORTED_ATTRIBUTE));
}

void test_attribute_store_undefine_value_helpers()
{
  attribute_store_node_t root_node;
  root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(123, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  uint8_t value = 12;
  attribute_store_set_node_attribute_value(test_node,
                                           REPORTED_ATTRIBUTE,
                                           &value,
                                           sizeof(value));
  attribute_store_set_node_attribute_value(test_node,
                                           DESIRED_ATTRIBUTE,
                                           &value,
                                           sizeof(value));

  uint8_t received_value      = 0;
  uint8_t received_value_size = 1;

  attribute_store_undefine_reported(test_node);
  attribute_store_get_node_attribute_value(test_node,
                                           REPORTED_ATTRIBUTE,
                                           &received_value,
                                           &received_value_size);
  TEST_ASSERT_NOT_EQUAL(value, received_value);
  TEST_ASSERT_EQUAL(0, received_value_size);

  attribute_store_undefine_desired(test_node);
  attribute_store_get_node_attribute_value(test_node,
                                           DESIRED_ATTRIBUTE,
                                           &received_value,
                                           &received_value_size);
  TEST_ASSERT_NOT_EQUAL(value, received_value);
  TEST_ASSERT_EQUAL(0, received_value_size);
}

void test_attribute_store_set_value_helpers()
{
  attribute_store_node_t root_node;
  root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1233, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  uint8_t value = 12;
  attribute_store_set_reported(test_node, &value, sizeof(value));

  uint8_t received_value      = 0;
  uint8_t received_value_size = 0;
  attribute_store_get_node_attribute_value(test_node,
                                           REPORTED_ATTRIBUTE,
                                           &received_value,
                                           &received_value_size);

  TEST_ASSERT_EQUAL(sizeof(uint8_t), received_value_size);
  TEST_ASSERT_EQUAL(value, received_value);

  uint32_t new_value = 586;
  attribute_store_set_desired(test_node, &new_value, sizeof(new_value));

  uint32_t received_new_value = 586;
  attribute_store_get_node_attribute_value(test_node,
                                           DESIRED_ATTRIBUTE,
                                           (uint8_t *)&received_new_value,
                                           &received_value_size);

  TEST_ASSERT_EQUAL(sizeof(uint32_t), received_value_size);
  TEST_ASSERT_EQUAL(new_value, received_new_value);
}

void test_attribute_store_set_reported_string_happy_case()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1233, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  const char *string = "Hello";
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_reported_string(test_node, string));

  // Read back
  uint8_t received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH - 1]     = 10;
  uint8_t received_value_size                                  = 0;
  attribute_store_get_node_attribute_value(test_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&received_value,
                                           &received_value_size);

  TEST_ASSERT_EQUAL(6, received_value_size);
  TEST_ASSERT_EQUAL('H', received_value[0]);
  TEST_ASSERT_EQUAL('e', received_value[1]);
  TEST_ASSERT_EQUAL('l', received_value[2]);
  TEST_ASSERT_EQUAL('l', received_value[3]);
  TEST_ASSERT_EQUAL('o', received_value[4]);
  TEST_ASSERT_EQUAL('\0', received_value[5]);
  TEST_ASSERT_EQUAL(
    10,
    received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH
                   - 1]);  // not Appended NULL termination, string was correct
}

void test_attribute_store_set_string_from_null_pointer()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1233, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  const char *string_1 = NULL;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_reported_string(test_node, string_1));

  // Read back
  uint8_t received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  uint8_t received_value_size                                  = 0;
  attribute_store_get_node_attribute_value(test_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&received_value,
                                           &received_value_size);

  TEST_ASSERT_EQUAL_STRING("", (const char *)received_value);
}

void test_attribute_store_concatenate_reported_string_happy_case()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1233, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  const char *string_1 = "Hello";
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_reported_string(test_node, string_1));

  const char *string_2 = ", World!";
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_concatenate_to_reported_string(test_node, string_2));

  // Read back
  uint8_t received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH - 1]     = 10;
  uint8_t received_value_size                                  = 0;
  attribute_store_get_node_attribute_value(test_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&received_value,
                                           &received_value_size);

  TEST_ASSERT_EQUAL(14, received_value_size);
  TEST_ASSERT_EQUAL('H', received_value[0]);
  TEST_ASSERT_EQUAL('e', received_value[1]);
  TEST_ASSERT_EQUAL('l', received_value[2]);
  TEST_ASSERT_EQUAL('l', received_value[3]);
  TEST_ASSERT_EQUAL('o', received_value[4]);
  TEST_ASSERT_EQUAL(',', received_value[5]);
  TEST_ASSERT_EQUAL(' ', received_value[6]);
  TEST_ASSERT_EQUAL('W', received_value[7]);
  TEST_ASSERT_EQUAL('o', received_value[8]);
  TEST_ASSERT_EQUAL('r', received_value[9]);
  TEST_ASSERT_EQUAL('l', received_value[10]);
  TEST_ASSERT_EQUAL('d', received_value[11]);
  TEST_ASSERT_EQUAL('!', received_value[12]);
  TEST_ASSERT_EQUAL('\0', received_value[13]);
  TEST_ASSERT_EQUAL(
    10,
    received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH
                   - 1]);  // not Appended NULL termination, string was correct
}

void test_attribute_store_concatenate_reported_string_undefined_previous_value()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1233, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  const char *string_1 = "Hello";
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_concatenate_to_reported_string(test_node, string_1));

  // Read back
  uint8_t received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  uint8_t received_value_size                                  = 0;
  attribute_store_get_node_attribute_value(test_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&received_value,
                                           &received_value_size);

  TEST_ASSERT_EQUAL_STRING(string_1, (const char *)received_value);
}

void test_attribute_store_concatenate_reported_empty_string()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1233, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  const char *string_1 = "Hello";
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_reported_string(test_node, string_1));

  const char *string_2 = "";
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_concatenate_to_reported_string(test_node, string_2));

  // Read back
  uint8_t received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  uint8_t received_value_size                                  = 0;
  attribute_store_get_node_attribute_value(test_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&received_value,
                                           &received_value_size);

  TEST_ASSERT_EQUAL_STRING(string_1, (const char *)received_value);
}

void test_attribute_store_concatenate_overflow()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1233, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  const char *string_1
    = "Hello, this is a long string. It aims at checking that we do not choke "
      "if a string larger than the maximum capacity of the attribute store is "
      "attempted to be saved. Ideally, it will ... ";
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_reported_string(test_node, string_1));

  const char *string_2 = "clip this string and append a NULL termination, so "
                         "the program and everybody is happy :-)";
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_concatenate_to_reported_string(test_node, string_2));

  // Read back
  uint8_t received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  uint8_t received_value_size                                  = 0;
  attribute_store_get_node_attribute_value(test_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&received_value,
                                           &received_value_size);

  TEST_ASSERT_EQUAL_STRING(
    "Hello, this is a long string. It aims at checking that we do not choke if "
    "a string larger than the maximum capacity of the attribute store is "
    "attempted to be saved. Ideally, it will ... clip this string and append a "
    "NULL termination, so the program and e",
    (const char *)received_value);
}

void test_attribute_store_set_reported_string_no_null_termination_case()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1233, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  char string[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = "Hello";
  // Erase the Null termination, fill with non-zero stuffs
  for (uint8_t i = 5; i < ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH; i++) {
    string[i] = 'B';
  }

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_reported_string(test_node, string));

  // Read back
  uint8_t received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH - 1]     = 10;
  uint8_t received_value_size                                  = 0;
  attribute_store_get_node_attribute_value(test_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&received_value,
                                           &received_value_size);

  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH, received_value_size);
  TEST_ASSERT_EQUAL('H', received_value[0]);
  TEST_ASSERT_EQUAL('e', received_value[1]);
  TEST_ASSERT_EQUAL('l', received_value[2]);
  TEST_ASSERT_EQUAL('l', received_value[3]);
  TEST_ASSERT_EQUAL('o', received_value[4]);
  for (uint8_t i = 5; i < ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH - 1; i++) {
    TEST_ASSERT_EQUAL('B', received_value[i]);
  }
  TEST_ASSERT_EQUAL('\0',
                    received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH
                                   - 1]);  // Appended NULL termination
}

void test_attribute_store_get_reported_string_happy_case()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1233, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  char string[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = "HelloAoXPC";

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_reported_string(test_node, string));

  // Read back
  char received_string[20];
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_get_reported_string(test_node, received_string, 20));

  TEST_ASSERT_EQUAL_STRING(string, received_string);
}

void test_attribute_store_get_desired_else_reported_string_happy_case()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1233, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  char string[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH]
    = "HelloConfigurationParameters";

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_node_attribute_value(test_node,
                                                             DESIRED_ATTRIBUTE,
                                                             (uint8_t *)string,
                                                             29));

  // Read back
  char received_string[29];
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_get_desired_else_reported_string(test_node,
                                                     received_string,
                                                     29));

  TEST_ASSERT_EQUAL_STRING(string, received_string);
}

void test_attribute_store_get_desired_else_reported_string_only_reported_defined()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1233, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  char string[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = "HelloZigPC";

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_reported_string(test_node, string));

  // Read back
  char received_string[11];
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_get_desired_else_reported_string(test_node,
                                                     received_string,
                                                     11));

  TEST_ASSERT_EQUAL_STRING(string, received_string);
}

void test_attribute_store_get_reported_string_null_pointer()
{
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_get_reported_string(ATTRIBUTE_STORE_INVALID_NODE, NULL, 1));
}

void test_attribute_store_get_reported_string_no_size()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1233, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  char string[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = "HelloAoXPC";

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_reported_string(test_node, string));

  char received_string[20];
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_get_reported_string(test_node, received_string, 0));
}

void test_attribute_store_get_reported_string_undefined_value()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1233, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  // Value is undefined
  // Read back
  char received_string[20];
  received_string[0] = 'a';
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_get_reported_string(test_node, received_string, 20));

  TEST_ASSERT_EQUAL('\0',
                    received_string[0]);  // Appended NULL termination
}

void test_attribute_store_get_reported_string_too_small_buffer()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1233, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  char string[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH]
    = "This is a string that is way too long";

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_reported_string(test_node, string));

  // Value is undefined
  // Read back
  char received_string[10];
  received_string[0] = 'a';
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_get_reported_string(test_node, received_string, 10));

  TEST_ASSERT_EQUAL('\0',
                    received_string[0]);  // Appended NULL termination
}

void test_attribute_store_get_reported_string_no_null_termination_in_attribute_store()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1233, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  uint8_t value[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  // Write the data that has no NULL termination
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_node_attribute_value(test_node,
                                             REPORTED_ATTRIBUTE,
                                             value,
                                             (uint8_t)sizeof(value)));

  // Read back
  char received_string[10];
  received_string[0] = 'a';
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_get_reported_string(test_node, received_string, 10));

  TEST_ASSERT_EQUAL('\0',
                    received_string[0]);  // Appended NULL termination
}

void test_attribute_store_set_child_reported()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node
    = attribute_store_add_node(0xAAAA, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  // Try setting the value, a child should be created here.
  uint32_t value = 9393;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_child_reported(test_node,
                                                       0xBBBB,
                                                       &value,
                                                       sizeof(value)));

  attribute_store_node_t created_node
    = attribute_store_get_node_child_by_type(test_node, 0xBBBB, 0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, created_node);

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(created_node, DESIRED_ATTRIBUTE));

  uint32_t received_value     = 0;
  uint8_t received_value_size = 0;
  attribute_store_get_node_attribute_value(created_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&received_value,
                                           &received_value_size);

  TEST_ASSERT_EQUAL(sizeof(value), received_value_size);
  TEST_ASSERT_EQUAL(value, received_value);

  // Try modifying the value, the child already exists
  value = 42;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_child_reported(test_node,
                                                       0xBBBB,
                                                       &value,
                                                       sizeof(value)));

  // Read back, from the created node in the first set
  attribute_store_get_node_attribute_value(created_node,
                                           REPORTED_ATTRIBUTE,
                                           (uint8_t *)&received_value,
                                           &received_value_size);

  TEST_ASSERT_EQUAL(sizeof(value), received_value_size);
  TEST_ASSERT_EQUAL(value, received_value);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_get_child_reported(test_node,
                                                       0xBBBB,
                                                       &received_value,
                                                       sizeof(received_value)));

  TEST_ASSERT_EQUAL(sizeof(value), received_value_size);
  TEST_ASSERT_EQUAL(value, received_value);

  // Finally, just for fun, call the API with an invalid node.
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_set_child_reported(ATTRIBUTE_STORE_INVALID_NODE,
                                       0xBBBB,
                                       &value,
                                       sizeof(value)));

  // Also try the invalid type
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_set_child_reported(test_node,
                                       ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
                                       &value,
                                       sizeof(value)));
}

void test_attribute_store_set_child_desired()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node
    = attribute_store_add_node(0xAAAA, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  // Try setting the value, a child should be created here.
  uint32_t value = 9393;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_child_desired(test_node,
                                                      0xBBBB,
                                                      &value,
                                                      sizeof(value)));

  attribute_store_node_t created_node
    = attribute_store_get_node_child_by_type(test_node, 0xBBBB, 0);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, created_node);

  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(created_node, REPORTED_ATTRIBUTE));

  uint32_t received_value     = 0;
  uint8_t received_value_size = 0;
  attribute_store_get_node_attribute_value(created_node,
                                           DESIRED_ATTRIBUTE,
                                           (uint8_t *)&received_value,
                                           &received_value_size);

  TEST_ASSERT_EQUAL(sizeof(value), received_value_size);
  TEST_ASSERT_EQUAL(value, received_value);

  // Try modifying the value, the child already exists
  value = 42;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_child_desired(test_node,
                                                      0xBBBB,
                                                      &value,
                                                      sizeof(value)));

  // Read back, from the created node in the first set
  attribute_store_get_node_attribute_value(created_node,
                                           DESIRED_ATTRIBUTE,
                                           (uint8_t *)&received_value,
                                           &received_value_size);

  TEST_ASSERT_EQUAL(sizeof(value), received_value_size);
  TEST_ASSERT_EQUAL(value, received_value);

  // Finally, just for fun, call the API with an invalid node.
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_set_child_desired(ATTRIBUTE_STORE_INVALID_NODE,
                                      0xBBBB,
                                      &value,
                                      sizeof(value)));

  // Also try the invalid type
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_set_child_desired(test_node,
                                      ATTRIBUTE_STORE_INVALID_ATTRIBUTE_TYPE,
                                      &value,
                                      sizeof(value)));
}

void test_attribute_store_get_child_reported()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node
    = attribute_store_add_node(0xAAAA, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  // No child, nothing will be retrieved
  uint32_t received_value = 0;
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    attribute_store_get_child_reported(test_node,
                                                       0xBBBB,
                                                       &received_value,
                                                       sizeof(received_value)));
  TEST_ASSERT_EQUAL(0, received_value);

  // Set the value of a child:
  uint32_t value = 9393;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_child_reported(test_node,
                                                       0xBBBB,
                                                       &value,
                                                       sizeof(value)));

  /// Read it back:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_get_child_reported(test_node,
                                                       0xBBBB,
                                                       &received_value,
                                                       sizeof(received_value)));

  TEST_ASSERT_EQUAL(value, received_value);

  // Try with the wrong size:
  received_value = 0;
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_get_child_reported(test_node, 0xBBBB, &received_value, 23));

  TEST_ASSERT_EQUAL(0, received_value);

  attribute_store_teardown();
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    attribute_store_get_child_reported(test_node,
                                                       0xBBBB,
                                                       &received_value,
                                                       sizeof(received_value)));
}

void test_attribute_store_delete_all_children()
{
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_delete_all_children(ATTRIBUTE_STORE_INVALID_NODE));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_delete_all_children(attribute_store_get_root()));

  // Create a few nodes
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node_1
    = attribute_store_add_node(0xAAAA, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node_1);
  attribute_store_node_t test_node_2
    = attribute_store_add_node(0x02, test_node_1);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node_2);
  attribute_store_node_t test_node_3
    = attribute_store_add_node(0x02, test_node_1);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node_3);

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_delete_all_children(test_node_1));

  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(test_node_1));
}

void test_attribute_store_walk_tree()
{
  // Create a few nodes
  uint8_t value                    = 0;
  attribute_store_node_t root_node = attribute_store_get_root();
  attribute_store_set_reported(root_node, &value, sizeof(value));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(root_node, REPORTED_ATTRIBUTE));

  attribute_store_node_t test_node_1
    = attribute_store_add_node(0xAAAA, root_node);
  attribute_store_set_reported(test_node_1, &value, sizeof(value));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(test_node_1, REPORTED_ATTRIBUTE));

  attribute_store_node_t test_node_2
    = attribute_store_add_node(0xAAAA, root_node);
  attribute_store_set_reported(test_node_2, &value, sizeof(value));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(test_node_2, REPORTED_ATTRIBUTE));

  attribute_store_node_t test_node_3
    = attribute_store_add_node(0xAAAA, test_node_1);
  attribute_store_set_reported(test_node_3, &value, sizeof(value));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(test_node_3, REPORTED_ATTRIBUTE));

  attribute_store_node_t test_node_4
    = attribute_store_add_node(0xAAAA, test_node_3);
  attribute_store_set_reported(test_node_4, &value, sizeof(value));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(test_node_4, REPORTED_ATTRIBUTE));

  attribute_store_node_t test_node_5
    = attribute_store_add_node(0xAAAA, test_node_4);
  attribute_store_set_reported(test_node_5, &value, sizeof(value));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(test_node_5, REPORTED_ATTRIBUTE));

  attribute_store_node_t test_node_6
    = attribute_store_add_node(0xAAAA, test_node_4);
  attribute_store_set_reported(test_node_6, &value, sizeof(value));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(test_node_6, REPORTED_ATTRIBUTE));

  attribute_store_node_t test_node_7
    = attribute_store_add_node(0xAAAA, test_node_4);
  attribute_store_set_reported(test_node_7, &value, sizeof(value));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(test_node_7, REPORTED_ATTRIBUTE));

  attribute_store_node_t test_node_8
    = attribute_store_add_node(0xAAAA, test_node_3);
  attribute_store_set_reported(test_node_8, &value, sizeof(value));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(test_node_8, REPORTED_ATTRIBUTE));

  attribute_store_node_t test_node_9
    = attribute_store_add_node(0xAAAA, test_node_3);
  attribute_store_set_reported(test_node_9, &value, sizeof(value));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(test_node_9, REPORTED_ATTRIBUTE));

  attribute_store_node_t test_node_10
    = attribute_store_add_node(0xAAAA, test_node_3);
  attribute_store_set_reported(test_node_10, &value, sizeof(value));
  TEST_ASSERT_TRUE(
    attribute_store_is_value_defined(test_node_10, REPORTED_ATTRIBUTE));

  // Invoke the magic function, it will undefine all reported in our tree:
  attribute_store_walk_tree(root_node, &attribute_store_undefine_reported);

  // Everything should be undefined:
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(root_node, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(test_node_1, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(test_node_2, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(test_node_3, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(test_node_4, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(test_node_5, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(test_node_6, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(test_node_7, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(test_node_8, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(test_node_9, REPORTED_ATTRIBUTE));
  TEST_ASSERT_FALSE(
    attribute_store_is_value_defined(test_node_10, REPORTED_ATTRIBUTE));
}

void test_attribute_store_get_reported_test()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1233, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  const int64_t value = -12394;

  // Write the value
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_node_attribute_value(test_node,
                                             REPORTED_ATTRIBUTE,
                                             (uint8_t *)&value,
                                             (uint8_t)sizeof(value)));

  // Read back with the get reported:
  int64_t received_value = 0;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_get_reported(test_node,
                                                 &received_value,
                                                 sizeof(received_value)));

  TEST_ASSERT_EQUAL(value, received_value);

  // Should give the same with desired_else_reported:
  received_value = 0;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_get_desired_else_reported(test_node,
                                              &received_value,
                                              sizeof(received_value)));

  TEST_ASSERT_EQUAL(value, received_value);

  // Finally, try with the desired value only being set:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_desired_as_reported(test_node));
  attribute_store_undefine_reported(test_node);

  received_value = 0;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_get_desired_else_reported(test_node,
                                              &received_value,
                                              sizeof(received_value)));
  TEST_ASSERT_EQUAL(value, received_value);
}

void test_attribute_store_get_reported_number_test()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1233, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  const int32_t value1 = -12394;

  // Write the value
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_node_attribute_value(test_node,
                                             REPORTED_ATTRIBUTE,
                                             (uint8_t *)&value1,
                                             (uint8_t)sizeof(value1)));

  // Read back with the get reported:
  TEST_ASSERT_EQUAL(value1, attribute_store_get_reported_number(test_node));

  // Try again after registering the attribute type:
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_type(1233, "Test name", 0, I64_STORAGE_TYPE));
  const int64_t value2 = -12394;
  // Write the value
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_node_attribute_value(test_node,
                                             REPORTED_ATTRIBUTE,
                                             (uint8_t *)&value2,
                                             (uint8_t)sizeof(value2)));

  TEST_ASSERT_EQUAL(value2, attribute_store_get_reported_number(test_node));
}

void test_attribute_store_get_float_number_test()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1234, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  const float value1 = -12394;

  // Read an undefined value:
  TEST_ASSERT_EQUAL_FLOAT(FLT_MIN,
                          attribute_store_get_reported_number(test_node));

  // Now make it official that it is a Float:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_register_type(1234,
                                                  "Test name 1234",
                                                  0,
                                                  FLOAT_STORAGE_TYPE));

  // Write the value
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_node_attribute_value(test_node,
                                             DESIRED_ATTRIBUTE,
                                             (uint8_t *)&value1,
                                             (uint8_t)sizeof(value1)));

  TEST_ASSERT_EQUAL(value1, attribute_store_get_desired_number(test_node));
}

void test_attribute_store_get_number_from_string_test()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_type(99, "", 0, C_STRING_STORAGE_TYPE));

  attribute_store_node_t test_node = attribute_store_add_node(99, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    attribute_store_set_reported_number(99, 2134));

  // Now read back:
  TEST_ASSERT_EQUAL_FLOAT(FLT_MIN,
                          attribute_store_get_reported_number(test_node));
}

void test_attribute_store_get_number_test()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_type(100, "", 0, U32_STORAGE_TYPE));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_type(101, "", 0, U64_STORAGE_TYPE));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_register_type(102, "", 0, I8_STORAGE_TYPE));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_type(103, "", 0, I16_STORAGE_TYPE));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_type(104, "", 0, I32_STORAGE_TYPE));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_register_type(105, "", 0, DOUBLE_STORAGE_TYPE));

  attribute_store_node_t u32_node    = attribute_store_add_node(100, root_node);
  attribute_store_node_t u64_node    = attribute_store_add_node(101, root_node);
  attribute_store_node_t i8_node     = attribute_store_add_node(102, root_node);
  attribute_store_node_t i16_node    = attribute_store_add_node(103, root_node);
  attribute_store_node_t i32_node    = attribute_store_add_node(104, root_node);
  attribute_store_node_t double_node = attribute_store_add_node(105, root_node);

  attribute_store_set_reported_number(u32_node, 123);
  TEST_ASSERT_EQUAL(123, attribute_store_get_reported_number(u32_node));
  attribute_store_set_reported_number(u64_node, 123);
  TEST_ASSERT_EQUAL(123, attribute_store_get_reported_number(u64_node));
  attribute_store_set_reported_number(i8_node, 123);
  TEST_ASSERT_EQUAL(123, attribute_store_get_reported_number(i8_node));
  attribute_store_set_reported_number(i16_node, 123);
  TEST_ASSERT_EQUAL(123, attribute_store_get_reported_number(i16_node));
  attribute_store_set_reported_number(i32_node, 123);
  TEST_ASSERT_EQUAL(123, attribute_store_get_reported_number(i32_node));
  attribute_store_set_reported_number(double_node, 123);
  TEST_ASSERT_EQUAL(123, attribute_store_get_reported_number(double_node));

  // Now write some wrong size data in the node
  const uint8_t garbage[]
    = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 34, 0xFF, 2, 100, 101, 102, 103};
  attribute_store_set_reported(u32_node, garbage, sizeof(garbage));
  attribute_store_set_reported(u64_node, garbage, sizeof(garbage));
  attribute_store_set_reported(i8_node, garbage, sizeof(garbage));
  attribute_store_set_reported(i16_node, garbage, sizeof(garbage));
  attribute_store_set_reported(i32_node, garbage, sizeof(garbage));
  attribute_store_set_reported(double_node, garbage, sizeof(garbage));

  // Reading their number will fail:
  TEST_ASSERT_EQUAL_FLOAT(FLT_MIN,
                          attribute_store_get_reported_number(u32_node));
  TEST_ASSERT_EQUAL_FLOAT(FLT_MIN,
                          attribute_store_get_reported_number(u64_node));
  TEST_ASSERT_EQUAL_FLOAT(FLT_MIN,
                          attribute_store_get_reported_number(i8_node));
  TEST_ASSERT_EQUAL_FLOAT(FLT_MIN,
                          attribute_store_get_reported_number(i16_node));
  TEST_ASSERT_EQUAL_FLOAT(FLT_MIN,
                          attribute_store_get_reported_number(i32_node));
  TEST_ASSERT_EQUAL_FLOAT(FLT_MIN,
                          attribute_store_get_reported_number(double_node));
}

void test_attribute_store_set_desired_string_happy_case()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node
    = attribute_store_add_node(154672, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  const char *string = "Hello";
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_desired_string(test_node, string));

  // Read back
  uint8_t received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH - 1]     = 10;
  uint8_t received_value_size                                  = 0;
  attribute_store_get_node_attribute_value(test_node,
                                           DESIRED_ATTRIBUTE,
                                           (uint8_t *)&received_value,
                                           &received_value_size);

  TEST_ASSERT_EQUAL(6, received_value_size);
  TEST_ASSERT_EQUAL('H', received_value[0]);
  TEST_ASSERT_EQUAL('e', received_value[1]);
  TEST_ASSERT_EQUAL('l', received_value[2]);
  TEST_ASSERT_EQUAL('l', received_value[3]);
  TEST_ASSERT_EQUAL('o', received_value[4]);
  TEST_ASSERT_EQUAL('\0', received_value[5]);
  TEST_ASSERT_EQUAL(
    10,
    received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH
                   - 1]);  // not Appended NULL termination, string was correct
}

void test_attribute_store_emplace()
{
  attribute_store_type_t test_type = 154672;
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  // Add a first node with the test type.
  int32_t value = -234;
  attribute_store_node_t test_node
    = attribute_store_add_node(test_type, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);
  attribute_store_set_reported(test_node, &value, sizeof(value));

  // Add a second node with the test type, no value.
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                        attribute_store_add_node(test_type, root_node));

  // Try to emplace a new node with the same value:
  attribute_store_node_t emplaced_node
    = attribute_store_emplace(root_node, test_type, &value, sizeof(value));

  TEST_ASSERT_EQUAL(test_node, emplaced_node);

  // Modify the value:
  value += 1;
  emplaced_node
    = attribute_store_emplace(root_node, test_type, &value, sizeof(value));
  TEST_ASSERT_NOT_EQUAL(test_node, emplaced_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, emplaced_node);

  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(root_node));
}

void test_attribute_store_emplace_desired()
{
  attribute_store_type_t test_type = 154672;
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  // Add a first node with the test type.
  int32_t value = -234;
  attribute_store_node_t test_node
    = attribute_store_add_node(test_type, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);
  attribute_store_set_desired(test_node, &value, sizeof(value));

  // Add a second node with the test type, no value.
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE,
                        attribute_store_add_node(test_type, root_node));

  // Try to emplace a new node with the same value:
  attribute_store_node_t emplaced_node
    = attribute_store_emplace_desired(root_node,
                                      test_type,
                                      &value,
                                      sizeof(value));

  TEST_ASSERT_EQUAL(test_node, emplaced_node);

  // Modify the value:
  value += 1;
  emplaced_node = attribute_store_emplace_desired(root_node,
                                                  test_type,
                                                  &value,
                                                  sizeof(value));
  TEST_ASSERT_NOT_EQUAL(test_node, emplaced_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, emplaced_node);

  TEST_ASSERT_EQUAL(3, attribute_store_get_node_child_count(root_node));
}

void test_attribute_store_set_child_reported_only_if_missing()
{
  attribute_store_type_t test_type = 154672;
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  // Add a first node with the test type.
  int8_t value = -94;
  attribute_store_node_t test_node
    = attribute_store_add_node(test_type, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);
  attribute_store_set_reported(test_node, &value, sizeof(value));
  value = 0;

  // The node already exists, nothing should change:
  TEST_ASSERT_EQUAL(
    SL_STATUS_ALREADY_EXISTS,
    attribute_store_set_child_reported_only_if_missing(root_node,
                                                       test_type,
                                                       &value,
                                                       sizeof(value)));

  // Check the value was unchanged
  attribute_store_get_reported(test_node, &value, sizeof(value));
  TEST_ASSERT_EQUAL(-94, value);

  // Now try again with a non-existing child:
  test_type = 3;
  value     = 3;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_child_reported_only_if_missing(root_node,
                                                       test_type,
                                                       &value,
                                                       sizeof(value)));
  value = 0;
  attribute_store_node_t new_node
    = attribute_store_get_first_child_by_type(root_node, test_type);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, new_node);
  attribute_store_get_reported(new_node, &value, sizeof(value));
  TEST_ASSERT_EQUAL(3, value);
}

void test_attribute_store_append_to_reported_happy_case()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1233, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  const uint8_t report_1[] = {23, 34, 45};
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_reported(test_node, report_1, sizeof(report_1)));

  const uint8_t report_2[] = {24, 35, 46};
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_append_to_reported(test_node, report_2, sizeof(report_2)));

  // Read back
  uint8_t received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH - 1]     = 10;
  uint8_t received_value_size                                  = 0;
  attribute_store_get_node_attribute_value(test_node,
                                           REPORTED_ATTRIBUTE,
                                           received_value,
                                           &received_value_size);

  TEST_ASSERT_EQUAL(6, received_value_size);
  TEST_ASSERT_EQUAL(23, received_value[0]);
  TEST_ASSERT_EQUAL(34, received_value[1]);
  TEST_ASSERT_EQUAL(45, received_value[2]);
  TEST_ASSERT_EQUAL(24, received_value[3]);
  TEST_ASSERT_EQUAL(35, received_value[4]);
  TEST_ASSERT_EQUAL(46, received_value[5]);
  TEST_ASSERT_EQUAL(
    10,
    received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH
                   - 1]);  // not Appended NULL termination, string was correct
}

void test_attribute_store_append_to_reported_undefined_previous_value()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1233, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  const uint8_t report_1[] = {23, 34, 45};
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_append_to_reported(test_node, report_1, sizeof(report_1)));

  // Read back
  uint8_t received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  uint8_t received_value_size                                  = 0;
  attribute_store_get_node_attribute_value(test_node,
                                           REPORTED_ATTRIBUTE,
                                           received_value,
                                           &received_value_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(report_1, received_value, received_value_size);
}

void test_attribute_store_append_to_reported_empty_string()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1233, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  const uint8_t report_1[] = {23, 34, 45};
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_reported(test_node, report_1, sizeof(report_1)));

  const uint8_t report_2[] = {};
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_append_to_reported(test_node, report_2, sizeof(report_2)));

  // Read back
  uint8_t received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  uint8_t received_value_size                                  = 0;
  attribute_store_get_node_attribute_value(test_node,
                                           REPORTED_ATTRIBUTE,
                                           received_value,
                                           &received_value_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(report_1, received_value, received_value_size);
}

void test_attribute_store_append_to_reported_large_array()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  attribute_store_node_t test_node = attribute_store_add_node(1233, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  const uint8_t report_1[] = {23, 34, 45};
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_reported(test_node, report_1, sizeof(report_1)));

  const uint8_t report_2[] = {12, 13, 14};
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_append_to_reported(test_node, report_2, 255));

  // Read back
  uint8_t received_value[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  uint8_t received_value_size                                  = 0;
  attribute_store_get_node_attribute_value(test_node,
                                           REPORTED_ATTRIBUTE,
                                           received_value,
                                           &received_value_size);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(report_1, received_value, received_value_size);
}

void test_attribute_store_set_child_reported_only_if_exists()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  const uint8_t value[] = {0x01, 0xFF, 0xA2};
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_set_child_reported_only_if_exists(root_node,
                                                      1,
                                                      value,
                                                      sizeof(value)));

  attribute_store_node_t test_node
    = attribute_store_get_first_child_by_type(root_node, 1);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  test_node = attribute_store_add_node(1, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(test_node));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_child_reported_only_if_exists(root_node,
                                                      1,
                                                      value,
                                                      sizeof(value)));
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(test_node));
}

void test_attribute_store_set_child_desired_only_if_exists()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  const uint8_t value[] = {0x01, 0xFF, 0xA2};
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    attribute_store_set_child_desired_only_if_exists(root_node,
                                                     1,
                                                     value,
                                                     sizeof(value)));

  attribute_store_node_t test_node
    = attribute_store_get_first_child_by_type(root_node, 1);
  TEST_ASSERT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  test_node = attribute_store_add_node(1, root_node);
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, test_node);

  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(test_node));
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_child_desired_only_if_exists(root_node,
                                                     1,
                                                     value,
                                                     sizeof(value)));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(test_node));
}

void test_set_all_children_reported()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  const int value = 43;

  // First check that nothing happens if we try to set all children when none exist.
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_all_children_reported(root_node,
                                                              10,
                                                              &value,
                                                              sizeof(int)));

  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(root_node));

  // Create a child with the wrong type, same, nothing will happen.
  attribute_store_node_t node_5 = attribute_store_add_node(11, root_node);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_all_children_reported(root_node,
                                                              10,
                                                              &value,
                                                              sizeof(int)));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(node_5));

  // Now create the children with the right type:
  attribute_store_node_t node_1 = attribute_store_add_node(10, root_node);
  attribute_store_node_t node_2 = attribute_store_add_node(10, root_node);
  attribute_store_node_t node_3 = attribute_store_add_node(10, root_node);
  attribute_store_node_t node_4 = attribute_store_add_node(10, root_node);
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(node_1));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(node_2));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(node_3));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(node_4));

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    attribute_store_set_all_children_reported(root_node,
                                                              10,
                                                              &value,
                                                              sizeof(int)));
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(node_1));
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(node_2));
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(node_3));
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(node_4));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(node_5));

  TEST_ASSERT_EQUAL(value, attribute_store_get_reported_number(node_1));
  TEST_ASSERT_EQUAL(value, attribute_store_get_reported_number(node_2));
  TEST_ASSERT_EQUAL(value, attribute_store_get_reported_number(node_3));
  TEST_ASSERT_EQUAL(value, attribute_store_get_reported_number(node_4));
}

void test_set_child_reported_only_if_undefined()
{
  attribute_store_node_t root_node = attribute_store_get_root();
  TEST_ASSERT_NOT_EQUAL(ATTRIBUTE_STORE_INVALID_NODE, root_node);

  int value = 43;

  // First check that nothing happens if we try to set all children when none exist.
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_child_reported_only_if_undefined(root_node,
                                                         10,
                                                         &value,
                                                         sizeof(int)));

  TEST_ASSERT_EQUAL(0, attribute_store_get_node_child_count(root_node));

  // Create a child with the wrong type, same, nothing will happen.
  attribute_store_node_t node_5 = attribute_store_add_node(11, root_node);
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_child_reported_only_if_undefined(root_node,
                                                         10,
                                                         &value,
                                                         sizeof(int)));
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(node_5));

  // Now create the children with the right type:
  attribute_store_node_t node_1 = attribute_store_add_node(10, root_node);
  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(node_1));
  TEST_ASSERT_EQUAL(0, attribute_store_get_reported_size(node_1));

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_child_reported_only_if_undefined(root_node,
                                                         10,
                                                         &value,
                                                         sizeof(int)));
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(node_1));
  TEST_ASSERT_EQUAL(value, attribute_store_get_reported_number(node_1));

  // Try with a new value
  value = 50;
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    attribute_store_set_child_reported_only_if_undefined(root_node,
                                                         10,
                                                         &value,
                                                         sizeof(int)));
  TEST_ASSERT_NOT_EQUAL(value, attribute_store_get_reported_number(node_1));
  TEST_ASSERT_EQUAL(43, attribute_store_get_reported_number(node_1));
}
