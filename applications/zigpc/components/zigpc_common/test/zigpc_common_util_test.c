#include "unity.h"

#include "zigpc_common_zigbee.h"
#include "workaround.hpp"

#include "sl_status.h"

void suiteSetUp() {}

int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp()
{
}

void tearDown() {}

void test_zigpc_binding_to_str_success(void)
{
    sl_status_t status = SL_STATUS_OK;
    TEST_ASSERT_EQUAL(SL_STATUS_OK, status);
}
