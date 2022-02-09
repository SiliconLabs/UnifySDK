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
#include "aoxpc_unid.h"
#include "unity.h"

void suiteSetUp(void) {}

void setUp(void) {}

void tearDown(void) {}

// Keep this test first, once we set the UNID, it cannot really be unset.
void test_unid_not_set()
{
  aoa_id_t unid = "Test_UNID";
  TEST_ASSERT_FALSE(is_aoxpc_unid(unid));
  TEST_ASSERT_EQUAL(SL_STATUS_FAIL, get_aoxpc_unid(unid));
}

void test_unid_happy_case()
{
  const uint8_t test_address[] = {1, 2, 3, 4, 5, 6, 7, 8};
  uint8_t test_address_type    = 1;
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    set_aoxpc_address(test_address, test_address_type));

  aoa_id_t unid = "";
  TEST_ASSERT_EQUAL(SL_STATUS_OK, get_aoxpc_unid(unid));
  const char expected_unid[] = "ble-sr-060504030201";
  TEST_ASSERT_EQUAL_STRING(expected_unid, unid);

  TEST_ASSERT_TRUE(is_aoxpc_unid(unid));
  TEST_ASSERT_FALSE(is_aoxpc_unid("ble-pd-12345"));
}
