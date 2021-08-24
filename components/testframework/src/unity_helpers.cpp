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
#include "unity_helpers.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>

extern "C" {
#include "unity.h"
#include "unity_internals.h"
}

extern "C" {
void INTERNAL_TEST_ASSERT_EQUAL_JSON(const char *expected,
                                     const char *actual,
                                     unsigned int line)
{
  bool result;
  char dbg[1001];
  // In case of failed tests Unity will make a goto/long_jump which will cause a memory leak,
  // if the cpp destructors are not called for std::stringstream and boost::property_tree,
  // thus those are scoped here, and the debug string is copied to the dbg char array.
  {
    std::stringstream ss_expected;
    ss_expected << expected;
    std::stringstream ss_actual;
    ss_actual << actual;
    boost::property_tree::ptree pt_expected;
    boost::property_tree::read_json(ss_expected, pt_expected);
    boost::property_tree::ptree pt_actual;
    boost::property_tree::read_json(ss_actual, pt_actual);
    result = pt_expected == pt_actual;
    std::stringstream ss_dbg;
    if (!result) {
      ss_dbg << "Expected: ";
      boost::property_tree::json_parser::write_json(ss_dbg, pt_expected);
      ss_dbg << " Actual: ";
      boost::property_tree::json_parser::write_json(ss_dbg, pt_actual);
      strncpy(dbg, ss_dbg.str().c_str(), sizeof(dbg) - 1);
      // Ensure null termination
      dbg[1000] = 0;
    }
  }
  UNITY_TEST_ASSERT(result, line, dbg);
}

__attribute__((weak)) void setUp() {

};
__attribute__((weak)) void tearDown() {

};
}