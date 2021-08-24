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
#include "zwave_unid.h"
#include "zpc_endian.h"
#include "unity.h"

void setUp()
{
  zwave_unid_set_home_id(0xdeadbeef);
}

void test_unid_from_node_id()
{
  unid_t unid;

  zwave_unid_from_node_id(0x42, unid);
  TEST_ASSERT_EQUAL_STRING("zw-DEADBEEF-0042", unid);
}

void test_unid_to_node_id()
{
  zwave_node_id_t node_id;

  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_unid_to_node_id("zw-DEADBEEF-0042", &node_id));
  TEST_ASSERT_EQUAL(node_id, 0x42);

  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_KEY,
                    zwave_unid_to_node_id("zw-FEDEABE0-0042", &node_id));

  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_PARAMETER,
                    zwave_unid_to_node_id("xx-DEADBEEF-0042", &node_id));

  TEST_ASSERT_EQUAL(SL_STATUS_INVALID_RANGE,
                    zwave_unid_to_node_id("zw-DEADBEEF-00FF", &node_id));
}
