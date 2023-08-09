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

#include "zigpc_datastore_test.hpp"

extern "C" {

// Unify shared includes
#include <unity.h>

// ZigPC includes
#include <zigpc_common_zigbee.h>

// Component includes
#include "zigpc_datastore_mock.h"

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

void test_get_device_id_list_sanity(void)
{
  size_t dev_count                       = 3;
  zigbee_eui64_t expected_dev[dev_count] = {
    {1, 2, 3, 4, 5, 6, 7, 8},
    {0xD, 0x2, 0xFF, 0x1, 0x12, 0xAA, 0x9D, 0xEE},
    {0, 0, 0, 0, 0xA, 0xB, 0xC, 0xD},
  };

  // ARRANGE
  zigpc_datastore_get_device_count_ExpectAndReturn(dev_count);
  for (size_t i = 0; i < dev_count; i++) {
    zigpc_datastore_find_device_by_index_ExpectAndReturn(i, NULL, SL_STATUS_OK);
    zigpc_datastore_find_device_by_index_IgnoreArg_eui64();
    zigpc_datastore_find_device_by_index_ReturnThruPtr_eui64(&expected_dev[i]);
  }

  // ACT
  auto dev_list = zigpc_datastore::device::get_id_list();

  // ASSERT
  TEST_ASSERT_EQUAL(dev_count, dev_list.size());
  for (size_t i = 0; i < dev_count; i++) {
    TEST_ASSERT_EQUAL_HEX64(zigbee_eui64_to_uint(expected_dev[i]), dev_list[i]);
  }
}

void test_get_device_id_list_empty(void)
{
  size_t dev_count = 0;

  // ARRANGE
  zigpc_datastore_get_device_count_ExpectAndReturn(dev_count);

  // ACT
  auto dev_list = zigpc_datastore::device::get_id_list();

  // ASSERT
  TEST_ASSERT_EQUAL(dev_count, dev_list.size());
}

void test_get_endpoint_id_list_invalid_args(void)
{
  // ARRANGE

  // ACT
  auto ep_list = zigpc_datastore::endpoint::get_id_list(nullptr);

  // ASSERT
  TEST_ASSERT_EQUAL(0, ep_list.size());
}

void test_get_endpoint_id_list_sanity(void)
{
  size_t ep_count    = 2;
  zigbee_eui64_t dev = {0xF, 0x1, 0x2, 0x4, 0x5, 0x99, 0xD, 0x2};
  zigbee_endpoint_id_t expected_ep[ep_count] = {1, 3};

  // ARRANGE
  zigpc_datastore_get_endpoint_count_ExpectAndReturn(dev, ep_count);
  for (size_t i = 0; i < ep_count; i++) {
    zigpc_datastore_find_endpoint_by_index_ExpectAndReturn(dev,
                                                           i,
                                                           NULL,
                                                           SL_STATUS_OK);
    zigpc_datastore_find_endpoint_by_index_IgnoreArg_endpoint_id();
    zigpc_datastore_find_endpoint_by_index_ReturnThruPtr_endpoint_id(
      &expected_ep[i]);
  }

  // ACT
  auto ep_list = zigpc_datastore::endpoint::get_id_list(dev);

  // ASSERT
  TEST_ASSERT_EQUAL(ep_count, ep_list.size());
  for (size_t i = 0; i < ep_count; i++) {
    TEST_ASSERT_EQUAL(expected_ep[i], ep_list[i]);
  }
}
void test_get_endpoint_id_list_empty(void)
{
  size_t ep_count    = 0;
  zigbee_eui64_t dev = {0xF, 0x1, 0x2, 0x4, 0x5, 0x99, 0xD, 0x2};

  // ARRANGE
  zigpc_datastore_get_endpoint_count_ExpectAndReturn(dev, ep_count);

  // ACT
  auto ep_list = zigpc_datastore::endpoint::get_id_list(dev);

  // ASSERT
  TEST_ASSERT_EQUAL(ep_count, ep_list.size());
}
void test_get_cluster_id_list_invalid_args(void)
{
  // ARRANGE

  // ACT
  auto cl_list = zigpc_datastore::cluster::get_id_list(nullptr,
                                                       0,
                                                       ZCL_CLUSTER_SERVER_SIDE);

  // ASSERT
  TEST_ASSERT_EQUAL(0, cl_list.size());
}
void test_get_cluster_id_list_sanity(void)
{
  size_t cl_count         = 6;
  zigbee_eui64_t dev      = {0xF, 0x1, 0x2, 0x4, 0x5, 0x99, 0xD, 0x2};
  zigbee_endpoint_id_t ep = 3;
  zcl_cluster_id_t expected_cl[cl_count] = {0x02FF, 0x004};

  // ARRANGE
  zigpc_datastore_get_cluster_count_ExpectAndReturn(dev,
                                                    ep,
                                                    ZCL_CLUSTER_SERVER_SIDE,
                                                    cl_count);
  for (size_t i = 0; i < cl_count; i++) {
    zigpc_datastore_find_cluster_by_index_ExpectAndReturn(
      dev,
      ep,
      ZCL_CLUSTER_SERVER_SIDE,
      i,
      NULL,
      SL_STATUS_OK);
    zigpc_datastore_find_cluster_by_index_IgnoreArg_cluster_id();
    zigpc_datastore_find_cluster_by_index_ReturnThruPtr_cluster_id(
      &expected_cl[i]);
  }

  // ACT
  auto cl_list
    = zigpc_datastore::cluster::get_id_list(dev, ep, ZCL_CLUSTER_SERVER_SIDE);

  // ASSERT
  TEST_ASSERT_EQUAL(cl_count, cl_list.size());
  for (size_t i = 0; i < cl_count; i++) {
    TEST_ASSERT_EQUAL_HEX16(expected_cl[i], cl_list[i]);
  }
}
void test_get_cluster_id_list_empty(void)
{
  size_t cl_count         = 0;
  zigbee_eui64_t dev      = {0xF, 0x1, 0x2, 0x4, 0x5, 0x99, 0xD, 0x2};
  zigbee_endpoint_id_t ep = 6;

  // ARRANGE
  zigpc_datastore_get_cluster_count_ExpectAndReturn(dev,
                                                    ep,
                                                    ZCL_CLUSTER_SERVER_SIDE,
                                                    cl_count);

  // ACT
  auto cl_list
    = zigpc_datastore::cluster::get_id_list(dev, ep, ZCL_CLUSTER_SERVER_SIDE);

  // ASSERT
  TEST_ASSERT_EQUAL(cl_count, cl_list.size());
}

}  // extern "C"
