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

#include "unity.h"

#include "zigpc_group_map.h"

/**
 * @brief Setup the test suite (called once before all test_xxx functions are called)
 *
 */
void suiteSetUp(void) {}

/**
 * @brief Teardown the test suite (called once after all test_xxx functions are called)
 *
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/**
 * @brief Setup before each test case
 *
 */
void setUp(void) {}

/**
 * @brief Teardown after each test case
 *
 */
void tearDown(void) {}

void test_retrieving_member_count_of_untracked_groupid(void)
{
  // ARRANGE

  // ACT
  size_t count = zigpc_group_map_get_memberlist_count(0x10);
  // ASSERT
  TEST_ASSERT_EQUAL(0U, count);
}

void test_retrieving_member_list_of_untracked_groupid(void)
{
  // ARRANGE
  zigpc_group_member_t list[2];

  // ACT
  sl_status_t status = zigpc_group_map_get_memberlist_items(0x10, list, 2);
  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_NOT_FOUND, status);
}

void test_adding_reported_group_should_add_member_for_groupid(void)
{
  // ARRANGE
  zigbee_group_id_t add_group_id = 0x20;
  zigpc_group_member_t get_list[1];
  zigpc_group_member_t add_member = {
    .eui64       = "\xDE\xAD\xBE\xEF\x02\x34\x7E\xAB",
    .endpoint_id = 2,
  };

  // ACT
  sl_status_t status = zigpc_group_map_add_new(add_group_id, add_member, true);
  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(1, zigpc_group_map_get_memberlist_count(add_group_id));

  status = zigpc_group_map_get_memberlist_items(add_group_id, get_list, 1);
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_MEMORY(&add_member,
                           &get_list,
                           sizeof(zigpc_group_member_t));
}

void test_removing_reported_group_should_remove_member_for_groupid(void)
{
  // ARRANGE
  zigbee_group_id_t rem_group_id = 0x20;
  zigpc_group_member_t get_list[1];
  zigpc_group_member_t rem_member = {
    .eui64       = "\xDE\xAD\xBE\xEF\x02\x34\x7E\xAB",
    .endpoint_id = 2,
  };

  // ACT
  sl_status_t status
    = zigpc_group_map_remove_group(rem_group_id, rem_member, true);
  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL(0, zigpc_group_map_get_memberlist_count(rem_group_id));

  status = zigpc_group_map_get_memberlist_items(rem_group_id, get_list, 1);
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_NOT_FOUND, status);
}

void test_adding_multiple_members_to_reported_groups(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_group_id_t group_id = 0x30;
  const size_t group_count   = 3;
  zigpc_group_member_t get_list[3];
  zigpc_group_member_t add_member[3]
    = {{
         .eui64       = "\x12\x23\x34\x45\x56\x67\x78\x89",
         .endpoint_id = 1,
       },
       {
         .eui64       = "\x12\x23\x34\x45\x56\x67\x78\x89",
         .endpoint_id = 2,
       },
       {
         .eui64       = "\x0F\x2D\xEA\x03\x99\x3D\x21\x09",
         .endpoint_id = 4,

       }};

  // ACT
  for (size_t i = 0; i < group_count; i++) {
    status = zigpc_group_map_add_new(group_id, add_member[i], true);
    TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
  }

  // ASSERT
  TEST_ASSERT_EQUAL(group_count,
                    zigpc_group_map_get_memberlist_count(group_id));

  status
    = zigpc_group_map_get_memberlist_items(group_id, get_list, group_count);
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);

  // The order of the array retrieved is dependent on how it's stored in cache
  TEST_ASSERT_EQUAL_MEMORY(&add_member[2],
                           &get_list[0],
                           sizeof(zigpc_group_member_t));
  TEST_ASSERT_EQUAL_MEMORY(&add_member[1],
                           &get_list[1],
                           sizeof(zigpc_group_member_t));
  TEST_ASSERT_EQUAL_MEMORY(&add_member[0],
                           &get_list[2],
                           sizeof(zigpc_group_member_t));
}

void test_adding_members_to_different_groupid(void)
{
  // ARRANGE
  sl_status_t status;
  zigbee_group_id_t group_id = 0x40;
  const size_t group_count   = 2;
  zigpc_group_member_t get_list[group_count];
  zigpc_group_member_t add_member[2]
    = {{
         .eui64       = "\x12\x23\x34\x45\x56\x67\x78\x89",
         .endpoint_id = 1,
       },
       {
         .eui64       = "\x0F\x2D\xEA\x03\x99\x3D\x21\x09",
         .endpoint_id = 4,

       }};

  // ACT
  for (size_t i = 0; i < group_count; i++) {
    status = zigpc_group_map_add_new(group_id, add_member[i], true);
    TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
  }

  // ASSERT
  TEST_ASSERT_EQUAL(group_count,
                    zigpc_group_map_get_memberlist_count(group_id));

  status
    = zigpc_group_map_get_memberlist_items(group_id, get_list, group_count);
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);

  // The order of the array retrieved is dependent on how it's stored in cache
  TEST_ASSERT_EQUAL_MEMORY(&add_member[1],
                           &get_list[0],
                           sizeof(zigpc_group_member_t));
  TEST_ASSERT_EQUAL_MEMORY(&add_member[0],
                           &get_list[1],
                           sizeof(zigpc_group_member_t));
}

void test_removing_all_reported_members_for_group(void)
{
  // Currently there are two groupIDs being serviced (0x30 and 0x40).
  // This test removes a member that is part of both groups.

  // ARRANGE
  sl_status_t status;
  zigpc_group_member_t member = {
    .eui64       = "\x12\x23\x34\x45\x56\x67\x78\x89",
    .endpoint_id = 1,
  };

  TEST_ASSERT_EQUAL(3, zigpc_group_map_get_memberlist_count(0x30));
  TEST_ASSERT_EQUAL(2, zigpc_group_map_get_memberlist_count(0x40));

  // ACT
  status = zigpc_group_map_remove_all(member);
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);

  // ASSERT
  TEST_ASSERT_EQUAL(2, zigpc_group_map_get_memberlist_count(0x30));
  TEST_ASSERT_EQUAL(1, zigpc_group_map_get_memberlist_count(0x40));
}

void test_add_group_with_name(void)
{
  // ARRANGE
  zigpc_group_map_clear();

  zigbee_group_id_t add_group_id = 0x20;
  zigpc_group_member_t get_list[1];
  zigpc_group_member_t add_member = {
    .eui64       = "\xDE\xAD\xBE\xEF\x02\x34\x7E\xAB",
    .endpoint_id = 2,
  };

  char add_name[] = "TestName";

  // ACT
  sl_status_t add_status = zigpc_group_map_add_new_with_name(add_group_id,
                                                             add_name,
                                                             add_member,
                                                             true);

  char test_name[ZCL_DEFAULT_STR_LENGTH];
  sl_status_t retrieve_name_status
    = zigpc_group_map_retrieve_group_name(add_member,
                                          add_group_id,
                                          true,
                                          test_name,
                                          ZCL_DEFAULT_STR_LENGTH);

  unsigned int num_members = zigpc_group_map_get_memberlist_count(add_group_id);
  sl_status_t memberlist_status
    = zigpc_group_map_get_memberlist_items(add_group_id, get_list, 1);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, add_status);
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, retrieve_name_status);
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, memberlist_status);

  TEST_ASSERT_EQUAL(1, num_members);
  TEST_ASSERT_EQUAL_MEMORY(&add_member,
                           &get_list,
                           sizeof(zigpc_group_member_t));

  TEST_ASSERT_EQUAL_STRING(add_name, test_name);

  zigpc_group_map_clear();
}

void test_set_group_name_success(void)
{
  // ARRANGE
  zigpc_group_map_clear();

  zigbee_group_id_t add_group_id = 0x20;
  zigpc_group_member_t get_list[1];
  zigpc_group_member_t add_member = {
    .eui64       = "\xDE\xAD\xBE\xEF\x02\x34\x7E\xAB",
    .endpoint_id = 2,
  };

  char add_name[]  = "TestName";
  char add_name2[] = "TestName2";

  // ACT
  sl_status_t add_status = zigpc_group_map_add_new_with_name(add_group_id,
                                                             add_name,
                                                             add_member,
                                                             true);

  char test_name[ZCL_DEFAULT_STR_LENGTH];
  sl_status_t retrieve_name_status
    = zigpc_group_map_retrieve_group_name(add_member,
                                          add_group_id,
                                          true,
                                          test_name,
                                          ZCL_DEFAULT_STR_LENGTH);

  sl_status_t set_name_status
    = zigpc_group_map_set_group_name(add_member, add_group_id, true, add_name2);

  char test_name2[ZCL_DEFAULT_STR_LENGTH];
  sl_status_t retrieve_name_status2
    = zigpc_group_map_retrieve_group_name(add_member,
                                          add_group_id,
                                          true,
                                          test_name2,
                                          ZCL_DEFAULT_STR_LENGTH);

  unsigned int num_members = zigpc_group_map_get_memberlist_count(add_group_id);
  sl_status_t memberlist_status
    = zigpc_group_map_get_memberlist_items(add_group_id, get_list, 1);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, add_status);
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, set_name_status);
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, retrieve_name_status);
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, retrieve_name_status2);
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, memberlist_status);

  TEST_ASSERT_EQUAL(1, num_members);
  TEST_ASSERT_EQUAL_MEMORY(&add_member,
                           &get_list,
                           sizeof(zigpc_group_member_t));

  TEST_ASSERT_EQUAL_STRING(add_name, test_name);
  TEST_ASSERT_EQUAL_STRING(add_name2, test_name2);

  zigpc_group_map_clear();
}
