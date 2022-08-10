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
// Test includes
#include "unity.h"
#include "zwave_tx_incoming_frames.hpp"

extern "C" {

void test_imcoming_frames()
{
  // Instanciate a class for incoming frames:
  zwave_tx_incoming_frames incoming_frames;

  // No data in by default.
  TEST_ASSERT_EQUAL(0, incoming_frames.get_frames(0));
  TEST_ASSERT_EQUAL(0, incoming_frames.get_frames(1));
  TEST_ASSERT_EQUAL(0, incoming_frames.get_frames(2));
  TEST_ASSERT_EQUAL(0, incoming_frames.get_frames(3));
  TEST_ASSERT_TRUE(incoming_frames.empty());

  // Try to add frames for a NodeID 2 and 3:
  TEST_ASSERT_EQUAL(SL_STATUS_OK, incoming_frames.set_frames(2, 3));
  TEST_ASSERT_EQUAL(SL_STATUS_OK, incoming_frames.set_frames(3, 1));
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND, incoming_frames.set_frames(4, 0));
  TEST_ASSERT_FALSE(incoming_frames.empty());

  // Interact with NodeID 2:
  TEST_ASSERT_EQUAL(3, incoming_frames.get_frames(2));
  TEST_ASSERT_EQUAL(SL_STATUS_OK, incoming_frames.decrement_frames(2));
  TEST_ASSERT_EQUAL(2, incoming_frames.get_frames(2));
  TEST_ASSERT_EQUAL(SL_STATUS_OK, incoming_frames.decrement_frames(2));
  TEST_ASSERT_EQUAL(1, incoming_frames.get_frames(2));
  TEST_ASSERT_EQUAL(SL_STATUS_OK, incoming_frames.decrement_frames(2));
  TEST_ASSERT_EQUAL(0, incoming_frames.get_frames(2));
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND, incoming_frames.decrement_frames(2));
  TEST_ASSERT_EQUAL(0, incoming_frames.get_frames(2));

  // Data for NodeID 3 (and 4) should not have changed
  TEST_ASSERT_EQUAL(1, incoming_frames.get_frames(3));
  TEST_ASSERT_EQUAL(0, incoming_frames.get_frames(4));

  // SHould not be possible to decrement frames for NodeID 4
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND, incoming_frames.decrement_frames(4));

  // Set NodeID 2 frames to 200, then 0
  TEST_ASSERT_EQUAL(SL_STATUS_OK, incoming_frames.set_frames(2, 200));
  TEST_ASSERT_EQUAL(SL_STATUS_OK, incoming_frames.set_frames(2, 0));
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND, incoming_frames.decrement_frames(2));

  // Try to max out the system:
  for (zwave_node_id_t node_id = 0;
       node_id < ZWAVE_TX_INCOMING_FRAMES_BUFFER_SIZE;
       node_id++) {
    TEST_ASSERT_EQUAL(SL_STATUS_OK, incoming_frames.set_frames(node_id, 42));
  }
  // There should be no more capacity at that point
  TEST_ASSERT_EQUAL(
    SL_STATUS_FAIL,
    incoming_frames.set_frames(ZWAVE_TX_INCOMING_FRAMES_BUFFER_SIZE, 42));

  TEST_ASSERT_FALSE(incoming_frames.empty());
  incoming_frames.clear();
  TEST_ASSERT_TRUE(incoming_frames.empty());
  TEST_ASSERT_EQUAL(0, incoming_frames.get_frames(1));
  TEST_ASSERT_EQUAL(0, incoming_frames.get_frames(2));
  TEST_ASSERT_EQUAL(0, incoming_frames.get_frames(3));
}
}