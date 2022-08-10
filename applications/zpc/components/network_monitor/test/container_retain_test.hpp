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

#ifndef CONTAINER_RETAIN_TEST_HPP
#define CONTAINER_RETAIN_TEST_HPP
#include "network_monitor_utils.h"
#include <vector>

#include "unity.h"

void test_buffered_delete()
{
  constexpr int size = 266;
  std::vector<int> vec;
  for (int i = 0; i < size; ++i) {
    vec.push_back(i);
  }

  retain(vec, [](std::vector<int>::const_iterator i) { return !(*i) % 2; });

  for (int i = 0; i < size / 2; i += 2) {
    TEST_ASSERT_EQUAL(i, vec[i]);
  }
}

#endif