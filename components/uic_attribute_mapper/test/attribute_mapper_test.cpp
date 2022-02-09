

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

using namespace attribute_store;

extern "C" {

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
            .emplace_node<uint8_t>(42, 0)
            .emplace_node(3333);
  n4444 = attribute::root()
            .emplace_node(1234)
            .emplace_node<uint8_t>(42, 1)
            .emplace_node(4444);
  attribute::root().emplace_node(1235).emplace_node(5555);

  n1234 = attribute::root().emplace_node(1234);
  n1235 = attribute::root().emplace_node(1235);

  n0 = attribute::root().emplace_node(1234).emplace_node<uint8_t>(42, 0);
  n1 = attribute::root().emplace_node(1234).emplace_node<uint8_t>(42, 1);
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
    }
  )");

  n1235.set_desired<uint32_t>(1);
  TEST_ASSERT_EQUAL(5, n1234.desired<uint32_t>());
  TEST_ASSERT_EQUAL(101, n4444.desired<uint32_t>());

  //  Check that we can create non existing attributes
  auto n11118888 = n0.child_by_type(11118888);
  TEST_ASSERT_TRUE(n11118888.is_valid());
  TEST_ASSERT_EQUAL(1, n11118888.reported<int32_t>());

  // Check that we evaluate the right hand side dependencies

  auto n3 = n1234.emplace_node<uint8_t>(42, 3);
  // As we have now created n3 the rule r'1234.42[3].11118888 = (d'9999) or (d'1235)
  // should succseed.
  auto n3_n11118888 = n3.child_by_type(11118888);
  TEST_ASSERT_TRUE(n3_n11118888.is_valid());
  TEST_ASSERT_EQUAL(1, n3_n11118888.reported<int32_t>());

  //Check that we delete the created node when we are deleting the n3 attribute
  n3.delete_node();
  TEST_ASSERT_FALSE(n3.child_by_type(11118888).is_valid());
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
  n1235.set_desired<uint32_t>(1);

  //Check that both attributes have the same desired value
  TEST_ASSERT_EQUAL(1, n1234.desired<int32_t>());
  TEST_ASSERT_EQUAL(1, n1235.desired<int32_t>());
  TEST_ASSERT_EQUAL(0, n1234.reported<int32_t>());
  TEST_ASSERT_EQUAL(0, n1235.reported<int32_t>());

  //Setting the repported value should clear the desired value
  n1234.set_reported<uint32_t>(1);
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

  TEST_IGNORE_MESSAGE("Make the hat operator work by beeing cleaver about the "
                      "depth in which we do evaluations.");

  TEST_ASSERT_EQUAL(43, n4444.reported<int>());
}
}
