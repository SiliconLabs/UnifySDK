#include <string.h>
#include "unity.h"
#include "zigpc_common_ucl.h"
#include "stdio.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp() {}

void tearDown() {}

void test_ucl_extract_unid_string_succeeds(void)
{
  //ARRANGE
  const char topic_string[] = "by-unid/zb-12345ABCDE/ep001/OnOff/Toggle";

  unsigned int unid_size = 12;  //all hex numbers, plus /, and space for NULL
  char unid_string[unid_size];

  sl_status_t status;

  //ACT
  status = ucl_extract_unid_string(topic_string, unid_string, unid_size);

  //ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_STRING("12345ABCDE", unid_string);
}

void test_ucl_extract_endpoint_string_succeeds(void)
{
  //ARRANGE
  const char topic_string[] = "by-unid/zb-12345ABCDE/ep001/OnOff/Toggle";

  unsigned int endpoint_size = 6;  //all hex numbers and space for NULL
  char endpoint_string[endpoint_size];

  sl_status_t status;

  //ACT
  status
    = ucl_extract_endpoint_string(topic_string, endpoint_string, endpoint_size);

  //ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_STRING("ep001", endpoint_string);
}

void test_ucl_extract_cluster_string_succeeds(void)
{
  //ARRANGE
  const char topic_string[] = "by-unid/zb-12345ABCDE/ep001/OnOff/Toggle";

  const size_t cluster_size = 6;
  char cluster_string[cluster_size];

  sl_status_t status;

  //ACT
  status
    = ucl_extract_cluster_string(topic_string, cluster_string, cluster_size);

  //ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_STRING("OnOff", cluster_string);
}

void test_ucl_extract_command_string_succeeds(void)
{
  //ARRANGE
  const char topic_string[] = "by-unid/zb-12345ABCDE/ep001/OnOff/Toggle";

  const size_t cmd_size = 7;
  char cmd_string[cmd_size];
  sl_status_t status;

  //ACT
  status = ucl_extract_cmd_string(topic_string, cmd_string, cmd_size);

  //ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
  TEST_ASSERT_EQUAL_STRING("Toggle", cmd_string);
}
