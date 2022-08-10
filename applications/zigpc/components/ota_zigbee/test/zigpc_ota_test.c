/******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "zigpc_ota_zigbee.h"
#include "zigpc_common_zigbee.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp() {}

void tearDown() {}

void zigpc_ota_init_succeeds()
{
    sl_status_t status = SL_STATUS_OK;
    status = zigpc_ota_init();
    TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}

void zigpc_ota_configure_node_succeeds()
{
    sl_status_t status = SL_STATUS_OK;
    zigbee_eui64_t eui64            = {1, 2, 3, 4, 5, 6, 7, 8};
    status =  zigpc_ota_configure_node(eui64);
    TEST_ASSERT_EQUAL_HEX(SL_STATUS_OK, status);
}
