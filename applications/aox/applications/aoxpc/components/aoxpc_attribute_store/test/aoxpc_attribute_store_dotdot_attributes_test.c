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
// Includes from this component
#include "aoxpc_attribute_store.h"
#include "aoxpc_attribute_store_network_helper.h"
#include "aoxpc_attribute_store_defined_attribute_types.h"

// Test includes
#include "unity.h"

// Unify includes
#include "datastore.h"
#include "attribute_store_fixt.h"
#include "attribute_store.h"

// Test constants
#define AOXPC_UNID        "AoXPCUNID_TeST"
#define AOXPC_ENDPOINT_ID 3

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  attribute_store_teardown();
  datastore_teardown();
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  // Ensure we start from scratch before creating our test network.
  attribute_store_delete_node(attribute_store_get_root());
}

void test_aoxpc_attribute_store_dotdot_attributes_cte_mode()
{
  // Ask if CTEMode is supported:
  TEST_ASSERT_FALSE(
    dotdot_is_supported_aox_locator_cte_mode(AOXPC_UNID, AOXPC_ENDPOINT_ID));

  // Create CTEMode
  const uint8_t test_cte_mode = 3;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    dotdot_set_aox_locator_cte_mode(AOXPC_UNID,
                                                    AOXPC_ENDPOINT_ID,
                                                    test_cte_mode));

  TEST_ASSERT_TRUE(
    dotdot_is_supported_aox_locator_cte_mode(AOXPC_UNID, AOXPC_ENDPOINT_ID));
  TEST_ASSERT_FALSE(
    dotdot_is_supported_aox_locator_cte_mode("another UNID",
                                             AOXPC_ENDPOINT_ID));
  TEST_ASSERT_FALSE(
    dotdot_is_supported_aox_locator_cte_mode(AOXPC_UNID,
                                             AOXPC_ENDPOINT_ID + 1));

  uint8_t received_cte_mode = 0;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    dotdot_get_aox_locator_cte_mode(AOXPC_UNID,
                                                    AOXPC_ENDPOINT_ID,
                                                    &received_cte_mode));

  TEST_ASSERT_EQUAL(test_cte_mode, received_cte_mode);
}

void test_aoxpc_attribute_store_dotdot_attributes_position_and_orientation_valid()
{
  // Ask if PositionAndOrientationValid is supported:
  TEST_ASSERT_FALSE(
    dotdot_is_supported_aox_locator_position_and_orientation_valid(
      AOXPC_UNID,
      AOXPC_ENDPOINT_ID));

  // Create PositionAndOrientationValid
  const bool test_locator_position_and_orientation_valid = true;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    dotdot_set_aox_locator_position_and_orientation_valid(
                      AOXPC_UNID,
                      AOXPC_ENDPOINT_ID,
                      test_locator_position_and_orientation_valid));

  TEST_ASSERT_TRUE(
    dotdot_is_supported_aox_locator_position_and_orientation_valid(
      AOXPC_UNID,
      AOXPC_ENDPOINT_ID));
  TEST_ASSERT_FALSE(
    dotdot_is_supported_aox_locator_position_and_orientation_valid(
      "another UNID",
      AOXPC_ENDPOINT_ID));
  TEST_ASSERT_FALSE(
    dotdot_is_supported_aox_locator_position_and_orientation_valid(
      AOXPC_UNID,
      AOXPC_ENDPOINT_ID + 1));

  bool received_position_and_orientation_valid = false;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    dotdot_get_aox_locator_position_and_orientation_valid(
                      AOXPC_UNID,
                      AOXPC_ENDPOINT_ID,
                      &received_position_and_orientation_valid));

  TEST_ASSERT_EQUAL(test_locator_position_and_orientation_valid,
                    received_position_and_orientation_valid);
}

void test_aoxpc_attribute_store_dotdot_attributes_position_and_orientation()
{
  // Ask if PositionAndOrientation is supported:
  TEST_ASSERT_FALSE(dotdot_is_supported_aox_locator_position_and_orientation(
    AOXPC_UNID,
    AOXPC_ENDPOINT_ID));

  // Create dotdot_is_supported_aox_locator_orientation
  const CoordinateAndOrientation test_configuration
    = {.CoordinateX  = 1.001,
       .CoordinateY  = -12.121212,
       .CoordinateZ  = 1234556789,
       .OrientationX = -0.0000001,
       .OrientationY = -1234567,
       .OrientationZ = 10};

  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    dotdot_set_aox_locator_position_and_orientation(AOXPC_UNID,
                                                    AOXPC_ENDPOINT_ID,
                                                    test_configuration));

  TEST_ASSERT_TRUE(dotdot_is_supported_aox_locator_position_and_orientation(
    AOXPC_UNID,
    AOXPC_ENDPOINT_ID));
  TEST_ASSERT_FALSE(dotdot_is_supported_aox_locator_position_and_orientation(
    "another UNID",
    AOXPC_ENDPOINT_ID));
  TEST_ASSERT_FALSE(dotdot_is_supported_aox_locator_position_and_orientation(
    AOXPC_UNID,
    AOXPC_ENDPOINT_ID + 1));

  CoordinateAndOrientation received_configuration = {};
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    dotdot_get_aox_locator_position_and_orientation(AOXPC_UNID,
                                                    AOXPC_ENDPOINT_ID,
                                                    &received_configuration));

  TEST_ASSERT_EQUAL_DOUBLE(test_configuration.CoordinateX,
                           received_configuration.CoordinateX);
  TEST_ASSERT_EQUAL_DOUBLE(test_configuration.CoordinateY,
                           received_configuration.CoordinateY);
  TEST_ASSERT_EQUAL_DOUBLE(test_configuration.CoordinateZ,
                           received_configuration.CoordinateZ);
  TEST_ASSERT_EQUAL_DOUBLE(test_configuration.OrientationX,
                           received_configuration.OrientationX);
  TEST_ASSERT_EQUAL_DOUBLE(test_configuration.OrientationY,
                           received_configuration.OrientationY);
  TEST_ASSERT_EQUAL_DOUBLE(test_configuration.OrientationZ,
                           received_configuration.OrientationZ);
}

void test_aoxpc_attribute_store_dotdot_attributes_allow_list()
{
  // Ask if AllowList is supported:
  TEST_ASSERT_FALSE(
    dotdot_is_supported_aox_locator_allow_list(AOXPC_UNID, AOXPC_ENDPOINT_ID));

  // Create dotdot_is_supported_aox_locator_orientation
  const size_t test_size                  = 6;
  const char *test_allow_list[] = {
    "Tag1",
    "Tag4",
    "Tag3",
    "This Is a very long tag in the allow list",
    "Tag1Again",
    "Tag5",
  };
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    dotdot_set_aox_locator_allow_list(AOXPC_UNID,
                                                      AOXPC_ENDPOINT_ID,
                                                      test_size,
                                                      test_allow_list));

  TEST_ASSERT_EQUAL(
    test_size,
    dotdot_get_aox_locator_allow_list_count(AOXPC_UNID, AOXPC_ENDPOINT_ID));

  TEST_ASSERT_TRUE(
    dotdot_is_supported_aox_locator_allow_list(AOXPC_UNID, AOXPC_ENDPOINT_ID));
  TEST_ASSERT_FALSE(
    dotdot_is_supported_aox_locator_allow_list("another UNID",
                                               AOXPC_ENDPOINT_ID));
  TEST_ASSERT_FALSE(
    dotdot_is_supported_aox_locator_allow_list(AOXPC_UNID,
                                               AOXPC_ENDPOINT_ID + 1));

  char received_list_item[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH] = {};
  for (size_t i = 0; i < test_size; i++) {
    TEST_ASSERT_EQUAL(SL_STATUS_OK,
                      dotdot_get_aox_locator_allow_list(AOXPC_UNID,
                                                        AOXPC_ENDPOINT_ID,
                                                        received_list_item,
                                                        i));
    TEST_ASSERT_EQUAL_STRING(test_allow_list[i], received_list_item);
  }

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    dotdot_get_aox_locator_allow_list(AOXPC_UNID,
                                                      AOXPC_ENDPOINT_ID,
                                                      received_list_item,
                                                      test_size));
}

void test_aoxpc_attribute_store_dotdot_attributes_azimuth_mask()
{
  // Ask if AllowList is supported:
  TEST_ASSERT_FALSE(
    dotdot_is_supported_aox_locator_azimuth_mask(AOXPC_UNID,
                                                 AOXPC_ENDPOINT_ID));

  // Create dotdot_is_supported_aox_locator_orientation
  const size_t test_size                  = 4;
  MinMaxPair test_azimuth_mask[] = {{.Min = 1.1, .Max = 2.2},
                                             {.Min = 10.01, .Max = 20.02},
                                             {.Min = 100.001, .Max = 200.002},
                                             {.Min = -1.1, .Max = 4.4}};

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    dotdot_set_aox_locator_azimuth_mask(AOXPC_UNID,
                                                        AOXPC_ENDPOINT_ID,
                                                        test_size,
                                                        test_azimuth_mask));

  TEST_ASSERT_EQUAL(
    test_size,
    dotdot_get_aox_locator_azimuth_mask_count(AOXPC_UNID, AOXPC_ENDPOINT_ID));

  TEST_ASSERT_TRUE(
    dotdot_is_supported_aox_locator_azimuth_mask(AOXPC_UNID,
                                                 AOXPC_ENDPOINT_ID));
  TEST_ASSERT_FALSE(
    dotdot_is_supported_aox_locator_azimuth_mask("another UNID",
                                                 AOXPC_ENDPOINT_ID));
  TEST_ASSERT_FALSE(
    dotdot_is_supported_aox_locator_azimuth_mask(AOXPC_UNID,
                                                 AOXPC_ENDPOINT_ID + 1));

  MinMaxPair received_azimuth_mask;

    for (size_t i = 0; i < test_size; i++)
  {
    TEST_ASSERT_EQUAL(SL_STATUS_OK,
                      dotdot_get_aox_locator_azimuth_mask(AOXPC_UNID,
                                                          AOXPC_ENDPOINT_ID,
                                                          &received_azimuth_mask,
                                                          i));
    TEST_ASSERT_EQUAL_DOUBLE(test_azimuth_mask[i].Min,
                             received_azimuth_mask.Min);
    TEST_ASSERT_EQUAL_DOUBLE(test_azimuth_mask[i].Max,
                             received_azimuth_mask.Max);
  }

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL,
                    dotdot_get_aox_locator_azimuth_mask(AOXPC_UNID,
                                                        AOXPC_ENDPOINT_ID,
                                                        &received_azimuth_mask,
                                                        test_size));
}