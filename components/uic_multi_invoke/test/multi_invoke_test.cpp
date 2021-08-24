#include "multi_invoke.hpp"

static multi_invoke<int, int> invoker;

extern "C" {
#include "unity.h"

static int cb1_val;
static int cb2_val;
static int cb1_calls = 0;
static int cb2_calls = 0;
void cb1(int val)
{
  cb1_val = val;
  cb1_calls++;
}
void cb2(int val)
{
  cb2_val = val;
  cb2_calls++;
}

void test_multi_invoke()
{
  // multi invoke should be empty to start with
  TEST_ASSERT_TRUE(invoker.empty());
  // Add callbacks on key 1
  invoker.add(1, cb1);

  TEST_ASSERT_FALSE(invoker.empty());
  // Add duplicate callback on key 1
  invoker.add(1, cb1);
  //invoker.add(1, cb1);
  invoker.add(1, cb2);
  // Call key 1
  invoker(1, 42);
  TEST_ASSERT_EQUAL(1, cb1_calls);
  TEST_ASSERT_EQUAL(1, cb2_calls);
  TEST_ASSERT_EQUAL(42, cb1_val);
  TEST_ASSERT_EQUAL(42, cb2_val);
  // Don't call callbacks when calling on key 2
  invoker(2, 1);
  TEST_ASSERT_EQUAL(1, cb1_calls);
  TEST_ASSERT_EQUAL(1, cb2_calls);
  // Don't remove cb1 if using key 2
  invoker.remove(2, cb1);
  invoker(1, 43);
  TEST_ASSERT_EQUAL(2, cb1_calls);
  TEST_ASSERT_EQUAL(2, cb2_calls);
  TEST_ASSERT_EQUAL(43, cb1_val);
  TEST_ASSERT_EQUAL(43, cb2_val);
  // Add cb1 on key 2
  invoker.add(2, cb1);
  invoker(2, 99);
  TEST_ASSERT_EQUAL(99, cb1_val);
  TEST_ASSERT_EQUAL(3, cb1_calls);
  // Remove cb1
  invoker.remove(1, cb1);
  invoker(1, 52);
  TEST_ASSERT_EQUAL(3, cb1_calls);
  TEST_ASSERT_EQUAL(3, cb2_calls);
  TEST_ASSERT_EQUAL(99, cb1_val);
  TEST_ASSERT_EQUAL(52, cb2_val);
  invoker(2, 100);
  TEST_ASSERT_EQUAL(4, cb1_calls);
  TEST_ASSERT_EQUAL(100, cb1_val);
  // test invoker.contains
  TEST_ASSERT_TRUE(invoker.contains(1));
  TEST_ASSERT_TRUE(invoker.contains(2));
  TEST_ASSERT_FALSE(invoker.contains(3));
  // Test erase
  invoker.add(1, cb1);
  invoker(1, 1234);
  TEST_ASSERT_EQUAL(5, cb1_calls);
  TEST_ASSERT_EQUAL(4, cb2_calls);
  TEST_ASSERT_EQUAL(1234, cb1_val);
  TEST_ASSERT_EQUAL(1234, cb2_val);
  invoker.erase(1);
  TEST_ASSERT_FALSE(invoker.contains(1));
  invoker(1, 0);
  TEST_ASSERT_EQUAL(5, cb1_calls);
  TEST_ASSERT_EQUAL(4, cb2_calls);
  TEST_ASSERT_EQUAL(1234, cb1_val);
  TEST_ASSERT_EQUAL(1234, cb2_val);
  // Test clear
  invoker.add(1, cb1);
  invoker.add(1, cb2);
  invoker.add(2, cb1);
  invoker.add(3, cb2);
  TEST_ASSERT_TRUE(invoker.contains(1));
  TEST_ASSERT_TRUE(invoker.contains(2));
  TEST_ASSERT_TRUE(invoker.contains(3));
  invoker.clear();
  TEST_ASSERT_FALSE(invoker.contains(1));
  TEST_ASSERT_FALSE(invoker.contains(2));
  TEST_ASSERT_FALSE(invoker.contains(3));
}
}
