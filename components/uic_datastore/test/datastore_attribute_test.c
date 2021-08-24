/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/
#include "datastore.h"
#include "datastore_attributes.h"

// Includes from other components
#include "sl_status.h"
#include "unity.h"
#include "sl_log.h"

// Generic Includes
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTeardown(int num_failures)
{
  return num_failures;
}

// Test storing attribute in the datastore
void test_datastore_attribute_storage()
{
  // Initialization
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_init(":memory:"),
                            "Initialization of datastore");

  // Wipe out the table
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    datastore_delete_all_attributes(),
    "Could not erase the attribute datastore correctly.");

  uint8_t test_array[] = {0x01, 0x02, 0xFF, 0x9F};

  TEST_ASSERT_FALSE_MESSAGE(datastore_contains_attribute(0x00),
                            "attribute 0x00 shouldn't be in the datastore");

  // Insert attribute 0x01
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_store_attribute(0x01,
                                                      0x01,
                                                      0x00,  // No parent
                                                      test_array,
                                                      sizeof(test_array),
                                                      false),
                            "Attribute 0x01 insertion went wrong.");

  TEST_ASSERT_TRUE_MESSAGE(datastore_contains_attribute(0x01),
                           "attribute 0x01 should be in the datastore");

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_store_attribute(0x01,
                                                      0x06,
                                                      0x00,
                                                      test_array,
                                                      sizeof(test_array),
                                                      true),
                            "Attribute 0x01 update went wrong.");

  // Insert attribute 0x02 without a valid parent_id
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_FAIL,
    datastore_store_attribute(0x02,
                              0x03,
                              0x03,
                              test_array,
                              sizeof(test_array),
                              false),
    "Attribute not 0x02 insertion went well, it should not have.");

  // Insert attribute 0x02
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_store_attribute(0x02,
                                                      0x03,
                                                      0x01,
                                                      test_array,
                                                      sizeof(test_array),
                                                      false),
                            "Attribute 0x02 insertion went wrong");

  uint32_t received_type              = 0xFF;
  datastore_attribute_id_t received_parent_id = 0xFF;
  uint8_t received_value[255]                 = {0};
  uint8_t received_value_size                 = 0xFF;
  bool received_desired                       = false;

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_fetch_attribute(0x01,
                                                      &received_type,
                                                      &received_parent_id,
                                                      received_value,
                                                      &received_value_size,
                                                      &received_desired),
                            "Fetching attribute node 0x01 went wrong.");

  TEST_ASSERT_EQUAL(0x06, received_type);
  TEST_ASSERT_EQUAL(0x00, received_parent_id);
  TEST_ASSERT_EQUAL_INT8_ARRAY(test_array, received_value, received_value_size);
  TEST_ASSERT_EQUAL(sizeof(test_array), received_value_size);
  TEST_ASSERT_EQUAL(true, received_desired);

  // Delete our node 0x01, it should not allow us as it has a child
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_FAIL,
    datastore_delete_attribute(0x01),
    "Should not be possible to erase attribute node 0x01");
  TEST_ASSERT_TRUE_MESSAGE(
    datastore_contains_attribute(0x01),
    "attribute node 0x01 should still be in the datastore");

  // Insert a second child to node 0x01
  test_array[0] = 35;
  test_array[1] = 34;
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_store_attribute(0x03,
                                                      0xFFFFF,
                                                      0x01,
                                                      test_array,
                                                      sizeof(test_array),
                                                      false),
                            "Attribute node 0x03 insertion went wrong");

  // Fetch second child of node 0x01:
  datastore_attribute_id_t received_id = 0x00;
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_OK,
    datastore_fetch_attribute_child(0x01,
                                    1,
                                    &received_id,
                                    &received_type,
                                    received_value,
                                    &received_value_size,
                                    &received_desired),
    "Fetching 2nd child of node 0x01 went wrong");

  // Verify that the data for child 2 was received properly:
  TEST_ASSERT_EQUAL(0x03, received_id);
  TEST_ASSERT_EQUAL(0xFFFFF, received_type);
  TEST_ASSERT_EQUAL_INT8_ARRAY(test_array, received_value, received_value_size);
  TEST_ASSERT_EQUAL(sizeof(test_array), received_value_size);
  TEST_ASSERT_EQUAL(false, received_desired);

  // Fetch third child of node 0x01 (should not exist):
  TEST_ASSERT_EQUAL_MESSAGE(
    SL_STATUS_NOT_FOUND,
    datastore_fetch_attribute_child(0x01,
                                    2,
                                    &received_id,
                                    &received_type,
                                    received_value,
                                    &received_value_size,
                                    &received_desired),
    "Fetching 3rd child of node 0x01 went well, though there should be only "
    "2!");

  // delete node 2
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_delete_attribute(0x02),
                            "Should be possible to erase attribute 0x02");
  TEST_ASSERT_FALSE_MESSAGE(datastore_contains_attribute(0x02),
                            "attribute 0x02 should not be in the datastore");

  // Insert attribute 0xf000000 with type 0xff000000
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_store_attribute(0xf0000000,
                                                      0xff000000,
                                                      0x00,  // No parent
                                                      test_array,
                                                      sizeof(test_array),
                                                      false),
                            "Attribute 0x01 insertion went wrong.");

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_fetch_attribute(0xf0000000,
                                                      &received_type,
                                                      &received_parent_id,
                                                      received_value,
                                                      &received_value_size,
                                                      &received_desired),
                            "Fetching attribute node 0xf0000000 went wrong.");

  TEST_ASSERT_EQUAL(0xff000000, received_type);
  TEST_ASSERT_EQUAL(0x00, received_parent_id);
  TEST_ASSERT_EQUAL_INT8_ARRAY(test_array, received_value, received_value_size);
  TEST_ASSERT_EQUAL(sizeof(test_array), received_value_size);
  TEST_ASSERT_EQUAL(false, received_desired);


  // Insert attribute 0xf000001 with type 0xff000001 and parent 0xf000000
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_store_attribute(0xf0000001,
                                                      0xff000001,
                                                      0xf0000000,
                                                      test_array,
                                                      sizeof(test_array),
                                                      false),
                            "Attribute 0x01 insertion went wrong.");

  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_fetch_attribute(0xf0000001,
                                                      &received_type,
                                                      &received_parent_id,
                                                      received_value,
                                                      &received_value_size,
                                                      &received_desired),
                            "Fetching attribute node 0xf0000001 went wrong.");

  TEST_ASSERT_EQUAL(0xff000001, received_type);
  TEST_ASSERT_EQUAL(0xf0000000, received_parent_id);
  TEST_ASSERT_EQUAL_INT8_ARRAY(test_array, received_value, received_value_size);
  TEST_ASSERT_EQUAL(sizeof(test_array), received_value_size);
  TEST_ASSERT_EQUAL(false, received_desired);

  // Teardown
  TEST_ASSERT_EQUAL_MESSAGE(SL_STATUS_OK,
                            datastore_teardown(),
                            "Expect teardown to match SL_STATUS_OK");
}
