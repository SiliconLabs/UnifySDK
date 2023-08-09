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

#include <stdlib.h>

#include "workaround.hpp"
#include "unity.h"

#include "dotdot_cluster_id_definitions.h"
#include "dotdot_attribute_id_definitions.h"

extern "C" {

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

/// Called after each and every test
void tearDown() {}

void test_dotdot_mqtt_additional_value_names()
{
  // Try with attribute values without a name
  TEST_ASSERT_FALSE(uic_dotdot_has_attribute_value_a_name(1, 1, 1));
  TEST_ASSERT_EQUAL_STRING(
    "",
    uic_dotdot_get_attribute_value_name(1, 1, 1).c_str());

  TEST_ASSERT_FALSE(uic_dotdot_has_attribute_value_a_name(
    DOTDOT_STATE_CLUSTER_ID,
    DOTDOT_STATE_MAXIMUM_COMMAND_DELAY_ATTRIBUTE_ID,
    0));

  // Try with attribute values with a name
  TEST_ASSERT_TRUE(uic_dotdot_has_attribute_value_a_name(
    DOTDOT_STATE_CLUSTER_ID,
    DOTDOT_STATE_MAXIMUM_COMMAND_DELAY_ATTRIBUTE_ID,
    0xFFFFFFFE));
  TEST_ASSERT_EQUAL_STRING("infinite",
                           uic_dotdot_get_attribute_value_name(
                             DOTDOT_STATE_CLUSTER_ID,
                             DOTDOT_STATE_MAXIMUM_COMMAND_DELAY_ATTRIBUTE_ID,
                             0xFFFFFFFE)
                             .c_str());
}
}
