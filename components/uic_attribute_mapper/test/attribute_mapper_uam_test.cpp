

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
#include "uic_version.h"
using namespace attribute_store;

extern "C" {

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
  attribute::root().delete_node();
}

/// Test uam files syntax
void test_uam_test()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_ep_type(attribute::root().type());
  e.reset();

  // Try to load and parse all uam files in rules directory
  TEST_ASSERT_TRUE(e.load_path( SOURCE_DIR "/applications/zpc/components/dotdot_mapper/rules" ));
}
}
