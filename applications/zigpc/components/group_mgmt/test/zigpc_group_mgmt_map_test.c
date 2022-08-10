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

#include <string.h>

// Unify includes
#include <unity.h>

// ZigPC includes
#include <zigpc_datastore_mock.h>

#include "zigpc_group_map.h"

// TODO: Test Reading multiple members from tracked group ID
// using zigpc_group_map_get_memberlist_items()

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
  size_t dev_count                = 1;
  zigbee_eui64_t tracked_device   = {0xD, 0xE, 0xE, 0xD, 0xC, 0xA, 0xE, 0xF};
  size_t ep_count                 = 1;
  zigbee_endpoint_id_t tracked_ep = 3;
  zigbee_group_id_t test_group_id = 0x10;

  // ARRANGE
  zigpc_datastore_get_device_count_ExpectAndReturn(dev_count);
  zigpc_datastore_find_device_by_index_ExpectAndReturn(0, NULL, SL_STATUS_OK);
  zigpc_datastore_find_device_by_index_IgnoreArg_eui64();
  zigpc_datastore_find_device_by_index_ReturnMemThruPtr_eui64(
    &tracked_device,
    sizeof(zigbee_eui64_t));

  zigpc_datastore_get_endpoint_count_ExpectAndReturn(tracked_device, ep_count);
  zigpc_datastore_find_endpoint_by_index_ExpectAndReturn(tracked_device,
                                                         0,
                                                         NULL,
                                                         SL_STATUS_OK);
  zigpc_datastore_find_endpoint_by_index_IgnoreArg_endpoint_id();
  zigpc_datastore_find_endpoint_by_index_ReturnMemThruPtr_endpoint_id(
    &tracked_ep,
    sizeof(zigbee_endpoint_id_t));
  
  zigpc_datastore_contains_group_ExpectAndReturn(tracked_device,
                                                 tracked_ep,
                                                 true,
                                                 test_group_id,
                                                 false);

  // ACT
  size_t count = zigpc_group_map_get_memberlist_count(test_group_id);
  // ASSERT
  TEST_ASSERT_EQUAL(0U, count);
}

void test_retrieving_member_list_of_untracked_groupid(void)
{
  size_t dev_count                = 1;
  zigbee_eui64_t tracked_device   = {0xD, 0xE, 0xE, 0xD, 0xC, 0xA, 0xE, 0xF};
  size_t ep_count                 = 1;
  zigbee_endpoint_id_t tracked_ep = 3;
  zigbee_group_id_t test_group_id = 0x10;

  zigpc_group_member_t list[2];

  // ARRANGE
  zigpc_datastore_get_device_count_ExpectAndReturn(dev_count);
  zigpc_datastore_find_device_by_index_ExpectAndReturn(0, NULL, SL_STATUS_OK);
  zigpc_datastore_find_device_by_index_IgnoreArg_eui64();
  zigpc_datastore_find_device_by_index_ReturnMemThruPtr_eui64(
    &tracked_device,
    sizeof(zigbee_eui64_t));

  zigpc_datastore_get_endpoint_count_ExpectAndReturn(tracked_device, ep_count);
  zigpc_datastore_find_endpoint_by_index_ExpectAndReturn(tracked_device,
                                                         0,
                                                         NULL,
                                                         SL_STATUS_OK);
  zigpc_datastore_find_endpoint_by_index_IgnoreArg_endpoint_id();
  zigpc_datastore_find_endpoint_by_index_ReturnMemThruPtr_endpoint_id(
    &tracked_ep,
    sizeof(zigbee_endpoint_id_t));
  zigpc_datastore_contains_group_ExpectAndReturn(tracked_device,
                                                 tracked_ep,
                                                 true,
                                                 test_group_id,
                                                 false);

  // ACT
  sl_status_t status
    = zigpc_group_map_get_memberlist_items(test_group_id, list, 2);
  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_NOT_FOUND, status);
}

void test_adding_reported_group_should_add_member_for_groupid(void)
{
  bool is_reported                = true;
  zigbee_group_id_t add_group_id  = 0x20;
  zigpc_group_member_t add_member = {
    .eui64       = "\xDE\xAD\xBE\xEF\x02\x34\x7E\xAB",
    .endpoint_id = 2,
  };

  zigpc_group_data_t group_data = {0};

  // ARRANGE
  zigpc_datastore_create_group_ExpectAndReturn(add_member.eui64,
                                               add_member.endpoint_id,
                                               is_reported,
                                               add_group_id,
                                               SL_STATUS_OK);
  zigpc_datastore_write_group_ExpectWithArrayAndReturn(
    add_member.eui64,
    sizeof(zigbee_eui64_t),
    add_member.endpoint_id,
    is_reported,
    add_group_id,
    &group_data,
    sizeof(zigpc_group_data_t),
    SL_STATUS_OK);

  // ACT
  sl_status_t status
    = zigpc_group_map_add_new(add_group_id, add_member, is_reported);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}

void test_removing_reported_group_should_remove_member_for_groupid(void)
{
  bool is_reported                = true;
  zigbee_group_id_t rem_group_id  = 0x20;
  zigpc_group_member_t rem_member = {
    .eui64       = "\xDE\xAD\xBE\xEF\x02\x34\x7E\xAB",
    .endpoint_id = 2,
  };
  // ARRANGE
  zigpc_datastore_remove_group_ExpectAndReturn(rem_member.eui64,
                                               rem_member.endpoint_id,
                                               is_reported,
                                               rem_group_id,
                                               SL_STATUS_OK);

  // ACT
  sl_status_t status
    = zigpc_group_map_remove_group(rem_group_id, rem_member, true);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}

void test_adding_multiple_members_to_reported_groups(void)
{
  // ARRANGE
  sl_status_t status;
  bool is_reported           = true;
  zigbee_group_id_t group_id = 0x30;
  const size_t group_count   = 3;
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
  // group data should be zeroed since there is no group name
  zigpc_group_data_t add_data = {0};

  // ARRANGE
  for (size_t i = 0; i < group_count; i++) {
    zigpc_datastore_create_group_ExpectAndReturn(add_member[i].eui64,
                                                 add_member[i].endpoint_id,
                                                 is_reported,
                                                 group_id,
                                                 SL_STATUS_OK);
    zigpc_datastore_write_group_ExpectWithArrayAndReturn(
      add_member[i].eui64,
      sizeof(zigbee_eui64_t),
      add_member[i].endpoint_id,
      is_reported,
      group_id,
      &add_data,
      sizeof(zigpc_group_data_t),
      SL_STATUS_OK);
  }

  for (size_t i = 0; i < group_count; i++) {
    // ACT
    status = zigpc_group_map_add_new(group_id, add_member[i], is_reported);
    // ASSERT
    TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
  }
}

void test_adding_members_to_different_groupid(void)
{
  sl_status_t status;
  bool is_reported           = true;
  zigbee_group_id_t group_id = 0x40;
  const size_t group_count   = 2;
  zigpc_group_member_t add_member[2]
    = {{
         .eui64       = "\x12\x23\x34\x45\x56\x67\x78\x89",
         .endpoint_id = 1,
       },
       {
         .eui64       = "\x0F\x2D\xEA\x03\x99\x3D\x21\x09",
         .endpoint_id = 4,

       }};
  // group data should be zeroed since there is no group name
  zigpc_group_data_t add_data = {0};

  // ARRANGE
  for (size_t i = 0; i < group_count; i++) {
    zigpc_datastore_create_group_ExpectAndReturn(add_member[i].eui64,
                                                 add_member[i].endpoint_id,
                                                 is_reported,
                                                 group_id,
                                                 SL_STATUS_OK);
    zigpc_datastore_write_group_ExpectWithArrayAndReturn(
      add_member[i].eui64,
      sizeof(zigbee_eui64_t),
      add_member[i].endpoint_id,
      is_reported,
      group_id,
      &add_data,
      sizeof(zigpc_group_data_t),
      SL_STATUS_OK);
  }

  for (size_t i = 0; i < group_count; i++) {
    // ACT
    status = zigpc_group_map_add_new(group_id, add_member[i], true);
    // ASSERT
    TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
  }
}

void test_removing_all_reported_members_for_group(void)
{
  sl_status_t status;
  zigpc_group_member_t member = {
    .eui64       = "\x12\x23\x34\x45\x56\x67\x78\x89",
    .endpoint_id = 1,
  };
  size_t desired_group_count  = 2;
  size_t reported_group_count = 1;

  zigbee_group_id_t desired_group_list[]  = {0x30, 0x40};
  zigbee_group_id_t reported_group_list[] = {0x30};

  // ARRANGE
  {
    // Expects for desired groups stored

    zigpc_datastore_get_group_count_ExpectAndReturn(member.eui64,
                                                    member.endpoint_id,
                                                    false,
                                                    desired_group_count);
    for (size_t i = 0U; i < desired_group_count; i++) {
      zigpc_datastore_find_group_by_index_ExpectAndReturn(member.eui64,
                                                          member.endpoint_id,
                                                          false,
                                                          i,
                                                          NULL,
                                                          SL_STATUS_OK);
      zigpc_datastore_find_group_by_index_IgnoreArg_group_id();
      zigpc_datastore_find_group_by_index_ReturnMemThruPtr_group_id(
        &desired_group_list[i],
        sizeof(zigbee_group_id_t));

      // Finally expect this group to be removed
      zigpc_datastore_remove_group_ExpectAndReturn(member.eui64,
                                                   member.endpoint_id,
                                                   false,
                                                   desired_group_list[i],
                                                   SL_STATUS_OK);
    }
  }
  {
    // Expects for reported groups stored
    zigpc_datastore_get_group_count_ExpectAndReturn(member.eui64,
                                                    member.endpoint_id,
                                                    true,
                                                    reported_group_count);
    for (size_t i = 0U; i < reported_group_count; i++) {
      zigpc_datastore_find_group_by_index_ExpectAndReturn(member.eui64,
                                                          member.endpoint_id,
                                                          true,
                                                          i,
                                                          NULL,
                                                          SL_STATUS_OK);
      zigpc_datastore_find_group_by_index_IgnoreArg_group_id();
      zigpc_datastore_find_group_by_index_ReturnMemThruPtr_group_id(
        &reported_group_list[i],
        sizeof(zigbee_group_id_t));

      // Finally expect this group to be removed
      zigpc_datastore_remove_group_ExpectAndReturn(member.eui64,
                                                   member.endpoint_id,
                                                   true,
                                                   reported_group_list[i],
                                                   SL_STATUS_OK);
    }
  }

  // ACT
  status = zigpc_group_map_remove_all(member);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, status);
}

void test_add_group_with_name(void)
{
  bool is_reported                = true;
  zigbee_group_id_t add_group_id  = 0x20;
  zigpc_group_member_t add_member = {
    .eui64       = "\xDE\xAD\xBE\xEF\x02\x34\x7E\xAB",
    .endpoint_id = 2,
  };

  zigpc_group_data_t add_group_data = {.group_name = "TestName"};

  // ARRANGE
  zigpc_datastore_create_group_ExpectAndReturn(add_member.eui64,
                                               add_member.endpoint_id,
                                               is_reported,
                                               add_group_id,
                                               SL_STATUS_OK);
  zigpc_datastore_write_group_ExpectWithArrayAndReturn(
    add_member.eui64,
    sizeof(zigbee_eui64_t),
    add_member.endpoint_id,
    is_reported,
    add_group_id,
    &add_group_data,
    sizeof(zigpc_group_data_t),
    SL_STATUS_OK);

  // ACT
  sl_status_t add_status
    = zigpc_group_map_add_new_with_name(add_group_id,
                                        add_group_data.group_name,
                                        add_member,
                                        is_reported);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, add_status);
}

void test_set_group_name_success(void)
{
  bool is_reported                = true;
  zigbee_group_id_t add_group_id  = 0x20;
  zigpc_group_member_t add_member = {
    .eui64       = "\xDE\xAD\xBE\xEF\x02\x34\x7E\xAB",
    .endpoint_id = 2,
  };

  zigpc_group_data_t add_group_data = {.group_name = "Test"};

  // ARRANGE
  zigpc_datastore_contains_group_ExpectAndReturn(add_member.eui64,
                                                 add_member.endpoint_id,
                                                 is_reported,
                                                 add_group_id,
                                                 true);

  zigpc_datastore_write_group_ExpectWithArrayAndReturn(
    add_member.eui64,
    ZIGBEE_EUI64_SIZE,
    add_member.endpoint_id,
    is_reported,
    add_group_id,
    &add_group_data,
    sizeof(zigpc_group_data_t),
    SL_STATUS_OK);

  // ACT
  sl_status_t set_name_status
    = zigpc_group_map_set_group_name(add_member,
                                     add_group_id,
                                     is_reported,
                                     add_group_data.group_name);

  // ASSERT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_OK, set_name_status);
}

void test_retrieving_empty_group_name(void)
{
  bool publish_type_reported        = false;
  zigbee_group_id_t group_id        = 0x05;
  zigpc_group_member_t group_member = {
    .eui64       = "\xDE\xAD\xBE\xEF\x02\x34\x7E\xAB",
    .endpoint_id = 2,
  };
  zigpc_group_data_t read_group_data = {.group_name = ""};

  // ARRANGE (Nothing to expect)
  zigpc_datastore_read_group_ExpectAndReturn(group_member.eui64,
                                             group_member.endpoint_id,
                                             publish_type_reported,
                                             group_id,
                                             NULL,
                                             SL_STATUS_OK);
  zigpc_datastore_read_group_IgnoreArg_data();
  zigpc_datastore_read_group_ReturnMemThruPtr_data(&read_group_data,
                                                   sizeof(zigpc_group_data_t));

  // ACT
  size_t test_name_size = ZCL_GROUP_NAME_LENGTH;
  char test_name[test_name_size];
  sl_status_t get_status
    = zigpc_group_map_retrieve_group_name(group_member,
                                          group_id,
                                          publish_type_reported,
                                          test_name,
                                          &test_name_size);

  // ACT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_EMPTY, get_status);
  TEST_ASSERT_EQUAL_STRING("", test_name);
  TEST_ASSERT_EQUAL(0U, test_name_size);
}

void test_retrieving_name_of_removed_group(void)
{
  bool publish_type_reported        = false;
  zigbee_group_id_t group_id        = 0x05;
  zigpc_group_member_t group_member = {
    .eui64       = "\xDE\xAD\xBE\xEF\x02\x34\x7E\xAB",
    .endpoint_id = 2,
  };

  // ARRANGE
  zigpc_datastore_read_group_ExpectAndReturn(group_member.eui64,
                                             group_member.endpoint_id,
                                             publish_type_reported,
                                             group_id,
                                             NULL,
                                             SL_STATUS_NOT_FOUND);
  zigpc_datastore_read_group_IgnoreArg_data();

  // ACT
  size_t test_name_size = ZCL_DEFAULT_STR_LENGTH;
  char test_name[test_name_size];
  sl_status_t get_status
    = zigpc_group_map_retrieve_group_name(group_member,
                                          group_id,
                                          publish_type_reported,
                                          test_name,
                                          &test_name_size);

  // ACT
  TEST_ASSERT_EQUAL_HEX8(SL_STATUS_NOT_FOUND, get_status);
  TEST_ASSERT_EQUAL_HEX(0U, test_name_size);
}
