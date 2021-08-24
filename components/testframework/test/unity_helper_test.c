#include "unity_helpers.h"
// Unity stuff to be able to test for failed results
#include "unity.h"
#define TEST_INSTANCES
#include "self_assessment_utils.h"
static int SetToOneMeanWeAlreadyCheckedThisGuy;

/**
 * @brief Test that TEST_ASSERT_EQUAL_JSON work as expected
 * 
 */
void test_unity_json_equal() {
  // C interface
  TEST_ASSERT_EQUAL_JSON("{\"mybool\":true}", "{\"mybool\":true}");
  EXPECT_ABORT_BEGIN
  TEST_ASSERT_EQUAL_JSON("{\"mybool\":true}", "{\"mybool\":false}");
  VERIFY_FAILS_END
}