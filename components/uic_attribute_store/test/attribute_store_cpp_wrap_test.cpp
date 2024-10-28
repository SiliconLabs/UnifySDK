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
#include "attribute.hpp"

#include "attribute_store_fixt.h"
#include "datastore_fixt.h"
#include "workaround.hpp"

extern "C" {
using namespace attribute_store;
/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  datastore_fixt_setup(":memory:");
  attribute_store_init();

  // Ensure we start from scratch before creating our test network.
  attribute_store_delete_node(attribute_store_get_root());

  // attribute_store_log();  // Print out so we can try to see the network
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  datastore_fixt_teardown();
  return num_failures;
}

/// Called before each and every test
void setUp() {}

#define TEST_ASSERT_EXCEPTION(f)             \
  try {                                      \
    f;                                       \
    TEST_FAIL_MESSAGE("Expected exception"); \
  } catch (...) {                            \
  };

void test_attribute_store_cpp_wrap_get_set_test()
{
  attribute r = attribute::root();

  attribute n = r.add_node(1);
  //Check uint8
  n.set_reported<uint8_t>(42);
  TEST_ASSERT_EQUAL(42, n.reported<uint8_t>());
  TEST_ASSERT_EQUAL(42, n.reported<int32_t>());

  //Check uint16
  n.set_reported<uint16_t>(42);
  TEST_ASSERT_EQUAL(42, n.reported<uint8_t>());
  TEST_ASSERT_EQUAL(42, n.reported<int32_t>());

  //Check uint32
  n.set_reported<uint32_t>(424242);
  TEST_ASSERT_EQUAL(424242, n.reported<uint32_t>());

  //Check that we get an exception
  TEST_ASSERT_EXCEPTION(n.desired<uint32_t>());

  n.set_desired<int>(-42);
  TEST_ASSERT_EQUAL(-42, n.desired<int>());

  //Check that we get an exception because of wrong data size
  TEST_ASSERT_EXCEPTION(n.desired<uint8_t>());

  //Check that we can read an write a vector

  std::vector<uint8_t> v1 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0};
  n.set_desired<std::vector<uint8_t>>(v1);
  std::vector<uint8_t> v2 = {0};
  v2                      = n.desired<std::vector<uint8_t>>();
  TEST_ASSERT_TRUE(v1 == v2);

  //Can we store a string?
  n.set_desired<std::string>("hello world");
  std::string str = n.desired<std::string>();
  TEST_ASSERT_EQUAL_STRING("hello world",str.c_str());
  TEST_ASSERT_TRUE("hello world" == str);
  // Can we use a list container with a 32bit type?
  std::list<int32_t> l1 = {1, 2, 3, -4, 5, -6, 7, 8, 9, 0};
  n.set_desired(l1);
  std::list<int32_t> l2 = n.desired<std::list<int32_t>>();
  TEST_ASSERT_TRUE(l1 == l2);

  n.delete_node();
}

void test_attribute_store_cpp_wrap_navigation_test()
{
  /*         A
 *        / \
 *        B   E
 *       / \
 *      C   D
*/

  attribute a = attribute::root().add_node('A');
  attribute b = a.add_node('B');
  attribute e = a.add_node('E');
  attribute c = b.add_node('C');
  attribute d = b.add_node('D');

  //Test child count
  TEST_ASSERT_EQUAL(2, a.child_count());
  TEST_ASSERT_EQUAL(2, b.child_count());
  TEST_ASSERT_EQUAL(0, c.child_count());
  TEST_ASSERT_EQUAL(0, d.child_count());
  TEST_ASSERT_EQUAL(0, e.child_count());

  //Test child access and compare
  TEST_ASSERT_TRUE(a.child(0) == b);
  TEST_ASSERT_TRUE(a.child(1) == e);
  TEST_ASSERT_TRUE(b.child(0) == c);
  TEST_ASSERT_TRUE(b.child(1) == d);

  // Test first parent
  TEST_ASSERT_TRUE(d.first_parent('B') == b);
  TEST_ASSERT_EQUAL_INT(d.first_parent('E'), ATTRIBUTE_STORE_INVALID_NODE);
  TEST_ASSERT_TRUE(d.first_parent('A') == a);
  TEST_ASSERT_EQUAL_INT(d.first_parent('C'), ATTRIBUTE_STORE_INVALID_NODE);

  // Test parent
  TEST_ASSERT_TRUE(d.parent() == b);
  TEST_ASSERT_TRUE(c.parent() == b);
  TEST_ASSERT_TRUE(b.parent() == a);
  TEST_ASSERT_TRUE(e.parent() == a);
  TEST_ASSERT_TRUE(a.parent() == attribute::root());
  TEST_ASSERT_EQUAL_INT(attribute::root().parent(),
                        ATTRIBUTE_STORE_INVALID_NODE);

  //Test casing
  attribute_store_node_t _d = d;
  TEST_ASSERT_TRUE(d == _d);

  attribute n1 = d.add_node(0x42);
  attribute n2 = d.add_node(0x42);
  attribute n3 = d.add_node(0x42);

  n1.set_reported<uint8_t>(0);
  n1.set_desired<uint8_t>(3);

  n2.set_reported<uint8_t>(1);
  n2.set_desired<uint8_t>(4);

  n3.set_reported<uint8_t>(2);

  for (int i = 0; i < 3; i++) {
    TEST_ASSERT_EQUAL(i, d.child_by_type(0x42, i).reported<uint8_t>());
  }


  TEST_ASSERT_EQUAL(n3, d.child_by_type_and_value<uint8_t>(0x42, 2));
  TEST_ASSERT_EQUAL(
    n3, d.child_by_type_and_value<uint8_t>(0x42, 2, REPORTED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(
    n2, d.child_by_type_and_value<uint8_t>(0x42, 4, DESIRED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(
    n1, d.child_by_type_and_value<uint8_t>(0x42, 3, DESIRED_OR_REPORTED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(
    n3,
    d.child_by_type_and_value<uint8_t>(0x42, 2, DESIRED_OR_REPORTED_ATTRIBUTE));

  // DESIRED_OR_REPORTED_ATTRIBUTE checks for desired value if defined, otherwise takes reported.
  // Since n2 have it's desired value set to 4, it will not be returned here.
  TEST_ASSERT_EQUAL(
    ATTRIBUTE_STORE_INVALID_NODE,
    d.child_by_type_and_value<uint8_t>(0x42, 1, DESIRED_OR_REPORTED_ATTRIBUTE));
  TEST_ASSERT_EQUAL(
    ATTRIBUTE_STORE_INVALID_NODE,
    d.child_by_type_and_value<uint8_t>(0x42, 5, DESIRED_ATTRIBUTE));
}

void test_attribute_store_cpp_wrap_strings()
{
  attribute a = attribute::root().add_node(2);

  // Save a C-String, read it back as a std::string
  char test_c_string[] = "a string value";
  attribute_store_set_reported_string(a, test_c_string);

  try {
    auto read_value = a.reported<std::string>();
    TEST_ASSERT_EQUAL_STRING(read_value.c_str(), test_c_string);
  } catch (const std::exception &e) {
    TEST_FAIL_MESSAGE("Could not read the reported value back");
  }

  // Save a std::string, read it as a std::string:
  std::string new_value = std::string("my new string value");
  a.set_desired<std::string>(new_value);
  a.set_desired("my new string value");

  try {
    auto read_value = a.desired<std::string>();
    TEST_ASSERT_EQUAL_STRING("my new string value", read_value.c_str());
  } catch (const std::exception &e) {
    TEST_FAIL_MESSAGE("Could not read the desired value back");
  }

  char read_string[ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH];
  attribute_store_get_desired_string(a,
                                      read_string,
                                      ATTRIBUTE_STORE_MAXIMUM_VALUE_LENGTH);
  TEST_ASSERT_EQUAL_STRING("my new string value", read_string);
}

void test_attribute_store_cpp_wrap_emplace_happy_case()
{
  constexpr attribute_store_type_t node_id = 0x42;

  auto root = attribute::root();
  auto node = root.add_node(node_id);

  node.set_reported(12);

  TEST_ASSERT_EQUAL_MESSAGE(node,
                            root.emplace_node(node_id, 12),
                            "Should not create a new node");

  TEST_ASSERT_EQUAL_MESSAGE(node + 1,
                            root.emplace_node(node_id, 13),
                            "Should have created a new node");

  node.set_desired(15);


  TEST_ASSERT_EQUAL_MESSAGE(node,
                            root.emplace_node(node_id, 15, DESIRED_ATTRIBUTE),
                            "Should not create a new node");

  TEST_ASSERT_EQUAL_MESSAGE(node + 2,
                            root.emplace_node(node_id, 16, DESIRED_ATTRIBUTE),
                            "Should have created a new node");
}

} // extern "C"
