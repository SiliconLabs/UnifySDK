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
// Components being tested
#include "unify_stdin_attribute_store_command_handler.h"
#include "uic_stdin_command_handler.h"

// Unify Components
#include "datastore.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "attribute_store_type_registration.h"

// Test includes
#include "unity.h"

attribute_store_node_t nodes[16];

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  unify_stdin_add_attribute_store_commands();

  // Create a network with all kind of types. Then log it.
  nodes[0] = attribute_store_get_root();

  for (int i = 1; i < 16; i++) {
    nodes[i]
      = attribute_store_add_node(i, nodes[((i % 2 == 0) ? i - 1 : i) >> 1]);
  }

  attribute_store_register_type(1, "Empty (root) node", 0, EMPTY_STORAGE_TYPE);
  attribute_store_register_type(2, "Unknown (i32)", 0, UNKNOWN_STORAGE_TYPE);
  attribute_store_register_type(3, "U8 Value", 0, U8_STORAGE_TYPE);
  attribute_store_register_type(4, "U16 Value", 0, U16_STORAGE_TYPE);
  attribute_store_register_type(5, "U32 Value", 0, U32_STORAGE_TYPE);
  attribute_store_register_type(6, "U64 Value", 0, U64_STORAGE_TYPE);
  attribute_store_register_type(7, "I8 Value", 0, I8_STORAGE_TYPE);
  attribute_store_register_type(8, "I16 Value", 0, I16_STORAGE_TYPE);
  attribute_store_register_type(9, "I32 Value", 0, I32_STORAGE_TYPE);
  attribute_store_register_type(10, "I64 Value", 0, I64_STORAGE_TYPE);
  attribute_store_register_type(11, "Float Value", 0, FLOAT_STORAGE_TYPE);
  attribute_store_register_type(12, "Double Value", 0, DOUBLE_STORAGE_TYPE);
  attribute_store_register_type(13, "String", 0, C_STRING_STORAGE_TYPE);
  attribute_store_register_type(14, "byte array", 0, BYTE_ARRAY_STORAGE_TYPE);
  attribute_store_register_type(15,
                                "Fixed sized Struct",
                                0,
                                FIXED_SIZE_STRUCT_STORAGE_TYPE);

  attribute_store_set_desired_as_reported(nodes[1]);
  attribute_store_set_reported_number(nodes[2], 23);
  attribute_store_set_desired_number(nodes[2], 24);
  attribute_store_set_reported_number(nodes[3], 254);
  attribute_store_set_desired_number(nodes[3], 252);
  attribute_store_set_reported_number(nodes[4], 254);
  attribute_store_set_desired_number(nodes[4], 252);
  attribute_store_set_reported_number(nodes[5], 23254);
  attribute_store_set_desired_number(nodes[5], 23252);
  attribute_store_set_reported_number(nodes[6], 12392938);
  attribute_store_set_desired_number(nodes[6], 0);
  attribute_store_set_reported_number(nodes[7], 300);
  attribute_store_set_desired_number(nodes[7], 0);
  attribute_store_set_reported_number(nodes[8], 300);
  attribute_store_set_desired_number(nodes[9], 430);
  attribute_store_set_reported_number(nodes[10], -434530);
  attribute_store_set_desired_number(nodes[10], -430);
  attribute_store_set_reported_number(nodes[11], 3456.23945);
  attribute_store_set_desired_number(nodes[11], -0.0001);
  attribute_store_set_reported_number(nodes[12], ((double)1 / 3));
  attribute_store_set_desired_number(nodes[12], 0.9999999);
  attribute_store_set_reported_string(nodes[13], "Test String value");
  uint8_t byte_array[] = {1, 2, 5, 4, 5, 8, 10, 12, 14, 21, 52, 35, 9, 255};
  attribute_store_set_reported(nodes[14], byte_array, sizeof(byte_array));
  attribute_store_set_desired(nodes[14], byte_array, sizeof(byte_array));
  attribute_store_set_reported(nodes[15], byte_array, sizeof(byte_array));
  attribute_store_set_desired(nodes[15], byte_array, sizeof(byte_array));
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_attribute_store_log_search()
{
  sl_status_t state;
  state = uic_stdin_handle_command("attribute_store_log");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("attribute_store_log Unknown (i32),23");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("attribute_store_log_search Next");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_store_log_search U8");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("attribute_store_log_node 3");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("attribute_store_log_node 3,1");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_attribute_store_log_cstr()
{
  sl_status_t state = uic_stdin_handle_command("attribute_store_log_node 13");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_attribute_store_log_number()
{
  sl_status_t state = uic_stdin_handle_command("attribute_store_log_node 2");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("attribute_store_log U32 Value,23254");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_attribute_store_log_unknown()
{
  sl_status_t state = uic_stdin_handle_command("attribute_store_log_node 14");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("attribute_store_log_node 15");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}