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

// Component includes
#include "zigpc_discovery_events.hpp"
#include "zigpc_discovery_process.hpp"

extern "C" {

// Shared Unify includes
#include <unity.h>
#include <sys/process.h>
#include <contiki_test_helper.h>

PROCESS_NAME(zigpc_discovery_process);

/**
 * Setup the test suite (called once before all test_xxx functions are called)
 *
 */
void suiteSetUp(void) {}

/**
 * Teardown the test suite (called once after all test_xxx functions are called)
 *
 */
int suiteTearDown(int num_failures)
{
  return num_failures;
}

/**
 * Setup before each test case
 *
 */
void setUp(void)
{
  process_start(&zigpc_discovery_process, NULL);
  contiki_test_helper_init();
  contiki_test_helper_run_once(0);
}

/**
 * Teardown after each test case
 *
 */
void tearDown(void)
{
  process_exit(&zigpc_discovery_process);
  contiki_test_helper_run_once(0);
}

class TestEvent : public zigpc_discovery::events::DiscoverEvent
{
  public:
  int handle_calls = 0;

  TestEvent() : zigpc_discovery::events::DiscoverEvent("TestEvent") {}

  sl_status_t handle(void)
  {
    handle_calls++;

    return SL_STATUS_OK;
  }
};

void test_enqueueing_event(void)
{
  zigpc_discovery::process::event_t event = std::make_unique<TestEvent>();
  TestEvent *event_ptr = dynamic_cast<TestEvent *>(event.get());

  // ARRANGE

  // ACT
  sl_status_t add_status
    = zigpc_discovery::process::enqueue_event(std::move(event));
  TEST_ASSERT_EQUAL(0, event_ptr->handle_calls);

  size_t queue_size_add = zigpc_discovery::process::queue_size();

  contiki_test_helper_run_once(0);

  size_t queue_size_after = zigpc_discovery::process::queue_size();

  // ASSERT 2
  TEST_ASSERT_EQUAL(SL_STATUS_OK, add_status);
  TEST_ASSERT_EQUAL(1, queue_size_add);
  TEST_ASSERT_EQUAL(0, queue_size_after);
  TEST_ASSERT_EQUAL(1, event_ptr->handle_calls);
}

void test_peeking_event(void)
{
  zigpc_discovery::process::event_t event = std::make_unique<TestEvent>();
  TestEvent *event_ptr = dynamic_cast<TestEvent *>(event.get());

  // ARRANGE

  // ACT
  sl_status_t add_status
    = zigpc_discovery::process::enqueue_event(std::move(event));

  const zigpc_discovery::process::event_t *peek_event
    = zigpc_discovery::process::peek_event();

  // ASSERT
  TEST_ASSERT_EQUAL(SL_STATUS_OK, add_status);
  TEST_ASSERT_EQUAL_PTR(event_ptr, peek_event->get());
}

}  // extern "C"
