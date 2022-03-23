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
#include "unify_validator.h"

#include <stdbool.h>

#define LOG_TAG "uic_validator_test"

extern "C" {

#include "unity.h"

  void test_validate_string_contains_letter_or_digit(void)
  {
    TEST_ASSERT_EQUAL(validator_validate_string_contains_letter_or_digit("new_loc"), true);
    TEST_ASSERT_EQUAL(validator_validate_string_contains_letter_or_digit("new-loc"), true);
    TEST_ASSERT_EQUAL(validator_validate_string_contains_letter_or_digit("new loc"), true);
    TEST_ASSERT_EQUAL(validator_validate_string_contains_letter_or_digit("1"),       true);
    TEST_ASSERT_EQUAL(validator_validate_string_contains_letter_or_digit("_"),       false);
    TEST_ASSERT_EQUAL(validator_validate_string_contains_letter_or_digit("---"),     false);
    TEST_ASSERT_EQUAL(validator_validate_string_contains_letter_or_digit(""),        false);
  }

  void test_validate_unid(void)
  {
    TEST_ASSERT_EQUAL(validator_validate_unid("zw-EE7F73CF-0001"),     true);
    TEST_ASSERT_EQUAL(validator_validate_unid("zw_EE7F73CF_0001"),     true);
    TEST_ASSERT_EQUAL(validator_validate_unid("zw_EE7F73CF-0001"),     true);
    TEST_ASSERT_EQUAL(validator_validate_unid("z"),                    true);
    TEST_ASSERT_EQUAL(validator_validate_unid("Z"),                    true);
    TEST_ASSERT_EQUAL(validator_validate_unid("2"),                    true);
    TEST_ASSERT_EQUAL(validator_validate_unid("zw-EE7F 73CF-0001"),    false);
    TEST_ASSERT_EQUAL(validator_validate_unid("zw-EE7F/73CF-0001"),    false);
    TEST_ASSERT_EQUAL(validator_validate_unid(R"(zw-EE7F\73CF-0001)"), false);
    TEST_ASSERT_EQUAL(validator_validate_unid(""),                     false);
    TEST_ASSERT_EQUAL(validator_validate_unid("\""),                   false);
    TEST_ASSERT_EQUAL(validator_validate_unid("\"\"\""),               false);
    TEST_ASSERT_EQUAL(validator_validate_unid("\'"),                   false);
    TEST_ASSERT_EQUAL(validator_validate_unid("\'\'\'"),               false);
  }

}
