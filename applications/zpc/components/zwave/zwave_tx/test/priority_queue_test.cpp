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

// Includes from other components
#include "sl_log.h"
#include "unity.h"
#include "priority_queue.hpp"
#include "zwave_tx_queue.hpp"
extern "C" {

void test_reversed_priority_queue()
{
  priority_queue<int, 6, std::less<int>> queue;
  TEST_ASSERT_EQUAL_PTR(queue.begin(), queue.end());
  TEST_ASSERT_EQUAL(0, queue.size());
  TEST_ASSERT_TRUE(queue.empty());

  queue.insert(5);
  TEST_ASSERT_EQUAL(1, queue.size());
  TEST_ASSERT_FALSE(queue.empty());
  TEST_ASSERT_EQUAL_PTR(queue.end(), queue.begin() + 1);

  queue.insert(4);
  queue.insert(1);
  queue.insert(6);
  queue.insert(2);
  queue.insert(7);
  TEST_ASSERT_EQUAL(6, queue.size());
  TEST_ASSERT_EQUAL(1, queue.pop_front());
  TEST_ASSERT_EQUAL(5, queue.size());
  TEST_ASSERT_EQUAL(2, queue.pop_front());
  TEST_ASSERT_EQUAL(4, queue.size());
  TEST_ASSERT_EQUAL(4, queue.pop_front());
  TEST_ASSERT_EQUAL(3, queue.size());
  TEST_ASSERT_EQUAL(5, queue.pop_front());
  TEST_ASSERT_EQUAL(2, queue.size());
  TEST_ASSERT_EQUAL(6, queue.pop_front());
  TEST_ASSERT_EQUAL(1, queue.size());
  TEST_ASSERT_EQUAL(7, queue.pop_front());

  TEST_ASSERT_EQUAL_PTR(queue.begin(), queue.end());
  TEST_ASSERT_EQUAL(0, queue.size());
  TEST_ASSERT_TRUE(queue.empty());
}

void test_priority_queue_iterators()
{
  priority_queue<int, 6> queue;
  queue.insert(4);
  queue.insert(1);
  queue.insert(6);
  queue.insert(2);
  queue.insert(7);

  // please note that a heap is not a sorted array!
  // values can therefore not be in a linear order
  auto it = queue.begin();
  TEST_ASSERT_EQUAL(7, *it);
  it++;
  TEST_ASSERT_EQUAL(6, *it);
  it++;
  TEST_ASSERT_EQUAL(4, *it);
  it++;
  TEST_ASSERT_EQUAL(1, *it);
  it++;
  TEST_ASSERT_EQUAL(2, *it);
  it++;
  TEST_ASSERT_EQUAL_PTR(it, queue.end());
}

void test_max_capacity()
{
  priority_queue<int, 1> queue;
  TEST_ASSERT_TRUE(queue.insert(1));
  TEST_ASSERT_FALSE(queue.insert(3));
  TEST_ASSERT_EQUAL(queue.size(), queue.max_size());
}

void test_erase_of_element_front()
{
  priority_queue<int, 3> queue;
  queue.insert(1);
  queue.insert(2);
  queue.insert(3);

  TEST_ASSERT_EQUAL(2, *queue.erase(queue.begin()));
}

void test_erase_of_element_middle()
{
  priority_queue<int, 3> queue;
  queue.insert(1);
  queue.insert(2);
  queue.insert(3);

  TEST_ASSERT_EQUAL(2, *queue.erase(queue.begin() + 1));
}

void test_erase_of_element_one()
{
  priority_queue<int, 3> queue;
  queue.insert(1);

  auto it = queue.erase(queue.begin());
  TEST_ASSERT_EQUAL_PTR(queue.end(), it);
}

void test_erase_of_element_back()
{
  priority_queue<int, 3> queue;
  queue.insert(1);
  queue.insert(2);
  queue.insert(3);

  auto it = queue.erase(queue.begin() + 2);
  TEST_ASSERT_EQUAL_PTR(queue.end(), it);
}

void test_erase_of_elements_spanning_childs()
{
  priority_queue<int, 15> queue;
  queue.insert(10);
  queue.insert(6);
  queue.insert(8);
  queue.insert(3);
  queue.insert(4);
  queue.insert(2);
  queue.insert(1);

  for (int i = 0; i < 8; ++i) {
    queue.insert(1);
  }

  auto it = queue.erase(queue.begin() + 1);

  it = queue.begin();
  TEST_ASSERT_EQUAL(10, *it);
  it++;
  TEST_ASSERT_EQUAL(4, *it);
  it++;
  TEST_ASSERT_EQUAL(8, *it);
  it++;
  TEST_ASSERT_EQUAL(3, *it);
  it++;
  TEST_ASSERT_EQUAL(1, *it);
  it++;
  TEST_ASSERT_EQUAL(2, *it);
  it++;
  for (int i = 0; i < 8; ++i) {
    TEST_ASSERT_EQUAL(1, *it);
    it++;
  }
  TEST_ASSERT_EQUAL_PTR(it, queue.end());
}
}