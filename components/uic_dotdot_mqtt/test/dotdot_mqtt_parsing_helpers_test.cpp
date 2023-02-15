/******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#include "workaround.hpp"

extern "C" {

#include <unity.h>

/**
 * @brief Setup the test suite (called once before all test_xxx functions are called)
 *
 */
void suiteSetUp(void) {}

/**
 * @brief Teardown the test suite (called once after all test_xxx functions are called)
 *
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/**
 * @brief Setup before each test case
 *
 */
void setUp(void) {}

/**
 * @brief Teardown after each test case
 *
 */
void tearDown(void) {}

void test_parse_topic_group_id_sanity(void)
{
  dotdot_group_id_t group_id;

  TEST_ASSERT_FALSE(uic_dotdot_mqtt::parse_topic_group_id("ucl", group_id));
  TEST_ASSERT_FALSE(
    uic_dotdot_mqtt::parse_topic_group_id("ucl/by-group", group_id));
  TEST_ASSERT_FALSE(
    uic_dotdot_mqtt::parse_topic_group_id("ucl/by-group/", group_id));
  TEST_ASSERT_FALSE(
    uic_dotdot_mqtt::parse_topic_group_id("ucl/by-group/a", group_id));
  TEST_ASSERT_FALSE(
    uic_dotdot_mqtt::parse_topic_group_id("ucl/by-group/?", group_id));

  // below min group_id
  TEST_ASSERT_FALSE(
    uic_dotdot_mqtt::parse_topic_group_id("ucl/by-group/0", group_id));

  // min group_id case
  TEST_ASSERT_TRUE(
    uic_dotdot_mqtt::parse_topic_group_id("ucl/by-group/1", group_id));
  TEST_ASSERT_EQUAL(1, group_id);

  TEST_ASSERT_TRUE(uic_dotdot_mqtt::parse_topic_group_id(
    "ucl/by-group/21/OnOff/Commands/Toggle",
    group_id));
  TEST_ASSERT_EQUAL(21, group_id);

  // max group_id case
  TEST_ASSERT_TRUE(
    uic_dotdot_mqtt::parse_topic_group_id("ucl/by-group/65535/Command",
                                          group_id));
  TEST_ASSERT_EQUAL(65535, group_id);

  // above max group_id
  TEST_ASSERT_FALSE(
    uic_dotdot_mqtt::parse_topic_group_id("ucl/by-group/65536", group_id));
}
}
