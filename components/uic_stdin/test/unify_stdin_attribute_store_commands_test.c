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
#include "unify_stdin_attribute_store_command_handler.h"
#include "uic_stdin_command_handler.h"
#include "unity.h"

// Mock includes
#include "attribute_store_configuration_mock.h"
#include "attribute_store_helper_mock.h"
#include "attribute_store_mock.h"
#include "attribute_store_type_registration_mock.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  unify_stdin_add_attribute_store_commands();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_attribute_store_configuration_set_auto_save_cooldown_interval()
{
  sl_status_t state;
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_cooldown_interval");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_cooldown_interval zzz");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_cooldown_interval 1,2");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_store_configuration_set_auto_save_cooldown_interval_Expect(1);
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_cooldown_interval 1");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  attribute_store_configuration_set_auto_save_cooldown_interval_Expect(999);
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_cooldown_interval 999");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  attribute_store_configuration_set_auto_save_cooldown_interval_Expect(0);
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_cooldown_interval 0");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_attribute_store_configuration_set_auto_save_safety_interval()
{
  sl_status_t state;
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_safety_interval");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_safety_interval zzz");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_safety_interval 1,2");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_store_configuration_set_auto_save_safety_interval_Expect(1);
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_safety_interval 1");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  attribute_store_configuration_set_auto_save_safety_interval_Expect(999);
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_safety_interval 999");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  attribute_store_configuration_set_auto_save_safety_interval_Expect(0);
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_auto_save_safety_interval 0");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_attribute_store_configuration_set_type_validation()
{
  sl_status_t state;
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_type_validation");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_type_validation zzz");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_type_validation 1,2");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_store_configuration_set_type_validation_Expect(true);
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_type_validation 1");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  attribute_store_configuration_set_type_validation_Expect(false);
  state = uic_stdin_handle_command(
    "attribute_store_configuration_set_type_validation 0");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_attribute_store_log()
{
  attribute_store_walk_tree_Ignore();

  attribute_store_get_root_IgnoreAndReturn(ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_get_node_type_IgnoreAndReturn(1);
  attribute_store_get_storage_type_IgnoreAndReturn(UNKNOWN_STORAGE_TYPE);
  attribute_store_is_value_defined_IgnoreAndReturn(false);
  attribute_store_get_type_name_IgnoreAndReturn("Hest");
  sl_status_t state = uic_stdin_handle_command("attribute_store_log");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_store_type_t node_type_id
    = 0x0003;  // Taken from ZPC's ATTRIBUTE_NODE_ID.
  attribute_store_get_type_by_name_ExpectAndReturn("NodeID", node_type_id);
  state = uic_stdin_handle_command("attribute_store_log NodeID,9");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("attribute_store_log FAIL");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);
}

void test_attribute_store_log_search()
{
  // Log search handeler needs a single argument; Data type. If the type is
  // unknown in the attribute_store, it failes.
  attribute_store_get_root_IgnoreAndReturn(ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_get_node_type_IgnoreAndReturn(1);
  attribute_store_get_storage_type_IgnoreAndReturn(UNKNOWN_STORAGE_TYPE);
  attribute_store_is_value_defined_IgnoreAndReturn(false);
  attribute_store_get_type_name_IgnoreAndReturn("Hest");

  sl_status_t state;
  state = uic_stdin_handle_command("attribute_store_log_search Hest");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_store_log_search");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);
}

void test_attribute_store_set_desired()
{
  sl_status_t state = uic_stdin_handle_command("attribute_store_set_desired");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_store_set_desired 34");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_store_set_desired 3fdsa083");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_store_set_desired 34,hello");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_store_set_desired_number_ExpectAndReturn(2394,
                                                     124,
                                                     SL_STATUS_NOT_READY);

  state = uic_stdin_handle_command("attribute_store_set_desired 2394,124");
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_READY, state);
}

void test_attribute_store_set_all_desired_types_invalid_node()
{
  sl_status_t state;
  state = uic_stdin_handle_command("attribute_store_set_all_desired_types");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_store_set_all_desired_types 34");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_store_get_root_IgnoreAndReturn(ATTRIBUTE_STORE_INVALID_NODE);

  state
    = uic_stdin_handle_command("attribute_store_set_all_desired_types 34,49");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command(
    "attribute_store_set_all_desired_types 3adsæfkm4,34fdahhtn4553");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_attribute_store_set_all_desired_types()
{
  // Assume a node space consisting of a parent node (ID 9) and its single child
  // (ID 18). The parent has a storage type of 34, and child has 99. A command
  // "set_all_desired_types 34,49" filters out nodes of type 34 and set their
  // desired values to 49. Since The child's type doesn't match, it remains
  // intact.

  sl_status_t state;
  attribute_store_get_root_IgnoreAndReturn(9);
  attribute_store_get_node_type_ExpectAndReturn(9, 34);
  attribute_store_set_desired_number_IgnoreAndReturn(SL_STATUS_OK);
  attribute_store_get_node_child_count_ExpectAndReturn(9, 1);
  attribute_store_get_node_child_ExpectAndReturn(9, 0, 18);
  attribute_store_get_node_type_ExpectAndReturn(18, 99);
  attribute_store_get_node_child_count_ExpectAndReturn(18, 0);
  attribute_store_get_node_child_count_ExpectAndReturn(9, 1);

  state
    = uic_stdin_handle_command("attribute_store_set_all_desired_types 34,49");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_attribute_store_set_reported()
{
  attribute_store_set_node_attribute_value_StopIgnore();

  sl_status_t state = uic_stdin_handle_command("attribute_store_set_reported");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_store_set_reported 34");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_store_set_reported 3fdsa083");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_store_set_reported 34,hello");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_store_set_reported_number_ExpectAndReturn(2394, 124, SL_STATUS_OK);

  state = uic_stdin_handle_command("attribute_store_set_reported 2394,124");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_attribute_store_set_reported_string()
{
  attribute_store_set_node_attribute_value_StopIgnore();

  sl_status_t state
    = uic_stdin_handle_command("attribute_store_set_reported_string");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state = uic_stdin_handle_command("attribute_store_set_reported_string 34");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  state
    = uic_stdin_handle_command("attribute_store_set_reported_string 3fdsa083");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_store_set_reported_string_ExpectAndReturn(43, NULL, SL_STATUS_OK);
  attribute_store_set_reported_string_IgnoreArg_string();
  state
    = uic_stdin_handle_command("attribute_store_set_reported_string 43,hello");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_attribute_store_log_node()
{
  sl_status_t state = uic_stdin_handle_command("attribute_store_log_node");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_store_walk_tree_Ignore();

  attribute_store_get_root_IgnoreAndReturn(ATTRIBUTE_STORE_INVALID_NODE);
  attribute_store_get_node_type_IgnoreAndReturn(1);
  attribute_store_get_storage_type_IgnoreAndReturn(UNKNOWN_STORAGE_TYPE);
  attribute_store_is_value_defined_IgnoreAndReturn(false);
  attribute_store_get_node_child_count_IgnoreAndReturn(0);
  attribute_store_get_type_name_IgnoreAndReturn("Node Type Name");

  state = uic_stdin_handle_command("attribute_store_log_node 1");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("attribute_store_log_node 1,1");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);
}

void test_attribute_store_delete_node()
{
  sl_status_t state = uic_stdin_handle_command("attribute_store_delete");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_store_delete_node_ExpectAndReturn(3, SL_STATUS_OK);
  state = uic_stdin_handle_command("attribute_store_delete 3");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("attribute_store_delete zyglyf");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);
}

void test_attribute_store_add_node()
{
  sl_status_t state = uic_stdin_handle_command("attribute_store_add");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);

  attribute_store_add_node_ExpectAndReturn(4, 3, SL_STATUS_OK);
  state = uic_stdin_handle_command("attribute_store_add 3,4");
  TEST_ASSERT_EQUAL(SL_STATUS_OK, state);

  state = uic_stdin_handle_command("attribute_store_add zyglyf");
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, state);
}
