

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
#include "attribute_mapper_engine.hpp"
#include "attribute_mapper_process.h"
#include "attribute_mapper.h"

#include "unity.h"

// Unify components
#include "attribute_store_fixt.h"
#include "attribute_store_type_registration.h"
#include "datastore.h"

// Test helpers
#include "contiki_test_helper.h"

using namespace attribute_store;

extern "C" {

// Mock components
#include "config_mock.h"

static const char *non_existing_uam_path = "./non/existing/path";
static const char *valid_uam_path
  = "../../../../applications/zpc/components/dotdot_mapper/rules";

attribute n1234;
attribute n1235;
attribute n0;
attribute n1;
attribute n4444;
attribute n3333;

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_init(":memory:");
  attribute_store_init();
  attribute_store_register_type(42, "Index node", 0, U16_STORAGE_TYPE);
  contiki_test_helper_init();
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
  /*
  *       /   \
  *    1234  1235
  *    /  \     \
  *  42:0 42:1  5555
  *  /      \
  * 3333    4444
  */
  attribute::root().delete_node();

  n3333 = attribute::root()
            .emplace_node(1234)
            .emplace_node<uint16_t>(42, 0)
            .emplace_node(3333);
  n4444 = attribute::root()
            .emplace_node(1234)
            .emplace_node<uint16_t>(42, 1)
            .emplace_node(4444);
  attribute::root().emplace_node(1235).emplace_node(5555);

  n1234 = attribute::root().emplace_node(1234);
  n1235 = attribute::root().emplace_node(1235);

  n0 = attribute::root().emplace_node(1234).emplace_node<uint16_t>(42, 0);
  n1 = attribute::root().emplace_node(1234).emplace_node<uint16_t>(42, 1);
  process_start(&unify_attribute_mapper_process, nullptr);
}

/// Called after each and every test
void tearDown()
{
  // Stop the process in between tests, so we can re-create a network
  // without evaluations.
  process_exit(&unify_attribute_mapper_process);
}

void test_mapper_engine_test()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_ep_type(attribute::root().type());
  e.reset();
  e.add_expression(R"(
    def three 3
    def mylightswitch 1235

    scope 0 {
      d'1234 = 2 * (d'1235) + (three & 0xff) - 0
      d'1234.42[1].4444 = (d'mylightswitch) + 0x64
      r'1234.42[0].11118888 = (d'9999) or (d'1235)
      r'1234.42[3].11118888 = (d'9999) or (d'1235)
      r'1 = r'2
      r'20 = (r'2 / (-20)) * 5
    }
  )");

  n1235.set_desired<uint32_t>(1);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(5, n1234.desired<uint32_t>());
  TEST_ASSERT_EQUAL(101, n4444.desired<uint32_t>());

  //  Check that we can create non existing attributes
  auto n11118888 = n0.child_by_type(11118888);
  contiki_test_helper_run(0);
  TEST_ASSERT_TRUE(n11118888.is_valid());
  TEST_ASSERT_EQUAL(1, n11118888.reported<int32_t>());

  // Check that we evaluate the right hand side dependencies

  auto n3 = n1234.emplace_node<uint16_t>(42, 3);
  contiki_test_helper_run(0);
  // As we have now created n3 the rule r'1234.42[3].11118888 = (d'9999) or (d'1235)
  // should succseed.
  auto n3_n11118888 = n3.child_by_type(11118888);
  TEST_ASSERT_TRUE(n3_n11118888.is_valid());
  TEST_ASSERT_EQUAL(1, n3_n11118888.reported<int32_t>());

  //Check that we delete the created node when we are deleting the n3 attribute
  n3.delete_node();
  TEST_ASSERT_FALSE(n3.child_by_type(11118888).is_valid());

  // Direct mapping check, (r'1 = r'2) create an attribute 2, check that
  // attribute 1 is created and has the same value
  attribute::root().add_node(2).set_reported<int32_t>(3);
  contiki_test_helper_run(0);
  auto n1 = attribute::root().child_by_type(1);
  TEST_ASSERT_TRUE(n1.is_valid());
  TEST_ASSERT_EQUAL(3, attribute_store_get_reported_number(n1));

  // Check that we kept intermediate decimal digit information in the computation:
  auto n20 = attribute::root().child_by_type(20);
  TEST_ASSERT_TRUE(n20.is_valid());
  // 3/ (-20) * 5 = -0.75, it should be rounded to -1
  TEST_ASSERT_EQUAL(-1, attribute_store_get_reported_number(n20));
}

void test_mapper_clear_desired()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_ep_type(attribute::root().type());
  e.reset();
  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 0 {
      d'1234 = d'1235
      r'1235 = r'1234
    }
  )"));

  n1234.set_reported<uint32_t>(0);
  contiki_test_helper_run(0);
  n1235.set_desired<uint32_t>(1);
  contiki_test_helper_run(0);

  //Check that both attributes have the same desired value
  TEST_ASSERT_EQUAL(1, n1234.desired<int32_t>());
  TEST_ASSERT_EQUAL(1, n1235.desired<int32_t>());
  TEST_ASSERT_EQUAL(0, n1234.reported<int32_t>());
  TEST_ASSERT_EQUAL(0, n1235.reported<int32_t>());

  //Setting the repported value should clear the desired value
  n1234.set_reported<uint32_t>(1);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(1, n1234.reported<int32_t>());
  TEST_ASSERT_EQUAL(1, n1235.reported<int32_t>());

  //Check that the repported value has been cleared
  try {
    TEST_ASSERT_EQUAL(1, n1235.desired<int32_t>());
    TEST_FAIL_MESSAGE("Exception not thrown");
  } catch (std::exception &e) {
  }
}

void test_mapper_cyclic_check()
{
  TEST_IGNORE_MESSAGE("Cyclic checker is disabled");

  MapperEngine &e = MapperEngine::get_instance();
  e.set_ep_type(attribute::root().type());
  e.reset();
  TEST_ASSERT_FALSE(e.add_expression(R"(
    scope 0 {
      d'1234 = d'1235
      d'1235 = d'1234
    }
  )"));

  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 0 {
      r'1234[0].1111 = d'1235
      r'1234[1].1111 = d'1235
    }
  )"));
}

void test_const_assignment_check()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_ep_type(attribute::root().type());
  e.reset();
  TEST_ASSERT_FALSE(e.add_expression(R"(
    scope 0 {
      d'1234 = d'1235
      d'1235 = 42
    }
  )"));
}

void test_parent_operator()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_ep_type(attribute::root().type());
  e.reset();

  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 0 {
      r'4444 = r'^.^.42[0] + 1
    }
  )"));

  n3333.set_reported(42);
  contiki_test_helper_run(0);

  TEST_IGNORE_MESSAGE("Make the hat operator work by beeing cleaver about the "
                      "depth in which we do evaluations.");

  TEST_ASSERT_EQUAL(43, n4444.reported<int>());
}

// We check that we can handle attribute type values all the way
// from 0 to UINT32_T max value
void test_mapper_engine_high_attribute_type()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_ep_type(attribute::root().type());
  e.reset();
  e.add_expression(R"(
    def attribute_1   0x0300400A
    def attribute_2   0xFEDCBA98
    def attribute_3   0xFFEEDDCC

    scope 0 {
      r'attribute_1 = (r'1234)
      r'attribute_2 = 2 * (r'1234)
      r'attribute_3 = 3 * (r'1234) / 3
    }
  )");

  attribute_store_set_reported_number(n1234, 3);
  contiki_test_helper_run(0);

  // Check that there new attributes have been created correctly
  auto attribute_1 = attribute::root().child_by_type(0x0300400A);
  TEST_ASSERT_TRUE(attribute_1.is_valid());
  TEST_ASSERT_EQUAL(3, attribute_store_get_reported_number(attribute_1));

  auto attribute_2 = attribute::root().child_by_type(0xFEDCBA98);
  TEST_ASSERT_TRUE(attribute_2.is_valid());
  TEST_ASSERT_EQUAL(6, attribute_store_get_reported_number(attribute_2));

  auto attribute_3 = attribute::root().child_by_type(0xFFEEDDCC);
  TEST_ASSERT_TRUE(attribute_3.is_valid());
  TEST_ASSERT_EQUAL(3, attribute_store_get_reported_number(attribute_3));
}

void test_mapper_engine_attribute_deletion()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_ep_type(attribute::root().type());
  e.reset();
  e.add_expression(R"(
    scope 0 {
      // Bind the existence of 1 to 1234.
      e'1 = e'1234
      r'1 = (r'1234)
    }
  )");

  attribute_store_set_reported_number(n1234, 3);
  contiki_test_helper_run(0);

  // Check that there new attributes have been created correctly
  auto attribute_1 = attribute::root().child_by_type(1);
  TEST_ASSERT_TRUE(attribute_1.is_valid());
  TEST_ASSERT_EQUAL(3, attribute_store_get_reported_number(attribute_1));

  // Now delete attribute 1234. attribute 1 should be deleted too.
  n1234.delete_node();
  attribute_1 = attribute::root().child_by_type(1);
  TEST_ASSERT_FALSE(attribute_1.is_valid());
}

void test_mapper_engine_existence_link()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_ep_type(attribute::root().type());
  e.reset();
  e.add_expression(R"(
    scope 0 {
      // Bind the existence of 2 to 3.
      e'2 = e'3
    }
  )");

  auto attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_FALSE(attribute_2.is_valid());

  auto attribute_3 = attribute::root().add_node(3);

  // Check that there new attributes have been created correctly
  attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_TRUE(attribute_2.is_valid());

  // Now delete attribute 1234. attribute 1 should be deleted too.
  attribute_3.delete_node();
  attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_FALSE(attribute_2.is_valid());
}

void test_mapper_engine_instance_existence()
{
  TEST_IGNORE_MESSAGE("UIC-2183");

  MapperEngine &e = MapperEngine::get_instance();
  e.set_ep_type(attribute::root().type());
  e.reset();
  e.add_expression(R"(
    scope 0 {
      // I want to create attribute 2 if there is an attribute 3 with value 10.
      e'2 = e'3[10]
    }
  )");

  auto attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_FALSE(attribute_2.is_valid());

  auto attribute_3_1 = attribute::root().add_node(3);
  attribute_store_set_reported_number(attribute_3_1, 1);
  contiki_test_helper_run(0);

  // Still should not exist, attribute 3_1 has value 1
  attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_FALSE(attribute_2.is_valid());

  // Now create an attribute type 3 with value 10, attribute 2 should get created.
  auto attribute_3_10 = attribute::root().add_node(3);
  attribute_store_set_reported_number(attribute_3_10, 10);
  contiki_test_helper_run(0);

  attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_TRUE(attribute_2.is_valid());

  // Create yet another attribute 3 with value != 10, attribute 2 should survive
  auto attribute_3_2 = attribute::root().add_node(3);
  attribute_store_set_reported_number(attribute_3_2, 2);
  contiki_test_helper_run(0);

  attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_TRUE(attribute_2.is_valid());

  // Now delete attribute attribute_3_10. attribute 2 should be deleted too.
  attribute_3_10.delete_node();
  attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_FALSE(attribute_2.is_valid());
}

void test_mapper_engine_instance_existence_dependency()
{
  TEST_IGNORE_MESSAGE("UIC-2183");

  MapperEngine &e = MapperEngine::get_instance();
  e.set_ep_type(attribute::root().type());
  e.reset();
  e.add_expression(R"(
    scope 0 {
      // I want to (create and )set attribute 2 if there is an attribute 3 with value 10.
      // Value 2 should probably be set if there is an attribute 3 but none with value 10.
      r'2 = if (e'3[10]) 1 2
    }
  )");

  auto attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_FALSE(attribute_2.is_valid());

  auto attribute_3_1 = attribute::root().add_node(3);
  attribute_store_set_reported_number(attribute_3_1, 1);
  contiki_test_helper_run(0);

  // Now attribute 2 should have value 2, there is no instance with 10.
  attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_TRUE(attribute_2.is_valid());
  TEST_ASSERT_EQUAL(2, attribute_store_get_reported_number(attribute_2));

  // Now create an attribute type 3 with value 10, attribute 2 should be updated to 1
  auto attribute_3_10 = attribute::root().add_node(3);
  attribute_store_set_reported_number(attribute_3_10, 10);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(1, attribute_store_get_reported_number(attribute_2));

  attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_TRUE(attribute_2.is_valid());

  // Create yet another attribute 3 with value != 10, attribute 2 should stay at 1
  auto attribute_3_2 = attribute::root().add_node(3);
  attribute_store_set_reported_number(attribute_3_2, 2);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(1, attribute_store_get_reported_number(attribute_2));

  // Delete attribute 3 with value 2, attribute 2 should stay at 1
  attribute_3_2.delete_node();
  TEST_ASSERT_EQUAL(1, attribute_store_get_reported_number(attribute_2));

  // Now delete attribute attribute_3_10. attribute 2 should go back to 2
  attribute_3_10.delete_node();
  TEST_ASSERT_EQUAL(1, attribute_store_get_reported_number(attribute_2));
}

void test_attribute_mapper_config_init()
{
  // Config init will just add the mapdir configuration.
  config_add_string_ExpectAndReturn(
    "mapdir",
    "directory for attribute mapping files(*.uam)",
    "/usr/share/uic/rules",
    CONFIG_STATUS_OK);
  attribute_mapper_config_init();
}

void test_mapper_engine_init_fail()
{
  // Try with a non-existing path:
  config_get_as_string_ExpectAndReturn("mapdir", NULL, CONFIG_STATUS_OK);
  config_get_as_string_IgnoreArg_result();
  config_get_as_string_ReturnThruPtr_result(&non_existing_uam_path);

  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, attribute_mapper_init());
}

void test_mapper_engine_init_ok()
{
  // Try with a valid path:
  config_get_as_string_ExpectAndReturn("mapdir", NULL, CONFIG_STATUS_OK);
  config_get_as_string_IgnoreArg_result();
  config_get_as_string_ReturnThruPtr_result(&valid_uam_path);

  TEST_ASSERT_EQUAL(SL_STATUS_OK, attribute_mapper_init());
}
}
