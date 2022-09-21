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
// Component being tested
#include "attribute_store_process.h"

// Test helpers
#include "contiki_test_helper.h"

#include "unity.h"

/// Setup the test suite (called once before all test_xxx functions are called)
void suiteSetUp()
{
  contiki_test_helper_init();
}

/// Teardown the test suite (called once after all test_xxx functions are called)
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/// Called before each and every test
void setUp() {}

void test_attribute_store_process_unhandled_events_test()
{
  // Inject a few error events for test coverage
  process_start(&unify_attribute_store_process, NULL);
  contiki_test_helper_run(1);

  // Unknown event
  process_post(&unify_attribute_store_process,
               RESTART_AUTO_SAVE_COOLDOWN_TIMER_EVENT + 1,
               NULL);

  contiki_test_helper_run(1);

  // Unknown timer:
  process_post(&unify_attribute_store_process, PROCESS_EVENT_TIMER, NULL);
  contiki_test_helper_run(1);

  // Unknown timer:
  process_exit(&unify_attribute_store_process);
  contiki_test_helper_run(1);
}