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
#include "unity.h"
#include "zwave_s2_nonce_management.h"

// LibS2 includes
#include "s2_protocol.h"

// Generic includes
#include <string.h>

// Mock includes
#include "zwave_network_management_mock.h"

// Create an S2 Context here:
struct S2 test_ctx;
// S2 nonce management relies on the context owned by zwave_s2_transport.c
struct S2 *s2_ctx;

// Static test variables
static span_entry_t span_data = {};
static mpan_entry_t mpan_data = {};

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp()
{
  s2_ctx = &test_ctx;
  memset(s2_ctx, 0, sizeof(struct S2));
  memset(&span_data, 0, sizeof(span_entry_t));
  memset(&mpan_data, 0, sizeof(mpan_entry_t));
}

void test_zwave_s2_nonce_management_get_span_data()
{
  // Make sure we run with the ZW_CONTROLLER definition for LibS2
  TEST_ASSERT_EQUAL(254, MPAN_TABLE_SIZE);
  TEST_ASSERT_EQUAL(254, SPAN_TABLE_SIZE);

  zwave_node_id_t node_id = 2;
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    zwave_s2_get_span_data(node_id, &span_data));

  // Add some span data in our context
  test_ctx.span_table[1].rnode      = node_id + 1;
  test_ctx.span_table[1].state      = SPAN_NEGOTIATED;
  test_ctx.span_table[1].rx_seq     = 0x23;
  test_ctx.span_table[1].tx_seq     = 0xA0;
  test_ctx.span_table[1].d.rng.v[2] = 26;
  test_ctx.span_table[1].d.rng.v[3] = 96;
  test_ctx.span_table[1].d.rng.v[5] = 0xFF;
  test_ctx.span_table[1].d.rng.k[2] = 23;
  test_ctx.span_table[1].d.rng.k[3] = 10;
  test_ctx.span_table[1].d.rng.k[5] = 183;
  test_ctx.span_table[1].class_id   = 2;

  // Still not found
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    zwave_s2_get_span_data(node_id, &span_data));

  // Now use the right NodeID
  test_ctx.span_table[1].rnode = node_id;

  // Now it should be found
  TEST_ASSERT_EQUAL(SL_STATUS_OK, zwave_s2_get_span_data(node_id, &span_data));
  TEST_ASSERT_EQUAL(test_ctx.span_table[1].class_id, span_data.class_id);
  TEST_ASSERT_EQUAL(test_ctx.span_table[1].tx_seq, span_data.tx_sequence);
  TEST_ASSERT_EQUAL(test_ctx.span_table[1].rx_seq, span_data.rx_sequence);
  TEST_ASSERT_EQUAL(test_ctx.span_table[1].d.rng.df, span_data.df);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(test_ctx.span_table[1].d.rng.v,
                                span_data.working_state,
                                sizeof(span_data.working_state));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(test_ctx.span_table[1].d.rng.k,
                                span_data.key,
                                sizeof(span_data.key));
}

void test_zwave_s2_nonce_management_zwave_s2_set_span_table()
{
  zwave_node_id_t zpc_node_id = 6;
  zwave_network_management_get_node_id_IgnoreAndReturn(zpc_node_id);

  span_data.class_id         = 234;
  span_data.df               = 22;
  span_data.key[0]           = 1;
  span_data.key[2]           = 2;
  span_data.key[5]           = 4;
  span_data.working_state[0] = 4;
  span_data.working_state[3] = 94;
  span_data.working_state[5] = 34;
  span_data.rx_sequence      = 99;
  span_data.tx_sequence      = 98;

  // Make the first entry of the SPAN table occupied, so it takes the 2nd one
  test_ctx.span_table[0].state = SPAN_SOS;

  zwave_node_id_t node_id = 2;
  zwave_s2_set_span_table(node_id, &span_data);

  // There was nothing in the SPAN table,
  // so it should have taken the first empty slot.
  TEST_ASSERT_EQUAL(node_id, test_ctx.span_table[1].rnode);
  TEST_ASSERT_EQUAL(zpc_node_id, test_ctx.span_table[1].lnode);
  TEST_ASSERT_EQUAL(span_data.class_id, test_ctx.span_table[1].class_id);
  TEST_ASSERT_EQUAL(span_data.tx_sequence, test_ctx.span_table[1].tx_seq);
  TEST_ASSERT_EQUAL(span_data.rx_sequence, test_ctx.span_table[1].rx_seq);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(span_data.working_state,
                                test_ctx.span_table[1].d.rng.v,
                                sizeof(span_data.working_state));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(span_data.key,
                                test_ctx.span_table[1].d.rng.k,
                                sizeof(span_data.key));
}

void test_zwave_s2_nonce_management_zwave_s2_set_span_table_reuse_entry()
{
  zwave_node_id_t zpc_node_id = 6;
  zwave_network_management_get_node_id_IgnoreAndReturn(zpc_node_id);

  span_data.class_id         = 234;
  span_data.df               = 22;
  span_data.key[0]           = 1;
  span_data.key[2]           = 2;
  span_data.key[5]           = 4;
  span_data.working_state[0] = 4;
  span_data.working_state[3] = 94;
  span_data.working_state[5] = 34;
  span_data.rx_sequence      = 99;
  span_data.tx_sequence      = 98;

  // Make that Entry 3 has the NodeID we are looking for in the S2 Context
  zwave_node_id_t node_id          = 2;
  uint8_t entry                    = 15;
  test_ctx.span_table[entry].rnode = node_id;

  zwave_s2_set_span_table(node_id, &span_data);

  // There was nothing in the SPAN table,
  // so it should have taken the first empty slot.
  TEST_ASSERT_EQUAL(node_id, test_ctx.span_table[entry].rnode);
  TEST_ASSERT_EQUAL(zpc_node_id, test_ctx.span_table[entry].lnode);
  TEST_ASSERT_EQUAL(span_data.class_id, test_ctx.span_table[entry].class_id);
  TEST_ASSERT_EQUAL(span_data.tx_sequence, test_ctx.span_table[entry].tx_seq);
  TEST_ASSERT_EQUAL(span_data.rx_sequence, test_ctx.span_table[entry].rx_seq);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(span_data.working_state,
                                test_ctx.span_table[entry].d.rng.v,
                                sizeof(span_data.working_state));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(span_data.key,
                                test_ctx.span_table[entry].d.rng.k,
                                sizeof(span_data.key));
}

void test_zwave_s2_nonce_management_get_mpan_data()
{
  zwave_multicast_group_id_t group_id = 17;
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    zwave_s2_get_mpan_data(0, group_id, &mpan_data));

  // Add some MPAN data in our context
  test_ctx.mpan_table[1].class_id       = 23;
  test_ctx.mpan_table[1].state          = MPAN_SET;
  test_ctx.mpan_table[1].group_id       = group_id + 1;
  test_ctx.mpan_table[1].owner_id       = 0;
  test_ctx.mpan_table[1].inner_state[0] = 65;
  test_ctx.mpan_table[1].inner_state[3] = 66;
  test_ctx.mpan_table[1].inner_state[6] = 67;

  // Still not found (group_id not equal)
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    zwave_s2_get_mpan_data(0, group_id, &mpan_data));

  test_ctx.mpan_table[1].owner_id = 2;

  // Still not found (group_id and owner_node_id not equal)
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    zwave_s2_get_mpan_data(0, group_id, &mpan_data));

  test_ctx.mpan_table[1].group_id = group_id;

  // Still not found (owner_node_id not equal)
  TEST_ASSERT_EQUAL(SL_STATUS_NOT_FOUND,
                    zwave_s2_get_mpan_data(0, group_id, &mpan_data));

  // Now use the right settings
  test_ctx.mpan_table[1].owner_id = 0;

  // Now it should be found
  TEST_ASSERT_EQUAL(SL_STATUS_OK,
                    zwave_s2_get_mpan_data(0, group_id, &mpan_data));
  TEST_ASSERT_EQUAL(test_ctx.mpan_table[1].class_id, mpan_data.class_id);
  TEST_ASSERT_EQUAL(test_ctx.mpan_table[1].group_id, mpan_data.group_id);
  TEST_ASSERT_EQUAL(test_ctx.mpan_table[1].owner_id, mpan_data.owner_node_id);

  TEST_ASSERT_EQUAL_UINT8_ARRAY(test_ctx.mpan_table[1].inner_state,
                                mpan_data.inner_state,
                                sizeof(mpan_data.inner_state));
}

void test_zwave_s2_nonce_management_zwave_s2_set_mpan_data()
{
  zwave_node_id_t owner_node_id       = 23;
  zwave_multicast_group_id_t group_id = 17;

  mpan_data.class_id       = 234;
  mpan_data.group_id       = group_id;
  mpan_data.owner_node_id  = owner_node_id;
  mpan_data.inner_state[0] = 2;
  mpan_data.inner_state[2] = 4;
  mpan_data.inner_state[5] = 59;

  // Make the first entry of the MPAN table occupied, so it takes the 2nd one
  test_ctx.mpan_table[0].state = MPAN_SET;

  zwave_s2_set_mpan_data(owner_node_id, group_id, &mpan_data);

  // There was nothing in the SPAN table,
  // so it should have taken the first empty slot.
  TEST_ASSERT_EQUAL(mpan_data.class_id, test_ctx.mpan_table[1].class_id);
  TEST_ASSERT_EQUAL(MPAN_SET, test_ctx.mpan_table[1].state);
  TEST_ASSERT_EQUAL(group_id, test_ctx.mpan_table[1].group_id);
  TEST_ASSERT_EQUAL(owner_node_id, test_ctx.mpan_table[1].owner_id);

  TEST_ASSERT_EQUAL_UINT8_ARRAY(mpan_data.inner_state,
                                test_ctx.mpan_table[1].inner_state,
                                sizeof(mpan_data.inner_state));
}

void test_zwave_s2_nonce_management_zwave_s2_set_mpan_data_reuse_entry()
{
  // Make that Entry 3 has the GroupID / Owner ID we are looking for
  //  in the S2 Context
  uint8_t entry                       = 15;
  zwave_node_id_t owner_node_id       = 23;
  zwave_multicast_group_id_t group_id = 17;

  mpan_data.class_id       = 234;
  mpan_data.group_id       = group_id;
  mpan_data.owner_node_id  = owner_node_id;
  mpan_data.inner_state[0] = 2;
  mpan_data.inner_state[2] = 4;
  mpan_data.inner_state[5] = 59;

  test_ctx.mpan_table[entry].group_id = group_id;
  test_ctx.mpan_table[entry].owner_id = owner_node_id;

  zwave_s2_set_mpan_data(owner_node_id, group_id, &mpan_data);

  // There was nothing in the SPAN table,
  // so it should have taken the first empty slot.
  TEST_ASSERT_EQUAL(mpan_data.class_id, test_ctx.mpan_table[entry].class_id);
  TEST_ASSERT_EQUAL(MPAN_SET, test_ctx.mpan_table[entry].state);
  TEST_ASSERT_EQUAL(group_id, test_ctx.mpan_table[entry].group_id);
  TEST_ASSERT_EQUAL(owner_node_id, test_ctx.mpan_table[entry].owner_id);

  TEST_ASSERT_EQUAL_UINT8_ARRAY(mpan_data.inner_state,
                                test_ctx.mpan_table[entry].inner_state,
                                sizeof(mpan_data.inner_state));
}