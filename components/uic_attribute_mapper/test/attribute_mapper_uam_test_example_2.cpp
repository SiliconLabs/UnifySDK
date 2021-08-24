

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
#include "attribute_store_fixt.h"
#include "datastore.h"
#include "attribute_mapper_engine.hpp"
#include "attribute_store_test_helpers.hpp"
#include "attribute_store_debug.h"

extern "C" {

// Short alias for this namespace
using namespace std;
using namespace attribute_store;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  attribute_store_register_attribute(
    111,
    "DOTDOT_ATTRIBUTE_ID_TEMPERATURE_MEASUREMENT_MEASURED_VALUE");
  attribute_store_register_attribute(222,
                                     "ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE");
  attribute_store_register_attribute(333, "ATTRIBUTE_SENSOR_MULTILEVEL_SCALE");
  attribute_store_register_attribute(444,
                                     "ATTRIBUTE_SENSOR_MULTILEVEL_PRECISION");
  attribute_store_register_attribute(
    555,
    "ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_VALUE");
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  attribute::root().delete_node();
}

// Example with string as branch descriptor
void test_mapper_engine_example()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_ep_type(attribute::root().type());
  e.reset();

  // Create attribute tree
  create_branch("DOTDOT_ATTRIBUTE_ID_TEMPERATURE_MEASUREMENT_MEASURED_VALUE")
    .set_reported<uint32_t>(100);

  create_branch("ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[1].ATTRIBUTE_SENSOR_"
                "MULTILEVEL_SCALE")
    .set_reported<uint32_t>(0);
  create_branch("ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[1].ATTRIBUTE_SENSOR_"
                "MULTILEVEL_PRECISION")
    .set_reported<uint32_t>(2);
  create_branch("ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[1].ATTRIBUTE_SENSOR_"
                "MULTILEVEL_SENSOR_VALUE")
    .set_reported<uint32_t>(10);

  create_branch("ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[2].ATTRIBUTE_SENSOR_"
                "MULTILEVEL_SCALE")
    .set_reported<uint32_t>(1);
  create_branch("ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[2].ATTRIBUTE_SENSOR_"
                "MULTILEVEL_PRECISION")
    .set_reported<uint32_t>(2);
  create_branch("ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[2].ATTRIBUTE_SENSOR_"
                "MULTILEVEL_SENSOR_VALUE")
    .set_reported<uint32_t>(10);

  attribute_store_log();

  // Load UAM file
  // Assumes that file located in uic/components/uic_attribute_mapper/test/rules/
  TEST_ASSERT_TRUE(e.load_file("../../../../components/uic_attribute_mapper/"
                               "test/rules/complex_test_map_2.uam"));

  // Activate rules
  check_branch("ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[1].ATTRIBUTE_SENSOR_"
               "MULTILEVEL_SENSOR_VALUE")
    .set_reported<uint32_t>(100);

  attribute_store_log();

  // Test
  TEST_ASSERT_EQUAL(
    10,
    check_branch("DOTDOT_ATTRIBUTE_ID_TEMPERATURE_MEASUREMENT_MEASURED_VALUE")
      .reported<uint32_t>());
}

// Example with string as branch descriptor
void test_mapper_engine_example_()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_ep_type(attribute::root().type());
  e.reset();

  // Create attribute tree
  create_branch("DOTDOT_ATTRIBUTE_ID_TEMPERATURE_MEASUREMENT_MEASURED_VALUE")
    .set_reported<uint32_t>(100);

  create_branch("ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[1].ATTRIBUTE_SENSOR_"
                "MULTILEVEL_SCALE")
    .set_reported<uint32_t>(1);
  create_branch("ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[1].ATTRIBUTE_SENSOR_"
                "MULTILEVEL_PRECISION")
    .set_reported<uint32_t>(2);
  create_branch("ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[1].ATTRIBUTE_SENSOR_"
                "MULTILEVEL_SENSOR_VALUE")
    .set_reported<uint32_t>(100);

  create_branch("ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[2].ATTRIBUTE_SENSOR_"
                "MULTILEVEL_SCALE")
    .set_reported<uint32_t>(1);
  create_branch("ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[2].ATTRIBUTE_SENSOR_"
                "MULTILEVEL_PRECISION")
    .set_reported<uint32_t>(2);
  create_branch("ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[2].ATTRIBUTE_SENSOR_"
                "MULTILEVEL_SENSOR_VALUE")
    .set_reported<uint32_t>(100);

  attribute_store_log();

  // Load UAM file
  // Assumes that file located in uic/components/uic_attribute_mapper/test/rules/
  TEST_ASSERT_TRUE(e.load_file("../../../../components/uic_attribute_mapper/"
                               "test/rules/complex_test_map_2.uam"));

  // Activate rules
  check_branch("ATTRIBUTE_SENSOR_MULTILEVEL_SENSOR_TYPE[1].ATTRIBUTE_SENSOR_"
               "MULTILEVEL_SENSOR_VALUE")
    .set_reported<uint32_t>(167000);

  attribute_store_log();

  // Test FIXME!
  //  TEST_ASSERT_EQUAL(
  //    750,
  //    check_branch("DOTDOT_ATTRIBUTE_ID_TEMPERATURE_MEASUREMENT_MEASURED_VALUE")
  //      .reported<uint32_t>());
}
}