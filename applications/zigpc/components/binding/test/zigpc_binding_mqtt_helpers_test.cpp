#include <sl_status.h>
#include <string.h>
#include <stdio.h>

#include "zigpc_binding_mqtt_helpers.h"

extern "C" { 
#include "unity.h"
#include "cmock.h"

#include "zigpc_gateway_mock.h"
#include "zigpc_common_zigbee_mock.h"

void suiteSetUp(void) {}

int suiteTearDown(int num_failures)
{
  return num_failures;
}

void test_zigpc_binding_mqtt_sub_bind(void)
{
    sl_status_t status = SL_STATUS_OK; 
    TEST_ASSERT_EQUAL_HEX(SL_STATUS_FAIL, status);
}
}
