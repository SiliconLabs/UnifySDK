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
#include "unity.h"

// Component being tested:
#include "aox_locator_configuration.h"
#include "aox_locator_configuration_internal.h"
#include "aox_locator_configuration_internal.h"

// Generic includes
#include <stdlib.h>

// Unify includes
#include "datastore.h"
#include "attribute_store_fixt.h"
#include "attribute_store.h"

// Mock components
#include "aoxpc_unid_mock.h"

// Test constants
#define AOXPC_UNID "AoXPCUNID"

// Stubs
sl_status_t get_aoxpc_unid_stub(aoa_id_t aoxpc_unid, int cmock_num_calls)
{
  snprintf(aoxpc_unid, sizeof(aoa_id_t), AOXPC_UNID);
  return SL_STATUS_OK;
}

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
  attribute_store_delete_node(attribute_store_get_root());
}

void test_save_and_restore_configuration()
{
  is_aoxpc_unid_IgnoreAndReturn(true);
  get_aoxpc_unid_Stub(&get_aoxpc_unid_stub);

  // Init our AoXLocator component
  TEST_ASSERT_EQUAL(SL_STATUS_OK, aox_locator_configuration_init());

  // Nothing should happen if we try to load the config from an empty
  // attribute store
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    aox_locator_load_attributes_from_attribute_store());

  // Set some values
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    aox_locator_set_position_and_orientation_valid_attribute(true));
  CoordinateAndOrientation position_and_orientation = {.CoordinateX  = 1.1,
                                                       .CoordinateY  = 2.2,
                                                       .CoordinateZ  = 3.3,
                                                       .OrientationX = -1.1,
                                                       .OrientationY = -2.2,
                                                       .OrientationZ = -3.3};
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    aox_locator_set_position_and_orientation_attribute(
                      position_and_orientation));

  const size_t azimuth_mask_size = 3;
  const MinMaxPair azimuth_mask[] = {{.Min = 1.01, .Max = 2.02},
                                     {.Min = -1.01, .Max = 20.02},
                                     {.Min = 1.0000001, .Max = 200.02}};
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    aox_locator_set_azimuth_mask_attribute(azimuth_mask_size, azimuth_mask));

  const size_t elevation_mask_size = 4;
  const MinMaxPair elevation_mask[] = {{.Min = 9.01, .Max = 29.02},
                                       {.Min = -19.01, .Max = 209.02},
                                       {.Min = 1.90000001, .Max = 2900.02},
                                       {.Min = 1.00090001, .Max = 200.902}};
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    aox_locator_set_elevation_mask_attribute(elevation_mask_size,
                                             elevation_mask));

  const size_t allow_list_size            = 5;
  const char *allow_list[]                = {"List Item 1",
                              "List Item 2",
                              "List Item 3",
                              "List Item 4",
                              "List Item 5"};
  TEST_ASSERT_EQUAL(
    SL_STATUS_OK,
    aox_locator_set_allow_list_attribute(allow_list_size, allow_list));
  TEST_ASSERT_EQUAL(SL_STATUS_OK, aox_locator_set_cte_mode_attribute(23));

  // Now back-up the entire thing.
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    aox_locator_store_attributes_in_attribute_store());

  // Calling twice should not break anything.
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    aox_locator_store_attributes_in_attribute_store());

  // Make a reset config:
  TEST_ASSERT_EQUAL(SL_STATUS_OK, aox_locator_reset_configuration());

  // check our config is dead:
  const aox_locator_attributes_t *config
    = aox_locator_configuration_get_configuration();
  TEST_ASSERT_EQUAL(0, config->position_and_orientation.CoordinateX);
  TEST_ASSERT_EQUAL(0, config->position_and_orientation.CoordinateY);
  TEST_ASSERT_EQUAL(0, config->position_and_orientation.CoordinateZ);
  TEST_ASSERT_EQUAL(0, config->position_and_orientation.OrientationX);
  TEST_ASSERT_EQUAL(0, config->position_and_orientation.OrientationY);
  TEST_ASSERT_EQUAL(0, config->position_and_orientation.OrientationZ);
  TEST_ASSERT_EQUAL(0, config->allow_list_count);
  TEST_ASSERT_EQUAL_PTR(NULL, config->allow_list);
  TEST_ASSERT_EQUAL(0, config->azimuth_mask_count);
  TEST_ASSERT_EQUAL_PTR(NULL, config->azimuth_mask);
  TEST_ASSERT_EQUAL(0, config->elevation_mask_count);
  TEST_ASSERT_EQUAL_PTR(NULL, config->elevation_mask);
  TEST_ASSERT_EQUAL(0, config->cte_mode);
  TEST_ASSERT_EQUAL(false, config->position_and_orientation_valid);

  // reload from the attribute store:
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    aox_locator_load_attributes_from_attribute_store());

  // Check that it did its magic:
  TEST_ASSERT_EQUAL(position_and_orientation.CoordinateX,
                    config->position_and_orientation.CoordinateX);
  TEST_ASSERT_EQUAL(position_and_orientation.CoordinateY,
                    config->position_and_orientation.CoordinateY);
  TEST_ASSERT_EQUAL(position_and_orientation.CoordinateZ,
                    config->position_and_orientation.CoordinateZ);
  TEST_ASSERT_EQUAL(position_and_orientation.OrientationX,
                    config->position_and_orientation.OrientationX);
  TEST_ASSERT_EQUAL(position_and_orientation.OrientationY,
                    config->position_and_orientation.OrientationY);
  TEST_ASSERT_EQUAL(position_and_orientation.OrientationZ,
                    config->position_and_orientation.OrientationZ);
  TEST_ASSERT_EQUAL(allow_list_size, config->allow_list_count);

  TEST_ASSERT_EQUAL_STRING(allow_list[0], config->allow_list[0]);
  TEST_ASSERT_EQUAL_STRING(allow_list[1], config->allow_list[1]);
  TEST_ASSERT_EQUAL_STRING(allow_list[2], config->allow_list[2]);
  TEST_ASSERT_EQUAL_STRING(allow_list[3], config->allow_list[3]);
  TEST_ASSERT_EQUAL_STRING(allow_list[4], config->allow_list[4]);

  TEST_ASSERT_EQUAL(azimuth_mask_size, config->azimuth_mask_count);
  TEST_ASSERT_EQUAL_DOUBLE(azimuth_mask[0].Min, config->azimuth_mask[0].Min);
  TEST_ASSERT_EQUAL_DOUBLE(azimuth_mask[0].Max, config->azimuth_mask[0].Max);
  TEST_ASSERT_EQUAL_DOUBLE(azimuth_mask[1].Min, config->azimuth_mask[1].Min);
  TEST_ASSERT_EQUAL_DOUBLE(azimuth_mask[1].Max, config->azimuth_mask[1].Max);
  TEST_ASSERT_EQUAL_DOUBLE(azimuth_mask[2].Min, config->azimuth_mask[2].Min);
  TEST_ASSERT_EQUAL_DOUBLE(azimuth_mask[2].Max, config->azimuth_mask[2].Max);

  TEST_ASSERT_EQUAL(elevation_mask_size, config->elevation_mask_count);
  TEST_ASSERT_EQUAL_DOUBLE(elevation_mask[0].Min, config->elevation_mask[0].Min);
  TEST_ASSERT_EQUAL_DOUBLE(elevation_mask[0].Max, config->elevation_mask[0].Max);
  TEST_ASSERT_EQUAL_DOUBLE(elevation_mask[1].Min, config->elevation_mask[1].Min);
  TEST_ASSERT_EQUAL_DOUBLE(elevation_mask[1].Max, config->elevation_mask[1].Max);
  TEST_ASSERT_EQUAL_DOUBLE(elevation_mask[2].Min, config->elevation_mask[2].Min);
  TEST_ASSERT_EQUAL_DOUBLE(elevation_mask[2].Max, config->elevation_mask[2].Max);
  TEST_ASSERT_EQUAL_DOUBLE(elevation_mask[3].Min, config->elevation_mask[3].Min);
  TEST_ASSERT_EQUAL_DOUBLE(elevation_mask[3].Max, config->elevation_mask[3].Max);

  TEST_ASSERT_EQUAL(23, config->cte_mode);
  TEST_ASSERT_EQUAL(true, config->position_and_orientation_valid);
}
