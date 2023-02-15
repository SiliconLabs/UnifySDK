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
#include "zpc_stdin_command_handling.h"
#include "zpc_stdin_fixt.h"

// Unify Components
#include "datastore.h"
#include "attribute_store.h"
#include "attribute_store_helper.h"
#include "attribute_store_fixt.h"
#include "attribute_store_type_registration.h"

// Test includes
#include "unity.h"
#include "uic_stdin_test.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  // Setup CLI with ZPC application
  zpc_stdin_setup_fixt();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

void test_zpc_stdin_attribute_store_log_test()
{
  // Create a network will all kind of types. Then log it.
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

  attribute_store_node_t root_node = attribute_store_get_root();
  attribute_store_node_t node_1    = attribute_store_add_node(1, root_node);
  attribute_store_set_desired_as_reported(node_1);
  attribute_store_node_t node_2 = attribute_store_add_node(2, node_1);
  attribute_store_set_reported_number(node_2, 23);
  attribute_store_set_desired_number(node_2, 24);
  attribute_store_node_t node_3 = attribute_store_add_node(3, node_2);
  attribute_store_set_reported_number(node_3, 254);
  attribute_store_set_desired_number(node_3, 252);
  attribute_store_node_t node_4 = attribute_store_add_node(4, node_2);
  attribute_store_set_reported_number(node_4, 254);
  attribute_store_set_desired_number(node_4, 252);
  attribute_store_node_t node_5 = attribute_store_add_node(5, node_2);
  attribute_store_set_reported_number(node_5, 23254);
  attribute_store_set_desired_number(node_5, 23252);
  attribute_store_node_t node_6 = attribute_store_add_node(6, node_2);
  attribute_store_set_reported_number(node_6, 12392938);
  attribute_store_set_desired_number(node_6, 0);
  attribute_store_node_t node_7 = attribute_store_add_node(7, node_1);
  attribute_store_set_reported_number(node_7, 300);  // Should be clipped here.
  attribute_store_set_desired_number(node_7, 0);
  attribute_store_node_t node_8 = attribute_store_add_node(8, node_1);
  attribute_store_set_reported_number(node_8, 300);
  attribute_store_node_t node_9 = attribute_store_add_node(9, node_3);
  attribute_store_set_desired_number(node_9, 430);
  attribute_store_node_t node_10 = attribute_store_add_node(10, node_3);
  attribute_store_set_reported_number(node_10, -434530);
  attribute_store_set_desired_number(node_10, -430);
  attribute_store_node_t node_11 = attribute_store_add_node(11, node_3);
  attribute_store_set_reported_number(node_11, 3456.23945);
  attribute_store_set_desired_number(node_11, -0.0001);
  attribute_store_node_t node_12 = attribute_store_add_node(12, node_3);
  attribute_store_set_reported_number(node_12, ((double)1 / 3));
  attribute_store_set_desired_number(node_12, 0.9999999);
  attribute_store_node_t node_13 = attribute_store_add_node(13, node_3);
  attribute_store_set_reported_string(node_13, "Test String value");
  attribute_store_node_t node_14 = attribute_store_add_node(14, node_1);
  uint8_t byte_array[] = {1, 2, 5, 4, 5, 8, 10, 12, 14, 21, 52, 35, 9, 255};
  attribute_store_set_reported(node_14, byte_array, sizeof(byte_array));
  attribute_store_set_desired(node_14, byte_array, sizeof(byte_array));
  attribute_store_node_t node_15 = attribute_store_add_node(15, node_1);
  attribute_store_set_reported(node_15, byte_array, sizeof(byte_array));
  attribute_store_set_desired(node_15, byte_array, sizeof(byte_array));

  sl_status_t state;
  state = uic_stdin_handle_command("attribute_store_log_network");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("attribute_store_log_network 0");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);
}
