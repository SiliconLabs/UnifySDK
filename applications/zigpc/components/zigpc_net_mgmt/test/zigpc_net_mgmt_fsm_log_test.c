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
#include <string.h>

#include "unity.h"

#include "zigpc_net_mgmt_fsm.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp() {}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

void setUp() {}

void tearDown() {}

/**
 * @brief Verify all known states are covered
 * 
 */
void test_zigpc_net_mgmt_fsm_log_should_print_all_known_states(void)
{
  for (uint8_t i = (ZIGPC_NET_MGMT_FSM_STATE_MIN_VAL + 1);
       i < ZIGPC_NET_MGMT_FSM_STATE_MAX_VAL;
       i++) {
    TEST_ASSERT_TRUE(strstr(zigpc_net_mgmt_fsm_state_name(i), "UNKNOWN")
                     == NULL);
  }
}

/**
 * @brief Verify unknown state print
 * 
 */
void test_zigpc_net_mgmt_fsm_log_should_print_unkown_states(void)
{
  TEST_ASSERT_FALSE(
    strstr(zigpc_net_mgmt_fsm_state_name(ZIGPC_NET_MGMT_FSM_STATE_MIN_VAL),
           "UNKNOWN")
    == NULL);
  TEST_ASSERT_FALSE(
    strstr(zigpc_net_mgmt_fsm_state_name(ZIGPC_NET_MGMT_FSM_STATE_MAX_VAL),
           "UNKNOWN")
    == NULL);
  TEST_ASSERT_FALSE(strstr(zigpc_net_mgmt_fsm_state_name(0xFF), "UNKNOWN")
                    == NULL);
}

/**
 * @brief Verify all known events are covered
 * 
 */
void test_zigpc_net_mgmt_fsm_log_should_print_all_known_events(void)
{
  for (uint8_t i = (ZIGPC_NET_MGMT_FSM_EVENT_MIN_VAL + 1);
       i < ZIGPC_NET_MGMT_FSM_EVENT_MAX_VAL;
       i++) {
    TEST_ASSERT_TRUE(strstr(zigpc_net_mgmt_fsm_event_name(i), "UNKNOWN")
                     == NULL);
  }
}

/**
 * @brief Verify unknown state print
 * 
 */
void test_zigpc_net_mgmt_fsm_log_should_print_unkown_events(void)
{
  TEST_ASSERT_FALSE(
    strstr(zigpc_net_mgmt_fsm_state_name(ZIGPC_NET_MGMT_FSM_EVENT_MIN_VAL),
           "UNKNOWN")
    == NULL);
  TEST_ASSERT_FALSE(
    strstr(zigpc_net_mgmt_fsm_state_name(ZIGPC_NET_MGMT_FSM_EVENT_MAX_VAL),
           "UNKNOWN")
    == NULL);
  TEST_ASSERT_FALSE(strstr(zigpc_net_mgmt_fsm_state_name(0xFF), "UNKNOWN")
                    == NULL);
}
