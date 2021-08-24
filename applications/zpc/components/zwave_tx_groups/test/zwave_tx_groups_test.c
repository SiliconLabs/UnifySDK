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
#include "zwave_tx_groups.h"

#include "zwave_controller_internal_mock.h"

// Generic includes
#include <string.h>

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_zwave_tx_groups_test_happy_case()
{
  zwave_nodemask_t my_test_nodemask;
  memset(my_test_nodemask, 0, sizeof(zwave_nodemask_t));

  // First try to create an empty group
  zwave_multicast_group_id_t received_group = ZWAVE_TX_INVALID_GROUP;
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    zwave_tx_assign_group(my_test_nodemask, &received_group));
  TEST_ASSERT_EQUAL(ZWAVE_TX_INVALID_GROUP, received_group);

  // Now add nodes to the nodemask
  ZW_ADD_NODE_TO_MASK(34, my_test_nodemask);

  zwave_controller_on_multicast_group_deleted_Expect(1);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_assign_group(my_test_nodemask, &received_group));

  // We expect group 1 has been assigned
  TEST_ASSERT_EQUAL(1, received_group);

  // Add another 2 nodes in the list
  ZW_ADD_NODE_TO_MASK(3, my_test_nodemask);
  ZW_ADD_NODE_TO_MASK(23, my_test_nodemask);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_assign_group(my_test_nodemask, &received_group));

  // We expect group 1 has been reused
  TEST_ASSERT_EQUAL(1, received_group);

  // Remove node 3
  ZW_REMOVE_NODE_FROM_MASK(3, my_test_nodemask);
  zwave_controller_on_multicast_group_deleted_Expect(2);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_assign_group(my_test_nodemask, &received_group));

  // We expect group 2 has been assigned
  TEST_ASSERT_EQUAL(2, received_group);

  // Add node 3 again
  ZW_ADD_NODE_TO_MASK(3, my_test_nodemask);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_assign_group(my_test_nodemask, &received_group));

  // We expect group 1 to be returned
  TEST_ASSERT_EQUAL(1, received_group);

  // Try to see if we can read the group from the GroupID
  zwave_nodemask_t received_nodemask = {};
  memset(received_nodemask, 0, sizeof(zwave_nodemask_t));
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_tx_get_nodes(received_nodemask, 1));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(my_test_nodemask,
                                received_nodemask,
                                sizeof(zwave_nodemask_t));
}

void test_zwave_tx_groups_lock_unlock_groups()
{
  // First try to lock / unlock non-existing groups
  TEST_ASSERT_FALSE(zwave_tx_is_group_locked(23));
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND, zwave_tx_lock_group(23));
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND, zwave_tx_unlock_group(23));

  // Create a group with an initial set of nodes
  zwave_nodemask_t my_test_nodemask;
  memset(my_test_nodemask, 0, sizeof(zwave_nodemask_t));

  ZW_ADD_NODE_TO_MASK(34, my_test_nodemask);
  ZW_ADD_NODE_TO_MASK(120, my_test_nodemask);
  ZW_ADD_NODE_TO_MASK(121, my_test_nodemask);
  zwave_multicast_group_id_t received_group = ZWAVE_TX_INVALID_GROUP;

  zwave_controller_on_multicast_group_deleted_Expect(3);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_assign_group(my_test_nodemask, &received_group));

  // Note that the groups created earlier still exists. So here we get Group 3
  TEST_ASSERT_EQUAL(3, received_group);

  // Lock group 3.
  TEST_ASSERT_FALSE(zwave_tx_is_group_locked(3));
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_tx_lock_group(3));
  TEST_ASSERT_TRUE(zwave_tx_is_group_locked(3));
  zwave_tx_group_log(3);

  // Add 2 nodes to the mask
  ZW_ADD_NODE_TO_MASK(2, my_test_nodemask);
  ZW_ADD_NODE_TO_MASK(3, my_test_nodemask);

  // Ask for a group. Group 1 is locked, a new group will be assigned.
  zwave_controller_on_multicast_group_deleted_Expect(4);
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_assign_group(my_test_nodemask, &received_group));
  TEST_ASSERT_EQUAL(4, received_group);

  // Remove node 3 and unlock group 3. Group 3 should be reused now.
  ZW_REMOVE_NODE_FROM_MASK(3, my_test_nodemask);
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_tx_unlock_group(3));
  TEST_ASSERT_FALSE(zwave_tx_is_group_locked(3));
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_tx_assign_group(my_test_nodemask, &received_group));

  // We expect group 3 has been reused
  TEST_ASSERT_EQUAL(3, received_group);
}
