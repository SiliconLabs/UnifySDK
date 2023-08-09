

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
#include "workaround.hpp"
#include "attribute_mapper_process.h"
#include "attribute_mapper.h"

#include "unity.h"

// Unify components
#include "attribute_store_fixt.h"
#include "attribute_store_type_registration.h"
#include "sl_log.h"
#include "datastore.h"
#include "uic_version.h"

// Test helpers
#include "contiki_test_helper.h"

using namespace attribute_store;

extern "C" {

// Mock components
#include "config_mock.h"

static const char *non_existing_uam_path = "./non/existing/path";
static const char *valid_uam_path        = "./rules";

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
  contiki_test_helper_run(0);
}

/// Called after each and every test
void tearDown()
{
  // Stop the process in between tests, so we can re-create a network
  // without evaluations.
  process_exit(&unify_attribute_mapper_process);
  contiki_test_helper_run(0);
}

void test_mapper_engine_test()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
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
  TEST_ASSERT_EQUAL(5, attribute_store_get_desired_number(n1234));
  TEST_ASSERT_EQUAL(101, attribute_store_get_desired_number(n4444));

  //  Check that we can create non existing attributes
  auto n11118888 = n0.child_by_type(11118888);
  contiki_test_helper_run(0);
  TEST_ASSERT_TRUE(n11118888.is_valid());
  TEST_ASSERT_EQUAL(1, attribute_store_get_reported_number(n11118888));

  // Check that we evaluate the right hand side dependencies

  auto n3 = n1234.emplace_node<uint16_t>(42, 3);
  contiki_test_helper_run(0);
  // As we have now created n3 the rule r'1234.42[3].11118888 = (d'9999) or (d'1235)
  // should succseed.
  auto n3_n11118888 = n3.child_by_type(11118888);
  TEST_ASSERT_TRUE(n3_n11118888.is_valid());
  TEST_ASSERT_EQUAL(1, attribute_store_get_reported_number(n3_n11118888));

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
  e.set_common_parent_type(attribute::root().type());
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
  TEST_ASSERT_EQUAL(1, attribute_store_get_desired_number(n1234));
  TEST_ASSERT_EQUAL(1, attribute_store_get_desired_number(n1235));
  TEST_ASSERT_EQUAL(0, attribute_store_get_reported_number(n1234));
  TEST_ASSERT_EQUAL(0, attribute_store_get_reported_number(n1235));

  //Setting the repported value should clear the desired value
  n1234.set_reported<uint32_t>(1);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(1, attribute_store_get_reported_number(n1234));
  TEST_ASSERT_EQUAL(1, attribute_store_get_reported_number(n1235));

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
  e.set_common_parent_type(attribute::root().type());
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
  e.set_common_parent_type(attribute::root().type());
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
  e.set_common_parent_type(attribute::root().type());
  e.reset();

  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 0 {
      r'4444 = r'^.^.42[0] + 1
    }
  )"));

  n3333.set_reported(42);
  contiki_test_helper_run(0);

  TEST_IGNORE_MESSAGE("Make the hat operator work by beeing clever about the "
                      "depth in which we do evaluations.");

  TEST_ASSERT_EQUAL(43, n4444.reported<int>());
}

// We check that we can handle attribute type values all the way
// from 0 to UINT32_T max value
void test_mapper_engine_high_attribute_type()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
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
  e.set_common_parent_type(attribute::root().type());
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
  e.set_common_parent_type(attribute::root().type());
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

  // Now delete attribute 3. attribute 2 should be deleted too.
  attribute_3.delete_node();
  attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_FALSE(attribute_2.is_valid());
}

void test_mapper_engine_instance_existence()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();
  e.add_expression(R"(
    scope 0 {
      // I want to create attribute 2 if there is an attribute 3 with value 10.
      e'2 = fn_are_all_defined(r'3[10])
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
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();
  e.add_expression(R"(
    scope 0 {
      // I want to (create and )set attribute 2 if there is an attribute 3 with value 10.
      // Value 2 should probably be set if there is an attribute 3 but none with value 10.
      r'2 =  if (fn_are_all_defined(r'3[10])) 1 2
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
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(2, attribute_store_get_reported_number(attribute_2));
}

void test_mapper_engine_instance_creation_with_index()
{
  // Remove the default tree.
  attribute::root().delete_node();

  // Get the engine started:
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();
  e.add_expression(R"(
    scope 0 {
      // I want to have an instance of an attribute 2 with reported value 10 if
      // there is an attribute 3 with value 10.
      i:r'2[10] =  if (fn_are_all_defined(r'3[10])) 1 0
    }
  )");

  auto attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_FALSE(attribute_2.is_valid());

  auto attribute_3 = attribute::root().add_node(3);
  attribute_store_set_reported_number(attribute_3, 1);
  contiki_test_helper_run(0);
  TEST_ASSERT_FALSE(attribute::root().child_by_type(2).is_valid());

  auto other_attribute_2 = attribute::root().add_node(2);
  attribute_store_set_reported_number(other_attribute_2, 2);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(2, attribute_store_get_reported_number(other_attribute_2));

  attribute_store_set_reported_number(attribute_3, 2);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(other_attribute_2, attribute::root().child_by_type(2));
  TEST_ASSERT_FALSE(attribute::root().child_by_type(2, 1).is_valid());

  attribute_store_set_reported_number(attribute_3, 10);
  contiki_test_helper_run(0);
  attribute_2 = attribute::root().child_by_type(2, 1);
  TEST_ASSERT_TRUE(attribute_2.is_valid());
  TEST_ASSERT_EQUAL(10, attribute_store_get_reported_number(attribute_2));

  // Now check that the instance gets deleted if I change the value of attribute 3:
  attribute_store_set_reported_number(attribute_3, 1);
  contiki_test_helper_run(0);
  attribute_2 = attribute::root().child_by_type(2, 1);
  TEST_ASSERT_FALSE(attribute_2.is_valid());
}

void test_mapper_engine_instance_creation()
{
  // Remove the default tree.
  attribute::root().delete_node();

  // Get the engine started:
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();
  e.add_expression(R"(
    scope 0 {
      // Here I want an instance of 2 with the value 1 if attribute 3 has reported value 10.
      // No deletion once the instance of 2 with value 1 has been created.
      i:r'2 =  if (fn_are_all_defined(r'3[10])) 1 undefined
    }
  )");

  auto attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_FALSE(attribute_2.is_valid());

  // Create and set attribute 3 to 1:
  auto attribute_3 = attribute::root().add_node(3);
  attribute_store_set_reported_number(attribute_3, 1);
  contiki_test_helper_run(0);
  TEST_ASSERT_FALSE(attribute::root().child_by_type(2).is_valid());

  // Set Create an attribute 2 with value 2. it won't affect anything
  auto other_attribute_2 = attribute::root().add_node(2);
  attribute_store_set_reported_number(other_attribute_2, 2);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(2, attribute_store_get_reported_number(other_attribute_2));

  // Set attribute 3 to 2:
  attribute_store_set_reported_number(attribute_3, 2);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(other_attribute_2, attribute::root().child_by_type(2));
  TEST_ASSERT_FALSE(attribute::root().child_by_type(2, 1).is_valid());

  // Set attribute 3 to 10:
  attribute_store_set_reported_number(attribute_3, 10);
  contiki_test_helper_run(0);
  attribute_2 = attribute::root().child_by_type(2, 1);
  TEST_ASSERT_TRUE(attribute_2.is_valid());
  TEST_ASSERT_EQUAL(1, attribute_store_get_reported_number(attribute_2));

  // Now delete attribute 3, attribute 2 instance will survive
  attribute_3.delete_node();
  contiki_test_helper_run(0);
  TEST_ASSERT_TRUE(attribute_2.is_valid());
}

void test_mapper_engine_instance_creation_scope_priority()
{
  // Remove the default tree.
  attribute::root().delete_node();

  // Get the engine started:
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();
  e.add_expression(R"(
    // Let's just check that all expressions get evaluated, because they are instances
    scope 0 {
      i:r'2[10] =  if (r'3 == 10) 1 0
    }
    scope 2 {
      i:r'2[11] =  if (r'3 == 10) 1 0
    }
    scope 3 {
      i:r'2[12] =  if (r'3 == 10) 1 0
    }
    scope 4 {
      i:r'2[13] =  if (r'3 == 10) 1 undefined
    }
  )");

  TEST_ASSERT_FALSE(attribute::root().child_by_type(2));

  // Create and set attribute 3 to 1:
  auto attribute_3 = attribute::root().add_node(3);
  attribute_store_set_reported_number(attribute_3, 1);
  contiki_test_helper_run(0);
  TEST_ASSERT_FALSE(attribute::root().child_by_type(2));

  // Set attribute 3 to 10:
  attribute_store_set_reported_number(attribute_3, 10);
  contiki_test_helper_run(0);
  auto attribute_2_10 = attribute::root().child_by_type(2, 0);
  auto attribute_2_11 = attribute::root().child_by_type(2, 1);
  auto attribute_2_12 = attribute::root().child_by_type(2, 2);
  auto attribute_2_13 = attribute::root().child_by_type(2, 3);
  TEST_ASSERT_TRUE(attribute_2_10.is_valid());
  TEST_ASSERT_TRUE(attribute_2_11.is_valid());
  TEST_ASSERT_TRUE(attribute_2_12.is_valid());
  TEST_ASSERT_TRUE(attribute_2_13.is_valid());
  TEST_ASSERT_EQUAL(10, attribute_store_get_reported_number(attribute_2_10));
  TEST_ASSERT_EQUAL(11, attribute_store_get_reported_number(attribute_2_11));
  TEST_ASSERT_EQUAL(12, attribute_store_get_reported_number(attribute_2_12));
  TEST_ASSERT_EQUAL(13, attribute_store_get_reported_number(attribute_2_13));

  // Only instance 13 will survive a deletion of attribute 3:
  attribute_3.delete_node();
  contiki_test_helper_run(0);
  TEST_ASSERT_FALSE(attribute_2_10.is_valid());
  TEST_ASSERT_FALSE(attribute_2_11.is_valid());
  TEST_ASSERT_FALSE(attribute_2_12.is_valid());
  TEST_ASSERT_TRUE(attribute_2_13.is_valid());
}

void test_mapper_engine_instance_creation_conflicting_priority()
{
  // Remove the default tree.
  attribute::root().delete_node();

  // Get the engine started:
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();
  e.add_expression(R"(
    // Let's play with the limit a bit, get conflicting instances based on priority
    // This is bad performance due to the fact that we have to evaluate everything everytime.
    scope 0 {
      i:d'2.4.5[10] =  if (r'3 < 5) 1 0
    }
    scope 2 {
      i:d'2.4.5[10] =  if (r'3 <= 10) 0 1
    }
  )");

  // Create and set attribute 3 to 5: (scope 2 active)
  auto attribute_3 = attribute::root().add_node(3);
  attribute_store_set_reported_number(attribute_3, 5);
  contiki_test_helper_run(0);

  // Instances will get to create only the last attribute of the branch.
  TEST_ASSERT_FALSE(attribute::root().child_by_type(5));
  auto attribute_4 = attribute::root().add_node(2).add_node(4);
  attribute_store_set_reported_number(attribute_3, 11);
  contiki_test_helper_run(0);
  auto attribute_5_10 = attribute_4.child_by_type(5);
  TEST_ASSERT_TRUE(attribute_5_10.is_valid());
  TEST_ASSERT_EQUAL(10, attribute_store_get_desired_number(attribute_5_10));

  // Now set attribute 3 to 2: Scope 0 will want to erase attribute 2, scope 2 will want to create it.
  attribute_store_set_reported_number(attribute_3, 12);
  contiki_test_helper_run(0);
  TEST_ASSERT_FALSE(attribute_5_10.is_valid());
  attribute_5_10 = attribute_4.child_by_type(5);
  TEST_ASSERT_TRUE(attribute_5_10.is_valid());
  TEST_ASSERT_EQUAL(10, attribute_store_get_desired_number(attribute_5_10));
}

void test_attribute_mapper_config_init()
{
  // Config init will just add the mapdir configuration.
  config_add_string_ExpectAndReturn(
    "mapdir",
    "directory for attribute mapping files(*.uam)",
    DEFAULT_RULES_MAPPING_DIR,
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

void test_mapper_engine_triggers_no_evaluation_on_unrelated_parent_value_change()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();
  e.add_expression(R"(
    scope 0 {
      // r'2 should be updated if attribute 2 or 4 are updated, not 9 or 8.
      r'2 = r'9.3[2].8.4
    }
  )");

  auto attribute_9 = attribute::root().add_node(9);
  auto attribute_3 = attribute_9.add_node(3);
  auto attribute_8 = attribute_3.add_node(8);
  auto attribute_4 = attribute_8.add_node(4);
  attribute_store_set_reported_number(attribute_3, 2);
  attribute_store_set_reported_number(attribute_4, 4);
  TEST_ASSERT_TRUE(attribute_mapper_has_pending_evaluations());

  contiki_test_helper_run(0);
  auto attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_TRUE(attribute_2.is_valid());
  TEST_ASSERT_EQUAL(4, attribute_store_get_reported_number(attribute_2));

  attribute_store_set_reported_number(attribute_9, 34);
  TEST_ASSERT_FALSE(attribute_mapper_has_pending_evaluations());

  attribute_store_set_reported_number(attribute_8, 20);
  TEST_ASSERT_FALSE(attribute_mapper_has_pending_evaluations());
}

void test_mapper_engine_are_all_defined_built_in_function()
{
  attribute::root().delete_node();
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();
  e.add_expression(R"(
    scope 0 {
      r'1 = fn_are_all_defined(r'9, r'10)
      r'2 = fn_are_all_defined(r'9, r'6.3[2].8, r'10)
    }
  )");

  // None of r'1 / r'2 are defined yet.
  contiki_test_helper_run(0);
  auto attribute_1 = attribute::root().child_by_type(1);
  TEST_ASSERT_FALSE(attribute_1.is_valid());
  auto attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_FALSE(attribute_2.is_valid());

  // Create attribute r'9:
  // Just adding the node does not trigger any dependency check unfortunately.
  auto attribute_9 = attribute::root().add_node(9);
  attribute_store_set_reported_number(attribute_9, 0);
  contiki_test_helper_run(0);

  // Now both r'1 and r'2 should be set to 0, because not everything is defined:
  attribute_1 = attribute::root().child_by_type(1);
  TEST_ASSERT_TRUE(attribute_1.is_valid());
  TEST_ASSERT_EQUAL(0, attribute_store_get_reported_number(attribute_1));
  attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_TRUE(attribute_2.is_valid());
  TEST_ASSERT_EQUAL(0, attribute_store_get_reported_number(attribute_2));

  // Create r'10 and define it:
  auto attribute_10 = attribute::root().add_node(10);
  attribute_store_set_reported_number(attribute_10, 0);
  contiki_test_helper_run(0);
  // Now r'1 indicates that both 9 and 10 are defined, r'2 is still unhappy.
  TEST_ASSERT_EQUAL(1, attribute_store_get_reported_number(attribute_1));
  TEST_ASSERT_EQUAL(0, attribute_store_get_reported_number(attribute_2));

  auto attribute_3 = attribute::root().add_node(6).add_node(3);
  attribute_store_set_reported_number(attribute_3, 2);
  auto attribute_8 = attribute_3.add_node(8);
  attribute_store_set_reported_number(attribute_8, 2);
  contiki_test_helper_run(0);
  // Now everybody is happy.
  TEST_ASSERT_EQUAL(1, attribute_store_get_reported_number(attribute_1));
  TEST_ASSERT_EQUAL(1, attribute_store_get_reported_number(attribute_2));

  attribute_store_undefine_reported(attribute_9);
  contiki_test_helper_run(0);
  // Now everybody is upset again.
  TEST_ASSERT_EQUAL(0, attribute_store_get_reported_number(attribute_1));
  TEST_ASSERT_EQUAL(0, attribute_store_get_reported_number(attribute_2));
}

void test_mapper_engine_log10_built_in_function()
{
  attribute::root().delete_node();
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();
  e.add_expression(R"(
    scope 0 {
      r'2 = fn_log10(r'9, r'10)
    }
  )");

  // None of r'9 / r'10 are defined yet.
  contiki_test_helper_run(0);
  auto attribute_9  = attribute::root().add_node(9);
  auto attribute_10 = attribute::root().add_node(10);
  contiki_test_helper_run(0);
  auto attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_FALSE(attribute_2.is_valid());
  attribute_store_set_reported_number(attribute_10, 100);
  contiki_test_helper_run(0);
  attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_TRUE(attribute_2.is_valid());
  TEST_ASSERT_EQUAL(2, attribute_store_get_reported_number(attribute_2));

  // r'9 has priority over r'10
  attribute_store_set_reported_number(attribute_9, 10000);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(4, attribute_store_get_reported_number(attribute_2));
}

void test_mapper_engine_scream_with_unknown_functions()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();

  // This should go okay:
  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 0 {
      r'2 = r'3[10].5 + 3 * (fn_absolute_value(r'1234,(r'2 / (-20)) * 5 % 6) / 3)
      r'2 = r'3[10].4.6.7[fn_min_value(r'1,r'2)] + 10 ** 4
    }
  )"));

  // This should work: No limit on encapsulating functions
  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 0 {
      r'2 = fn_absolute_value(fn_absolute_value(fn_absolute_value(fn_absolute_value(r'1))))
    }
  )"));

  // This should not work: fn_unknown_function encapsulated
  TEST_ASSERT_FALSE(e.add_expression(R"(
    scope 0 {
      r'2 = fn_absolute_value(fn_absolute_value(fn_unknown_function(fn_absolute_value(r'1))))
    }
  )"));

  // This should not work: (fn_min_value_x)
  TEST_ASSERT_FALSE(e.add_expression(R"(
    scope 0 {
      r'2 = r'3[10].5 + 3 * (fn_absolute_value(r'1234,(r'2 / (-20)) * 5 % 6) / 3)
      r'2 = r'3[10].4.6.7[fn_min_value_x(r'1,r'2)] + 10 ** 4
    }
  )"));

  // This should not work either: (fn_Absolute_value)
  TEST_ASSERT_FALSE(e.add_expression(R"(
    scope 0 {
      r'2 = r'3[10].5 + 3 * (fn_Absolute_value(r'1234,(r'2 / (-20)) * 5 % 6) / 3)
      r'2 = r'3[10].4.6.7[fn_min_value(r'1,r'2)] + 10 ** 4
    }
  )"));

  // This should not work either: (fn_min_value_x)
  TEST_ASSERT_FALSE(e.add_expression(R"(
    scope 0 {
      r'2 = r'3[10].4.fn_min_value_x(r'1,r'2).7[4] + 10 ** 4
    }
  )"));

  // This should not work either: (fn_unknown)
  TEST_ASSERT_FALSE(e.add_expression(R"(
    scope 0 {
      r'2 = r'3[10].4.5.fn_min_value(r'1,r'2+fn_unknown(1,2))[4] + 10 ** 4
    }
  )"));
}

void test_mapper_engine_expression_for_built_in_functions()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();

  // There must be at least one expression in a function call
  TEST_ASSERT_FALSE(e.add_expression(R"(
    scope 0 {
      r'2 = fn_average_value() * r'5
    }
  )"));
}

void test_mapper_engine_no_common_parent_type_configured()
{
  MapperEngine &e = MapperEngine::get_instance();
  attribute_mapper_set_endpoint_id_attribute_type(ATTRIBUTE_STORE_INVALID_NODE);
  e.reset();
  TEST_ASSERT_TRUE(e.add_expression(R"( scope 0 { r'2 = r'5} )"));

  // Try to get r'2 mapped while the endpoint type does not match
  auto attribute_5 = attribute::root().add_node(5);
  attribute_store_set_reported_number(attribute_5, 5);
  contiki_test_helper_run(0);

  // Evaluation and mapping will fail without a proper endpoint type set.
  auto attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_FALSE(attribute_2.is_valid());

  // Set a valid endpoint type:
  attribute_mapper_set_endpoint_id_attribute_type(attribute::root().type());
  attribute_store_set_reported_number(attribute_5, 4);
  contiki_test_helper_run(0);
  attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_TRUE(attribute_2.is_valid());
}

void test_mapper_engine_load_scope_configurations()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();

  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 0 chain_reaction(0) clear_desired(0) {r'2 =  r'5}
  )"));

  // Unknown setting won't be accepted
  TEST_ASSERT_FALSE(e.add_expression(R"(
    scope 0 chain_reaction(0) new_unknown_setting(0) {r'2 =  r'5}
  )"));

  // setting value has to be integer
  TEST_ASSERT_FALSE(e.add_expression(R"(
    scope 0 chain_reaction(0) clear_desired(on) {r'2 =  r'5}
  )"));
}

void test_mapper_engine_load_multiple_scopes()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();
  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 0 chain_reaction(0) clear_desired(0) {r'2 =  r'5}
    scope 0 chain_reaction(1) {r'3 =  r'4}
    scope 0 clear_desired(1)  {r'20 =  d'4}
  )"));
}

void test_mapper_engine_clear_desired_configuration()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();

  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 0 clear_desired(1) {r'2 =  r'5}
  )"));

  auto attribute_5 = attribute::root().add_node(5);
  auto attribute_2 = attribute::root().add_node(2);
  attribute_store_set_desired_number(attribute_2, 20);
  attribute_store_set_reported_number(attribute_5, 5);
  contiki_test_helper_run(0);

  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(attribute_2));
  TEST_ASSERT_EQUAL(5, attribute_store_get_reported_number(attribute_2));

  e.reset();
  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 0 clear_desired(0) {r'2 =  r'5}
  )"));

  attribute_store_set_desired_number(attribute_2, 20);
  attribute_store_set_reported_number(attribute_5, 5);
  contiki_test_helper_run(0);

  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(attribute_2));
  TEST_ASSERT_EQUAL(5, attribute_store_get_reported_number(attribute_2));
  TEST_ASSERT_EQUAL(20, attribute_store_get_desired_number(attribute_2));
}

void test_mapper_engine_chain_reaction_configuration()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();

  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 0 chain_reaction(1) {
      r'2 =  r'5
      r'3 =  r'2
    }
  )"));

  auto attribute_5 = attribute::root().add_node(5);
  attribute_store_set_reported_number(attribute_5, 5);
  contiki_test_helper_run(0);

  auto attribute_2 = attribute::root().child_by_type(2);
  auto attribute_3 = attribute::root().child_by_type(3);
  TEST_ASSERT_TRUE(attribute_2.is_valid());
  TEST_ASSERT_TRUE(attribute_3.is_valid());

  TEST_ASSERT_EQUAL(5, attribute_store_get_reported_number(attribute_2));
  TEST_ASSERT_EQUAL(5, attribute_store_get_reported_number(attribute_3));

  attribute::root().delete_node();
  e.reset();
  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 0 chain_reaction(0) {
      r'2 =  r'5
      r'3 =  r'2
    }
  )"));

  attribute_5 = attribute::root().add_node(5);
  attribute_store_set_reported_number(attribute_5, 5);
  contiki_test_helper_run(0);

  attribute_2 = attribute::root().child_by_type(2);
  attribute_3 = attribute::root().child_by_type(3);
  TEST_ASSERT_TRUE(attribute_2.is_valid());
  TEST_ASSERT_FALSE(attribute_3.is_valid());

  TEST_ASSERT_EQUAL(5, attribute_store_get_reported_number(attribute_2));
}

void test_mapper_engine_multi_scopes_with_different_configurations()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();

  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 1 chain_reaction(0) clear_desired(0) {
      r'11 =  r'2
      r'2  =  r'11
      d'11 =  d'2
      d'2  =  d'11
    }
    scope 2 chain_reaction(0) clear_desired(1) {
      r'3 =  d'4
      r'4 =  d'3
    }
    scope 3 chain_reaction(1) clear_desired(0) {
      d'6 =  r'5
      r'6 =  d'6
    }
    scope 4 chain_reaction(2) clear_desired(2) {
      r'8 =  d'9
      r'9 =  r'8
      r'10 =  r'9
    }
  )"));

  // Testing scope 0
  auto attribute_11 = attribute::root().add_node(11);
  attribute_store_set_reported_number(attribute_11, 5);
  contiki_test_helper_run(0);
  auto attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_TRUE(attribute_2.is_valid());
  TEST_ASSERT_EQUAL(5, attribute_store_get_reported_number(attribute_2));

  attribute_store_set_desired_number(attribute_11, 6);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(5, attribute_store_get_reported_number(attribute_11));
  TEST_ASSERT_EQUAL(5, attribute_store_get_reported_number(attribute_2));
  TEST_ASSERT_EQUAL(6, attribute_store_get_desired_number(attribute_11));
  TEST_ASSERT_EQUAL(6, attribute_store_get_desired_number(attribute_2));

  attribute_store_set_desired_number(attribute_2, 7);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(5, attribute_store_get_reported_number(attribute_11));
  TEST_ASSERT_EQUAL(5, attribute_store_get_reported_number(attribute_2));
  TEST_ASSERT_EQUAL(7, attribute_store_get_desired_number(attribute_11));
  TEST_ASSERT_EQUAL(7, attribute_store_get_desired_number(attribute_2));

  attribute_store_set_reported_number(attribute_2, 2);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(2, attribute_store_get_reported_number(attribute_11));
  TEST_ASSERT_EQUAL(2, attribute_store_get_reported_number(attribute_2));
  TEST_ASSERT_EQUAL(7, attribute_store_get_desired_number(attribute_11));
  TEST_ASSERT_EQUAL(7, attribute_store_get_desired_number(attribute_2));

  // Testing second scope 1
  auto attribute_3 = attribute::root().add_node(3);
  attribute_store_set_desired_number(attribute_3, 3);
  contiki_test_helper_run(0);
  auto attribute_4 = attribute::root().child_by_type(4);
  TEST_ASSERT_TRUE(attribute_4.is_valid());
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(attribute_4));
  TEST_ASSERT_EQUAL(3, attribute_store_get_reported_number(attribute_4));
  TEST_ASSERT_EQUAL(3, attribute_store_get_desired_number(attribute_3));

  attribute_store_set_desired_number(attribute_4, 4);
  contiki_test_helper_run(0);
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(attribute_3));
  TEST_ASSERT_EQUAL(4, attribute_store_get_reported_number(attribute_3));
  TEST_ASSERT_EQUAL(4, attribute_store_get_desired_number(attribute_4));

  // Testing third scope 2
  auto attribute_5 = attribute::root().add_node(5);
  auto attribute_6 = attribute::root().add_node(6);
  attribute_store_set_reported_number(attribute_5, 5);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(5, attribute_store_get_reported_number(attribute_5));
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(attribute_5));
  TEST_ASSERT_EQUAL(5, attribute_store_get_reported_number(attribute_6));
  TEST_ASSERT_EQUAL(5, attribute_store_get_desired_number(attribute_6));

  // Testing scope 3
  auto attribute_9 = attribute::root().add_node(9);
  attribute_store_set_desired_number(attribute_9, 9);
  contiki_test_helper_run(0);
  auto attribute_8  = attribute::root().child_by_type(8);
  auto attribute_10 = attribute::root().child_by_type(10);
  TEST_ASSERT_TRUE(attribute_8.is_valid());
  TEST_ASSERT_TRUE(attribute_10.is_valid());
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(attribute_9));
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(attribute_8));
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(attribute_10));
  TEST_ASSERT_EQUAL(9, attribute_store_get_reported_number(attribute_8));
  TEST_ASSERT_EQUAL(9, attribute_store_get_reported_number(attribute_9));
  TEST_ASSERT_EQUAL(9, attribute_store_get_reported_number(attribute_10));
}

void test_mapper_engine_manual_ignore_override()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();

  TEST_ASSERT_TRUE(e.add_expression(R"(scope 0 { r'2 =  r'5})"));

  auto attribute_5 = attribute::root().add_node(5);
  attribute_mapper_pause_reactions_to_attribute_updates(attribute_5);
  attribute_store_set_reported_number(attribute_5, 5);
  contiki_test_helper_run(0);

  auto attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_FALSE(attribute_2.is_valid());

  attribute_mapper_resume_reactions_to_attribute_updates(attribute_5);
  attribute_store_set_reported_number(attribute_5, 0);
  contiki_test_helper_run(0);
  attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_TRUE(attribute_2.is_valid());

  // Now Check that if some other component paused mapping to an attribute update,
  // the mapper does not unpause it.
  TEST_ASSERT_TRUE(
    e.add_expression(R"(scope 0 chain_reaction(0) { r'3 =  r'6})"));
  auto attribute_6 = attribute::root().add_node(6);
  attribute_store_set_reported_number(attribute_6, 6);
  contiki_test_helper_run(0);
  auto attribute_3 = attribute::root().child_by_type(3);
  TEST_ASSERT_TRUE(attribute_3.is_valid());
  TEST_ASSERT_EQUAL(6, attribute_store_get_reported_number(attribute_3));
  attribute_mapper_pause_reactions_to_attribute_updates(attribute_6);
  attribute_store_set_reported_number(attribute_6, 7);
  contiki_test_helper_run(0);
  // Manually paused attribute_6 effects, so attribute 3 should never change value.
  TEST_ASSERT_EQUAL(6, attribute_store_get_reported_number(attribute_3));
  attribute_store_set_reported_number(attribute_6, 8);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(6, attribute_store_get_reported_number(attribute_3));
  attribute_store_set_reported_number(attribute_6, 9);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(6, attribute_store_get_reported_number(attribute_3));

  // Now the external component resumes:
  attribute_mapper_resume_reactions_to_attribute_updates(attribute_6);
  attribute_store_set_reported_number(attribute_6, 10);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(10, attribute_store_get_reported_number(attribute_3));
}

void test_mapper_engine_pause_and_resume()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();

  TEST_ASSERT_TRUE(e.add_expression(R"(scope 0 { r'2 =  r'5})"));

  attribute_mapper_pause_mapping();
  auto attribute_5 = attribute::root().add_node(5);
  attribute_store_set_reported_number(attribute_5, 5);
  contiki_test_helper_run(0);

  auto attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_FALSE(attribute_2.is_valid());
  attribute_mapper_resume_mapping();

  attribute_store_set_reported_number(attribute_5, 0);
  contiki_test_helper_run(0);
  attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_TRUE(attribute_2.is_valid());
}

void test_mapper_engine_reported_create_attributes_setting()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();

  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 0 create_attributes(0) {
      r'2 =  r'11
      e'3 =  r'11
      d'4 =  r'11
    }
  )"));

  // Testing setting 0. (existence only)
  auto attribute_11 = attribute::root().add_node(11);
  attribute_store_set_reported_number(attribute_11, 5);
  contiki_test_helper_run(0);
  auto attribute_2 = attribute::root().child_by_type(2);
  auto attribute_3 = attribute::root().child_by_type(3);
  auto attribute_4 = attribute::root().child_by_type(4);
  TEST_ASSERT_FALSE(attribute_2.is_valid());
  TEST_ASSERT_TRUE(attribute_3.is_valid());
  TEST_ASSERT_FALSE(attribute_4.is_valid());

  e.reset();
  attribute::root().delete_node();
  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 0 create_attributes(2) {
      r'2 =  r'11
      e'3 =  r'11
      d'4 =  r'11
    }
  )"));

  // Testing setting 2. (DRE)
  attribute_11 = attribute::root().add_node(11);
  attribute_store_set_reported_number(attribute_11, 5);
  contiki_test_helper_run(0);
  attribute_2 = attribute::root().child_by_type(2);
  attribute_3 = attribute::root().child_by_type(3);
  attribute_4 = attribute::root().child_by_type(4);
  TEST_ASSERT_TRUE(attribute_2.is_valid());
  TEST_ASSERT_TRUE(attribute_3.is_valid());
  TEST_ASSERT_TRUE(attribute_4.is_valid());

  e.reset();
  attribute::root().delete_node();
  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 0 create_attributes(1) {
      r'2 =  r'11
      e'3 =  r'11
      d'4 =  r'11
    }
  )"));

  // Testing setting 1. (RE)
  attribute_11 = attribute::root().add_node(11);
  attribute_store_set_reported_number(attribute_11, 5);
  contiki_test_helper_run(0);
  attribute_2 = attribute::root().child_by_type(2);
  attribute_3 = attribute::root().child_by_type(3);
  attribute_4 = attribute::root().child_by_type(4);
  TEST_ASSERT_TRUE(attribute_2.is_valid());
  TEST_ASSERT_TRUE(attribute_3.is_valid());
  TEST_ASSERT_FALSE(attribute_4.is_valid());
}

void test_mapper_engine_common_parent_type_override()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();

  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 0 common_parent_type(3) {
      r'2 =  r'11
    }
  )"));

  // Maps need to happen under a node of type 3.
  // Adding r'11 under the root will not do anything.
  auto attribute_0_11 = attribute::root().add_node(11);
  attribute_store_set_reported_number(attribute_0_11, 5);
  contiki_test_helper_run(0);
  auto attribute_0_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_FALSE(attribute_0_2.is_valid());

  // Now add Node Type 3:
  auto attribute_3    = attribute::root().add_node(3);
  auto attribute_3_11 = attribute_3.add_node(11);
  attribute_store_set_reported_number(attribute_3_11, 5);
  contiki_test_helper_run(0);
  auto attribute_3_2 = attribute_3.child_by_type(2);
  TEST_ASSERT_TRUE(attribute_3_2.is_valid());
  TEST_ASSERT_EQUAL(5, attribute_store_get_reported_number(attribute_3_2));
}

void test_mapper_engine_number_and_expression_defines()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();
  TEST_ASSERT_TRUE(e.add_expression(R"(
    // Number def
    def number_11 11
    def expression_1 fn_max_value(r'number_11)
    def number_2 0x2
    def expression_2 (number_2 * expression_1)
    def expression_3 (0x4 + r'11)
    scope 0 {
      r'2 = expression_1
      r'3 = fn_min_value(r'2*30, number_2)
      r'4 = expression_3
    }
  )"));

  auto attribute_11 = attribute::root().add_node(11);
  attribute_store_set_reported_number(attribute_11, 1);
  contiki_test_helper_run(0);
  auto attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_TRUE(attribute_2.is_valid());
  TEST_ASSERT_EQUAL(1, attribute_store_get_reported_number(attribute_2));
  auto attribute_3 = attribute::root().child_by_type(3);
  TEST_ASSERT_TRUE(attribute_3.is_valid());
  TEST_ASSERT_EQUAL(2, attribute_store_get_reported_number(attribute_3));
  auto attribute_4 = attribute::root().child_by_type(4);
  TEST_ASSERT_TRUE(attribute_4.is_valid());
  TEST_ASSERT_EQUAL(5, attribute_store_get_reported_number(attribute_4));
}

void test_use_number_def_in_scope_settings()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();
  // This does not work yet
  TEST_IGNORE_MESSAGE("defs in scope settings not available yet");
  TEST_ASSERT_TRUE(e.add_expression(R"(
    // Number def
    def parent 0x02
    scope 0 common_parent_type(parent) {
      r'3 = r'11
    }
  )"));

  auto attribute_2  = attribute::root().add_node(2);
  auto attribute_11 = attribute_2.add_node(11);
  attribute_store_set_reported_number(attribute_11, 1111);
  contiki_test_helper_run(0);
  auto attribute_3 = attribute_2.child_by_type(3);
  TEST_ASSERT_TRUE(attribute_3.is_valid());
  TEST_ASSERT_EQUAL(1111, attribute_store_get_reported_number(attribute_3));
}

void test_use_multi_def_starting_with_same_substring()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();
  TEST_ASSERT_TRUE(e.add_expression(R"(
    // Number def
    def a 0x02
    def ab 1-2
    def abc r'23
    scope 0 {
      r'3 = abc - 3
    }
  )"));

  auto attribute_23 = attribute::root().add_node(23);
  attribute_store_set_reported_number(attribute_23, 1111);
  contiki_test_helper_run(0);
  auto attribute_3 = attribute::root().child_by_type(3);
  TEST_ASSERT_TRUE(attribute_3.is_valid());
  TEST_ASSERT_EQUAL(1108, attribute_store_get_reported_number(attribute_3));
}

void test_dependency_trigger_with_wrong_path()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();
  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 0 {
      r'3 = r'11
    }
  )"));

  // Here we try to trick the mapper, by creating attribute 1 at the wrong place
  // root -> 2 -> 1. Mapping happens under the root node parent type
  auto attribute_2_11 = attribute::root().add_node(2).add_node(11);
  attribute_store_set_reported_number(attribute_2_11, 21);
  contiki_test_helper_run(0);
  auto attribute_3 = attribute::root().child_by_type(3);
  TEST_ASSERT_FALSE(attribute_3.is_valid());

  // Now pause the mapper, create the right attribute 1
  attribute_mapper_pause_mapping();
  auto attribute_11 = attribute::root().add_node(11);
  attribute_store_set_reported_number(attribute_11, 1);
  attribute_mapper_resume_mapping();
  // Should still not have happened due to the mapping pause:
  attribute_3 = attribute::root().child_by_type(3);
  TEST_ASSERT_FALSE(attribute_3.is_valid());

  // Update the Attribute 1 under attribute 2:
  attribute_store_set_reported_number(attribute_2_11, 21);
  contiki_test_helper_run(0);

  // Modified an attribute with non-matching paths, so no eval.
  attribute_3 = attribute::root().child_by_type(3);
  TEST_ASSERT_FALSE(attribute_3.is_valid());

  // Update the right Attribute 1:
  attribute_store_set_reported_number(attribute_11, 2);
  contiki_test_helper_run(0);
  attribute_3 = attribute::root().child_by_type(3);
  TEST_ASSERT_TRUE(attribute_3.is_valid());
  TEST_ASSERT_EQUAL(2, attribute_store_get_reported_number(attribute_3));

  // Now pause mapping, modify attribute 3:
  attribute_mapper_pause_mapping();
  attribute_store_set_reported_number(attribute_11, 3);
  attribute_mapper_resume_mapping();
  // Expecting unchanged value:
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(2, attribute_store_get_reported_number(attribute_3));

  // Now try to modify the wrong attribute 1. what happens to Attribute 3?
  // It actually gets re-evaluated, which is kind of undesirable.
  attribute_store_set_reported_number(attribute_2_11, 22);
  contiki_test_helper_run(0);
  TEST_MESSAGE("Ideally here we still have the value 2");
  //TEST_ASSERT_EQUAL(2, attribute_store_get_reported_number(attribute_3));
  TEST_ASSERT_EQUAL(3, attribute_store_get_reported_number(attribute_3));
}

void test_reevaluation_after_deletion()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();
  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 0 {
      e'3 = r'5
      r'2 = r'3 or r'4
    }
  )"));

  // Here we check that r'2 gets set to r'4 after the mapper itself deletes r'3.
  auto attribute_5 = attribute::root().add_node(5);
  // Here attribute 3 becomes alive.
  attribute_store_set_reported_number(attribute_5, 1);
  contiki_test_helper_run(0);
  auto attribute_3 = attribute::root().child_by_type(3);
  TEST_ASSERT_TRUE(attribute_3.is_valid());
  TEST_ASSERT_FALSE(attribute_3.reported_exists());

  // Set a value in 3, so that 2 gets this value:
  auto attribute_4 = attribute::root().add_node(4);
  attribute_store_set_reported_number(attribute_3, 30);
  attribute_store_set_reported_number(attribute_4, 40);
  contiki_test_helper_run(0);
  // Attribute 2 gets created when r'3 is first defined:
  auto attribute_2 = attribute::root().child_by_type(2);
  TEST_ASSERT_TRUE(attribute_2.is_valid());
  TEST_ASSERT_EQUAL(30, attribute_store_get_reported_number(attribute_2));

  // Now set r'5 to 0, meaning that the mapper deletes Attribute 3
  attribute_store_set_reported_number(attribute_5, 0);
  contiki_test_helper_run(0);

  // Check that r'2 got updated to the value of r'4
  TEST_ASSERT_EQUAL(40, attribute_store_get_reported_number(attribute_4));
}

void test_scope_priority_simple_case()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();
  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 0 {
      r'1 = r'10
    }
    scope 1 {
      r'1 = r'2
    }
  )"));

  // Scope 1 should prevail. Let's first create attribute 10:
  auto attribute_10 = attribute::root().add_node(10);
  // Here attribute 1 becomes alive.
  attribute_store_set_reported_number(attribute_10, 10);
  contiki_test_helper_run(0);
  auto attribute_1 = attribute::root().child_by_type(1);
  TEST_ASSERT_TRUE(attribute_1.is_valid());
  TEST_ASSERT_TRUE(attribute_1.reported_exists());
  TEST_ASSERT_EQUAL(10, attribute_store_get_reported_number(attribute_1));

  // Now create attribute 2, it has higher priority:
  auto attribute_2 = attribute::root().add_node(2);
  attribute_store_set_reported_number(attribute_2, 2);
  contiki_test_helper_run(0);
  // Now attribute 1 has the attribute 2 value:
  TEST_ASSERT_EQUAL(2, attribute_store_get_reported_number(attribute_1));

  // Now test changing attribute 10, it will have 2 effect as attribute 2 provides a valid mapping:
  attribute_store_set_reported_number(attribute_10, 11);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(2, attribute_store_get_reported_number(attribute_1));

  // Delete attribute 2, the value will fall back on Attribute 10:
  attribute_2.delete_node();
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(11, attribute_store_get_reported_number(attribute_1));
}

void test_scope_priority_tricky_paths()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();
  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 10 {
      r'3.2 = r'10
    }
    scope 20 common_parent_type(3) {
      r'2 = r'11
    }
  )"));

  // Create attribute 3, which is the mapping base-point for both maps basically:
  auto attribute_3 = attribute::root().add_node(3);

  // Scope 20 should prevail. Let's first create attribute 10:
  auto attribute_10 = attribute::root().add_node(10);
  // Here attribute 2 becomes alive.
  attribute_store_set_reported_number(attribute_10, 10);
  contiki_test_helper_run(0);
  auto attribute_2 = attribute_3.child_by_type(2);
  TEST_ASSERT_TRUE(attribute_2.is_valid());
  TEST_ASSERT_TRUE(attribute_2.reported_exists());
  TEST_ASSERT_EQUAL(10, attribute_store_get_reported_number(attribute_2));

  // Now create attribute 11, it has higher priority under the other scope:
  auto attribute_11 = attribute_3.add_node(11);
  attribute_store_set_reported_number(attribute_11, 11);
  contiki_test_helper_run(0);
  // Now attribute 2 has the attribute 11 value (scope 20):
  TEST_ASSERT_EQUAL(11, attribute_store_get_reported_number(attribute_2));

  // Now test changing attribute 10, it will have no effect as attribute 11 provides a valid mapping:
  attribute_store_set_reported_number(attribute_10, 20);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(11, attribute_store_get_reported_number(attribute_2));

  // Delete attribute 11, the value will fall back on Attribute 10:
  attribute_11.delete_node();
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(20, attribute_store_get_reported_number(attribute_2));
}

void test_scope_priority_tricky_paths_part_2()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();
  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 1 {
      r'3.2 = r'10
    }
    scope 2 {
      r'3[0].2 = r'11
    }
  )"));

  // Create attribute 3, which is the mapping base-point for both maps basically:
  auto attribute_3 = attribute::root().add_node(3);

  // Attribute 11 has no value, so the scope 2 map does not match:
  auto attribute_11 = attribute::root().add_node(11);
  attribute_store_set_reported_number(attribute_11, 11);
  contiki_test_helper_run(0);
  auto attribute_2 = attribute_3.child_by_type(2);
  TEST_ASSERT_FALSE(attribute_2.is_valid());

  // Scope 1 map should work:
  auto attribute_10 = attribute::root().add_node(10);
  // Here attribute 2 becomes alive.
  attribute_store_set_reported_number(attribute_10, 10);
  contiki_test_helper_run(0);
  attribute_2 = attribute_3.child_by_type(2);
  TEST_ASSERT_TRUE(attribute_2.is_valid());
  TEST_ASSERT_EQUAL(10, attribute_store_get_reported_number(attribute_2));

  // Now get scope 2 to work:
  attribute_store_set_reported_number(attribute_3, 0);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(11, attribute_store_get_reported_number(attribute_2));

  // Update attribute 10, it should be ignored:
  attribute_store_set_reported_number(attribute_10, 100);
  contiki_test_helper_run(0);
  TEST_ASSERT_EQUAL(11, attribute_store_get_reported_number(attribute_2));
}

void test_scope_priority_parent_type_mismatched()
{
  MapperEngine &e = MapperEngine::get_instance();
  // Use attribute 10 as the common parent type:
  e.set_common_parent_type(10);
  e.reset();
  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 1 {
      r'3.2.11 = r'4
    }
    scope 2 {
      r'3.2[0].11 = r'4+1
    }
    scope 3 {
      r'3.2[20].11 = r'4+2
    }
  )"));

  // Create LHS attributes
  auto attribute_3  = attribute::root().add_node(3);
  auto attribute_2  = attribute_3.add_node(2);
  auto attribute_11 = attribute_2.add_node(11);

  // Create attribute 4 : nothing will happen as the common parent type is mismatched.
  auto attribute_4 = attribute::root().add_node(4);
  attribute_store_set_reported_number(attribute_4, 4);
  contiki_test_helper_run(0);
  TEST_ASSERT_FALSE(attribute_11.reported_exists());

  // Try at different levels:
  attribute_4 = attribute_3.add_node(4);
  attribute_store_set_reported_number(attribute_4, 4);
  contiki_test_helper_run(0);
  TEST_ASSERT_FALSE(attribute_11.reported_exists());

  attribute_4 = attribute_2.add_node(4);
  attribute_store_set_reported_number(attribute_4, 4);
  contiki_test_helper_run(0);
  TEST_ASSERT_FALSE(attribute_11.reported_exists());
}

void test_scope_priority_many_assignments_same_dependency()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();

  // Let's have everything depend on attribute 4:
  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 1 {
      r'3.2.11 = r'4
    }
    scope 2 {
      r'3.11 = r'4
    }
    scope 3 {
      r'2.11 = r'4
    }
    scope 4 {
     r'11 = r'4
    }
  )"));

  // Create LHS attributes
  auto attribute_3   = attribute::root().add_node(3);
  auto attribute_3_2 = attribute_3.add_node(2);
  auto attribute_2   = attribute::root().add_node(2);

  // Create attributes 4 : Actually here none of the LHS targets the same destination, so they should all apply regardless of priority:
  auto attribute_4 = attribute::root().add_node(4);
  attribute_store_set_reported_number(attribute_4, 4);
  contiki_test_helper_run(0);

  // Check all these attributes
  // Scope 1:
  auto attribute_11 = attribute_3_2.child_by_type(11);
  TEST_ASSERT_TRUE(attribute_11.is_valid());
  TEST_ASSERT_EQUAL(4, attribute_store_get_reported_number(attribute_11));

  // Scope 2:
  attribute_11 = attribute_3.child_by_type(11);
  TEST_ASSERT_TRUE(attribute_11.is_valid());
  TEST_ASSERT_EQUAL(4, attribute_store_get_reported_number(attribute_11));

  // Scope 3:
  attribute_11 = attribute_2.child_by_type(11);
  TEST_ASSERT_TRUE(attribute_11.is_valid());
  TEST_ASSERT_EQUAL(4, attribute_store_get_reported_number(attribute_11));

  // Scope 3:
  attribute_11 = attribute::root().child_by_type(11);
  TEST_ASSERT_TRUE(attribute_11.is_valid());
  TEST_ASSERT_EQUAL(4, attribute_store_get_reported_number(attribute_11));
}

void test_scope_priority_create_delete_schizophrenia()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();
  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 12 {
      e'3 = r'10
    }
    scope 200 {
      e'3 =
        if (r'11 == 1) 1
        if (r'11 == 0) 0
        undefined
    }
  )"));

  // Here we have a "destination" that would be created or deleted based on the
  // priority.
  auto attribute_10 = attribute::root().add_node(10);
  attribute_store_set_reported_number(attribute_10, 10);
  contiki_test_helper_run(0);

  auto attribute_3 = attribute::root().child_by_type(3);
  TEST_ASSERT_TRUE(attribute_3.is_valid());

  // Now create r'11, will not do anything without a value (scope 12 prevails):
  auto attribute_11 = attribute::root().add_node(11);
  contiki_test_helper_run(0);
  attribute_3 = attribute::root().child_by_type(3);
  TEST_ASSERT_TRUE(attribute_3.is_valid());

  // Now set r'11 to 0, attribute 3 should be dead (scope 200):
  attribute_store_set_reported_number(attribute_11, 0);
  contiki_test_helper_run(0);
  attribute_3 = attribute::root().child_by_type(3);
  TEST_ASSERT_FALSE(attribute_3.is_valid());

  // Now set r'11 to 3, attribute 3 should be alive again (scope 12):
  attribute_store_set_reported_number(attribute_11, 3);
  contiki_test_helper_run(0);
  attribute_3 = attribute::root().child_by_type(3);
  TEST_ASSERT_TRUE(attribute_3.is_valid());

  // Now set r'10 to 0, attribute 3 should be dead (scope 12):
  attribute_store_set_reported_number(attribute_10, 0);
  contiki_test_helper_run(0);
  attribute_3 = attribute::root().child_by_type(3);
  TEST_ASSERT_FALSE(attribute_3.is_valid());

  // Now set r'11 to 1, attribute 3 should be alive (scope 200):
  attribute_store_set_reported_number(attribute_11, 1);
  contiki_test_helper_run(0);
  attribute_3 = attribute::root().child_by_type(3);
  TEST_ASSERT_TRUE(attribute_3.is_valid());
}

void test_scope_priority_multiple_value_type_dependencies()
{
  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();

  // Both assignments have to run, even if they have the same original node and
  // destination, because they assign a different value type.
  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 200 chain_reaction(0) clear_desired(0) {
      r'3 =
        if (fn_are_all_defined(r'4) == 0) undefined
        if (r'4 > 0) 1 0

      d'3 =
        if (fn_is_any_defined(d'4, r'4) == 0) undefined
        if ((d'4 or r'4) > 0) 1 0
    }
  )"));

  // Here we have a "destination" that would be created or deleted based on the
  // priority.
  auto attribute_4 = attribute::root().add_node(4);
  attribute_store_set_reported_number(attribute_4, 4);
  contiki_test_helper_run(0);  // that will create the attribute 3

  auto attribute_3 = attribute::root().child_by_type(3);
  TEST_ASSERT_TRUE(attribute_3.is_valid());
  TEST_ASSERT_EQUAL(1, attribute_store_get_reported_number(attribute_3));
  TEST_ASSERT_EQUAL(1, attribute_store_get_desired_number(attribute_3));

  attribute_store_set_reported_number(attribute_4, 0);
  attribute_store_undefine_desired(attribute_4);
  contiki_test_helper_run(0);

  TEST_ASSERT_EQUAL(0, attribute_store_get_reported_number(attribute_3));
  TEST_ASSERT_EQUAL(0, attribute_store_get_desired_number(attribute_3));
}

void test_clearance_assignments()
{
  attribute::root().delete_node();

  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();

  // Both assignments have to run, even if they have the same original node and
  // destination, because they assign a different value type.
  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 200 chain_reaction(0) clear_desired(0) {
      r'3 =
        if (fn_are_all_defined(r'4) == 0) undefined
        if (r'4 > 0) 1 0

      d'3 =
        if (fn_is_any_defined(d'4, r'4) == 0) undefined
        if ((d'4 or r'4) > 0) 1 0

      // Now have 3 get undefined when 4 is undefined
      c:r'3 = if(fn_are_all_defined(r'4) == 0) 1 undefined
      c:d'3 = if(fn_are_all_defined(r'4) == 0) 1 undefined
    }
  )"));

  // Create the dependency. Attribyute 4, with both Reported set.
  auto attribute_4 = attribute::root().add_node(4);
  attribute_store_set_reported_number(attribute_4, 4);
  contiki_test_helper_run(0);  // that will create the attribute 3

  auto attribute_3 = attribute::root().child_by_type(3);
  TEST_ASSERT_TRUE(attribute_3.is_valid());
  TEST_ASSERT_EQUAL(1, attribute_store_get_reported_number(attribute_3));
  TEST_ASSERT_EQUAL(1, attribute_store_get_desired_number(attribute_3));

  // Now undefine the reported of attribute 4, the clearance rules will apply
  attribute_store_undefine_reported(attribute_4);
  contiki_test_helper_run(0);

  TEST_ASSERT_FALSE(attribute_store_is_reported_defined(attribute_3));
  TEST_ASSERT_FALSE(attribute_store_is_desired_defined(attribute_3));
}

void test_clearance_and_instance_assignments_with_existence_type()
{
  attribute::root().delete_node();

  MapperEngine &e = MapperEngine::get_instance();
  e.set_common_parent_type(attribute::root().type());
  e.reset();

  // This is illegal, so both expression should just create logging errors and nothing will be mapped.
  TEST_ASSERT_TRUE(e.add_expression(R"(
    scope 99 chain_reaction(0) clear_desired(0) {
      c:e'2 =
        if (fn_are_all_defined(r'4) == 0) undefined
        if (r'4 > 0) 1 0

      i:e'3 =
        if (fn_are_all_defined(r'4) == 0) undefined
        if (r'4 > 0) 1 0
    }
  )"));

  // Create the dependency. Attribyute 4, with both Reported set.
  auto attribute_4 = attribute::root().add_node(4);
  attribute_store_set_reported_number(attribute_4, 4);
  contiki_test_helper_run(0);

  // Attribute 3 should not exist
  auto attribute_3 = attribute::root().child_by_type(3);
  TEST_ASSERT_FALSE(attribute_3.is_valid());

  // Add attribute 2, check that it does not get cleared in any way:
  auto attribute_2 = attribute::root().add_node(2);
  TEST_ASSERT_TRUE(attribute_2.is_valid());
  attribute_store_set_reported_number(attribute_2, 4);
  attribute_store_set_desired_number(attribute_2, 4);
  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(attribute_2));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(attribute_2));

  // Attribute 4 to 0:
  attribute_store_set_reported_number(attribute_4, 0);
  contiki_test_helper_run(0);

  TEST_ASSERT_TRUE(attribute_store_is_reported_defined(attribute_2));
  TEST_ASSERT_TRUE(attribute_store_is_desired_defined(attribute_2));
}
}
