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

/**
 * @file unity_helpers.h
 * @brief Various helpers/extensions for unity test framework
 *
 * This component contains various additions to the unity test framework
 *
 * @{
 */

#ifndef UNITY_CPP_HELPERS_HPP
#define UNITY_CPP_HELPERS_HPP
#ifdef __cplusplus
extern "C" {
#endif

void INTERNAL_TEST_ASSERT_EQUAL_JSON(const char *expected,
                                     const char *actual,
                                     unsigned int line);
/**
 * @brief Test that two JSON objects are equal,
 * 
 * see @ref INTERNAL_TEST_ASSERT_EQUAL_JSON implementations for details
 */
#define TEST_ASSERT_EQUAL_JSON(expected, actual) \
  INTERNAL_TEST_ASSERT_EQUAL_JSON(expected, actual, __LINE__)

#ifdef __cplusplus
}
#endif
#endif  //UNITY_CPP_HELPERS_HPP
/** @} end unity_cpp_helpers */
